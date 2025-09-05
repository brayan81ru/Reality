#pragma once
#include "rendering/Renderer.h"
#include "core/Shader.h"
#include "BasicMath.hpp"

using namespace Diligent;

namespace Reality {

    class PrimitiveRenderer {
    public:
        explicit PrimitiveRenderer(Renderer* renderer);

        void CreatePipelineState();

        void CreateVertexBuffer();

        void CreateIndexBuffer();

        float4x4 GetAdjustedProjectionMatrix(float FOV, float NearPlane, float FarPlane) const;

        float4x4 GetSurfacePretransformMatrix(const float3 &f3CameraViewAxis) const;

        void Render();

    private:
        RefCntAutoPtr<IPipelineState>         m_pPSO;
        RefCntAutoPtr<IShaderResourceBinding> m_pSRB;
        RefCntAutoPtr<IBuffer>                m_CubeVertexBuffer;
        RefCntAutoPtr<IBuffer>                m_CubeIndexBuffer;
        Renderer* m_pRenderer;
        Shader * m_pShader;
        bool m_ConvertPSOutputToGamma = true;
        int m_pEngineFactory;
        RefCntAutoPtr<IBuffer> m_VSConstants;
        Diligent::Matrix4x4<float> m_WorldViewProjMatrix;
    };

} // namespace Reality