# OpenEmu library experience on Linux and Windows

This roadmap tracks the requested feature blueprint, including work still to
ship. Current preview: **0.8.0**, tested on Linux x86_64. Windows is a target,
not yet a working or tested release.

## Architecture to preserve

- Python/Qt owns the desktop interface. SQLite owns persistent library data.
  Additive, versioned upgrades must preserve existing libraries.
- Reusable C++ middleware owns emulation. Standard, unmodified Libretro cores
  run under this application's control. Never launch RetroArch or another
  standalone emulator. Keep the isolated game helper for crash containment.
- Retain functioning original core adapters while adding Libretro support.
  Avoid wholesale rewrites of working subsystems.
- Describe systems, core associations, BIOS requirements and capabilities in
  data. Hide emulator implementation details behind the native API.
- Share Linux/Windows logic and isolate platform services. Avoid Cocoa/AppKit.
  OpenEmu is the interaction reference, not the implementation architecture.
- Keep ROM filenames intact. Separate collection removal, library removal and
  confirmed disk deletion. Never unexpectedly move or delete original files.

## Stage 1 — foundation

**Working:** SQLite, isolated game processes, original GB/GBC/GBA/SNES adapters,
standard software-rendered Libretro hosting, audio, keyboard/player-one SDL
controls, battery saves, checked state slots, SNES launch checks. Six official
Libretro cores passed ten diagnostic core/system combinations.

**Further work:** consistent firmware/file validation, hardware rendering and
other Libretro environment interfaces, native capability/error reporting and
additional systems. These integrate in later stages without replacing the
working emulation path.

## Stage 2 — library experience (0.5.0)

**Working:** system icons; All Games, Recently Added (30 days), Recently Played,
Favorites and Never Played; cover grid and sortable/resizable list; shared
multi-selection and context actions; Return/double-click launch; cover size;
current-section search of title, system, filename and stored information;
regular collections with drag/drop, context additions, creation from selection,
rename/delete/remove membership; smart collections combining system, rating,
favorites, never-played and recent-date rules; batch 0–5 ratings; remembered
section, view, cover size, table widths and sort; editable information; original
filename preservation; additive numbered database migrations. Ratings and
favorites are independent.

**Polish ahead:** transitions, large-library rendering/model optimization,
saved selection/scroll per section and localization. Save States and Screenshots
navigation arrives with functional media libraries in Stage 5.

## Stage 3 — metadata, artwork and importing

**Already working:** background file/folder/ZIP import for supported cartridges,
SHA-256 duplicate detection, bounded archive reading, managed copies, filename
migration, OpenVGDB artwork identification, cached automatic covers, Libretro
thumbnail fallback, retry/cancellation and image replacement from file/drop.
Version 0.6 adds managed/external storage, system folders for new managed ROMs,
consolidation, scan/import progress and cancellation, persistent issue resolver,
unknown-system selection, empty/missing/permission/archive diagnostics, safe
folder traversal and hash-checked missing-ROM relinking with save-name continuity.
Version 0.7 adds the replaceable metadata-provider interface, automatic OpenVGDB
enrichment with source/retry records, raw ROM hashes, editable descriptions,
per-field manual-edit protection (including upgrades and stale editors), explicit
restoration of downloaded values, independent lookup/artwork settings and cancel
actions, and safe, scoped cover replacement with immutable image cache paths.

**Next:** deeper ROM-header identification and ambiguous core selection; disc
reference validation (expanded in Stage 7); confirmed disk deletion. Preserve all multi-file dependencies and
original source files. Disc formats currently report unsupported-system issues;
they are not yet imported or validated as complete game sets.

## Stage 4 — controllers and gameplay

**Already working:** keyboard and first SDL controller, hot-plug polling,
pause/reset, hold fast-forward, fullscreen game window, aspect ratio, integer
scaling, mute/volume and focus pause settings.
Version 0.8 adds the responsive auto-hiding HUD, mouse hiding, fullscreen launch
and clean fullscreen menu/status behavior, persistent volume/mute, 2–10×
fast-forward target, aspect preference, audio device selection/disconnect fallback
and buffer targets, audio suspension during pause/fast-forward and flushing on
state transitions. Existing shortcuts and nine state slots remain available.

**Next:** visual per-system/player/device binding editor, multiple players,
controller profiles/database, frontend action bindings, stable device
assignments, embedded/pop-out display choices, deeper audio/video synchronization
and device/performance testing. Hide advanced controls from the main library flow.

## Stage 5 — states, screenshots and history

**Already working:** separate battery/state storage, safe writes, periodic/exit
auto state and SRAM flush, nine slots, screenshots, last played and play count.
Libretro state compatibility includes core/build identity.

**Next:** continue/start prompt, dedicated quick slot, unlimited named states,
previews and creation/core/version records, state browser with rename/load/
delete/sort/grouping, screenshot browser with rename/delete/external open,
capture options, battery-save registry and periodic flushing independent of
serialization, play sessions and total play time. Never load an incompatible
core or ROM state.

## Stage 6 — visual options and advanced play

**Next:** reliable serialization capability and bounded rewind buffer; generic
core-option/display-mode interface; palettes and DS layouts via capabilities;
touch/pointer and multiple screens; persistent cheats via Libretro cheat APIs;
rendering interfaces and compatible shaders/presets/parameters where practical;
per-system/game display overrides; dynamic integer-scale choices. Unsupported
features must remain unavailable rather than exposing nonfunctional controls.

## Stage 7 — systems, firmware and discs

**Next:** data-driven system/core registry derived from installed information;
per-game core preferences and launch overrides; firmware requirements/checksums
and user-supplied BIOS import; cartridge/disc/tape capability interface; CUE/BIN,
CCD/IMG/SUB, ISO, CHD and M3U as supported by cores; one entry for multi-disc
games; remembered disc and eject/change/insert via Libretro disk APIs; safe
validation/copying of complete disc sets. Never bundle copyrighted BIOS files.

Additional target systems: Atari 2600/5200/7800/Lynx, ColecoVision, Famicom Disk
System, Game Gear, Intellivision, Neo Geo Pocket, NES/Famicom, Nintendo 64/DS,
Odyssey², PC Engine and PC Engine CD, SG-1000, Sega 32X/CD/Genesis/Master System/
Saturn, PlayStation, PSP, Vectrex, Virtual Boy, WonderSwan. Add capability and
launch tests as each becomes available; installing a core does not establish
that the host implements every interface it needs.

## Stage 8 — setup and cross-platform release

**Already working:** General/Core Selection/Core Downloads settings, official
Linux core download/update/import, immutable builds with integrity checks,
system defaults, artwork preferences and BIOS folder.

**Next:** setup assistant (library/storage/cores/artwork/controllers), safe
library relocation, visible-system preferences, resettable warnings, complete
gameplay settings, core removal/update availability/location/extensions/version
display and SQLite core registry, Linux/Windows discovery and installers,
platform abstractions for dynamic loading/locking/directories, packaging and
Windows validation, diagnostic log/viewer and actionable errors, localizable
Python strings/native error codes, UI polish. Optional Homebrew catalog later.

## Verification gates

After each stage, check new behavior and existing emulation, saving, importer
and artwork integration. Use diagnostic/homebrew ROMs; do not bundle commercial
games. Inspect rendered Qt windows at usable sizes. Test database upgrades and
verify ROM bytes/names, saves, metadata and IDs survive. Keep pending features
explicit. Shipping one stage does not mean the full blueprint is complete.
