#pragma once

#include "GraphicsTypes.h"

namespace Reality {
    // Forward declarations
    class IGraphicsDevice;

    // Graphics Factory
    class GraphicsFactory {
    public:
        static IGraphicsDevice* CreateDevice(GraphicsAPI api, void* nativeWindow);
    };
}