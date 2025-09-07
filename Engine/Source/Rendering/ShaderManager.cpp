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

    // Add this helper function to read shader files (similar to your Shader class)
    std::string ReadShaderFileAndRemoveBOM(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + filePath);
        }

        // Read the entire file
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(fileSize + 1);
        file.read(buffer.data(), fileSize);
        buffer[fileSize] = '\0';

        // Check for UTF-8 BOM
        if (fileSize >= 3 &&
            static_cast<unsigned char>(buffer[0]) == 0xEF &&
            static_cast<unsigned char>(buffer[1]) == 0xBB &&
            static_cast<unsigned char>(buffer[2]) == 0xBF) {
            return std::string(buffer.data() + 3);
            }

        return std::string(buffer.data());
    }

    void ShaderManager::LoadShader(const std::string& name, const std::string& path, ShaderType type) {
        try {
            // Read the shader file manually (like your Shader class does)
            const std::string shaderSource = ReadShaderFileAndRemoveBOM(path);

            // Create shader info
            Diligent::ShaderCreateInfo ShaderCI;
            ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
            ShaderCI.Desc.UseCombinedTextureSamplers = true;

            // Set the shader source directly instead of using FilePath
            ShaderCI.Source = shaderSource.c_str();

            // Set shader type and entry point
            switch (type) {
                case ShaderType::VERTEX:
                    ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
                    break;
                case ShaderType::PIXEL:
                    ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
                    break;
                    // Add other shader types as needed
            }
            ShaderCI.EntryPoint = "main";

            // Create the shader
            Renderer::GetInstance().GetDevice()->CreateShader(ShaderCI, &m_shaders[name]);

        } catch (const std::exception& e) {
            // Handle error (log it, throw, etc.)
            printf("Error loading shader %s: %s\n", name.c_str(), e.what());
        }
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
        /*
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
        */
        return true;
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