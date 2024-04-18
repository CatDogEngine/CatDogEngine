#pragma once

#ifdef SPDLOG_ENABLE

#include "Math/Quaternion.hpp"

#include <spdlog/spdlog.h>

namespace engine
{

class Log
{
public:
	static void Init();
	static std::shared_ptr<spdlog::logger> GetEngineLogger() { return s_pEngineLogger; }
	static std::shared_ptr<spdlog::logger> GetApplicationLogger() { return s_pApplicationLogger; }

	static const std::ostringstream& GetSpdOutput() { return m_oss; }
	static void ClearBuffer();

private:
	static std::shared_ptr<spdlog::logger> s_pEngineLogger;
	static std::shared_ptr<spdlog::logger> s_pApplicationLogger;

	// Note that m_oss will be cleared after OutputLog::AddSpdLog be called.
	static std::ostringstream m_oss;
};

}

// Engine log macros.
#define CD_ENGINE_TRACE(...) ::engine::Log::GetEngineLogger()->trace(__VA_ARGS__)
#define CD_ENGINE_INFO(...)  ::engine::Log::GetEngineLogger()->info(__VA_ARGS__)
#define CD_ENGINE_WARN(...)  ::engine::Log::GetEngineLogger()->warn(__VA_ARGS__)
#define CD_ENGINE_ERROR(...) ::engine::Log::GetEngineLogger()->error(__VA_ARGS__)
#define CD_ENGINE_FATAL(...) ::engine::Log::GetEngineLogger()->critical(__VA_ARGS__)

// Application log macros.
#define CD_TRACE(...) ::engine::Log::GetApplicationLogger()->trace(__VA_ARGS__)
#define CD_INFO(...)  ::engine::Log::GetApplicationLogger()->info(__VA_ARGS__)
#define CD_WARN(...)  ::engine::Log::GetApplicationLogger()->warn(__VA_ARGS__)
#define CD_ERROR(...) ::engine::Log::GetApplicationLogger()->error(__VA_ARGS__)
#define CD_FATAL(...) ::engine::Log::GetApplicationLogger()->critical(__VA_ARGS__)

// Runtime assert.
#define CD_ENGINE_ASSERT(x, ...) { if(!(x)) { CD_FATAL(...); __debugbreak(); } }
#define CD_ASSERT(x, ...) { if(!(x)) { CD_FATAL(...); __debugbreak(); } }

template<>
struct std::formatter<cd::Vec2f> : std::formatter<std::string>
{
	auto format(const cd::Vec2f& vec, std::format_context& context) const
	{
		return formatter<string>::format(std::format("vec2:({}, {})", vec.x(), vec.y()), context);
	}
};

template<>
struct std::formatter<cd::Vec3f> : std::formatter<std::string>
{
	auto format(const cd::Vec3f& vec, std::format_context& context) const
	{
		return formatter<string>::format(std::format("vec3:({}, {}, {})", vec.x(), vec.y(), vec.z()), context);
	}
};

template<>
struct std::formatter<cd::Vec4f> : std::formatter<std::string>
{
	auto format(const cd::Vec4f& vec, std::format_context& context) const
	{
		return formatter<string>::format(std::format("vec4:({}, {}, {}, {})", vec.x(), vec.y(), vec.z(), vec.w()), context);
	}
};

template<>
struct std::formatter<cd::Quaternion> : std::formatter<std::string>
{
	auto format(const cd::Quaternion& qua, std::format_context& context) const
	{
		return formatter<string>::format(std::format("Vector = ({}, {}, {}), Scalar = {}", qua.x(), qua.y(), qua.z(), qua.w()), context);
	}
};

#else

namespace engine
{
class Log
{
public:
	static void Init() {}
};
}

#define CD_ENGINE_TRACE(...)
#define CD_ENGINE_INFO(...) 
#define CD_ENGINE_WARN(...) 
#define CD_ENGINE_ERROR(...)
#define CD_ENGINE_FATAL(...)
#define CD_TRACE(...)
#define CD_INFO(...) 
#define CD_WARN(...) 
#define CD_ERROR(...)
#define CD_FATAL(...)

#define CD_ENGINE_ASSERT(x, ...)
#define CD_ASSERT(x, ...)

#endif