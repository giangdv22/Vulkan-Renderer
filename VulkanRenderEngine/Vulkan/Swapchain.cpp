#include "gpch.h"
#include "Swapchain.h"
#include "Renderer.h"
#include "Utils.h"
#include "Init.h"
namespace Giang
{
	Swapchain::Swapchain(Renderer* renderer)
	{
		rendererObj = renderer;
		appObj = Application::GetInstance();
	}

	void Swapchain::InitializeSwapchain()
	{
		CreateSurface();

		uint32_t index = GetGraphicsQueueWithPresentationSupport();

		if (index == UINT32_MAX)
		{
			std::cout << "Could not find a graphics and a present queue\n";
		}

		rendererObj->GetDevice()->graphicsQueueWithPresentIndex = index;

		GetSupportedFormats();
	}

	void Swapchain::CreateSwapchain(const vk::CommandBuffer& commandBuffer)
	{
		if (SwapchainPublicVars.Surface == VK_NULL_HANDLE)
		{
			CreateSurface();
		}

		GetSurfaceCapabilitiesAndPresentMode();

		ManagePresentMode();

		CreateSwapChainColorImages();

		CreateColorImageView(commandBuffer);

		CreateSyncObject();
	}

	void Swapchain::CreateSwapChainColorImages()
	{
		vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
			vk::SwapchainCreateFlagsKHR(),
			SwapchainPublicVars.Surface,
			SwapchainPrivateVars.DesiredNumberOfSwapchainImages,
			SwapchainPublicVars.format,
			SwapchainPublicVars.colorSpace,
			SwapchainPrivateVars.SwapchainExtent,
			1,
			vk::ImageUsageFlagBits::eColorAttachment
		);

		uint32_t queueFamilyIndices[] = { rendererObj->GetDevice()->GraphicsQueueIndex.value(), rendererObj->GetDevice()->graphicsQueueWithPresentIndex.value() };

		if (rendererObj->GetDevice()->GraphicsQueueIndex.value() != rendererObj->GetDevice()->graphicsQueueWithPresentIndex.value())
		{
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		else
		{
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		}

		createInfo.preTransform = SwapchainPrivateVars.PreTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

		createInfo.presentMode = SwapchainPrivateVars.SwapchainPresentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

		try
		{
			SwapchainPublicVars.SwapChain = rendererObj->GetDevice()->LogicalDevice.createSwapchainKHR(createInfo);
		}
		catch (vk::SystemError err)
		{
			throw std::runtime_error("Failed to create Swapchain!");
		}

		SwapchainPrivateVars.SwapchainImages = rendererObj->GetDevice()->LogicalDevice.getSwapchainImagesKHR(SwapchainPublicVars.SwapChain);
		SwapchainPublicVars.SwapchainImageCount = SwapchainPrivateVars.SwapchainImages.size();

	}

	void Swapchain::CreateColorImageView(const vk::CommandBuffer& cmdBuffer)
	{
		SwapchainBuffer sc_buffer;
		SwapchainPublicVars.ColorBuffer.clear();
		for (uint32_t i = 0; i < SwapchainPublicVars.SwapchainImageCount; i++)
		{
			vk::ImageViewCreateInfo createInfo = {};
			createInfo.image = SwapchainPrivateVars.SwapchainImages[i];
			createInfo.viewType = vk::ImageViewType::e2D;

			createInfo.components.r = vk::ComponentSwizzle::eIdentity;
			createInfo.components.g = vk::ComponentSwizzle::eIdentity;
			createInfo.components.b = vk::ComponentSwizzle::eIdentity;
			createInfo.components.a = vk::ComponentSwizzle::eIdentity;

			createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			createInfo.format = SwapchainPublicVars.format;

			sc_buffer.image = SwapchainPrivateVars.SwapchainImages[i];
			createInfo.image = sc_buffer.image;
			sc_buffer.view = rendererObj->GetDevice()->LogicalDevice.createImageView(createInfo);

			SwapchainPublicVars.ColorBuffer.push_back(sc_buffer);
		}
		SwapchainPublicVars.CurrentColorBuffer = 0;
	}

