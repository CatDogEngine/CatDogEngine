#include "ShaderResource.h"

#include "Log/Log.h"
#include "Path/Path.h"
#include "Resources/ResourceLoader.h"

#include <bgfx/bgfx.h>

#include <cassert>

namespace engine
{

ShaderResource::~ShaderResource()
{
	SetStatus(ResourceStatus::Garbage);
	Update();
}

void ShaderResource::Update()
{
	switch (GetStatus())
	{
		case ResourceStatus::Loading:
		{
			// Read bin shader file

			auto& shader = m_shaders[0];
			if (shader.binBlob.empty())
			{
				auto& shaderBinPath = shader.binPath;
				if (shaderBinPath.empty())
				{
					shaderBinPath = Path::GetShaderOutputPath(shader.name.c_str());
				}

				shader.binBlob = engine::ResourceLoader::LoadFile(shaderBinPath.c_str());
				SetStatus(ResourceStatus::Loaded);
			}

			if (ShaderProgramType::Standard == m_type)
			{
				auto& fragmentShader = m_shaders[1];
				if (fragmentShader.binBlob.empty())
				{
					auto& fragmentShaderBinPath = fragmentShader.binPath;
					if (fragmentShaderBinPath.empty())
					{
						fragmentShaderBinPath = Path::GetShaderOutputPath(fragmentShader.name.c_str());
					}

					fragmentShader.binBlob = engine::ResourceLoader::LoadFile(fragmentShaderBinPath.c_str());
				}
			}

			break;
		}
		case ResourceStatus::Loaded:
		{
			// Check something

			if (!m_shaders[0].binBlob.empty() && !(ShaderProgramType::Standard == m_type && m_shaders[1].binBlob.empty()))
			{
				SetStatus(ResourceStatus::Building);
			}
			break;
		}
		case ResourceStatus::Building:
		{
			// Build Rendering data
			// It seems no Rendering data for shader?

			SetStatus(ResourceStatus::Built);
			break;
		}
		case ResourceStatus::Built:
		{
			// Build GPU handle

			if (BuildShaderHandle() && BuildProgramHandle())
			{
				m_recycleCount = 0U;
				SetStatus(ResourceStatus::Ready);
			}
			break;
		}
		case ResourceStatus::Ready:
		{
			// Delete CPU data delayed

			constexpr uint32_t recycleDelayFrames = 30U;
			if (m_recycleCount++ >= recycleDelayFrames)
			{
				FreeShaderData(0);
				FreeShaderData(1);

				m_recycleCount = 0U;
				SetStatus(ResourceStatus::Optimized);
			}
			break;
		}
		case ResourceStatus::Garbage:
		{
			// Distory GPU handle

			DistoryShaderHandle(0);
			DistoryShaderHandle(1);
			DistoryProgramHandle();
			SetStatus(ResourceStatus::Destroyed);
			break;
		}
		default:
			break;
	}
}
void ShaderResource::Reset()
{
	// Clear everything
	ClearShaderData(0);
	ClearShaderData(1);
	DistoryShaderHandle(0);
	DistoryShaderHandle(1);
	DistoryProgramHandle();
	SetStatus(ResourceStatus::Loading);
}

ShaderResource::ShaderInfo& ShaderResource::GetVertexShaderInfo()
{
	assert(ShaderProgramType::Standard == m_type || ShaderProgramType::VertexOnly == m_type);
	return m_shaders[0];
}

const ShaderResource::ShaderInfo& ShaderResource::GetVertexShaderInfo() const
{
	assert(ShaderProgramType::Standard == m_type || ShaderProgramType::VertexOnly == m_type);
	return m_shaders[0];
}

void ShaderResource::SetVertexShaderInfo(ShaderInfo info)
{
	assert(ShaderProgramType::Standard == m_type || ShaderProgramType::VertexOnly == m_type);
	m_shaders[0] = cd::MoveTemp(info);
}

ShaderResource::ShaderInfo& ShaderResource::GetFragmentShaderInfo()
{
	assert(ShaderProgramType::Standard == m_type);
	return m_shaders[1];
}

const ShaderResource::ShaderInfo& ShaderResource::GetFragmentShaderInfo() const
{
	assert(ShaderProgramType::Standard == m_type);
	return m_shaders[1];
}

void ShaderResource::SetFragmentShaderInfo(ShaderInfo info)
{
	assert(ShaderProgramType::Standard == m_type);
	m_shaders[1] = cd::MoveTemp(info);
}

ShaderResource::ShaderInfo& ShaderResource::GetComputeShaderInfo()
{
	assert(ShaderProgramType::Compute == m_type);
	return m_shaders[0];
}

const ShaderResource::ShaderInfo& ShaderResource::GetComputeShaderInfo() const
{
	assert(ShaderProgramType::Compute == m_type);
	return m_shaders[0];
}

void ShaderResource::SetComputeShaderInfo(ShaderInfo info)
{
	assert(ShaderProgramType::Compute == m_type);
	m_shaders[0] = cd::MoveTemp(info);
}

bool ShaderResource::BuildShaderHandle()
{
	if (m_shaders[0].binBlob.empty())
	{
		return false;
	}

	assert(!bgfx::isValid(bgfx::ShaderHandle{ m_shaders[0].handle }));
	bgfx::ShaderHandle handle = bgfx::createShader(bgfx::makeRef(m_shaders[0].binBlob.data(), static_cast<uint32_t>(m_shaders[0].binBlob.size())));
	if (!bgfx::isValid(handle))
	{
		ClearShaderData(0);
		return false;
	}
	m_shaders[0].handle = handle.idx;

	if (ShaderProgramType::Standard == m_type)
	{
		if (m_shaders[1].binBlob.empty())
		{
			ClearShaderData(0);
			DistoryShaderHandle(0);
			return false;
		}

		assert(!bgfx::isValid(bgfx::ShaderHandle{ m_shaders[1].handle }));
		bgfx::ShaderHandle fragmentShaderHandle = bgfx::createShader(bgfx::makeRef(m_shaders[1].binBlob.data(), static_cast<uint32_t>(m_shaders[1].binBlob.size())));
		if (!bgfx::isValid(fragmentShaderHandle))
		{
			ClearShaderData(0);
			ClearShaderData(1);
			DistoryShaderHandle(0);
			return false;
		}
		m_shaders[1].handle = fragmentShaderHandle.idx;
	}

	return true;
}

bool ShaderResource::BuildProgramHandle()
{
	assert(!bgfx::isValid(bgfx::ProgramHandle{ m_programHandle }));

	if (ShaderProgramType::Standard == m_type)
	{
		m_programHandle = bgfx::createProgram(bgfx::ShaderHandle{ m_shaders[0].handle }, bgfx::ShaderHandle{ m_shaders[1].handle }).idx;
	}
	else if (ShaderProgramType::Compute == m_type || ShaderProgramType::VertexOnly == m_type)
	{
		m_programHandle = bgfx::createProgram(bgfx::ShaderHandle{ m_shaders[0].handle }).idx;
	}
	else
	{
		CD_WARN("Unknow shader program type of {}!", m_name);
	}

	if (!bgfx::isValid(bgfx::ProgramHandle{ m_programHandle }))
	{
		return false;
	}

	return true;
}

void ShaderResource::ClearShaderData(size_t index)
{
	m_shaders[index].binBlob.clear();
}

void ShaderResource::FreeShaderData(size_t index)
{
	ClearShaderData(index);
	ShaderBlob().swap(m_shaders[index].binBlob);
}

void ShaderResource::DistoryShaderHandle(size_t index)
{
	if (bgfx::isValid(bgfx::ShaderHandle{ m_shaders[index].handle }))
	{
		bgfx::destroy(bgfx::ShaderHandle{ m_shaders[index].handle });
		m_shaders[index].handle = bgfx::kInvalidHandle;
	}
}

void ShaderResource::DistoryProgramHandle()
{
	if (bgfx::isValid(bgfx::ProgramHandle{ m_programHandle }))
	{
		bgfx::destroy(bgfx::ProgramHandle{ m_programHandle });
		m_programHandle = bgfx::kInvalidHandle;
	}
}

}