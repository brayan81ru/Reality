#pragma once

#include <cstdint>
#include <string>

namespace Reality {
    // Graphics API Types
    enum class GraphicsAPI {
        DirectX12,
        Vulkan,
        // OpenGL optional
    };

    // Resource Types
    enum class ResourceType {
        Buffer,
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube
    };

    // Resource Usage
    enum class ResourceUsage {
        Default,    // GPU read/write
        Immutable,  // GPU read only, initialized at creation
        Dynamic,    // CPU write, GPU read
        Staging    // CPU read/write, GPU copy
    };

    // Shader Types
    enum class ShaderType {
        Vertex,
        Pixel,
        Geometry,
        Hull,
        Domain,
        Compute
    };

    // Resource States
    enum class ResourceState {
        Common,
        VertexBuffer,
        IndexBuffer,
        ConstantBuffer,
        ShaderResource,
        UnorderedAccess,
        RenderTarget,
        DepthWrite,
        DepthRead,
        CopyDest,
        CopySource,
        Present
    };

    // Format Types
    enum class Format {
        Unknown,
        R8_UNORM,
        R8G8_UNORM,
        R8G8B8A8_UNORM,
        R8G8B8A8_UNORM_SRGB,
        B8G8R8A8_UNORM,
        B8G8R8A8_UNORM_SRGB,
        R16_FLOAT,
        R16G16_FLOAT,
        R16G16B16A16_FLOAT,
        R32_FLOAT,
        R32G32_FLOAT,
        R32G32B32_FLOAT,
        R32G32B32A32_FLOAT,
        D32_FLOAT,
        D24_UNORM_S8_UINT,
        D16_UNORM
    };

    // Primitive Topology
    enum class PrimitiveTopology {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip
    };

    // Filter Types
    enum class Filter {
        MinMagMipPoint,
        MinMagPointMipLinear,
        MinPointMagLinearMipPoint,
        MinPointMagMipLinear,
        MinLinearMagMipPoint,
        MinLinearMagPointMipLinear,
        MinMagLinearMipPoint,
        MinMagMipLinear,
        Anisotropic
    };

    // Texture Address Modes
    enum class TextureAddressMode {
        Wrap,
        Mirror,
        Clamp,
        Border,
        MirrorOnce
    };

    // Comparison Function
    enum class ComparisonFunc {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    // Blend Operations
    enum class BlendOp {
        Add,
        Subtract,
        RevSubtract,
        Min,
        Max
    };

    // Blend Factors
    enum class Blend {
        Zero,
        One,
        SrcColor,
        InvSrcColor,
        SrcAlpha,
        InvSrcAlpha,
        DestAlpha,
        InvDestAlpha,
        DestColor,
        InvDestColor,
        SrcAlphaSat,
        BlendFactor,
        InvBlendFactor,
        Src1Color,
        InvSrc1Color,
        Src1Alpha,
        InvSrc1Alpha
    };

    // Stencil Operations
    enum class StencilOp {
        Keep,
        Zero,
        Replace,
        IncrSat,
        DecrSat,
        Invert,
        Incr,
        Decr
    };

    // Cull Modes
    enum class CullMode {
        None,
        Front,
        Back
    };

    // Fill Modes
    enum class FillMode {
        Solid,
        Wireframe
    };

    // Input Classification - MOVED HERE
    enum class InputClassification {
        PerVertex,
        PerInstance
    };

    // Viewport and Scissor
    struct Viewport {
        float x;
        float y;
        float width;
        float height;
        float minDepth;
        float maxDepth;

        Viewport() : x(0), y(0), width(0), height(0), minDepth(0), maxDepth(1) {}
        Viewport(float x, float y, float width, float height, float minDepth = 0, float maxDepth = 1)
            : x(x), y(y), width(width), height(height), minDepth(minDepth), maxDepth(maxDepth) {}
    };

    struct Rect {
        int32_t left;
        int32_t top;
        int32_t right;
        int32_t bottom;

