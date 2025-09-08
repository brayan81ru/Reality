#pragma once

#include "GraphicsDevice.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace Reality {
    class VulkanDevice : public IGraphicsDevice {
    private:
        VkInstance m_instance = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;
        std::vector<VkFramebuffer> m_swapChainFramebuffers;
        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        VkCommandPool m_commandPool = VK_NULL_HANDLE;

        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
        std::vector<VkFence> m_imagesInFlight;

        uint32_t m_currentFrame = 0;
        uint32_t m_imageIndex = 0;
        uint32_t m_graphicsQueueFamilyIndex = 0;
        uint32_t m_presentQueueFamilyIndex = 0;

        uint32_t m_width = 0;
        uint32_t m_height = 0;
        VkFormat m_swapChainImageFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D m_swapChainExtent = {};

        void CreateInstance();
        void CreateSurface(void* nativeWindow);
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateSwapChain();
        void CreateImageViews();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();


        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);




    public:
        VulkanDevice();
        ~VulkanDevice();


        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        // IGraphicsDevice interface
        void Initialize(void* nativeWindow) override;
        void Shutdown() override;
        void Resize(uint32_t width, uint32_t height) override;

        IBuffer* CreateBuffer(const BufferDesc& desc, const void* initialData = nullptr) override;
        ITexture* CreateTexture(const TextureDesc& desc, const void* initialData = nullptr) override;
        IShader* CreateShader(const ShaderDesc& desc) override;
        IPipelineState* CreatePipelineState(const PipelineStateDesc& desc) override;

        ICommandList* CreateCommandList() override;
        void ExecuteCommandLists(ICommandList* const* commandLists, uint32_t count) override;

        IFence* CreateFence() override;
        void WaitForIdle() override;

        void Present() override;
        uint32_t GetBackBufferIndex() const override;
        ITexture* GetBackBuffer(uint32_t index) override;

        // Helper methods
        VkInstance GetInstance() const { return m_instance; }
        VkDevice GetDevice() const { return m_device; }
        VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
        VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }
        VkQueue GetPresentQueue() const { return m_presentQueue; }
        VkSurfaceKHR GetSurface() const { return m_surface; }
        VkRenderPass GetRenderPass() const { return m_renderPass; }
        VkCommandPool GetCommandPool() const { return m_commandPool; }
        uint32_t GetGraphicsQueueFamilyIndex() const { return m_graphicsQueueFamilyIndex; }
        uint32_t GetPresentQueueFamilyIndex() const { return m_presentQueueFamilyIndex; }
    };
}
