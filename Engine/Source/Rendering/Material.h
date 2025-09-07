// Material.h
#pragma once
#include <string>
#include <memory>
#include "RefCntAutoPtr.hpp"
#include "GraphicsTypes.h"
#include "ShaderManager.h"
#include <Core/MathF.h>

namespace Reality {
    class Texture;

    class Material {
    public:
        Material(const std::string& name);
        ~Material() = default;

        // PBR properties
        void SetAlbedo(const MathF::Vector3f& albedo);
        void SetMetallic(float metallic);
        void SetRoughness(float roughness);
        void SetAO(float ao);
        void SetEmissive(const MathF::Vector3f& emissive);
        void SetOpacity(float opacity);

        // Textures
        void SetAlbedoTexture(Texture* texture);
        void SetNormalTexture(Texture* texture);
        void SetMetallicTexture(Texture* texture);
        void SetRoughnessTexture(Texture* texture);
        void SetAOTexture(Texture* texture);
        void SetEmissiveTexture(Texture* texture);
        void SetOpacityTexture(Texture* texture);

        // Shader
        void SetShader(Diligent::IShader* vertexShader, Diligent::IShader* pixelShader);

        // Material flags
        void SetTransparent(bool transparent);
        void SetDoubleSided(bool doubleSided);

        // Binding
        void Bind(Diligent::IDeviceContext* context);

        // Getters
        const std::string& GetName() const { return m_name; }
        bool IsTransparent() const { return m_transparent; }
        bool IsDoubleSided() const { return m_doubleSided; }

    private:
        void UpdatePipelineState();
        void UpdateShaderResourceBinding();

        std::string m_name;

        // PBR properties
        MathF::Vector3f m_albedo = MathF::Vector3f(1.0f, 1.0f, 1.0f);
        float m_metallic = 0.0f;
        float m_roughness = 0.5f;
        float m_ao = 1.0f;
        MathF::Vector3f m_emissive = MathF::Vector3f(0.0f, 0.0f, 0.0f);
        float m_opacity = 1.0f;

        // Textures
        Texture* m_albedoTexture = nullptr;
        Texture* m_normalTexture = nullptr;
        Texture* m_metallicTexture = nullptr;
        Texture* m_roughnessTexture = nullptr;
        Texture* m_aoTexture = nullptr;
        Texture* m_emissiveTexture = nullptr;
        Texture* m_opacityTexture = nullptr;

        // Shaders
        Diligent::RefCntAutoPtr<Diligent::IShader> m_vertexShader;
        Diligent::RefCntAutoPtr<Diligent::IShader> m_pixelShader;

        // Pipeline state
        Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
        Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

        // Material flags
        bool m_transparent = false;
        bool m_doubleSided = false;

        // Dirty flags
        bool m_psoDirty = true;
        bool m_srbDirty = true;
    };
}
