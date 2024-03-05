#include "gpch.h"
#include "Pipeline.h"

#include "Application.h"
#include "Shader.h"
#include "Init.h"
namespace Giang
{
	Pipeline::Pipeline()
	{
		appObj = Application::GetInstance();
		deviceObj = appObj->DeviceObj;
	}

	Pipeline::~Pipeline()
	{
		DestroyPipelineCache();
	}

	void Pipeline::CreatePipelineCache()
	{
		vk::PipelineCacheCreateInfo createInfo = {};
		createInfo.flags = vk::PipelineCacheCreateFlags();
		createInfo.initialDataSize = 0;
		createInfo.pInitialData = NULL;

		try {
			pipelineCache = deviceObj->LogicalDevice.createPipelineCache(createInfo);
		}
		catch (vk::SystemError)
		{
			std::cout << "Failed to create pipeline cache!\n";
		}

	}
	bool Pipeline::CreatePipeline(Drawable* drawableObj, vk::Pipeline* pipeline, Shader* shaderObj, vk::Bool32 includeDepth, vk::Bool32 includeVi)
	{
		std::vector<vk::DynamicState> dynamicStateEnables;
		//dynamicStateEnables.clear();

		vk::PipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.flags = vk::PipelineDynamicStateCreateFlags();
		dynamicState.dynamicStateCount = 0;
		dynamicState.pDynamicStates = NULL;

		vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
		vertexInputStateInfo.flags = vk::PipelineVertexInputStateCreateFlags();
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = &drawableObj->GetVertexInputDescription()->GetBinding();
		vertexInputStateInfo.vertexAttributeDescriptionCount = drawableObj->GetVertexInputDescription()->GetAttributes().size();
		vertexInputStateInfo.pVertexAttributeDescriptions = drawableObj->GetVertexInputDescription()->GetAttributes().data();

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
		// Primitive restart are applicable to only index geometries and used with
		// (vkCmdDrawIndexed and vkCmdDrawIndexedIndirect) drawing APIs
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
		inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;

		vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo = {};
		rasterizationStateInfo.flags = vk::PipelineRasterizationStateCreateFlags();
		rasterizationStateInfo.polygonMode = vk::PolygonMode::eFill;
		rasterizationStateInfo.cullMode = vk::CullModeFlagBits::eBack;
		rasterizationStateInfo.frontFace = vk::FrontFace::eClockwise;
		rasterizationStateInfo.depthClampEnable = includeDepth;
		rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationStateInfo.depthBiasEnable = VK_FALSE;
		rasterizationStateInfo.depthBiasConstantFactor = 0;
		rasterizationStateInfo.depthBiasClamp = 0;
		rasterizationStateInfo.depthBiasSlopeFactor = 0;
		rasterizationStateInfo.lineWidth = 1.0f;

		vk::PipelineColorBlendAttachmentState colorBlendAttachmentStateInfo[1] = {};
		colorBlendAttachmentStateInfo[0].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		colorBlendAttachmentStateInfo[0].blendEnable = VK_FALSE;
		colorBlendAttachmentStateInfo[0].alphaBlendOp = vk::BlendOp::eAdd;
		colorBlendAttachmentStateInfo[0].colorBlendOp = vk::BlendOp::eAdd;
		colorBlendAttachmentStateInfo[0].srcColorBlendFactor = vk::BlendFactor::eZero;
		colorBlendAttachmentStateInfo[0].dstColorBlendFactor = vk::BlendFactor::eZero;
		colorBlendAttachmentStateInfo[0].srcAlphaBlendFactor = vk::BlendFactor::eZero;
		colorBlendAttachmentStateInfo[0].dstAlphaBlendFactor = vk::BlendFactor::eZero;

		vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo = {};
		colorBlendStateInfo.flags = vk::PipelineColorBlendStateCreateFlags();
		colorBlendStateInfo.logicOpEnable = VK_FALSE;
		colorBlendStateInfo.logicOp = vk::LogicOp::eNoOp;
		colorBlendStateInfo.attachmentCount = 1;
		colorBlendStateInfo.pAttachments = colorBlendAttachmentStateInfo;
		colorBlendStateInfo.blendConstants[0] = 1.0f;
		colorBlendStateInfo.blendConstants[1] = 1.0f;
		colorBlendStateInfo.blendConstants[2] = 1.0f;
		colorBlendStateInfo.blendConstants[3] = 1.0f;

		vk::Viewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = appObj->RendererObj->width;
		viewport.height = appObj->RendererObj->height;

		vk::Rect2D scissor = {};
		scissor.offset.x = 0.0f;
		scissor.offset.y = 0.0f;
		scissor.extent.width = appObj->RendererObj->width;
		scissor.extent.height = appObj->RendererObj->height;

		vk::PipelineViewportStateCreateInfo viewportStateInfo = {};
		viewportStateInfo.flags = vk::PipelineViewportStateCreateFlags();
		viewportStateInfo.viewportCount = 1;
		viewportStateInfo.scissorCount = 1;
		viewportStateInfo.pScissors = NULL;
		viewportStateInfo.pViewports = NULL;

		dynamicStateEnables.push_back(vk::DynamicState::eViewport);
		dynamicStateEnables.push_back(vk::DynamicState::eScissor);
		dynamicState.dynamicStateCount = dynamicStateEnables.size();
		dynamicState.pDynamicStates = &dynamicStateEnables[0];

		vk::PipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
		depthStencilStateInfo.flags = vk::PipelineDepthStencilStateCreateFlags();
		depthStencilStateInfo.depthTestEnable = includeDepth;
		depthStencilStateInfo.depthWriteEnable = includeDepth;
		depthStencilStateInfo.depthCompareOp = vk::CompareOp::eLessOrEqual;
		depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilStateInfo.stencilTestEnable = VK_FALSE;
		depthStencilStateInfo.back.failOp = vk::StencilOp::eKeep;
		depthStencilStateInfo.back.passOp = vk::StencilOp::eKeep;
		depthStencilStateInfo.back.compareOp = vk::CompareOp::eAlways;
		depthStencilStateInfo.back.compareMask = 0;
		depthStencilStateInfo.back.reference = 0;
		depthStencilStateInfo.back.depthFailOp = vk::StencilOp::eKeep;
		depthStencilStateInfo.back.writeMask = 0;
		depthStencilStateInfo.minDepthBounds = 0;
		depthStencilStateInfo.maxDepthBounds = 0;
		depthStencilStateInfo.stencilTestEnable = VK_FALSE;
		depthStencilStateInfo.front = depthStencilStateInfo.back;

		vk::PipelineMultisampleStateCreateInfo multiSampleStateInfo = {};
		multiSampleStateInfo.flags = vk::PipelineMultisampleStateCreateFlags();
		multiSampleStateInfo.sampleShadingEnable = VK_FALSE;
		multiSampleStateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;

		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo.flags = vk::PipelineLayoutCreateFlags();
		vk::PipelineLayout layout = deviceObj->LogicalDevice.createPipelineLayout(layoutInfo);

		// This structure specify programmable stages, fixed-function pipeline stages
		//  render pass, sub-passes and pipeline layouts.
		vk::GraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.flags = vk::PipelineCreateFlags();
		pipelineInfo.layout = drawableObj->PipelineLayout;
		pipelineInfo.basePipelineHandle = vk::Pipeline();
		pipelineInfo.basePipelineIndex = 0;
		pipelineInfo.pVertexInputState = &vertexInputStateInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pRasterizationState = &rasterizationStateInfo;
		pipelineInfo.pColorBlendState = &colorBlendStateInfo;
		pipelineInfo.pTessellationState = NULL;
		pipelineInfo.pMultisampleState = &multiSampleStateInfo;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.pViewportState = &viewportStateInfo;
		pipelineInfo.pDepthStencilState = &depthStencilStateInfo;
		pipelineInfo.stageCount = shaderObj->shaderStages.size();
		pipelineInfo.pStages = shaderObj->shaderStages.data();
		pipelineInfo.renderPass = appObj->RendererObj->RenderPass;
		pipelineInfo.subpass = 0;

		try {
			*pipeline = (deviceObj->LogicalDevice.createGraphicsPipeline(pipelineCache, pipelineInfo)).value;

			return true;
		}
		catch (vk::SystemError err)
		{
			std::cout << "Failed to create Graphics Pipeline!" << std::endl;
			return false;
		}
	}
	void Pipeline::DestroyPipelineCache()
	{
		deviceObj->LogicalDevice.destroyPipelineCache(pipelineCache);
		//deviceObj->LogicalDevice.destroypipe
	}
}