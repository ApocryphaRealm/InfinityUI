#pragma once

#include "utils/Trampoline.h"

#include "RE/B/BSCoreTypes.h"

#include "State.h"

namespace hooks
{
	class BSScaleformManager
	{
		static constexpr REL::RelocationID LoadMovieId{ 80302, 82325 };

	public:

		static inline REL::Relocation<bool(*)(const RE::BSScaleformManager*, RE::IMenu*,
											  RE::GPtr<RE::GFxMovieView>&, const char*,
											  RE::GFxMovieView::ScaleModeType, float)> LoadMovie{ LoadMovieId };
	};

	class GFxMovieRoot
	{
	public:

		static inline REL::Relocation<const char*(*)(RE::GFxMovieRoot*, const char*, const char*, va_list)> InvokeArgs;
	};

	void PatchGFxMovie(RE::GFxMovieRoot* a_movieRoot, float a_deltaT, std::uint32_t a_frameCatchUpCount, RE::IMenu* a_menu);
	void InitExtensionsAndNotify(RE::GFxMovieRoot* a_movieRoot, const char* a_methodName, const char* a_argFmt,
								 RE::IMenu* a_menu, va_list args);

	// Where the two calls sit inside BSScaleformManager::LoadMovie. SE 1.5.97 has them two bytes later than
	// the later builds: AE 1.6.1170 and Skyrim 1.7.104 both have them at +0x399 and +0x3C0 (1.7.104 read from
	// a dump of the running game on 2026-09-11 - around both calls the function is instruction-for-
	// instruction the same as SE, and rdi is reloaded from the IMenu* argument's home slot in both).
	inline std::uintptr_t AdvanceCallOffset() { return REL::Module::IsSE() ? 0x39B : 0x399; }
	inline std::uintptr_t InitExtensionsCallOffset() { return REL::Module::IsSE() ? 0x3C2 : 0x3C0; }

	// Checks that both sites hold the instructions the hooks replace before anything is written:
	//   Advance call         FF 90 xx xx xx xx   call qword ptr [rax + disp32]   (6 bytes)
	//   InitExtensions call  E8 xx xx xx xx      call rel32                      (5 bytes)
	// A game update that moves them, or another copy of this plugin that already patched them, fails the
	// check; Infinity UI then leaves the game untouched and logs why, instead of writing a jump into the
	// middle of some other instruction.
	bool SitesLookRight(std::uintptr_t a_advanceSite, std::uintptr_t a_initExtensionsSite);
	void LogInstalled();

	static inline void Install()
	{
		// BSScaleformManager::LoadMovie call to movieView->Advance(0.0F, 2)
		struct PatchGFxMovieHook : Hook<6>
		{
			struct HookCodeGenerator : Xbyak::CodeGenerator
			{
				HookCodeGenerator(std::uintptr_t a_hookedAddress)
				{
					Xbyak::Label hookLabel;
					Xbyak::Label retnLabel;

					mov(r9, rdi);  // rdi = IMenu*
					call(ptr[rip + hookLabel]);

					jmp(ptr[rip + retnLabel]);

					L(hookLabel), dq(reinterpret_cast<std::uintptr_t>(&PatchGFxMovie));
					L(retnLabel), dq(a_hookedAddress + 6);

					ready();
				}
			};

			PatchGFxMovieHook(std::uintptr_t a_hookedAddress) :
				Hook<6>{ a_hookedAddress, HookCodeGenerator{ a_hookedAddress } }
			{}
		};

		// BSScaleformManager::LoadMovie call to movieView->InvokeArgs("_root.InitExtensions", nullptr, nullptr)
		struct InitExtensionsAndNotifyHook : Hook<5>
		{
			struct HookCodeGenerator : Xbyak::CodeGenerator
			{
				HookCodeGenerator(std::uintptr_t a_hookedAddress)
				{
					Xbyak::Label hookLabel;
					Xbyak::Label retnLabel;

					mov(r9, rdi);  // rdi = IMenu*
					call(ptr[rip + hookLabel]);

					jmp(ptr[rip + retnLabel]);

					L(hookLabel), dq(reinterpret_cast<std::uintptr_t>(&InitExtensionsAndNotify));
					L(retnLabel), dq(a_hookedAddress + 5);

					ready();
				}
			};

			InitExtensionsAndNotifyHook(std::uintptr_t a_hookedAddress) :
				Hook<5>{ a_hookedAddress, HookCodeGenerator{ a_hookedAddress } }
			{}
		};

		const std::uintptr_t base = REL::Module::get().base();
		const std::uintptr_t loadMovie = BSScaleformManager::LoadMovie.address();
		const std::uintptr_t advanceSite = loadMovie + AdvanceCallOffset();
		const std::uintptr_t initExtensionsSite = loadMovie + InitExtensionsCallOffset();

		iui::state::loadMovieRva = loadMovie - base;
		iui::state::advanceRva = advanceSite - base;
		iui::state::initExtensionsRva = initExtensionsSite - base;

		if (!SitesLookRight(advanceSite, initExtensionsSite)) {
			return;
		}

		PatchGFxMovieHook patchGFxMovieHook{ advanceSite };
		InitExtensionsAndNotifyHook initExtensionsAndNotifyHook{ initExtensionsSite };

		static DefaultTrampoline defaultTrampoline{ patchGFxMovieHook.getSize() + initExtensionsAndNotifyHook.getSize() };

		defaultTrampoline.write_branch(patchGFxMovieHook);
		GFxMovieRoot::InvokeArgs = defaultTrampoline.write_branch(initExtensionsAndNotifyHook);

		iui::state::hooksInstalled = true;
		LogInstalled();
	}
}
