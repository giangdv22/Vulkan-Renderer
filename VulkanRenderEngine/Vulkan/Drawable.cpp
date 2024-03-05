#include "gpch.h"
#include "Drawable.h"

#include "Application.h"
#include "Renderer.h"
#include "CommandBufferMgr.h"
#include "Init.h"

namespace Giang
{
	Drawable::Drawable(Renderer* parent)
	{
		memset(&m_VertexBufferData, 0, sizeof(m_VertexBufferData));
		rendererObj = parent;

		Device* deviceObj = Application::GetInstance()->DeviceObj;
		presentCompleteSemaphore = Init::MakeSemaphore(deviceObj->LogicalDevice);
		drawingCompleteSemaphore = Init::MakeSemaphore(deviceObj->LogicalDevice);
	}

	Ref<Drawable> Drawable::Create(Renderer* parent)
	{
		return CreateRef<Drawable>(parent);
	}

	Drawable::~Drawable()
	{
		DestroyVertexBuffer();
	}

	void Drawable::Prepare()
	{
		Device* deviceObj = rendererObj->GetDevice();
		CommandDraws.resize(rendererObj->GetSwapchain()->SwapchainPublicVars.ColorBuffer.size());

		// For each swapchain color image buffer
		// allocate the corresponding command buffer
		for (int i = 0; i < rendererObj->GetSwapchain()->SwapchainPublicVars.ColorBuffer.size(); i++)
		{
			CommandBufferMgr::allocCommandBuffer(&deviceObj->LogicalDevice, *rendererObj->GetCommandPool(), &CommandDraws[i]);

			//Create the renderpass
			RecordCommandBuffer(i, CommandDraws[i]);

		}
	}

	void Drawable::Update(Timestep ts)
	{
		//Device* deviceObj = rendererObj->GetDevice();
		//uint8_t* pData;

		//glm::mat4 mProjection = glm::perspective(glm::radians(45.f), 1.0f, 0.1f, 100.0f);
		//glm::mat4 mView = glm::lookAt(
		//	glm::vec3(0, 0, 5), // Camera is in World Space
		//	glm::vec3(0, 0, 0), // and looks at the origin
		//	glm::vec3(0, 1, 0)); // Head is up
		//glm::mat4 mModel = glm::mat4(1.0f);
		//static float rot = 0;
		////rot += .0005f;
		//mModel = glm::rotate(mModel, rot, glm::vec3(0.0, 1.0, 0.0))
		//	* glm::rotate(mModel, rot, glm::vec3(1.0, 1.0, 1.0));
		//glm::mat4 mMVP = mProjection * mView * mModel;


		//deviceObj->LogicalDevice.invalidateMappedMemoryRanges(m_UniformData.MappedRange);
		//memcpy(m_UniformData.pData, &mMVP, sizeof(mMVP));
		//deviceObj->LogicalDevice.flushMappedMemoryRanges(m_UniformData.MappedRange);
	}

	void Drawable::RecordCommandBuffer(int currentImage, vk::CommandBuffer& cmdDraw)
	{
		CommandBufferMgr::beginCommandBuffer(cmdDraw);
		vk::ClearValue clearValues[2];

		clearValues[0].color.float32[0] = 0.4f;
		clearValues[0].color.float32[1] = 0.38f;
		clearValues[0].color.float32[2] = 0.33f;
		clearValues[0].color.float32[3] = 0.0f;

		// Specify the depth/stencil clear value
		clearValues[1].depthStencil.depth = 1.0f;
		clearValues[1].depthStencil.stencil = 0;

		vk::RenderPassBeginInfo beginInfo = {};
		beginInfo.renderPass = rendererObj->RenderPass;
		beginInfo.framebuffer = rendererObj->Framebuffers[currentImage];
		beginInfo.renderArea.extent.width = rendererObj->width;
		beginInfo.renderArea.extent.height = rendererObj->height;
		beginInfo.clearValueCount = 2;
		beginInfo.pClearValues = clearValues;

		cmdDraw.beginRenderPass(beginInfo, vk::SubpassContents::eInline);

		// Execute the commands as per requirement

		// pipeline bind, geometry, viewport, scissoring
		cmdDraw.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
		cmdDraw.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, PipelineLayout,
			0, descriptorSets.size(), descriptorSets.data(), 0, NULL);

