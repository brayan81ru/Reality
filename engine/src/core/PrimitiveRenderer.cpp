#include "PrimitiveRenderer.h"
#include "MapHelper.hpp"
#include "Log.h"
#include "GraphicsUtilities.h"

namespace Reality {

    PrimitiveRenderer::PrimitiveRenderer(Renderer *renderer) {
        m_pRenderer = renderer;

        if (!m_pRenderer) {
            return;
        }

        CreatePipelineState();
        CreateVertexBuffer();
        CreateIndexBuffer();
    }



    void PrimitiveRenderer::CreatePipelineState() {
        // Pipeline state object encompasses configuration of all GPU stages

        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        PSOCreateInfo.PSODesc.Name = "Cube PSO";

        // clang-format off
        // This tutorial will render to a single render target
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 1;
        // Set render target format which is the format of the swap chain's color buffer
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = m_pRenderer->GetSwapChain()->GetDesc().ColorBufferFormat;
        // Set depth buffer format which is the format of the swap chain's back buffer
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = m_pRenderer->GetSwapChain()->GetDesc().DepthBufferFormat;
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // Cull back faces
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_BACK;
        // Enable depth testing
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
        // clang-format on

        ShaderCreateInfo ShaderCI;
        // Tell the system that the shader source code is in HLSL.
        // For OpenGL, the engine will convert this into GLSL under the hood.
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        // Pack matrices in row-major order
        ShaderCI.CompileFlags = SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;

        // Presentation engine always expects input in gamma space. Normally, pixel shader output is
        // converted from linear to gamma space by the GPU. However, some platforms (e.g. Android in GLES mode,
        // or Emscripten in WebGL mode) do not support gamma-correction. In this case the application
        // has to do the conversion manually.
        ShaderMacro Macros[] = {{"CONVERT_PS_OUTPUT_TO_GAMMA", !m_ConvertPSOutputToGamma ? "0" : "1"}};
        ShaderCI.Macros      = {Macros, _countof(Macros)};

        // In this tutorial, we will load shaders from file. To be able to do that,
        // we need to create a shader source stream factory
        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pRenderer->GetEngineFactory()->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

        // Create a vertex shader
        RefCntAutoPtr<IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "Cube VS";
            ShaderCI.FilePath        = "Assets/Shaders/cube.vsh";
            m_pRenderer->GetDevice()->CreateShader(ShaderCI, &pVS);
            // Create dynamic uniform buffer that will store our transformation matrix
            // Dynamic buffers can be frequently updated by the CPU
            BufferDesc CBDesc;
            CBDesc.Name           = "VS constants CB";
            CBDesc.Size           = sizeof(float4x4);
            CBDesc.Usage          = USAGE_DYNAMIC;
            CBDesc.BindFlags      = BIND_UNIFORM_BUFFER;
            CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
            m_pRenderer->GetDevice()->CreateBuffer(CBDesc, nullptr, &m_VSConstants);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "Cube PS";
            ShaderCI.FilePath        = "Assets/Shaders/cube.psh";
            m_pRenderer->GetDevice()->CreateShader(ShaderCI, &pPS);
        }

        // clang-format off
        // Define vertex shader input layout
        LayoutElement LayoutElems[] =
        {
            // Attribute 0 - vertex position
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            // Attribute 1 - vertex color
            LayoutElement{1, 0, 4, VT_FLOAT32, False}
        };

