#pragma once
#include "Shader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexInputDescription.h"

namespace Giang
{
	class Drawable2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene();
		static void EndScene();
		static void Flush();

		static void DrawQuad(const glm::vec4& position, const glm::vec4& color);

	private:
		static void StartBatch();
		static void NextBatch();
	};
}

