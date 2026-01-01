#include "core/Application.hpp"
#include "core/Logger.hpp"
#include <iostream>

int main() {
    try {
        Logger::info("Starting Interstate 3D...");
        
        Application app;
        
        if (!app.initialize()) {
            Logger::error("Failed to initialize application");
            return -1;
        }
        
        app.run();
        
        Logger::info("Application shutdown complete");
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return -1;
    }
}
