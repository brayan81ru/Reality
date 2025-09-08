#include "GraphicsFactory.h"
#include "GraphicsDevice.h"
#include <cassert>
#include <algorithm>
#include "Backends/D3D12/D3D12Device.h"

namespace Reality {
    DevicePtr GraphicsFactory::CreateDevice(const DeviceCreationParams& params) {
        switch (params.api) {
            case GraphicsAPI::DirectX12: {
                auto device = std::make_unique<D3D12Device>();
                if (device->Initialize(params)) {
                    return DevicePtr(device.release(), ResourceDeleter<IGraphicsDevice>(device.get()));
                }
                break;
            }
            case GraphicsAPI::Vulkan:
                // Not implemented yet
                assert(false && "Vulkan backend not implemented yet");
                break;
            case GraphicsAPI::Metal:
                // Not implemented yet
                assert(false && "Metal backend not implemented yet");
                break;
            default:
                assert(false && "Unknown graphics API");
                break;
        }

        return nullptr;
    }

    DeviceFeatures GraphicsFactory::GetDeviceFeatures(IGraphicsDevice* device) {
        if (!device) {
            return DeviceFeatures();
        }
        return device->GetFeatures();
    }

    bool GraphicsFactory::IsAPISupported(GraphicsAPI api) {
        // For now, we'll assume all APIs are supported
        // In a real implementation, this would check the system capabilities
        return true;
    }

    std::vector<GraphicsAPI> GraphicsFactory::GetSupportedAPIs() {
        std::vector<GraphicsAPI> supported;
        
        // Check which APIs are supported
        if (IsAPISupported(GraphicsAPI::DirectX12)) {
            supported.push_back(GraphicsAPI::DirectX12);
        }
        if (IsAPISupported(GraphicsAPI::Vulkan)) {
            supported.push_back(GraphicsAPI::Vulkan);
        }
        if (IsAPISupported(GraphicsAPI::Metal)) {
            supported.push_back(GraphicsAPI::Metal);
        }
        
        return supported;
    }

    GraphicsAPI GraphicsFactory::GetBestAvailableAPI() {
        auto supported = GetSupportedAPIs();
        
        // Return the first available API
        // In a real implementation, this might prioritize based on performance or features
        if (!supported.empty()) {
            return supported[0];
        }
        
        return GraphicsAPI::Count;
    }

    std::string GraphicsFactory::GetAPIName(GraphicsAPI api) {
        switch (api) {
            case GraphicsAPI::DirectX12:
                return "DirectX 12";
            case GraphicsAPI::Vulkan:
                return "Vulkan";
            case GraphicsAPI::Metal:
                return "Metal";
            default:
                return "Unknown";
        }
    }
}
