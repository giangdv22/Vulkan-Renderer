#include "gpch.h"
#include "Application.h"

namespace Giang
{
	std::unique_ptr<Application> Application::s_InstanceApp;

	Application::Application()
	{
		DeviceObj = NULL;
		RendererObj = NULL;

		isPrepared = false;
		isResizing = false;
	}


	Application* Application::GetInstance()
	{
		if (s_InstanceApp == nullptr)
		{
			s_InstanceApp.reset(new Application());
		}
		return s_InstanceApp.get();
	}

	void Application::Run()
	{
		float time = glfwGetTime();
		Timestep timestep = time - m_LastFrameTime;
		m_LastFrameTime = time;

		while (!glfwWindowShouldClose(m_Window))
		{
			glfwPollEvents();

			Update(timestep);
			Render();

		}
	}

	void Application::Initialize()
	{
		InstanceObj.CreateDebugMessenger();

		// Device
		DeviceObj = new Device();
		DeviceObj->ChoosePhysicalDevice(InstanceObj);
		DeviceObj->CreateLogicalDevice();

		//Renderer
		if (!RendererObj)
		{
			RendererObj = new Renderer(this, DeviceObj);
			RendererObj->CreatePresentationWindow(1280, 720);
			RendererObj->GetSwapchain()->InitializeSwapchain();
		}

		RendererObj->Initialize();
	}

	void Application::Prepare()
	{
		isPrepared = false;
		RendererObj->Prepare();
		isPrepared = true;
	}

	void Application::Update(Timestep ts)
	{
		RendererObj->Update(ts);
	}

	void Application::Render()
	{
		if (!isPrepared)
			return ;

		RendererObj->Render();
	}

	void Application::Deinitialize()
	{
		RendererObj->DestroyPipeline();

		RendererObj->GetPipelineObject()->DestroyPipelineCache();

		for (Drawable* drawableObj : RendererObj->getDrawingItems())
		{
			drawableObj->DestroyDescriptor();
		}

		RendererObj->getShader()->DestroyShader();
		RendererObj->DestroyFrameBuffers();
		RendererObj->DestroyRenderpass();
		RendererObj->DestroyDrawableVertexBuffer();
		RendererObj->DestroyDrawableUniformBuffer();
		RendererObj->DestroyDepthBuffer();
		RendererObj->GetSwapchain()->DestroySwapchain();
		RendererObj->DestroyCommandBuffer();
		RendererObj->DestroyCommandPool();
		RendererObj->DestroyPresentationWindow();

		DeviceObj->DestroyLogicalDevice();
	}

	void Application::Resize()
	{
		if (!isPrepared) {
			return;
		}

		isResizing = true;

		DeviceObj->LogicalDevice.waitIdle();

		RendererObj->DestroyFrameBuffers();
		RendererObj->DestroyCommandPool();
		RendererObj->DestroyPipeline();
		RendererObj->GetPipelineObject()->DestroyPipelineCache();
		RendererObj->DestroyRenderpass();
		RendererObj->GetSwapchain()->DestroySwapchain();
		RendererObj->DestroyDrawableVertexBuffer();
		RendererObj->DestroyDepthBuffer();

		RendererObj->Initialize();
		Prepare();

		isResizing = false;
	}

	void Application::CalculateFrameRate()
	{
		float time = glfwGetTime();
		Timestep timestep = time - m_LastFrameTime;
		m_LastFrameTime = time;

	}

	Application::~Application()
	{
		delete RendererObj;
		DeviceObj->DestroyLogicalDevice();
		//InstanceObj.Destroy();
		s_InstanceApp.release();
	}

}