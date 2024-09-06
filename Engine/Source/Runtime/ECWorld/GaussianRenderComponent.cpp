#include "GaussianRenderComponent.h"


namespace engine
{
	void GaussianRenderComponent::Initlayout()
	{
		ms_layout
			.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.end();
	}

	void GaussianRenderComponent::InitlayoutInstance()
	{
		ms_layoutInstance
			.begin()
			.add(bgfx::Attrib::TexCoord7, 4, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord6, 4, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord5, 4, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord4, 4, bgfx::AttribType::Float)
			.end();
	}

	void GaussianRenderComponent::Build()
	{
		for (int i = 0; i < 2; i++) {
			m_splatData[i].m_vertexCount = static_cast<uint32_t>(m_splatCount);
			m_splatData[i].m_buffer = new InstanceDataVertex[m_splatCount];
			memset(m_splatData[i].m_buffer, 0, sizeof(InstanceDataVertex) * m_splatCount);
			m_splatData[i].m_vbh = bgfx::createDynamicVertexBuffer(bgfx::makeRef(m_splatData[i].m_buffer, (uint32_t)m_splatCount * sizeof(InstanceDataVertex)), ms_layoutInstance);
		}

		const static PosVertex triangleVertices[4] = { 2, -2, -2, -2, 2, 2, -2, 2 };
		m_vbh = bgfx::createVertexBuffer(bgfx::makeRef(triangleVertices, sizeof(PosVertex) * 4), ms_layout);
	}



}