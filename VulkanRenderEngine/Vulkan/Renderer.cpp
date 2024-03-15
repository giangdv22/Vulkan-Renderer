#include "gpch.h"
#include "Renderer.h"

#include "CommandBufferMgr.h"
#include "Init.h"
#include "MeshData.h"
#include "EditorCamera.h"
#include "UniformBuffer.h"
#include "Texture.h"
#include <gli/gli/gli.hpp>
namespace Giang
{

	struct QuadVertex
	{
		glm::vec4 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
	};

	struct Renderer2DData
	{
		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexInputDescription> QuadViDescription;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> QuadShader;
		Ref<Drawable> QuadDrawable;
		Ref<Texture> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture>, MaxTextureSlots> TextureSlots;
		std::array<vk::DescriptorImageInfo, 32> DescriptorImageInfos;
		uint32_t TextureSlotIndex = 1; // 0 = White Texture;

		glm::vec4 QuadVertexPositions[4] {
		{ -0.5f, -0.5f, 0.0f, 1.0f },
		{  -0.5f, 0.5f, 0.0f, 1.0f },
		{  0.5f,  0.5f, 0.0f, 1.0f },
		{ 0.5f,  -0.5f, 0.0f, 1.0f }
		};

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;
		Ref<EditorCamera> CameraEditor;

		Ref<Texture> TestTexture;
	};

	Renderer2DData* s_Data = new Renderer2DData();

	void Renderer::Init2DData()
	{
		s_Data->QuadViDescription = VertexInputDescription::Create();
		s_Data->QuadVertexBuffer = VertexBuffer::Create(s_Data->MaxVertices * sizeof(QuadVertex));
		s_Data->QuadVertexBuffer->SetLayout(
			{
				{ShaderDataType::Float4 },
				{ShaderDataType::Float4 },
				{ShaderDataType::Float2 },
				{ShaderDataType::Float}
			});
		s_Data->QuadViDescription->SetVertexBuffer(s_Data->QuadVertexBuffer);

		s_Data->QuadVertexBufferBase = new QuadVertex[s_Data->MaxIndices];

		uint32_t* quadIndices = new uint32_t[s_Data->MaxIndices];
		uint32_t offset = 0;

		for (uint32_t i = 0; i < s_Data->MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data->MaxIndices);
		s_Data->QuadViDescription->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		s_Data->WhiteTexture = Texture::Create();
		uint32_t whiteTextureData = 0xfffffffff;
		s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		s_Data->DescriptorImageInfos[0] = s_Data->WhiteTexture->GetTextureData().DescriptorInfo;

		int samplers[s_Data->MaxTextureSlots];
		for (uint32_t i = 0; i < s_Data->MaxTextureSlots; i++)
		{
			samplers[i] = i;
		}

		s_Data->TextureSlots[0] = s_Data->WhiteTexture;

		s_Data->CameraEditor = CreateRef<EditorCamera>(45.0f, 1.778f, 0.1f, 100.0f);

		s_Data->QuadShader = Shader::Create("shaders/QuadVertex.spv", "shaders/QuadFragment.spv");
		s_Data->QuadDrawable = Drawable::Create(Application::GetInstance()->RendererObj);
		s_Data->QuadDrawable->SetVertexInputDescription(s_Data->QuadViDescription);
		s_Data->CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
		s_Data->QuadDrawable->SetUniformBuffer(s_Data->CameraUniformBuffer);
		s_Data->QuadDrawable->SetTextureSlots(s_Data->MaxTextureSlots);
		s_Data->QuadDrawable->SetDescriptorImageInfo(s_Data->DescriptorImageInfos);
	}

