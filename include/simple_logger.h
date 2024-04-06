/**
 * MIT License
 *
 * Copyright (c) 2024 Marek Zelený
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <cstdint>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <source_location>
#include <chrono>
#include <cstring>

namespace simple_logger {

enum class LogLevel : uint8_t {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
};

inline constexpr const char *logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "Debug";
        case LogLevel::Info: return "Info";
        case LogLevel::Warning: return "Warning";
        case LogLevel::Error: return "Error";
        default: return "Unknown";
    }
}

/**
 * Configuration class of the logger.
 *
 * Feel free to edit the constant values directly in this file, other values can be adjusted anywhere in code as needed.
 */
class Config {
public:
    /**
     * Determines verbosity of the logger.
     *
     * Only logs with equal or higher level will be printed by the logger, lower-level logs will be ignored.
     * The first value (if NDEBUG is set) is for RELEASE builds, the second value is for DEBUG builds.
     * (NDEBUG is automatically set if the appropriate compiler flag is used, otherwise feel free to use any other macro
     * available)
     */
#ifdef NDEBUG
    static constexpr LogLevel logLevel{LogLevel::Info};
#else
    static constexpr LogLevel logLevel{LogLevel::Debug};
#endif

    /**
     * If true, each log contains function signature, otherwise only file and line are logged.
     */
    static constexpr bool includeFunctionSignature{false};

    /**
     * If you need precise time information adjusted for timezone, use this variable to add/subtract hours.
     */
    static constexpr long timezoneAdjustment{0};

    /**
     * If logging to file is used, set this variable to the desired log file path/name.
     */
    static inline std::string logFileName{std::string(logLevelToString(logLevel)) + ".log"};

    /**
     * Determines the default stream for each log level where output will be printed.
     *
     * This function is used by convenience macros defined later.
     * Adjust the return values for individual levels to use desired output streams.
     *
     * NOTE: A function is used instead of static variables to correctly open log files if they're used.
     */
    template<LogLevel Level>
    static std::ostream &getDefaultStream() {
        if constexpr (Level == LogLevel::Debug) {
            return getLogFile();
        } else if constexpr (Level == LogLevel::Info) {
            return getLogFile();
        } else if constexpr (Level == LogLevel::Warning) {
            return getLogFile();
        } else if constexpr (Level == LogLevel::Error) {
            return getLogFile();
        } else {
            return std::cout;
        }
    }

    static std::ofstream &getLogFile() {
        if (!logFile.is_open()) {
            logFile = std::ofstream(logFileName);
        }
        return logFile;
    }

private:
    static inline std::ofstream logFile;
};

/**
 * Log class intended to be used as a temporary object for each log message.
 *
 * You can either use this class directly, or use the convenience macros defined later for less verbose usage.
 * @tparam Level Verbosity level of the log message (the message will be ignored if Config's log level is higher)
 */
template<LogLevel Level>
class Log {
public:
    explicit Log(std::ostream &stream = Config::getDefaultStream<Level>(),
            const std::source_location location = std::source_location::current()) :
            m_stream(isActive ? stream : nullStream) {
        if constexpr (isActive) {
            *this << "[";
            printTime();
            *this << "][" << logLevelToString(Level) << "][";
            printFileName(location.file_name());
            *this << ":" << location.line() << "]";
            if constexpr (Config::includeFunctionSignature) {
                *this << "[" << location.function_name() << "]";
            }
            *this << " ";
        }
    }

    ~Log() {
        if constexpr (isActive) {
            m_stream << std::endl;
        }
    }

    std::ostream &getStream() {
        return m_stream;
    }

    template<typename T>
    Log &operator<<(const T &token) {
        if constexpr (isActive) {
            m_stream << token;
        }
        return *this;
    }

private:
    static constexpr bool isActive{Level >= Config::logLevel};
    static inline std::ostream nullStream{nullptr};
    std::ostream &m_stream;

    /**
     * Very efficient (and simplistic) implementation of log timestamp.
     *
     * Using the STL's timezone-supporting implementation and format strings would slow down the logging by a lot.
     */
    void printTime() {
        auto now = std::chrono::high_resolution_clock::now();
        auto timeSinceEpoch = now.time_since_epoch();
        auto h = std::chrono::duration_cast<std::chrono::hours>(timeSinceEpoch).count() % 24
                + Config::timezoneAdjustment;
        auto min = std::chrono::duration_cast<std::chrono::minutes>(timeSinceEpoch).count() % 60;
        auto s = std::chrono::duration_cast<std::chrono::seconds>(timeSinceEpoch).count() % 60;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch).count() % 100;

        *this << std::setfill('0') << std::setw(2) << h << ":"
              << std::setfill('0') << std::setw(2) << min << ":"
              << std::setfill('0') << std::setw(2) << s << "."
              << std::setfill('0') << std::setw(3) << ms;
    }

    void printFileName(const char* filePath) {
        const char* slashPosition = std::strrchr(filePath, '/');
        if (slashPosition != nullptr) {
            *this << slashPosition + 1;
        } else {
            *this << filePath;
        }
    }
};

} // simple_logger

/**
 * Comment out this definition to disable convenience macros if you don't like them.
 */
#define SIMPLE_LOGGER_ENABLE_MACROS

#ifdef SIMPLE_LOGGER_ENABLE_MACROS

/**
 * Log message on a given level to default output stream with a single stream chain.
 */
#define SIMPLE_LOGGER_LOG(level) simple_logger::Log<simple_logger::LogLevel::level>()

/**
 * Log a debug message with a single stream chain.
 */
#define LOG_DEBUG SIMPLE_LOGGER_LOG(Debug)

/**
 * Log an info message with a single stream chain.
 */
#define LOG_INFO SIMPLE_LOGGER_LOG(Info)

/**
 * Log a warning message with a single stream chain.
 */
#define LOG_WARNING SIMPLE_LOGGER_LOG(Warning)

/**
 * Log an error message with a single stream chain.
 */
#define LOG_ERROR SIMPLE_LOGGER_LOG(Error)

/**
 * Create a local instance of a log on a given level and get the logger's default stream as a variable of given name.
 *
 * Use this macro (or derived macros) for more detailed log message control, e.g. giving the stream as a function
 * argument.
 */
#define GET_LOG_STREAM(level, name) \
    simple_logger::Log<simple_logger::LogLevel::level> _sl_log{}; std::ostream &name = _sl_log.getStream()

/**
 * Create a local instance of a debug log and get the logger's default stream as a variable of given name.
 */
#define GET_LOG_STREAM_DEBUG(name) GET_LOG_STREAM(Debug, name)

/**
 * Create a local instance of an info log and get the logger's default stream as a variable of given name.
 */
#define GET_LOG_STREAM_INFO(name) GET_LOG_STREAM(Info, name)

/**
 * Create a local instance of a warning log and get the logger's default stream as a variable of given name.
 */
#define GET_LOG_STREAM_WARNING(name) GET_LOG_STREAM(Warning, name)

/**
 * Create a local instance of an error log and get the logger's default stream as a variable of given name.
 */
#define GET_LOG_STREAM_ERROR(name) GET_LOG_STREAM(Error, name)

#endif // SIMPLE_LOGGER_ENABLE_MACROS
