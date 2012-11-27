ScummVM Windows CE FAQ
Last updated: 2011-12-05
Release version: x.x.x
------------------------------------------------------------------------

New in this version
-------------------
x.x.x:
- Removed FLAC support for audio datafiles (now for real, this was originally
  announced for 1.0.0, but the library was still included until now). This is
  done because of size constrains of the executable and also FLAC on a mobile
  device isn't really recommended - so please use MP3 or Ogg for your audio
  datafiles.

1.4.0:
- Changed the memory management so that it is finally possible to break the
  32MB per process barrier on Windows CE. It should be possible now (finally)
  to play nearly every game with the "big" binary (scummvm.exe, which includes
  all game engines).
- Changed default values for "high_sample_rate" & "FM_high_quality" to "true"
  as most devices today are fast enough to handle this. It's still possible to
  set this to "false" if you have a slower device.
- Fix for TeenAgent & Hugo engines (both weren't running at all, crashed right
  at the beginning)
- Discworld 2 is now playable (works now because of the new memory management)
- Replaced the game mass-adding functionality with the functionality used on
  all other platforms. It now shows progress while searching for games.
- Mapped "Skip" button to F10 for AGI games
- Mapped "Multi Function" to F10 in Simon 1 & 2 (enables hotspot highlighting)

1.3.1:
- Fix for Normal2xAspect scaler which was causing screen update issues in some
  games.
- Fix for Normal1xAspect scaler which caused problems in the bottom part of the
  screen when toolbar was hidden.
- Fix for freelook mode.
- Fix for timer manager, caused timing issues in some games.
- Activated runtime language detection for ScummVM gui.
- Toolbar is now hidden when returning to the game list.
- Double-tap right-click emulation is now turned off for SCI games by default.
- Added a new option "no_doubletap_paneltoggle" for scummvm.ini to disable
  toolbar toggling when double-tapping on the top part of the screen.
- SDL library related fixes:
  * Fix for screen/mouse-cursor rotation issues (fixes erratic touchscreen
    behaviour)
  * Fix for hardware keyboard on some devices (HTC Touch Pro, etc.)

1.3.0:
This is the first official Windows CE release since 1.1.1.

The following new engines are now included (changes since last WinCE release):
 - Draci Engine (Dragon History)
 - Hugo Engine (Hugo Trilogy)
 - Mohawk Engine (Myst, Riven, Living Book games & Where in Time is Carmen
   Sandiego?)
 - SCI Engine (Sierra SCI games, see main README for a list of supported games)
 - Toon Engine (Toonstruck)

Also, there are now 4 binaries in this distribution, a single executable
which contains all engines (for devices with enough memory) and 3 smaller
binaries which contain only some of the engines. The following lists all
executables and the engines they contain:

scummvm.exe:
 - all supported engines
scummvm1.exe:
 - scumm, agi, cruise, draci, lure, queen, sky, sword1, tinsel, touche
scummvm2.exe:
 - agos, cine, drascula, gob, groovie, kyra, made, parallaction, saga,
   teenagent, tucker
scummvm3.exe:
 - hugo, mohawk, sci, sword2, toon, tsage

There are no other port specific changes.

1.2.1:
(Note: No official 1.2.1 release)

1.2.0:
(Note: No official 1.2.0 release)

1.1.1:
Fix to the Normal2xAspect scaler that was causing crashes.

1.1.0:
The TeenAgent engine is now included, but there are no other port specific
changes since 1.0.0.

This are 3 binaries in this distribution. Combining all the engines into a
single executable produces something that is too large to run on most
devices. We have therefore split the engines roughly into two and built 2
separate exes as follows:

scummvm1.exe:
 - scumm, sword1, sword2, queen, sky, lure, agi, touche, tinsel, cruise
scummvm2.exe:
 - gob, cine, saga, kyra, agos, parallaction, drascula, groovie, tucker, made,
   teenagent

For those lucky enough to have devices with enough memory we also have a
combined executable scummvm.exe which contains all of those engines.

