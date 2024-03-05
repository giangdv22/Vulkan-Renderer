#include "gpch.h"
#include "Drawable2D.h"
#include "Drawable.h"
#include "Application.h"
namespace Giang
{
	struct QuadVertex
	{
		glm::vec4 Position;
		glm::vec4 Color;
	};

	struct Drawable2DData
	{
		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;

		Ref<VertexInputDescription> QuadViDescription;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> QuadShader;
		Ref<Drawable> QuadDrawable;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		glm::vec4 QuadVertexPositions[4]{
			{ -0.5,  0.5, 0.0, 1.0f},
			{  0.5,  0.5, 0.0, 1.0f},
			{  0.5, -0.5, 0.0, 1.0f},
			{ -0.5, -0.5, 0.0, 1.0f}
		};
	};

	static Drawable2DData s_Data;

	void Drawable2D::Init()
	{
		s_Data.QuadViDescription = VertexInputDescription::Create();
		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
		s_Data.QuadVertexBuffer->SetLayout(
			{
				{ShaderDataType::Float4 },
				{ShaderDataType::Float4 }
			});
		s_Data.QuadViDescription->SetVertexBuffer(s_Data.QuadVertexBuffer);

		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];
		uint32_t offset = 0;

		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
		s_Data.QuadViDescription->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		s_Data.QuadShader = Shader::Create("QuadVertex.spv", "QuadFragment.spv");
		s_Data.QuadDrawable = Drawable::Create(Application::GetInstance()->RendererObj);
		s_Data.QuadDrawable->SetVertexInputDescription(s_Data.QuadViDescription);
		s_Data.QuadDrawable->CreateDescriptorSetLayout(false);
		s_Data.QuadDrawable->CreateDescriptor(false);
	}

	void Drawable2D::Shutdown()
	{
		delete[] s_Data.QuadVertexBufferBase;
	}

	void Drawable2D::BeginScene()
	{
		StartBatch();
	}
	void Drawable2D::EndScene()
	{
		Flush();
	}
	void Drawable2D::StartBatch()
	{
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
	}

	void Drawable2D::Flush()
	{
		if (s_Data.QuadIndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
			s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		}
	}
	void Drawable2D::DrawQuad(const glm::vec4& position, const glm::vec4& color)
	{
		constexpr size_t quadVertexCount = 4;
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), { position.x, position.y, position.z });
		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
	}
}