	void Swapchain::CreateSyncObject()
	{
		SwapchainPublicVars.ImageAvailableSamephores.resize(SwapchainPublicVars.SwapchainImageCount);
		SwapchainPublicVars.RenderFinishedSemaphores.resize(SwapchainPublicVars.SwapchainImageCount);
		SwapchainPublicVars.fences.resize(SwapchainPublicVars.SwapchainImageCount);
		for (uint32_t i = 0; i < SwapchainPublicVars.SwapchainImageCount; i++)
		{
			SwapchainPublicVars.ImageAvailableSamephores[i] = Init::MakeSemaphore(appObj->DeviceObj->LogicalDevice);
			SwapchainPublicVars.RenderFinishedSemaphores[i] = Init::MakeSemaphore(appObj->DeviceObj->LogicalDevice);
			SwapchainPublicVars.fences[i] = Init::MakeFence(appObj->DeviceObj->LogicalDevice);
		}
	}

	uint32_t Swapchain::GetGraphicsQueueWithPresentationSupport()
	{
		Device* device = appObj->DeviceObj;
		uint32_t queueCount = device->queueFamilyCount;
		vk::PhysicalDevice gpu = device->Gpu;

		std::vector<vk::QueueFamilyProperties>& queueProps = device->QueueFamilyProps;

		vk::Bool32* supportPresent = (vk::Bool32*)malloc(queueCount * sizeof(vk::Bool32));

		for (uint32_t i = 0; i < queueCount; i++)
		{
			supportPresent[i] = gpu.getSurfaceSupportKHR(i, SwapchainPublicVars.Surface);
		}

		uint32_t graphicsQueueNodeIndex = UINT32_MAX;
		uint32_t presentQueueNodeIndex = UINT32_MAX;

		// Search for a graphics & present queue in the array of queue families
		// that support both
		for (uint32_t i = 0; i < queueCount; i++)
		{
			if ((queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics))
			{
				if (graphicsQueueNodeIndex == UINT32_MAX)
				{
					graphicsQueueNodeIndex = i;
				}

				if (supportPresent[i] == VK_TRUE)
				{
					graphicsQueueNodeIndex = i;
					presentQueueNodeIndex = i;
					break;
				}
			}
		}


		// If not, find another queue Family that support the present
		if (presentQueueNodeIndex == UINT32_MAX)
		{
			for (uint32_t i = 0; i < queueCount; ++i)
			{
				if (supportPresent[i] == VK_TRUE)
				{
					presentQueueNodeIndex = i;
					break;
				}
			}
		}

		free(supportPresent);

		// Generate error if could not find both a graphics and a present queue
		if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
		{
			return UINT32_MAX;
		}

		return graphicsQueueNodeIndex;
	}

	void Swapchain::CreateSurface()
	{
		VkSurfaceKHR c_style_surface;
		if (glfwCreateWindowSurface(appObj->InstanceObj.InstanceObj, appObj->m_Window, nullptr, &c_style_surface) != VK_SUCCESS)
		{
			std::cout << "Failed to abstract the glfw surface for Vulkan!\n";
		}

		else
		{
			std::cout << "Successfully abstracted the glfw surface for Vulkan!\n";
		}

		SwapchainPublicVars.Surface = c_style_surface;
	}

