#include <Reality.h>

using namespace Reality;

int main() {

    RealityWindow window("Reallity Engine Sandbox", 1920, 1080);

    window.Show();

    Reality::IGraphicsDevice* device = GraphicsFactory::CreateDevice(Reality::GraphicsAPI::DirectX12, window.GetNativeHandle());

    device->Initialize(window.GetNativeHandle());

    while (!window.ShouldClose()) {
        window.ProcessMessages();
    }

    delete device;

    return 0;
}
