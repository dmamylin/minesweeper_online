#pragma once

#include <string>
#include <string_view>

using String = std::string;
using StringView = std::string_view;

StringView Strip(StringView view);
String Strip(const String& string);
