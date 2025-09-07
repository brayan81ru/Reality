#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "RefCntAutoPtr.hpp"
#include "Shader.h"

namespace Diligent {
    struct IShader;
}

namespace Reality {
    class ShaderManager {
    public:
        static ShaderManager& GetInstance();

        // Shader types
        enum class ShaderType {
            VERTEX,
            PIXEL,
            GEOMETRY,
            COMPUTE,
            RAYGEN,
            MISS,
            CLOSEST_HIT,
            ANY_HIT
        };

        // Shader permutations
        struct ShaderPermutation {
            std::string name;
            std::vector<std::pair<std::string, std::string>> defines;
        };

        // Load shader from file
        Diligent::IShader* LoadShader(
            const std::string& name,
            const std::string& filePath,
            ShaderType type,
            const std::vector<ShaderPermutation>& permutations = {}
        );

        // Get shader by name
        Diligent::IShader* GetShader(const std::string& name);

        // Reload shader (hot-reloading)
        bool ReloadShader(const std::string& name);

        // Create shader permutation
        Diligent::IShader* CreateShaderPermutation(
            const std::string& baseName,
            const std::string& permutationName,
            const std::vector<std::pair<std::string, std::string>>& defines
        );

    private:
        ShaderManager() = default;
        ~ShaderManager() = default;

        // Helper to convert defines to ShaderMacroArray
        static Diligent::ShaderMacroArray ConvertToMacroArray(
            const std::vector<std::pair<std::string, std::string>>& defines
        );

        std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShader>> m_shaders;
        std::unordered_map<std::string, std::string> m_shaderPaths;
        std::unordered_map<std::string, ShaderType> m_shaderTypes;
    };
}
