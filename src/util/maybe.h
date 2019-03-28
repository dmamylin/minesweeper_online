#pragma once

#include <optional>

template <typename T>
using Maybe = std::optional<T>;

template <typename T>
Maybe<T> Nothing() {
    return std::nullopt;
}
