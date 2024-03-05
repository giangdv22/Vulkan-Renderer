#include "gpch.h"
#include "LayerAndExtension.h"
namespace Giang
{
	LayerAndExtension::LayerAndExtension()
	{
		GetInstanceLayerProperties();
		GetExtensionProperties();
	}
	LayerAndExtension::~LayerAndExtension()
	{

	}
	void LayerAndExtension::GetInstanceLayerProperties()
	{
		supportedLayers = vk::enumerateInstanceLayerProperties();

		std::cout << "Device can support the following layers:\n";
		for (vk::LayerProperties supportedLayer : supportedLayers)
		{
			std::cout << "\t" << supportedLayer.layerName << "\n";
		}

	}

	void LayerAndExtension::GetExtensionProperties()
	{
		supportedExtensions = vk::enumerateInstanceExtensionProperties();

		std::cout << "Device can support the following extension:\n";
		for (vk::ExtensionProperties supportedExtension : supportedExtensions)
		{
			std::cout << "\t" << supportedExtension.extensionName << "\n";
		}
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "[VALIDATION LAYER]: " << pCallbackData->pMessage << std::endl;
		std::cout << "\n";

		return VK_FALSE;
	}

	bool LayerAndExtension::AreSupportedExtensions(std::vector<const char*>& extensions)
	{
		for (const char* extension : extensions)
		{
			bool found = false;
			for (vk::ExtensionProperties supportedExtension : supportedExtensions)
			{
				if (strcmp(extension, supportedExtension.extensionName) == 0)
				{
					std::cout << "Extension \"" << extension << "\" is supported!\n";
					found = true;
				}
			}

			if (!found)
			{
				std::cout << "Extension \"" << extension << "\" is not supported!\n";
				return false;
			}
		}
		return true;
	}

	bool LayerAndExtension::AreSupportedLayers(std::vector<const char*>& layers)
	{
		for (const char* layer : layers)
		{
			bool found = false;
			for (vk::LayerProperties supportedLayer : supportedLayers)
			{
				if (strcmp(layer, supportedLayer.layerName) == 0)
				{
					found = true;

					std::cout << "Layer \"" << layer << "\" is supported\n";

					break;
				}
			}
			if (!found)
			{
				std::cout << "Layer \"" << layer << "\" is not supported\n";

				return false;
			}
		}
		return true;
	}
	vk::DebugUtilsMessengerEXT LayerAndExtension::MakeDebugMessenger(vk::Instance& instance, vk::DispatchLoaderDynamic& dldi)
	{
		vk::DebugUtilsMessengerCreateInfoEXT createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
			vk::DebugUtilsMessengerCreateFlagsEXT(),
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
			DebugCallback,
			nullptr
		);

		return instance.createDebugUtilsMessengerEXT(createInfo, nullptr, dldi);
	}

}
