#include "string.h"

#include <cctype>

template <typename S>
S StripBase(const S& s) {
    if (s.size() == 0) {
        return {};
    }

    size_t begin = 0;
    for (; begin < s.size() && isspace(s[begin]); ++begin);
    if (begin == s.size()) {
        return {};
    }

    size_t end = static_cast<size_t>(s.size() - 1);
    for (; end > begin && isspace(s[end]); --end);

    return s.substr(begin, end - begin + 1);
}

StringView Strip(StringView view) {
    return StripBase(view);
}

String Strip(const String& string) {
    return StripBase(string);
}
