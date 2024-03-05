#include "gpch.h"
#include "Instance.h"
#include <glfw3.h>
namespace Giang
{
	//static LayerAndExtension layerExtension;
	Instance::Instance()
	{
		CreateInstance("Application");
		dldi = vk::DispatchLoaderDynamic(InstanceObj, vkGetInstanceProcAddr);
		CreateDebugMessenger();
	}

	void Instance::CreateInstance(const char* applicationName)
	{
		uint32_t version{ 0 };
		vkEnumerateInstanceVersion(&version);

		std::cout << "System can support vulkan Variant: " << VK_API_VERSION_VARIANT(version)
			<< ", Major: " << VK_API_VERSION_MAJOR(version)
			<< ", Minor: " << VK_API_VERSION_MINOR(version)
			<< ", Patch: " << VK_API_VERSION_PATCH(version) << '\n';

		version &= ~(0xFFFU);

		version = VK_MAKE_API_VERSION(0, 1, 0, 0);

		vk::ApplicationInfo appInfo = vk::ApplicationInfo(
			applicationName,
			version,
			"Engine",
			version,
			version
		);

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		extensions.push_back("VK_EXT_debug_utils");
		extensions.push_back("VK_KHR_surface");
		extensions.push_back("VK_KHR_win32_surface");

		std::cout << "extensions to be requested:\n";

		for (const char* extensionName : extensions) {
			std::cout << "\t\"" << extensionName << "\"\n";
		}

		std::vector<const char*> layers;
		layers.push_back("VK_LAYER_KHRONOS_validation");

		if (!layerExtension.AreSupportedExtensions(extensions) || !layerExtension.AreSupportedLayers(layers))
		{
			return;
		}

		vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo(
			vk::InstanceCreateFlags(),
			&appInfo,
			static_cast<uint32_t>(layers.size()), layers.data(),
			static_cast<uint32_t>(extensions.size()), extensions.data()
		);

		vk::Result result = vk::createInstance(&createInfo, nullptr, &InstanceObj);
		assert(result == vk::Result::eSuccess);
	}

	void Instance::DestroyInstance()
	{
		InstanceObj.destroy();
	}

	 vk::Instance& Instance::GetInstance()
	{
		// TODO: insert return statement here
		return InstanceObj;
	}

	void Instance::CreateDebugMessenger()
	{
		debugMessenger = layerExtension.MakeDebugMessenger(InstanceObj, dldi);
	}

	void Instance::Destroy()
	{
		InstanceObj.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
		DestroyInstance();
	}

	Instance::~Instance()
	{
		Destroy();
	}
}