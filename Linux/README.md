# OpenEmu Linux — community port

This OpenEmu source fork includes a native Linux library window and Linux
adapters for the original, pinned Gambatte, mGBA and Snes9x cores. This is an
early community port; it is not an official OpenEmu Linux release or a complete
port of every macOS feature.

## Run the existing build

Run **Launch OpenEmu Linux.sh** in the repository root, or run `./Linux/run.sh`.
The current `Linux/build/cores` binaries were built and tested on Fedora 44
x86_64. Python 3 and PySide6 are required; SDL2 enables game controllers.

Use **Import games** or drop ROMs onto the library. SNES supports `.sfc`, `.smc`,
`.snes`, `.fig`, and ZIP archives containing these files. Headered `.smc` files
are supported. Choose **Super Nintendo** in the sidebar, then double-click a
game. Standard SNES cartridges do not require a separate BIOS.

**Try the test ROMs** imports the original diagnostic cartridges bundled with
this port. The SNES diagnostic displays a marker for each pressed button and
plays a quiet tone. It also exercises battery saving. No commercial games are
included.

## Library views, collections and ratings

Version **0.5.0** adds **Grid / List**, adjustable covers and multiple selection
(Ctrl/Shift-click). The list has sortable, resizable columns for title, system,
rating, last played, date added, play count, core and file status. Both views
share game actions and selection; double-click or press Return to launch one
selected game. The last section, view, cover size, widths and sort are remembered.

Search matches title, system name, original filename and stored information
inside the current section or collection. Clearing search restores the section.
**Recently added** shows the last 30 days. **Recently played** and **Never played**
update as games are launched.

Use **+ New collection** for regular or smart collections. Drag games from either
view onto a regular collection, or use **Add to collection** in the game menu.
**Create collection from selection** supports different systems. Right-click a
collection to rename/delete it or edit its smart rules. **Remove from collection**
only changes membership. Smart collections automatically match all chosen system,
minimum rating, favorites, never played and recent-date rules.

Right-click selected games to set **Rating** from unrated to five stars. Ratings
also appear in **Game information**, with editable title, region, developer,
publisher, release date, genre, players and notes. These details are stored
locally and can be enriched automatically as described below.
Editing them preserves original ROM names and paths. **Copy ROM path(s)**
and **Open ROM folder** locate the files.

Drop an image onto a game or choose **Add cover art from file…** to replace its
cover. Automatic downloads preserve manually chosen images. Database updates
are additive and versioned; collections, ratings and information survive restart.

See [ROADMAP.md](ROADMAP.md) for the complete feature plan and stage status.
Named state/screenshot libraries, advanced controllers, BIOS/disc management,
shaders, more systems and Windows packaging remain in later stages.

## Settings and RetroArch / Libretro cores

Open **Library → Settings…** (`Ctrl+,`). Settings are saved automatically.
The General tab includes game volume, pause on focus loss, integer scaling,
artwork preferences and a BIOS/system folder. Game settings apply on the next
launch; artwork preferences also affect the library immediately.

The **Library** tab includes **Copy games into library when importing**. It is
enabled by default. Turn it off to keep new, uncompressed ROMs in their existing
folders. Existing entries are unaffected. ZIP imports always extract managed
copies, since games need a real ROM file when launched.

Version **0.6.0** adds visible scan/import progress, cancellation between files,
separate new-game/duplicate counts and persistent **Import issues**. Unknown
files, empty files, unreadable/missing files and damaged/unsupported archives
are explained in the resolver. Select an available system for an unknown file
and retry, choose a replacement file, or dismiss the issue. The issues button
and **Library → Import issues…** reopen unresolved problems after a restart.
Folder scans skip hidden directories and common documentation, image and save
sidecars, avoid directory symlink loops, and do not scan the managed library
back into itself. Explicit file selections always receive a result.

Right-click external games and choose **Consolidate files into library** to
copy them without moving/deleting their originals. **Locate missing ROM…**
reconnects an existing game only after its SHA-256 matches. Titles, ratings and
save identity remain intact. Launching also checks for missing/changed ROMs
before initializing a core. Disc-based imports still await compatible systems
and complete multi-file/disc support; they currently receive an explanatory
issue instead of becoming broken library entries.

