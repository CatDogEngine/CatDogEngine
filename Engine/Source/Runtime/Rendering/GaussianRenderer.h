#pragma once

#include "Renderer.h"
#include "RenderContext.h"
#include <atomic>
#include <vector>

namespace engine
{
class SceneWorld;

class GaussianRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }

private:
	SceneWorld* m_pCurrentSceneWorld = nullptr;
	std::atomic<int>	m_curBuffer = 0;
	std::atomic<bool>	m_isSorting = false;
	float m_curView[16];
	float m_lastView[16];
};

}