		// Bound the commandbuffer with the graphics pipeline
		const vk::DeviceSize offsets[1] = { 0 };

		cmdDraw.bindVertexBuffers(0, m_VertexInputDescription->GetVertexBuffer()->m_Buffer, offsets);
		cmdDraw.bindIndexBuffer(m_VertexInputDescription->GetIndexBuffer()->m_Index, 0, vk::IndexType::eUint32);

		InitViewports(&cmdDraw);
		InitScissors(&cmdDraw);

		cmdDraw.drawIndexed(m_IndexCount, 1, 0, 0, 0);
		//cmdDraw.draw(36, 1, 0, 0);

		cmdDraw.endRenderPass();
		CommandBufferMgr::endCommandBuffer(cmdDraw);
	}

	void Drawable::Render()
	{
		Device* deviceObj = rendererObj->GetDevice();
		Swapchain* swapchainObj = rendererObj->GetSwapchain();

		uint32_t& currentColorImage = swapchainObj->SwapchainPublicVars.CurrentColorBuffer;

		vk::SwapchainKHR& swapchain = swapchainObj->SwapchainPublicVars.SwapChain;

		deviceObj->LogicalDevice.waitForFences(1, &(swapchainObj->SwapchainPublicVars.fences[currentColorImage]), VK_TRUE, UINT64_MAX);
		deviceObj->LogicalDevice.resetFences(1, &(swapchainObj->SwapchainPublicVars.fences[currentColorImage]));

		currentColorImage = deviceObj->LogicalDevice.acquireNextImageKHR(swapchain, UINT64_MAX, swapchainObj->SwapchainPublicVars.ImageAvailableSamephores[currentColorImage], nullptr).value;
		CommandDraws[currentColorImage].reset();
		RecordCommandBuffer(currentColorImage, CommandDraws[currentColorImage]);
		vk::PipelineStageFlags submitPipelineStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

		vk::SubmitInfo submitInfo = {};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &swapchainObj->SwapchainPublicVars.ImageAvailableSamephores[currentColorImage];
		submitInfo.pWaitDstStageMask = &submitPipelineStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &CommandDraws[currentColorImage];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &swapchainObj->SwapchainPublicVars.RenderFinishedSemaphores[currentColorImage];

		CommandBufferMgr::submitCommandBuffer(deviceObj->Queue, &CommandDraws[currentColorImage], &submitInfo, swapchainObj->SwapchainPublicVars.fences[currentColorImage]);

		vk::PresentInfoKHR present = {};
		present.swapchainCount = 1;
		present.pSwapchains = &swapchain;
		present.pImageIndices = &currentColorImage;
		present.waitSemaphoreCount = 1;
		present.pWaitSemaphores = &swapchainObj->SwapchainPublicVars.RenderFinishedSemaphores[currentColorImage];
		present.pResults = NULL;
		deviceObj->Queue.presentKHR(present);

		//deviceObj->LogicalDevice.destroySemaphore(presentCompleteSemaphore, NULL);

		swapchainObj->SwapchainPublicVars.CurrentColorBuffer = (swapchainObj->SwapchainPublicVars.CurrentColorBuffer + 1) % swapchainObj->SwapchainPublicVars.SwapchainImageCount;
	}



	void Drawable::InitViewports(vk::CommandBuffer* cmd)
	{
		viewport.height = (float)rendererObj->height;
		viewport.width = (float)rendererObj->width;
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;
		viewport.x = 0;
		viewport.y = 0;
		cmd->setViewport(0, viewport);
	}

	void Drawable::InitScissors(vk::CommandBuffer* cmd)
	{
		scissor.extent.width = rendererObj->width;
		scissor.extent.height = rendererObj->height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		cmd->setScissor(0, scissor);
	}

	void Drawable::DestroyCommandBuffer()
	{
		Application* appObj = Application::GetInstance();
		Device* deviceObj = appObj->DeviceObj;

		for (int i = 0; i < CommandDraws.size(); i++)
		{
			deviceObj->LogicalDevice.freeCommandBuffers(rendererObj->cmdPool, CommandDraws[i]);
		}
	}

	void Drawable::destroySynchronizationObjects()
	{
		Application* appObj = Application::GetInstance();
		Device* deviceObj = appObj->DeviceObj;

		deviceObj->LogicalDevice.destroySemaphore(presentCompleteSemaphore);
		deviceObj->LogicalDevice.destroySemaphore(drawingCompleteSemaphore);
	}

	void Drawable::CreateVertexBuffer(const void* vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture)
	{
		Application* appObj = Application::GetInstance();
		Device* deviceObj = appObj->DeviceObj;

		// Create the buffer resource metadata in4
		vk::BufferCreateInfo bufferInfo;
		bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
		bufferInfo.size = dataSize;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = NULL;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;
		bufferInfo.flags = (vk::BufferCreateFlagBits)0;

		// Create the buffer resource
		m_VertexBufferData.Buffer = deviceObj->LogicalDevice.createBuffer(bufferInfo);

		vk::MemoryRequirements memoryRequirements = deviceObj->LogicalDevice.getBufferMemoryRequirements(m_VertexBufferData.Buffer);

		// Create memory allocation metadata in4
		vk::MemoryAllocateInfo allocInfo = {};
		allocInfo.memoryTypeIndex = deviceObj->MemoryTypeFromProperties(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		allocInfo.allocationSize = memoryRequirements.size;

		m_VertexBufferData.Memory = deviceObj->LogicalDevice.allocateMemory(allocInfo);

		m_VertexBufferData.BufferInfo.range = memoryRequirements.size;
		m_VertexBufferData.BufferInfo.offset = 0;

		// Map the physical device memory region to the host

		void* memoryAllocation = deviceObj->LogicalDevice.mapMemory(m_VertexBufferData.Memory, 0, memoryRequirements.size);
		memcpy(memoryAllocation, vertexData, dataSize);

		deviceObj->LogicalDevice.unmapMemory(m_VertexBufferData.Memory);

		deviceObj->LogicalDevice.bindBufferMemory(m_VertexBufferData.Buffer, m_VertexBufferData.Memory, 0);

		/*ViIpBinding.binding = 0;
		ViIpBinding.inputRate = vk::VertexInputRate::eVertex;
		ViIpBinding.stride = dataStride;

		ViIpAttrb[0].binding = 0;
		ViIpAttrb[0].location = 0;
		ViIpAttrb[0].format = vk::Format::eR32G32B32A32Sfloat;
		ViIpAttrb[0].offset = 0;
		ViIpAttrb[1].binding = 0;
		ViIpAttrb[1].location = 1;
		ViIpAttrb[1].format = useTexture ? vk::Format::eR32G32Sfloat : vk::Format::eR32G32B32A32Sfloat;
		ViIpAttrb[1].offset = 16;*/
	}

	void Drawable::CreateVertexIndex(const void* indexData, uint32_t dataSize, uint32_t dataStride)
	{
		Application* appObj = Application::GetInstance();
		Device* deviceObj = appObj->DeviceObj;

		vk::BufferCreateInfo createInfo = {};
		createInfo.flags = vk::BufferCreateFlags();
		createInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer;
		createInfo.size = dataSize;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = NULL;
		createInfo.sharingMode = vk::SharingMode::eExclusive;

		try
		{
			m_VertexIndexData.Index = deviceObj->LogicalDevice.createBuffer(createInfo);
		}
		catch (vk::SystemError err)
		{
			std::cout << "Failed to create index buffer!\n";
		}

		vk::MemoryRequirements memoryRequirements = deviceObj->LogicalDevice.getBufferMemoryRequirements(m_VertexIndexData.Index);

		vk::MemoryAllocateInfo allocInfo = {};

		allocInfo.memoryTypeIndex = deviceObj->MemoryTypeFromProperties(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		allocInfo.allocationSize = memoryRequirements.size;

		m_VertexIndexData.Memory = deviceObj->LogicalDevice.allocateMemory(allocInfo);
		m_VertexIndexData.BufferInfo.range = memoryRequirements.size;
		m_VertexIndexData.BufferInfo.offset = 0;

		void* memoryAllocation = deviceObj->LogicalDevice.mapMemory(m_VertexIndexData.Memory, 0, memoryRequirements.size);
		memcpy(memoryAllocation, indexData, dataSize);
		deviceObj->LogicalDevice.unmapMemory(m_VertexIndexData.Memory);

		deviceObj->LogicalDevice.bindBufferMemory(m_VertexIndexData.Index, m_VertexIndexData.Memory, 0);

	}

	void Drawable::CreateDescriptorSetLayout(bool useTexture)
	{
		vk::DescriptorSetLayoutBinding layoutBindings[2];
		layoutBindings[0].binding = 0;
		layoutBindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		layoutBindings[0].descriptorCount = 1;
		layoutBindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex;
		layoutBindings[0].pImmutableSamplers = NULL;

		if (useTexture)
		{
			layoutBindings[1].binding = 1;
			layoutBindings[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBindings[1].descriptorCount = 1;
			layoutBindings[1].stageFlags = vk::ShaderStageFlagBits::eFragment;
			layoutBindings[1].pImmutableSamplers = NULL;
		}

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayout = {};
		descriptorSetLayout.bindingCount = useTexture ? 2 : 1;
		descriptorSetLayout.pBindings = layoutBindings;

		DescriptorLayouts.resize(1);
		DescriptorLayouts[0] = deviceObj->LogicalDevice.createDescriptorSetLayout(descriptorSetLayout);
	}

	void Drawable::CreatePipelineLayout()
	{
		vk::PipelineLayoutCreateInfo createInfo = {};
		createInfo.pushConstantRangeCount = 0;
		createInfo.pPushConstantRanges = NULL;
		createInfo.setLayoutCount = DescriptorLayouts.size();
		createInfo.pSetLayouts = DescriptorLayouts.data();

		try
		{
			PipelineLayout = deviceObj->LogicalDevice.createPipelineLayout(createInfo);
		}
		catch (vk::SystemError err)
		{
			std::cout << "Failed to create pipeline layout!\n";
		}
	}

	void Drawable::CreateDescriptorPool(bool useTexture)
	{
		vk::DescriptorPoolSize descriptorTypePool[2];

		descriptorTypePool[0].type = vk::DescriptorType::eUniformBuffer;
		descriptorTypePool[0].descriptorCount = 1;

		if (useTexture)
		{
			descriptorTypePool[1].type = vk::DescriptorType::eCombinedImageSampler;
			descriptorTypePool[1].descriptorCount = 1;
		}

		vk::DescriptorPoolCreateInfo createInfo = {};
		createInfo.maxSets = 1;
		createInfo.poolSizeCount = useTexture ? 2 : 1;
		createInfo.pPoolSizes = descriptorTypePool;

		try
		{
			DescriptorPool = deviceObj->LogicalDevice.createDescriptorPool(createInfo);
		}
		catch (vk::SystemError err)
		{
			std::cout << "Failed to create descriptor pool !\n";
		}
	}

	void Drawable::CreateDescriptorResources()
	{
		//CreateUniformBuffer();
	}

	void Drawable::CreateUniformBuffer()
	{
		/*Projection = glm::perspective(glm::radians(45.0f), 1.f, .1f, 100.f);
		View = glm::lookAt(
			glm::vec3(10, 3, 10),
			glm::vec3(0, 0, 0),
			glm::vec3(0, -1, 0)
		);
		Model = glm::mat4(1.0f);

		MVP = Projection * View * Model;

		vk::BufferCreateInfo createInfo;
		createInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		createInfo.size = sizeof(MVP);
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = NULL;
		createInfo.sharingMode = vk::SharingMode::eExclusive;
		createInfo.flags = vk::BufferCreateFlags();

		try
		{
			m_UniformData.Buffer = deviceObj->LogicalDevice.createBuffer(createInfo);
		}
		catch (vk::SystemError err)
		{
			std::cout << "Failed to create uniform buffer!\n";
		}

		vk::MemoryRequirements memoryRequirements = deviceObj->LogicalDevice.getBufferMemoryRequirements(m_UniformData.Buffer);
		vk::MemoryAllocateInfo memAllocInfo = {};
		memAllocInfo.memoryTypeIndex = deviceObj->MemoryTypeFromProperties(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		memAllocInfo.allocationSize = memoryRequirements.size;

		m_UniformData.Memory = deviceObj->LogicalDevice.allocateMemory(memAllocInfo);

		m_UniformData.pData = deviceObj->LogicalDevice.mapMemory(m_UniformData.Memory, 0, memoryRequirements.size);
		memcpy(m_UniformData.pData, &MVP, sizeof(MVP));
		m_UniformData.MappedRange.resize(1);
		m_UniformData.MappedRange[0].memory = m_UniformData.Memory;
		m_UniformData.MappedRange[0].offset = 0;
		m_UniformData.MappedRange[0].size = sizeof(MVP);

		deviceObj->LogicalDevice.invalidateMappedMemoryRanges(m_UniformData.MappedRange[0]);

		deviceObj->LogicalDevice.bindBufferMemory(m_UniformData.Buffer, m_UniformData.Memory, 0);

		m_UniformData.BufferInfo.buffer = m_UniformData.Buffer;
		m_UniformData.BufferInfo.offset = 0;
		m_UniformData.BufferInfo.range = sizeof(MVP);
		m_UniformData.MemoryRequirement = memoryRequirements;*/
	}

	void Drawable::CreateDescriptorSet(bool useTexture)
	{
		Pipeline* pipelineObj = rendererObj->GetPipelineObject();

		vk::DescriptorSetAllocateInfo descAllocInfo;

		descAllocInfo.descriptorPool = DescriptorPool;
		descAllocInfo.descriptorSetCount = 1;
		descAllocInfo.pSetLayouts = DescriptorLayouts.data();

		descriptorSets.resize(1);

		descriptorSets = deviceObj->LogicalDevice.allocateDescriptorSets(descAllocInfo);

		vk::WriteDescriptorSet writes[2];
		memset(&writes, 0, sizeof(writes));
		writes[0].dstSet = descriptorSets[0];
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		writes[0].pBufferInfo = &m_UniformBuffer->GetBufferInfo();
		writes[0].dstArrayElement = 0;
		writes[0].dstBinding = 0;
		writes[0].sType = vk::StructureType::eWriteDescriptorSet;

		if (useTexture)
		{
			writes[1].dstSet = descriptorSets[0];
			writes[1].dstBinding = 1;
			writes[1].descriptorCount = 1;
			writes[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
			writes[1].pImageInfo = NULL;
			writes[1].dstArrayElement = 0;
		}

		deviceObj->LogicalDevice.updateDescriptorSets(useTexture ? 2 : 1, writes, 0, NULL);
	}

	void Drawable::DestroyVertexBuffer()
	{
		Device* deviceObj = rendererObj->GetDevice();

		// deviceObj->LogicalDevice.destroyBuffer(m_VertexBufferData.Buffer);
		// deviceObj->LogicalDevice.freeMemory(m_VertexBufferData.Memory);
	}

	void Drawable::DestroyVertexIndex()
	{
		deviceObj->LogicalDevice.destroyBuffer(m_VertexIndexData.Index);
		deviceObj->LogicalDevice.freeMemory(m_VertexIndexData.Memory);
	}
	void Drawable::DestroyUniformBuffer()
	{
		m_UniformBuffer->DestroyData();
	}
}