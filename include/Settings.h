#pragma once

namespace SKSE::log
{
	using level = spdlog::level::level_enum;
}
namespace logger = SKSE::log;

namespace settings
{
	void Init(const std::string& a_iniFileName);

	// Default values

	namespace debug
	{
		// Info, matching the shipped InfinityUI.ini's uLogLevel=2 (rules 14 and 16). A reporter sets 0 (trace)
		// in the INI to capture everything; "no INI" and "the shipped INI" log the same.
		inline logger::level logLevel = logger::level::info;
	}
}