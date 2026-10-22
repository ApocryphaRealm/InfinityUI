# Changelog - Infinity UI (ApocryphaRealm build)

A maintained build of alexsylex's Infinity UI (MIT, https://github.com/alexsylex/InfinityUI; Nexus 74483,
last release 2.0.3 in October 2024). Newest first. Each entry carries its version-ledger status (rule 61).

## 2.1.3 - untested

- **Fix: "failed to open address library file" on Skyrim 1.7.x.** The installer's first choice (SE 1.5.97 /
  AE 1.6.x) was pre-selected whatever the game, and that DLL cannot run on 1.7: its CommonLibSSE-NG 3.7
  treats a 1.7 game as SE and looks for `version-1-7-104-0.bin`, which does not exist (1.7 ships
  `versionlib-1-7-104-0.bin`, a database format 3.7 cannot read anyway), so it died with the library's bare
  message. Now:
  - the installer asks the mod manager for the game version and pre-selects the 1.7.x build on a 1.7 game
    (and the SE/AE build otherwise); the SE/AE option says it cannot start on 1.7;
  - both DLLs check the game version before touching the Address Library, and a build that does not match the
    game loads inert with a message that says which installer option to pick, instead of the bare error;
  - both DLLs log the Address Library file they need (`[AddressLibrary]`) and load inert with a message naming
    it when it is missing.
  No change to what the hooks do.

### Fixed
- The Skyrim 1.7.x DLL's log lost its first two lines (the 'Loading InfinityUI' line and the [AddressLibrary] line): CommonLibSSE-NG 7.2 started its own logger in SKSE::Init and recreated the file. It no longer does.
- The SE 1.5.97 / AE 1.6.x DLL on Skyrim 1.7.x still showed the bare 'failed to open address library file' and closed the game, because the hook's LoadMovie address was resolved by a static object built while the DLL loaded - before the build-line check could run. It is now resolved only when the hooks are installed, so the wrong-build message appears and the game continues.

### Changed
- The log now defaults to info (uLogLevel=2 in InfinityUI.ini, and the same when the INI is missing) instead of trace, so a normal session writes a short log. Set uLogLevel=0 to capture everything for a bug report.

## 2.1.1 - working (proven 2026-09-11 on Skyrim 1.7.104 and SE 1.5.97, after the strip rebuild)

- Rebuilt so the DLLs carry no build-machine paths (rule 45): CommonLibSSE-NG is compiled with
  vcpkg's installed directory relocated out of the project folder and its binary cache disabled, so
  the library's own absolute paths no longer name this project's build tree. No behaviour change -
  the version exists because an antivirus false-positive report is per file hash, so a rebuilt
  binary has to be distinguishable from the one it replaces.

## 2.1.0 - working (proven 2026-09-11 on Skyrim 1.7.104 and SE 1.5.97)

- **Skyrim 1.7.x support.** A second build line against CommonLibSSE-NG 7.2.0, which reads Address
  Library format 5 (1.7.99 / 1.7.104). The two `BSScaleformManager::LoadMovie` calls Infinity UI hooks
  were read from a dump of the running 1.7.104 game: they sit at the same offsets as on AE 1.6.1170
  (+0x399 for `Advance`, +0x3C0 for `InvokeArgs("_root.InitExtensions")`), with the same instructions and
  the same register holding the menu, so no address changed.
- The SE 1.5.97 / AE 1.6.x line builds against CommonLibSSE-NG 3.7, the same pinned port as the rest of
  this project's SE/AE builds.
- Before patching, both call sites are checked for the instructions the hooks replace. A mismatch - a
  future game update, or a second copy of Infinity UI that already patched them - skips the hooks and logs
  why, instead of writing a jump into the middle of another instruction.
- The compiled log level default is trace, matching the shipped `InfinityUI.ini` (upstream compiled
  `err` while shipping `uLogLevel=0`).
- The log opens with its active level, the game version and the build line.
- The movie definition is null-checked before its file URL is read.
- A read-only DevBench tool, `infinityui.state`: whether the hooks went in, the bytes found at each site,
  and how many movies were loaded, patched and had `InitExtensions` dispatched.
- Builds ship their debug symbols beside the DLL, carry no build-machine paths, and no longer copy
  themselves into a Steam game folder after building.
- Version 2.1.0 continues upstream's numbering: the last Nexus release is 2.0.3, and upstream's
  unreleased `CMakeLists.txt` already declared 2.1.0.
