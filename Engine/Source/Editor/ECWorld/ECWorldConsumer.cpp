#include "ECWorldConsumer.h"

#include "ECWorld/ComponentsStorage.hpp"
#include "ECWorld/HierarchyComponent.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/NameComponent.h"
#include "ECWorld/World.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Material/MaterialType.h"
#include "Rendering/RenderContext.h"
#include "Scene/Material.h"
#include "Scene/MaterialTextureType.h"
#include "Scene/SceneDatabase.h"
#include "Scene/VertexFormat.h"

#include <filesystem>
#include <format>
#include <map>

namespace engine
{

void ECWorldConsumer::SetSceneDatabaseIDs(uint32_t meshID)
{
	m_meshMinID = meshID;
}

void ECWorldConsumer::Execute(const cd::SceneDatabase* pSceneDatabase)
{
	assert(pSceneDatabase->GetMeshCount() > 0);

	std::map<cd::TransformID::ValueType, Entity> mapTransformIDToEntities;

	for (const auto& transform : pSceneDatabase->GetTransforms())
	{
		for (cd::MeshID meshID : transform.GetMeshIDs())
		{
			const auto& mesh = pSceneDatabase->GetMesh(meshID.Data());
			if (m_meshMinID > mesh.GetID().Data())
			{
				// The SceneDatabase can be reused when we import assets multiple times.
				// So we need to filter meshes which was generated in the past.
				continue;
			}

			assert(mesh.GetVertexCount() > 0 && mesh.GetPolygonCount() > 0);

			Entity meshEntity = m_pWorld->CreateEntity();

			NameComponent& nameComponent = m_pWorld->CreateComponent<NameComponent>(meshEntity);
			nameComponent.SetName(mesh.GetName());

			HierarchyComponent& hierarchyComponent = m_pWorld->CreateComponent<HierarchyComponent>(meshEntity);
			cd::TransformID parentTransformID = transform.GetParentID();
			bool hasParent = parentTransformID.Data() != cd::TransformID::InvalidID;
			if (hasParent)
			{
				// It requires that we parse Transform nodes from Top to Down as a Depth-First Search.
				assert(mapTransformIDToEntities.contains(parentTransformID.Data()));
				hierarchyComponent.SetParentEntity(mapTransformIDToEntities[parentTransformID.Data()]);
			}

			TransformComponent& transformComponent = m_pWorld->CreateComponent<TransformComponent>(meshEntity);


			StaticMeshComponent& staticMeshComponent = m_pWorld->CreateComponent<StaticMeshComponent>(meshEntity);
			staticMeshComponent.SetMeshData(&mesh);
			staticMeshComponent.SetRequiredVertexFormat(&m_pStandardMaterialType->GetVertexFormat());

			if (mesh.GetMaterialID().IsValid())
			{
				MaterialComponent& materialComponent = m_pWorld->CreateComponent<MaterialComponent>(meshEntity);

				constexpr uint64_t textureSamplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
				const auto& assignedMaterial = pSceneDatabase->GetMaterial(mesh.GetMaterialID().Data());
				bool isMissingTexture = false;

				std::optional<cd::TextureID> optBaseColorMap = assignedMaterial.GetTextureID(cd::MaterialTextureType::BaseColor);
				if (optBaseColorMap.has_value())
				{
					const cd::Texture& baseColorMap = pSceneDatabase->GetTexture(optBaseColorMap.value().Data());
					std::filesystem::path hackFilePath = baseColorMap.GetPath();
					std::string ddsFilePath = hackFilePath.stem().generic_string().c_str();
					ddsFilePath += ".dds";
					bgfx::TextureHandle textureHandle = m_pRenderContext->CreateTexture(ddsFilePath.c_str(), textureSamplerFlags | BGFX_TEXTURE_SRGB);
					if (bgfx::isValid(textureHandle))
					{
						bgfx::UniformHandle samplerHandle = m_pRenderContext->CreateUniform(std::format("s_textureBaseColor{}", mesh.GetMaterialID().Data()).c_str(), bgfx::UniformType::Sampler);
						materialComponent.SetTextureInfo(cd::MaterialTextureType::BaseColor, 0, samplerHandle.idx, textureHandle.idx);
					}
					else
					{
						isMissingTexture = true;
					}
				}

				std::optional<cd::TextureID> optNormalMap = assignedMaterial.GetTextureID(cd::MaterialTextureType::Normal);
				if (optNormalMap.has_value())
				{
					const cd::Texture& normalMap = pSceneDatabase->GetTexture(optNormalMap.value().Data());
					std::filesystem::path hackFilePath = normalMap.GetPath();
					std::string ddsFilePath = hackFilePath.stem().generic_string().c_str();
					ddsFilePath += ".dds";
					bgfx::TextureHandle textureHandle = m_pRenderContext->CreateTexture(ddsFilePath.c_str(), textureSamplerFlags);
					if (bgfx::isValid(textureHandle))
					{
						bgfx::UniformHandle samplerHandle = m_pRenderContext->CreateUniform(std::format("s_textureNormal{}", mesh.GetMaterialID().Data()).c_str(), bgfx::UniformType::Sampler);
						materialComponent.SetTextureInfo(cd::MaterialTextureType::Normal, 1, samplerHandle.idx, textureHandle.idx);
					}
					else
					{
						isMissingTexture = true;
					}
				}

				std::optional<cd::TextureID> optMetalnessMap = assignedMaterial.GetTextureID(cd::MaterialTextureType::Metalness);
				std::optional<cd::TextureID> optRoughnessMap = assignedMaterial.GetTextureID(cd::MaterialTextureType::Roughness);
				if (optMetalnessMap.has_value() && optRoughnessMap.has_value() && optMetalnessMap.value() == optRoughnessMap.value())
				{
					// Check AO is optional as the texture asset should have value 1.0 which doesn't affect shader results if it doesn't use AO.
					const cd::Texture& ormMap = pSceneDatabase->GetTexture(optMetalnessMap.value().Data());
					std::filesystem::path hackFilePath = ormMap.GetPath();
					std::string ddsFilePath = hackFilePath.stem().generic_string().c_str();
					ddsFilePath += ".dds";
					bgfx::TextureHandle textureHandle = m_pRenderContext->CreateTexture(ddsFilePath.c_str(), textureSamplerFlags);
					if (bgfx::isValid(textureHandle))
					{
						bgfx::UniformHandle samplerHandle = m_pRenderContext->CreateUniform(std::format("s_textureORM{}", mesh.GetMaterialID().Data()).c_str(), bgfx::UniformType::Sampler);
						materialComponent.SetTextureInfo(cd::MaterialTextureType::Metalness, 2, samplerHandle.idx, textureHandle.idx);
					}
					else
					{
						isMissingTexture = true;
					}
				}

				//std::optional<cd::TextureID> optEmissive = assignedMaterial.GetTextureID(cd::MaterialTextureType::Emissive);
				//if (optEmissive.has_value())
				//{
				//	const cd::Texture& emissiveMap = pSceneDatabase->GetTexture(optEmissive.value().Data());
				//	bgfx::UniformHandle samplerHandle = m_pRenderContext->CreateUniform(std::format("s_textureEmissive{}", mesh.GetMaterialID().Data()).c_str(), bgfx::UniformType::Sampler);
				//	bgfx::TextureHandle textureHandle = m_pRenderContext->CreateTexture(emissiveMap.GetPath(), textureSamplerFlags);
				//	materialComponent.SetTextureInfo(cd::MaterialTextureType::Emissive, 0, samplerHandle.idx, textureHandle.idx);
				//}

				bgfx::ProgramHandle shadingProgram;
				if (isMissingTexture)
				{
					shadingProgram = m_pRenderContext->CreateProgram("MissingTextures", m_pStandardMaterialType->GetVertexShaderName(), "fs_missing_textures.bin");
				}
				else
				{
					shadingProgram = m_pRenderContext->CreateProgram(m_pStandardMaterialType->GetMaterialName(), m_pStandardMaterialType->GetVertexShaderName(), m_pStandardMaterialType->GetFragmentShaderName());
				}
				materialComponent.SetShadingProgram(shadingProgram.idx);
			}

			m_meshEntities.push_back(meshEntity);
		}
	}
}

}