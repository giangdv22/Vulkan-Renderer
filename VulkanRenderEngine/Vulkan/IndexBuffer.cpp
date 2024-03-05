#include "gpch.h"
#include "IndexBuffer.h"
#include "Application.h"
#include "Device.h"
namespace Giang
{
	IndexBuffer::IndexBuffer(uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		appObj = Application::GetInstance();
		deviceObj = appObj->DeviceObj;

		vk::BufferCreateInfo createInfo = {};
		createInfo.flags = vk::BufferCreateFlags();
		createInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer;
		createInfo.size = count;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = NULL;
		createInfo.sharingMode = vk::SharingMode::eExclusive;

		try
		{
			m_Index = deviceObj->LogicalDevice.createBuffer(createInfo);
		}
		catch (vk::SystemError err)
		{
			std::cout << "Failed to create index buffer!\n";
		}

		vk::MemoryRequirements memoryRequirements = deviceObj->LogicalDevice.getBufferMemoryRequirements(m_Index);

		vk::MemoryAllocateInfo allocInfo = {};

		allocInfo.memoryTypeIndex = deviceObj->MemoryTypeFromProperties(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		allocInfo.allocationSize = memoryRequirements.size;

		m_Memory = deviceObj->LogicalDevice.allocateMemory(allocInfo);
		m_BufferInfo.range = memoryRequirements.size;
		m_BufferInfo.offset = 0;

		void* memoryAllocation = deviceObj->LogicalDevice.mapMemory(m_Memory, 0, memoryRequirements.size);
		memcpy(memoryAllocation, indices, count);
		deviceObj->LogicalDevice.unmapMemory(m_Memory);
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		return CreateRef<IndexBuffer>(indices, count);
	}
	void IndexBuffer::Bind() const
	{
		deviceObj->LogicalDevice.bindBufferMemory(m_Index, m_Memory, 0);
	}
}