1.0.0:
(Note: No changes since 1.0.0rc1)
This version features optimized ARM assembly versions for the Smartphone,
Normal2x and Normal2xAspect scalers, courtesy of Robin Watts. There should
be a speed improvement when using these scalers.

Also new is the aspect 2x upscaling mode, which is auto detected and used
when the scaler is set to (normal) 2x mode and the panel is hidden. Hence,
a 320x200 game running on a VGA or higher resolution device will be
aspect scaled to fill the 640x480 screen.

Be aware that Discworld 2 tries to allocate a big chunk of memory (10 MB)
and this will fail on many devices (file under the not enough memory
category).

From this version on, we're dropping support for FLAC and MPEG-2. The first
is a pain to maintain, while the second has been gradually phased out in
scummvm. Be sure to update your add-on packs and/or recompress your sound.


------------------------------------------------------------------------

This document is intended to give common answers to specific ScummVM
issues on Windows CE, in 3 sections
  * "General questions"       : browse this section to get started and see
                                general issues
  * "Game specific questions" : lists some common game specific issues
  * "Support and links"       : how to get support if you're still puzzled

------------------------------------------------------------------------
General questions
------------------------------------------------------------------------

Which devices are supported ?
-----------------------------

Official build
--------------

The official build is based on the ARM architecture and should work with any
Pocket PC 2002, Pocket PC 2003, Pocket PC 2003 SE, Smartphone 2002,
Smartphone 2003 or Windows Mobile 5 and 6 based device. It is known to work
on Pocket PC 2000 devices, but it has not been officially tested.

Support for old ARM architectures (Handheld PCs, Palm Size PCs) and other CPUs
(MIPS, SH3) is discontinued. Feel free to generate builds for these
architectures and contact us to include them on ScummVM website.

Games supported
---------------

The Windows CE port of ScummVM supports all available game engines.

ScummVM distinguishes devices based on two characteristics: Type and resolution.
Supported types of devices are Smartphones (usually no stylus) and Pocket PCs
(stylus). The supported resolutions are 176x220 (Smartphone), 240x240 (QVGA
square), 240x320 (QVGA), 320x240 (QVGA landscape), 480x640 (VGA). Devices with
resolutions larger than VGA should also be supported with automatic screen
centering.

* Low resolution Smartphones (176x220)

Support is only provided for all 320x200 or 320x240 games. The font can get
hard to read (you should prefer talkie games, or wear glasses :-P)
Games with 640x480 resolution such as COMI or BSWORD cannot be scaled down
to this resolution and still be playable.

* QVGA square devices (240x240)

Only 320x200 or 320x240 games are supported due to lack of downsampling scaler.

* QVGA Pocket PCs or Smartphones (240x320 or 320x240)

All games are playable in these devices. Landscape devices may not be able to
rotate the screen around.

* VGA (640x480) or higher Pocket PCs

All non VGA games should work properly on these devices. They can be resized
with different scalers. Moreover, VGA games will be displayed in true VGA mode.

Partial / Discontinued support
------------------------------

Support for the following devices is not complete, not working properly or
discontinued because the device is now obsolete. Feel free to contribute and
improve the port for your favorite device, or use the last release built with
the previous port architecture (0.5.1) which was less resource hungry and
supported more exotic devices.

* "Palleted" devices (non "true color")

These devices will be supported through the GDI layer which will slow down the
games a lot. You can try to disable the music/sound effects to get a better
game experience.

* "Mono" devices

I don't even think anything will be displayed on these devices :) you can try
and report your success ...

How do I install ScummVM for Windows CE ?
-----------------------------------------

Simple! Unpack the release package on your desktop pc, then copy all its
contents to a folder on your device. Typically, you should at least have
scummvm.exe, modern.ini and modern.zip in the same directory. Finally, upload
your beloved games and fire it up :-)

Some devices (like Pocket PC 2000) require GAPI to be present.

How do I install a game ?
-------------------------

You'll at least need to copy all the data files from your game, in a
sub-directory of your game directory.

