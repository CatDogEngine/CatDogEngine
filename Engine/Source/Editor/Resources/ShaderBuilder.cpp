#include "ShaderBuilder.h"

#include "ECWorld/SceneWorld.h"
#include "Log/Log.h"
#include "Path/Path.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/ResourceContext.h"
#include "Rendering/Resources/ShaderResource.h"

namespace editor
{

void ShaderBuilder::Build(engine::RenderContext* pRenderContext)
{
	for (const auto& [_, pShaderResource] : pRenderContext->GetShaderResources())
	{
		engine::ShaderProgramType programType = pShaderResource->GetType();

		if (engine::ShaderProgramType::Standard == programType)
		{
			ResourceBuilder::Get().AddShaderBuildTask(engine::ShaderType::Vertex,
				pShaderResource->GetShaderInfo(0).scPath.c_str(),
				pShaderResource->GetShaderInfo(0).binPath.c_str());

			ResourceBuilder::Get().AddShaderBuildTask(engine::ShaderType::Fragment,
				pShaderResource->GetShaderInfo(1).scPath.c_str(),
				pShaderResource->GetShaderInfo(1).binPath.c_str());
		}
		else
		{
			engine::ShaderType shaderType = engine::ProgramTypeToSingleShaderType.at(programType);
			ResourceBuilder::Get().AddShaderBuildTask(shaderType,
				pShaderResource->GetShaderInfo(0).scPath.c_str(),
				pShaderResource->GetShaderInfo(0).binPath.c_str());
		}
	}

	ResourceBuilder::Get().Update();
}

void ShaderBuilder::RegisterUberShaderAllVariants(engine::RenderContext* pRenderContext, engine::MaterialType* pMaterialType)
{
	const std::string& programName = pMaterialType->GetShaderSchema().GetShaderProgramName();
	const engine::ShaderResource* pOriginShaderResource = pRenderContext->GetResourceContext()->GetShaderResource(engine::StringCrc{ programName });
	const auto& combines = pMaterialType->GetShaderSchema().GetAllFeatureCombines();
	engine::ShaderProgramType programType = pOriginShaderResource->GetType();

	for (const auto& combine : combines)
	{
		if (engine::ShaderProgramType::Standard == programType)
		{
			pRenderContext->RegisterShaderProgram(programName.c_str(),
				pOriginShaderResource->GetShaderInfo(0).name.c_str(),
				pOriginShaderResource->GetShaderInfo(0).name.c_str(),
				combine);
		}
		else
		{
			pRenderContext->RegisterShaderProgram(programName.c_str(),
				pOriginShaderResource->GetShaderInfo(0).name.c_str(),
				programType,
				combine);
		}
	}

	CD_ENGINE_INFO("Compiling program {0} all variants with count : {1}", programName, combines.size());
}

void ShaderBuilder::BuildShaderInfos(engine::RenderContext* pRenderContext, TaskOutputCallbacks callbacks)
{
	for (auto info : pRenderContext->GetShaderCompileInfos())
	{
		const auto* pShaderResource = pRenderContext->GetResourceContext()->GetShaderResource(engine::StringCrc{ info.GetProgramName() + info.GetFeaturesCombine()});
		engine::ShaderProgramType programType = pShaderResource->GetType();
		if (engine::ShaderProgramType::Standard == programType)
		{
			const auto& vs = pShaderResource->GetShaderInfo(0);
			const auto& fs = pShaderResource->GetShaderInfo(1);

			TaskHandle vsTaskHandle = ResourceBuilder::Get().AddShaderBuildTask(vs.type,
				vs.scPath.c_str(), vs.binPath.c_str(), info.GetFeaturesCombine().c_str(), callbacks);
			TaskHandle fsTaskHandle = ResourceBuilder::Get().AddShaderBuildTask(fs.type,
				fs.scPath.c_str(), fs.binPath.c_str(), info.GetFeaturesCombine().c_str(), callbacks);

			info.AddTaskHandle(vsTaskHandle);
			info.AddTaskHandle(fsTaskHandle);
		}
		else
		{
			const auto& shader = pShaderResource->GetShaderInfo(0);
			TaskHandle taskHandle = ResourceBuilder::Get().AddShaderBuildTask(shader.type,
				shader.scPath.c_str(), shader.binPath.c_str(), info.GetFeaturesCombine().c_str(), callbacks);
			info.AddTaskHandle(taskHandle);
		}
	}
}

} // namespace editor
