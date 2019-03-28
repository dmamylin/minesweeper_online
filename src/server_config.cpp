#include "server_config.h"

#include <stdexcept>
#include <fstream>
#include <Poco/JSON/Parser.h>
#include <sstream>
#include "types.h"
#include "util/log.h"

ServerConfig ReadConfig(const String& path) {
    try {
        std::ifstream configStream(path);
        if (!configStream.is_open()) {
            throw std::runtime_error("Cannot open file: " + path);
        }

        Poco::JSON::Parser parser;
        auto config = parser.parse(configStream).extract<Poco::JSON::Object::Ptr>();
        return {
            /*GamePort =*/config->getValue<u16>("game_port"),
            /*AdminPort =*/config->getValue<u16>("admin_port"),
            /*MaxPlayerConnections =*/config->getValue<u16>("max_player_connections"),
            /*LogPath =*/config->has("log_path") ? config->getValue<String>("log_path") : Nothing<String>()
        };
    } catch (const Poco::JSON::JSONException& exception) {
        std::stringstream reason;
        reason << "Cannot parse config due to error: " << exception.message();
        throw std::runtime_error(reason.str());
    }
}

ServerConfig ParseArguments(int argc, const char** argv) {
    if (argc != 3) {
        throw std::runtime_error("provide exactly 1 required argument: config path");
    }

    String argName = argv[1];
    if (argName != "-c" && argName != "--config") {
        std::stringstream reason;
        reason << "expected argument: --config or -c but got: " << argName;
        throw std::runtime_error(reason.str());
    }

    const auto config = ReadConfig(argv[2]);
    if (config.LogPath) {
        Logger::SetLogFile(*config.LogPath);
    }
    return config;
}
