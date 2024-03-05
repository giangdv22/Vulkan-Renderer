#pragma once
#include "Descriptor.h"
#include "VertexInputDescription.h"
#include "UniformBuffer.h"
#include "Timestep.h"
namespace Giang
{
	class Renderer;

	class Drawable : public Descriptor
	{
	public:
		Drawable(Renderer* parent = 0);
		static Ref<Drawable> Create(Renderer* parent = 0);
		~Drawable();

		void CreateVertexBuffer(const void* vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture);
		void CreateVertexIndex(const void* indexData, uint32_t dataSize, uint32_t dataStride);
		void CreateDescriptorSetLayout(bool useTexture) override;
		void CreatePipelineLayout() override;
		void CreateDescriptorPool(bool useTexture) override;
		void CreateDescriptorResources() override;
		void CreateUniformBuffer();
		void CreateDescriptorSet(bool useTexture) override;

		void DestroyVertexBuffer();
		void DestroyVertexIndex();
		void DestroyUniformBuffer();

		// Prepares the drawing object before rendering
		// Allocate, create, record command buffer
		void Prepare();
		void Update(Timestep ts);
		// Renders the drawing object
		void Render();

		void InitViewports(vk::CommandBuffer* cmd);
		void InitScissors(vk::CommandBuffer* cmd);

		//Destroy the drawing command buffer object
		void DestroyCommandBuffer();
		void destroySynchronizationObjects();

		void setPipeline(vk::Pipeline* vkpipeline) { pipeline = vkpipeline; }

		void SetVertexInputDescription(Ref<VertexInputDescription> viDescription) { m_VertexInputDescription = viDescription; }
		Ref<VertexInputDescription> GetVertexInputDescription() { return m_VertexInputDescription; }

		void SetUniformBuffer(Ref<UniformBuffer> uniformBuffer) { m_UniformBuffer = uniformBuffer; }

		void SetIndexCount(uint32_t indexCount) { m_IndexCount = indexCount; }

		struct
		{
			vk::Buffer Buffer;
			vk::DeviceMemory Memory;
			vk::DescriptorBufferInfo BufferInfo;
		} m_VertexBufferData;

		struct
		{
			vk::Buffer Index;
			vk::DeviceMemory Memory;
			vk::DescriptorBufferInfo BufferInfo;
		} m_VertexIndexData;

		
		Ref<VertexInputDescription> m_VertexInputDescription;
		Ref<UniformBuffer> m_UniformBuffer;

		Renderer* rendererObj;

	private:
		std::vector<vk::CommandBuffer> CommandDraws;

		// Prepares render pass instance
		void RecordCommandBuffer(int currentImage, vk::CommandBuffer& cmdDraw);
		vk::Pipeline* pipeline;

		vk::Viewport viewport;
		vk::Rect2D scissor;
		vk::Semaphore presentCompleteSemaphore;
		vk::Semaphore drawingCompleteSemaphore;

		uint32_t m_IndexCount = 0;
	};
}