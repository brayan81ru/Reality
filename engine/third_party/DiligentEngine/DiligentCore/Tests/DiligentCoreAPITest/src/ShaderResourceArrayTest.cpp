/*
 *  Copyright 2019-2025 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence),
 *  contract, or otherwise, unless required by applicable law (such as deliberate
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental,
 *  or consequential damages of any character arising as a result of this License or
 *  out of the use or inability to use the software (including but not limited to damages
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */

#include "GPUTestingEnvironment.hpp"

#include "gtest/gtest.h"

#include "InlineShaders/DrawCommandTestHLSL.h"

#include "GraphicsTypesX.hpp"

using namespace Diligent;
using namespace Diligent::Testing;

namespace Diligent
{
namespace Testing
{
void RenderDrawCommandReference(ISwapChain* pSwapChain, const float* pClearColor);
}
} // namespace Diligent

namespace
{

TEST(ShaderResourceLayout, ResourceArray)
{
    GPUTestingEnvironment* pEnv     = GPUTestingEnvironment::GetInstance();
    IRenderDevice*         pDevice  = pEnv->GetDevice();
    IDeviceContext*        pContext = pEnv->GetDeviceContext();

    GPUTestingEnvironment::ScopedReset EnvironmentAutoReset;

    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    pDevice->GetEngineFactory()->CreateDefaultShaderSourceStreamFactory("shaders", &pShaderSourceFactory);
    ShaderCreateInfo ShaderCI;
    ShaderCI.pShaderSourceStreamFactory     = pShaderSourceFactory;
    ShaderCI.HLSLVersion                    = ShaderVersion{5, 0};
    ShaderCI.WebGPUEmulatedArrayIndexSuffix = "_";

    RefCntAutoPtr<IShader> pVS, pPS;
    {
        ShaderCI.Desc           = {"ShaderResourceArrayTest: VS", SHADER_TYPE_VERTEX, true};
        ShaderCI.FilePath       = "ShaderResourceArrayTest.vsh";
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        pDevice->CreateShader(ShaderCI, &pVS);
        ASSERT_NE(pVS, nullptr);
    }

    {
        ShaderCI.Desc           = {"ShaderResourceArrayTest: PS", SHADER_TYPE_PIXEL, true};
        ShaderCI.FilePath       = pDevice->GetDeviceInfo().IsWebGPUDevice() ? "ShaderResourceArrayTestWGPU.psh" : "ShaderResourceArrayTest.psh";
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        pDevice->CreateShader(ShaderCI, &pPS);
        ASSERT_NE(pPS, nullptr);
    }


    GraphicsPipelineStateCreateInfo PSOCreateInfo;
    PipelineStateDesc&              PSODesc          = PSOCreateInfo.PSODesc;
    GraphicsPipelineDesc&           GraphicsPipeline = PSOCreateInfo.GraphicsPipeline;

    ImmutableSamplerDesc ImtblSampler;
    ImtblSampler.Desc.MinFilter                 = FILTER_TYPE_LINEAR;
    ImtblSampler.Desc.MagFilter                 = FILTER_TYPE_LINEAR;
    ImtblSampler.Desc.MipFilter                 = FILTER_TYPE_LINEAR;
    ImtblSampler.ShaderStages                   = SHADER_TYPE_PIXEL;
    ImtblSampler.SamplerOrTextureName           = "g_tex2DTest";
    PSODesc.ResourceLayout.NumImmutableSamplers = 1;
    PSODesc.ResourceLayout.ImmutableSamplers    = &ImtblSampler;
    // clang-format off
    ShaderResourceVariableDesc Vars[] =
    {
        {SHADER_TYPE_PIXEL, "g_tex2DTest",  SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_PIXEL, "g_tex2DTest2", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, "g_tex2D",      SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}
    };
    // clang-format on
    PSODesc.ResourceLayout.Variables                        = Vars;
    PSODesc.ResourceLayout.NumVariables                     = _countof(Vars);
    GraphicsPipeline.DepthStencilDesc.DepthEnable           = False;
    GraphicsPipeline.RasterizerDesc.CullMode                = CULL_MODE_NONE;
    GraphicsPipeline.BlendDesc.IndependentBlendEnable       = False;
    GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = False;
    GraphicsPipeline.NumRenderTargets                       = 1;

    constexpr TEXTURE_FORMAT RTVFormat = TEX_FORMAT_RGBA8_UNORM;
    constexpr TEXTURE_FORMAT DSVFormat = TEX_FORMAT_D32_FLOAT;
    GraphicsPipeline.RTVFormats[0]     = RTVFormat;
    GraphicsPipeline.DSVFormat         = DSVFormat;
    PSOCreateInfo.pVS                  = pVS;
    PSOCreateInfo.pPS                  = pPS;

    GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    // clang-format off
    LayoutElement Elems[] =
    {
        LayoutElement{ 0, 0, 3, VT_FLOAT32, false, 0 },
        LayoutElement{ 1, 0, 2, VT_FLOAT32, false, sizeof( float ) * 3 }
    };
    // clang-format on
    GraphicsPipeline.InputLayout.LayoutElements = Elems;
    GraphicsPipeline.InputLayout.NumElements    = _countof(Elems);
    RefCntAutoPtr<IPipelineState> pPSO;
    pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &pPSO);
    ASSERT_NE(pPSO, nullptr);

