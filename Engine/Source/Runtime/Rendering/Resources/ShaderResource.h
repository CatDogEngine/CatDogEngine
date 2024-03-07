#pragma once

#include "Base/Template.h"
#include "IResource.h"
#include "Rendering/ShaderType.h"

#include <string>
#include <vector>

// The granularity of the ShaderResource is "per Uber Shader Program".

namespace engine
{

// TODO : Template?
class ShaderResource : public IResource
{
public:
	using ShaderBlob = std::vector<std::byte>;

	struct ShaderInfo
	{
		std::string name;
		std::string binPath;

		ShaderBlob blob;
		uint16_t handle = UINT16_MAX;
	};

public:
	ShaderResource() = default;
	ShaderResource(const ShaderResource&) = default;
	ShaderResource& operator=(const ShaderResource&) = default;
	ShaderResource(ShaderResource&&) = default;
	ShaderResource& operator=(ShaderResource&&) = default;
	virtual ~ShaderResource();

	virtual void Update() override;
	virtual void Reset() override;

	ShaderProgramType GetType() const { return m_type; }
	void SetType(ShaderProgramType type) { m_type = type; }

	std::string& GetName() { return m_programName; }
	const std::string& GetName() const { return m_programName; }
	void SetName(std::string name) { m_programName = cd::MoveTemp(name); }

	ShaderInfo& GetVertexShaderInfo();
	const ShaderInfo& GetVertexShaderInfo() const;
	void SetVertexShaderInfo(ShaderInfo info);

	ShaderInfo& GetFragmentShaderInfo();
	const ShaderInfo& GetFragmentShaderInfo() const;
	void SetFragmentShaderInfo(ShaderInfo info);

	ShaderInfo& GetComputeShaderInfo();
	const ShaderInfo& GetComputeShaderInfo() const;
	void SetComputeShaderInfo(ShaderInfo info);

	uint16_t GetHandle() const { return m_programHandle; }

private:
	bool BuildShaderHandle();
	bool BuildProgramHandle();

	void FreeShaderData(size_t index);
	void DistoryShaderHandle(size_t index);
	void DistoryProgramHandle();

	ShaderInfo m_shaders[2];

	// Runtime
	ShaderProgramType m_type = ShaderProgramType::None;
	std::string m_programName;
	uint32_t m_recycleCount = 0;

	// GPU
	uint16_t m_programHandle = UINT16_MAX;

	// TODO : Uber Shader
};

}