	void Swapchain::GetSupportedFormats()
	{
		vk::PhysicalDevice gpu = rendererObj->GetDevice()->Gpu;
		SwapchainPrivateVars.SurfaceFormats = gpu.getSurfaceFormatsKHR(SwapchainPublicVars.Surface);

		for (vk::SurfaceFormatKHR format : SwapchainPrivateVars.SurfaceFormats)
		{
			if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				SwapchainPublicVars.format = format.format;
				SwapchainPublicVars.colorSpace = format.colorSpace;
				return;
			}
		}
		SwapchainPublicVars.format = SwapchainPrivateVars.SurfaceFormats[0].format;
		SwapchainPublicVars.colorSpace = SwapchainPrivateVars.SurfaceFormats[0].colorSpace;
	}

	void Swapchain::GetSurfaceCapabilitiesAndPresentMode()
	{
		vk::PhysicalDevice gpu = rendererObj->GetDevice()->Gpu;
		SwapchainPrivateVars.SurfaceCapabilities = gpu.getSurfaceCapabilitiesKHR(SwapchainPublicVars.Surface);

		/*std::cout << "Swapchain can support the following surface capabilities:\n";

		std::cout << "\tmin image count: " << SwapchainPrivateVars.SurfaceCapabilities.minImageCount << "\n";
		std::cout << "\tmax image count: " << SwapchainPrivateVars.SurfaceCapabilities.maxImageCount << "\n";

		Utils::LogExtent(SwapchainPrivateVars.SurfaceCapabilities);

		std::cout << "\tmax image array layers: " << SwapchainPrivateVars.SurfaceCapabilities.maxImageArrayLayers << "\n";

		std::vector<std::string> stringList = Utils::LogTransformBits(SwapchainPrivateVars.SurfaceCapabilities.supportedTransforms);
		std::cout << "\tsupported transform:\n";
		for(std::string line : stringList)
		{
			std::cout << "\t\t" << line << "\n";
		}

		std::cout << "\tSupported alpha operations:\n";

		stringList = Utils::LogAlphaCompositeBits(SwapchainPrivateVars.SurfaceCapabilities.supportedCompositeAlpha);
		for (std::string line : stringList)
		{
			std::cout << "\t\t" << line << "\n";
		}

		std::cout << "\tsupport image usage:\n";
		stringList = Utils::LogImageUsageBits(SwapchainPrivateVars.SurfaceCapabilities.supportedUsageFlags);
		for (std::string line : stringList)
		{
			std::cout << "\t\t" << line << "\n";
		}*/

		SwapchainPrivateVars.PresentModes = gpu.getSurfacePresentModesKHR(SwapchainPublicVars.Surface);
		SwapchainPrivateVars.PresentModeCount = SwapchainPrivateVars.PresentModes.size();
		/*for (vk::PresentModeKHR presentMode : SwapchainPrivateVars.PresentModes)
		{
			std::cout << "\t" << Utils::LogPresentMode(presentMode) << "\n";
		}*/

		if (SwapchainPrivateVars.SurfaceCapabilities.currentExtent.width == (uint32_t)-1)
		{
			SwapchainPrivateVars.SwapchainExtent.width = rendererObj->width;
			SwapchainPrivateVars.SwapchainExtent.height = rendererObj->height;
		}

		else
		{
			SwapchainPrivateVars.SwapchainExtent = SwapchainPrivateVars.SurfaceCapabilities.currentExtent;
		}
	}

	void Swapchain::ManagePresentMode()
	{
		vk::PhysicalDevice gpu = rendererObj->GetDevice()->Gpu;
		SwapchainPrivateVars.SwapchainPresentMode = vk::PresentModeKHR::eFifo;

		for (uint32_t i = 0; i < SwapchainPrivateVars.PresentModeCount; i++)
		{
			if (SwapchainPrivateVars.PresentModes[i] == vk::PresentModeKHR::eMailbox)
			{
				SwapchainPrivateVars.SwapchainPresentMode = vk::PresentModeKHR::eMailbox;
				break;
			}
		}

		SwapchainPrivateVars.DesiredNumberOfSwapchainImages = SwapchainPrivateVars.SurfaceCapabilities.minImageCount + 1;
		if ((SwapchainPrivateVars.SurfaceCapabilities.maxImageCount > 0) &&
			(SwapchainPrivateVars.DesiredNumberOfSwapchainImages > SwapchainPrivateVars.SurfaceCapabilities.maxImageCount))
		{
			SwapchainPrivateVars.DesiredNumberOfSwapchainImages = SwapchainPrivateVars.SurfaceCapabilities.maxImageCount;
		}

		if (SwapchainPrivateVars.SurfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
		{
			SwapchainPrivateVars.PreTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
		}

		else
		{
			SwapchainPrivateVars.PreTransform = SwapchainPrivateVars.SurfaceCapabilities.currentTransform;
		}
	}

	void Swapchain::SetSwapchainExtent(uint32_t swapchainWidth, uint32_t swapchainHeight)
	{
		SwapchainPrivateVars.SwapchainExtent.width = swapchainWidth;
		SwapchainPrivateVars.SwapchainExtent.height = swapchainHeight;
		rendererObj->width = swapchainWidth;
		rendererObj->height = swapchainHeight;
	}

	void Swapchain::DestroySwapchain()
	{
		Device* deviceObj = appObj->DeviceObj;

		for (uint32_t i = 0; i < SwapchainPublicVars.SwapchainImageCount; i++) {
			deviceObj->LogicalDevice.destroyImageView(SwapchainPublicVars.ColorBuffer[i].view);
		}

		//fpDestroySwapchainKHR(deviceObj->device, scPublicVars.swapChain, NULL);
		deviceObj->LogicalDevice.destroySwapchainKHR(SwapchainPublicVars.SwapChain);
		appObj->InstanceObj.InstanceObj.destroySurfaceKHR(SwapchainPublicVars.Surface);
		SwapchainPublicVars.Surface = VK_NULL_HANDLE;
	}

	Swapchain::~Swapchain()
	{
		DestroySwapchain();

		SwapchainPrivateVars.SwapchainImages.clear();
		SwapchainPrivateVars.SurfaceFormats.clear();
		SwapchainPrivateVars.PresentModes.clear();

	}

}