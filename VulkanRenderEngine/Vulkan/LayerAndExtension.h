#pragma once

namespace Giang
{
	class LayerAndExtension
	{
	public:
		LayerAndExtension();
		~LayerAndExtension();


		bool AreSupportedExtensions(std::vector<const char*>& extensions);
		bool AreSupportedLayers(std::vector<const char*>& layers);

		vk::DebugUtilsMessengerEXT MakeDebugMessenger(vk::Instance& instance, vk::DispatchLoaderDynamic& dldi);
	private:
		std::vector<vk::ExtensionProperties> supportedExtensions;
		std::vector<vk::LayerProperties> supportedLayers;

	public:
		void GetInstanceLayerProperties();
		void GetExtensionProperties();
	};

}