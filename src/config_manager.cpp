#include "config_manager.hpp"
#include "toml.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

ConfigManager::ConfigManager() {
    config_path_ = get_config_path();
}

fs::path ConfigManager::get_config_path() const {
    const char* xdg_config_home = std::getenv("XDG_CONFIG_HOME");

    fs::path config_dir;
    if (xdg_config_home && fs::path(xdg_config_home).is_absolute()) {
        config_dir = fs::path(xdg_config_home) / "bznota";
    } else {
        const char* home = std::getenv("HOME");
        if (home) {
            config_dir = fs::path(home) / ".config" / "bznota";
        } else {
            config_dir = ".bznota";
        }
    }

    return config_dir / "config.toml";
}

void ConfigManager::set_defaults() {
    dark_mode = true;
}

bool ConfigManager::load() {
    set_defaults();

    if (!fs::exists(config_path_)) {
        return true;
    }

    try {
        auto config = toml::parse_file(config_path_.string());

        if (auto dark_mode = config["theme"]["dark_mode"].value<bool>()) {
            dark_mode = *dark_mode;
        }

        return true;
    } catch (const toml::parse_error& e) {
        std::cerr << "Failed to parse config: " << e.what() << std::endl;
        set_defaults();
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load config: " << e.what() << std::endl;
        set_defaults();
        return false;
    }
}

bool ConfigManager::save() {
    try {
        fs::path dir = config_path_.parent_path();
        if (!fs::exists(dir)) {
            fs::create_directories(dir);
        }

        toml::table config;
        config.insert("theme", toml::table{
            {"dark_mode", dark_mode}
        });

        std::ofstream file(config_path_);
        if (!file) {
            std::cerr << "Failed to open config file for writing: " << config_path_ << std::endl;
            return false;
        }

        file << config;
        return file.good();
    } catch (const std::exception& e) {
        std::cerr << "Failed to save config: " << e.what() << std::endl;
        return false;
    }
}
