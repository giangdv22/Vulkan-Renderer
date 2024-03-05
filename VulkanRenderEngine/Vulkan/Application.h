#pragma once
#include "Instance.h"
#include "Device.h"
#include "Renderer.h"
#include "Timestep.h"

#include <glfw3.h>


namespace Giang
{
	class Application
	{
	private:
		Application();
	public:
		~Application();

	private:
		static std::unique_ptr<Application> s_InstanceApp;

	public:
		static Application* GetInstance();

		void Run();

		void Initialize();
		void Prepare();
		void Update(Timestep ts);
		void Render();
		void Deinitialize();
		void Resize();

		void CalculateFrameRate();
	public:
		Instance InstanceObj;
		Device*	 DeviceObj;
		Renderer* RendererObj;

		GLFWwindow* m_Window;

	private:
		bool isPrepared;
		bool isResizing;

		float m_LastFrameTime;
	};
}