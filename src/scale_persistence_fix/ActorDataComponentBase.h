#pragma once
#include <mc/deps/vanilla_components/ActorDataComponentBase.h>

namespace scale_persistence_fix {
template <typename T>
struct is_array : std::false_type {};
template <typename T, size_t N>
struct is_array<std::array<T, N>> : std::true_type {};
template <typename T>
concept is_array_v = is_array<T>::value;
} // namespace scale_persistence_fix

template <scale_persistence_fix::is_array_v T>
class ActorDataComponentBase<T> {
public:
    using ArrayType = T;

public:
    T mData;
};