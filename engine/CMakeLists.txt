# Real library (not INTERFACE since you have .cpp files)

add_library(Engine STATIC
        Source/Reality.h
        Source/Core/Config.h
        Source/Core/Log.cpp
        Source/Core/Timer.cpp
        Source/Core/MathF.h

        Source/Platform/DisplayManager.cpp
        Source/Platform/Window.cpp

        Source/RenderingBackend/RAW/DX12Renderer.cpp

        Source/Rendering/GraphicsTypes.h
        Source/Rendering/GraphicsDevice.h
        Source/Rendering/Resource.h
        Source/Rendering/CommandList.cpp
        Source/Rendering/GraphicsFactory.cpp
        Source/Rendering/HighLevelRenderer.cpp

        Source/Rendering/Backends/D3D12/D3D12Device.cpp
        Source/Rendering/Backends/D3D12/D3D12SwapChain.cpp
        Source/Rendering/Backends/D3D12/D3D12Buffer.cpp
        Source/Rendering/Backends/D3D12/D3D12Texture.cpp
        Source/Rendering/Backends/D3D12/D3D12Shader.cpp
        Source/Rendering/Backends/D3D12/D3D12PipelineState.cpp
        Source/Rendering/Backends/D3D12/D3D12CommandList.cpp
        Source/Rendering/Backends/D3D12/D3D12Fence.cpp





)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb)

# 4. Diligent Engine libraries.
target_link_libraries(Engine PUBLIC
        d3d12
        dxgi
        d3dcompiler
)