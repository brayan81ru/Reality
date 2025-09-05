# Real library (not INTERFACE since you have .cpp files)

add_library(Engine STATIC
        src/core/Config.h
        src/Reality.h
        src/core/Log.cpp
        src/core/PrimitiveRenderer.cpp
        src/core/RealityApplication.cpp
        src/core/Timer.cpp
        src/core/Vector3.h
        src/core/Vector2.h
        src/core/Color.h
        src/core/Matrix4x4.h
        src/core/Quaternion.h
        src/core/TextureManager.cpp
        src/core/Texture.cpp
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