#pragma once

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace Marshal::Inference {

inline std::string read_text_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) return {};
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

inline void json_escape(std::ostream& o, const std::string& s) {
    o << '"';
    for (char c : s) {
        if (c == '"' || c == '\\') o << '\\';
        o << c;
    }
    o << '"';
}

inline std::string json_get_string(const std::string& text, const std::string& key) {
    const std::string needle = "\"" + key + "\": \"";
    const size_t p = text.find(needle);
    if (p == std::string::npos) return {};
    size_t i = p + needle.size();
    std::string out;
    while (i < text.size() && text[i] != '"') {
        if (text[i] == '\\' && i + 1 < text.size()) {
            out += text[++i];
        } else {
            out += text[i];
        }
        ++i;
    }
    return out;
}

inline double json_get_double(const std::string& text, const std::string& key) {
    const std::string needle = "\"" + key + "\": ";
    const size_t p = text.find(needle);
    if (p == std::string::npos) return 0;
    return std::strtod(text.c_str() + p + needle.size(), nullptr);
}

inline bool json_get_bool(const std::string& text, const std::string& key) {
    const std::string needle = "\"" + key + "\": ";
    const size_t p = text.find(needle);
    if (p == std::string::npos) return false;
    const size_t v = p + needle.size();
    return text.compare(v, 4, "true") == 0;
}

inline std::vector<std::string> json_get_string_array(const std::string& text,
                                                      const std::string& key) {
    std::vector<std::string> out;
    const std::string needle = "\"" + key + "\": [";
    const size_t p = text.find(needle);
    if (p == std::string::npos) return out;
    size_t i = p + needle.size();
    while (i < text.size()) {
        while (i < text.size() && text[i] != '"' && text[i] != ']') ++i;
        if (i >= text.size() || text[i] == ']') break;
        ++i;
        std::string s;
        while (i < text.size() && text[i] != '"') {
            if (text[i] == '\\' && i + 1 < text.size()) s += text[++i];
            else s += text[i];
            ++i;
        }
        if (!s.empty()) out.push_back(s);
        ++i;
    }
    return out;
}

inline std::vector<std::string> split_plus_tokens(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == '+') {
            if (!cur.empty()) {
                size_t a = cur.find_first_not_of(" \t");
                size_t b = cur.find_last_not_of(" \t");
                if (a != std::string::npos) out.push_back(cur.substr(a, b - a + 1));
                cur.clear();
            }
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) {
        size_t a = cur.find_first_not_of(" \t");
        size_t b = cur.find_last_not_of(" \t");
        if (a != std::string::npos) out.push_back(cur.substr(a, b - a + 1));
    }
    return out;
}

inline std::string normalize_lemma_alias(const std::string& id) {
    if (id == "compact_sinc2_falsification") return "cylinder_direct_sum_falsified";
    return id;
}

}  // namespace Marshal::Inference
