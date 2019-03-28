#include "log.h"

#include <fstream>
#include <iostream>
#include <mutex>
#include <vector>

#include "holder.h"

constexpr auto LEVEL_ID_WARN = 0;
constexpr auto LEVEL_ID_ERROR = 1;
constexpr auto LEVEL_ID_INFO = 2;
constexpr auto LEVEL_ID_DEBUG = 3;

struct Logger::LoggerImpl {
    std::mutex Mutex;
    Holder<std::ofstream> LogFile;
    std::ostream& Output;
    std::vector<std::string> Levels = {
        "WARN",
        "ERRR",
        "INFO",
        "DEBG"
    };

    Holder<std::ofstream> CreateLogFile() {
        const auto& filename = Logger::SetLogFile(Nothing<String>());
        if (filename) {
            return MakeHolder<std::ofstream>(*filename);
        }
        return nullptr;
    }

    LoggerImpl()
        : LogFile(CreateLogFile())
        , Output(LogFile ? *LogFile : std::cout)
    {}
};

LoggerLevel::LoggerLevel(Logger& parentLogger, const std::string& level)
    : ParentLogger(parentLogger)
    , Level(level)
{}

LoggerLevel::~LoggerLevel() {
    ParentLogger.Write(*this);
}

const std::string LoggerLevel::ToString() const {
    return Buffer.str();
}

Logger::Logger()
    : Impl(new LoggerImpl())
{}

Logger::~Logger() {
    delete Impl;
}

LoggerLevel Logger::Warn() {
    return { *this, Impl->Levels[LEVEL_ID_WARN] };
}

LoggerLevel Logger::Error() {
    return { *this, Impl->Levels[LEVEL_ID_ERROR] };
}

LoggerLevel Logger::Info() {
    return { *this, Impl->Levels[LEVEL_ID_INFO] };
}

LoggerLevel Logger::Debug() {
    return { *this, Impl->Levels[LEVEL_ID_DEBUG] };
}

void Logger::Write(const LoggerLevel& level) {
    std::lock_guard<std::mutex> guard(Impl->Mutex);
    Impl->Output << level.Level << ' ' << level.ToString() << std::endl;
}

const Maybe<String>& Logger::SetLogFile(const Maybe<String>& filename) {
    static Maybe<String> name = filename;
    return name;
}

Logger& Log() {
    static Logger logger;
    return logger;
}
