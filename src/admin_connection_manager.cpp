#include "admin_connection_manager.h"

#include "util/log.h"
#include "util/maybe.h"
#include "util/string.h"

#include <Poco/Net/NetException.h>
#include <Poco/Net/TCPServer.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>

using namespace Poco::Net;

class TerminationWaiter {
public:
    TerminationWaiter() = default;

    void Terminate() {
        std::lock_guard<std::mutex> lock(Mutex);
        ShouldTerminate = true;
        Cv.notify_all();
    }

    void Wait() {
        std::unique_lock<std::mutex> lock(Mutex);
        Cv.wait(lock, [this](){ return ShouldTerminate; });
    }

private:
    std::mutex Mutex;
    std::condition_variable Cv;
    bool ShouldTerminate = false;
};

class ConnectionOwner {
public:
    ConnectionOwner() = default;

    bool TryOwn() {
        if (!Mutex.try_lock()) {
            return false;
        }
        if (HasConnection) {
            Mutex.unlock();
            return false;
        }
        HasConnection = true;
        Mutex.unlock();
        return true;
    }

    void Release() {
        Log().Info() << "Trying to release a connection...";
        std::lock_guard<std::mutex> lock(Mutex);
        HasConnection = false;
        Log().Info() << "Connection released";
    }

private:
    std::mutex Mutex;
    bool HasConnection = false;
};

class AdminConnectionFilter : public TCPServerConnectionFilter {
public:
    explicit AdminConnectionFilter(ConnectionOwner& uniqueConnection)
        : UniqueConnection(uniqueConnection)
    {
    }

    bool accept(const StreamSocket& socket) override {
        return UniqueConnection.TryOwn();
    }

private:
    ConnectionOwner& UniqueConnection;
};

struct ConnectionContext {
    TerminationWaiter& Waiter;
    ConnectionOwner& UniqueConnection;
};

template <typename S>
class AdminConnectionFactory final : public TCPServerConnectionFactory {
public:
    AdminConnectionFactory(ConnectionContext& ctx)
        : TCPServerConnectionFactory()
        , Ctx(ctx)
    {
    }
    
    TCPServerConnection* createConnection(const StreamSocket& socket) {
        return new S(socket, Ctx);
    }

private:
    ConnectionContext& Ctx;
};

class AdminConnection final : public TCPServerConnection {
public:
    AdminConnection(const StreamSocket& socket, ConnectionContext& ctx)
        : TCPServerConnection(socket)
        , Socket(this->socket())
        , Ctx(ctx)
    {
        Socket.setBlocking(true);
        Log().Info() << "Admin connection open";
    }

    ~AdminConnection() {
        Log().Info() << "Admin connection closed";
        if (!StopOnConnectionClose) {
            Ctx.UniqueConnection.Release();
        } else {
            Ctx.Waiter.Terminate();
        }
    }

    void run() override {
        Log().Info() << "New admin connection from: " << Socket.peerAddress().toString();
        while (IsOpen) {
            try {
                auto bytesReceived = ReceiveBytes();
                if (bytesReceived <= 0) {
                    IsOpen = false;
                } else {
                    const auto answer = OnReceive(static_cast<size_t>(bytesReceived));
                    if (answer) {
                        SendAnswer(*answer);
                    }
                }
            } catch (Poco::Net::ConnectionResetException&) {
                Log().Warn() << "Admin connection was resetted";
                IsOpen = false;
            } catch (Poco::Exception& ex) {
                Log().Error() << "Admin connection closed due to error: " << ex.what();
                IsOpen = false;
            }
        }
    }

private:
    static constexpr size_t RECEIVE_BYTES_MAX = 1024;

    StreamSocket& Socket;
    ConnectionContext& Ctx;
    char ReceiveBuffer[RECEIVE_BYTES_MAX];
    bool IsOpen = true;
    bool StopOnConnectionClose = false;

private:
    int ReceiveBytes() {
        return Socket.receiveBytes(ReceiveBuffer, RECEIVE_BYTES_MAX);
    }

    void SendAnswer(const String& answer) {
        if (answer.size() == 0) {
            return;
        }

        const size_t bytesToSend = answer.size() + 1;
        int bytesSent = Socket.sendBytes(answer.c_str(), bytesToSend);
        if (bytesSent < 0) {
            Log().Error() << "AdminServer is in unknown state: " << bytesSent;
        }

        if (static_cast<size_t>(bytesSent) < bytesToSend) {
            Log().Error() << "AdminServer was sending " << bytesToSend
                          << " bytes but it is able to send only " << bytesSent;
        }
    }

    StringView GetCommand(size_t amountBytes) {
        if (amountBytes >= RECEIVE_BYTES_MAX) {
            amountBytes = RECEIVE_BYTES_MAX;
        }
        return {ReceiveBuffer, amountBytes};
    }

    Maybe<String> OnReceive(size_t amountBytes) {
        auto command = Strip(GetCommand(amountBytes));
        if (command == "STOP") {
            StopOnConnectionClose = true;
            IsOpen = false;
            return Nothing<String>();
        }

        return "Unknown command\n";
    }
};

class AdminConnectionManager final : public IAdminConnectionManager {
public:
    explicit AdminConnectionManager(const ServerConfig& config)
        : Ctx({Waiter, UniqueConnection})
        , Server(new AdminConnectionFactory<AdminConnection>(Ctx), config.AdminPort)
    {
        Log().Info() << "Admin server is listening for connections on port " << config.AdminPort;
        Server.setConnectionFilter(new AdminConnectionFilter(UniqueConnection));
    }

    ~AdminConnectionManager() {
        Shutdown();
    }

    void AddTerminationListener(ITerminationListener& listener) noexcept override {
        TerminationListeners.push_back(&listener);
    }

    void Start() override {
        Server.start();
        Log().Info() << "Admin server started";
    }

    void Wait() override {
        Log().Info() << "Admin server is waiting";
        Waiter.Wait();
        Shutdown();
    }

private:
    ConnectionOwner UniqueConnection;
    TerminationWaiter Waiter;
    ConnectionContext Ctx;

    TCPServer Server;
    std::vector<ITerminationListener*> TerminationListeners;

private:
    void Shutdown() {
        Server.stop();
        for (auto* listener : TerminationListeners) {
            listener->OnTerminate();
        }
        TerminationListeners.clear();
    }
};

Holder<IAdminConnectionManager> IAdminConnectionManager::Create(const ServerConfig& config) {
    return MakeHolder<AdminConnectionManager>(config);
}
