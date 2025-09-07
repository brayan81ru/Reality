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
    SHADER_TYPE ConvertShaderType(ShaderManager::ShaderType type) {
        switch (type) {
            case ShaderManager::ShaderType::VERTEX:
                return SHADER_TYPE_VERTEX;
            case ShaderManager::ShaderType::PIXEL:
                return SHADER_TYPE_PIXEL;
            case ShaderManager::ShaderType::GEOMETRY:
                return SHADER_TYPE_GEOMETRY;
            case ShaderManager::ShaderType::COMPUTE:
                return SHADER_TYPE_COMPUTE;
            case ShaderManager::ShaderType::RAYGEN:
                return SHADER_TYPE_RAY_GEN;
            case ShaderManager::ShaderType::MISS:
                return SHADER_TYPE_RAY_MISS;
            case ShaderManager::ShaderType::CLOSEST_HIT:
                return SHADER_TYPE_RAY_CLOSEST_HIT;
            case ShaderManager::ShaderType::ANY_HIT:
                return SHADER_TYPE_RAY_ANY_HIT;
            default:
                return SHADER_TYPE_UNKNOWN;
        }
    }

    ShaderMacroArray ShaderManager::ConvertToMacroArray(
        const std::vector<std::pair<std::string, std::string>>& defines
    ) {
        if (defines.empty()) {
            // Return empty array
            return {};
        }

        // Convert to ShaderMacro array
        std::vector<ShaderMacro> macros;
        for (const auto& define : defines) {
            macros.emplace_back(define.first.c_str(), define.second.c_str());
        }

        return {macros.data(), static_cast<Uint32>(macros.size())};
    }

    IShader* ShaderManager::LoadShader(
        const std::string& name,
        const std::string& filePath,
        ShaderType type,
        const std::vector<ShaderPermutation>& permutations
    ) {
        if (m_shaders.contains(name)) {
            RLOG_WARNING("Shader '%s' already loaded. Returning existing shader.", name.c_str());
            return m_shaders[name];
        }

        ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.Desc.UseCombinedTextureSamplers = true;
        shaderCI.CompileFlags = SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;
        shaderCI.FilePath = filePath.c_str();

        // Explicitly set Macros to empty array to avoid default argument issue
        shaderCI.Macros = ShaderMacroArray();

        // Set shader type
        shaderCI.Desc.ShaderType = ConvertShaderType(type);

        shaderCI.EntryPoint = "main";
        shaderCI.Desc.Name = name.c_str();

        RefCntAutoPtr<IShader> shader;
        const auto& renderer = Renderer::GetInstance();
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

    IShader* ShaderManager::GetShader(const std::string& name) {
        const auto it = m_shaders.find(name);
        if (it != m_shaders.end()) {
            return it->second;
        }
        RLOG_ERROR("Shader '%s' not found", name.c_str());
        return nullptr;
    }

    bool ShaderManager::ReloadShader(const std::string& name) {
        const auto it = m_shaders.find(name);
        if (it == m_shaders.end()) {
            RLOG_ERROR("Cannot reload shader '%s': not found", name.c_str());
            return false;
        }

        // Unload the current shader
        m_shaders.erase(it);

        // Reload with the same parameters
        const IShader* newShader = LoadShader(name,
                                                   m_shaderPaths[name],
                                                   m_shaderTypes[name]);

        return newShader != nullptr;
    }

    IShader* ShaderManager::CreateShaderPermutation(
        const std::string& baseName,
        const std::string& permutationName,
        const std::vector<std::pair<std::string, std::string>>& defines
    ) {
        const std::string fullName = baseName + "_" + permutationName;

        if (m_shaders.contains(fullName)) {
            RLOG_WARNING("Shader permutation '%s' already exists", fullName.c_str());
            return m_shaders[fullName];
        }

        // Create shader macros
        const ShaderMacroArray macros = ConvertToMacroArray(defines);

        ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.Desc.UseCombinedTextureSamplers = true;
        shaderCI.CompileFlags = SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;
        shaderCI.FilePath = m_shaderPaths[baseName].c_str();
        shaderCI.Macros = macros;

        // Set shader type
        shaderCI.Desc.ShaderType = ConvertShaderType(m_shaderTypes[baseName]);

        shaderCI.EntryPoint = "main";
        shaderCI.Desc.Name = fullName.c_str();

        RefCntAutoPtr<IShader> shader;
        const auto& renderer = Renderer::GetInstance();
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