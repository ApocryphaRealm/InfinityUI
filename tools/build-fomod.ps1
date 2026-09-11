<#
.SYNOPSIS
    Assembles the Infinity UI FOMOD package from the two built lines.

.DESCRIPTION
    Layout (the same shape as this project's other two-line FOMODs):
        Common\          SKSE\Plugins\InfinityUI.ini, README.txt, LICENSE-InfinityUI-MIT.txt
        Line-SE-AE16\    SKSE\Plugins\InfinityUI.{dll,pdb}   - build\relwithdebinfo-se-only (CommonLibSSE-NG 3.7)
        Line-17\         SKSE\Plugins\InfinityUI.{dll,pdb}   - build\relwithdebinfo-17 (CommonLibSSE-NG 7.2)
                         NOTICE.txt, LICENSE-GPL-3.0.txt, CommonLibSSE-NG-EXCEPTIONS.md
        fomod\           info.xml, ModuleConfig.xml (one step: which Skyrim)

    Each DLL and its PDB come from the same build directory, and each DLL must report -Version in its
    file-version resource, so a stale build cannot be packaged under a new number. The GPL texts come from
    the 1.7 build's own CommonLibSSE-NG install (the port installs COPYING as `copyright` plus
    EXCEPTIONS.md), so they are the library's verbatim texts.

.EXAMPLE
    .\tools\build-fomod.ps1 -Version 2.1.0 -OutDir "<test builds folder>"
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][ValidatePattern('^\d+\.\d+\.\d+$')][string]$Version,
    [Parameter(Mandatory = $true)][string]$OutDir
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$F = Join-Path $OutDir "Infinity UI $Version FOMOD"
if (Test-Path -LiteralPath $F) { throw "$F already exists - remove it first" }
$utf8 = New-Object System.Text.UTF8Encoding($false)

$lines = [ordered]@{ 'Line-SE-AE16' = 'relwithdebinfo-se-only'; 'Line-17' = 'relwithdebinfo-17' }
foreach ($l in $lines.Keys) {
    foreach ($ext in 'dll', 'pdb') {
        $p = Join-Path $repo "build\$($lines[$l])\InfinityUI.$ext"
        if (-not (Test-Path -LiteralPath $p)) { throw "missing $p - build that line first" }
    }
    $v = (Get-Item -LiteralPath (Join-Path $repo "build\$($lines[$l])\InfinityUI.dll")).VersionInfo.FileVersion
    if ($v -ne "$Version.0") { throw "$l DLL reports $v, not $Version.0 - rebuild it after the version bump" }
}
# The GPL texts come from the 1.7 line's own CommonLibSSE-NG install. That lives inside the build tree
# normally, and under C:\vcpkg-installed\ once the rule-45 strip has relocated VCPKG_INSTALLED_DIR.
$shareCandidates = @(
    (Join-Path $repo 'build\relwithdebinfo-17\vcpkg_installed\x64-windows-static-md\share\commonlibsse-ng'),
    'C:\vcpkg-installed\InfinityUI-17\x64-windows-static-md\share\commonlibsse-ng'
)
$share = $shareCandidates | Where-Object { (Test-Path -LiteralPath (Join-Path $_ 'copyright')) -and (Test-Path -LiteralPath (Join-Path $_ 'EXCEPTIONS.md')) } | Select-Object -First 1
if (-not $share) { throw "CommonLibSSE-NG's copyright/EXCEPTIONS.md not found - looked in:`n  " + ($shareCandidates -join "`n  ") }

foreach ($d in 'Common\SKSE\Plugins', 'fomod', 'Line-SE-AE16\SKSE\Plugins', 'Line-17\SKSE\Plugins') {
    New-Item -ItemType Directory -Force -Path (Join-Path $F $d) | Out-Null
}

Copy-Item -LiteralPath (Join-Path $repo 'dist\SKSE\Plugins\InfinityUI.ini') -Destination "$F\Common\SKSE\Plugins\"
$readme = [IO.File]::ReadAllText((Join-Path $repo 'dist\README.txt'))
[IO.File]::WriteAllText("$F\Common\README.txt", ([regex]::Replace($readme, '(?m)^Version \d+\.\d+\.\d+', "Version $Version", 'None')), $utf8)
Copy-Item -LiteralPath (Join-Path $repo 'LICENSE') -Destination "$F\Common\LICENSE-InfinityUI-MIT.txt"

foreach ($l in $lines.Keys) {
    foreach ($ext in 'dll', 'pdb') {
        Copy-Item -LiteralPath (Join-Path $repo "build\$($lines[$l])\InfinityUI.$ext") -Destination "$F\$l\SKSE\Plugins\"
    }
}
Copy-Item -LiteralPath (Join-Path $share 'copyright') -Destination "$F\Line-17\LICENSE-GPL-3.0.txt"
Copy-Item -LiteralPath (Join-Path $share 'EXCEPTIONS.md') -Destination "$F\Line-17\CommonLibSSE-NG-EXCEPTIONS.md"

