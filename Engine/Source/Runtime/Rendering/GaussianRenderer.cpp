#include "GaussianRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/ShaderResource.h"
#include "Log\Log.h"

namespace engine
{
void engine::GaussianRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("GaussianProgram", "vs_gaussianSplatting", "fs_gaussianSplatting"));

	GetRenderContext()->CreateUniform("u_texture", bgfx::UniformType::Sampler);
	//GetRenderContext()->CreateUniform("projection", bgfx::UniformType::Mat4);
	//GetRenderContext()->CreateUniform("view", bgfx::UniformType::Mat4);
	//GetRenderContext()->CreateUniform("focal", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateUniform("viewport", bgfx::UniformType::Vec4);

	//GetRenderContext()->CreateUniform("Translation", bgfx::UniformType::Vec4);
	//GetRenderContext()->CreateUniform("Rotation", bgfx::UniformType::Vec4);
	//GetRenderContext()->CreateUniform("Scale", bgfx::UniformType::Vec4);

	bgfx::setViewName(GetViewID(), "GaussianRenderer");
	// NOTE: CreateTexture
	//const bgfx::Memory* mem = bgfx::copy(texturedata, sizeof(texturedata));
	//bgfx::TextureHandle texture = bgfx::createTexture(mem, BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE);


	//NOTE : CreateTexture2D
	//uint16_t width = 1024;
	//uint16_t height = 1024; 
	//bool hasMips = false;  //if-mip-map
	//uint16_t numLayers = 1;
	//bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8; // fomat

	//const bgfx::Memory* mem = bgfx::copy(texturedata, sizeof(texturedata));
	//bgfx::TextureHandle texture = bgfx::createTexture2D(width, height, hasMips, numLayers, format, BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, mem);
}

