// Minimal placeholder to satisfy build in this environment.
// Replace with the official single-header nlohmann/json.hpp in a real setup.
#pragma once
#include <string>
#include <vector>
#include <map>
#include <stdexcept>

namespace nlohmann {
class json {
public:
    static json parse(const std::string&) { throw std::runtime_error("nlohmann/json.hpp placeholder: real header required"); }

    bool is_object() const { return false; }
    bool is_array() const { return false; }
    bool is_null() const { return false; }

    auto find(const char*) const { return end(); }
    auto end() const { return iterator(); }

    template<typename T> T get() const { throw std::runtime_error("placeholder"); }

    json& operator[](size_t) { return *this; }
    const json& operator[](size_t) const { return *this; }

    size_t size() const { return 0; }

    struct iterator {
        bool operator==(const iterator&) const { return true; }
        bool operator!=(const iterator&) const { return false; }
        const json& operator*() const { return *parent; }
        const json* operator->() const { return parent; }
        json* parent = nullptr;
    };
};
}

