#pragma once

namespace Giang
{
	namespace Init
	{
		vk::Semaphore MakeSemaphore(vk::Device device, bool debug = true);
		vk::Fence MakeFence(vk::Device device, bool debug = true);
		std::vector<char> readFile(std::string filename, bool debug = true);
		vk::ShaderModule CreateModule(std::string filename, vk::Device device, bool debug = true);
	}
}