$notice = @"
Infinity UI $Version - Skyrim 1.7.x build line

This DLL statically links CommonLibSSE-NG 7.2.0 (https://github.com/alandtse/CommonLibSSE-NG,
commit 7a60f4de794095d7b0f8928d1b930a52e9a7da83), which is licensed GPL-3.0-or-later WITH
Modding Exception AND GPL-3.0 Linking Exception. The combined work in this folder is therefore
distributed under the GNU General Public License v3.0 or later (LICENSE-GPL-3.0.txt), with the
library's exceptions (CommonLibSSE-NG-EXCEPTIONS.md).

Infinity UI's own source code remains MIT-licensed (alexsylex 2022, ApocryphaRealm 2026) and is the
corresponding source: https://github.com/ApocryphaRealm/InfinityUI (tag v$Version).

The SE 1.5.97 / AE 1.6.x build line (folder Line-SE-AE16) links CommonLibSSE-NG 3.7 (MIT) and is
MIT throughout.
"@
[IO.File]::WriteAllText("$F\Line-17\NOTICE.txt", $notice, $utf8)

$info = @"
<?xml version="1.0" encoding="UTF-8"?>
<fomod>
	<Name>Infinity UI</Name>
	<Author>alexsylex, maintained by ApocryphaRealm</Author>
	<Version>$Version</Version>
	<Website>https://github.com/ApocryphaRealm/InfinityUI</Website>
	<Description>Lets other mods replace pieces of the game's Scaleform menus as they load. Pick the build for your game version.</Description>
</fomod>
"@
[IO.File]::WriteAllText("$F\fomod\info.xml", $info, $utf8)

$config = @"
<?xml version="1.0" encoding="UTF-8"?>
<config xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://qconsulting.ca/fo3/ModConfig5.0.xsd">
	<moduleName>Infinity UI</moduleName>
	<requiredInstallFiles>
		<folder source="Common" destination="" priority="0" />
	</requiredInstallFiles>
	<installSteps order="Explicit">
		<installStep name="Which Skyrim do you have?">
			<optionalFileGroups order="Explicit">
				<group name="Game version" type="SelectExactlyOne">
					<plugins order="Explicit">
						<plugin name="Skyrim SE 1.5.97 or AE 1.6.1170">
							<description>For Skyrim Special Edition 1.5.97 and Anniversary Edition 1.6.x (1.6.1170 is the last Steam build before 1.7). One DLL covers both. Built on CommonLibSSE-NG 3.7 (MIT). Not sure which you have? Right-click SkyrimSE.exe, Properties, Details, and read the file version.</description>
							<files>
								<folder source="Line-SE-AE16" destination="" priority="0" />
							</files>
							<typeDescriptor>
								<type name="Recommended" />
							</typeDescriptor>
						</plugin>
						<plugin name="Skyrim 1.7.x (current Steam version)">
							<description>For Skyrim 1.7.99 and 1.7.104 - what Steam installs since 28 August 2026. Needs SKSE 2.3.x and Address Library for SKSE Plugins v13 or newer. Built on CommonLibSSE-NG 7.2.0; this DLL is distributed under GPL-3.0-or-later (the licence texts install with it).</description>
							<files>
								<folder source="Line-17" destination="" priority="0" />
							</files>
							<typeDescriptor>
								<type name="Optional" />
							</typeDescriptor>
						</plugin>
					</plugins>
				</group>
			</optionalFileGroups>
		</installStep>
	</installSteps>
</config>
"@
[IO.File]::WriteAllText("$F\fomod\ModuleConfig.xml", $config, $utf8)

"--- $F"
foreach ($l in $lines.Keys) {
    $d = Get-Item -LiteralPath "$F\$l\SKSE\Plugins\InfinityUI.dll"
    '{0,-13} dll {1} bytes v{2}; pdb {3:N1} MB' -f $l, $d.Length, $d.VersionInfo.FileVersion, ((Get-Item -LiteralPath "$F\$l\SKSE\Plugins\InfinityUI.pdb").Length / 1MB)
}
$sevenZip = Join-Path $env:ProgramFiles '7-Zip\7z.exe'
if (Test-Path -LiteralPath $sevenZip) {
    & $sevenZip a -tzip "$F.zip" "$F\*" -mx=5 | Out-Null
    "zip: {0:N1} MB (7z exit {1})" -f ((Get-Item -LiteralPath "$F.zip").Length / 1MB), $LASTEXITCODE
} else {
    "7-Zip not found; folder assembled, no zip"
}
