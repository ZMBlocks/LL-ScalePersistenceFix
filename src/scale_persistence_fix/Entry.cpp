#include "scale_persistence_fix/Entry.h"
#include <ll/api/memory/Hook.h>
#include <ll/api/mod/RegisterHelper.h>
#include <magic_enum.hpp>
#include <mc/deps/vanilla_components/ActorDataDirtyFlagsComponent.h>
#include <mc/nbt/CompoundTagVariant.h>
#include <mc/world/actor/Actor.h>
#include <ranges>
#include <utility>

namespace scale_persistence_fix {

Entry& Entry::getInstance() {
    static Entry instance;
    return instance;
}

bool Entry::load() { return true; }

bool Entry::enable() {
    ll::memory::HookRegistrar<ActorLoadHook, ActorSaveHook, ActorInitComponentHook>::hook();
    return true;
}

bool Entry::disable() {
    ll::memory::HookRegistrar<ActorLoadHook, ActorSaveHook, ActorInitComponentHook>::unhook();
    return true;
}

bool Entry::unload() { return true; }

void Entry::setBoundingBox(Actor& entity, ActorDataBoundingBoxComponent::ArrayType const& box) {
    entity.mEntityContext->getOrAddComponent<ActorDataBoundingBoxComponent>().mData = box;
    (
        *entity.mEntityContext->getOrAddComponent<ActorDataDirtyFlagsComponent>().mDirtyFlags
    )[std::to_underlying(ActorFlags::ShowBottom)] = true;
}

std::optional<ActorDataBoundingBoxComponent::ArrayType> Entry::getBoundingBox(Actor& entity) {
    return entity.mEntityContext->tryGetComponent<ActorDataBoundingBoxComponent>().transform([](auto&& component) {
        return component.mData;
    });
}

LL_TYPE_INSTANCE_HOOK(
    Entry::ActorLoadHook,
    HookPriority::Low,
    Actor,
    &Actor::$load,
    bool,
    CompoundTag const& tag,
    DataLoadHelper&    dataLoadHelper
) {
    if (auto result = origin(tag, dataLoadHelper); result) {
        if (tag.contains(mTagName, Tag::Type::List)
            && tag[mTagName].size() == magic_enum::enum_count<ActorDataBoundingBoxComponent::Type>()) {
            // tag[mTagName]
            //   | std::views::transform([](auto&& value) { return static_cast<FloatTag const&>(value).data; })
            //   | std::ranges::to<ActorDataBoundingBoxComponent::ArrayType>();
            ActorDataBoundingBoxComponent::ArrayType array;
            for (auto&& [index, value] : tag[mTagName] | std::views::enumerate) {
                array[index] = static_cast<FloatTag const&>(value);
            }
            setBoundingBox(*this, array);
            getInstance().mBoundingBoxes.emplace(this, array);
        }
        return true;
    }
    return false;
}

LL_TYPE_INSTANCE_HOOK(Entry::ActorSaveHook, HookPriority::Low, Actor, &Actor::save, bool, CompoundTag& entityTag) {
    if (auto result = origin(entityTag); result) {
        if (!entityTag.contains(mTagName)) {
            if (auto bbox = getBoundingBox(*this); bbox) {
                entityTag[mTagName] = *bbox
                                    | std::views::transform([](auto value) { return CompoundTagVariant{value}; })
                                    | std::ranges::to<ListTag>();
            }
        }
        return true;
    }
    return false;
}

LL_TYPE_INSTANCE_HOOK(
    Entry::ActorInitComponentHook,
    HookPriority::Low,
    Actor,
    &Actor::$initializeComponents,
    void,
    ActorInitializationMethod   method,
    VariantParameterList const& params
) {
    origin(method, params);
    auto& bboxs = Entry::getInstance().mBoundingBoxes;
    if (auto it = bboxs.find(this); it != bboxs.end()) {
        setBoundingBox(*this, it->second);
        bboxs.erase(it);
    }
}

} // namespace scale_persistence_fix

LL_REGISTER_MOD(scale_persistence_fix::Entry, scale_persistence_fix::Entry::getInstance());

static_assert(
    magic_enum::enum_count<ActorDataBoundingBoxComponent::Type>() == ActorDataBoundingBoxComponent::ArrayType{}.size(),
    "ActorDataBoundingBoxComponent::Type and ActorDataBoundingBoxComponent::ArrayType size mismatch"
);