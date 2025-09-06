#pragma once
#include <Rendering/Renderer.h>
using namespace Diligent;

namespace Reality {

    class PrimitiveRenderer {
    public:
        explicit PrimitiveRenderer();

        void CreatePipelineState();

        void CreateVertexBuffer();

        void CreateIndexBuffer();

        [[nodiscard]] float4x4 GetAdjustedProjectionMatrix(float FOV, float NearPlane, float FarPlane) const;

        [[nodiscard]] float4x4 GetSurfacePretransformMatrix(const float3 &f3CameraViewAxis) const;

        void Render();

    private:
        RefCntAutoPtr<IPipelineState>         m_pPSO;
        RefCntAutoPtr<IShaderResourceBinding> m_pSRB;
        RefCntAutoPtr<IBuffer>                m_CubeVertexBuffer;
        RefCntAutoPtr<IBuffer>                m_CubeIndexBuffer;
        Renderer* m_pRenderer;
        bool m_ConvertPSOutputToGamma = true;
        int m_pEngineFactory;
        RefCntAutoPtr<IBuffer> m_VSConstants;
        Matrix4x4<float> m_WorldViewProjMatrix;
    };

} // namespace Reality