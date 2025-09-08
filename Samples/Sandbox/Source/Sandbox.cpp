#include <Reality.h>
using namespace Reality;

// Simple vertex shader for triangle
const char* vertexShaderSource = R"(
    struct VSInput {
        float3 position : POSITION;
        float4 color : COLOR;
    };

    struct VSOutput {
        float4 position : SV_POSITION;
        float4 color : COLOR;
    };

    VSOutput main(VSInput input) {
        VSOutput output;
        output.position = float4(input.position, 1.0f);
        output.color = input.color;
        return output;
    }
)";

// Simple pixel shader for triangle
const char* pixelShaderSource = R"(
    struct PSInput {
        float4 position : SV_POSITION;
        float4 color : COLOR;
    };

    float4 main(PSInput input) : SV_TARGET {
        return input.color;
    }
)";

// Triangle vertex data
struct Vertex {
    float position[3];
    float color[4];
};

Vertex triangleVertices[] = {
    {{ 0.0f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},  // Top - Red
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},  // Bottom Left - Green
    {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}   // Bottom Right - Blue
};

uint32_t triangleIndices[] = {
    0, 1, 2  // Triangle indices
};

int main() {
    Window window("TEST WINDOW", 1920, 1080);
    Timer::Init();
    window.Show();
    HWND hwnd = window.GetNativeHandle();

    // Initialize Graphics Factory and Device
    DeviceCreationParams deviceParams;
    deviceParams.api = GraphicsAPI::DirectX12;
    deviceParams.nativeWindow = hwnd;
    deviceParams.width = 1920;
    deviceParams.height = 1080;
    deviceParams.enableDebugLayer = true;
    deviceParams.enableGPUValidation = true;

    // Create the graphics device
    DevicePtr device = GraphicsFactory::CreateDevice(deviceParams);
    if (!device) {
        RLOG_ERROR("Failed to create graphics device!");
        return -1;
    }

    // Create high-level renderer
    HighLevelRenderer renderer(device.get());
    if (!renderer.Initialize(hwnd, 1920, 1080)) {
        RLOG_ERROR("Failed to initialize renderer!");
        return -1;
    }

    // Create vertex buffer
    BufferPtr vertexBuffer = renderer.CreateVertexBuffer(
        triangleVertices,
        sizeof(triangleVertices),
        sizeof(Vertex)
    );

    // Create index buffer
    BufferPtr indexBuffer = renderer.CreateIndexBuffer(
        triangleIndices,
        sizeof(triangleIndices)
    );

    // Create shaders
    ShaderDesc vsDesc;
    vsDesc.type = ShaderType::Vertex;
    vsDesc.source = vertexShaderSource;
    vsDesc.entryPoint = "main";
    vsDesc.target = "vs_5_0";

    ShaderDesc psDesc;
    psDesc.type = ShaderType::Pixel;
    psDesc.source = pixelShaderSource;
    psDesc.entryPoint = "main";
    psDesc.target = "ps_5_0";

    ShaderPtr vertexShader = ShaderPtr(device->CreateShader(vsDesc), ResourceDeleter(device.get()));
    ShaderPtr pixelShader = ShaderPtr(device->CreateShader(psDesc), ResourceDeleter(device.get()));

    if (!vertexShader || !pixelShader) {
        Log::Error("Failed to create shaders!");
        return -1;
    }

    // Create pipeline state
    PipelineStateDesc psoDesc;
    psoDesc.vertexShader = vsDesc;
    psoDesc.pixelShader = psDesc;
    psoDesc.primitiveTopology = PrimitiveTopology::TriangleList;
    psoDesc.numRenderTargets = 1;
    psoDesc.renderTargetFormats[0] = Format::R8G8B8A8_UNORM;
    psoDesc.depthStencilFormat = Format::Unknown;

    // Define input layout
    InputElementDesc inputElements[] = {
        {"POSITION", 0, Format::R32G32B32_FLOAT, 0, 0, InputClassification::Vertex, 0},
        {"COLOR", 0, Format::R32G32B32A32_FLOAT, 0, 12, InputClassification::Vertex, 0}
    };
    psoDesc.inputElements = inputElements;
    psoDesc.numInputElements = 2;

    PipelineStatePtr pipelineState = PipelineStatePtr(
        device->CreatePipelineState(psoDesc),
        ResourceDeleter(device.get())
    );

    if (!pipelineState) {
        RLOG_ERROR("Failed to create pipeline state!");
        return -1;
    }

    // Set up viewport and scissor
    renderer.SetViewport(0, 0, 1920, 1080);
    renderer.SetScissor(0, 0, 1920, 1080);

    RLOG_INFO("Graphics initialization complete!");

    // Main loop
    while (!window.ShouldClose()) {
        window.ProcessMessages();
        Timer::Update();

        // Begin frame
        renderer.BeginFrame();

        // Clear the screen to a dark blue color
        renderer.Clear(Vector4(0.1f, 0.1f, 0.3f, 1.0f));

        // Set pipeline state
        renderer.SetPipelineState(pipelineState.get());

        // Set vertex and index buffers
        renderer.SetVertexBuffer(0, vertexBuffer.get());
        renderer.SetIndexBuffer(indexBuffer.get());

        // Draw the triangle
        renderer.DrawIndexed(3);

        // End frame and present
        renderer.EndFrame();
        renderer.Present();
    }

    RLOG_INFO("Shutting down...");
    return 0;
}