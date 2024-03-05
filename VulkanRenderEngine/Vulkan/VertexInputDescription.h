#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
namespace Giang
{
	class VertexInputDescription
	{
	public:
		VertexInputDescription();
	public:
		~VertexInputDescription();

		void SetVertexBuffer(const Ref<VertexBuffer>& vertexBuffer);
		void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer);

		Ref<IndexBuffer>& GetIndexBuffer() { return m_IndexBuffer; }
		Ref<VertexBuffer>& GetVertexBuffer() { return m_VertexBuffer; }

		vk::VertexInputBindingDescription& GetBinding() { return m_Binding; }
		std::vector<vk::VertexInputAttributeDescription>& GetAttributes(){ return m_Attrib; }

		static Ref<VertexInputDescription> Create();
	private:
		vk::VertexInputBindingDescription m_Binding;
		std::vector<vk::VertexInputAttributeDescription> m_Attrib;

		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
	};
}
