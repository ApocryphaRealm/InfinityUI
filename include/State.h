#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>

// Live state for the "infinityui.state" DevBench tool (rule 31). Written where each decision is made and
// read from DevBench's own listener thread, so every field is either atomic or behind the mutex.
namespace iui::state
{
	inline std::atomic<bool> hooksInstalled{ false };
	inline std::atomic<int>  siteCheck{ 0 };  // 0 not run yet, 1 both sites as expected, -1 refused

	inline std::atomic<std::uint64_t> loadMovieRva{ 0 };
	inline std::atomic<std::uint64_t> advanceRva{ 0 };
	inline std::atomic<std::uint64_t> initExtensionsRva{ 0 };

	inline std::atomic<std::uint64_t> moviesLoaded{ 0 };
	inline std::atomic<std::uint64_t> moviesWithPatches{ 0 };
	inline std::atomic<std::uint64_t> instancesPatched{ 0 };
	inline std::atomic<std::uint64_t> initExtensionsDispatched{ 0 };

	inline std::mutex  mutex;
	inline std::string advanceBytes;
	inline std::string initExtensionsBytes;
	inline std::string lastMovie;

	inline void SetSiteBytes(std::string a_advance, std::string a_initExtensions)
	{
		std::scoped_lock lock{ mutex };
		advanceBytes = std::move(a_advance);
		initExtensionsBytes = std::move(a_initExtensions);
	}

	inline void SetLastMovie(const char* a_url)
	{
		std::scoped_lock lock{ mutex };
		lastMovie = a_url ? a_url : "";
	}
}
