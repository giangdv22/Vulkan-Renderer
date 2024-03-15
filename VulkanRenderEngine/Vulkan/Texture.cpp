#include "gpch.h"
#include "Texture.h"
#include "Application.h"
#include "Init.h"
#include <stb_image.h>
#include <gli/gli/gli.hpp>
#include "CommandBufferMgr.h"
namespace Giang
{
	static uint32_t s_TexID = 1;
	static vk::CommandPool cmdPool = {};

	Texture::Texture()
	{

		deviceObj = Application::GetInstance()->DeviceObj;

		if (!cmdPool)
		{
			CreateCommandPool();
		}

		m_Texture.Width = 1;
		m_Texture.Height = 1;
		m_Texture.MipMapLevels = 1;
		m_TexID = 0;

		vk::ImageCreateInfo imageCreateInfo= {};
		imageCreateInfo.imageType = vk::ImageType::e2D;
		imageCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
		imageCreateInfo.extent.width = 1;
		imageCreateInfo.extent.height = 1;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
		imageCreateInfo.queueFamilyIndexCount = 0;
		imageCreateInfo.pQueueFamilyIndices = NULL;
		imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		imageCreateInfo.flags = vk::ImageCreateFlags();
		imageCreateInfo.initialLayout = vk::ImageLayout::ePreinitialized;
		imageCreateInfo.tiling = vk::ImageTiling::eLinear;
		imageCreateInfo.usage = vk::ImageUsageFlagBits::eSampled;


		m_Texture.Image = deviceObj->LogicalDevice.createImage(imageCreateInfo);

		vk::MemoryRequirements memoryRequirements = deviceObj->LogicalDevice.getImageMemoryRequirements(m_Texture.Image);

		vk::MemoryAllocateInfo memAlloc = {};
		memAlloc.allocationSize = memoryRequirements.size;
		memAlloc.memoryTypeIndex = deviceObj->MemoryTypeFromProperties(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);

		m_Texture.Memory = deviceObj->LogicalDevice.allocateMemory(memAlloc);

		deviceObj->LogicalDevice.bindImageMemory(m_Texture.Image, m_Texture.Memory, 0);

	}

