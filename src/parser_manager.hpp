    #pragma once

    #include <string>
    #include <vector>
    #include <map>
    #include "md4c.h"
    #include "ftxui/dom/elements.hpp"
    #include "shared_types.hpp"

    // The intermediate object you requested: "hello"{bold}
    struct StyledChunk {
        std::string text;
        bool bold = false;
        bool italic = false;
        bool underline = false;
        bool strike = false;
        bool is_code = false; // For backticks `code`
    };

    class ParserManager {
    public:
        ParserManager() = default;
        ftxui::Element parse_line(const std::string& raw_line, EditorMode mode/* , bool is_active_line = false */);

    private:
        // Internal MD4C Bridge
        std::vector<StyledChunk> parse_markdown(const std::string& input);
        
        // FTXUI Builder
        ftxui::Element build_element(const std::vector<StyledChunk>& chunks);
    };