    RefCntAutoPtr<IShaderResourceBinding> pSRB;
    pPSO->CreateShaderResourceBinding(&pSRB);
    ASSERT_NE(pSRB, nullptr);

    // clang-format off
    float Vertices[] =
    {
         0,  0, 0,   0,1,
         0,  1, 0,   0,0,
         1,  0, 0,   1,1,
         1,  1, 0,   1,0
    };
    // clang-format on
    constexpr float fMinXCoord = 0.4f;
    constexpr float fMinYCoord = -0.9f;
    constexpr float fXExtent   = 0.5f;
    constexpr float fYExtent   = 0.5f;
    for (int v = 0; v < 4; ++v)
    {
        Vertices[v * 5 + 0] = Vertices[v * 5 + 0] * fXExtent + fMinXCoord;
        Vertices[v * 5 + 1] = Vertices[v * 5 + 1] * fYExtent + fMinYCoord;
    }

    RefCntAutoPtr<IBuffer> pVertexBuff;
    {
        BufferDesc BuffDesc;
        BuffDesc.Size      = sizeof(Vertices);
        BuffDesc.BindFlags = BIND_VERTEX_BUFFER;
        BuffDesc.Usage     = USAGE_IMMUTABLE;
        BufferData BuffData;
        BuffData.pData    = Vertices;
        BuffData.DataSize = BuffDesc.Size;
        pDevice->CreateBuffer(BuffDesc, &BuffData, &pVertexBuff);
        ASSERT_NE(pVertexBuff, nullptr);
    }

