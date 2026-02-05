#pragma once
#include <string>
#include <vector>

class Editor {
public:
    Editor(const std::string& filename);
    void run(); // main loop (to be implemented later)
    void load_file();
    void save_file();
    const std::vector<std::string>& get_buffer() const;
    bool is_modified() const;
private:
    std::vector<std::string> buffer;
    std::string filename;
    bool modified = false;
};
