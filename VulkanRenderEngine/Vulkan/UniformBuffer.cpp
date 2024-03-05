#include "gpch.h"
#include "UniformBuffer.h"
#include "Device.h"
#include "Application.h"
namespace Giang
{
	UniformBuffer::UniformBuffer(uint32_t size, uint32_t binding)
	{
		deviceObj = Application::GetInstance()->DeviceObj;

		vk::BufferCreateInfo createInfo;
		createInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		createInfo.size = size;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = NULL;
		createInfo.sharingMode = vk::SharingMode::eExclusive;
		createInfo.flags = vk::BufferCreateFlags();
		try
		{
			m_Buffer = deviceObj->LogicalDevice.createBuffer(createInfo);
		}
		catch (vk::SystemError err)
		{
			std::cout << "Failed to create Uniform Buffer!\n";
		}

		vk::MemoryRequirements memoryRequirements = deviceObj->LogicalDevice.getBufferMemoryRequirements(m_Buffer);
		vk::MemoryAllocateInfo memAllocInfo = {};
		memAllocInfo.memoryTypeIndex = deviceObj->MemoryTypeFromProperties(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		memAllocInfo.allocationSize = memoryRequirements.size;

		m_Memory = deviceObj->LogicalDevice.allocateMemory(memAllocInfo);
		p_Data = deviceObj->LogicalDevice.mapMemory(m_Memory, 0, memoryRequirements.size);
		m_MappedRange.resize(1);
		m_MappedRange[0].memory = m_Memory;
		m_MappedRange[0].offset = 0;
		m_MappedRange[0].size = size;
		deviceObj->LogicalDevice.invalidateMappedMemoryRanges(m_MappedRange[0]);

		deviceObj->LogicalDevice.bindBufferMemory(m_Buffer, m_Memory, 0);

		m_BufferInfo.buffer = m_Buffer;
		m_BufferInfo.offset = 0;
		m_BufferInfo.range = size;
		m_MemoryRequirement = memoryRequirements;
	}


	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		return CreateRef<UniformBuffer>(size, binding);
	}

	void UniformBuffer::DestroyData()
	{
		if (m_Memory != NULL)
		{
			deviceObj->LogicalDevice.unmapMemory(m_Memory);
			deviceObj->LogicalDevice.destroyBuffer(m_Buffer);
			deviceObj->LogicalDevice.freeMemory(m_Memory);
		}
	}

	void UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		deviceObj->LogicalDevice.invalidateMappedMemoryRanges(m_MappedRange);
		memcpy(p_Data, data, size);
		deviceObj->LogicalDevice.flushMappedMemoryRanges(m_MappedRange);
	}

	UniformBuffer::~UniformBuffer()
	{

	}

}