    RefCntAutoPtr<ITextureView> pRTV, pDSV;
    {
        RefCntAutoPtr<ITexture> pRenderTarget = pEnv->CreateTexture("ShaderResourceLayout: offscreen render target", TEX_FORMAT_RGBA8_UNORM, BIND_RENDER_TARGET, 256, 256);
        ASSERT_NE(pRenderTarget, nullptr);
        pRTV = pRenderTarget->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        ASSERT_NE(pRTV, nullptr);

        RefCntAutoPtr<ITexture> pDepth = pEnv->CreateTexture("ShaderResourceLayout: offscreen depth", TEX_FORMAT_D32_FLOAT, BIND_DEPTH_STENCIL, 256, 256);
        ASSERT_NE(pDepth, nullptr);
        pDSV = pDepth->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);
        ASSERT_NE(pDSV, nullptr);
    }

    RefCntAutoPtr<ISampler> pSampler;
    SamplerDesc             SamDesc;
    pDevice->CreateSampler(SamDesc, &pSampler);
    RefCntAutoPtr<ITexture> pTextures[8];
    for (Uint32 t = 0; t < _countof(pTextures); ++t)
    {
        TextureDesc TexDesc;
        TexDesc.Type      = RESOURCE_DIM_TEX_2D;
        TexDesc.Width     = 256;
        TexDesc.Height    = 256;
        TexDesc.MipLevels = 8;
        TexDesc.Usage     = USAGE_IMMUTABLE;
        TexDesc.Format    = TEX_FORMAT_RGBA8_UNORM;
        TexDesc.BindFlags = BIND_SHADER_RESOURCE;
        TexDesc.Name      = "Test Texture";

        std::vector<Uint8>             Data(TexDesc.Width * TexDesc.Height * 4, 128);
        std::vector<TextureSubResData> SubResources(TexDesc.MipLevels);
        for (Uint32 i = 0; i < TexDesc.MipLevels; ++i)
        {
            TextureSubResData& SubResData{SubResources[i]};
            SubResData.pData  = Data.data();
            SubResData.Stride = TexDesc.Width * 4;
        }

        //float ColorOffset[4] = {(float)t * 0.13f, (float)t * 0.21f, (float)t * 0.29f, 0};
        //TestTexturing::GenerateTextureData(pDevice, Data, SubResources, TexDesc, ColorOffset);
        TextureData TexData;
        TexData.pSubResources   = SubResources.data();
        TexData.NumSubresources = (Uint32)SubResources.size();

        pDevice->CreateTexture(TexDesc, &TexData, &pTextures[t]);
        ASSERT_NE(pTextures[t], nullptr);
        pTextures[t]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE)->SetSampler(pSampler);
    }

    // clang-format off
    ResourceMappingEntry ResMpEntries [] =
    {
        ResourceMappingEntry{"g_tex2DTest", pTextures[0]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE), 0},
        ResourceMappingEntry{"g_tex2DTest", pTextures[1]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE), 1},
        ResourceMappingEntry{"g_tex2DTest", pTextures[2]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE), 2},
        ResourceMappingEntry{"g_tex2DTest2", pTextures[0]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE), 0}, // Unused
        ResourceMappingEntry{"g_tex2DTest2", pTextures[5]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE), 1},
        ResourceMappingEntry{"g_tex2DTest2", pTextures[0]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE), 2}, // Unused
        ResourceMappingEntry{"g_tex2DTest2", pTextures[0]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE), 3}, // Unused
        ResourceMappingEntry{"g_tex2D", pTextures[6]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE), 0}
    };
    // clang-format on

    ResourceMappingCreateInfo ResMappingCI;
    ResMappingCI.pEntries   = ResMpEntries;
    ResMappingCI.NumEntries = _countof(ResMpEntries);
    RefCntAutoPtr<IResourceMapping> pResMapping;
    pDevice->CreateResourceMapping(ResMappingCI, &pResMapping);

    //pVS->BindResources(m_pResourceMapping, 0);
    IDeviceObject* ppSRVs[] = {pTextures[3]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE)};
    pPSO->BindStaticResources(SHADER_TYPE_VERTEX | SHADER_TYPE_PIXEL, pResMapping, BIND_SHADER_RESOURCES_KEEP_EXISTING);
    EXPECT_EQ(pSRB->CheckResources(SHADER_TYPE_VERTEX | SHADER_TYPE_PIXEL, pResMapping, BIND_SHADER_RESOURCES_UPDATE_ALL), SHADER_RESOURCE_VARIABLE_TYPE_FLAG_MUT_DYN);
    EXPECT_EQ(pSRB->CheckResources(SHADER_TYPE_VERTEX | SHADER_TYPE_PIXEL, pResMapping, BIND_SHADER_RESOURCES_VERIFY_ALL_RESOLVED), SHADER_RESOURCE_VARIABLE_TYPE_FLAG_MUT_DYN);

    pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "g_tex2DTest2")->SetArray(ppSRVs, 4, 1);

    pPSO->InitializeStaticSRBResources(pSRB);
    pSRB->BindResources(SHADER_TYPE_PIXEL, pResMapping, BIND_SHADER_RESOURCES_KEEP_EXISTING | BIND_SHADER_RESOURCES_UPDATE_MUTABLE | BIND_SHADER_RESOURCES_UPDATE_DYNAMIC);
    ppSRVs[0] = pTextures[4]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_tex2DTest")->SetArray(ppSRVs, 3, 1);

    ITextureView* pRTVs[] = {pRTV};
    pContext->SetRenderTargets(1, pRTVs, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    pContext->SetPipelineState(pPSO);
    ppSRVs[0] = {pTextures[7]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE)};
    pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_tex2D")->SetArray(ppSRVs, 1, 1);

    EXPECT_EQ(pSRB->CheckResources(SHADER_TYPE_PIXEL, pResMapping, BIND_SHADER_RESOURCES_VERIFY_ALL_RESOLVED), SHADER_RESOURCE_VARIABLE_TYPE_FLAG_NONE);

    pResMapping->AddResource("g_tex2DTest", pTextures[1]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE), false);
    EXPECT_EQ(pSRB->CheckResources(SHADER_TYPE_PIXEL, pResMapping, BIND_SHADER_RESOURCES_UPDATE_ALL), SHADER_RESOURCE_VARIABLE_TYPE_FLAG_MUTABLE);
    EXPECT_EQ(pSRB->CheckResources(SHADER_TYPE_PIXEL, pResMapping, BIND_SHADER_RESOURCES_KEEP_EXISTING | BIND_SHADER_RESOURCES_VERIFY_ALL_RESOLVED), SHADER_RESOURCE_VARIABLE_TYPE_FLAG_NONE);

    pResMapping->AddResource("g_tex2D", pTextures[1]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE), false);
    EXPECT_EQ(pSRB->CheckResources(SHADER_TYPE_PIXEL, pResMapping, BIND_SHADER_RESOURCES_UPDATE_ALL), SHADER_RESOURCE_VARIABLE_TYPE_FLAG_MUT_DYN);
    EXPECT_EQ(pSRB->CheckResources(SHADER_TYPE_PIXEL, pResMapping, BIND_SHADER_RESOURCES_KEEP_EXISTING | BIND_SHADER_RESOURCES_VERIFY_ALL_RESOLVED), SHADER_RESOURCE_VARIABLE_TYPE_FLAG_NONE);

    pContext->CommitShaderResources(pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    IBuffer* pBuffs[] = {pVertexBuff};
    pContext->SetVertexBuffers(0, 1, pBuffs, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

    // Draw a quad
    DrawAttribs DrawAttrs(4, DRAW_FLAG_VERIFY_ALL);
    pContext->Draw(DrawAttrs);
}

namespace HLSL
{
const std::string DynamicArrayIndexingPS{R"(
struct PSInput
{
    float4 Pos   : SV_POSITION;
    float3 Color : COLOR;
};

SamplerState      g_Sampler;
Texture2D<float4> g_Textures[4];  // register t0
Texture2D<float4> g_OtherTexture; // register t4

cbuffer cbConstants
{
    float4 g_TextureIndex;
}

float4 main(in PSInput PSIn) : SV_Target
{
    float3 Color = PSIn.Color.rgb;
    Color *= g_Textures[g_TextureIndex.x].Sample(g_Sampler, float2(0.5, 0.5)).rgb;
    Color *= g_OtherTexture.Sample(g_Sampler, float2(0.5, 0.5)).rgb;
    return float4(Color.rgb, 1.0);
}
)"};
}

TEST(ShaderResourceArrayTest, DynamicArrayIndexing)
{
    GPUTestingEnvironment*  pEnv       = GPUTestingEnvironment::GetInstance();
    IRenderDevice*          pDevice    = pEnv->GetDevice();
    IDeviceContext*         pContext   = pEnv->GetDeviceContext();
    const RenderDeviceInfo& DeviceInfo = pDevice->GetDeviceInfo();

    if (!(DeviceInfo.Type == RENDER_DEVICE_TYPE_D3D12 || DeviceInfo.Type == RENDER_DEVICE_TYPE_VULKAN))
    {
        GTEST_SKIP() << "Dynamic array indexing is not supported on this device";
    }

    GPUTestingEnvironment::ScopedReset EnvironmentAutoReset;

    ISwapChain*          pSwapChain = pEnv->GetSwapChain();
    const SwapChainDesc& SCDesc     = pSwapChain->GetDesc();

    float ClearColor[] = {0.875, 0.125, 0.75, 0.25};
    RenderDrawCommandReference(pSwapChain, ClearColor);

    ShaderCreateInfo ShaderCI;
    ShaderCI.HLSLVersion = ShaderVersion{5, 1};

    RefCntAutoPtr<IShader> pVS, pPS;
    {
        ShaderCI.Desc           = {"ShaderResourceArrayTest.DynamicArrayIndexing: VS", SHADER_TYPE_VERTEX, false};
        ShaderCI.EntryPoint     = "main";
        ShaderCI.Source         = HLSL::DrawTest_ProceduralTriangleVS.c_str();
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        pDevice->CreateShader(ShaderCI, &pVS);
        ASSERT_NE(pVS, nullptr);
    }

    {
        ShaderCI.Desc           = {"ShaderResourceArrayTest.DynamicArrayIndexing: PS", SHADER_TYPE_PIXEL, false};
        ShaderCI.Source         = HLSL::DynamicArrayIndexingPS.c_str();
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        pDevice->CreateShader(ShaderCI, &pPS);
        ASSERT_NE(pPS, nullptr);
    }

    GraphicsPipelineStateCreateInfo PSOCreateInfo;
    PipelineStateDesc&              PSODesc          = PSOCreateInfo.PSODesc;
    GraphicsPipelineDesc&           GraphicsPipeline = PSOCreateInfo.GraphicsPipeline;

    PSODesc.Name = "ShaderResourceArrayTest.DynamicArrayIndexing";

    RefCntAutoPtr<IPipelineResourceSignature> pSignature;
    {
        PipelineResourceSignatureDescX SignDesc;
        SignDesc
            // Define g_OtherTexture first to make it remapped to register t0, and g_Textures remapped to register t1
            .AddResource(SHADER_TYPE_PIXEL, "g_OtherTexture", 1u, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE)
            .AddResource(SHADER_TYPE_PIXEL, "g_Textures", 4u, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE)
            .AddImmutableSampler(SHADER_TYPE_PIXEL, "g_Sampler", SamplerDesc{})
            .AddResource(SHADER_TYPE_PIXEL, "cbConstants", 1u, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE);
        pDevice->CreatePipelineResourceSignature(SignDesc, &pSignature);
        ASSERT_NE(pSignature, nullptr);
    }

    IPipelineResourceSignature* ppSignatures[] = {pSignature};
    PSOCreateInfo.ResourceSignaturesCount      = _countof(ppSignatures);
    PSOCreateInfo.ppResourceSignatures         = ppSignatures;

    GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
    GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
    GraphicsPipeline.NumRenderTargets             = 1;
    GraphicsPipeline.RTVFormats[0]                = SCDesc.ColorBufferFormat;
    PSOCreateInfo.pVS                             = pVS;
    PSOCreateInfo.pPS                             = pPS;

    RefCntAutoPtr<IPipelineState> pPSO;
    pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &pPSO);
    ASSERT_NE(pPSO, nullptr);

    RefCntAutoPtr<IShaderResourceBinding> pSRB;
    pSignature->CreateShaderResourceBinding(&pSRB);
    ASSERT_NE(pSRB, nullptr);

    float4                 BufferData{2, 0, 0, 0};
    RefCntAutoPtr<IBuffer> pBuffer = pEnv->CreateBuffer(BufferDesc{"ShaderResourceArrayTest.DynamicArrayIndexing", sizeof(BufferData), BIND_UNIFORM_BUFFER, USAGE_DEFAULT}, BufferData.Data());
    ASSERT_NE(pBuffer, nullptr);
    pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "cbConstants")->Set(pBuffer);

    std::vector<Uint32>     BlackTextData(64 * 64, 0);
    std::vector<Uint32>     WhiteTextData(64 * 64, ~0u);
    RefCntAutoPtr<ITexture> pBlackTex = pEnv->CreateTexture("Black texture", TEX_FORMAT_RGBA8_UNORM, BIND_SHADER_RESOURCE, 64, 64, BlackTextData.data());
    RefCntAutoPtr<ITexture> pWhiteTex = pEnv->CreateTexture("White texture", TEX_FORMAT_RGBA8_UNORM, BIND_SHADER_RESOURCE, 64, 64, WhiteTextData.data());
    ASSERT_NE(pBlackTex, nullptr);
    ASSERT_NE(pWhiteTex, nullptr);
    ITextureView* pWhiteTexSRV = pWhiteTex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    ITextureView* pBlackTexSRV = pBlackTex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    ASSERT_NE(pWhiteTexSRV, nullptr);
    ASSERT_NE(pBlackTexSRV, nullptr);

    IDeviceObject* pTextures[] = {pBlackTexSRV, pBlackTexSRV, pWhiteTexSRV, pBlackTexSRV};
    pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Textures")->SetArray(pTextures, 0, _countof(pTextures));
    pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_OtherTexture")->Set(pWhiteTexSRV);

    ITextureView* ppRTVs[] = {pSwapChain->GetCurrentBackBufferRTV()};
    pContext->SetRenderTargets(1, ppRTVs, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    pContext->ClearRenderTarget(ppRTVs[0], ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    pContext->SetPipelineState(pPSO);
    pContext->CommitShaderResources(pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    DrawAttribs DrawAttrs{6, DRAW_FLAG_VERIFY_ALL};
    pContext->Draw(DrawAttrs);

    pSwapChain->Present();
}

} // namespace
