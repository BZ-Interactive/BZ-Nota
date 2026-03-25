#pragma once
#include <filesystem>
#include "shared_types.hpp"

class ConfigManager {
public:
    ConfigManager();

    [[nodiscard]] ConfigStatus load();
    [[nodiscard]] ConfigStatus save();

    const std::string& last_error() const { return last_error_; }

    bool is_dark_mode() const { return dark_mode; }
    void set_dark_mode(bool dark) { dark_mode = dark; }

private:
    std::filesystem::path get_config_path() const;
    void set_defaults();

    bool dark_mode = true;
    std::filesystem::path config_path_;
    std::string last_error_;
};
