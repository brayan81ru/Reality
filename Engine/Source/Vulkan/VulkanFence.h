#pragma once

#include "GraphicsDevice.h"
#include "Resource.h"
#include <vulkan/vulkan.h>

namespace Reality {
    class VulkanDevice;

    class VulkanFence : public FenceBase {
    private:
        VkFence m_fence = VK_NULL_HANDLE;
        VulkanDevice* m_device;
        
    public:
        VulkanFence(VulkanDevice* device);
        ~VulkanFence();
        
        // Helper methods
        VkFence GetFence() const { return m_fence; }
        
    protected:
        uint64_t GetCompletedValueImpl() override;
        void SignalImpl(uint64_t value) override;
        void WaitImpl(uint64_t value) override;
    };
}