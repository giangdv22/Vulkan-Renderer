#pragma once

namespace Giang
{
	class Shader
	{
	public:
		Shader();
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
		~Shader();

		void DestroyShader();

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

		static Ref<Shader> Create(const std::string& vertexSrc, const std::string& fragmentSrc);
	private:
		std::vector<char> ReadFile(const std::string& filename, bool debug = true);
		vk::ShaderModule CreateModule(const std::string& filename, vk::Device& device, bool debug = true);
	};
}