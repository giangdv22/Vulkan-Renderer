#pragma once
namespace Giang
{
	class Instance;
	class Device;
	class Renderer;
	class Application;

	struct SwapchainBuffer
	{
		vk::Image image;
		vk::ImageView view;
	};

	struct SwapchainPrivateVariables
	{
		vk::SurfaceCapabilitiesKHR SurfaceCapabilities;

		uint32_t PresentModeCount;
		std::vector<vk::PresentModeKHR> PresentModes;

		vk::Extent2D SwapchainExtent;
		uint32_t	DesiredNumberOfSwapchainImages;

		vk::SurfaceTransformFlagBitsKHR PreTransform;

		vk::PresentModeKHR SwapchainPresentMode;

		std::vector<vk::Image> SwapchainImages;

		std::vector<vk::SurfaceFormatKHR> SurfaceFormats;
	};

	struct SwapchainPublicVariables
	{
		vk::SurfaceKHR Surface;
		uint32_t SwapchainImageCount;

		vk::SwapchainKHR SwapChain = nullptr;

		std::vector<SwapchainBuffer> ColorBuffer;

		std::vector<vk::Semaphore> ImageAvailableSamephores;
		std::vector<vk::Semaphore> RenderFinishedSemaphores;

		std::vector<vk::Fence> fences;

		// Current drawing surface index in use
		uint32_t CurrentColorBuffer;

		vk::Format format; // Format of the image
		vk::ColorSpaceKHR colorSpace;
	};

	class Swapchain
	{
	public:
		Swapchain(Renderer* renderer);
		~Swapchain();
		void InitializeSwapchain();
		void CreateSwapchain(const vk::CommandBuffer& commandBuffer);
		void DestroySwapchain();

		void SetSwapchainExtent(uint32_t swapchainWidth, uint32_t swapchainHeight);
	public:
		SwapchainPublicVariables SwapchainPublicVars;

	private:
		void CreateSwapChainColorImages();
		void CreateColorImageView(const vk::CommandBuffer& cmdBuffer);
		void CreateSyncObject();
		void CreateSurface();

		uint32_t GetGraphicsQueueWithPresentationSupport();
		void GetSupportedFormats();
		void GetSurfaceCapabilitiesAndPresentMode();
		void ManagePresentMode();

	private:
		SwapchainPrivateVariables SwapchainPrivateVars;
		Renderer* rendererObj;
		Application* appObj;
	};
}