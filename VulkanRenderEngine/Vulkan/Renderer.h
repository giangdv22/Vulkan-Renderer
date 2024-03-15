#pragma once
#include "Swapchain.h"
#include "Application.h"
#include "Drawable.h"
#include "Shader.h"
#include "Pipeline.h"
namespace Giang
{
	class EditorCamera;
	class Renderer
	{
	public:
		Renderer(Application* app, Device* deviceObject);
		~Renderer();

		void Initialize();
		void Prepare();
		void Render();
		void Update(Timestep ts);

		static void SetImageLayout(vk::Image image, vk::ImageAspectFlags aspectMask, vk::ImageLayout oldImageLayout, vk::ImageLayout newLayout,
			vk::ImageSubresourceRange subresourceRange, vk::CommandBuffer& commandBuffer);

		Device* GetDevice() { return deviceObj; }
		Swapchain* GetSwapchain() { return swapchainObj; }
		Pipeline* GetPipelineObject() { return &pipelineObj; }
		std::vector<Drawable*> getDrawingItems() { return drawables; }
		Shader* getShader() { return shaderObj.get(); }
		vk::CommandPool* GetCommandPool() { return &cmdPool; }

		void SetWidth(int width) { width = width; }
		void SetHeight(int height) { height = height; }

		void BuildSwapchainAndDepthImage();

		void CreatePresentationWindow(const int& windowWidth, const int& windowHeight);
		void CreateDepthImage();
		void createCommandPool(); // Create command pool
		void CreateRenderpass(bool includeDepth, bool clear = true);
		void CreateFrameBuffer(bool includeDepth, bool clear = true);
		void CreateVertexBuffer();
		void CreateTexture();
		void CreateShader();
		void CreatePipelineStateManagement();
		void CreateDescriptors();
		void CreateSampler(Ref<Texture>& texture);
		void CreateImageView(Ref<Texture>& texture);
		void CreateTextureOptimal(const char* filename, TextureData* texture, vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eSampled, vk::Format format = vk::Format::eR8G8B8A8Unorm);

		void DestroyRenderpass();
		void DestroyFrameBuffers();
		void DestroyDepthBuffer();
		void DestroyDrawableVertexBuffer();
		void DestroyDrawableUniformBuffer();
		void DestroyPipeline();
		void DestroyCommandBuffer();
		void DestroyCommandPool();
		void DestroyPresentationWindow();

		// Renderer 2D
	private:
		static void Init2DData();
	public:
		static void DrawQuad(const glm::vec3& position, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const Ref<Texture>& texture, const glm::vec4 tintColor = glm::vec4(1.0f));

	public:
		// Data structure used for depth image
		struct {
			vk::Format format;
			vk::Image image;
			vk::DeviceMemory deviceMemory;
			vk::ImageView view;
		}Depth;

		vk::CommandBuffer cmdDepthImage, cmdVertexBuffer, cmdTexture;
		vk::CommandPool cmdPool;

		int width, height;

		vk::RenderPass RenderPass;

		std::vector<vk::Framebuffer> Framebuffers;
		std::vector<vk::Pipeline*> Pipelines;
	private:
		Swapchain* swapchainObj;
		Application* application;
		Device* deviceObj;

		std::vector<Drawable*> drawables;
		Ref<Shader> shaderObj;
		Pipeline pipelineObj;


	};
}