You'll need to put the data files in a directory named after ScummVM game
name (see "Supported Games" section in ScummVM readme) for the games having
"generic" data files (.LFL files). Recent games can be put in any directory.

You can compress the multimedia files (sound/video) as described in the
ScummVM readme.

You can compress the audio tracks of Loom or Monkey Island 1 as described in
the ScummVM readme. If you are running these games on a slow device with Ogg
Vorbis compression, it's recommended to sample the files to 11 kHz (this sample
rate is not supported by other versions of ScummVM).

If you need more details, you can check SirDave's mini-manual online available
at: http://forums.scummvm.org/viewtopic.php?t=936
and at: http://www.pocketmatrix.com/forums/viewtopic.php?t=8606

How do I run a game ?
---------------------

If it's the first time you're running ScummVM for Windows CE, have installed or
removed games, you need to rescan your game directory.
 * Select Add Game, tap the root directory of your games, and tap "Yes" to begin
   an automatic scan of the installed games.

Usually all games are detected and you can start playing right away. If your
game is not detected check its directory name and your data files.

To play a game, tap on its name then tap the "Start" button or double tap its
name.

How do I play a game on a Pocket PC or Handheld PC device ?
-----------------------------------------------------------

The stylus is your mouse cursor, and a tap is a left mouse button click.

As the Pocket PC lacks some keys, a toolbar is displayed at the bottom of the
screen to make the most common functions just a tap away
  * The disk icon opens ScummVM options menu to save your game, or change your
    current game settings (depends on the game)
  * The movie icon skips a non interactive sequence, the current dialog or
    behaves like the ESC key on a regular keyboard (depends on the game)
  * The sound icon turns all sound effects and music off and on
  * The key icon allow you to map a key action to a device button
  * The monkey icon switches between portrait, landscape and inverse landscape
    mode (depends on the display drivers)

You can map additional actions on your device hardware buttons using the
"Options" / "Key" menu in the ScummVM options menu. To associate an action to
a key, tap the action, then the "Map" button and press the hardware key.
The following actions are available :

  * Pause          : pause the game
  * Save           : open ScummVM option menu
  * Quit           : quit ScummVM (without saving, be careful when using it)
  * Skip           : skip a non interactive sequence, the current dialog or
                     behaves like the ESC key on a regular keyboard
                     All AGI games    -> F10 to quit full-screen dialogs
  * Hide           : hide or display the toolbar
  * Keyboard       : hide or display the virtual keyboard
  * Sound          : turns all sound effects and music off and on
  * Right click    : acts as a right mouse button click
  * Cursor         : hide or display the mouse cursor
  * Free look      : go in or out of free-look mode. In this mode, you can tap
                     the screen to look for interesting locations without
                     walking. Click a second time near the pointer's location
                     equals to a left click.
  * Zoom up        : magnify the upper part of the screen for 640x480 games
                     rendered on a QVGA device.
  * Zoom down      : magnify the lower part of the screen for 640x480 games
                     rendered on a QVGA device.
  * Multi Function : performs a different function depending on the game :
                     Full Throttle    -> win an action sequence (cheat)
                     Fate of Atlantis -> sucker punch (cheat)
                     Bargon           -> F1 (start the game)
                     All AGI games    -> bring up the predictive input dialog
                     Simon 1 & 2      -> highlight all hotspots in screen
  * Bind keys        map a key action to a device button
  * Up,Down,Left   :
    Right,         : emulate mouse/stylus behavior
    Left Click     :

The default key bindings for Pocket PCs are (note that not all keys are mapped):
  * Up, Down, Left, Right  : (dpad) arrow keys
  * Left Click             : softkey A

If you start a game when a Right click mapping is necessary, ScummVM will ask
you to map one of your hardware key to this action before playing. Just press
the key you want to map if you see this message.

Notes:
- THE TOOLBAR CAN BE CYCLED BY DOUBLE TAPPING (SEE BELOW)
- YOU MUST HIDE THE TOOLBAR TO SCROLL THROUGH THE INVENTORY IN ZAK
- YOU MUST DISPLAY THE KEYBOARD TO FIGHT IN INDIANA JONES 3
- YOU MUST MAP THE RIGHT CLICK ACTION TO PLAY SEVERAL GAMES
- YOU MUST USE THE FREE LOOK ACTION TO PLAY LURE OF THE TEMPTRESS

