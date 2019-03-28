#pragma once

#include "../types.h"
#include "field.h"

#include <mutex>

class GameSession {
public:
    struct Context {
        u32 MineCount = 10;
        u8 FieldWidth = 10;
        u8 FieldHeight = 10;
    };

public:
    explicit GameSession(const Context& ctx);

    void OnDisconnect();
    void OnConnect();

public:
    GameSession(const GameSession&) = delete;
    GameSession& operator=(const GameSession&) = delete;

private:
    std::mutex Mutex;
    Field GameField;
    u8 PlayerCount;
    bool GameIsRunning;
};
