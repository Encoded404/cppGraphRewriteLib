#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include <gtest/gtest.h>

#include <modern_cmake_library/logging/logging.hpp>

#include "ostream_logger.hpp"

namespace test_logging {

// GTest listener that routes per-test logs to TEST_LOG_DIR/[suite]_[test].txt.
class PerTestFileLogger final : public ::testing::EmptyTestEventListener {
public:
    void OnTestStart(const ::testing::TestInfo& info) override {
        const char* env = std::getenv("TEST_LOG_DIR");
        if (!env || *env == '\0') {
            return;  // Logging disabled when directory is not provided.
        }

        const std::filesystem::path dir(env);
        active_ = ensure_directory(dir);
        if (!active_) {
            return;
        }

        const std::string filename = std::string(info.test_suite_name()) + "_" + info.name() + ".txt";
        const auto filepath = dir / filename;

        stream_.emplace(filepath);
        if (!stream_->is_open()) {
            stream_.reset();
            active_ = false;
            return;
        }

        logger_.emplace(*stream_);
        previous_logger_ = ::logiface::get_logger();
        ::logiface::set_logger(&*logger_);
    }

    void OnTestEnd(const ::testing::TestInfo&) override {
        if (active_) {
            ::logiface::set_logger(previous_logger_);
        }
        logger_.reset();
        stream_.reset();
        active_ = false;
        previous_logger_ = nullptr;
    }

private:
    static bool ensure_directory(const std::filesystem::path& dir) {
        std::error_code ec;
        if (std::filesystem::exists(dir, ec)) {
            return !ec;
        }
        std::filesystem::create_directories(dir, ec);
        return !ec;
    }

    std::optional<std::ofstream> stream_{};
    std::optional<::logiface::ostream_logger> logger_{};
    ::logiface::logger* previous_logger_{nullptr};
    bool active_{false};
};

inline void InstallPerTestFileLogger() {
    static bool installed = [] {
        ::testing::UnitTest::GetInstance()->listeners().Append(new PerTestFileLogger{});
        return true;
    }();
    (void)installed;
}

}  // namespace test_logging
