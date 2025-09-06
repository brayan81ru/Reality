#pragma once
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace Reality {
    class Config {
        std::map<std::string, std::map<std::string, std::string>> data;

        // Helper function to trim whitespace from both ends of a string
        static std::string Trim(const std::string& str) {
            const size_t first = str.find_first_not_of(" \t\n\r");
            if (std::string::npos == first) return "";
            const size_t last = str.find_last_not_of(" \t\n\r");
            return str.substr(first, (last - first + 1));
        }

        // Helper function to convert string to lowercase
        static std::string ToLower(const std::string& str) {
            std::string result = str;
            std::ranges::transform(result, result.begin(),[](unsigned char c) { return std::tolower(c); });
            return result;
        }

    public:

        static Config& GetInstance() {
            static Config instance;
            return instance;
        }

        // Delete copy constructor and assignment operator
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;

        // Set a configuration value
        void Set(const std::string& section, const std::string& key, const std::string& value) {
            data[section][key] = value;
        }

        // Get a configuration value as string
        [[nodiscard]] std::string Get(const std::string& section, const std::string& key) const {
            if (const auto secIt = data.find(section); secIt != data.end()) {
                if (const auto keyIt = secIt->second.find(key); keyIt != secIt->second.end()) {
                    return keyIt->second;
                }
            }
            return "";
        }

        // Get a configuration value as integer
        [[nodiscard]] int GetInt(const std::string& section, const std::string& key) const {
            if (const std::string value = Get(section, key); !value.empty()) {
                try {
                    return std::stoi(value);
                } catch (...) {
                    // Return 0 on conversion failure
                }
            }
            return 0;
        }

        // Get a configuration value as boolean
        [[nodiscard]] bool GetBool(const std::string& section, const std::string& key) const {
            const std::string value = ToLower(Get(section, key));
            if (value == "true" || value == "1" || value == "yes" || value == "on") {
                return true;
            }
            if (value == "false" || value == "0" || value == "no" || value == "off") {
                return false;
            }
            return false;
        }

        // Get a configuration value as float
        [[nodiscard]] float GetFloat(const std::string& section, const std::string& key) const {
            if (const std::string value = Get(section, key); !value.empty()) {
                try {
                    return std::stof(value);
                } catch (...) {
                    // Return 0.0f on conversion failure
                }
            }
            return 0.0f;
        }

        // Load configuration from INI file
        bool Load(const std::string& filename) {
            std::ifstream file(filename);
            if (!file.is_open()) {
                return false;
            }

            std::string line;
            std::string currentSection;

            while (std::getline(file, line)) {
                // Trim whitespace
                line = Trim(line);

                // Skip empty lines and comments
                if (line.empty() || line[0] == ';' || line[0] == '#') {
                    continue;
                }

                // Check for section header
                if (line[0] == '[' && line[line.length() - 1] == ']') {
                    currentSection = line.substr(1, line.length() - 2);
                    continue;
                }

                // Parse key-value pair
                if (const size_t equalPos = line.find('='); equalPos != std::string::npos) {
                    std::string key = Trim(line.substr(0, equalPos));
                    const std::string value = Trim(line.substr(equalPos + 1));

                    if (!currentSection.empty() && !key.empty()) {
                        data[currentSection][key] = value;
                    }
                }
            }

            file.close();
            return true;
        }

        // Save configuration to INI file
        [[nodiscard]] bool Save(const std::string& filename) const {
            std::ofstream file(filename);
            if (!file.is_open()) {
                return false;
            }

            for (const auto&[fst, snd] : data) {
                // Write section header
                file << "[" << fst << "]\n";

                // Write all key-value pairs in this section
                for (const auto&[fst, snd] : snd) {
                    file << fst << "=" << snd << "\n";
                }

                // Add empty line between sections
                file << "\n";
            }

            file.close();
            return true;
        }

        // Clear all configuration data
        void Clear() {
            data.clear();
        }

        // Check if a section exists
        [[nodiscard]] bool HasSection(const std::string& section) const {
            return data.contains(section);
        }

        // Check if a key exists in a section
        [[nodiscard]] bool HasKey(const std::string& section, const std::string& key) const {
            if (const auto secIt = data.find(section); secIt != data.end()) {
                return secIt->second.contains(key);
            }
            return false;
        }
    private:
        Config() = default;

        ~Config() {
            Clear();
        };
    };
}