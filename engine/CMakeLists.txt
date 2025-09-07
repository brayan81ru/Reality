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
        Source/Rendering/DisplayManager.cpp
        Source/Rendering/Renderer.cpp
        Source/Platform/Window.cpp
        Source/Rendering/ShaderManager.cpp
        Source/Rendering/Texture.cpp
        Source/Rendering/Material.cpp
)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/SDL2)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/DiligentEngine)

find_package(SDL2 REQUIRED)

# 4. Diligent Engine libraries.
target_link_libraries(Engine PUBLIC
        SDL2::SDL2
        SDL2::SDL2main
        Diligent-Common
        Diligent-Imgui
        Diligent-ShaderTools
        Diligent-GraphicsTools
        Diligent-TextureLoader
        Diligent-GraphicsEngineD3D12-static
        Diligent-GraphicsEngineD3D11-static
        Diligent-GraphicsEngineOpenGL-static
        Diligent-GraphicsEngineVk-static
)