	Renderer::Renderer(Application* app, Device* deviceObject)
	{

		memset(&Depth, 0, sizeof(Depth));

		application = app;
		deviceObj = deviceObject;

		swapchainObj = new Swapchain(this);

	}
	void Renderer::Initialize()
	{
		createCommandPool();

		BuildSwapchainAndDepthImage();


		const bool includeDepth = true;

		CreateRenderpass(includeDepth);
		CreateFrameBuffer(includeDepth);

		Init2DData();

		s_Data->QuadDrawable->CreateDescriptorSetLayout(true);
		s_Data->QuadDrawable->CreateDescriptorPool(true);
		s_Data->QuadDrawable->CreateDescriptorSet(true);
		s_Data->QuadDrawable->UpdateDescriptorSet(true, s_Data->TextureSlotIndex);

		s_Data->TestTexture = Texture::Create("Vulkan/LearningVulkan.ktx");


		CreatePipelineStateManagement();
	}
	void Renderer::Prepare()
	{
		s_Data->QuadDrawable->Prepare();
	}
	void Renderer::Render()
	{
		s_Data->QuadIndexCount = 0;
		s_Data->QuadVertexBufferPtr = s_Data->QuadVertexBufferBase;
		DrawQuad({ -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f });
		//DrawQuad({ 0.f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
		//DrawQuad({ 2.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f });
		DrawQuad({ 0.0f, 0.0f, 1.0f }, s_Data->TestTexture);
		s_Data->QuadDrawable->UpdateDescriptorSet(true, s_Data->TextureSlotIndex);
		s_Data->QuadDrawable->SetIndexCount(s_Data->QuadIndexCount);
		CreateVertexBuffer();
		s_Data->QuadDrawable->Render();
	}

	void Renderer::Update(Timestep ts)
	{
		s_Data->CameraEditor->Update(ts);

		s_Data->CameraBuffer.ViewProjection = s_Data->CameraEditor->GetViewProjection();
		s_Data->CameraUniformBuffer->SetData(&s_Data->CameraBuffer, sizeof(Renderer2DData::CameraData));
	}


