// Material.cpp
#include "Material.h"
#include "Texture.h"
#include "Renderer.h"
#include "Core/Log.h"

namespace Reality {
    Material::Material(const std::string& name) : m_name(name) {
    }

    void Material::SetAlbedo(const MathF::Vector3f& albedo) {
        m_albedo = albedo;
    }

    void Material::SetMetallic(float metallic) {
        m_metallic = metallic;
    }

    void Material::SetRoughness(float roughness) {
        m_roughness = roughness;
    }

    void Material::SetAO(float ao) {
        m_ao = ao;
    }

    void Material::SetEmissive(const MathF::Vector3f& emissive) {
        m_emissive = emissive;
    }

    void Material::SetOpacity(float opacity) {
        m_opacity = opacity;
        m_transparent = (opacity < 1.0f);
        m_psoDirty = true;
    }

    void Material::SetAlbedoTexture(Texture* texture) {
        m_albedoTexture = texture;
        m_srbDirty = true;
    }

    void Material::SetNormalTexture(Texture* texture) {
        m_normalTexture = texture;
        m_srbDirty = true;
    }

    void Material::SetMetallicTexture(Texture* texture) {
        m_metallicTexture = texture;
        m_srbDirty = true;
    }

    void Material::SetRoughnessTexture(Texture* texture) {
        m_roughnessTexture = texture;
        m_srbDirty = true;
    }

    void Material::SetAOTexture(Texture* texture) {
        m_aoTexture = texture;
        m_srbDirty = true;
    }

    void Material::SetEmissiveTexture(Texture* texture) {
        m_emissiveTexture = texture;
        m_srbDirty = true;
    }

    void Material::SetOpacityTexture(Texture* texture) {
        m_opacityTexture = texture;
        m_srbDirty = true;
    }

    void Material::SetShader(Diligent::IShader* vertexShader, Diligent::IShader* pixelShader) {
        m_vertexShader = vertexShader;
        m_pixelShader = pixelShader;
        m_psoDirty = true;
    }

    void Material::SetTransparent(bool transparent) {
        m_transparent = transparent;
        m_psoDirty = true;
    }

    void Material::SetDoubleSided(bool doubleSided) {
        m_doubleSided = doubleSided;
        m_psoDirty = true;
    }

    void Material::UpdatePipelineState() {
        if (!m_psoDirty) return;

        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = m_name.c_str();

        // Set render target format
        auto& renderer = Renderer::GetInstance();
        const auto& scDesc = renderer.GetSwapChain()->GetDesc();
        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = scDesc.ColorBufferFormat;
        psoCI.GraphicsPipeline.DSVFormat = scDesc.DepthBufferFormat;

        // Set primitive topology
        psoCI.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        // Set rasterizer state
        psoCI.GraphicsPipeline.RasterizerDesc.CullMode = m_doubleSided ?
            Diligent::CULL_MODE_NONE : Diligent::CULL_MODE_BACK;

        // Set depth-stencil state
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = !m_transparent;
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthFunc = Diligent::COMPARISON_FUNC_LESS_EQUAL;

        // Set blend state
        if (m_transparent) {
            psoCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = true;
            psoCI.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA;
            psoCI.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
            psoCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOp = Diligent::BLEND_OPERATION_ADD;
            psoCI.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
            psoCI.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
            psoCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;
        }

        // Define input layout
        Diligent::LayoutElement layoutElems[] = {
            {0, 0, 3, Diligent::VT_FLOAT32, false}, // Position
            {1, 0, 3, Diligent::VT_FLOAT32, false}, // Normal
            {2, 0, 2, Diligent::VT_FLOAT32, false}, // TexCoord
            {3, 0, 3, Diligent::VT_FLOAT32, false}  // Tangent
        };
        psoCI.GraphicsPipeline.InputLayout.LayoutElements = layoutElems;
        psoCI.GraphicsPipeline.InputLayout.NumElements = _countof(layoutElems);

        // Set shaders
        psoCI.pVS = m_vertexShader;
        psoCI.pPS = m_pixelShader;

        // Define variable type
        psoCI.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        // Create pipeline state
        renderer.GetDevice()->CreateGraphicsPipelineState(psoCI, &m_pso);

        m_psoDirty = false;
        m_srbDirty = true;
    }

    void Material::UpdateShaderResourceBinding() {
        if (!m_srbDirty) return;

        if (!m_pso) {
            UpdatePipelineState();
        }

        // Create shader resource binding
        m_pso->CreateShaderResourceBinding(&m_srb, true);

        // Bind textures
        if (m_albedoTexture) {
            auto* var = m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_AlbedoMap");
            var->Set(m_albedoTexture->GetView());
        }

        if (m_normalTexture) {
            auto* var = m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_NormalMap");
            var->Set(m_normalTexture->GetView());
        }

        if (m_metallicTexture) {
            auto* var = m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_MetallicMap");
            var->Set(m_metallicTexture->GetView());
        }

        if (m_roughnessTexture) {
            auto* var = m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_RoughnessMap");
            var->Set(m_roughnessTexture->GetView());
        }

        if (m_aoTexture) {
            auto* var = m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_AOMap");
            var->Set(m_aoTexture->GetView());
        }

        if (m_emissiveTexture) {
            auto* var = m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_EmissiveMap");
            var->Set(m_emissiveTexture->GetView());
        }

        if (m_opacityTexture) {
            auto* var = m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_OpacityMap");
            var->Set(m_opacityTexture->GetView());
        }

        m_srbDirty = false;
    }

    void Material::Bind(Diligent::IDeviceContext* context) {
        UpdatePipelineState();
        UpdateShaderResourceBinding();

        // Set pipeline state
        context->SetPipelineState(m_pso);

        // Commit shader resources
        context->CommitShaderResources(m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }
}
