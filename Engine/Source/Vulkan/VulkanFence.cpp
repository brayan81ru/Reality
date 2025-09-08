#include "VulkanFence.h"
#include "VulkanDevice.h"
#include <cassert>

namespace Reality {
    VulkanFence::VulkanFence(VulkanDevice* device) : m_device(device) {
        VkDevice vkDevice = device->GetDevice();

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateFence(vkDevice, &fenceInfo, nullptr, &m_fence) != VK_SUCCESS) {
            assert(false && "Failed to create fence");
            return;
        }
    }

    VulkanFence::~VulkanFence() {
        VkDevice vkDevice = m_device->GetDevice();
        if (m_fence != VK_NULL_HANDLE) {
            vkDestroyFence(vkDevice, m_fence, nullptr);
        }
    }

    uint64_t VulkanFence::GetCompletedValueImpl() {
        VkDevice vkDevice = m_device->GetDevice();

        VkResult result = vkGetFenceStatus(vkDevice, m_fence);
        if (result == VK_SUCCESS) {
            return 1; // Signaled
        } else if (result == VK_NOT_READY) {
            return 0; // Not signaled
        } else {
            assert(false && "Failed to get fence status");
            return 0;
        }
    }

    void VulkanFence::SignalImpl(uint64_t value) {
        VkDevice vkDevice = m_device->GetDevice();

        if (vkResetFences(vkDevice, 1, &m_fence) != VK_SUCCESS) {
            assert(false && "Failed to reset fence");
            return;
        }
    }

    void VulkanFence::WaitImpl(uint64_t value) {
        VkDevice vkDevice = m_device->GetDevice();

        if (vkWaitForFences(vkDevice, 1, &m_fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
            assert(false && "Failed to wait for fence");
            return;
        }
    }
}
