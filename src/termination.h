#pragma once

class ITerminationListener {
public:
    virtual ~ITerminationListener() = default;

    virtual void OnTermination() = 0;
};

class ITerminationController {
public:
    virtual ~ITerminationController() = default;

    virtual void AddTerminationListener(ITerminationListener&) = 0;
};
