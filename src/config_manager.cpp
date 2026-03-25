#include <config_manager.hpp>
#include <toml.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <shared_types.hpp>

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

ConfigStatus ConfigManager::load() {
    set_defaults();

    if (!fs::exists(config_path_)) {
        last_error_ = "Config file not found, creating defaults";
        (void)save();
        return ConfigStatus::FILE_NOT_FOUND;
    }

    try {
        auto config = toml::parse_file(config_path_.string());

        if (auto dark_mode = config["theme"]["dark_mode"].value<bool>()) {
            this->dark_mode = *dark_mode;
        } else {
            last_error_ = "Dark_mode not found in config";
            set_defaults();
            return ConfigStatus::PARSE_ERROR;
        }

        last_error_.clear();
        return ConfigStatus::SUCCESS;

    } catch (const toml::parse_error& e) {
        last_error_ = std::string("Parse error: ") + e.what();
        set_defaults();
        return ConfigStatus::PARSE_ERROR;
    } catch (const fs::filesystem_error& e) {
        last_error_ = std::string("Directory error: ") + e.what();
        set_defaults();
        return ConfigStatus::DIRECTORY_ERROR;
    } catch (const std::exception& e) {
        last_error_ = std::string("Load error: ") + e.what();
        set_defaults();
        return ConfigStatus::FAILURE;
    }
}

ConfigStatus ConfigManager::save() {
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
            last_error_ = "Failed to open file for writing (permission error)";
            return ConfigStatus::PERMISSION_ERROR;
        }

        file << config;
        if (!file.good()) {
            last_error_ = "Failed to write to file";
            return ConfigStatus::WRITE_ERROR;
        }

        last_error_.clear();
        return ConfigStatus::SUCCESS;

    } catch (const fs::filesystem_error& e) {
        last_error_ = std::string("Directory error: ") + e.what();
        return ConfigStatus::DIRECTORY_ERROR;
    } catch (const std::exception& e) {
        last_error_ = std::string("Save error: ") + e.what();
        return ConfigStatus::FAILURE;
    }
}
