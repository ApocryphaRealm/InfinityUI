Infinity UI
===========
Version 2.1.3

This build's own number. alexsylex's last Nexus release of Infinity UI is 2.0.3 (October 2024); this
continues from there.

WHAT IT IS
----------
A framework other mods use to swap pieces of the game's Scaleform menus - the HUD compass, the local
map and so on - for their own, while each menu is loading. On its own it changes nothing you can see.
Compass Navigation Overhaul, Local Map Upgrade, Dragon's Eye Minimap and other mods need it.

This is a maintained build of alexsylex's Infinity UI, whose author is no longer active on Nexus, with
a build for the current Steam version of Skyrim added.

WHICH BUILD
-----------
The installer asks which Skyrim you have:
  * Skyrim SE 1.5.97 or AE 1.6.1170 - one DLL covers both.
  * Skyrim 1.7.x - what Steam installs since 28 August 2026. Needs SKSE 2.3.x and Address Library for
    SKSE Plugins v13 or newer.
Not sure? Right-click SkyrimSE.exe, Properties, Details, and read the file version. The SE/AE DLL cannot
run on 1.7.x: pick by version. A DLL that does not match the game loads inert and says so.

INSTALLING
----------
This replaces the original Infinity UI - both are the same plugin, InfinityUI.dll. Disable or remove
the original so only one is installed. Mods that need Infinity UI keep working unchanged.

REQUIREMENTS
------------
  * SKSE (2.0.20 for 1.5.97, 2.2.x for 1.6.1170, 2.3.x for 1.7.x)
  * Address Library for SKSE Plugins

LOG
---
Documents\My Games\Skyrim Special Edition\SKSE\InfinityUI.log - it opens with the game version and
build line. Include it with any bug report. uLogLevel in SKSE\Plugins\InfinityUI.ini sets how much it
records (0 = everything, the default).

If the game version is one Infinity UI does not recognise, the log says so and Infinity UI leaves the
game untouched instead of patching blind.

LICENCE
-------
GPL-3.0-or-later - ApocryphaRealm (2026), from alexsylex's MIT original (2022; notice in THIRD_PARTY_NOTICES.md). The Skyrim 1.7.x DLL statically links CommonLibSSE-NG
7.2.0 and is distributed under GPL-3.0-or-later; NOTICE.txt beside it explains.

Source: https://github.com/ApocryphaRealm/InfinityUI

WHAT CHANGED

Version 2.1.3
Fixed "failed to open address library file" on Skyrim 1.7.x: the installer now pre-selects the build that matches your game, and a DLL that does not match the game loads inert with a message that says which option to pick, instead of CommonLibSSE-NG's bare error.

Version 2.1.2
Relicensed under GPL-3.0-or-later, as the libraries it is built on require; no gameplay change.
