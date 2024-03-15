#pragma once

#include "Material/MaterialType.h"
#include "Resources/ResourceBuilder.h"
#include "Rendering/ShaderCompileInfo.h"

#include <map>
#include <string>

namespace engine
{

class RenderContext;

}

namespace editor
{

class ShaderBuilder
{
public:
	static void Build(engine::RenderContext* pRenderContext);
	static void RegisterUberShaderAllVariants(engine::RenderContext* pRenderContext, engine::MaterialType* pMaterialType);

	// Compile specified shader program/program variant.
	static void BuildShaderInfos(engine::RenderContext* pRenderContext, TaskOutputCallbacks callbacks = {});
};

} // namespace editor
