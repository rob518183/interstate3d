#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>

class Logger {
public:
    enum class Level {
        Debug,
        Info,
        Warning,
        Error
    };
    
    static void initialize(const std::string& logFile = "interstate3d.log");
    static void shutdown();
    
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
    
    static void setLevel(Level level);
    
private:
    static void log(Level level, const std::string& message);
    static std::string levelToString(Level level);
    static std::string getCurrentTimestamp();
    
    static std::unique_ptr<std::ofstream> s_logFile;
    static std::mutex s_mutex;
    static Level s_currentLevel;
    static bool s_initialized;
};
