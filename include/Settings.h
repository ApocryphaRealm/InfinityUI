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
		// Trace, matching the shipped InfinityUI.ini's uLogLevel=0 (rules 14 and 16) - upstream compiled
		// err here while shipping 0, so "no INI" and "the shipped INI" logged differently.
		inline logger::level logLevel = logger::level::trace;
	}
}