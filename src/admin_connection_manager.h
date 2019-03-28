#pragma once

#include "server_config.h"
#include "termination.h"
#include "util/holder.h"

class IAdminConnectionManager : public ITerminationController {
public:
    static Holder<IAdminConnectionManager> Create(const ServerConfig& config);

public:
    virtual ~IAdminConnectionManager() = default;

    virtual void Start() = 0;
    virtual void Wait() = 0;
};