        Rect() : left(0), top(0), right(0), bottom(0) {}
        Rect(int32_t left, int32_t top, int32_t right, int32_t bottom)
            : left(left), top(top), right(right), bottom(bottom) {}
    };

    // Color
    struct Color {
        float r, g, b, a;

        Color() : r(0), g(0), b(0), a(1) {}
        Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
        explicit Color(uint32_t rgba) {
            r = ((rgba >> 24) & 0xFF) / 255.0f;
            g = ((rgba >> 16) & 0xFF) / 255.0f;
            b = ((rgba >> 8) & 0xFF) / 255.0f;
            a = (rgba & 0xFF) / 255.0f;
        }
    };

    // Buffer Descriptors
    struct BufferDesc {
        mutable uint32_t size;
        mutable uint32_t stride;
        mutable ResourceUsage usage;
        mutable uint32_t bindFlags; // Combination of BufferBindFlags
        uint32_t cpuAccessFlags; // Combination of CPUAccessFlags

        BufferDesc() : size(0), stride(0), usage(ResourceUsage::Default), bindFlags(0), cpuAccessFlags(0) {}
    };

    // Texture Descriptors
    struct TextureDesc {
        ResourceType type;
        Format format;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t mipLevels;
        uint32_t arraySize;
        ResourceUsage usage;
        uint32_t bindFlags; // Combination of TextureBindFlags

        TextureDesc()
            : type(ResourceType::Texture2D), format(Format::Unknown), width(0), height(0), depth(1),
              mipLevels(1), arraySize(1), usage(ResourceUsage::Default), bindFlags(0) {}
    };

    // Shader Descriptors
    struct ShaderDesc {
        ShaderType type;
        std::string source;
        std::string entryPoint;
        std::string target; // e.g., "vs_5_0", "ps_5_0", "vs_6_0", etc.

        ShaderDesc() : type(ShaderType::Vertex) {}
    };

    // Sampler Descriptors
    struct SamplerDesc {
        Filter filter;
        TextureAddressMode addressU;
        TextureAddressMode addressV;
        TextureAddressMode addressW;
        float mipLODBias;
        uint32_t maxAnisotropy;
        ComparisonFunc comparisonFunc;
        Color borderColor;
        float minLOD;
        float maxLOD;

        SamplerDesc()
            : filter(Filter::MinMagMipLinear),
              addressU(TextureAddressMode::Wrap),
              addressV(TextureAddressMode::Wrap),
              addressW(TextureAddressMode::Wrap),
              mipLODBias(0.0f),
              maxAnisotropy(16),
              comparisonFunc(ComparisonFunc::Never),
              borderColor(0, 0, 0, 0),
              minLOD(-FLT_MAX),
              maxLOD(FLT_MAX) {}
    };

    // Blend State Descriptors
    struct BlendDesc {
        bool alphaToCoverageEnable;
        bool independentBlendEnable;
        struct RenderTarget {
            bool blendEnable;
            Blend srcBlend;
            Blend destBlend;
            BlendOp blendOp;
            Blend srcBlendAlpha;
            Blend destBlendAlpha;
            BlendOp blendOpAlpha;
            uint8_t renderTargetWriteMask; // 0-7 for each channel
        } renderTarget[8];

        BlendDesc() : alphaToCoverageEnable(false), independentBlendEnable(false) {
            for (int i = 0; i < 8; i++) {
                renderTarget[i] = {};
                renderTarget[i].renderTargetWriteMask = 0x0F; // All channels
            }
        }
    };

    // Rasterizer State Descriptors
    struct RasterizerDesc {
        FillMode fillMode;
        CullMode cullMode;
        bool frontCounterClockwise;
        int depthBias;
        float depthBiasClamp;
        float slopeScaledDepthBias;
        bool depthClipEnable;
        bool multisampleEnable;
        bool antialiasedLineEnable;
        uint32_t forcedSampleCount;
        bool conservativeRaster;

