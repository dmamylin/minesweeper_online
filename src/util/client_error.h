#include <exception>

#include "string.h"

class ClientError : public std::exception {
public:
    explicit ClientError(const String& reason)
        : std::exception()
        , Reason(reason)
    {}

    const String& Message() const {
        return Reason;
    }

private:
    String Reason;

    const char* what() const override {
        return nullptr;
    }
};
