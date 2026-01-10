#ifndef LOGGING_CONSOLELOGGER_HPP
#define LOGGING_CONSOLELOGGER_HPP

#include "logging.hpp"

namespace logiface
{
    // Example thread-safe console logger
    struct console_logger : public logger {
        explicit console_logger(level lvl = level::info, 
                            std::ostream& out = std::clog,
                            std::ostream& err = std::cerr) noexcept
            : lvl_(lvl), out_(out), err_(err) {}

        void log(const record& r) override {
            std::lock_guard<std::mutex> g(mutex_);
            auto& os = (r.lvl == level::error || r.lvl == level::critical || r.lvl == level::warn)
                    ? err_
                    : out_;
            auto tt = std::chrono::system_clock::to_time_t(r.timestamp);
            os << "[" << std::put_time(std::localtime(&tt), "%F %T") << "] "
                    << to_string(r.lvl) << " "
                    << "(" << r.file << ":" << r.line << "): "
                    << r.message << '\n';
        }
        void set_level(level lvl) noexcept override { lvl_ = lvl; }
        level get_level() const noexcept override { return lvl_; }
    private:
        level lvl_;
        std::ostream& out_;
        std::ostream& err_;
        std::mutex mutex_;
    };
} // namespace logiface


#endif // LOGGING_CONSOLELOGGER_HPP