#pragma once


namespace Giang
{
	class Device;
	class Descriptor
	{
	public:
		Descriptor();
		~Descriptor();

		void CreateDescriptor(bool useTexture);
		void DestroyDescriptor();

		virtual void CreateDescriptorSetLayout(bool useTexture) = 0;
		void DestroyDescriptorSetLayout();

		virtual void CreateDescriptorPool(bool useTexture) = 0;
		void DestroyDescriptorPool();

		virtual void CreateDescriptorResources() = 0;

		virtual void CreateDescriptorSet(bool useTexture) = 0;
		void DestroyDescriptorSet();

		virtual void CreatePipelineLayout() = 0;
		void DestroyPipelineLayouts();

	public:
		vk::PipelineLayout PipelineLayout;
		std::vector<vk::DescriptorSetLayout> DescriptorLayouts;
		vk::DescriptorPool DescriptorPool;
		std::vector<vk::DescriptorSet> descriptorSets;
		Device* deviceObj;
	};

}