#include "gpch.h"
#include "Device.h"
namespace Giang
{
	Device::Device()
	{
		queueFamilyCount			  = 0;
	}

	Device::~Device()
	{
		DestroyLogicalDevice();
	}

	void Device::ChoosePhysicalDevice(Instance& instance)
	{
		std::cout << "Choosing physical device...\n";

		std::vector <vk::PhysicalDevice> availableDevices = instance.GetInstance().enumeratePhysicalDevices();

		std::cout << "There are " << availableDevices.size() << " physical devices\n";

		int deviceIndex = -1;
		uint32_t queueCount = 0;
		for (int i = 0; i < availableDevices.size(); i++)
		{
			LogDeviceProperties(availableDevices[i]);
			GetQueueFamilyCount(availableDevices[i]);
			if (IsSuitable(availableDevices[i]) && GetQueueFamilyCount(availableDevices[i]) > queueCount)
			{
				queueCount = GetQueueFamilyCount(availableDevices[i]);
				deviceIndex = i;
			}
		}

		Gpu = availableDevices[deviceIndex];
		queueFamilyCount = queueCount;
		QueueFamilyProps = Gpu.getQueueFamilyProperties();
		GetGraphicsQueueHandle();
	}

	void Device::CreateLogicalDevice()
	{
		float queuePriority[1] = {1.0f};
		vk::DeviceQueueCreateInfo queueCreateInfo = vk::DeviceQueueCreateInfo(
			vk::DeviceQueueCreateFlags(), GraphicsQueueIndex.value(),
			queuePriority
		);

		deviceFeatures = vk::PhysicalDeviceFeatures();
		deviceFeatures.depthClamp = true;
		vk::PhysicalDeviceFeatures setEnabledFeatures = { VK_FALSE };
		setEnabledFeatures.samplerAnisotropy = deviceFeatures.samplerAnisotropy;
		setEnabledFeatures.depthClamp = deviceFeatures.depthClamp;
		std::vector<const char*> enabledLayers;
		enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
		std::vector<const char*> enabledExtensions;
		enabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			1, &queueCreateInfo,
			enabledLayers.size(), enabledLayers.data(),
			enabledExtensions.size(), enabledExtensions.data(),
			&setEnabledFeatures
		);

		try
		{
			LogicalDevice = Gpu.createDevice(deviceInfo);

			std::cout << "GPU has been successfully abstracted!\n";
		}
		catch (vk::SystemError err)
		{
			std::cout << "Logical device creation failed!\n";
		}
	}

	void Device::DestroyLogicalDevice()
	{
		LogicalDevice.destroy();
	}

	void Device::LogDeviceProperties(vk::PhysicalDevice& device)
	{
		vk::PhysicalDeviceProperties properties = device.getProperties();

		std::cout << "Device name: " << properties.deviceName << "\n";

		std::cout << "Device type: ";
		switch (properties.deviceType)
		{
		case(vk::PhysicalDeviceType::eCpu):
			std::cout << "CPU\n";
			break;

		case(vk::PhysicalDeviceType::eDiscreteGpu):
			std::cout << "Discrete GPU\n";
			break;

		case(vk::PhysicalDeviceType::eIntegratedGpu):
			std::cout << "Integrated GPU\n";
			break;

		case(vk::PhysicalDeviceType::eVirtualGpu):
			std::cout << "Virtual GPU\n";
			break;

		default:
			std::cout << "Other Device\n";
			break;
		}
	}

	uint32_t Device::MemoryTypeFromProperties(uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties)
	{
		vk::PhysicalDeviceMemoryProperties memoryProperties = Gpu.getMemoryProperties();

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			bool supported = static_cast<bool>(supportedMemoryIndices & (1 << i));
			bool sufficent = (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties;

			if (supported && sufficent)
			{
				return i;
			}
		}
	}

	bool Device::CheckDeviceExtensionSupport(const vk::PhysicalDevice& device, const std::vector<const char*>& requestedExtensions)
	{
		std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());

		//std::cout << "Device can support extensions:\n";
		for (vk::ExtensionProperties& extension : device.enumerateDeviceExtensionProperties())
		{
			//std::cout << "\t\"" << extension.extensionName << "\"\n";

			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();
	}

	bool Device::IsSuitable(const vk::PhysicalDevice& device)
	{
		std::cout << "Checking if device is suitable\n";

		const std::vector<const char*> requestedExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		std::cout << "We are requesting device extensions:\n";
		for (const char* extension : requestedExtensions)
		{
			std::cout << "\t\"" << extension << "\"\n";
		}

		if (bool extensionSupported = CheckDeviceExtensionSupport(device, requestedExtensions))
		{
			std::cout << "Device can support the requested extensions!\n";
		}
		else
		{
			std::cout << "Device can't support the requested extensions!\n";
			return false;
		}

		return true;
	}

	void Device::GetDeviceQueue()
	{
		Queue = LogicalDevice.getQueue(graphicsQueueWithPresentIndex.value(), 0);
	}

	void Device::GetGraphicsQueueHandle()
	{
		bool found = false;
		for (uint32_t i = 0; i < queueFamilyCount; i++)
		{
			if (QueueFamilyProps[i].queueFlags & vk::QueueFlagBits::eGraphics)
			{
				found = true;
				GraphicsQueueIndex = i;
				break;
			}
		}

		assert(found);

	}

	uint32_t Device::GetQueueFamilyCount(vk::PhysicalDevice device)
	{
		std::vector<vk::QueueFamilyProperties> queueProps = device.getQueueFamilyProperties();

		return static_cast<uint32_t>(queueProps.size());
	}

}