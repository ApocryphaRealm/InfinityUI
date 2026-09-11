#include "DevBenchTool.h"

#include "DevBench/DevBenchAPI.h"
#include "Settings.h"
#include "State.h"

#include <format>
#include <string>
#include <string_view>

namespace DevBenchTool
{
	namespace
	{
		std::string EscapeJson(std::string_view a_in)
		{
			std::string out;
			out.reserve(a_in.size() + 8);
			for (const char c : a_in) {
				switch (c) {
				case '\\': out += "\\\\"; break;
				case '"': out += "\\\""; break;
				case '\n': out += "\\n"; break;
				default: out += c; break;
				}
			}
			return out;
		}

		const char* SiteCheckName(int a_value)
		{
			switch (a_value) {
			case 1: return "ok";
			case -1: return "refused";
			default: return "not run";
			}
		}

		void StateTool(void*, const char*, void* a_sink, DevBenchAPI::WriteFn a_write)
		{
			using namespace iui::state;

			std::string advance, initExt, last;
			{
				std::scoped_lock lock{ mutex };
				advance = advanceBytes;
				initExt = initExtensionsBytes;
				last = lastMovie;
			}

			const auto* plugin = SKSE::PluginDeclaration::GetSingleton();
			const std::string json = std::format(
				"{{\"ok\":true,\"plugin\":\"InfinityUI\",\"version\":\"{}\",\"buildLine\":\"{}\",\"game\":\"{}\",\"logLevel\":{},"
				"\"hooks\":{{\"installed\":{},\"siteCheck\":\"{}\",\"loadMovieRva\":\"0x{:X}\","
				"\"advanceRva\":\"0x{:X}\",\"advanceBytes\":\"{}\",\"initExtensionsRva\":\"0x{:X}\",\"initExtensionsBytes\":\"{}\"}},"
				"\"movies\":{{\"loaded\":{},\"withPatches\":{},\"instancesPatched\":{},\"initExtensionsDispatched\":{},\"last\":\"{}\"}}}}",
				plugin ? plugin->GetVersion().string() : "?", RUNTIME_LINE == 17 ? "1.7.x" : "SE/AE",
				REL::Module::get().version().string(), static_cast<int>(settings::debug::logLevel),
				hooksInstalled.load(), SiteCheckName(siteCheck.load()), loadMovieRva.load(),
				advanceRva.load(), advance, initExtensionsRva.load(), initExt,
				moviesLoaded.load(), moviesWithPatches.load(), instancesPatched.load(), initExtensionsDispatched.load(),
				EscapeJson(last));
			a_write(a_sink, json.c_str());
		}
	}

	void Init(bool a_lastAttempt)
	{
		static bool registered = false;
		if (registered) {
			return;
		}

		DevBenchAPI::IDevBenchInterface001* devBench = DevBenchAPI::GetDevBenchInterface001();
		if (!devBench) {
			if (a_lastAttempt) {
				logger::info("DevBench not detected; skipping the \"infinityui.state\" tool");
			} else {
				logger::debug("DevBench not detected yet; will retry at kDataLoaded");
			}
			return;
		}

		constexpr const char* descriptor =
			"{"
			"\"description\":\"Infinity UI live state: plugin version, build line, game version, whether the two "
			"BSScaleformManager::LoadMovie hooks were installed and the bytes found at each site, and how many movies "
			"were loaded, patched, and had InitExtensions dispatched, with the last movie's URL.\","
			"\"inputSchema\":{\"type\":\"object\",\"properties\":{}},"
			"\"readOnly\":true"
			"}";

		if (devBench->RegisterTool("infinityui.state", descriptor, &StateTool, nullptr)) {
			logger::info("Registered \"infinityui.state\" with DevBench (build {})", devBench->GetBuildNumber());
			registered = true;
		}
	}
}
