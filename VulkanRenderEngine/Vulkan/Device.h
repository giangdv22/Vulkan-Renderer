#pragma once

#include "Instance.h"

namespace Giang
{
	class Device
	{
	public:
		Device();
		~Device();

		vk::Device LogicalDevice = nullptr;
		vk::PhysicalDevice Gpu;
		vk::PhysicalDeviceMemoryProperties MemoryProperties;

	public:
		vk::Queue Queue = nullptr;
		std::vector<vk::QueueFamilyProperties> QueueFamilyProps;
		std::optional<uint32_t>							   GraphicsQueueIndex;
		std::optional<uint32_t>							   graphicsQueueWithPresentIndex;
		uint32_t							   queueFamilyCount;

	public:
		void ChoosePhysicalDevice(Instance& instance);
		void CreateLogicalDevice();
		void DestroyLogicalDevice();
		void GetGraphicsQueueHandle();
		void GetDeviceQueue();
		uint32_t MemoryTypeFromProperties(uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties);
	private:
		uint32_t GetQueueFamilyCount(vk::PhysicalDevice device);
		void LogDeviceProperties(vk::PhysicalDevice& device);
		bool CheckDeviceExtensionSupport(const vk::PhysicalDevice& device, const std::vector<const char*>& requestedExtensions);
		bool IsSuitable(const vk::PhysicalDevice& device);


	};

}