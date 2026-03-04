#include "parser_manager.hpp"
#include "md4c.h"
#include <cstring>

using namespace ftxui;

namespace {

struct MDProperties {
    bool b = false, i = false, u = false, s = false, code = false;
    std::vector<StyledChunk>* out;
};

int md4c_enter_span(MD_SPANTYPE type, void* detail, void* userdata) {
    (void)detail; // they are needed for signature so don't remove
    auto* prop = static_cast<MDProperties*>(userdata);
    switch (type) {
        case MD_SPAN_STRONG: prop->b = true; break;
        case MD_SPAN_EM: prop->i = true; break;
        case MD_SPAN_U: prop->u = true; break;
        case MD_SPAN_DEL: prop->s = true; break;
        case MD_SPAN_CODE: prop->code = true; break;
        default: break;
    }
    return 0;
}

int md4c_leave_span(MD_SPANTYPE type, void* detail, void* userdata) {
    (void)detail; // they are needed for signature so don't remove
    auto* prop = static_cast<MDProperties*>(userdata);
    switch (type) {
        case MD_SPAN_STRONG: prop->b = false; break;
        case MD_SPAN_EM: prop->i = false; break;
        case MD_SPAN_U: prop->u = false; break;
        case MD_SPAN_DEL: prop->s = false; break;
        case MD_SPAN_CODE: prop->code = false; break;
        default: break;
    }
    return 0;
}

int md4c_text(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* userdata) {
    (void)type; // they are needed for signature so don't remove
    auto* prop = static_cast<MDProperties*>(userdata);
    std::string str(text, size);
    StyledChunk chunk;
    chunk.text = str;
    chunk.bold = prop->b;
    chunk.italic = prop->i;
    chunk.underline = prop->u;
    chunk.strike = prop->s;
    chunk.is_code = prop->code;
    prop->out->push_back(chunk);
    return 0;
}

} // anonymous namespace

std::vector<StyledChunk> ParserManager::parse_markdown(const std::string& input) {
    std::vector<StyledChunk> result;
    MDProperties ctx;
    ctx.out = &result;

    MD_PARSER parser{};
    parser.abi_version = 0;
    parser.enter_span = md4c_enter_span;
    parser.leave_span = md4c_leave_span;
    parser.text = md4c_text;

    md_parse(input.c_str(), input.size(), &parser, &ctx);
    return result;
}

ftxui::Element ParserManager::build_element(const std::vector<StyledChunk>& chunks) {
    using namespace ftxui;
    std::vector<Element> children;
    for (const auto& chunk : chunks) {
        Element e = text(chunk.text);
        if (chunk.is_code) {
            e = e | dim | bgcolor(Color::Black);
        } else {
            if (chunk.bold) e = e | bold;
            if (chunk.italic) e = e | italic;
            if (chunk.underline) e = e | underlined;
            if (chunk.strike) e = e | strikethrough;
        }
        children.push_back(std::move(e));
    }
    return hbox(std::move(children));
}

ftxui::Element ParserManager::parse_line(const std::string& raw_line, EditorMode mode/* , bool is_active_line */) {
    if (mode == EditorMode::BASIC || mode == EditorMode::CODE) {
        return ftxui::text(raw_line);
    }
    auto chunks = parse_markdown(raw_line);
    return build_element(chunks);
}
