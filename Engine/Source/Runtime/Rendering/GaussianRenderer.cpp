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
	GetRenderContext()->CreateUniform("viewport", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateUniform("depthIndex", bgfx::UniformType::Vec4);

	bgfx::setViewName(GetViewID(), "GaussianRenderer");
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
		auto gaussianCount = pGaussianComponent->GetVertexCount();
		f_buffer.assign(reinterpret_cast<float*>(gaussianBuffer.data()),
			reinterpret_cast<float*>(gaussianBuffer.data() + gaussianBuffer.size())); //f_buffer over

		float maxDepth = -std::numeric_limits<float>::infinity();
		float minDepth = std::numeric_limits<float>::infinity();

		sizeList.resize(gaussianCount);
		for (size_t i = 0; i < gaussianCount; ++i)
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
		for (size_t i = 0; i < gaussianCount; ++i)
		{
			sizeList[i] = static_cast<int32_t>((sizeList[i] - minDepth) * depthInv);
			counts0[sizeList[i]]++;
		}

		starts0.assign(256 * 256, 0);
		for (int i = 1; i < 256 * 256; ++i)
		{
			starts0[i] = starts0[i - 1] + counts0[i - 1];
		}

		depthIndex.resize(gaussianCount);
		depthIndexFloat.resize(gaussianCount);
		for (uint32_t i = 0; i < gaussianCount; ++i)
		{
			depthIndex[starts0[sizeList[i]]++] = i;
		}

		for (uint32_t i = 0; i < gaussianCount; i++)
		{
			constexpr StringCrc GaussianSampler("u_texture");
			bgfx::setTexture(0, GetRenderContext()->GetUniform(GaussianSampler), pGaussianComponent->GetGaussianTextureHandle());

			constexpr StringCrc viewportCrc("viewport");
			cd::Vec4f viewport{viewWidth, viewHeight, 0.0f, 0.0f};
			bgfx::setUniform(GetRenderContext()->GetUniform(viewportCrc), &viewport, 1);

			constexpr StringCrc depthIndexCrc("depthIndex");

			cd::Vec4f Index{*reinterpret_cast<float*>(&depthIndex[i]), 0.0f, 0.0f,0.0f};
			bgfx::setUniform(GetRenderContext()->GetUniform(depthIndexCrc), &Index, 1);
			bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pGaussianComponent->GetVertexBufferHandle() });
			bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pGaussianComponent->GetIndexBufferHandle() });

			constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
				BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_TRISTRIP;
			bgfx::setState(state);

			constexpr StringCrc programHandleIndex{ "GaussianProgram" };
			GetRenderContext()->Submit(GetViewID(), programHandleIndex);
		}
		
	}
}
}
