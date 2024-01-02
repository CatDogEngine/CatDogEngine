#pragma once

#include "Core/StringCrc.h"
#include "ECWorld/SkyComponent.h"
#include "Material/ShaderSchema.h"
#include "Scene/MaterialTextureType.h"
#include "Scene/Texture.h"

#include <cstdint>
#include <map>
#include <optional>
#include <unordered_set>
#include <vector>

namespace bgfx
{

struct Memory;

}

namespace cd
{

class Material;

}

namespace engine
{

class MaterialType;
class RenderContext;

class MaterialComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("MaterialComponent");
		return className;
	}

	using TextureBlob = std::vector<std::byte>;
	struct TextureInfo
	{
	public:
		const bgfx::Memory* data;
		uint64_t flag;
		uint32_t width;
		uint32_t height;
		uint32_t depth;
		cd::TextureFormat format;
		cd::Vec2f uvOffset;
		cd::Vec2f uvScale;
		uint16_t samplerHandle;
		uint16_t textureHandle;
		uint8_t slot;
		uint8_t mipCount;

		// TODO : Improve TextureInfo 
		cd::Vec2f& GetUVOffset() { return uvOffset; }
		const cd::Vec2f& GetUVOffset() const { return uvOffset; }
		cd::Vec2f& GetUVScale() { return uvScale; }
		const cd::Vec2f& GetUVScale() const  { return uvScale; }
		void SetUVOffset(const cd::Vec2f& offset) { uvOffset = offset; }
		void SetUVScale(const cd::Vec2f& scale) { uvScale = scale; }
	};

public:
	MaterialComponent() = default;
	MaterialComponent(const MaterialComponent&) = default;
	MaterialComponent& operator=(const MaterialComponent&) = default;
	MaterialComponent(MaterialComponent&&) = default;
	MaterialComponent& operator=(MaterialComponent&&) = default;
	~MaterialComponent() = default;

	void Init();

	void SetMaterialData(const cd::Material* pMaterialData) { m_pMaterialData = pMaterialData; }
	cd::Material* GetMaterialData() { return const_cast<cd::Material*>(m_pMaterialData); }
	const cd::Material* GetMaterialData() const { return m_pMaterialData; }

	void SetMaterialType(const engine::MaterialType* pMaterialType) { m_pMaterialType = pMaterialType; }
	const engine::MaterialType* GetMaterialType() const { return m_pMaterialType; }

	void Reset();
	void Build();

	// Basic data.
	void SetName(std::string name) { m_name = cd::MoveTemp(name); }
	std::string& GetName() { return m_name; }
	const std::string& GetName() const { return m_name; }
	const std::string& GetShaderProgramName() const;

	// Uber shader data.
	void ActivateShaderFeature(ShaderFeature feature);
	void DeactiveShaderFeature(ShaderFeature feature);
	void SetShaderFeatures(std::set<ShaderFeature> options) { m_shaderFeatures = cd::MoveTemp(m_shaderFeatures); }
	std::set<ShaderFeature>& GetShaderFeatures() { return m_shaderFeatures; }
	const std::set<ShaderFeature>& GetShaderFeatures() const { return m_shaderFeatures; }
	const std::string& GetFeaturesCombine();

	// Texture data.
	void AddTextureBlob(cd::MaterialTextureType textureType, cd::TextureFormat textureFormat, cd::TextureMapMode uMapMode, cd::TextureMapMode vMapMode, TextureBlob textureBlob, uint32_t width, uint32_t height, uint32_t depth = 1);
	void AddTextureFileBlob(cd::MaterialTextureType textureType, const cd::Material* pMaterial, const cd::Texture& texture, TextureBlob textureBlob);

	const std::map<cd::MaterialTextureType, TextureInfo>& GetTextureResources() const { return m_textureResources; }
	TextureInfo* GetTextureInfo(cd::MaterialTextureType textureType);
	const TextureInfo* GetTextureInfo(cd::MaterialTextureType textureType) const;

	void SetAlbedoColor(cd::Vec3f color) { m_albedoColor = cd::MoveTemp(color); }
	cd::Vec3f& GetAlbedoColor() { return m_albedoColor; }
	const cd::Vec3f& GetAlbedoColor() const { return m_albedoColor; }

	void SetMetallicFactor(float factor) { m_metallicFactor = factor; }
	float& GetMetallicFactor() { return m_metallicFactor; }
	float GetMetallicFactor() const { return m_metallicFactor; }

	void SetRoughnessFactor(float factor) { m_roughnessFactor = factor; }
	float& GetRoughnessFactor() { return m_roughnessFactor; }
	float GetRoughnessFactor() const { return m_roughnessFactor; }

	void SetEmissiveColor(cd::Vec3f color) { m_emissiveColor = cd::MoveTemp(color); }
	cd::Vec3f& GetEmissiveColor() { return m_emissiveColor; }
	const cd::Vec3f& GetEmissiveColor() const { return m_emissiveColor; }

	// Cull parameters. 
	void SetTwoSided(bool value) { m_twoSided = value; }
	bool& GetTwoSided() { return m_twoSided; }
	bool GetTwoSided() const { return m_twoSided; }

	// Blend parameters.
	void SetBlendMode(cd::BlendMode blendMode) { m_blendMode = blendMode; }
	cd::BlendMode& GetBlendMode() { return m_blendMode; }
	cd::BlendMode GetBlendMode() const { return m_blendMode; }

	//OutLine parameters
	void SetOutLine(bool value) { m_outLine = value; }
	bool& GetOutLine() { return m_outLine; }
	bool GetOutLine() const { return m_outLine; }

	void SetAlphaCutOff(float value) { m_alphaCutOff = value; }
	float& GetAlphaCutOff() { return m_alphaCutOff; }
	float GetAlphaCutOff() const { return m_alphaCutOff; }

	void SetDividLine(cd::Vec4f line) { m_dividLine = line; }
	cd::Vec4f& GetDividLine() { return m_dividLine; }
	cd::Vec4f GetDividLine() const { return m_dividLine; }

private:
	// Input
	const cd::Material* m_pMaterialData = nullptr;
	const engine::MaterialType* m_pMaterialType = nullptr;

	std::string m_name;
	cd::Vec3f m_albedoColor;
	float m_metallicFactor;
	float m_roughnessFactor;
	cd::Vec3f m_emissiveColor;
	cd::Vec4f m_dividLine =cd::Vec4f(0.5f,0.3f,0.0f,1.0f);
	bool m_twoSided;
	bool m_outLine;
	cd::BlendMode m_blendMode;
	float m_alphaCutOff;

	bool m_isShaderFeatureDirty = false;
	std::set<ShaderFeature> m_shaderFeatures;
	std::string m_featureCombine;

	std::vector<TextureBlob> m_cacheTextureBlobs;

	// Output
	std::map<cd::MaterialTextureType, TextureInfo> m_textureResources;
};

}
