#pragma once

namespace Giang
{
	class CommandBufferMgr
	{
	public:
		static void allocCommandBuffer(const vk::Device* device, const vk::CommandPool& cmdPool, vk::CommandBuffer* cmdBuf, const vk::CommandBufferAllocateInfo *commandBufferInfo = NULL);
		static void beginCommandBuffer(vk::CommandBuffer& cmdBuf, vk::CommandBufferBeginInfo* inCmdBufInfo = NULL);
		static void endCommandBuffer(vk::CommandBuffer& cmdBuf);
		static void submitCommandBuffer(const vk::Queue& queue, const vk::CommandBuffer* commandBuffer, const vk::SubmitInfo* inSubmitInfo = NULL, const vk::Fence& fence = VK_NULL_HANDLE);
	};

}