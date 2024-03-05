#pragma once


namespace Giang
{
	class Device;

	struct TextureData
	{
		vk::Sampler Sampler;
		vk::Image Image;
		vk::ImageLayout ImageLayout;
		vk::MemoryAllocateInfo MemoryAlloc;
		vk::DeviceMemory Memory;
		vk::ImageView View;
		uint32_t MipMapLevels = 1;
		uint32_t LayerCount;
		uint32_t Width;
		uint32_t Height;
		vk::DescriptorImageInfo DescriptorInfo;
	};

	class Texture
	{
	public:
		Texture(const std::string& path);
		~Texture();

		const uint32_t GetWidth() const { return texture.Width; }
		const uint32_t GetHeight() const { return texture.Height; }

	private:
		Device* deviceObj;
		TextureData texture;
	};
}