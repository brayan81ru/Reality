#pragma once

#include "GraphicsDevice.h"
#include "Resource.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace Reality {
    class VulkanShader : public ShaderBase {
    private:
        VkShaderModule m_module = VK_NULL_HANDLE;

    public:
        VulkanShader(const ShaderDesc& desc);
        ~VulkanShader();

        // Helper methods
        VkShaderModule GetModule() const { return m_module; }
    };
}