In **Core downloads**, select a core and click **Download / update selected**.
The installer downloads the official Linux x86_64 build from
[Libretro's build server](https://buildbot.libretro.com/nightly/linux/x86_64/latest/).
Then choose it for a system in **Core selection**. Each system remembers its
own selection; the original bundled OpenEmu cores remain selectable.

| Downloadable core | Systems offered |
| --- | --- |
| Gambatte, SameBoy | Game Boy, Game Boy Color |
| mGBA | Game Boy, Game Boy Color, Game Boy Advance |
| Snes9x, Snes9x 2010, bsnes | Super Nintendo |

**Update all installed** checks all cores downloaded by the app, skipping
unchanged binaries. Downloads are checked against the server's CRC, extracted
without archive paths, checked for the correct Linux architecture, and probed
in a separate process before installation. Installation is atomic. Core builds
are stored by SHA-256, so running games and previous installations are not
overwritten by an update. Updates run only when requested.

**Import core file…** copies an existing Linux `.so` Libretro core into the
library, including a core from your RetroArch installation. The original file
is preserved. Imported cores are not updated from the official server; import
a newer file to add another build. Available systems are determined from the
core's advertised ROM formats.

These are the actual Libretro shared libraries, hosted directly by this app;
RetroArch does not need to be installed. The host supports software video
(RGB565, 0RGB1555, XRGB8888), a single joypad, stereo audio converted to 48 kHz,
core default options, serialization, battery RAM and RTC storage. The current
library supports GB, GBC, GBA and SNES. Hardware-rendered cores, additional
systems/input devices and editing individual Libretro core options are not
implemented yet. Some imported cores may require optional interfaces or BIOS
files that this frontend does not supply. Select required BIOS files' folder
in General; BIOS files are not downloaded.

Battery saves are separated by core, and save states by core **and build**.
Switching cores does not convert saves between their formats. Older saves and
builds remain on disk. A missing or damaged selected core produces an error
and can be downloaded again or replaced through Core selection.

## System icons and box art

Version **0.7.0** adds automatic game information from OpenVGDB on startup and
after imports. It shares the locally cached catalog with artwork, matches the
ROM hash and system, and prefers the local region. Title, region, developer,
publisher, release date, genre and description are filled when present. Missing
fields stay blank; the current catalog has no player-count field. Library titles
can change after identification; physical ROM filenames and save identities do
not change.

**Game information → Look up game information** retries now; a selected game's
context menu can limit lookup to that game. Disable automatic lookup from that
menu or **Settings → Library**. Information lookup and artwork settings are
independent, and each has its own cancel command. Startup/import work runs one
catalog job at a time. Network cancellation can wait for the active read timeout.

Manual titles and field edits—including intentionally cleared fields—are kept
through automatic updates. Existing custom information is protected during the
schema upgrade. Saving an editor that was open during a lookup publishes only
controls the user actually changed. **Use downloaded information…** explicitly
restores fields for which a cached match has values, retaining notes and fields
the provider cannot supply.

The information editor shows the source, last check, description and available
ROM hashes. New imports calculate MD5, SHA-1 and CRC32 locally even with automatic
lookup off; SHA-256 remains the library identity. Older libraries acquire the
additional hashes during lookup. Unmatched games retry after 30 days, failures
after one hour; explicit retry bypasses this delay. A failed refresh keeps cached
information. Changed external ROMs are rejected before metadata/artwork lookup.

Providers implement a small `MetadataProvider` interface. SQLite stores ordinary
library fields and provider provenance; it does not mirror the OpenVGDB schema.
Other providers can supply the same fields without changing the database or UI.

The sidebar uses OpenEmu's original console icons. Game Boy and Game Boy Color
share OpenEmu's Game Boy system icon.

Box art downloads automatically in the background after importing games and
when opening a library that has missing covers. **Artwork → Download missing
box art** retries immediately; a game's right-click menu also has **Download
missing cover art**. Covers chosen with **Add cover art from file…** are preserved.

The primary source is [OpenVGDB](https://github.com/OpenVGDB/OpenVGDB), the same
catalog used by [OpenEmu](https://github.com/OpenEmu/OpenEmu/wiki/User-guide%3A-Box-art).
The first lookup downloads its catalog (release 29: approximately 9 MB compressed,
42 MB on disk); later runs reuse it and check for updates at most once daily.
Matching uses the ROM's MD5 locally, including removing a SNES copier header
when present. Renaming a known ROM does not break its match. ROM contents are
never uploaded.

If OpenVGDB's image host refuses a download or has no matching artwork,
[Libretro thumbnails](https://github.com/libretro-thumbnails/libretro-thumbnails)
provides a backup. It matches the verified OpenVGDB ROM filename/title, or an
exact normalized title for unknown ROMs. Regional artwork is preferred when
available. Backup indexes are cached for seven days; no account, API key or
RetroArch installation is needed. Both automatic downloads and the backup can
be switched off from the **Artwork** menu. Hover over a downloaded cover to
see which source supplied it.

Downloaded images are checked and saved locally for offline use. Unavailable
downloads retry after an hour; unmatched games retry after 30 days or immediately
with the retry command. Unknown homebrew and modified ROMs may need a manually
chosen cover. The bundled diagnostic ROMs do not have published box art.
Closing the library cancels pending artwork work; a network request already in
progress may take up to its timeout to stop.

**Download replacement cover art…** in the game menu can replace existing covers
after confirmation. The old image remains until a valid replacement succeeds.
Downloaded images use immutable cache paths; a manually chosen cover during a
download wins. **Artwork → Cancel cover downloads** cancels active and queued
cover requests without changing already completed results.

## Controls

Version **0.8.0** replaces the always-visible gameplay toolbar with a compact
overlay. Move the mouse over the game to reveal **Stop, Pause/Resume, Save,
Load, Mute, Fullscreen, Volume and Options**. It hides after inactivity, including
when the pointer rests over it, and remains available while paused, dragging a
control or using a popup menu. Small windows use icons; tooltips and accessible
names identify the buttons. The volume slider is available in wider windows;
volume commands remain in Options and on the keyboard at every size.

The **Game** menu and HUD **Options** contain all actions. **Save slot** selects
one of the existing nine slots. Fullscreen hides the menu/status bars, and the
same shortcuts continue working. Escape leaves fullscreen. **Settings → Gameplay**
adds fullscreen on launch, idle-pointer hiding, a 2–10× fast-forward target,
system-default/square-pixel display aspect, audio output and an audio-buffer
target. Existing focus-pause and integer-scaling settings remain in General.

Volume and mute changes are remembered. Fast-forward is silent; pause and focus
pause suspend the audio stream. Returning from fast-forward, resetting or loading
a state clears old queued audio. If a selected output disconnects, the player
uses the system default and can return to the selected output when it reconnects.
Requested buffer sizes are targets; the audio backend may adjust them. Actual
fast-forward speed is limited by available CPU performance.

| Action / SNES button | Keyboard | Controller |
| --- | --- | --- |
| Move | Arrow keys | D-pad or left stick |
| A / B | X / Z | East / south face button |
| X / Y | S / A | North / west face button |
| L / R | Q / W | Left / right shoulder |
| Start / Select | Enter / Backspace | Start / Back |
| Pause | Ctrl+P | — |
| Save / load selected state slot | F5 / F8 | — |
| Restore automatic save | Ctrl+F8 | — |
| Full screen | F11 | — |
| Screenshot | F12 | — |
| Fast forward | Hold Tab | — |
| Mute | Ctrl+M | — |
| Volume up / down | Ctrl+Up / Ctrl+Down | — |

F1 shows controls in the game window. Games pause when their window loses
focus. SNES uses a 4:3 display by default; Ctrl+I enables integer scaling with
square pixels. NTSC and PAL timing and changing frame dimensions are handled
by the core adapter. One player is currently exposed by the Linux interface.

## Games and saves

The library lives in `${XDG_DATA_HOME:-~/.local/share}/openemu-linux`.
`OPENEMU_DATA_DIR` or `--data-dir /path` can select a separate library. Imports
deduplicate by SHA-256 and copy ROMs unless external storage is selected in
Settings. Original files are preserved.

Imported copies keep their exact original filename, including capitalization,
underscores, spaces and extension. ZIP imports keep the ROM member's basename
without extracting its directory path. Each distinct ROM has its own hash
folder inside its system folder, so two different games named `Game.sfc`
cannot overwrite one another. Older managed copies retain their existing paths.
Importing identical ROM bytes again keeps the first library entry and filename.
Renaming a game's library title does not rename the ROM file.

On opening an older library, a background update restores original filenames
using stored import paths, or matching ROM bytes inside the original ZIP. If a
ZIP is no longer available, reimport its original ROM or ZIP to recover the
filename. Existing game IDs, custom titles, covers and state slots are retained.
Filename-dependent battery saves are copied to the new name when the game next
opens, after acquiring its game lock. Existing named saves are never overwritten.
Legacy ROM and save copies remain on disk for compatibility with running helpers.

- `roms/<system>/<game hash>/<original filename>`: new imported ROM copies.
- `saves/<game hash>/`: battery saves; SNES uses `.sav` files.
- `states/<game hash>/`: nine manual slots and an automatic save every 30 seconds
  and when closing a game normally.
- `covers/` and `screenshots/`: local artwork and captured frames.
- `metadata/`: OpenVGDB catalog and cached backup artwork indexes.
- SQLite metadata tables: editable fields, manual overrides, cached provider
  matches/retry status and ROM hashes; ROMs and image binaries remain on disk.
- `cores/<core ID>/<build SHA-256>.so`: installed Libretro builds; `installed.json`
  selects the current build of each core.
- `saves/<game hash>/libretro/<core ID>/`: Libretro battery RAM (`battery.srm`)
  and RTC (`clock.rtc`), plus any files a core writes in its save directory.
- `states/<game hash>/libretro/<core ID>/<build SHA-256>/`: Libretro state slots.
- `system/`: default BIOS/system folder for Libretro cores.

Save states include an integrity check and the ROM identity. A state from
another game or damaged state is rejected. SNES states use Snes9x's native
serialization inside this Linux container format. macOS `.oesavestate` bundles
and macOS library databases are not imported by this version.

Removing a library entry preserves ROMs and saves. Each game runs in a separate
process so a core failure does not terminate the library window. Snes9x itself
has global state; the adapter rejects a second instance in the same process.

## Build from source

Requirements: C and C++14 compilers, CMake 3.20+, Ninja, Python 3.10+, PySide6,
and optional SDL2. The bundled emulator cores need no Apple frameworks,
RetroArch, downloaded binaries, or system installation of Snes9x. The build
also produces `liboe_libretro.so`, the host for optional downloaded cores.

From this directory:

```sh
git -C .. submodule update --init --depth 1 Gambatte mGBA SNES9x
python3 -m venv .venv
.venv/bin/python -m pip install -r requirements.txt
./build.sh
./run.sh
```

Use `CC`, `CXX`, or CMake arguments to select a compiler on a fresh build.
`CMAKE=/path/to/cmake ./build.sh` selects a particular CMake installation.
`OPENEMU_CORE_DIR=/path/to/cores` overrides the compiled-core directory.
The checked-out build cache contains this machine's compiler paths; for a
different toolchain, configure a fresh build directory, for example:

```sh
cmake -S . -B build-local -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-local
OPENEMU_CORE_DIR="$PWD/build-local/cores" ./run.sh
```

## Validation

```sh
QT_QPA_PLATFORM=offscreen python3 -m unittest discover -s tests -p 'test_*.py' -v
```

The emulation tests exercise actual compiled Snes9x execution using the original
65816/SPC700 cartridge in `tests/snes_rom.py`:

- nonblank video, non-silent generated audio, and all twelve controller inputs;
- battery save persistence across closing, reopening and resetting;
- state restore, damaged-state rejection and wrong-ROM rejection;
- PAL, overscan, 512-column rendering and 478-line interlace;
- headered SMC, ZIP import, duplicate detection and safe archive handling;
- the one-instance restriction;
- library selection, Qt keyboard events and F5/F8 shortcuts;
- SDL virtual-controller buttons and disconnects.

Artwork tests cover exact ROM matching, SNES copier headers, region selection,
blocked-host fallback, offline caches, retries, manual-cover races, image and
download validation, safe catalog updates, visible system icons, Qt background
completion and cancellation on closing the window. These tests use a small
fixture catalog and controlled network responses. A separate live check
downloaded the real OpenVGDB 29 catalog, identified Aero the Acro-Bat by its ROM
hash, and downloaded its SNES cover through the backup after the primary image
host returned HTTP 403. A live Game Boy cover download was also verified.

Desktop audio was separately verified with the real Qt audio sink active and
processing approximately two seconds of samples. Physical controller hardware
and commercial-game compatibility have not been exhaustively tested. The
original GB, GBC and GBA diagnostics are also smoke-tested after shared changes.

Core-manager tests exercise installation, CRC rejection, failed-update
preservation, immutable builds, corruption repair, importing a `.so`, Settings
downloads, per-system selection and saved preferences. To include the actual
downloaded-core integration tests, set `OPENEMU_TEST_LIBRETRO_DIR` to a test
library with the `snes9x` Libretro core installed before running the command
above. Those tests cover all twelve inputs, resampled audio, serialization,
battery persistence, PAL/interlace, player selection and save isolation.

Filename tests cover exact Unicode/case/underscore preservation, symlink names,
same-name collisions, duplicate imports, ZIP traversal prevention, legacy name
recovery, reimporting a missing archive's ROM, background migration, and actual
Snes9x battery/save-state continuity after restoring a filename.

Library-view tests cover old database upgrades, repeatable migrations, safe
collection removal, live smart rules, batch ratings, literal filename searches,
information search, multi-selection across grid/list, numeric sorting, remembered
preferences, sidebar and cover drops, information editing and keyboard launch.

Importer tests cover managed/external storage, exact names, consolidation without
source changes, ZIP extraction and partial-failure accounting, unknown-system
resolution, persistent issues, permission/missing/empty/archive errors, duplicate
counts, progress/cancellation, safe folder traversal, hash-checked relinking and
save-name continuity through repeated relinks. A real SNES core also runs an
externally referenced ROM after system selection in the issue resolver.

Metadata tests cover exact/headered ROM matches, raw-file hashes, provider
replacement, missing fields, plain-text descriptions, search enrichment,
manual/cleared-field protection, old-library migration, stale-editor protection,
deleted-game races, offline caches and retries, startup/import/cancellation,
changed external files and scoped cover replacement. A live cached OpenVGDB
catalog also identified Aero the Acro-Bat's SNES ROM and returned its matching
system-specific information.

Gameplay tests cover HUD reveal/hide (including a stationary pointer), compact
layout, pause/resume, slot selection, fullscreen/hidden-menu shortcuts, saved
volume/mute/fullscreen/display/speed settings, audio-output selection/fallback
and audio suspension/queue reset. Automated device enumeration is simulated so
the suite does not require a desktop sound service. A separate live Qt audio
check ran the SNES diagnostic through the real default output, verified active
sample delivery, pause suspension and recovery after fast-forward; output was
muted during that check. Controller remapping and multiplayer remain planned.

All six offered cores were separately downloaded from the official server and
run against the diagnostic ROMs for their offered systems. This checks ten
core/system combinations, video, audio delivery and state round trips. The
Game Boy diagnostics now include standard cartridge validation bytes so cores
that perform the boot sequence can run them. Aero the Acro-Bat was also played
through downloaded Snes9x with the real Qt audio sink active.

## Scope and provenance

The Linux UI replaces Cocoa, the library uses SQLite instead of Core Data, Qt
handles display/audio, and SDL2 handles controllers. System names and suffixes
come from OpenEmu's original system-plugin property lists. The native adapters
derive from OpenEmu's `GBGameCore.mm`, `mGBAGameCore.m` and `SNESGameCore.mm`.

| Source | Pinned revision |
| --- | --- |
| OpenEmu | `1d205104640d8410659d321809889cbfd06b99a9` |
| Gambatte | `ef91ce464c450838d245311c1e45a80d0c5ca93f` |
| mGBA | `8e956c93d59c6054f9022699a5570a9f01d3c675` |
| Snes9x 1.62.3 | `6b1495d48bdef85e04cb96f5050601b9c6be3d88` |

Other console systems remain unported. SNES multitap, mouse/light guns,
Satellaview/Sufami Turbo BIOS workflows, MSU-1 sidecar imports, cheats UI,
rewind, shaders and macOS library migration are outside
this build. Regular cartridge emulation, video, stereo audio, player-one
controls, battery saves and state slots are implemented.

See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for source attributions and
license locations. The original sources and their license notices are retained
in this checkout.
