#pragma once

#include <memory>

template <typename T>
using Holder = std::unique_ptr<T>;

template <typename T, typename... Args>
Holder<T> MakeHolder(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}
