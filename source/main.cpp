#include "Hooks.h"
#include "Settings.h"
#include "DevBenchTool.h"

#include "utils/Logger.h"
#include "utils/AddressLibraryGuard.h"

namespace
{
	void OnSKSEMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		if (!a_msg) {
			return;
		}

		switch (a_msg->type) {
		case SKSE::MessagingInterface::kPostLoad:
			DevBenchTool::Init(false);
			break;
		case SKSE::MessagingInterface::kDataLoaded:
			DevBenchTool::Init(true);
			break;
		default:
			break;
		}
	}
}

namespace
{
	// This DLL is one of two build lines (RUNTIME_LINE): 1 reads SE 1.5.97 / AE 1.6.x, 17 reads Skyrim 1.7.x. The
	// installer asks which game the player has, and a wrong pick surfaced as CommonLibSSE-NG's bare "failed to
	// open address library file" (Nexus bug report, Skyrim 1.7.104 with the SE/AE DLL): that library calls a 1.7
	// game "SE" and looks for version-1-7-104-0.bin, which does not exist (1.7 ships versionlib-1-7-104-0.bin, a
	// format the 3.7 library cannot read anyway). Say so before anything resolves an address.
	bool BuildLineMatchesGame()
	{
		const auto ver = REL::Module::get().version();
		const bool game17 = ver[1] >= 7;
		const bool ok = RUNTIME_LINE == 17 ? ver[1] >= 6 : !game17;
		if (ok) {
			return true;
		}
		const std::string text = std::format(
			"Infinity UI cannot start: this is the {} build, but the game is Skyrim {}.{}.{}.\n\n"
			"Run the Infinity UI installer again and choose \"{}\". Updating Address Library will not change this.\n\n"
			"The plugin has loaded inert; the game continues.",
			RUNTIME_LINE == 17 ? "Skyrim 1.7.x" : "SE 1.5.97 / AE 1.6.x", ver[0], ver[1], ver[2],
			game17 ? "Skyrim 1.7.x (current Steam version)" : "Skyrim SE 1.5.97 or AE 1.6.1170");
		logger::critical("[BuildLine] {}", text);
		MessageBoxA(nullptr, text.c_str(), "Infinity UI", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
		return false;
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	REL::Module::reset();

	// Both checks run before SKSE::Init, which opens the Address Library itself (see AddressLibraryGuard.h).
	if (!BuildLineMatchesGame() || !AddressLibraryGuard::Guard("Infinity UI")) {
		return true;
	}

	const SKSE::PluginDeclaration* plugin = SKSE::PluginDeclaration::GetSingleton();

	if (!logger::init(plugin->GetName()))
	{
		return false;
	}

	logger::info("Loading {} {}...", plugin->GetName(), plugin->GetVersion());
	if (const auto& r = AddressLibraryGuard::Last(); !r.runtime.empty()) {
		logger::info("[AddressLibrary] runtime {} ({}); opened \"{}\": present", r.runtime, r.edition, r.file);
	}

#if RUNTIME_LINE == 17
	// CommonLibSSE-NG 7.2's Init(a_skse, a_log = true) starts its own logger, which recreates InfinityUI.log empty
	// and wipes the two lines above (the 1.7.104 log of the 2.1.3 test opened at "InfinityUI v2-1-3-0"). This
	// plugin's logger is already running, so CommonLib's is not wanted.
	SKSE::Init(a_skse, false);
#else
	SKSE::Init(a_skse);
#endif

	settings::Init(std::string(plugin->GetName()) + ".ini");

	logger::set_level(settings::debug::logLevel, settings::debug::logLevel);

	// Written at the active level so it always clears the threshold (rule 14): a log someone sends in says
	// what it captured without needing the INI that produced it.
	logger::at_level(settings::debug::logLevel,
		"Log level {} - 0 trace, 1 debug, 2 info, 3 warning, 4 error, 5 critical; lines below it are dropped. "
		"Set uLogLevel in InfinityUI.ini (0 captures everything).",
		static_cast<int>(settings::debug::logLevel));
	logger::at_level(settings::debug::logLevel, "Game {} - {} build line", REL::Module::get().version().string(),
		RUNTIME_LINE == 17 ? "Skyrim 1.7.x" : "SE 1.5.97 / AE 1.6.x");

	hooks::Install();

	if (auto* messaging = SKSE::GetMessagingInterface()) {
		messaging->RegisterListener(OnSKSEMessage);
	}

	logger::set_level(logger::level::info, logger::level::info);
	logger::info("{}", iui::state::hooksInstalled ? "Succesfully loaded!" :
												   "Loaded, but no menus will be patched this session - see the error above.");

	logger::set_level(settings::debug::logLevel, settings::debug::logLevel);

	return true;
}
