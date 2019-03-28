#pragma once

#include "admin_connection_manager.h"
#include "server_config.h"

class Application {
public:
    Application(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;

    int Run();

    static Application& GetInstance(int argc, const char** argv) {
        static Application instance(argc, argv);
        return instance;
    }

private:
    explicit Application(int argc, const char** argv);

private:
    ServerConfig Config;
    Holder<IAdminConnectionManager> AdminConnections;
};
