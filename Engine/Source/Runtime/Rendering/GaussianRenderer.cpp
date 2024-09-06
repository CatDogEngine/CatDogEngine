#include "GaussianRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/ShaderResource.h"
#include "Log\Log.h"

namespace engine
{

namespace
{

std::vector<float> GetViewMatrix(const float R[3][3], const float t[3])
{
	std::vector<float> camToWorld(16);

	camToWorld[0] = R[0][0]; camToWorld[1] = R[0][1]; camToWorld[2] = R[0][2]; camToWorld[3] = 0;
	camToWorld[4] = R[1][0]; camToWorld[5] = R[1][1]; camToWorld[6] = R[1][2]; camToWorld[7] = 0;
	camToWorld[8] = R[2][0]; camToWorld[9] = R[2][1]; camToWorld[10] = R[2][2]; camToWorld[11] = 0;
	camToWorld[12] = -t[0] * R[0][0] - t[1] * R[1][0] - t[2] * R[2][0];
	camToWorld[13] = -t[0] * R[0][1] - t[1] * R[1][1] - t[2] * R[2][1];
	camToWorld[14] = -t[0] * R[0][2] - t[1] * R[1][2] - t[2] * R[2][2];
	camToWorld[15] = 1;

	return camToWorld;
}

std::vector<float> GetProjectionMatrix(float fx, float fy, float width, float height)
{
	float znear = 0.2f;
	float zfar = 200.0f;

	return { (2.0f * fx) / width, 0.0f, 0.0f, 0.0f,
		0.0f, -(2.0f * fy) / height, 0.0f, 0.0f,
		0.0f, 0.0f, zfar / (zfar - znear), 1.0f,
		0.0f, 0.0f, -(zfar * znear) / (zfar - znear), 0.0f };
}

std::vector<float> Multiply4(const std::vector<float> &a, const std::vector<float> &b)
{
	return {
		 b[0] * a[0] + b[1] * a[4] + b[2] * a[8] + b[3] * a[12],
		 b[0] * a[1] + b[1] * a[5] + b[2] * a[9] + b[3] * a[13],
		 b[0] * a[2] + b[1] * a[6] + b[2] * a[10] + b[3] * a[14],
		 b[0] * a[3] + b[1] * a[7] + b[2] * a[11] + b[3] * a[15],
		 b[4] * a[0] + b[5] * a[4] + b[6] * a[8] + b[7] * a[12],
		 b[4] * a[1] + b[5] * a[5] + b[6] * a[9] + b[7] * a[13],
		 b[4] * a[2] + b[5] * a[6] + b[6] * a[10] + b[7] * a[14],
		 b[4] * a[3] + b[5] * a[7] + b[6] * a[11] + b[7] * a[15],
		 b[8] * a[0] + b[9] * a[4] + b[10] * a[8] + b[11] * a[12],
		 b[8] * a[1] + b[9] * a[5] + b[10] * a[9] + b[11] * a[13],
		 b[8] * a[2] + b[9] * a[6] + b[10] * a[10] + b[11] * a[14],
		 b[8] * a[3] + b[9] * a[7] + b[10] * a[11] + b[11] * a[15],
		 b[12] * a[0] + b[13] * a[4] + b[14] * a[8] + b[15] * a[12],
		 b[12] * a[1] + b[13] * a[5] + b[14] * a[9] + b[15] * a[13],
		 b[12] * a[2] + b[13] * a[6] + b[14] * a[10] + b[15] * a[14],
		 b[12] * a[3] + b[13] * a[7] + b[14] * a[11] + b[15] * a[15]
	};
}

} // namespace

void engine::GaussianRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("GaussianProgram", "vs_gaussianSplatting", "fs_gaussianSplatting"));

	GetRenderContext()->CreateUniform("u_texture", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("projection", bgfx::UniformType::Mat4);
	GetRenderContext()->CreateUniform("view", bgfx::UniformType::Mat4);
	GetRenderContext()->CreateUniform("focal", bgfx::UniformType::Vec4);
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
		if (ResourceStatus::Optimized != pResource->GetStatus())
		{
			return;
		}
	}

	constexpr float rotation[3][3] = {
		{0.876134201218856f, 0.06925962026449776f, 0.47706599800804744f},
		{-0.04747421839895102f, 0.9972110940209488f, -0.057586739349882114f},
		{-0.4797239414934443f, 0.027805376500959853f, 0.8769787916452908f}
	};

	constexpr float position[3] = {
		-3.0089893469241797f, -0.11086489695181866f, -3.7527640949141428f
	};

	constexpr float fx = 1159.5880733038064f;
	constexpr float fy = 1164.6601287484507f;
	constexpr float viewWidth = 619.0f;
	constexpr float viewHeight = 438.0f;

	std::vector<float> viewMatrix = GetViewMatrix(rotation, position);
	std::vector<float> projMartrix = GetProjectionMatrix(fx,fy, viewWidth, viewHeight);
	std::vector<float> viewProj = Multiply4(projMartrix, viewMatrix);
	for (Entity entity : m_pCurrentSceneWorld->GetGaussianRenderEntities())
	{
		auto* pGaussianComponent = m_pCurrentSceneWorld->GetGaussianRenderComponent(entity);
		const uint32_t gaussianCount = m_pCurrentSceneWorld->GetGaussianRenderComponent(entity)->GetVertexCount();
		static std::vector<uint32_t> depthIndices(gaussianCount);

		// Sort
		bool enableSort = false;
		if(enableSort)
		{
			static std::vector<int32_t> sizeList(gaussianCount);
			static std::vector<uint32_t> count(256 * 256, 0);
			static std::vector<uint32_t> start(256 * 256, 0);

			memset(count.data(), 0, count.size() * sizeof(uint32_t));
			memset(start.data(), 0, start.size() * sizeof(uint32_t));

			auto &gaussianBuffer = pGaussianComponent->GetGaussianBuffer();
			float *pFloatBuffer = reinterpret_cast<float *>(gaussianBuffer.data());

			float maxDepth = -std::numeric_limits<float>::max();
			float minDepth = std::numeric_limits<float>::max();
			for (size_t i = 0; i < gaussianCount; ++i)
			{
				float depth = ((viewProj[2] * pFloatBuffer[8 * i] +
					viewProj[6] * pFloatBuffer[8 * i + 1] +
					viewProj[10] * pFloatBuffer[8 * i + 2]) * 4096);
				sizeList[i] = (int32_t)depth;
				maxDepth = (depth > maxDepth) ? depth : maxDepth;
				minDepth = (depth < minDepth) ? depth : minDepth;
			}

			float depthInv = (256 * 256) / (maxDepth - minDepth);
			for (size_t i = 0; i < gaussianCount; ++i)
			{
				sizeList[i] = static_cast<int32_t>((sizeList[i] - minDepth) * depthInv);
				count[sizeList[i]]++;
			}

			for (int i = 1; i < 256 * 256; ++i)
			{
				start[i] = start[i - 1] + count[i - 1];
			}

			for (uint32_t i = 0; i < gaussianCount; ++i)
			{
				depthIndices[start[sizeList[i]]++] = i;
			}
		}
		else
		{
			for (uint32_t i = 0; i < gaussianCount; ++i)
			{
				depthIndices[i] = i;
			}
		}

		for (size_t i = 0; i < gaussianCount; ++i)
		{
			constexpr StringCrc GaussianSampler("u_texture");
			bgfx::setTexture(0, GetRenderContext()->GetUniform(GaussianSampler), bgfx::TextureHandle{ pGaussianComponent->GetGaussianTextureHandle() });

			constexpr StringCrc projectionCrc("projection");
			bgfx::setUniform(GetRenderContext()->GetUniform(projectionCrc), projMartrix.data());

			constexpr StringCrc viewCrc("view");
			bgfx::setUniform(GetRenderContext()->GetUniform(viewCrc), viewMatrix.data());

			constexpr StringCrc focalCrc("focal");
			float focal[2] = { fx, fy };
			bgfx::setUniform(GetRenderContext()->GetUniform(focalCrc), focal);

			constexpr StringCrc viewportCrc("viewport");
			float viewport[2] = { viewWidth, viewHeight };
			bgfx::setUniform(GetRenderContext()->GetUniform(viewportCrc), viewport);

			constexpr StringCrc depthIndexCrc("depthIndex");
			float depthIndex = *reinterpret_cast<float *>(&depthIndices[i]);
			bgfx::setUniform(GetRenderContext()->GetUniform(depthIndexCrc), &depthIndex);

			bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pGaussianComponent->GetVertexBufferHandle() });
			bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pGaussianComponent->GetIndexBufferHandle() });

			bgfx::setState(
				BGFX_STATE_WRITE_RGB |
				BGFX_STATE_WRITE_A |
				BGFX_STATE_BLEND_EQUATION_ADD |
				BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));

			constexpr StringCrc programHandleIndex{ "GaussianProgram" };
			GetRenderContext()->Submit(GetViewID(), programHandleIndex);
		}
	}
}

}