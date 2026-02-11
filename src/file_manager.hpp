#pragma once
#include <string>
#include <vector>
#include "shared_types.hpp"

/// @brief Result structure for file operations
struct FileOperationResult {
    bool success;
    std::string message;
    int error_code;
    StatusBarType status_type;
    
    explicit FileOperationResult(bool success, const std::string& msg = "", int code = 0, StatusBarType type = StatusBarType::NORMAL) 
        : success(success), message(msg), error_code(code), status_type(type) {}
};

/// @brief Handles file I/O operations (load, save, and eventually rename)
class FileManager {
public:
    FileManager() = default;
    ~FileManager() = default;
    
    /// @brief Load file contents into buffer
    /// @param filename Path to file to load
    /// @param buffer Output buffer to fill with file contents
    /// @return Result indicating success or failure
    [[nodiscard]] FileOperationResult load_file(const std::string& filename, std::vector<std::string>& buffer);
    
    /// @brief Save buffer contents to file
    /// @param filename Path to file to save
    /// @param buffer Buffer containing lines to save
    /// @return Result indicating success or failure
    [[nodiscard]] FileOperationResult save_file(const std::string& filename, const std::vector<std::string>& buffer);
};