	Texture::Texture(const std::string& path)
		: m_Path(path)
	{
		deviceObj = Application::GetInstance()->DeviceObj;

		if (cmdPool == NULL)
		{
			CreateCommandPool();
		}

		gli::texture2d image2D(gli::load(path));
		assert(!image2D.empty());

		m_Texture.Width = uint32_t(image2D.extent().x);
		m_Texture.Height = uint32_t(image2D.extent().y);
		m_Texture.MipMapLevels = uint32_t(image2D.levels());
		m_TexID = s_TexID;

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

		vk::DeviceMemory deviceMemory;
		deviceMemory = deviceObj->LogicalDevice.allocateMemory(memAllocInfo);

		deviceObj->LogicalDevice.bindBufferMemory(buffer, deviceMemory, 0);

		void* data = deviceObj->LogicalDevice.mapMemory(deviceMemory, 0, memoryRequirements.size);
		memcpy(data, image2D.data(), image2D.size());
		deviceObj->LogicalDevice.unmapMemory(deviceMemory);

		vk::ImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.imageType = vk::ImageType::e2D;
		imageCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
		imageCreateInfo.mipLevels = m_Texture.MipMapLevels;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
		imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
		imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageCreateInfo.extent = vk::Extent3D(m_Texture.Width, m_Texture.Height, 1);
		imageCreateInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage;

		if (!(imageCreateInfo.usage & vk::ImageUsageFlagBits::eTransferDst))
		{
			imageCreateInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
		}

		try {
			m_Texture.Image = deviceObj->LogicalDevice.createImage(imageCreateInfo);
		}
		catch (vk::SystemError err)
		{
			std::cout << "Failed to create Texture Image!\n";
		}

		memoryRequirements = deviceObj->LogicalDevice.getImageMemoryRequirements(m_Texture.Image);

		memAllocInfo.allocationSize = memoryRequirements.size;
		memAllocInfo.memoryTypeIndex = deviceObj->MemoryTypeFromProperties(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

		m_Texture.Memory = deviceObj->LogicalDevice.allocateMemory(memAllocInfo);

		deviceObj->LogicalDevice.bindImageMemory(m_Texture.Image, m_Texture.Memory, 0);

		vk::ImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = m_Texture.MipMapLevels;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		CommandBufferMgr::allocCommandBuffer(&deviceObj->LogicalDevice, cmdPool, &cmdBuffer);
		CommandBufferMgr::beginCommandBuffer(cmdBuffer);

		Renderer::SetImageLayout(m_Texture.Image, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, subresourceRange, cmdBuffer);

		std::vector<vk::BufferImageCopy> bufferImgCopyList;

		uint32_t bufferOffset = 0;
		// Iterater through each mip level and set buffer image copy -
		for (uint32_t i = 0; i < m_Texture.MipMapLevels; i++)
		{
			VkBufferImageCopy bufImgCopyItem = {};
			bufImgCopyItem.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufImgCopyItem.imageSubresource.mipLevel = i;
			bufImgCopyItem.imageSubresource.layerCount = 1;
			bufImgCopyItem.imageSubresource.baseArrayLayer = 0;
			bufImgCopyItem.imageExtent.width = uint32_t(image2D[i].extent().x);
			bufImgCopyItem.imageExtent.height = uint32_t(image2D[i].extent().y);
			bufImgCopyItem.imageExtent.depth = 1;
			bufImgCopyItem.bufferOffset = bufferOffset;

			bufferImgCopyList.push_back(bufImgCopyItem);

			// adjust buffer offset
			bufferOffset += uint32_t(image2D[i].size());
		}
		cmdBuffer.copyBufferToImage(buffer, m_Texture.Image, vk::ImageLayout::eTransferDstOptimal, uint32_t(bufferImgCopyList.size()), bufferImgCopyList.data());
		m_Texture.ImageLayout = vk::ImageLayout::eShaderReadOnlyOptimal ;
		Renderer::SetImageLayout(m_Texture.Image, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eTransferDstOptimal, m_Texture.ImageLayout, subresourceRange, cmdBuffer);
		CommandBufferMgr::endCommandBuffer(cmdBuffer);

		vk::Fence fence;
		vk::FenceCreateInfo fenceCI = {};
		fenceCI.flags = vk::FenceCreateFlags();

		fence = deviceObj->LogicalDevice.createFence(fenceCI);

		vk::SubmitInfo submitInfo = {};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		CommandBufferMgr::submitCommandBuffer(deviceObj->Queue, &cmdBuffer, &submitInfo, fence);

		deviceObj->LogicalDevice.waitForFences(1, &fence, VK_TRUE, 10000000000);
		deviceObj->LogicalDevice.destroyFence(fence);

		deviceObj->LogicalDevice.freeMemory(deviceMemory);
		deviceObj->LogicalDevice.destroyBuffer(buffer);

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
		samplerCreateInfo.maxLod = (float)m_Texture.MipMapLevels;
		samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		m_Texture.Sampler = deviceObj->LogicalDevice.createSampler(samplerCreateInfo);

		vk::ImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.viewType = vk::ImageViewType::e2D;
		viewCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
		viewCreateInfo.components.r = vk::ComponentSwizzle::eR;
		viewCreateInfo.components.g = vk::ComponentSwizzle::eG;
		viewCreateInfo.components.b = vk::ComponentSwizzle::eB;
		viewCreateInfo.components.a = vk::ComponentSwizzle::eA;
		viewCreateInfo.subresourceRange = subresourceRange;
		viewCreateInfo.subresourceRange.levelCount = m_Texture.MipMapLevels;
		viewCreateInfo.image = m_Texture.Image;

		m_Texture.View = deviceObj->LogicalDevice.createImageView(viewCreateInfo);

		m_Texture.DescriptorInfo.imageView = m_Texture.View;
		m_Texture.DescriptorInfo.sampler   = m_Texture.Sampler;
		m_Texture.DescriptorInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		s_TexID++;
	}

	void Texture::SetData(void* data, uint32_t size)
	{
		void* memoryAllocation = deviceObj->LogicalDevice.mapMemory(m_Texture.Memory, 0, size);
		memcpy(memoryAllocation, data, size);
		deviceObj->LogicalDevice.unmapMemory(m_Texture.Memory);

		CommandBufferMgr::allocCommandBuffer(&deviceObj->LogicalDevice, cmdPool, &cmdBuffer);
		CommandBufferMgr::beginCommandBuffer(cmdBuffer);

		vk::ImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = m_Texture.MipMapLevels;
		subresourceRange.layerCount = 1;

		m_Texture.ImageLayout = vk::ImageLayout::eGeneral;
		Renderer::SetImageLayout(m_Texture.Image, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::ePreinitialized, m_Texture.ImageLayout, subresourceRange, cmdBuffer);

		CommandBufferMgr::endCommandBuffer(cmdBuffer);

		vk::Fence fence;
		vk::FenceCreateInfo fenceCI= {};
		fenceCI.flags = vk::FenceCreateFlags();

		fence = deviceObj->LogicalDevice.createFence(fenceCI);

		vk::SubmitInfo submitInfo = {};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		CommandBufferMgr::submitCommandBuffer(deviceObj->Queue, &cmdBuffer, &submitInfo, fence);
		deviceObj->LogicalDevice.waitForFences(1, &fence, VK_TRUE, 10000000000);
		deviceObj->LogicalDevice.destroyFence(fence);

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
		samplerCreateInfo.maxLod = (float)m_Texture.MipMapLevels;
		samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		m_Texture.Sampler = deviceObj->LogicalDevice.createSampler(samplerCreateInfo);

		vk::ImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.viewType = vk::ImageViewType::e2D;
		viewCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
		viewCreateInfo.components.r = vk::ComponentSwizzle::eR;
		viewCreateInfo.components.g = vk::ComponentSwizzle::eG;
		viewCreateInfo.components.b = vk::ComponentSwizzle::eB;
		viewCreateInfo.components.a = vk::ComponentSwizzle::eA;
		viewCreateInfo.subresourceRange = subresourceRange;
		viewCreateInfo.subresourceRange.levelCount = m_Texture.MipMapLevels;
		viewCreateInfo.image = m_Texture.Image;

		m_Texture.View = deviceObj->LogicalDevice.createImageView(viewCreateInfo);

		m_Texture.DescriptorInfo.imageView = m_Texture.View;
		m_Texture.DescriptorInfo.sampler = m_Texture.Sampler;
		m_Texture.DescriptorInfo.imageLayout = vk::ImageLayout::eGeneral;
	}

	Ref<Texture> Texture::Create(const std::string& path)
	{
		return CreateRef<Texture>(path);
	}

	Ref<Texture> Texture::Create()
	{
		return CreateRef<Texture>();
	}

	void Texture::CreateSampler()
	{

	}

	void Texture::CreateCommandPool()
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

	Texture::~Texture()
	{
	}
}