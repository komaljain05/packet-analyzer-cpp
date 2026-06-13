#include "mini_json.hpp"

#include <cctype>
#include <string>
#include <vector>
#include <stdexcept>

namespace DPI {

namespace {

struct Cursor {
    const std::string& s;
    size_t i = 0;

    explicit Cursor(const std::string& str) : s(str) {}

    bool eof() const { return i >= s.size(); }

    char peek() const { return eof() ? '\0' : s[i]; }

    char get() {
        if (eof()) return '\0';
        return s[i++];
    }

    void skipWS() {
        while (!eof()) {
            unsigned char c = static_cast<unsigned char>(s[i]);
            if (std::isspace(c)) i++;
            else break;
        }
    }

    bool consume(char c) {
        skipWS();
        if (peek() == c) {
            i++;
            return true;
        }
        return false;
    }

    void expect(char c, const std::string& errPrefix) {
        if (!consume(c)) {
            throw std::runtime_error(errPrefix + " expected '" + std::string(1, c) + "'");
        }
    }

    static std::string unescapeJsonString(const std::string& raw) {
        // raw contents inside quotes, with backslash escapes.
        std::string out;
        out.reserve(raw.size());

        for (size_t k = 0; k < raw.size(); k++) {
            char c = raw[k];
            if (c != '\\') {
                out.push_back(c);
                continue;
            }

            if (k + 1 >= raw.size()) {
                throw std::runtime_error("Invalid escape at end of string");
            }

            char e = raw[++k];
            switch (e) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case '/': out.push_back('/'); break;
                case 'b': out.push_back('\b'); break;
                case 'f': out.push_back('\f'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                default:
                    // For simplicity, reject unsupported escapes.
                    throw std::runtime_error(std::string("Unsupported escape: ") + e);
            }
        }

        return out;
    }

    std::string parseString(const std::string& errPrefix) {
        skipWS();
        if (get() != '"') {
            throw std::runtime_error(errPrefix + " expected string");
        }

        std::string raw;
        while (!eof()) {
            char c = get();
            if (c == '"') {
                return unescapeJsonString(raw);
            }
            if (c == '\\') {
                if (eof()) throw std::runtime_error(errPrefix + " invalid escape sequence");
                char e = get();
                raw.push_back('\\');
                raw.push_back(e);
            } else {
                raw.push_back(c);
            }
        }
        throw std::runtime_error(errPrefix + " unterminated string");
    }

    // Parse a JSON string value and return it.
    std::string parseStringValue(const std::string& errPrefix) {
        return parseString(errPrefix);
    }

    // Parse array of strings: ["a", "b", ...]
    std::vector<std::string> parseStringArray(const std::string& errPrefix) {
        std::vector<std::string> out;
        expect('[', errPrefix);

        skipWS();
        if (consume(']')) {
            return out;
        }

        while (true) {
            std::string el = parseStringValue(errPrefix + " array element");
            out.push_back(el);

            skipWS();
            if (consume(',')) {
                continue;
            }
            if (consume(']')) {
                break;
            }
            throw std::runtime_error(errPrefix + " expected ',' or ']' in array");
        }

        return out;
    }

    // Parse object and extract the 3 keys.
    void skipObjectValue() {
        // Extremely small skipper for values we don't care about.
        // Supports strings, arrays of strings, numbers, booleans, null, and objects/arrays by nesting.
        skipWS();
        char c = peek();
        if (c == '"') {
            (void)parseString("Value");
            return;
        }
        if (c == '{') {
            // nesting
            size_t depth = 0;
            if (get() == '{') depth++;
            while (depth > 0 && !eof()) {
                char ch = get();
                if (ch == '"') {
                    // rewind one: parse string from previous quote
                    i--; // step back
                    (void)parseString("Value");
                    continue;
                }
                if (ch == '{') depth++;
                else if (ch == '}') depth--;
            }
            if (depth != 0) throw std::runtime_error("Unbalanced object braces");
            return;
        }
        if (c == '[') {
            size_t depth = 0;
            if (get() == '[') depth++;
            while (depth > 0 && !eof()) {
                char ch = get();
                if (ch == '"') {
                    i--; // step back
                    (void)parseString("Value");
                    continue;
                }
                if (ch == '[') depth++;
                else if (ch == ']') depth--;
            }
            if (depth != 0) throw std::runtime_error("Unbalanced array brackets");
            return;
        }

        // primitives: read token
        while (!eof()) {
            char ch = peek();
            if (std::isspace(static_cast<unsigned char>(ch)) || ch == ',' || ch == '}' || ch == ']') {
                break;
            }
            i++;
        }
    }

    bool consumeKeyIf(Cursor& cur, const std::string& expectedKey) {
        std::string key = cur.parseString("Key");
        return key == expectedKey;
    }

};

} // namespace

DPI::MiniBlockedConfig MiniJson::parseBlockedConfig(const std::string& text, const std::string& pathForErrors) {
    Cursor cur(text);
    cur.skipWS();
    if (!cur.consume('{')) {
        throw std::runtime_error("Invalid JSON config in " + pathForErrors + ": root must be an object");
    }

    MiniBlockedConfig cfg;

    cur.skipWS();
    if (cur.consume('}')) {
        return cfg;
    }

    while (true) {
        cur.skipWS();
        // Parse key string
        if (cur.peek() != '"') {
            throw std::runtime_error("Invalid JSON config in " + pathForErrors + ": expected string key");
        }
        std::string key = cur.parseString("Key");

        cur.skipWS();
        cur.expect(':', "Expected ':' after key");

        if (key == "blocked_ips") {
            auto arr = cur.parseStringArray("blocked_ips");
            cfg.blocked_ips = std::move(arr);
        } else if (key == "blocked_apps") {
            auto arr = cur.parseStringArray("blocked_apps");
            cfg.blocked_apps = std::move(arr);
        } else if (key == "blocked_domains") {
            auto arr = cur.parseStringArray("blocked_domains");
            cfg.blocked_domains = std::move(arr);
        } else {
            // Unknown key: skip its value.
            cur.skipObjectValue();
        }

        cur.skipWS();
        if (cur.consume(',')) {
            continue;
        }
        if (cur.consume('}')) {
            break;
        }
        throw std::runtime_error("Invalid JSON config in " + pathForErrors + ": expected ',' or '}'");
    }

    return cfg;
}

} // namespace DPI