How do I hide the toolbar ?
---------------------------

Note: THIS IS A VERY USEFUL AND SOMETIMES NECESSARY SHORTCUT

Double tapping the stylus at the top of the screen will switch between a
visible toolbar panel, a virtual keyboard, and hiding panel.  If any part of
the screen is obscured by the toolbar (like the load/save game dialogs) you can
use the invisible panel mode to get to it. For 320x200 games on QVGA Pocket
PCs, when the panel is hidden the game screen is resized to 320x240 (aspect
ratio correction) for better gaming experience.

How do I play a game on a Smartphone device ?
---------------------------------------------

On non-stylus devices, the mouse cursor is emulated via a set of keys.
The cursor will move faster if you keep the key down. You can tweak this
behaviour in the configuration file described below.

Here is
the list of available actions for Smartphones:

  * Up,Down,Left   :
    Right,         : emulate mouse/stylus behavior
    Left Click     :
    Right Click    :
  * Save           : open ScummVM option menu
  * Skip           : skip a non interactive sequence, the current dialog or
                     behaves like the ESC key on a regular keyboard
  * Zone           : switch between the 3 different mouse zones
  * Multi Function : performs a different function depending on the game
                     Full Throttle    -> win an action sequence (cheat)
                     Fate of Atlantis -> sucker punch (cheat)
                     Bargon           -> F1 (start the game)
                     All AGI games    -> bring up the predictive input dialog
  * Bind keys      : map a key action to a device button
  * Keyboard       : hide or display the virtual keyboard
  * Rotate         : rotate the screen (also rotates dpad keys)
  * Quit           : quit ScummVM (without saving, be careful when using it)

The "Zone" key is a *very* valuable addition allowing you to jump quickly
between three screen zones : the game zone, the verbs zone and the inventory
zone. When you switch to a zone the cursor will be reset to its former location
in this zone.

The default key map for these actions is:

  * Up, Down, Left, Right  : (dpad) arrow keys
  * Left Click             : softkey A
  * Right Click            : softkey B
  * Save                   : call/talk
  * Skip                   : back
  * Zone                   : 9
  * Multi Function         : 8
  * Bind keys              : end call
  * Keyboard               : (dpad) enter
  * Rotate                 : 5
  * Quit                   : 0

You can change the key mapping at any time by bringing up the key mapping menu
(Bind keys action).

How do I tweak the configuration of ScummVM ?
---------------------------------------------

See the section regarding the configuration file (scummvm.ini) in ScummVM
README file - the same keywords apply.

Some parameters are specific to this port :

Game specific sections (f.e. [monkey2]) - performance options

 *  high_sample_rate       bool     Desktop quality (22 kHz) sound output if
                                    set.  This is the default.
                                    If you have a slow device, you can set this
                                    to false to prevent lags/delays in the game.
 *  FM_high_quality        bool     Desktop quality FM synthesis if set. Lower
                                    quality otherwise. The default is high
                                    quality. You can change this if you have a
                                    slow device.
 *  sound_thread_priority  int      Set the priority of the sound thread (0, 1,
                                    2). Depending on the release, this is set
                                    to 1 internally (above normal).
                                    If you get sound stuttering try setting
                                    this to a higher value.
                                    Set to 0 if your device is fast enough or if
                                    you prefer better audio/video sync.

Game specific sections (f.e. [monkey2]) - game options

 *  landscape                int    0: Portrait, 1: Landscape,
                                    2: Inverse Landscape.
                                    You can also use this in the [scummvm]
                                    section to display the launcher in landscape
                                    for example, at startup.
 *  no_doubletap_rightclick  int    1: Turn off the default behavior of
                                    simulating a right-click when the screen is
                                    double-tapped.


[scummvm] section - keys definition

