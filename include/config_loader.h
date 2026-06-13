#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <string>
#include <vector>
#include <stdexcept>

namespace DPI {

class ConfigLoader {
public:
    struct BlockedRules {
        std::vector<std::string> blocked_ips;
        std::vector<std::string> blocked_apps;
        std::vector<std::string> blocked_domains;
    };

    class ConfigError : public std::runtime_error {
    public:
        explicit ConfigError(const std::string& message)
            : std::runtime_error(message) {}
    };

    // Loads JSON config from `path`.
    // - If file does not exist: throws ConfigError (caller decides whether to treat as warning).
    // - If malformed/invalid: throws ConfigError.
    // - Missing keys: treated as empty arrays.
    static BlockedRules loadBlockedRules(const std::string& path);

private:
    static std::string readFileOrThrow(const std::string& path);
    static bool isValidIPv4(const std::string& ip);
    static std::string trim(const std::string& s);
    static void requireArrayOfStrings(/*nlohmann::json*/ const std::string& fieldName,
                                       /*nlohmann::json*/ const std::string& pathHint,
                                       /*nlohmann::json*/ const std::string& valueText,
                                       std::vector<std::string>& out);
};

} // namespace DPI

#endif // CONFIG_LOADER_H

