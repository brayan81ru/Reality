#pragma once

#include "GraphicsDevice.h"
#include "Resource.h"
#include <vulkan/vulkan.h>

namespace Reality {
    class VulkanDevice;

    class VulkanPipelineState : public PipelineStateBase {
    private:
        VkPipeline m_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VulkanDevice* m_device;

        void CreatePipelineLayout();
        void CreateGraphicsPipeline();

    public:
        VulkanPipelineState(VulkanDevice* device, const PipelineStateDesc& desc);
        ~VulkanPipelineState();

        // Helper methods
        VkPipeline GetPipeline() const { return m_pipeline; }
        VkPipelineLayout GetLayout() const { return m_pipelineLayout; }
    };
}