#ifndef LOGGING_LOGGING_HPP
#define LOGGING_LOGGING_HPP

#include <atomic>
#include <chrono>
#include <string_view>

#ifndef LOGIFACE_ENABLE_LOGGING
#define LOGIFACE_ENABLE_LOGGING 1
#endif

namespace logiface {

enum class level {
    trace,
    debug,
    info,
    warn,
    error,
    critical
};

struct record {
    level lvl;
    std::string_view message;
    const char* file;
    const char* function;
    int line;
    std::chrono::system_clock::time_point timestamp;
};

struct logger {
    virtual ~logger() = default;
    virtual void log(const record& r) noexcept = 0;
};

namespace detail {
    inline std::atomic<logger*> g_logger{nullptr};
}

inline void set_logger(logger* l) noexcept {
    detail::g_logger.store(l, std::memory_order_release);
}

inline logger* get_logger() noexcept {
    return detail::g_logger.load(std::memory_order_acquire);
}

#if LOGIFACE_ENABLE_LOGGING
inline void log(level lvl,
                std::string_view msg,
                const char* file,
                const char* func,
                int line) noexcept {
    if (auto* lg = get_logger()) {
        lg->log(record{
            lvl,
            msg,
            file,
            func,
            line,
            std::chrono::system_clock::now()});
    }
}

#define LOGIFACE_LOG(lvl, msg)                                                \
    ::logiface::log(::logiface::level::lvl, std::string_view(msg), __FILE__,  \
                    __func__, __LINE__)

#else
inline void log(level, std::string_view, const char*, const char*, int) noexcept {}
#define LOGIFACE_LOG(lvl, msg) ((void)0)
#endif

} // namespace logiface

#endif // LOGGING_LOGGING_HPP