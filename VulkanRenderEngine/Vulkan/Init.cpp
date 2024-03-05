#include "gpch.h"
#include "Init.h"

namespace Giang
{
	namespace Init
	{
		vk::Semaphore MakeSemaphore(vk::Device device, bool debug)
		{
			vk::SemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.flags = vk::SemaphoreCreateFlags();

			try
			{
				return device.createSemaphore(semaphoreInfo);
			}
			catch (vk::SystemError err)
			{
				if (debug)
				{
					std::cout << "Failed to create semaphore" << std::endl;
				}
				return nullptr;
			}
		}
		vk::Fence MakeFence(vk::Device device, bool debug)
		{
			vk::FenceCreateInfo fenceInfo = {};
			fenceInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;

			try
			{
				return device.createFence(fenceInfo);
			}
			catch (vk::SystemError err)
			{
				if (debug)
				{
					std::cout << "Failed to create fence" << std::endl;
				}
				return nullptr;
			}
		}
		std::vector<char> readFile(std::string filename, bool debug)
		{
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (debug && !file.is_open())
			{
				std::cout << "Failed to load \"" << filename << "\"" << std::endl;
			}

			size_t fileSize = static_cast<size_t>(file.tellg());

			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);

			file.close();
			return buffer;
		}
		vk::ShaderModule CreateModule(std::string filename, vk::Device device, bool debug)
		{
			std::vector<char> sourceCode = readFile(filename, debug);
			vk::ShaderModuleCreateInfo moduleInfo = {};
			moduleInfo.flags = vk::ShaderModuleCreateFlags();
			moduleInfo.codeSize = sourceCode.size();
			moduleInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());

			try {
				return device.createShaderModule(moduleInfo);
			}
			catch (vk::SystemError err)
			{
				if (debug)
				{
					std::cout << "Failed to create shader module for \"" << filename << "\"";
				}
			}
		}
	}
}