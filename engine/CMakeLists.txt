# Real library (not INTERFACE since you have .cpp files)

add_library(Engine STATIC
        src/core/Scene.cpp
        src/core/TransformComponent.cpp
        src/core/BaseComponent.h
        src/core/BaseGameObject.cpp
        src/core/MathF.h
        src/core/Camera.cpp
        src/core/Config.h
        src/Reality.h
        src/core/Log.cpp
        src/core/PrimitiveRenderer.cpp
        src/core/RealityApplication.cpp
        src/core/Timer.cpp
        src/core/Shader.cpp
        src/rendering/ImguiBackend.cpp
        src/rendering/DisplayManager.cpp
        src/rendering/Renderer.cpp
        src/platform/Window.cpp
)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

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