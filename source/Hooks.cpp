#include "RE/B/BStimer.h"

#include "Hooks.h"

#include "GFxMoviePatcher.h"

#include <format>

namespace hooks
{
	namespace
	{
		std::string HexBytes(std::uintptr_t a_address, std::size_t a_count)
		{
			const auto* bytes = reinterpret_cast<const std::uint8_t*>(a_address);
			std::string out;
			for (std::size_t i = 0; i < a_count; ++i) {
				out += std::format("{}{:02X}", i ? " " : "", bytes[i]);
			}
			return out;
		}
	}

	bool SitesLookRight(std::uintptr_t a_advanceSite, std::uintptr_t a_initExtensionsSite)
	{
		const auto* advance = reinterpret_cast<const std::uint8_t*>(a_advanceSite);
		const auto* initExtensions = reinterpret_cast<const std::uint8_t*>(a_initExtensionsSite);

		std::string advanceBytes = HexBytes(a_advanceSite, 6);
		std::string initExtensionsBytes = HexBytes(a_initExtensionsSite, 5);

		const bool advanceOk = advance[0] == 0xFF && advance[1] == 0x90;
		const bool initExtensionsOk = initExtensions[0] == 0xE8;
		const std::uintptr_t base = REL::Module::get().base();

		logger::debug("LoadMovie Advance call site at +0x{:X}: {} ({})", a_advanceSite - base, advanceBytes,
			advanceOk ? "call [rax+disp32], as expected" : "NOT the expected call");
		logger::debug("LoadMovie InitExtensions call site at +0x{:X}: {} ({})", a_initExtensionsSite - base, initExtensionsBytes,
			initExtensionsOk ? "call rel32, as expected" : "NOT the expected call");

		iui::state::SetSiteBytes(std::move(advanceBytes), std::move(initExtensionsBytes));

		if (advanceOk && initExtensionsOk) {
			iui::state::siteCheck = 1;
			return true;
		}

		iui::state::siteCheck = -1;
		const bool alreadyPatched = (advance[0] == 0xFF && advance[1] == 0x25) || advance[0] == 0xE9 || initExtensions[0] == 0xE9;
		if (alreadyPatched) {
			logger::error("The two LoadMovie calls Infinity UI hooks are already patched - another copy of Infinity UI, or a "
						  "mod that hooks the same calls, loaded first. Remove the duplicate. Infinity UI will not patch any "
						  "menus this session; the game itself is unaffected.");
		} else {
			logger::error("BSScaleformManager::LoadMovie on this game version ({}) does not hold the calls Infinity UI hooks "
						  "where it expects them. Infinity UI will not patch any menus this session; the game itself is "
						  "unaffected. Please report this with your game version.",
				REL::Module::get().version().string());
		}
		return false;
	}

	void LogInstalled()
	{
		const std::uintptr_t base = REL::Module::get().base();
		logger::info("Hooked BSScaleformManager::LoadMovie (+0x{:X}): Advance call at +0x{:X}, InitExtensions call at +0x{:X}",
			BSScaleformManager::LoadMovie.address() - base, iui::state::advanceRva.load(), iui::state::initExtensionsRva.load());
		logger::debug("Original InitExtensions target (InvokeArgs) at +0x{:X}", GFxMovieRoot::InvokeArgs.address() - base);
	}

	void PatchGFxMovie(RE::GFxMovieRoot* a_movieRoot, float a_deltaT, std::uint32_t a_frameCatchUpCount, RE::IMenu* a_menu)
	{
		a_movieRoot->Advance(a_deltaT, a_frameCatchUpCount);

		IUI::GFxMoviePatcher moviePatcher{ a_menu, a_movieRoot };

		int loadedInstancesCount = moviePatcher.LoadInstancePatches();

		// GetMovieDef() is read straight after the movie loaded; guard it rather than assume it (rule 14).
		const RE::GFxMovieDef* movieDef = a_movieRoot->GetMovieDef();
		const char* movieRootFileUrl = movieDef ? movieDef->GetFileURL() : nullptr;
		if (!movieRootFileUrl) {
			movieRootFileUrl = "<unknown movie>";
		}

		iui::state::moviesLoaded++;
		iui::state::SetLastMovie(movieRootFileUrl);

		if (loadedInstancesCount)
		{
			iui::state::moviesWithPatches++;
			iui::state::instancesPatched += static_cast<std::uint64_t>(loadedInstancesCount);
			logger::info("Patches loaded for {}: {}", movieRootFileUrl, loadedInstancesCount);
		}
		else
		{
			logger::debug("Patches loaded for {}: 0", movieRootFileUrl);
		}
		logger::debug("");

		//if (loadedInstancesCount)
		//{
		//	IUI::g_moviePatchersList.emplace(a_movieRoot, moviePatcher);
		//}
	}

	void InitExtensionsAndNotify(RE::GFxMovieRoot* a_movieRoot, const char* a_methodName, const char* a_argFmt, RE::IMenu* a_menu, va_list args)
	{
		GFxMovieRoot::InvokeArgs(a_movieRoot, a_methodName, a_argFmt, args);

		iui::state::initExtensionsDispatched++;

		//if (IUI::g_moviePatchersList.contains(a_movieRoot))
		//{
			IUI::API::DispatchMessage(IUI::API::PostInitExtensionsMessage{ a_menu, a_movieRoot });
		//}
	}
}
