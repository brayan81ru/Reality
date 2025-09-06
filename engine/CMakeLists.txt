# Real library (not INTERFACE since you have .cpp files)

add_library(Engine STATIC
        Source/core/Scene.cpp
        Source/components/TransformComponent.cpp
        Source/core/BaseComponent.h
        Source/core/BaseGameObject.cpp
        Source/core/MathF.h
        Source/core/Camera.cpp
        Source/core/Config.h
        Source/Reality.h
        Source/core/Log.cpp
        Source/core/PrimitiveRenderer.cpp
        Source/core/RealityApplication.cpp
        Source/core/Timer.cpp
        Source/core/Shader.cpp
        Source/rendering/ImguiBackend.cpp
        Source/rendering/DisplayManager.cpp
        Source/rendering/Renderer.cpp
        Source/platform/Window.cpp
)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/DiligentEngine)

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