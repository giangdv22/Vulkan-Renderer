#pragma once

namespace Giang
{
	class Application;
	class Device;
	class IndexBuffer
	{
	public:
		IndexBuffer(uint32_t* indices, uint32_t count);
	public:
		~IndexBuffer() = default;

		void Bind() const;

		uint32_t GetCount() const { return m_Count; }

		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);

		vk::Buffer m_Index;
		vk::DeviceMemory m_Memory;
		vk::DescriptorBufferInfo m_BufferInfo;
	private:
		uint32_t m_Count;


		Application* appObj;
		Device* deviceObj;
	};

}