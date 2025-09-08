#pragma once
#include "GraphicsTypes.h"
#include <memory>
#include <vector>
#include "Resource.h"

namespace Reality {
    // Forward declarations
    class IGraphicsDevice;

    // Enhanced Graphics Factory
    class GraphicsFactory {
    public:
        // Device creation
        static DevicePtr CreateDevice(const DeviceCreationParams& params);

        // Device feature queries
        static DeviceFeatures GetDeviceFeatures(IGraphicsDevice* device);
        static bool IsAPISupported(GraphicsAPI api);
        static std::vector<GraphicsAPI> GetSupportedAPIs();

        // Helper functions
        static GraphicsAPI GetBestAvailableAPI();
        static std::string GetAPIName(GraphicsAPI api);
    };
}
