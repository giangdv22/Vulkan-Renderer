#pragma once

namespace Giang
{
	namespace Utils
	{
		std::vector<std::string> LogTransformBits(vk::SurfaceTransformFlagsKHR bits);
		std::vector<std::string> LogAlphaCompositeBits(vk::CompositeAlphaFlagsKHR bits);
		std::vector<std::string> LogImageUsageBits(vk::ImageUsageFlags bits);
		std::string LogPresentMode(vk::PresentModeKHR presentMode);

		void LogExtent(vk::SurfaceCapabilitiesKHR SurfaceCapabilities);
	}
}