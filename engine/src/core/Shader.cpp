#include "Shader.h"
#include <filesystem>
#include "DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "DiligentCore/Graphics/GraphicsTools/interface/MapHelper.hpp"
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace Reality {

    Shader::Shader(const Renderer* renderer) {
        m_pDevice = renderer->GetDevice();
        m_pContext = renderer->GetContext();
        m_pSwapChain = renderer->GetSwapChain();
    }

    Shader::~Shader() {
        // Automatic cleanup via RefCntAutoPtr
    }

    bool Shader::Load(const std::string& vertexPath, const std::string& pixelPath) {

        std::string vertexSrc = ReadShaderFileAndRemoveBOM(vertexPath);
        vertexSrc = ResolveIncludes(vertexSrc, vertexPath);

        std::string pixelSrc = ReadShaderFileAndRemoveBOM(pixelPath);
        pixelSrc = ResolveIncludes(pixelSrc, pixelPath);

        return LoadFromMemory(vertexSrc, pixelSrc);
    }

    bool Shader::LoadFromMemory(const std::string& vertexSrc, const std::string& pixelSrc) {
        // Compile shaders
        RefCntAutoPtr<IShader> pVS;
        if (!CompileShader(vertexSrc, SHADER_TYPE_VERTEX, &pVS)) {
            return false;
        }

        RefCntAutoPtr<IShader> pPS;
        if (!CompileShader(pixelSrc, SHADER_TYPE_PIXEL, &pPS)) {
            return false;
        }

        // Create pipeline state using GraphicsPipelineStateCreateInfo
        GraphicsPipelineStateCreateInfo PSOCreateInfo;
        PSOCreateInfo.PSODesc.Name = "Simple triangle PSO";
        PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        // Set shaders
        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        // Setup render targets
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = m_pSwapChain->GetDesc().ColorBufferFormat;
        PSOCreateInfo.GraphicsPipeline.DSVFormat = m_pSwapChain->GetDesc().DepthBufferFormat;

        // Default states
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;

        // Create PSO
        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_PSO);

        if (!m_PSO) return false;

        // Bind uniform buffer
        if (auto* constants = m_PSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")) {
            constants->Set(m_VSConstants);
        }

        // Create shader resource binding
        m_PSO->CreateShaderResourceBinding(&m_SRB, true);

        return true;
    }

    void Shader::Bind() const {
        m_pContext->SetPipelineState(m_PSO);
        m_pContext->CommitShaderResources(m_SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    bool Shader::CompileShader(const std::string& source, SHADER_TYPE type, IShader** ppShader) const {
        ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.Desc.ShaderType = type;
        shaderCI.Desc.Name = type == SHADER_TYPE_VERTEX ? "Vertex shader" : "Pixel shader";
        shaderCI.Source = source.c_str();
        shaderCI.EntryPoint = "main";
        m_pDevice->CreateShader(shaderCI, ppShader);
        return *ppShader != nullptr;
    }

    void Shader::AddIncludeDirectory(const std::string &path) {
        m_IncludeDirs.push_back(path);
    }

    std::string Shader::ProcessIncludes(const std::string &source, const std::string &parentPath) {
        //
        return "";
    }

    std::string Shader::ResolveIncludes(const std::string &source, const std::string &parentPath) {
        std::istringstream stream(source);
        std::ostringstream output;
        std::string line;

        while (std::getline(stream, line)) {
            if (line.find("#include") != std::string::npos) {
                size_t start = line.find('"');
                size_t end = line.rfind('"');
                if (start == std::string::npos || end == std::string::npos) {
                    throw std::runtime_error("Invalid include syntax");
                }

                std::string includeFile = line.substr(start + 1, end - start - 1);
                std::string includePath;

                // Check relative to current file first
                auto parentDir = std::filesystem::path(parentPath).parent_path();
                auto fullPath = parentDir / includeFile;

                if (!std::filesystem::exists(fullPath)) {
                    // Check include directories
                    bool found = false;
                    for (const auto& dir : m_IncludeDirs) {
                        fullPath = std::filesystem::path(dir) / includeFile;
                        if (std::filesystem::exists(fullPath)) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        throw std::runtime_error("Could not find include file: " + includeFile);
                    }
                }

                std::string includedContent = ReadShaderFileAndRemoveBOM(fullPath.string());
                includedContent = ResolveIncludes(includedContent, fullPath.string());
                output << includedContent << "\n";
            } else {
                output << line << "\n";
            }
        }

        return output.str();
    }

    bool Shader::ReadShaderFile(const std::string& path, std::string& outSource) {
        const std::ifstream file(path);
        if (!file.is_open()) return false;

        std::stringstream buffer;
        buffer << file.rdbuf();
        outSource = buffer.str();
        return true;
    }

    std::string Shader::ReadShaderFileAndRemoveBOM(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + filePath);
        }

        // Read the entire file into a buffer
        file.seekg(0, std::ios::end);
        const size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(fileSize + 1);
        file.read(buffer.data(), fileSize);
        buffer[fileSize] = '\0';

        // Check for UTF-8 BOM (ï»¿)
        if (fileSize >= 3 &&
            static_cast<unsigned char>(buffer[0]) == 0xEF &&
            static_cast<unsigned char>(buffer[1]) == 0xBB &&
            static_cast<unsigned char>(buffer[2]) == 0xBF) {
            // Skip BOM by returning from position 3
            return std::string(buffer.data() + 3);
            }

        return std::string(buffer.data());
    }

} // namespace Reality