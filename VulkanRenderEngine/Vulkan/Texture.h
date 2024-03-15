#pragma once


namespace Giang
{
	class Device;

	enum class ImageFormat
	{
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RGBA32F
	};

	struct TextureSpecification
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		ImageFormat Format = ImageFormat::RGBA8;
	};


	struct TextureData
	{
		vk::Sampler Sampler = {};
		vk::Image Image = {};
		vk::ImageLayout ImageLayout = {};
		vk::MemoryAllocateInfo MemoryAlloc = {};
		vk::DeviceMemory Memory = {};
		vk::ImageView View = {};
		uint32_t MipMapLevels = {};
		uint32_t LayerCount = 1;
		uint32_t Width = 0;
		uint32_t Height = 0;
		vk::DescriptorImageInfo DescriptorInfo = {};

	};

	class Texture
	{
	public:
		Texture();
		Texture(const std::string& path);
		~Texture();

		const uint32_t GetWidth() const { return m_Texture.Width; }
		const uint32_t GetHeight() const { return m_Texture.Height; }

		const std::string& GetPath() const { return m_Path; }
		TextureData& GetTextureData() { return m_Texture; }

		void Bind();
		void SetData(void* data, uint32_t size);


		static Ref<Texture> Create(const std::string& path);
		static Ref<Texture> Create();

		bool operator == (Texture& other) const
		{
			return m_TexID == other.m_TexID;
		}
	private:
		void CreateSampler();
		void CreateCommandPool();
	private:
		Device* deviceObj;
		vk::CommandBuffer cmdBuffer;
		TextureData m_Texture;

		std::string m_Path;

		uint32_t m_TexID = 0;


		/*vk::Buffer m_Buffer;
		vk::DeviceMemory m_DeviceMemory;
		vk::MemoryRequirements m_MemoryRequirements;*/

	};
}