#pragma once

#include "../pch.h"

enum class LoggingLevel
{
    DBG = 0,
    INFO = 1,
    WARN = 2,
    ERR = 3,
    SEVERE = 4,
    FATAL = 5
};

namespace shimakaze::console
{
    void log(LoggingLevel level, const char* name, const char* message);
    void debug(const char* name, const char* message);
    void debug_if(const char* name, const char* message, bool post = true);
    void info(const char* name, const char* message);
    void warn(const char* name, const char* message);
    void error(const char* name, const char* message);
    void severe(const char* name, const char* message);
    void fatal(const char* name, const char* message);
}