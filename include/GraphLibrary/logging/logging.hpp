#ifndef LOGGING_LOGGING_HPP
#define LOGGING_LOGGING_HPP

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <utility>

#ifndef LOGIFACE_ENABLE_LOGGING
#define LOGIFACE_ENABLE_LOGGING 1
#endif

namespace Logiface {

// Severity levels
enum class Level {
    trace,
    debug,
    info,
    warn,
    error,
    critical
};

// Convert level to text
inline const char* ToString(Level l) noexcept {
    switch (l) {
        case Level::trace:    return "TRACE";
        case Level::debug:    return "DEBUG";
        case Level::info:     return "INFO";
        case Level::warn:     return "WARN";
        case Level::error:    return "ERROR";
        case Level::critical: return "CRITICAL";
    }
    return "UNKNOWN";
}

// Log record
struct Record {
    Level lvl;
    std::string message;  // own the string
    const char* file;
    const char* function;
    int line;
    std::chrono::system_clock::time_point timestamp;
};

// Logger interface
struct Logger {
    virtual ~Logger() = default;
    virtual void Log(const Record& r) = 0;
    virtual void SetLevel(Level lvl) noexcept = 0;
    virtual Level GetLevel() const noexcept = 0;
};

// Global logger storage
namespace Detail {
    inline std::shared_ptr<Logger> g_logger{nullptr};
}

inline void SetLogger(std::shared_ptr<Logger> l) noexcept {
    Detail::g_logger = std::move(l);
}

inline std::shared_ptr<Logger> GetLogger() noexcept {
    return Detail::g_logger;
}

#if LOGIFACE_ENABLE_LOGGING

#ifndef LOGIFACE_MIN_LEVEL
#define LOGIFACE_MIN_LEVEL trace
#endif

// Logging macro
#define LOGIFACE_LOG(lvl, msg_expr)                                             \
    do {                                                                         \
        if (static_cast<int>(Logiface::Level::lvl) <                              \
            static_cast<int>(Logiface::Level::LOGIFACE_MIN_LEVEL))               \
            break;                                                               \
        auto lg = Logiface::GetLogger();                                         \
        if (!lg) break;                                                          \
        if (static_cast<int>(Logiface::Level::lvl) < static_cast<int>(lg->GetLevel())) \
            break;                                                               \
        lg->Log(Logiface::Record{                                               \
            Logiface::Level::lvl,                                               \
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
    (LOGIFACE_ENABLE_LOGGING && static_cast<int>(Logiface::Level::lvl) >= static_cast<int>(Logiface::Level::LOGIFACE_MIN_LEVEL))

} // namespace logiface

#endif // LOGGING_LOGGING_HPP