// ShaderManager.cpp
#include "ShaderManager.h"
#include "Renderer.h"
#include "Core/Log.h"

namespace Reality {
    ShaderManager& ShaderManager::GetInstance() {
        static ShaderManager instance;
        return instance;
    }

    // Helper function to convert our ShaderType to Diligent's SHADER_TYPE
    Diligent::SHADER_TYPE ConvertShaderType(ShaderManager::ShaderType type) {
        switch (type) {
            case ShaderManager::ShaderType::VERTEX:
                return Diligent::SHADER_TYPE_VERTEX;
            case ShaderManager::ShaderType::PIXEL:
                return Diligent::SHADER_TYPE_PIXEL;
            case ShaderManager::ShaderType::GEOMETRY:
                return Diligent::SHADER_TYPE_GEOMETRY;
            case ShaderManager::ShaderType::COMPUTE:
                return Diligent::SHADER_TYPE_COMPUTE;
            case ShaderManager::ShaderType::RAYGEN:
                return Diligent::SHADER_TYPE_RAY_GEN;
            case ShaderManager::ShaderType::MISS:
                return Diligent::SHADER_TYPE_RAY_MISS;
            case ShaderManager::ShaderType::CLOSEST_HIT:
                return Diligent::SHADER_TYPE_RAY_CLOSEST_HIT;
            case ShaderManager::ShaderType::ANY_HIT:
                return Diligent::SHADER_TYPE_RAY_ANY_HIT;
            default:
                return Diligent::SHADER_TYPE_UNKNOWN;
        }
    }

    Diligent::ShaderMacroArray ShaderManager::ConvertToMacroArray(
        const std::vector<std::pair<std::string, std::string>>& defines
    ) {
        if (defines.empty()) {
            // Return empty array
            return Diligent::ShaderMacroArray();
        }

        // Convert to ShaderMacro array
        std::vector<Diligent::ShaderMacro> macros;
        for (const auto& define : defines) {
            macros.push_back({define.first.c_str(), define.second.c_str()});
        }

        return Diligent::ShaderMacroArray(macros.data(), macros.size());
    }

    Diligent::IShader* ShaderManager::LoadShader(
        const std::string& name,
        const std::string& filePath,
        ShaderType type,
        const std::vector<ShaderPermutation>& permutations
    ) {
        if (m_shaders.find(name) != m_shaders.end()) {
            RLOG_WARNING("Shader '%s' already loaded. Returning existing shader.", name.c_str());
            return m_shaders[name];
        }

        Diligent::ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.Desc.UseCombinedTextureSamplers = true;
        shaderCI.CompileFlags = Diligent::SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;
        shaderCI.FilePath = filePath.c_str();

        // Explicitly set Macros to empty array to avoid default argument issue
        shaderCI.Macros = Diligent::ShaderMacroArray();

        // Set shader type
        shaderCI.Desc.ShaderType = ConvertShaderType(type);

        shaderCI.EntryPoint = "main";
        shaderCI.Desc.Name = name.c_str();

        Diligent::RefCntAutoPtr<Diligent::IShader> shader;
        auto& renderer = Renderer::GetInstance();
        renderer.GetDevice()->CreateShader(shaderCI, &shader);

        if (!shader) {
            RLOG_ERROR("Failed to load shader '%s' from '%s'", name.c_str(), filePath.c_str());
            return nullptr;
        }

        m_shaders[name] = shader;
        m_shaderPaths[name] = filePath;
        m_shaderTypes[name] = type;

        RLOG_INFO("Loaded shader '%s' from '%s'", name.c_str(), filePath.c_str());

        // Create permutations if specified
        for (const auto& permutation : permutations) {
            CreateShaderPermutation(name, permutation.name, permutation.defines);
        }

        return shader;
    }

    Diligent::IShader* ShaderManager::GetShader(const std::string& name) {
        auto it = m_shaders.find(name);
        if (it != m_shaders.end()) {
            return it->second;
        }
        RLOG_ERROR("Shader '%s' not found", name.c_str());
        return nullptr;
    }

    bool ShaderManager::ReloadShader(const std::string& name) {
        auto it = m_shaders.find(name);
        if (it == m_shaders.end()) {
            RLOG_ERROR("Cannot reload shader '%s': not found", name.c_str());
            return false;
        }

        // Unload the current shader
        m_shaders.erase(it);

        // Reload with the same parameters
        Diligent::IShader* newShader = LoadShader(name,
                                                   m_shaderPaths[name],
                                                   m_shaderTypes[name]);

        return newShader != nullptr;
    }

    Diligent::IShader* ShaderManager::CreateShaderPermutation(
        const std::string& baseName,
        const std::string& permutationName,
        const std::vector<std::pair<std::string, std::string>>& defines
    ) {
        std::string fullName = baseName + "_" + permutationName;

        if (m_shaders.find(fullName) != m_shaders.end()) {
            RLOG_WARNING("Shader permutation '%s' already exists", fullName.c_str());
            return m_shaders[fullName];
        }

        // Create shader macros
        Diligent::ShaderMacroArray macros = ConvertToMacroArray(defines);

        Diligent::ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.Desc.UseCombinedTextureSamplers = true;
        shaderCI.CompileFlags = Diligent::SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;
        shaderCI.FilePath = m_shaderPaths[baseName].c_str();
        shaderCI.Macros = macros;

        // Set shader type
        shaderCI.Desc.ShaderType = ConvertShaderType(m_shaderTypes[baseName]);

        shaderCI.EntryPoint = "main";
        shaderCI.Desc.Name = fullName.c_str();

        Diligent::RefCntAutoPtr<Diligent::IShader> shader;
        auto& renderer = Renderer::GetInstance();
        renderer.GetDevice()->CreateShader(shaderCI, &shader);

        if (!shader) {
            RLOG_ERROR("Failed to create shader permutation '%s'", fullName.c_str());
            return nullptr;
        }

        m_shaders[fullName] = shader;
        RLOG_INFO("Created shader permutation '%s'", fullName.c_str());

        return shader;
    }
}