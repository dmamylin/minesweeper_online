#pragma once

#include <sstream>
#include <string>

#include "maybe.h"
#include "string.h"

class Logger;

class LoggerLevel final {
public:
    friend class Logger;

public:
    LoggerLevel(const LoggerLevel&) = delete;
    LoggerLevel(LoggerLevel&&) = delete;
    LoggerLevel& operator=(const LoggerLevel&) = delete;
    LoggerLevel& operator=(LoggerLevel&&) = delete;

    ~LoggerLevel();

    template <typename T>
    LoggerLevel& operator<<(const T& data) {
        Buffer << data;
        return *this;
    }

private:
    LoggerLevel(Logger& parentLogger, const std::string& level);
    const std::string ToString() const;

private:
    Logger& ParentLogger;
    const std::string& Level;
    std::ostringstream Buffer;
};

class Logger final {
public:
    friend class LoggerLevel;
    friend Logger& Log();

public:
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

    ~Logger();

    LoggerLevel Warn();
    LoggerLevel Error();
    LoggerLevel Info();
    LoggerLevel Debug();

    static const Maybe<String>& SetLogFile(const Maybe<String>& filename);

private:
    struct LoggerImpl;

private:
    Logger();
    void Write(const LoggerLevel& level);

private:
    LoggerImpl* Impl;
};

Logger& Log();