You usually do not wish to modify these values directly, as they are set
by the option dialog, and are only given here for reference.

 *  action_mapping_version int       Mapping version linked to ScummVM version.
 *  action_mapping         string    Hex codes describing the key associated to
                                     each different action.
 *  debuglevel             int       Debug Level 1 is used by the WinCE port
                                     for reporting diagnostic output in the
                                     scummvm_stdout.txt and scummvm.stderr.txt
                                     files in the current working directory.

[scummvm] section - mouse emulation tuning

You can tweak these parameters to customize how the cursor is handled.

 *  repeatTrigger         int       Number of milliseconds a key must be held to
                                    consider being repeated.
 *  repeatX               int       Number of key repeat events before changing
                                    horizontal cursor behaviour.
 *  stepX1                int       Horizontal cursor offset value when the key
                                    is not repeated.
 *  stepX2                int       Horizontal cursor offset value when the key
                                    is repeated less than repeatX.
 *  stepX3                int       Horizontal cursor offset value when the key
                                    is repeated more than repeatX.
 *  repeatY               int       Number of key repeat events before changing
                                    vertical cursor behavior.
 *  stepY1                int       Vertical cursor offset value when the key is
                                    not repeated.
 *  stepY2                int       Horizontal cursor offset value when the key
                                    is repeated less than repeatY.
 *  stepY3                int       Vertical cursor offset value when the key is
                                    repeated more than repeatY.

------------------------------------------------------------------------
Game specific questions
------------------------------------------------------------------------

---------------
-- All Games --
---------------

I need to press a special key
-----------------------------

Bring up the virtual keyboard. On Smartphones take a look at the Keyboard
action above. On Pocket PCs it's easier to double-tap at the top of the screen.

The panel is obscuring the playfield area
-----------------------------------------

Double tap at the top of the screen to hide it. As an aside, the aspect ratio
correction scaler will kick in if the game/device combo is appropriate.

How do I name my save games ?
-----------------------------

Use the virtual keyboard (Keyboard action).

ScummVM is stuck for some reason
--------------------------------

Bind and use the quit action to quit.

I cannot rotate the screen to landscape/inverse landscape
---------------------------------------------------------

Depending on the video driver, ScummVM may opt to not provide such
functionality.  In general, when ScummVM starts in normal "portrait"
orientation, the device driver reports better display characteristics and you
should consider launching from portrait.

I'm having problems. Is there diagnostic output available ?
-----------------------------------------------------------

Insert a line in the [scummvm] section of scummvm.ini with the following:
debuglevel=1
Run ScummVM. When it closes scummvm_stdout.txt and scummvm_stderr.txt files
will be available at the program directory (see section above).

ScummVM crashes and returns to desktop
--------------------------------------

File a bug report including diagnostic output (see previous question).

--------------------------
-- Beneath a Steel Sky  --
--------------------------

Introduction movie is too slow or never ends ...
-------------------------------------------------

Skip it :)

How can I open the inventory in Beneath a Steel Sky ?
---------------------------------------------------

Tap the top of the screen. Check your stylus calibration if you still cannot
open it.

How can I use an item in Beneath a Steel Sky ?
----------------------------------------------

You need to map the right click button (see the General Questions section).

----------------------------
-- Curse of Monkey Island --
----------------------------

How can I open the inventory in Curse of Monkey Island ?
------------------------------------------------------

You need to map the right click button (see the General Questions section).

I'm experiencing random crashes ...
------------------------------------

This game has high memory requirements, and may crash sometimes on low
memory devices. Continue your game with the latest automatically saved
game and everything should be fine.
You can consider removing the music and voice files (VOXDISK.BUN, MUSDISK.BUN)
to lower these requirements.

Sound synchronization is lost in Curse of Monkey Island videos
--------------------------------------------------------------

