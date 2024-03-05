#pragma once

namespace Giang
{
	class Shader;
	class Drawable;
	class Device;
	class Application;

	class Pipeline
	{
	public:
		Pipeline();
		~Pipeline();

		void CreatePipelineCache();

		bool CreatePipeline(Drawable* drawableObj, vk::Pipeline* pipeline, Shader* shaderObj,
			vk::Bool32 includeDepth, vk::Bool32 includeVi = true);

		// Destruct the pipeline cache object
		void DestroyPipelineCache();

	public:
		vk::PipelineCache pipelineCache;
		vk::PipelineLayout pipelineLayout;
		Application* appObj;
		Device* deviceObj;
	};
}
