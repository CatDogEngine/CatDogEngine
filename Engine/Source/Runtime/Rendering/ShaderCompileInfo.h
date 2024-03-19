#pragma once

#include "Base/Template.h"
#include "ECWorld/Entity.h"

#include <set>
#include <string>

namespace engine
{

class ShaderCompileInfo
{
public:
	ShaderCompileInfo(std::string name, std::string combine = "") :
		m_programName(cd::MoveTemp(name)), m_featuresCombine(cd::MoveTemp(combine)) {}
	ShaderCompileInfo() = delete;
	ShaderCompileInfo(const ShaderCompileInfo&) = default;
	ShaderCompileInfo& operator=(const ShaderCompileInfo&) = default;
	ShaderCompileInfo(ShaderCompileInfo&&) = default;
	ShaderCompileInfo& operator=(ShaderCompileInfo&&) = default;
	~ShaderCompileInfo() = default;

	bool operator==(const ShaderCompileInfo& other) const
	{
		return (m_programName == other.m_programName) && (m_featuresCombine == other.m_featuresCombine);
	}

	bool operator<(const ShaderCompileInfo& other) const
	{
		if (m_programName < other.m_programName)
		{
			return true;
		}
		if (m_programName > other.m_programName)
		{
			return false;
		}

		return m_featuresCombine < other.m_featuresCombine;
	}

	std::string& GetProgramName() { return m_programName; }
	const std::string& GetProgramName() const { return m_programName; }
	void SetProgramName(std::string name) { m_programName = cd::MoveTemp(name); }

	std::string& GetFeaturesCombine() { return m_featuresCombine; }
	const std::string& GetFeaturesCombine() const { return m_featuresCombine; }
	void SetFeaturesCombine(std::string combine) { m_featuresCombine = cd::MoveTemp(combine); }

	void AddTaskHandle(uint32_t handle) { m_taskHandles.insert(handle); }
	std::set<uint32_t>& GetTaskHandles() { return m_taskHandles; }
	const std::set<uint32_t>& GetTaskHandles() const { return m_taskHandles; }
	void SetTaskHandles(std::set<uint32_t> handles) { m_taskHandles = cd::MoveTemp(handles); }

private:
	std::string m_programName;
	std::string m_featuresCombine;
	std::set<uint32_t> m_taskHandles;
};

}