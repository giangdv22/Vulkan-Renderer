#pragma once
#include "LayerAndExtension.h"

namespace Giang
{
	class Instance
	{
	public:
		Instance();
		~Instance();

		void CreateInstance(const char* applicationName);
		void DestroyInstance();
		vk::Instance& GetInstance();
		//LayerAndExtension& GetLED() { return layerExtension; }
		void CreateDebugMessenger();

		void Destroy();

	public:
		LayerAndExtension layerExtension;
	public:
		vk::Instance InstanceObj;
		vk::DebugUtilsMessengerEXT debugMessenger{ nullptr };
		vk::DispatchLoaderDynamic dldi;
	};

}