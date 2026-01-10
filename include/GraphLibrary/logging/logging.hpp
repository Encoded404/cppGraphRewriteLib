#ifndef LOGGING_LOGGING_HPP
#define LOGGING_LOGGING_HPP

#include <atomic>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#ifndef LOGIFACE_ENABLE_LOGGING
#define LOGIFACE_ENABLE_LOGGING 1
#endif

namespace logiface {

// Severity levels
enum class level {
    trace,
    debug,
    info,
    warn,
    error,
    critical
};

// Convert level to text
inline const char* to_string(level l) noexcept {
    switch (l) {
        case level::trace:    return "TRACE";
        case level::debug:    return "DEBUG";
        case level::info:     return "INFO";
        case level::warn:     return "WARN";
        case level::error:    return "ERROR";
        case level::critical: return "CRITICAL";
    }
    return "UNKNOWN";
}

// Log record
struct record {
    level lvl;
    std::string message;  // own the string
    const char* file;
    const char* function;
    int line;
    std::chrono::system_clock::time_point timestamp;
};

// Logger interface
struct logger {
    virtual ~logger() = default;
    virtual void log(const record& r) = 0;
    virtual void set_level(level lvl) noexcept = 0;
    virtual level get_level() const noexcept = 0;
};

// Global logger storage
namespace detail {
    inline std::atomic<std::shared_ptr<logger>> g_logger{nullptr};
}

inline void set_logger(std::shared_ptr<logger> l) noexcept {
    detail::g_logger.store(l, std::memory_order_release);
}

inline std::shared_ptr<logger> get_logger() noexcept {
    return detail::g_logger.load(std::memory_order_acquire);
}

#if LOGIFACE_ENABLE_LOGGING

#ifndef LOGIFACE_MIN_LEVEL
#define LOGIFACE_MIN_LEVEL trace
#endif

// Logging macro
#define LOGIFACE_LOG(lvl, msg_expr)                                             \
    do {                                                                         \
        if (static_cast<int>(logiface::level::lvl) <                              \
            static_cast<int>(logiface::level::LOGIFACE_MIN_LEVEL))               \
            break;                                                               \
        auto lg = logiface::get_logger();                                        \
        if (!lg) break;                                                          \
        if (static_cast<int>(logiface::level::lvl) < static_cast<int>(lg->get_level())) \
            break;                                                               \
        lg->log(logiface::record{                                               \
            logiface::level::lvl,                                               \
            (msg_expr), /* must produce std::string */                           \
            __FILE__,                                                            \
            __func__,                                                            \
            __LINE__,                                                            \
            std::chrono::system_clock::now()});                                  \
    } while (0)

#else
// Logging disabled
#define LOGIFACE_LOG(lvl, msg_expr) ((void)0)
#endif

// Optional helper macro to guard expensive expressions
#define LOGIFACE_ENABLED(lvl) \
    (LOGIFACE_ENABLE_LOGGING && static_cast<int>(logiface::level::lvl) >= static_cast<int>(logiface::level::LOGIFACE_MIN_LEVEL))

} // namespace logiface

#endif // LOGGING_LOGGING_HPP