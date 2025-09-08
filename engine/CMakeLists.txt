# Real library (not INTERFACE since you have .cpp files)

add_library(Engine STATIC
        Source/Reality.h
        Source/Core/Config.h
        Source/Core/Log.cpp
        Source/Core/Timer.cpp
        Source/Platform/DisplayManager.cpp
        Source/Platform/Window.cpp
        Source/RenderingBackend/RAW/DX12Renderer.cpp
)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb)

# 4. Diligent Engine libraries.
target_link_libraries(Engine PUBLIC
        d3d12
        dxgi
        d3dcompiler
)