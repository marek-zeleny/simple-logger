#pragma once

#include <cstdint>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <source_location>
#include <chrono>

namespace tiny_logger {

enum class LogLevel : uint8_t {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
};

struct TinyLoggerConfig {
#ifdef NDEBUG
    static constexpr LogLevel logLevel{LogLevel::Info};
#else
    static constexpr LogLevel logLevel{LogLevel::Debug};
#endif
    static constexpr long timezoneAdjustment{0};
    static inline std::ofstream logFile{"dp.log"};
};

template<LogLevel Level>
class TinyLogger {
public:
    explicit TinyLogger(std::ostream &stream, const std::source_location location = std::source_location::current()) :
            m_stream(stream) {
        *this << "[";
        print_time();
        *this << "][";
        print_log_level();
        *this << "][" << location.file_name() << ":" << location.line() << "]";
        *this << "[" << location.function_name() << "] ";
    }

    ~TinyLogger() {
        *this << std::endl;
    }

    template<typename T>
    TinyLogger &operator<<(const T &token) {
        if constexpr (Level <= TinyLoggerConfig::logLevel) {
            m_stream << token;
        }
        return *this;
    }

private:
    std::ostream &m_stream;

    void print_time() {
        auto now = std::chrono::high_resolution_clock::now();
        auto time_since_epoch = now.time_since_epoch();
        auto h = std::chrono::duration_cast<std::chrono::hours>(time_since_epoch).count() % 24
                + TinyLoggerConfig::timezoneAdjustment;
        auto min = std::chrono::duration_cast<std::chrono::minutes>(time_since_epoch).count() % 60;
        auto s = std::chrono::duration_cast<std::chrono::seconds>(time_since_epoch).count() % 60;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch).count() % 100;

        *this << std::setfill('0') << std::setw(2) << h << ":"
              << std::setfill('0') << std::setw(2) << min << ":"
              << std::setfill('0') << std::setw(2) << s << "."
              << std::setfill('0') << std::setw(3) << ms;
    }

    void print_log_level() {
        if constexpr (Level == LogLevel::Debug) {
            *this << "Debug";
        } else if constexpr (Level == LogLevel::Info) {
            *this << "Info";
        } else if constexpr (Level == LogLevel::Warning) {
            *this << "Warning";
        } else if constexpr (Level == LogLevel::Error) {
            *this << "Error";
        }
    }
};

} // tiny_logger

#define TINY_LOGGER_LOG(level, stream) tiny_logger::TinyLogger<tiny_logger::LogLevel::level>(stream)
#define LOG_DEBUG TINY_LOGGER_LOG(Debug, tiny_logger::TinyLoggerConfig::logFile)
#define LOG_INFO TINY_LOGGER_LOG(Info, tiny_logger::TinyLoggerConfig::logFile)
#define LOG_WARNING TINY_LOGGER_LOG(Warning, tiny_logger::TinyLoggerConfig::logFile)
#define LOG_ERROR TINY_LOGGER_LOG(Error, tiny_logger::TinyLoggerConfig::logFile)
