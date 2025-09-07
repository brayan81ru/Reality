# Real library (not INTERFACE since you have .cpp files)

add_library(Engine STATIC
        Source/Core/Scene.cpp
        Source/Components/TransformComponent.cpp
        Source/Core/BaseComponent.h
        Source/Core/BaseGameObject.cpp
        Source/Core/MathF.h
        Source/Core/Camera.cpp
        Source/Core/Config.h
        Source/Reality.h
        Source/Core/Log.cpp
        Source/Rendering/PrimitiveRenderer.cpp
        Source/Core/RealityApplication.cpp
        Source/Core/Timer.cpp
        Source/Rendering/ImguiBackend.cpp
        Source/Platform/DisplayManager.cpp
        Source/Rendering/Renderer.cpp
        Source/Rendering/ShaderManager.cpp
        Source/Rendering/Texture.cpp
        Source/Rendering/Material.cpp
        Source/Platform/RealityWindow.cpp

        Source/RenderingBackend/GraphicsTypes.h
        Source/RenderingBackend/GraphicsDevice.h
        Source/RenderingBackend/Resource.h
        Source/RenderingBackend/CommandList.cpp
        Source/RenderingBackend/GraphicsFactory.cpp

        Source/RenderingBackend/D3D12Buffer.cpp
        Source/RenderingBackend/D3D12Texture.cpp
        Source/RenderingBackend/D3D12Shader.cpp
        Source/RenderingBackend/D3D12PipelineState.cpp
        Source/RenderingBackend/D3D12CommandList.cpp
        Source/RenderingBackend/D3D12Fence.cpp
        Source/RenderingBackend/D3D12Device.cpp


)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/DiligentEngine)

# 4. Diligent Engine libraries.
target_link_libraries(Engine PUBLIC
        d3d12
        dxgi
        d3dcompiler
        Diligent-Common
        Diligent-Imgui
        Diligent-ShaderTools
        Diligent-GraphicsTools
        Diligent-TextureLoader
        Diligent-GraphicsEngineD3D12-static
        Diligent-GraphicsEngineD3D11-static
        Diligent-GraphicsEngineOpenGL-static
)