void engine::GaussianRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void engine::GaussianRenderer::Render(float deltaTime)
{
	for (const auto pResource : m_dependentShaderResources)
	{
		if (ResourceStatus::Ready != pResource->GetStatus() &&
			ResourceStatus::Optimized != pResource->GetStatus())
		{
			return;
		}
	}

	CameraComponent* pMainCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(m_pCurrentSceneWorld->GetMainCameraEntity());
	const cd::Matrix4x4 camView = pMainCameraComponent->GetViewMatrix();
	const cd::Matrix4x4 camProj = pMainCameraComponent->GetProjectionMatrix();
	const cd::Matrix4x4 camViewProj = camProj * camView;
	auto FocalX = pMainCameraComponent->GetFocalX();
	auto FocalY = pMainCameraComponent->GetFocalY();
	auto viewWidth = pMainCameraComponent->GetViewWidth();
	auto viewHeight = pMainCameraComponent->GetViewHeight();
	for (Entity entity : m_pCurrentSceneWorld->GetGaussianRenderEntities())
	{
		if (auto* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			pTransformComponent->Build();
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
		}
		auto* pGaussianComponent = m_pCurrentSceneWorld->GetGaussianRenderComponent(entity);
		auto* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity);
		auto& Transform = pTransformComponent->GetTransform();

		auto gaussianBuffer = pGaussianComponent->GetGaussianBuffer();
		auto gaussianVertexCount = pGaussianComponent->GetVertexCount();
		f_buffer.assign(reinterpret_cast<float*>(gaussianBuffer.data()),
			reinterpret_cast<float*>(gaussianBuffer.data() + gaussianBuffer.size()));

		float maxDepth = -std::numeric_limits<float>::infinity();
		float minDepth = std::numeric_limits<float>::infinity();

		sizeList.resize(gaussianVertexCount);
		for (size_t i = 0; i < gaussianVertexCount; ++i)
		{
			float depth = ((camViewProj.Data(0, 2) * f_buffer[8 * i + 0] +
				camViewProj.Data(1, 2) * f_buffer[8 * i + 1] +
				camViewProj.Data(2, 2) * f_buffer[8 * i + 2]) * 4096);
			sizeList[i] = static_cast<int32_t>(depth);
			if (depth > maxDepth) maxDepth = depth;
			if (depth < minDepth) minDepth = depth;
		}

		float depthInv = (256 * 256) / (maxDepth - minDepth);
		counts0.assign(256 * 256, 0);
		for (size_t i = 0; i < gaussianVertexCount; ++i)
		{
			sizeList[i] = static_cast<uint32_t>((sizeList[i] - minDepth) * depthInv);
			if (sizeList[i] == 65536) sizeList[i] -= 1;
			counts0[sizeList[i]]++;
		}

		starts0.assign(256 * 256, 0);
		for (int i = 1; i < 256 * 256; ++i)
		{
			starts0[i] = starts0[i - 1] + counts0[i - 1];
		}

		depthIndex.resize(gaussianVertexCount);
		for (uint32_t i = 0; i < gaussianVertexCount; ++i)
		{
			depthIndex[starts0[sizeList[i]]++] = i;
		}

		//depthIndex.resize(gaussianVertexCount * 4);
		//for (uint32_t i = 0; i < gaussianVertexCount; ++i)
		//{
		//	uint32_t index = starts0[sizeList[i]]++;
		//	for (uint32_t j = 0; j < 4; ++j)
		//	{
		//		depthIndex[i * 4 + j] = index;
		//	}
		//}

		////Instace
		//const uint16_t instanceStride = 16;
		//// to total number of instances to draw
		//uint32_t totalSprites;
		//totalSprites = gaussianVertexCount;
		//uint32_t drawnSprites = bgfx::getAvailInstanceDataBuffer(totalSprites, instanceStride);

		//bgfx::InstanceDataBuffer idb;
		//bgfx::allocInstanceDataBuffer(&idb, drawnSprites, instanceStride);

		//uint8_t* data = idb.data;
		//for (uint32_t ii = 0; ii < drawnSprites; ++ii)
		//{
		////	float* mtx = (float*)data;
		////bx::mtxSRT(mtx, Transform.GetScale().x(), Transform.GetScale().y(), Transform.GetScale().z(),
		////	Transform.GetRotation().Pitch(), Transform.GetRotation().Yaw(), Transform.GetRotation().Roll(),
		////	Transform.GetTranslation().x(), Transform.GetTranslation().y(), Transform.GetTranslation().z());

		//	uint32_t* texIndex = (uint32_t*)&data;
		//	texIndex[0] = depthIndex[ii];
		//	texIndex[1] = 0;
		//	texIndex[2] = 0;
		//	texIndex[3] = 0;
		//	data += instanceStride;
		//}
		constexpr StringCrc GaussianSampler("u_texture");
		bgfx::setTexture(0, GetRenderContext()->GetUniform(GaussianSampler), pGaussianComponent->GetGaussianTextureHandle());
		//GetRenderContext()->CreateUniform("projection", bgfx::UniformType::Mat4);
		//GetRenderContext()->CreateUniform("view", bgfx::UniformType::Mat4);
		//GetRenderContext()->CreateUniform("focal", bgfx::UniformType::Vec4);
		//GetRenderContext()->CreateUniform("viewport", bgfx::UniformType::Vec4);
		//constexpr StringCrc projectionCrc("projection");
		//bgfx::setUniform(GetRenderContext()->GetUniform(projectionCrc), &camProj, 1);
		//constexpr StringCrc viewCrc("view");
		//bgfx::setUniform(GetRenderContext()->GetUniform(viewCrc), &camView, 1);
		//constexpr StringCrc focalCrc("focal");
		//cd::Vec4f focal{FocalX, FocalY, 0.0f, 0.0f};
		//bgfx::setUniform(GetRenderContext()->GetUniform(focalCrc), &focal, 1);
		constexpr StringCrc viewportCrc("viewport");
		cd::Vec4f viewport{viewWidth, viewHeight, 0.0f, 0.0f};
		bgfx::setUniform(GetRenderContext()->GetUniform(viewportCrc), &viewport, 1);

		//constexpr StringCrc TranslationCrc("Translation");
		//bgfx::setUniform(GetRenderContext()->GetUniform(TranslationCrc), &Transform.GetTranslation(), 1);

		//constexpr StringCrc RotationCrc("Rotation");
		//bgfx::setUniform(GetRenderContext()->GetUniform(RotationCrc), &Transform.GetRotation(), 1);

		//constexpr StringCrc ScaleCrc("Scale");
		//bgfx::setUniform(GetRenderContext()->GetUniform(ScaleCrc), &Transform.GetScale(), 1);
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pGaussianComponent->GetVertexBufferHandle() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pGaussianComponent->GetIndexBufferHandle() });

		//bgfx::setInstanceDataBuffer(&idb,0, gaussianVertexCount);

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_TRISTRIP;
		bgfx::setState(state);

		constexpr StringCrc programHandleIndex{ "GaussianProgram" };
		GetRenderContext()->Submit(GetViewID(), programHandleIndex);
	}
}
}
