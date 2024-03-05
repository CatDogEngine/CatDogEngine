#pragma once

#include "base/Platform.h"

#include <string>

namespace engine
{

enum class ShaderType
{
	None,
	Compute,
	Vertex,
	Fragment
};

inline const ShaderType GetShaderType(const std::string& fileName)
{
	if (fileName._Starts_with("vs_") || fileName._Starts_with("VS_"))
	{
		return ShaderType::Vertex;
	}
	else if (fileName._Starts_with("fs_") || fileName._Starts_with("FS_"))
	{
		return ShaderType::Fragment;
	}
	else if (fileName._Starts_with("cs_") || fileName._Starts_with("CS_"))
	{
		return ShaderType::Compute;
	}
	else
	{
		return ShaderType::None;
	}
}

}