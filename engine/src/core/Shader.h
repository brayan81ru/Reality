#pragma once
#include "DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h"
#include "DiligentCore/Graphics/GraphicsEngine/interface/ShaderResourceBinding.h"
#include "DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#include "Core/Matrix4x4.h"
#include "Core/Texture.h"
#include <string>
#include "rendering/Renderer.h"

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

        [[nodiscard]] Diligent::IPipelineState* GetPipelineState() const { return m_PSO.RawPtr(); }

        [[nodiscard]] Diligent::IShaderResourceBinding* GetResourceBinding() const { return m_SRB.RawPtr(); }

        void AddIncludeDirectory(const std::string& path);

    private:
        static bool ReadShaderFile(const std::string& path, std::string& outSource);

        static std::string ReadShaderFileAndRemoveBOM(const std::string &filePath);

        bool CompileShader(const std::string& source, Diligent::SHADER_TYPE type, Diligent::IShader** ppShader) const;

        Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO;

        Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_SRB;

        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_VSConstants;

        Diligent::RefCntAutoPtr<Diligent::IRenderDevice> m_pDevice;

        Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_pContext;

        std::vector<std::string> m_IncludeDirs;

        Diligent::RefCntAutoPtr<Diligent::ISwapChain> m_pSwapChain;

        static std::string ProcessIncludes(const std::string& source, const std::string& parentPath);

        std::string ResolveIncludes(const std::string &source, const std::string &parentPath);
    };
} // namespace REngine