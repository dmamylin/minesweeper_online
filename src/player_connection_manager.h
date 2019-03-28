#pragma once

#include "server_config.h"
#include "termination.h"
#include "util/holder.h"

class IPlayerConnectionManager : public ITerminationListener {
public:
    static Holder<IPlayerConnectionManager> Create(const ServerConfig& config);

public:
    virtual ~IPlayerConnectionManager() = default;

    
};
