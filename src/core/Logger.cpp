#include "Logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

std::unique_ptr<std::ofstream> Logger::s_logFile;
std::mutex Logger::s_mutex;
Logger::Level Logger::s_currentLevel = Logger::Level::Info;
bool Logger::s_initialized = false;

void Logger::initialize(const std::string& logFile) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    if (s_initialized) {
        return;
    }
    
    s_logFile = std::make_unique<std::ofstream>(logFile, std::ios::app);
    s_initialized = true;
    
    info("Logger initialized");
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    if (s_initialized) {
        info("Logger shutdown");
        s_logFile.reset();
        s_initialized = false;
    }
}

void Logger::debug(const std::string& message) {
    log(Level::Debug, message);
}

void Logger::info(const std::string& message) {
    log(Level::Info, message);
}

void Logger::warn(const std::string& message) {
    log(Level::Warning, message);
}

void Logger::error(const std::string& message) {
    log(Level::Error, message);
}

void Logger::setLevel(Level level) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_currentLevel = level;
}

void Logger::log(Level level, const std::string& message) {
    if (level < s_currentLevel) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(s_mutex);
    
    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = levelToString(level);
    std::string logEntry = "[" + timestamp + "] [" + levelStr + "] " + message;
    
    // Output to console
    std::cout << logEntry << std::endl;
    
    // Output to file if initialized
    if (s_initialized && s_logFile && s_logFile->is_open()) {
        *s_logFile << logEntry << std::endl;
        s_logFile->flush();
    }
}

std::string Logger::levelToString(Level level) {
    switch (level) {
        case Level::Debug:   return "DEBUG";
        case Level::Info:    return "INFO";
        case Level::Warning: return "WARN";
        case Level::Error:   return "ERROR";
        default:             return "UNKNOWN";
    }
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}
