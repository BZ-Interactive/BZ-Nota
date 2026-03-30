#include <file_manager.hpp>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <libgen.h>
#include <cstdlib>
#include <unistd.h>

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

bool FileManager::privilege_is_cached() {
    std::string tool = get_privilege_tool();
    if (tool.empty()) return false;
    if (tool != "sudo") return false; // Only sudo supports reliable caching
    std::string check_cmd = tool + " -n true 2>/dev/null";
    return system(check_cmd.c_str()) == 0;
}

std::string FileManager::get_privilege_tool() {
    static std::string tool = [] {
        const char* tools[] = {"sudo", "doas", "pkexec"};
        for (const char* t : tools) {
            std::string check_cmd = std::string("command -v ") + t + " >/dev/null 2>&1";
            if (system(check_cmd.c_str()) == 0) {
                return std::string(t);
            }
        }
        return std::string();
    }();
    return tool;
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
            if (privilege_is_cached()) {
                return save_file_with_priviledge(filename, buffer, false);
            }
            error_msg = "Permission denied! Save with " + get_privilege_tool() + "? (y/n)";
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

std::string FileManager::shell_quote(const std::string& path) {
    std::string result = "'";
    for (char c : path) {
        if (c == '\'') result += "'\\''"; // Close, escape, reopen
        else result += c;
    }
    return result + "'";
}

FileOperationResult FileManager::save_file_with_priviledge(const std::string& filename, const std::vector<std::string>& buffer, bool interactive) {
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d", static_cast<int>(getpid()));
    std::string temp_file = "/tmp/bznota_priv_" + std::string(pid_str) + ".tmp";

    {
        std::ofstream temp_ofs(temp_file);
        if (!temp_ofs) {
            return FileOperationResult(false, "Failed to create temp file for privilege save!", 0, StatusBarType::ERROR);
        }
        for (const auto& line : buffer) {
            temp_ofs << line << '\n';
        }
    }

    std::string tool = get_privilege_tool();

    std::string safe_file = shell_quote(filename);
    std::string safe_temp = shell_quote(temp_file);

    // check for cached privilege, if cached bypass the interactive prompt
    int result;
    if (interactive && !privilege_is_cached()) {
        std::string cmd = "printf '\\033[2J\\033[H'; printf '\\nRequesting privilege access to save: " +
            filename + "\\n\\n'; " + tool + " tee " + safe_file + " > /dev/null < " + safe_temp;
        result = system(cmd.c_str());
    } else { // Only for sudo
        std::string cmd = tool + " -n tee " + safe_file + " > /dev/null < " + safe_temp;
        result = system(cmd.c_str());
    }
    std::remove(temp_file.c_str());

    if (result == -1) { // fail of shell execution, example: out of memory!
        return FileOperationResult(false, "Failed to execute shell command", -1, StatusBarType::ERROR);
    }

    int exit_code = WEXITSTATUS(result);
    if (exit_code != 0) {
        return FileOperationResult(false, tool + " save failed!", exit_code, StatusBarType::ERROR);
    }

    return FileOperationResult(true, "File saved with " + tool, 0, StatusBarType::SUCCESS);
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
