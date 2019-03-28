#include "application.h"

#include <exception>
#include <iostream>

int main(int argc, const char** argv) {
    try {
        auto& instance = Application::GetInstance(argc, argv);
        return instance.Run();
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }
}
