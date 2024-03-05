#include "gpch.h"
#include "Shader.h"
#include "Application.h"
namespace Giang
{
    Shader::Shader()
    {
    }
    Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc)
    {
        Device* deviceObj = Application::GetInstance()->DeviceObj;

        vk::ShaderModule vertexShader = CreateModule(vertexSrc, deviceObj->LogicalDevice);
        vk::PipelineShaderStageCreateInfo vertexShaderInfo = {};
        vertexShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
        vertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertexShaderInfo.module = vertexShader;
        vertexShaderInfo.pName = "main";

        shaderStages.push_back(vertexShaderInfo);

        vk::ShaderModule fragmentShader = CreateModule(fragmentSrc, deviceObj->LogicalDevice);
        vk::PipelineShaderStageCreateInfo fragmentShaderInfo = {};
        fragmentShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
        fragmentShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
        fragmentShaderInfo.module = fragmentShader;
        fragmentShaderInfo.pName = "main";
        shaderStages.push_back(fragmentShaderInfo);
    }
    Ref<Shader> Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
    {
        return CreateRef<Shader>(vertexSrc, fragmentSrc);
    }

    Shader::~Shader()
    {
    }

    void Shader::DestroyShader()
    {
        Device* deviceObj = Application::GetInstance()->DeviceObj;
        deviceObj->LogicalDevice.destroyShaderModule(shaderStages[0].module);
        deviceObj->LogicalDevice.destroyShaderModule(shaderStages[1].module);
    }


    std::vector<char> Shader::ReadFile(const std::string& filename, bool debug)
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

    vk::ShaderModule Shader::CreateModule(const std::string& filename, vk::Device& device, bool debug)
    {
        std::vector<char> sourceCode = ReadFile(filename, debug);
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