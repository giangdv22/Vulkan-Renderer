#include "gpch.h"
#include "CommandBufferMgr.h"

namespace Giang
{
	void CommandBufferMgr::allocCommandBuffer(const vk::Device* device, const vk::CommandPool& cmdPool, vk::CommandBuffer* cmdBuf, const vk::CommandBufferAllocateInfo* commandBufferInfo)
	{
		if (commandBufferInfo)
		{
			try {
				*cmdBuf = device->allocateCommandBuffers(*commandBufferInfo)[0];
			}
			catch (vk::SystemError err)
			{
				std::cout << "Failed to allocated command buffer!" << std::endl;
			}
			return;
		}

		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.commandPool = cmdPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;

		try {
			*cmdBuf = device->allocateCommandBuffers(allocInfo)[0];
		}
		catch (vk::SystemError err)
		{
			std::cout << "Failed to allocated command buffer" << std::endl;
		}
	}

	void CommandBufferMgr::beginCommandBuffer(vk::CommandBuffer& cmdBuf, vk::CommandBufferBeginInfo* inCmdBufInfo)
	{
			try
			{
				if (inCmdBufInfo)
					cmdBuf.begin(*inCmdBufInfo);
				else
				{
					vk::CommandBufferInheritanceInfo cmdBufInheritInfo = {};
					cmdBufInheritInfo.renderPass = VK_NULL_HANDLE;
					cmdBufInheritInfo.subpass = 0;
					cmdBufInheritInfo.framebuffer = VK_NULL_HANDLE;
					cmdBufInheritInfo.occlusionQueryEnable = VK_FALSE;
					cmdBufInheritInfo.queryFlags = (vk::QueryControlFlagBits)0;
					cmdBufInheritInfo.pipelineStatistics = (vk::QueryPipelineStatisticFlagBits) 0;

					vk::CommandBufferBeginInfo cmdBufInfo;
					cmdBufInfo.pInheritanceInfo = &cmdBufInheritInfo;
					cmdBuf.begin(cmdBufInfo);
				}
			}
			catch (vk::SystemError err)
			{
				std::cout << "Failed to begin recording command buffer!" << std::endl;
			}
	}

	void CommandBufferMgr::endCommandBuffer(vk::CommandBuffer& cmdBuf)
	{
		try {
			cmdBuf.end();
		}
		catch (vk::SystemError err) {

			std::cout << "Failed to record command buffer!" << std::endl;
		}
	}

	void CommandBufferMgr::submitCommandBuffer(const vk::Queue& queue, const vk::CommandBuffer* commandBuffer, const vk::SubmitInfo* inSubmitInfo, const vk::Fence& fence)
	{
		if (inSubmitInfo)
		{
			try
			{
				queue.submit(*inSubmitInfo, fence);
			}
			catch (vk::SystemError err)
			{
				std::cout << "Failed to submit command buffer\n";
			}
			return;
		}

		vk::SubmitInfo submitInfo = {};
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = NULL;
		submitInfo.pWaitDstStageMask = NULL;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = commandBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = NULL;

		try {
			queue.submit(submitInfo, fence);
		}
		catch (vk::SystemError err)
		{
			std::cout << "Failed to submit command buffer\n";
		}

		queue.waitIdle();
	}

}