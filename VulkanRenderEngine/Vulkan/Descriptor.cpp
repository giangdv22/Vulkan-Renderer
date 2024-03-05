#include "gpch.h"
#include "Descriptor.h"
#include "Application.h"

namespace Giang
{
	Descriptor::Descriptor()
	{
		deviceObj = Application::GetInstance()->DeviceObj;
	}

	void Descriptor::CreateDescriptor(bool useTexture)
	{
		CreateDescriptorResources();
		CreateDescriptorPool(useTexture);
		CreateDescriptorSet(useTexture);

	}

	void Descriptor::DestroyDescriptorSetLayout()
	{
		for (int i = 0; i < DescriptorLayouts.size(); i++)
		{
			deviceObj->LogicalDevice.destroyDescriptorSetLayout(DescriptorLayouts[i]);
		}
		DescriptorLayouts.clear();
	}

	void Descriptor::DestroyDescriptorPool()
	{
		deviceObj->LogicalDevice.destroyDescriptorPool(DescriptorPool);
	}

	void Descriptor::DestroyDescriptorSet()
	{
		deviceObj->LogicalDevice.freeDescriptorSets(DescriptorPool, descriptorSets);
	}

	void Descriptor::DestroyPipelineLayouts()
	{
		deviceObj->LogicalDevice.destroyPipelineLayout(PipelineLayout);
	}

	void Descriptor::DestroyDescriptor()
	{
		DestroyDescriptorSetLayout();
		DestroyPipelineLayouts();
		DestroyDescriptorSet();
		DestroyDescriptorPool();
	}

	Descriptor::~Descriptor()
	{

	}
}