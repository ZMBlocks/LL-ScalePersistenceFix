#pragma once
#include "scale_persistence_fix/ActorDataComponentBase.h"
#include <ll/api/mod/NativeMod.h>
#include <mc/deps/vanilla_components/ActorDataBoundingBoxComponent.h>

class Actor;

namespace scale_persistence_fix {

class Entry {
public:
    struct ActorSaveHook;
    struct ActorLoadHook;
    struct ActorInitComponentHook;
    constexpr static std::string_view mTagName = "BoundingBox";

public:
    static Entry& getInstance();

    Entry() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    bool load();
    bool enable();
    bool disable();
    bool unload();

    static void setBoundingBox(Actor& entity, ActorDataBoundingBoxComponent::ArrayType const& box);
    static std::optional<ActorDataBoundingBoxComponent::ArrayType> getBoundingBox(Actor& entity);

private:
    ll::mod::NativeMod&                                                 mSelf;
    ll::SmallDenseMap<Actor*, ActorDataBoundingBoxComponent::ArrayType> mBoundingBoxes;
};

} // namespace scale_persistence_fix