#pragma once

#include "types.h"
#include "util/maybe.h"
#include "util/string.h"

struct ServerConfig {
    const u16 GamePort;
    const u16 AdminPort;
    const u16 MaxPlayerConnections;
    const Maybe<String> LogPath;
};

ServerConfig ParseArguments(int argc, const char** argv);
