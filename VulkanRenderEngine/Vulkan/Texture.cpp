#include "gpch.h"
#include "Texture.h"
#include "Application.h"
namespace Giang
{
	Texture::Texture(const std::string& path)
	{
		deviceObj = Application::GetInstance()->DeviceObj;

		vk::ImageCreateInfo createInfo = {};
		createInfo.flags = vk::ImageCreateFlags();
		createInfo.imageType = vk::ImageType::e2D;
		createInfo.extent = vk::Extent3D(texture.Width, texture.Height, 1);
		createInfo.mipLevels = texture.MipMapLevels;
		createInfo.arrayLayers = 1;
		createInfo.samples = vk::SampleCountFlagBits::e1;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = NULL;
		createInfo.sharingMode = vk::SharingMode::eExclusive;
		createInfo.usage = vk::ImageUsageFlagBits::eSampled;
		createInfo.initialLayout = vk::ImageLayout::ePreinitialized;
		createInfo.tiling = vk::ImageTiling::eOptimal;

		try {
			texture.Image = deviceObj->LogicalDevice.createImage(createInfo);
		}
		catch (vk::SystemError err)
		{
			std::cout << "Unable to make texture!\n";
		}
	}
}