	void Renderer::CreatePresentationWindow(const int&
		windowWidth, const int& windowHeight)
	{
		width = windowWidth;
		height = windowHeight;
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//resizing breaks the swapchain, we'll disable it for now
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		if (application->GetInstance()->m_Window = glfwCreateWindow(width, height, "Application", nullptr, nullptr))
		{
			std::cout << "Successfully made a glfw window called \"Application\", width: " << width << ", height: " << height << '\n';
		}
		else
		{
			std::cout << "GLFW window creation failed\n";
		}

		auto& Window = Application::GetInstance()->m_Window;
		glfwSetWindowSizeCallback(Window, [](GLFWwindow* window, int width, int height) mutable {
			Application::GetInstance()->RendererObj->GetSwapchain()->SetSwapchainExtent(width, height);
			Application::GetInstance()->Resize();
			});

		glfwSetScrollCallback(Application::GetInstance()->m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				s_Data->CameraEditor->OnMouseScroll(xOffset, yOffset);
			});
	}



	void Renderer::SetImageLayout(vk::Image image, vk::ImageAspectFlags aspectMask, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout, vk::ImageSubresourceRange subresourceRange, vk::CommandBuffer& commandBuffer)
	{
		assert(commandBuffer != VK_NULL_HANDLE);

		//assert(deviceObj->Queue != VK_NULL_HANDLE);

		vk::ImageMemoryBarrier imageMemoryBarrier = {};

		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eNone;
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eNone;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		vk::PipelineStageFlags srcStages = vk::PipelineStageFlagBits::eTopOfPipe;
		vk::PipelineStageFlags dstStages = vk::PipelineStageFlagBits::eTopOfPipe;

		if (oldImageLayout == vk::ImageLayout::eColorAttachmentOptimal) {
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		}

		switch (newImageLayout)
		{
			// Ensure that anything that was copying from this image
			// has completed. An image in this layout can only be
			// used as the destination operand of the commands
		case vk::ImageLayout::eTransferDstOptimal:
		case vk::ImageLayout::ePresentSrcKHR:
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			srcStages = vk::PipelineStageFlagBits::eTopOfPipe;
			dstStages = vk::PipelineStageFlagBits::eTransfer;
			break;

			// Ensure any Copy or CPU writes to image are flushed. An image
			// in this layout can only be used as a read-only shader resource
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			srcStages = vk::PipelineStageFlagBits::eTransfer;
			dstStages = vk::PipelineStageFlagBits::eFragmentShader;
			break;

		case vk::ImageLayout::eColorAttachmentOptimal:
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
			break;

			// An image in this layout can only be used as a
			// framebuffer depth/stencil attachment
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;

			dstStages = vk::PipelineStageFlagBits::eEarlyFragmentTests;
			break;
		}


		commandBuffer.pipelineBarrier(srcStages, dstStages, {}, nullptr, nullptr, imageMemoryBarrier);
	}

	void Renderer::createCommandPool()
	{
		vk::CommandPoolCreateInfo poolInfo = {};
		poolInfo.flags = vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		poolInfo.queueFamilyIndex = deviceObj->graphicsQueueWithPresentIndex.value();

		try
		{
			cmdPool = deviceObj->LogicalDevice.createCommandPool(poolInfo);
		}

		catch (vk::SystemError err)
		{
			std::cout << "Failed to create Command Pool!" << std::endl;
		}
	}

	void Renderer::BuildSwapchainAndDepthImage()
	{
		deviceObj->GetDeviceQueue();

		swapchainObj->CreateSwapchain(cmdDepthImage);

		CreateDepthImage();
	}

	void Renderer::CreateDepthImage()
	{
		vk::ImageCreateInfo imageInfo = {};

		if (Depth.format == vk::Format::eUndefined)
		{
			Depth.format = vk::Format::eD16Unorm;
		}

		const vk::Format depthFormat = Depth.format;

		vk::FormatProperties props = deviceObj->Gpu.getFormatProperties(depthFormat);

		if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		{
			imageInfo.tiling = vk::ImageTiling::eOptimal;
			//std::cout << "Device can support optimal Depth format!\n";
		}
		else if (props.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		{
			imageInfo.tiling = vk::ImageTiling::eLinear;
			//std::cout << "Device can support linear Depth format!\n";
		}
		else
		{
			//std::cout << "Unsupported Depth Format, try other Depth formats!\n";
			exit(-1);
		}

		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = depthFormat;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;

		Depth.image = deviceObj->LogicalDevice.createImage(imageInfo);

		// Get the memory requirements
		vk::MemoryRequirements memoryRequirements = deviceObj->LogicalDevice.getImageMemoryRequirements(Depth.image);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = deviceObj->MemoryTypeFromProperties(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

		Depth.deviceMemory = deviceObj->LogicalDevice.allocateMemory(allocInfo);

		deviceObj->LogicalDevice.bindImageMemory(Depth.image, Depth.deviceMemory, 0);

		vk::ImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.image = VK_NULL_HANDLE;
		imageViewInfo.format = depthFormat;
		imageViewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
		imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		imageViewInfo.viewType = vk::ImageViewType::e2D;
		imageViewInfo.flags = {};

		if (depthFormat == vk::Format::eD16UnormS8Uint ||
			depthFormat == vk::Format::eD24UnormS8Uint ||
			depthFormat == vk::Format::eD32SfloatS8Uint)
		{
			imageViewInfo.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}

		CommandBufferMgr::allocCommandBuffer(&deviceObj->LogicalDevice, cmdPool, &cmdDepthImage);
		CommandBufferMgr::beginCommandBuffer(cmdDepthImage);

		{
			vk::ImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = 1;
			subresourceRange.layerCount = 1;

			SetImageLayout(Depth.image,
				imageViewInfo.subresourceRange.aspectMask,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eDepthStencilAttachmentOptimal,
				subresourceRange , cmdDepthImage);
		}

		CommandBufferMgr::endCommandBuffer(cmdDepthImage);
		CommandBufferMgr::submitCommandBuffer(deviceObj->Queue, &cmdDepthImage);

		imageViewInfo.image = Depth.image;
		try
		{
			Depth.view = deviceObj->LogicalDevice.createImageView(imageViewInfo);
		}
		catch (vk::SystemError err)
		{
			std::cout << "Failed to create Depth image view";
		}

	}

	void Renderer::CreateRenderpass(bool includeDepth, bool clear)
	{
		vk::AttachmentDescription attachments[2];
		attachments[0].format = swapchainObj->SwapchainPublicVars.format;
		attachments[0].samples = vk::SampleCountFlagBits::e1;
		attachments[0].loadOp = clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare;
		attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[0].initialLayout = vk::ImageLayout::eUndefined;
		attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;
		attachments[0].flags = vk::AttachmentDescriptionFlags();

		if (includeDepth)
		{
			attachments[1].format = Depth.format;
			attachments[1].samples = vk::SampleCountFlagBits::e1;
			attachments[1].loadOp = clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare;
			attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
			attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eLoad;
			attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eStore;
			attachments[1].initialLayout = vk::ImageLayout::eUndefined;
			attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			attachments[1].flags = vk::AttachmentDescriptionFlags();
		}

		// Define the color buffer attachment binding input point and layout in4
		vk::AttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

		// Define the depth buffer attachment binding point and layout in4;
		vk::AttachmentReference depthReference = {};
		depthReference.attachment = 1;
		depthReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		// Specify the attachments - color, depth, resolve, preserve, ...
		vk::SubpassDescription subpass = {};
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

		subpass.flags = vk::SubpassDescriptionFlags();
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = NULL;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorReference;
		subpass.pResolveAttachments = NULL;
		subpass.pDepthStencilAttachment = includeDepth ? &depthReference : NULL;

		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = NULL;

		// Specify the attachment and subpass associate with render pass
		vk::RenderPassCreateInfo createInfo = {};
		createInfo.attachmentCount = includeDepth ? 2 : 1;
		createInfo.pAttachments = attachments;
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpass;
		createInfo.dependencyCount = 0;
		createInfo.pDependencies = NULL;

		RenderPass = deviceObj->LogicalDevice.createRenderPass(createInfo);
	}

	void Renderer::DestroyRenderpass()
	{
		deviceObj->LogicalDevice.destroyRenderPass(RenderPass);
	}

	void Renderer::CreateFrameBuffer(bool includeDepth, bool clear)
	{
		vk::ImageView attachments[2];

		attachments[1] = Depth.view;

		vk::FramebufferCreateInfo createInfo = {};
		createInfo.flags = vk::FramebufferCreateFlags();
		createInfo.renderPass = RenderPass;
		createInfo.attachmentCount = includeDepth ? 2 : 1;
		createInfo.pAttachments = attachments;
		createInfo.width = width;
		createInfo.height = height;
		createInfo.layers = 1;

		uint32_t i;
		Framebuffers.clear();

		Framebuffers.resize(swapchainObj->SwapchainPublicVars.SwapchainImageCount);

		for (i = 0; i < swapchainObj->SwapchainPublicVars.SwapchainImageCount; i++)
		{
			attachments[0] = swapchainObj->SwapchainPublicVars.ColorBuffer[i].view;

			try {
				Framebuffers.at(i) = deviceObj->LogicalDevice.createFramebuffer(createInfo);
			}
			catch (vk::SystemError err)
			{
				std::cout << "Failed to create Framebuffer!\n";
			}

		}
	}

	void Renderer::CreateVertexBuffer()
	{

		CommandBufferMgr::allocCommandBuffer(&deviceObj->LogicalDevice, cmdPool, &cmdVertexBuffer);
		CommandBufferMgr::beginCommandBuffer(cmdVertexBuffer);

		if (s_Data->QuadIndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data->QuadVertexBufferPtr - (uint8_t*)s_Data->QuadVertexBufferBase);
			s_Data->QuadVertexBuffer->SetData(s_Data->QuadVertexBufferBase, dataSize);
			//s_Data->QuadVertexBuffer->Bind();
		}
		CommandBufferMgr::endCommandBuffer(cmdVertexBuffer);
		CommandBufferMgr::submitCommandBuffer(deviceObj->Queue, &cmdVertexBuffer);

	}

	void Renderer::CreatePipelineStateManagement()
	{

		s_Data->QuadDrawable->CreatePipelineLayout();
		pipelineObj.CreatePipelineCache();

		const bool depthPresent = true;
		vk::Pipeline* pipeline = (vk::Pipeline*)malloc(sizeof(vk::Pipeline));
		if (pipelineObj.CreatePipeline(s_Data->QuadDrawable.get(), pipeline, s_Data->QuadShader.get(), true))
		{
			Pipelines.push_back(pipeline);
			s_Data->QuadDrawable->setPipeline(pipeline);
		}
		else
		{
			free(pipeline);
			pipeline = NULL;
		}
	}

	void Renderer::CreateDescriptors()
	{
		s_Data->QuadDrawable->CreateDescriptorSetLayout(false);
	}

	void Renderer::CreateSampler(Ref<Texture>& texture)
	{
		vk::SamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.magFilter = vk::Filter::eLinear;
		samplerCreateInfo.magFilter = vk::Filter::eLinear;
		samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		samplerCreateInfo.mipLodBias = 0.0f;

		if (deviceObj->deviceFeatures.samplerAnisotropy == VK_TRUE)
		{
			samplerCreateInfo.anisotropyEnable = VK_TRUE;
			samplerCreateInfo.maxAnisotropy = 8;
		}
		else
		{
			samplerCreateInfo.anisotropyEnable = VK_FALSE;
			samplerCreateInfo.maxAnisotropy = 1;
		}

		samplerCreateInfo.compareOp = vk::CompareOp::eNever;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = (float)texture->GetTextureData().MipMapLevels;
		samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		texture->GetTextureData().Sampler = deviceObj->LogicalDevice.createSampler(samplerCreateInfo);
	}

	void Renderer::CreateImageView(Ref<Texture>& texture)
	{
		vk::ImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = texture->GetTextureData().MipMapLevels;
		subresourceRange.layerCount = 1;
		// Image view
		vk::ImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.viewType = vk::ImageViewType::e2D;
		viewCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
		viewCreateInfo.components.r = vk::ComponentSwizzle::eR;
		viewCreateInfo.components.g = vk::ComponentSwizzle::eG;
		viewCreateInfo.components.b = vk::ComponentSwizzle::eB;
		viewCreateInfo.components.a = vk::ComponentSwizzle::eA;
		viewCreateInfo.subresourceRange = subresourceRange;
		viewCreateInfo.subresourceRange.levelCount = texture->GetTextureData().MipMapLevels;
		viewCreateInfo.image = texture->GetTextureData().Image;

		texture->GetTextureData().View = deviceObj->LogicalDevice.createImageView(viewCreateInfo);
	}

	void Renderer::CreateTextureOptimal(const char* filename, TextureData* texture, vk::ImageUsageFlags imageUsageFlags, vk::Format format)
	{
		gli::texture2d image2D(gli::load(filename));
		std::cout << sizeof(image2D);
		assert(!image2D.empty());

		texture->Width = uint32_t(image2D.extent().x);
		texture->Height = uint32_t(image2D.extent().y);
		texture->MipMapLevels = uint32_t(image2D.levels());

		vk::BufferCreateInfo createInfo = {};
		createInfo.size = image2D.size();
		createInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		createInfo.sharingMode = vk::SharingMode::eExclusive;

		vk::Buffer buffer;
		try {
			buffer = deviceObj->LogicalDevice.createBuffer(createInfo);
		}
		catch (vk::SystemError err)
		{
			std::cout << "Failed to create texture buffer!\n";
		}

		vk::MemoryRequirements memoryRequirements = deviceObj->LogicalDevice.getBufferMemoryRequirements(buffer);

		vk::MemoryAllocateInfo memAllocInfo = {};
		memAllocInfo.allocationSize = memoryRequirements.size;
		memAllocInfo.memoryTypeIndex = deviceObj->MemoryTypeFromProperties(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		vk::DeviceMemory deviceMemory = deviceObj->LogicalDevice.allocateMemory(memAllocInfo);

		deviceObj->LogicalDevice.bindBufferMemory(buffer, deviceMemory, 0);

		void* data = deviceObj->LogicalDevice.mapMemory(deviceMemory, 0, memoryRequirements.size);
		memcpy(data, image2D.data(), image2D.size());
		deviceObj->LogicalDevice.unmapMemory(deviceMemory);

		vk::ImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.imageType = vk::ImageType::e2D;
		imageCreateInfo.format = format;
		imageCreateInfo.mipLevels = texture->MipMapLevels;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
		imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
		imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageCreateInfo.extent = vk::Extent3D(texture->Width, texture->Height, 1);
		imageCreateInfo.usage = imageUsageFlags;

		if (!(imageCreateInfo.usage & vk::ImageUsageFlagBits::eTransferDst))
		{
			imageCreateInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
		}

		try {
			texture->Image = deviceObj->LogicalDevice.createImage(imageCreateInfo);
		}
		catch (vk::SystemError err)
		{
			std::cout << "Failed to create Texture Image!\n";
		}

		memoryRequirements = deviceObj->LogicalDevice.getImageMemoryRequirements(texture->Image);

		memAllocInfo.allocationSize = memoryRequirements.size;
		memAllocInfo.memoryTypeIndex = deviceObj->MemoryTypeFromProperties(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

		texture->Memory = deviceObj->LogicalDevice.allocateMemory(memAllocInfo);

		deviceObj->LogicalDevice.bindImageMemory(texture->Image, texture->Memory, 0);

		vk::ImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = texture->MipMapLevels;
		subresourceRange.layerCount = 1;

		CommandBufferMgr::allocCommandBuffer(&deviceObj->LogicalDevice, cmdPool, &cmdTexture);
		CommandBufferMgr::beginCommandBuffer(cmdTexture);

		SetImageLayout(texture->Image, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, subresourceRange, cmdTexture);

		std::vector<vk::BufferImageCopy> bufferImageCopyList;

		uint32_t bufferOffset = 0;
		for (uint32_t i = 0; i < texture->MipMapLevels; i++)
		{
			vk::BufferImageCopy bufferImageCopyItem = {};
			bufferImageCopyItem.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			bufferImageCopyItem.imageSubresource.mipLevel = i;
			bufferImageCopyItem.imageSubresource.layerCount = 1;
			bufferImageCopyItem.imageSubresource.baseArrayLayer = 0;
			bufferImageCopyItem.imageExtent.width = uint32_t(image2D[i].extent().x);
			bufferImageCopyItem.imageExtent.height = uint32_t(image2D[i].extent().y);
			bufferImageCopyItem.imageExtent.depth = 1;
			bufferImageCopyItem.bufferOffset = bufferOffset;

			bufferImageCopyList.push_back(bufferImageCopyItem);

			bufferOffset += uint32_t(image2D[i].size());
		}

		cmdTexture.copyBufferToImage(buffer, texture->Image, vk::ImageLayout::eTransferDstOptimal, bufferImageCopyList.size(), bufferImageCopyList.data());

		texture->ImageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		SetImageLayout(texture->Image, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eTransferDstOptimal, texture->ImageLayout, subresourceRange, cmdTexture);
		CommandBufferMgr::endCommandBuffer(cmdTexture);

		vk::Fence fence = Init::MakeFence(deviceObj->LogicalDevice);

		vk::SubmitInfo submitInfo = {};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdTexture;
		CommandBufferMgr::submitCommandBuffer(deviceObj->Queue, &cmdTexture, &submitInfo, fence);
		deviceObj->LogicalDevice.waitForFences(1, &fence, VK_TRUE, 10000000000);
		deviceObj->LogicalDevice.destroyFence(fence);
		deviceObj->LogicalDevice.freeMemory(deviceMemory);
		deviceObj->LogicalDevice.destroyBuffer(buffer);

		// Create an sampler
		vk::SamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.magFilter = vk::Filter::eLinear;
		samplerCreateInfo.magFilter = vk::Filter::eLinear;
		samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		samplerCreateInfo.mipLodBias = 0.0f;

		if (deviceObj->deviceFeatures.samplerAnisotropy == VK_TRUE)
		{
			samplerCreateInfo.anisotropyEnable = VK_TRUE;
			samplerCreateInfo.maxAnisotropy = 8;
		}
		else
		{
			samplerCreateInfo.anisotropyEnable = VK_FALSE;
			samplerCreateInfo.maxAnisotropy = 1;
		}

		samplerCreateInfo.compareOp = vk::CompareOp::eNever;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = (float)texture->MipMapLevels;
		samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		texture->Sampler = deviceObj->LogicalDevice.createSampler(samplerCreateInfo);

		// Image view
		vk::ImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.viewType = vk::ImageViewType::e2D;
		viewCreateInfo.format = format;
		viewCreateInfo.components.r = vk::ComponentSwizzle::eR;
		viewCreateInfo.components.g = vk::ComponentSwizzle::eG;
		viewCreateInfo.components.b = vk::ComponentSwizzle::eB;
		viewCreateInfo.components.a = vk::ComponentSwizzle::eA;
		viewCreateInfo.subresourceRange = subresourceRange;
		viewCreateInfo.subresourceRange.levelCount = texture->MipMapLevels;
		viewCreateInfo.image = texture->Image;

		texture->View = deviceObj->LogicalDevice.createImageView(viewCreateInfo);
		texture->DescriptorInfo.imageView = texture->View;
		texture->DescriptorInfo.sampler = texture->Sampler;
		texture->DescriptorInfo.imageLayout = vk::ImageLayout::eGeneral;
	}

	void Renderer::DrawQuad(const glm::vec3& position, const glm::vec4& color)
	{
		constexpr size_t quadVertexCount = 4;
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		const float textureIndex = 0.0f;
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data->QuadVertexBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
			s_Data->QuadVertexBufferPtr->Color = color;
			s_Data->QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data->QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data->QuadVertexBufferPtr++;
		}

		s_Data->QuadIndexCount += 6;
	}

	void Renderer::DrawQuad(const glm::vec3& position, const Ref<Texture>& texture, const glm::vec4 tintColor)
	{
		constexpr size_t quadVertexCount = 4;
		constexpr glm::vec2 textureCoords[] = { { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f } };
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++)
		{
			if (*s_Data->TextureSlots[i] == *texture)
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;
			s_Data->DescriptorImageInfos[s_Data->TextureSlotIndex] = texture->GetTextureData().DescriptorInfo;
			s_Data->QuadDrawable->SetDescriptorImageInfo(s_Data->DescriptorImageInfos);
			s_Data->TextureSlotIndex++;
		}

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data->QuadVertexBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
			s_Data->QuadVertexBufferPtr->Color = tintColor;
			s_Data->QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data->QuadVertexBufferPtr->TexIndex = textureIndex;

			s_Data->QuadVertexBufferPtr++;
		}

		s_Data->QuadIndexCount += 6;
	}

	void Renderer::DestroyFrameBuffers()
	{
		for (uint32_t i = 0; i < swapchainObj->SwapchainPublicVars.SwapchainImageCount; i++)
		{
			deviceObj->LogicalDevice.destroyFramebuffer(Framebuffers.at(i));
		}

		Framebuffers.clear();
	}

	void Renderer::DestroyDepthBuffer()
	{
		deviceObj->LogicalDevice.destroyImageView(Depth.view);
		deviceObj->LogicalDevice.destroyImage(Depth.image);
		deviceObj->LogicalDevice.freeMemory(Depth.deviceMemory);
	}

	void Renderer::DestroyDrawableVertexBuffer()
	{
		for (Drawable* drawableObj : drawables)
		{
			drawableObj->DestroyVertexBuffer();
		}
	}

	void Renderer::DestroyDrawableUniformBuffer()
	{
		for (Drawable* drawable : drawables)
		{
			drawable->DestroyUniformBuffer();
		}
	}

	void Renderer::DestroyPipeline()
	{
		for (vk::Pipeline * pipeline : Pipelines)
		{
			deviceObj->LogicalDevice.destroyPipeline(*pipeline);
			free(pipeline);
		}
		Pipelines.clear();
	}

	void Renderer::DestroyCommandBuffer()
	{
		vk::CommandBuffer cmdBufs[] = { cmdDepthImage, cmdVertexBuffer };

		deviceObj->LogicalDevice.freeCommandBuffers(cmdPool, sizeof(cmdBufs) / sizeof(vk::CommandBuffer), cmdBufs);
	}

	void Renderer::DestroyCommandPool()
	{
		Device* deviceObj = application->DeviceObj;

		deviceObj->LogicalDevice.destroyCommandPool(cmdPool);
	}

	void Renderer::DestroyPresentationWindow()
	{

	}

	Renderer::~Renderer()
	{
		//deviceObj->LogicalDevice.destroyDeviceMemo
		delete swapchainObj;
		DestroyRenderpass();
		try {
			deviceObj->LogicalDevice.destroyCommandPool(cmdPool);
		}
		catch (vk::SystemError err)
		{
			std::cout << err.what() << std::endl;
		}
	}
}