Use a faster device :-(

--------------------
-- Full Throttle  --
--------------------

I'm experiencing random crashes ...
------------------------------------

This game has high memory requirements, and may crash sometimes on low
memory devices. Continue your game with the latest automatically saved
game and everything should be fine.
You can consider removing the voice file (MONSTER.SOU) and disable the
music to lower these requirements.

----------------------------------------
-- Indiana Jones and the Last Crusade --
----------------------------------------

How can I fight in Indiana Jones and the Last Crusade ?
-----------------------------------------------------

You need to map the keyboard button (see the General Questions section).

---------------
-- Sam & Max --
---------------

How can I change the current action ?
-------------------------------------

You need to map the right click button (see the General Questions section).

How can I exit a mini game ?
----------------------------

Use the skip toolbar icon (see the General Questions section).

-------------------
-- Simon 1 and 2 --
-------------------

How can I save or quit in Simon ?
--------------------------------

"Use" (use the use verb :p) the postcard. The ScummVM option dialog is disabled
in Simon games.

On Smartphone, you'll need to push the Action button (center of the pad) to
quit the game.

-------------
-- The Dig --
-------------

I'm experiencing random crashes ...
------------------------------------

This game has high memory requirements, and may crash sometimes on low
memory devices. Continue your game with the latest automatically saved
game and everything should be fine.
You can consider removing the music and voice files (VOXDISK.BUN, MUSDISK.BUN)
to lower these requirements.

--------------------
-- Zak Mc Kracken --
--------------------

How can I scroll through my inventory items in Zak Mc Kracken ?
---------------------------------------------------------------

You need to map the hide toolbar button (see the General Questions section) or
double tap at the top of the screen (from 0.8.0+)

-------------------------
-- Broken Sword I & II --
-------------------------

I've installed the movies pack but they are not playing/they are slow
---------------------------------------------------------------------

MPEG 2 playback takes too much memory in the current release, and may prevent
movies from playing in VGA mode. Consider changing to the DXA cutscene pack
which is many times faster.

---------------
-- Gobliiins --
---------------

How do I enter a code ?
-----------------------

Use the virtual keyboard.

-------------------
-- Bargon Attack --
-------------------

How do I start the game (F1 : Game, F2 : Demo)
----------------------------------------------

Use the Multi Function action.

----------------------
-- AGI engine games --
----------------------

Do you expect me to play these games on keyboard-less devices ?
---------------------------------------------------------------

Sure we do :-)
If you want to get some mileage on your stylus you can use the virtual
keyboard.  There is a very useful alternative though, the AGI engine's
predictive input dialog.  It requires a dictionary to be present. Just tap on
the command line or use the Multi Function action to bring it up. On
Smartphones, when the dialog is shown all key mapping is disabled temporarily
(including mouse emulation). Input is performed either by pressing the phone's
numeric keypad keys and dpad enter to close the dialog, or by navigating the
buttons using the dpad arrows and pressing with dpad enter. Check the main
Readme file for more information on this.

---------------------------
-- Lure of the Temptress --
---------------------------

The control scheme is awkward (Pocket PCs)
------------------------------------------

Map and use the 'Free Look' action. Since normal pointer operation is to
enter a left click at each tap position, the free look mode enables
'hovering' the mouse on an object, then right clicking either by using the
double tap method or by pressing the 'Right Click' action. Also, a left click
can be entered while in free look mode, by clicking a second time near the
current pointer's location. Note that two taps equal a left click.

---------------
-- Discworld --
---------------

By default, the double tap to right click action is disabled in this game
as this interferes with the game's controls. This setting can be overridden
(see 'no_doubletap_rightclick' parameter above).

-----------------
-- Discworld 2 --
-----------------

Crashes at startup of this game are usually due to the high memory
requirements of this game.

-------------------------
-- Cruise for a Corpse --
-------------------------

As with Discworld, the double-tap-to-right-click action interferes and will
be disabled by default.


------------------------------------------------------------------------
Support
------------------------------------------------------------------------

Help, I've read everything and ...
-----------------------------------

Luckily, as there is a huge variety of Windows CE devices, a specific forum
is dedicated to this ScummVM port. You can ask your question on the WinCE
ScummVM forum available at http://forums.scummvm.org/viewforum.php?f=6

