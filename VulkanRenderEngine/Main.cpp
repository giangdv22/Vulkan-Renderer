#include "gpch.h"
#include "Vulkan/Application.h"
int main()
{
	Giang::Application* App = Giang::Application::GetInstance();
	App->Initialize();
	App->Prepare();
	App->Run();
	//App->Deinitialize();
	delete App;
	return 0;
}