#include "file_manager.hpp"
#include <fstream>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <libgen.h>
#include <cstdlib>

FileOperationResult FileManager::load_file(const std::string& filename, std::vector<std::string>& buffer) {
    buffer.clear();
    
    std::ifstream ifs(filename);
    if (!ifs) {
        // File doesn't exist - start with empty buffer
        buffer.push_back("");
        return FileOperationResult(false, "File not found, new file created: \"" + filename + "\"", 0, StatusBarType::WARNING); // Not an error, just a new file
    }
    
    std::string line;
    while (std::getline(ifs, line)) {
        buffer.push_back(line);
    }
    
    if (buffer.empty()) {
        buffer.push_back("");
    }
    
    return FileOperationResult(true);
}

FileOperationResult FileManager::save_file(const std::string& filename, const std::vector<std::string>& buffer) {
    // Create directory if needed
    char* filename_copy = strdup(filename.c_str());
    char* dir = dirname(filename_copy);
    mkdir(dir, 0755);
    free(filename_copy);
    
    std::ofstream ofs(filename);
    if (!ofs) {
        int err = errno;
        std::string error_msg;
        StatusBarType status_type = StatusBarType::ERROR;
        
        if (err == EACCES) {
            error_msg = "Permission denied when saving file! Save as sudo? (y/n)";
            // TODO: implement "save as sudo" flow for permission errors
        } else if (err == ENOENT) {
            error_msg = "Directory does not exist!";
        } else if (err == EAGAIN) {
            error_msg = "Try again!";
            status_type = StatusBarType::WARNING;
        } else if (err == EROFS) {
            error_msg = "Read-only file system!";
        } else {
            error_msg = "Could not save file! (" + std::string(strerror(err)) + ")";
        }
        
        return FileOperationResult(false, error_msg, err, status_type);
    }
    
    // Write all lines to file
    for (const auto& line : buffer) {
        ofs << line << '\n';
    }
    
    // Check for I/O errors AFTER writing
    if (!ofs.good()) {
        return FileOperationResult(false, "I/O error while saving file!", 0, StatusBarType::ERROR);
    }
    
    return FileOperationResult(true, "File saved successfully", 0, StatusBarType::SUCCESS);
}

FileOperationResult FileManager::rename_file(const std::string& old_filename, const std::string& new_filename) {
    if (std::rename(old_filename.c_str(), new_filename.c_str()) != 0) {
        int err = errno;
        std::string error_msg;
        StatusBarType status_type = StatusBarType::ERROR;
        
        if (err == EACCES) {
            error_msg = "Permission denied when renaming file!";
        } else if (err == ENOENT) {
            error_msg = "File does not exist!";
        } else if (err == EAGAIN) {
            error_msg = "Try again!";
            status_type = StatusBarType::WARNING; 
        } else if (err == EEXIST) {
            error_msg = "A file with the new name already exists!";
        } else if (err == EISDIR || err == ENOTDIR) {
            error_msg = "Invalid file or directory!";
        } else {
            error_msg = "Could not rename file! (" + std::string(strerror(err)) + ")";
        }
        
        return FileOperationResult(false, error_msg, err, status_type);
    }
    
    return FileOperationResult(true, "File renamed to \"" + new_filename + "\"", 0, StatusBarType::SUCCESS);
}
