#include "GaussianRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Log/Log.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/ShaderResource.h"

namespace engine
{

void engine::GaussianRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("GaussianProgram", "vs_gaussianSplatting", "fs_gaussianSplatting"));

	GetRenderContext()->CreateUniform("u_focal", bgfx::UniformType::Vec4);

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
		if (ResourceStatus::Optimized != pResource->GetStatus())
		{
			return;
		}
	}

	CameraComponent* pMainCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(m_pCurrentSceneWorld->GetMainCameraEntity());
	float fov = pMainCameraComponent->GetFov();
	float width = pMainCameraComponent->GetViewWidth();
	float height = pMainCameraComponent->GetViewHeight();
	float aspect = pMainCameraComponent->GetAspect();
	float fx = width/ 2.0f / tanf(bx::toRad(fov / 2.0f));
	float fy = height * aspect / 2.0f / tanf(bx::toRad(fov / 2.0f));
	auto& viewMatrix = pMainCameraComponent->GetViewMatrix();

	for (Entity entity : m_pCurrentSceneWorld->GetGaussianRenderEntities())
	{
		auto* pGaussianComponent = m_pCurrentSceneWorld->GetGaussianRenderComponent(entity);
		auto* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity);
		bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());

		bx::memCopy(m_curView, viewMatrix.begin(), 16 * sizeof(float));
		bgfx::setState(
			BGFX_STATE_PT_TRISTRIP |
			BGFX_STATE_WRITE_RGB |
			BGFX_STATE_WRITE_A |
			BGFX_STATE_BLEND_EQUATION_ADD |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA) |
			BGFX_STATE_DEPTH_TEST_LESS);

		constexpr StringCrc focalCrc("u_focal");
		float focal[2] = { fx, fy };
		bgfx::setUniform(GetRenderContext()->GetUniform(focalCrc), focal);
		bgfx::setVertexBuffer(0, pGaussianComponent->GetVBH());
		auto &splatData = pGaussianComponent->GetSplatData(m_curBuffer);
		bgfx::setInstanceDataBuffer(splatData.m_vbh, 0, splatData.m_vertexCount);

		constexpr StringCrc programHandleIndex{ "GaussianProgram" };
		GetRenderContext()->Submit(GetViewID(), programHandleIndex);

		if (bx::memCmp(m_curView, m_lastView, 16 * sizeof(float)) != 0)
		{
			float dot = m_lastView[2] * m_curView[2] +
				m_lastView[6] * m_curView[6] +
				m_lastView[10] * m_curView[10];
			if (abs(dot - 1.0f) > 0.01f && !m_isSorting)
			{
				if (!pGaussianComponent->GetSplatData(m_curBuffer).m_vertexCount)
				{
					return;
				}

				m_isSorting = true;
				int nextBuffer = 1 - m_curBuffer;
				uint32_t vertexCount = pGaussianComponent->GetSplatData(nextBuffer).m_vertexCount;

				std::vector<uint32_t> depthIndex(vertexCount + 1);
				InstanceDataVertex *vertexBuffer = pGaussianComponent->GetSplatData(nextBuffer).m_buffer;

				float maxDepth = -FLT_MAX;
				float minDepth = FLT_MAX;
				static std::vector<int32_t> sizeList(vertexCount);
				bx::Vec3 zAxis(m_curView[2] * 4096.0f, m_curView[6] * 4096.0f, m_curView[10] * 4096.0f);
				auto &splatFileData = pGaussianComponent->GetSplatFileData();
				for (uint32_t i = 0; i < vertexCount; ++i)
				{
					float depth = bx::dot(bx::Vec3(splatFileData[i].m_cx, splatFileData[i].m_cy, splatFileData[i].m_cz), zAxis);
					sizeList[i] = static_cast<uint32_t>(depth);
					if (depth > maxDepth) maxDepth = depth;
					if (depth < minDepth) minDepth = depth;
				}

				float depthInv = (256.0f * 256.0f) / (maxDepth - minDepth);
				static std::array<uint32_t, 256 * 256 + 1> counts0;
				memset(counts0.data(), 0, (256 * 256 + 1) * sizeof(uint32_t));
				for (uint32_t i = 0; i < vertexCount; ++i)
				{
					sizeList[i] = static_cast<uint32_t>(((sizeList[i] - minDepth) * depthInv));
					counts0[sizeList[i]]++;
				}

				static std::array<uint32_t, 256 * 256 + 1> starts0;
				starts0[0] = 0;
				for (uint32_t i = 1; i < 256 * 256; ++i)
				{
					starts0[i] = starts0[i - 1] + counts0[i - 1];
				}
				for (uint32_t i = 0; i < vertexCount; ++i)
				{
					depthIndex[starts0[sizeList[i]]++] = i;
				}

				for (uint32_t j = 0; j < vertexCount; ++j)
				{
					const uint32_t i = depthIndex[vertexCount - j - 1];
					vertexBuffer[j] = splatFileData[i];
				}

				bgfx::ReleaseFn releaseFn = [](void *data, void *bufferPtr)
				{
					BX_UNUSED(data);
					BX_UNUSED(bufferPtr);
					GaussianRenderer *example = (GaussianRenderer *)bufferPtr;
					example->m_curBuffer = 1 - example->m_curBuffer;
				};
				bgfx::update(pGaussianComponent->GetSplatData(nextBuffer).m_vbh, 0,
					bgfx::makeRef(pGaussianComponent->GetSplatData(nextBuffer).m_buffer,
					pGaussianComponent->GetSplatData(0).m_vertexCount * sizeof(InstanceDataVertex), releaseFn, this));

				m_isSorting = false;

				bx::memCopy(m_lastView, m_curView, 16 * sizeof(float));
			}
		}
	}
}

}