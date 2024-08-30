#pragma once
#include "Core/StringCrc.h"
#include <vector>

namespace engine
{
enum ElementType
{
    None,
    Float,
    Double,
    UChar
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
	~GaussianRenderComponent() = default;

	void Build();

	uint16_t& GetVertexBufferHandle() { return m_vertexBufferHandle; }
	uint16_t& GetIndexBufferHandle() { return m_indexBufferHandle; }

	std::vector<std::byte>& GetGaussianBuffer() { return m_gausianAttributesBuffer; }

	// ply
	void SetPlyData(std::vector<std::byte> buffer) { m_readBuffer = std::move(buffer); }
	void ProcessingPlyBuffer();
	void GenerateTexture();

	uint32_t& GetVertexCount() { return m_vertextCount; }
	uint16_t GetGaussianTextureHandle() { return m_textureHandle; }

private:
	uint16_t m_vertexBufferHandle = UINT16_MAX;
	uint16_t m_indexBufferHandle = UINT16_MAX;

	// ply
	std::vector<std::byte> m_readBuffer;

	std::vector<float> m_sizeList;
	std::vector<uint32_t> m_sizeIndex;
	uint32_t m_vertextCount;

	// gaussian
	std::vector<std::byte> m_gausianAttributesBuffer;

	// texture
	std::vector<uint32_t> m_textureBuffer;
	uint16_t m_textureHandle;
};

}