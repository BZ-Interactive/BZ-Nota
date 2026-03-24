#pragma once
#include <filesystem>

class ConfigManager {
public:
    ConfigManager();

    bool load();
    bool save();

    bool is_dark_mode() const { return dark_mode; }
    void set_dark_mode(bool dark) { dark_mode = dark; }

private:
    std::filesystem::path get_config_path() const;
    void set_defaults();

    bool dark_mode = true;
    std::filesystem::path config_path_;
};
