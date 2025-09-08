#include <Reality.h>

using namespace Reality;
int main() {
    Window window("TEST WINDOW", 1920, 1080);
    Timer::Init();
    window.Show();
    HWND hwnd = window.GetNativeHandle();

    // Create DX12 renderer
    Reality::DX12Renderer renderer(hwnd, 1920, 1080);

    // Main loop
    while (!window.ShouldClose()) {
        window.ProcessMessages();
        Timer::Update();

        // Render frame
        renderer.Render();
    }

    return 0;
}