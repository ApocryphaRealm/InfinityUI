#include "Hooks.h"
#include "Settings.h"
#include "DevBenchTool.h"

#include "utils/Logger.h"

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

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	REL::Module::reset();

	const SKSE::PluginDeclaration* plugin = SKSE::PluginDeclaration::GetSingleton();

	if (!logger::init(plugin->GetName()))
	{
		return false;
	}

	logger::info("Loading {} {}...", plugin->GetName(), plugin->GetVersion());

	SKSE::Init(a_skse);

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
