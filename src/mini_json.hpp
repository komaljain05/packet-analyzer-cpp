#pragma once
// Minimal schema-specific JSON parser fallback (used only if vendored nlohmann/json is a placeholder).
// Supports exactly the config schema used in this project:
//   {
//     "blocked_ips": ["..."],
//     "blocked_apps": ["..."],
//     "blocked_domains": ["..."]
//   }

#include <string>
#include <vector>
#include <stdexcept>

namespace DPI {

struct MiniBlockedConfig {
    std::vector<std::string> blocked_ips;
    std::vector<std::string> blocked_apps;
    std::vector<std::string> blocked_domains;
};

class MiniJson {
public:
    static MiniBlockedConfig parseBlockedConfig(const std::string& text, const std::string& pathForErrors);
};

} // namespace DPI


