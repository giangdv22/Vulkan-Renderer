#include "gpch.h"
#include "VertexBuffer.h"
#include "Application.h"

namespace Giang
{
    Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
    {
        return CreateRef<VertexBuffer>(size);
    }

    VertexBuffer::VertexBuffer(uint32_t size)
    {
        appObj = Application::GetInstance();
        deviceObj = appObj->DeviceObj;

        vk::BufferCreateInfo bufferInfo;
        bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
        bufferInfo.size = size;
        bufferInfo.queueFamilyIndexCount = 0;
        bufferInfo.pQueueFamilyIndices = NULL;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;
        bufferInfo.flags = vk::BufferCreateFlags();

        m_Buffer = deviceObj->LogicalDevice.createBuffer(bufferInfo);

        memoryRequirements = deviceObj->LogicalDevice.getBufferMemoryRequirements(m_Buffer);

        vk::MemoryAllocateInfo allocInfo;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = deviceObj->MemoryTypeFromProperties(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        m_Memory = deviceObj->LogicalDevice.allocateMemory(allocInfo);
        m_BufferInfo.range = memoryRequirements.size;
        m_BufferInfo.offset = 0;

    }

    void VertexBuffer::Bind() const
    {
        deviceObj->LogicalDevice.bindBufferMemory(m_Buffer, m_Memory, 0);
    }

    void VertexBuffer::SetData(const void* data, uint32_t size)
    {
        void* memoryAllocation = deviceObj->LogicalDevice.mapMemory(m_Memory, 0, memoryRequirements.size);
        memcpy(memoryAllocation, data, size);

        deviceObj->LogicalDevice.unmapMemory(m_Memory);
    }

}