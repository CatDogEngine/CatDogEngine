#include "PostProcessRenderer.h"

#include "RenderContext.h"

namespace engine
{

void PostProcessRenderer::Init()
{
	m_pRenderContext->CreateUniform("s_lightingColor", bgfx::UniformType::Sampler);
	m_pRenderContext->CreateProgram("PostProcessProgram", "vs_fullscreen.bin", "fs_PBR_postProcessing.bin");

	bgfx::setViewName(GetViewID(), "PostProcessRenderer");
}

PostProcessRenderer::~PostProcessRenderer()
{
}

void PostProcessRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	// Output to swap chain
	bgfx::setViewFrameBuffer(GetViewID(), *GetRenderTarget()->GetFrameBufferHandle());
	bgfx::setViewRect(GetViewID(), 0, 0, GetRenderTarget()->GetWidth(), GetRenderTarget()->GetHeight());

	cd::Matrix4x4 orthoMatrix = cd::Matrix4x4::Orthographic(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1000.0f, 0.0f, bgfx::getCaps()->homogeneousDepth).Transpose();
	bgfx::setViewTransform(GetViewID(), nullptr, orthoMatrix.Begin());
}

void PostProcessRenderer::Render(float deltaTime)
{
	// TODO : expose these resource names to outside.
	constexpr StringCrc lightingResultSampler("s_lightingColor");
	constexpr StringCrc sceneRenderTarget("SceneRenderTarget");
	bgfx::setTexture(0, m_pRenderContext->GetUniform(lightingResultSampler), m_pRenderContext->GetRenderTarget(sceneRenderTarget)->GetTextureHandle(0));
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	Renderer::ScreenSpaceQuad(static_cast<float>(GetRenderTarget()->GetWidth()), static_cast<float>(GetRenderTarget()->GetHeight()), false);

	constexpr StringCrc programName("PostProcessProgram");
	bgfx::submit(GetViewID(), m_pRenderContext->GetProgram(programName));
}

}