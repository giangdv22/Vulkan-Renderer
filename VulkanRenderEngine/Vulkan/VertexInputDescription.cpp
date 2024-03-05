#include "gpch.h"
#include "VertexInputDescription.h"
#include "IndexBuffer.h"
namespace Giang
{
    namespace Utils
    {
        static vk::Format ShaderDataTypeToVulkanFormat(ShaderDataType type)
        {
            switch (type)
            {
            case ShaderDataType::Float:    return vk::Format::eR32Sfloat;
            case ShaderDataType::Float2:   return vk::Format::eR32G32Sfloat;
            case ShaderDataType::Float3:   return vk::Format::eR32G32B32Sfloat;
            case ShaderDataType::Float4:   return vk::Format::eR32G32B32A32Sfloat;
            case ShaderDataType::Int:      return vk::Format::eR32Sint;
            case ShaderDataType::Int2:     return vk::Format::eR32G32Sint;
            case ShaderDataType::Int3:     return vk::Format::eR32G32B32Sint;
            case ShaderDataType::Int4:     return vk::Format::eR32G32B32A32Sint;
            }
        }
    }

    VertexInputDescription::VertexInputDescription()
    {

    }

    VertexInputDescription::~VertexInputDescription()
    {

    }

    Ref<VertexInputDescription> VertexInputDescription::Create()
    {
        return CreateRef<VertexInputDescription>();
    }

    void VertexInputDescription::SetVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
    {
        m_VertexBuffer = vertexBuffer;
        vertexBuffer->Bind();

        const auto& layout = vertexBuffer->GetLayout();
        m_Binding.binding = 0;
        m_Binding.inputRate = vk::VertexInputRate::eVertex;
        m_Binding.stride = layout.GetStride();

        m_Attrib.resize(layout.GetElements().size());
        uint32_t i = 0;
        for (const auto& element : layout)
        {
            m_Attrib[i].binding = 0;
            m_Attrib[i].location = i;
            m_Attrib[i].format = Utils::ShaderDataTypeToVulkanFormat(element.Type);
            m_Attrib[i].offset = element.Offset;

            i++;
        }
    }
    void VertexInputDescription::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
    {
        indexBuffer->Bind();

        m_IndexBuffer = indexBuffer;
    }
}