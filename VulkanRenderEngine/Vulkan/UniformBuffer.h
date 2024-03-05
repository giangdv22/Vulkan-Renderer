#pragma once

namespace Giang
{
	class Device;
	class UniformBuffer
	{
	public:
		UniformBuffer(uint32_t size, uint32_t binding);
		~UniformBuffer();

		void SetData(const void* data, uint32_t size, uint32_t offset = 0);
		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);

		const vk::DescriptorBufferInfo& GetBufferInfo() { return m_BufferInfo; }

		void DestroyData();

	private:
		Device* deviceObj;

		vk::Buffer m_Buffer;
		vk::DeviceMemory m_Memory;
		vk::DescriptorBufferInfo m_BufferInfo;
		vk::MemoryRequirements m_MemoryRequirement;
		std::vector<vk::MappedMemoryRange> m_MappedRange;
		void* p_Data;
	};
}
