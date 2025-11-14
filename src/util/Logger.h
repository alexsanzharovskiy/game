#pragma once
#include <iostream>
#include <string>

namespace util {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    static void Log(LogLevel level, const std::string& msg) {
        switch (level) {
        case LogLevel::DEBUG: std::cout << "[DEBUG] "; break;
        case LogLevel::INFO:  std::cout << "[INFO ] "; break;
        case LogLevel::WARN:  std::cout << "[WARN ] "; break;
        case LogLevel::ERROR: std::cout << "[ERROR] "; break;
        }
        std::cout << msg << std::endl;
    }

    static void Info(const std::string& msg)  { Log(LogLevel::INFO, msg); }
    static void Debug(const std::string& msg) { Log(LogLevel::DEBUG, msg); }
    static void Warn(const std::string& msg)  { Log(LogLevel::WARN, msg); }
    static void Error(const std::string& msg) { Log(LogLevel::ERROR, msg); }
};

} // namespace util
