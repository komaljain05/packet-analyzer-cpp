#include "config_loader.h"

#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>

// Prefer single-header nlohmann/json (vendored as include/nlohmann/json.hpp).
// If the vendored header is just a placeholder (common in this environment),
// we fall back to a tiny schema-specific parser.
#include <nlohmann/json.hpp>
#include "mini_json.hpp"

namespace DPI {

namespace {
static bool isNlohmannPlaceholder() {
    // The placeholder header throws on parse().
    // We detect by attempting parse on empty string.
    try {
        (void)nlohmann::json::parse("{}");
        return false;
    } catch (...) {
        return true;
    }
}
}


std::string ConfigLoader::trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        start++;
    }
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        end--;
    }
    if (start == 0 && end == s.size()) return s;
    return s.substr(start, end - start);
}

std::string ConfigLoader::readFileOrThrow(const std::string& path) {
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in.is_open()) {
        throw ConfigError("Config file not found or cannot be opened: " + path);
    }

    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

bool ConfigLoader::isValidIPv4(const std::string& ip) {
    // Strict IPv4 dotted-decimal validation.
    // Accepts only 0-255 octets.
    int parts = 0;
    int value = 0;
    bool hasDigit = false;

    for (size_t i = 0; i <= ip.size(); i++) {
        char c = (i < ip.size()) ? ip[i] : '.'; // sentinel delimiter

        if (c == '.') {
            if (!hasDigit) return false;
            if (value < 0 || value > 255) return false;
            parts++;
            if (parts > 4) return false;
            value = 0;
            hasDigit = false;
            continue;
        }

        if (c >= '0' && c <= '9') {
            hasDigit = true;
            value = value * 10 + (c - '0');
            continue;
        }

        return false;
    }

    return parts == 4;
}

ConfigLoader::BlockedRules ConfigLoader::loadBlockedRules(const std::string& path) {
    const std::string text = readFileOrThrow(path);

    BlockedRules rules;

    if (isNlohmannPlaceholder()) {
        // Tiny fallback parser for the exact schema.
        // If this file is present, the real nlohmann/json header is unavailable.
        try {
            auto mini = MiniJson::parseBlockedConfig(text, path);
            rules.blocked_ips = std::move(mini.blocked_ips);
            rules.blocked_apps = std::move(mini.blocked_apps);
            rules.blocked_domains = std::move(mini.blocked_domains);
        } catch (const std::exception& e) {
            throw ConfigError(std::string("Invalid JSON config in ") + path + ": " + e.what());
        }
    } else {
        json root;
        try {
            root = json::parse(text);
        } catch (const std::exception& e) {
            throw ConfigError(std::string("Invalid JSON in ") + path + ": " + e.what());
        }

        if (!root.is_object()) {
            throw ConfigError("Invalid config format in " + path + ": root must be a JSON object");
        }

        auto loadStringArray = [&](const char* key, std::vector<std::string>& target) {
            auto it = root.find(key);
            if (it == root.end() || it->is_null()) {
                return; // missing keys => empty
            }
            if (!it->is_array()) {
                throw ConfigError(std::string("Invalid type for key '") + key + "' in " + path + ": expected array of strings");
            }

            for (size_t i = 0; i < it->size(); i++) {
                const auto& el = (*it)[i];
                if (!el.is_string()) {
                    throw ConfigError(std::string("Invalid element type for '") + key + "' in " + path + " at index " + std::to_string(i) + ": expected string");
                }
                std::string s = trim(el.get<std::string>());
                if (s.empty()) {
                    throw ConfigError(std::string("Empty string is not allowed for '") + key + "' in " + path + " at index " + std::to_string(i));
                }
                target.push_back(std::move(s));
            }
        };

        loadStringArray("blocked_ips", rules.blocked_ips);
        loadStringArray("blocked_apps", rules.blocked_apps);
        loadStringArray("blocked_domains", rules.blocked_domains);
    }

    // Validate IPs strictly.
    for (size_t i = 0; i < rules.blocked_ips.size(); i++) {
        if (!isValidIPv4(rules.blocked_ips[i])) {
            throw ConfigError(std::string("Invalid IPv4 address in blocked_ips in ") + path + " at index " + std::to_string(i) + ": " + rules.blocked_ips[i]);
        }
    }

    return rules;
}


} // namespace DPI

