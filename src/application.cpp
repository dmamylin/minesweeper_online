#include "application.h"

#include "util/log.h"

Application::Application(int argc, const char** argv)
    : Config(ParseArguments(argc, argv))
    , AdminConnections(IAdminConnectionManager::Create(Config))
{
}

int Application::Run() {
    AdminConnections->Start();
    AdminConnections->Wait();

    return 0;
}
