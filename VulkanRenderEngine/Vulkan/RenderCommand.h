#pragma once
#include "VertexInputDescription.h"

namespace Giang
{
	class RenderCommand
	{
	public:
		static void DrawIndexed(const Ref<VertexInputDescription>& desc, uint32_t indexCount = 0)
		{
			uint32_t count = indexCount ? indexCount : desc->GetIndexBuffer()->GetCount();
		}
	};

}