#include "gpch.h"
#include "Renderer.h"

#include "CommandBufferMgr.h"
#include "Init.h"
#include "MeshData.h"
#include "EditorCamera.h"
#include "UniformBuffer.h"
namespace Giang
{
	struct QuadVertex
	{
		glm::vec4 Position;
		glm::vec4 Color;
	};

	struct Renderer2DData
	{
		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;

		Ref<VertexInputDescription> QuadViDescription;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> QuadShader;
		Ref<Drawable> QuadDrawable;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		glm::vec4 QuadVertexPositions[4]{
			{ -0.5,  0.5, 0.0, 1.0f},
			{  0.5,  0.5, 0.0, 1.0f},
			{  0.5, -0.5, 0.0, 1.0f},
			{ -0.5, -0.5, 0.0, 1.0f}
		};

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;
		Ref<EditorCamera> CameraEditor;
	};

	static Renderer2DData s_Data;

	void Renderer::Init2DData()
	{
		s_Data.QuadViDescription = VertexInputDescription::Create();
		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
		s_Data.QuadVertexBuffer->SetLayout(
			{
				{ShaderDataType::Float4 },
				{ShaderDataType::Float4 }
			});
		s_Data.QuadViDescription->SetVertexBuffer(s_Data.QuadVertexBuffer);

		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxIndices];

		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];
		uint32_t offset = 0;

		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
		s_Data.QuadViDescription->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		s_Data.CameraEditor = CreateRef<EditorCamera>(45.0f, 1.778f, 0.1f, 100.0f);

		s_Data.QuadShader = Shader::Create("shaders/QuadVertex.spv", "shaders/QuadFragment.spv");
		s_Data.QuadDrawable = Drawable::Create(Application::GetInstance()->RendererObj);
		s_Data.QuadDrawable->SetVertexInputDescription(s_Data.QuadViDescription);
		s_Data.QuadDrawable->CreateDescriptorSetLayout(false);
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
		s_Data.QuadDrawable->SetUniformBuffer(s_Data.CameraUniformBuffer);
		s_Data.QuadDrawable->CreateDescriptor(false);
	}

	Renderer::Renderer(Application* app, Device* deviceObject)
	{
		
		memset(&Depth, 0, sizeof(Depth));

		application = app;
		deviceObj = deviceObject;

		swapchainObj = new Swapchain(this);

		Drawable* drawableObj = new Drawable(this);
		drawables.push_back(drawableObj);

	}
	void Renderer::Initialize()
	{
		createCommandPool();

		BuildSwapchainAndDepthImage();


		const bool includeDepth = true;

		CreateRenderpass(includeDepth);
		CreateFrameBuffer(includeDepth);
		//CreateShader();
		//CreateDescriptors();

		//CreateVertexBuffer();
		Init2DData();
		CreatePipelineStateManagement();
	}
	void Renderer::Prepare()
	{
		/*for (Drawable* drawable : drawables)
		{
			drawable->Prepare();
		}*/
		s_Data.QuadDrawable->Prepare();
	}
	void Renderer::Render()
	{
		/*for (Drawable* drawable : drawables)
		{
			drawable->Render();
		}*/
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
		DrawQuad({ -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f });
		DrawQuad({ 0.f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
		DrawQuad({ 2.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f });
		DrawQuad({ -2.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f });
		s_Data.QuadDrawable->SetIndexCount(s_Data.QuadIndexCount);
		CreateVertexBuffer();
		s_Data.QuadDrawable->Render();
	}

	void Renderer::Update(Timestep ts)
	{
		s_Data.CameraEditor->Update(ts);

		s_Data.CameraBuffer.ViewProjection = s_Data.CameraEditor->GetViewProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));
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
				s_Data.CameraEditor->OnMouseScroll(xOffset, yOffset);
			});
	}



	void Renderer::SetImageLayout(vk::Image image, vk::ImageAspectFlags aspectMask, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout, vk::AccessFlagBits srcAccessMask, vk::CommandBuffer& commandBuffer)
	{
		assert(commandBuffer != VK_NULL_HANDLE);

		assert(deviceObj->Queue != VK_NULL_HANDLE);

		vk::ImageMemoryBarrier imageMemoryBarrier = {};

		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eNone;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		imageMemoryBarrier.subresourceRange.levelCount = 1;
		imageMemoryBarrier.subresourceRange.layerCount = 1;

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
			break;

			// Ensure any Copy or CPU writes to image are flushed. An image
			// in this layout can only be used as a read-only shader resource
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			break;

		case vk::ImageLayout::eColorAttachmentOptimal:
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
			break;

			// An image in this layout can only be used as a
			// framebuffer depth/stencil attachment
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			break;
		}

		vk::PipelineStageFlags srcStages = vk::PipelineStageFlagBits::eTopOfPipe;
		vk::PipelineStageFlags dstStages = vk::PipelineStageFlagBits::eEarlyFragmentTests;

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
			std::cout << "Device can support optimal Depth format!\n";
		}
		else if (props.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		{
			imageInfo.tiling = vk::ImageTiling::eLinear;
			std::cout << "Device can support linear Depth format!\n";
		}
		else
		{
			std::cout << "Unsupported Depth Format, try other Depth formats!\n";
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
			SetImageLayout(Depth.image,
				imageViewInfo.subresourceRange.aspectMask,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eDepthStencilAttachmentOptimal,
				vk::AccessFlagBits::eNone, cmdDepthImage);
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

		if (s_Data.QuadIndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
			s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);
			//s_Data.QuadVertexBuffer->Bind();
		}
		CommandBufferMgr::endCommandBuffer(cmdVertexBuffer);
		CommandBufferMgr::submitCommandBuffer(deviceObj->Queue, &cmdVertexBuffer);

	}

	void Renderer::CreateShader()
	{
		shaderObj = Shader::Create("shaders/vertex.spv", "shaders/fragment.spv");
	}

	void Renderer::CreatePipelineStateManagement()
	{

		/*for (Drawable* drawable : drawables)
		{
			drawable->CreatePipelineLayout();
		}*/
		s_Data.QuadDrawable->CreatePipelineLayout();
		pipelineObj.CreatePipelineCache();

		const bool depthPresent = true;
		/*for (Drawable * drawableObj : drawables)
		{
			vk::Pipeline* pipeline = (vk::Pipeline*)malloc(sizeof(vk::Pipeline));
			if (pipelineObj.CreatePipeline(drawableObj, pipeline, shaderObj.get(), depthPresent))
			{
				Pipelines.push_back(pipeline);
				drawableObj->setPipeline(pipeline);
			}
			else
			{
				free(pipeline);
				pipeline = NULL;
			}
		}*/
		vk::Pipeline* pipeline = (vk::Pipeline*)malloc(sizeof(vk::Pipeline));
		if (pipelineObj.CreatePipeline(s_Data.QuadDrawable.get(), pipeline, s_Data.QuadShader.get(), true))
		{
			Pipelines.push_back(pipeline);
			s_Data.QuadDrawable->setPipeline(pipeline);
		}
		else
		{
			free(pipeline);
			pipeline = NULL;
		}
	}

	void Renderer::CreateDescriptors()
	{
		/*for (Drawable* drawable : drawables)
		{
			drawable->CreateDescriptorSetLayout(false);
			drawable->CreateDescriptor(false);
		}*/
		s_Data.QuadDrawable->CreateDescriptorSetLayout(false);
	}

	void Renderer::DrawQuad(const glm::vec3& position, const glm::vec4& color)
	{
		constexpr size_t quadVertexCount = 4;
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
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