        // clang-format on
        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        m_pRenderer->GetDevice()->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSO);


        // Since we did not explicitly specify the type for 'Constants' variable, default
        // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
        // change and are bound directly through the pipeline state object.
        m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_VSConstants);

        // Create a shader resource binding object and bind all static resources in it
        m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);
    }

    void PrimitiveRenderer::CreateVertexBuffer() {
        // Layout of this structure matches the one we defined in the pipeline state
        struct Vertex
        {
            float3 pos;
            float4 color;
        };

        // Cube vertices

        //      (-1,+1,+1)________________(+1,+1,+1)
        //               /|              /|
        //              / |             / |
        //             /  |            /  |
        //            /   |           /   |
        //(-1,-1,+1) /____|__________/(+1,-1,+1)
        //           |    |__________|____|
        //           |   /(-1,+1,-1) |    /(+1,+1,-1)
        //           |  /            |   /
        //           | /             |  /
        //           |/              | /
        //           /_______________|/
        //        (-1,-1,-1)       (+1,-1,-1)
        //

        constexpr Vertex CubeVerts[8] =
            {
            {float3{-1, -1, -1}, float4{1, 0, 0, 1}},
            {float3{-1, +1, -1}, float4{0, 1, 0, 1}},
            {float3{+1, +1, -1}, float4{0, 0, 1, 1}},
            {float3{+1, -1, -1}, float4{1, 1, 1, 1}},

            {float3{-1, -1, +1}, float4{1, 1, 0, 1}},
            {float3{-1, +1, +1}, float4{0, 1, 1, 1}},
            {float3{+1, +1, +1}, float4{1, 0, 1, 1}},
            {float3{+1, -1, +1}, float4{0.2f, 0.2f, 0.2f, 1.f}},
        };

        // Create a vertex buffer that stores cube vertices
        BufferDesc VertBuffDesc;
        VertBuffDesc.Name      = "Cube vertex buffer";
        VertBuffDesc.Usage     = USAGE_IMMUTABLE;
        VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
        VertBuffDesc.Size      = sizeof(CubeVerts);
        BufferData VBData;
        VBData.pData    = CubeVerts;
        VBData.DataSize = sizeof(CubeVerts);
        m_pRenderer->GetDevice()->CreateBuffer(VertBuffDesc, &VBData, &m_CubeVertexBuffer);

    }

    void PrimitiveRenderer::CreateIndexBuffer()
    {
        // clang-format off
        constexpr Uint32 Indices[] =
        {
            2,0,1, 2,3,0,
            4,6,5, 4,7,6,
            0,7,4, 0,3,7,
            1,0,4, 1,4,5,
            1,5,2, 5,6,2,
            3,6,7, 3,2,6
        };
        // clang-format on

        BufferDesc IndBuffDesc;
        IndBuffDesc.Name      = "Cube index buffer";
        IndBuffDesc.Usage     = USAGE_IMMUTABLE;
        IndBuffDesc.BindFlags = BIND_INDEX_BUFFER;
        IndBuffDesc.Size      = sizeof(Indices);
        BufferData IBData;
        IBData.pData    = Indices;
        IBData.DataSize = sizeof(Indices);
        m_pRenderer->GetDevice()->CreateBuffer(IndBuffDesc, &IBData, &m_CubeIndexBuffer);
    }

    float4x4 PrimitiveRenderer::GetAdjustedProjectionMatrix(const float FOV, const float NearPlane, const float FarPlane) const
    {
        const auto& SCDesc = m_pRenderer->GetSwapChain()->GetDesc();

        const float AspectRatio = static_cast<float>(SCDesc.Width) / static_cast<float>(SCDesc.Height);

        float XScale, YScale;

        if (SCDesc.PreTransform == SURFACE_TRANSFORM_ROTATE_90 ||
            SCDesc.PreTransform == SURFACE_TRANSFORM_ROTATE_270 ||
            SCDesc.PreTransform == SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90 ||
            SCDesc.PreTransform == SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270)
        {
            // When the screen is rotated, vertical FOV becomes horizontal FOV
            XScale = 1.f / std::tan(FOV / 2.f);
            // Aspect ratio is inversed
            YScale = XScale * AspectRatio;
        }
        else
        {
            YScale = 1.f / std::tan(FOV / 2.f);
            XScale = YScale / AspectRatio;
        }

        float4x4 Proj;
        Proj._11 = XScale;
        Proj._22 = YScale;
        Proj.SetNearFarClipPlanes(NearPlane, FarPlane, m_pRenderer->GetDevice()->GetDeviceInfo().NDC.MinZ == -1);
        return Proj;
    }

    float4x4 PrimitiveRenderer::GetSurfacePretransformMatrix(const float3& f3CameraViewAxis) const {
        const auto& SCDesc = m_pRenderer->GetSwapChain()->GetDesc();

        switch (SCDesc.PreTransform)
        {
            case SURFACE_TRANSFORM_ROTATE_90:
                // The image content is rotated 90 degrees clockwise.
                return float4x4::RotationArbitrary(f3CameraViewAxis, -PI_F / 2.f);

            case SURFACE_TRANSFORM_ROTATE_180:
                // The image content is rotated 180 degrees clockwise.
                return float4x4::RotationArbitrary(f3CameraViewAxis, -PI_F);

            case SURFACE_TRANSFORM_ROTATE_270:
                // The image content is rotated 270 degrees clockwise.
                return float4x4::RotationArbitrary(f3CameraViewAxis, -PI_F * 3.f / 2.f);

            case SURFACE_TRANSFORM_OPTIMAL:
                UNEXPECTED("SURFACE_TRANSFORM_OPTIMAL is only valid as parameter during swap chain initialization.");
                return float4x4::Identity();

            case SURFACE_TRANSFORM_HORIZONTAL_MIRROR:
            case SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90:
            case SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180:
            case SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270:
                UNEXPECTED("Mirror transforms are not supported");
                return float4x4::Identity();

            default:
                return float4x4::Identity();
        }
    }

    void PrimitiveRenderer::Render()
    {
        // Initial transformations
        float4x4 CubeModelTransform = float4x4::Translation(0.0f,0.0f,0.0f) * float4x4::RotationY(5.0f) * float4x4::RotationX(-PI_F * 0.1f);

        // Camera is at (0, 0, -5) looking along the Z axis
        constexpr float4x4 View = float4x4::Translation(0.f, 0.0f, 5.0f);

        // Get pre transform matrix that rotates the scene according the surface orientation
        float4x4 SrfPreTransform = GetSurfacePretransformMatrix(float3{0, 0, 1});

        // Get projection matrix adjusted to the current screen orientation
        const float4x4 Proj = GetAdjustedProjectionMatrix(PI_F / 4.0f, 0.1f, 100.f);

        // Compute world-view-projection matrix
        m_WorldViewProjMatrix = CubeModelTransform * View * SrfPreTransform * Proj;

        // Bind vertex and index buffers
        constexpr Uint64 offset   = 0;
        IBuffer* pBuffs[] = {m_CubeVertexBuffer};
        m_pRenderer->GetContext()->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
        m_pRenderer->GetContext()->SetIndexBuffer(m_CubeIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        {
            // Map the buffer and write current world-view-projection matrix
            MapHelper<float4x4> CBConstants(m_pRenderer->GetContext(), m_VSConstants, MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = m_WorldViewProjMatrix;
        }

        // Set the pipeline state
        m_pRenderer->GetContext()->SetPipelineState(m_pPSO);
        // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
        // makes sure that resources are transitioned to required states.
        m_pRenderer->GetContext()->CommitShaderResources(m_pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
        DrawAttrs.IndexType  = VT_UINT32; // Index type
        DrawAttrs.NumIndices = 36;
        // Verify the state of vertex and index buffers
        DrawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
        m_pRenderer->GetContext()->DrawIndexed(DrawAttrs);
    }
} // namespace Reality