        RasterizerDesc()
            : fillMode(FillMode::Solid),
              cullMode(CullMode::Back),
              frontCounterClockwise(false),
              depthBias(0),
              depthBiasClamp(0.0f),
              slopeScaledDepthBias(0.0f),
              depthClipEnable(true),
              multisampleEnable(false),
              antialiasedLineEnable(false),
              forcedSampleCount(0),
              conservativeRaster(false) {}
    };

    // Depth Stencil State Descriptors
    struct DepthStencilDesc {
        bool depthEnable;
        bool depthWriteMask;
        ComparisonFunc depthFunc;
        bool stencilEnable;
        uint8_t stencilReadMask;
        uint8_t stencilWriteMask;
        struct Face {
            StencilOp stencilFailOp;
            StencilOp stencilDepthFailOp;
            StencilOp stencilPassOp;
            ComparisonFunc stencilFunc;
        } frontFace, backFace;

        DepthStencilDesc()
            : depthEnable(true),
              depthWriteMask(true),
              depthFunc(ComparisonFunc::Less),
              stencilEnable(false),
              stencilReadMask(0xFF),
              stencilWriteMask(0xFF) {
            frontFace = { StencilOp::Keep, StencilOp::Keep, StencilOp::Keep, ComparisonFunc::Always };
            backFace = { StencilOp::Keep, StencilOp::Keep, StencilOp::Keep, ComparisonFunc::Always };
        }
    };

    // Input Element Descriptors - NOW USING InputClassification
    struct InputElementDesc {
        const char* semanticName;
        uint32_t semanticIndex;
        Format format;
        uint32_t inputSlot;
        uint32_t alignedByteOffset;
        InputClassification inputSlotClass;
        uint32_t instanceDataStepRate;

        InputElementDesc()
            : semanticName(nullptr),
              semanticIndex(0),
              format(Format::Unknown),
              inputSlot(0),
              alignedByteOffset(~0u),
              inputSlotClass(InputClassification::PerVertex),
              instanceDataStepRate(0) {}
    };

    // Pipeline State Descriptors
    struct PipelineStateDesc {
        ShaderDesc vertexShader;
        ShaderDesc pixelShader;
        ShaderDesc geometryShader;
        ShaderDesc hullShader;
        ShaderDesc domainShader;
        ShaderDesc computeShader;
        InputElementDesc* inputElements;
        uint32_t numInputElements;
        BlendDesc blendState;
        RasterizerDesc rasterizerState;
        DepthStencilDesc depthStencilState;
        PrimitiveTopology primitiveTopology;
        uint32_t numRenderTargets;
        Format renderTargetFormats[8];
        Format depthStencilFormat;
        uint32_t sampleCount;
        uint32_t sampleQuality;
        uint32_t nodeMask;

        PipelineStateDesc()
            : inputElements(nullptr),
              numInputElements(0),
              primitiveTopology(PrimitiveTopology::TriangleList),
              numRenderTargets(0),
              depthStencilFormat(Format::Unknown),
              sampleCount(1),
              sampleQuality(0),
              nodeMask(0) {
            for (int i = 0; i < 8; i++) {
                renderTargetFormats[i] = Format::Unknown;
            }
        }
    };

    // Bind Flags
    namespace BufferBindFlags {
        enum {
            VertexBuffer = 1 << 0,
            IndexBuffer = 1 << 1,
            ConstantBuffer = 1 << 2,
            ShaderResource = 1 << 3,
            UnorderedAccess = 1 << 4,
            StreamOutput = 1 << 5,
            IndirectArg = 1 << 6
        };
    }

    namespace TextureBindFlags {
        enum {
            ShaderResource = 1 << 0,
            RenderTarget = 1 << 1,
            DepthStencil = 1 << 2,
            UnorderedAccess = 1 << 3
        };
    }

    namespace CPUAccessFlags {
        enum {
            None = 0,
            Read = 1 << 0,
            Write = 1 << 1
        };
    }
}