Some older questions and very nice tutorials are still available on the historic
PocketMatrix forum at http://www.pocketmatrix.com/forums/viewforum.php?f=20
where the community is always glad to help and have dealt with all the bugs for
many years now :)

I think I found a bug, ScummVM crashes in ...
---------------------------------------------

See the "Reporting Bugs" section in ScummVM readme.

If you have a Pocket PC or Handheld PC, be sure to include its resolution
(obtained on the second dialog displayed on the "About" menu) in your bug
report.

If you cannot reproduce this bug on another ScummVM version, you can cross
post your bug report on ScummVM forums.

I want to compile my own ScummVM for Windows CE
-----------------------------------------------

Take a look at:
http://wiki.scummvm.org/index.php/Compiling_ScummVM/Windows_CE


------------------------------------------------------------------------
Good Luck and Happy Adventuring!
The ScummVM team.
http://www.scummvm.org/
------------------------------------------------------------------------


------------------------------------------------------------------------
Old news follow ...
------------------------------------------------------------------------

0.13.0:
Important: Two builds for ScummVM CE

For this release, two binaries (executables) are provided. The first,
with file name scummvm1.exe, includes support for the following engines:
 - scumm, sword1, sword2, queen, sky, lure, agi, touche
while the second, with file name scummvm2.exe:
 - gob, cine, saga, kyra, agos, parallaction, drascula, groovie, tucker
The user must make sure to execute the correct file for a game. All
previously detected games will be shown in the launcher. Trying to launch
a gob engine game with scummvm1.exe will not work.
Detection also works as implied: scummvm1.exe will detect only the games
for which it has support; the same holds for scummvm2.exe.
This change has been done so users with less free memory can play more
memory hungry games.

Also noted are problems with flac support. Your mileage may vary. Please
consider using ogg or mp3 for those games (smaller sizes are better for
handheld devices too!)


0.12.0:
- Improved SMUSH support (deprecated 'Smush_force_redraw' option)
No skipped frames in Full Throttle action sequences. The 'Smush_force_redraw'
option is not needed/honored anymore.

- Fixed MultiFuntion key in Full Throttle

- Improved sound output
Fixed a long standing bug which led to distorted sound output in all games.

- Switched to faster ogg vorbis library
Robin Watts' libTremolo is used for ogg vorbis (tremor) replay. Info patch
by Lostech.

- New right click through double tap inhibiting option
Check out the 'no_doubletap_rightclick' option if double-tapping as a right
click input method annoys you. Patch by spookypeanut.


0.11.0:
- Redesigned 'Free Look' action (Pocket PCs)
In order to accommodate for the requirements of the lure engine, the
usage characteristics of the 'Free Look' action have been improved. The
new behavior is available for use in all engines, but is is *strongly*
recommended for at least when playing 'Lure of the Temptress'. By using
the new scheme, when in 'Free Look' mode, it is now possible to enter
left clicks by clicking a second time near the current location of the
mouse pointer. Left and Right clicks at the current point location
are also available by using the respective actions' bound key.

- Reduced optimization build
The ScummVM executable has grown quite large, prohibiting some devices
from running memory demanding games (or any games at all). Code
optimization level has been reduced to offset the growth of the executable.
Games run slightly slower. This will be addressed before next release.

- Several bugfixes


0.10.0:
Major improvements have taken place in this version, mostly for behind-
the-scenes stuff. First, we have migrated to GCC for building the Windows
CE port. This helped take care of some obscure compiler bugs which were
in there for quite a long time. It has also lead to efficient code
generation due to GCC's advanced capabilities and consequently increased
runtime speed. The second important change was the overhaul of the SDL
library port. The benefits from this are twofold: The real-time code paths
have been optimized, including contributed ARM assembly code for critical
functions. Further, the screen display and mouse/keyboard input code has
been partially rewritten to allow for increased compatibility across all
devices.
Due to the update of keyboard handling code, the keycodes have changed
slightly. Running this version of ScummVM will overwrite your key bindings
with the new defaults. See the section on how to play on Smartphones and
Pocket PCs below for the new default key bindings.
