#include "console.h"

#include <chrono>
#include <format>
#include <iostream>

static const char *level_enumstr[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "SEVERE",
    "FATAL"
};

namespace shimakaze {
    namespace console {
        void log(LoggingLevel level, const char *name, const char *message) {
            // convert enum to string & get current date
            std::string level_str = level_enumstr[static_cast<int>(level)];
            const auto now = std::chrono::system_clock::now();

            // format message
            std::cout << std::format("[{:%F}] [{}] {} >> {}", now, level_str.c_str(), name, message) << std::endl;
        }

        // make functions for each level
        void debug(const char *name, const char *message) {
            log(LoggingLevel::DBG, name, message);
        }

        void info(const char *name, const char *message) {
            log(LoggingLevel::INFO, name, message);
        }

        void warn(const char *name, const char *message) {
            log(LoggingLevel::WARN, name, message);
        }

        void error(const char *name, const char *message) {
            log(LoggingLevel::ERR, name, message);
        }

        void severe(const char *name, const char *message) {
            log(LoggingLevel::SEVERE, name, message);
        }

        void fatal(const char *name, const char *message) {
            log(LoggingLevel::FATAL, name, message);
        }

    }
}