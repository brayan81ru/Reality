#pragma once
#include "DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h"
#include "DiligentCore/Graphics/GraphicsEngine/interface/ShaderResourceBinding.h"
#include "DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#include "Core/Matrix4x4.h"
#include "Core/Texture.h"
#include <string>
#include "rendering/Renderer.h"

using namespace Diligent;

namespace Reality {

    class Shader {
    public:
        explicit Shader(const Renderer* renderer);

        ~Shader();

        bool Load(const std::string& vertexPath, const std::string& pixelPath);
        bool LoadFromMemory(const std::string& vertexSrc, const std::string& pixelSrc);

        void Bind() const;

        void SetUniform(const std::string& name, const Matrix4x4& value) const;
        void SetTexture(const std::string& name, const Texture& texture) const;

        [[nodiscard]] IPipelineState* GetPipelineState() const { return m_PSO.RawPtr(); }

        [[nodiscard]] IShaderResourceBinding* GetResourceBinding() const { return m_SRB.RawPtr(); }

        void AddIncludeDirectory(const std::string& path);

    private:
        static bool ReadShaderFile(const std::string& path, std::string& outSource);

        static std::string ReadShaderFileAndRemoveBOM(const std::string &filePath);

        bool CompileShader(const std::string& source, SHADER_TYPE type, IShader** ppShader) const;

        RefCntAutoPtr<IPipelineState> m_PSO;

        RefCntAutoPtr<IShaderResourceBinding> m_SRB;

        RefCntAutoPtr<IBuffer> m_VSConstants;

        RefCntAutoPtr<IRenderDevice> m_pDevice;

        RefCntAutoPtr<IDeviceContext> m_pContext;

        std::vector<std::string> m_IncludeDirs;

        RefCntAutoPtr<ISwapChain> m_pSwapChain;

        static std::string ProcessIncludes(const std::string& source, const std::string& parentPath);

        std::string ResolveIncludes(const std::string &source, const std::string &parentPath);
    };
} // namespace REngine