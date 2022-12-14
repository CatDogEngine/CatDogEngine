#pragma once

#include "MeshRenderData.h"
#include "Renderer.h"

#include <vector>

namespace engine
{

	class TerrainRenderer final : public Renderer
	{
	private:
		struct MeshHandle
		{
		public:
			bgfx::VertexBufferHandle vbh;
			bgfx::IndexBufferHandle ibh;
		};

		struct TextureHandle
		{
			bgfx::UniformHandle sampler;
			bgfx::TextureHandle texture;
		};

		struct TerrainMaterialHandle
		{
		public:
			TextureHandle baseColor;
			TextureHandle debug;
		};

	public:
		using Renderer::Renderer;

		virtual void Init() override;
		virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
		virtual void Render(float deltaTime) override;

	private:
		bgfx::ProgramHandle m_program;

		RenderDataContext m_renderDataContext;

		std::vector<MeshHandle> m_meshHandles;
		std::vector<TerrainMaterialHandle> m_materialHandles;
	};

}