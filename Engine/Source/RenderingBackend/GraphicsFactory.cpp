#include "GraphicsFactory.h"
#include "GraphicsDevice.h"
#include "D3D12Device.h"
#include <cassert>

namespace Reality {
    IGraphicsDevice* GraphicsFactory::CreateDevice(GraphicsAPI api, void* nativeWindow) {
        switch (api) {
            case GraphicsAPI::DirectX12:
                return new D3D12Device();
            case GraphicsAPI::Vulkan:
                assert(false && "NOT IMPLEMENTED");
                return nullptr;
            default:
                assert(false && "Unknown graphics API");
                return nullptr;
        }
    }
}