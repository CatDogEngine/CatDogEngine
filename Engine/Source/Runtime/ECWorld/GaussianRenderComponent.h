#pragma once
#include "Core/StringCrc.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Scene/VertexFormat.h"
#include "Scene/Types.h"

#include <bgfx/bgfx.h>
#include <vector>

namespace engine
{
struct SplatFileRecord {
	float center[3];
	float scale[3];
	uint32_t color;
	uint8_t i;
	uint8_t j;
	uint8_t k;
	uint8_t l;
};

struct InstanceDataVertex
{
	float m_r;
	float m_g;
	float m_b;
	float m_a;
	float m_cx;
	float m_cy;
	float m_cz;
	float m_cw;
	float m_cova_x;
	float m_cova_y;
	float m_cova_z;
	float m_cova_w;
	float m_covb_x;
	float m_covb_y;
	float m_covb_z;
	float m_covb_w;
};

struct PosVertex
{
	float m_x;
	float m_y;
};

struct GaussianSplatData {
	uint32_t			m_vertexCount = 0;
	InstanceDataVertex* m_buffer = nullptr;
	bgfx::DynamicVertexBufferHandle m_vbh = BGFX_INVALID_HANDLE;
	bgfx::InstanceDataBuffer m_idb;
};

class GaussianRenderComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("GaussianRenderComponent");
		return className;
	}

public:
	GaussianRenderComponent() = default;
	GaussianRenderComponent(const GaussianRenderComponent&) = default;
	GaussianRenderComponent& operator=(const GaussianRenderComponent&) = default;
	GaussianRenderComponent(GaussianRenderComponent&&) = default;
	GaussianRenderComponent& operator=(GaussianRenderComponent&&) = default;
	~GaussianRenderComponent() = default;;

	std::vector<InstanceDataVertex>& GetSplatFileData() { return m_splatFileData; }
	void Build();
	void SetSplatCount(size_t splatCount) { m_splatCount = splatCount; }

	void Initlayout();
	void InitlayoutInstance();

	bgfx::VertexBufferHandle& GetVBH() { return m_vbh; }
	GaussianSplatData& GetSplatData(int index) {
		if (index < 0 || index >= 2) {
			throw std::out_of_range("Index out of range");
		}
		return m_splatData[index];
	}


private:
	std::vector<InstanceDataVertex>	m_splatFileData;
	GaussianSplatData	m_splatData[2];
	size_t m_splatCount;
	bgfx::VertexLayout ms_layout;
	bgfx::VertexLayout ms_layoutInstance;

	bgfx::VertexBufferHandle m_vbh;
};

}