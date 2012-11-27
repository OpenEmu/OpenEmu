


                    The official port of ScummVM
                 to the Nintendo DS handheld console
                     by Neil Millstone (agentq)
                   http://scummvm.drunkencoders.com
------------------------------------------------------------------------
Visit the main ScummVM website <http://www.scummvm.org>




     Contents
     ------------------------------------------------------------------------



    * What's New?
    * What is ScummVM DS?
    * Features
    * Screenshots
    * How to Get ScummVM DS Onto Your DS - Simple Explanation
          o Using a CF/SD/Mini SD/Micro SD card reader and a DLDI driver
          o Instructions for specific card readers
    * How to use ScummVM DS
    * Game Specific Controls
    * DS Options Screen
    * Which games are compatible with ScummVM DS
    * Predictive dictionary for Sierra AGI games
    * Converting your CD audio
    * Converting speech to MP3 format
    * Frequently Asked Questions
    * Downloads
    * Contributors
    * Donations and Getting Help
    * Building from Sources



      What's New?
      ------------------------------------------------------------------------

ScummVM DS 1.2.0

 * New game supported: Fascination

ScummVM DS 1.1.1

 * Bugfix release: no new features

ScummVM DS 1.1.0

 * New games are supported in this stable build: Return to Zork, Rodney's
   Funscreen, Manhole, Leather Goddess of Phobos 2, Cruise for a Corpse.

ScummVM DS 1.0.0

 * No changes!

ScummVM DS 1.0.0 RC1

 * Gamma correction feature to improve brightness level of game on the
   original DS.
 * The usual round of bugfixes


ScummVM DS 0.13.1

 * Support for the ScummVM Global Main Menu (hold select during the game)
 * The usual round of bugfixes


ScummVM DS 0.12.0

 * New games supported: Lure of the Temptress, Nippon Safes, Lost in Time.
 * New laptop-style trackpad input method.  Uses relative movement when you
   drag on the touch screen.
 * New option which allows you to drag to hover, tap the touch screen to
   click, and double tap the screen to right click.
 * Reorganised DS Options screen into three tabs for clearer navigation
 * New top screen scaling options let you choose the scaling factor used
   on startup.
 * The usual round of bug fixes.


ScummVM DS 0.11.1

 * Bugfix release - No new DS port features


ScummVM DS 0.11.0

 * New games supported: Elvira 1 and 2, Waxworks (Amiga version)
 * Software scaler for improved image quality. Turn it on using the DS options
   screen (press select during the game). Thanks to Tramboi and Robin Watts for
   this feature!
 * Function keys added to virtual keyboard (used in AGI games)
 * Plenty of bug fixes

      What is ScummVM DS?
      ------------------------------------------------------------------------

ScummVM DS is a part of the ScummVM project.  The ScummVM project is an
attempt to re-engineer many classic point and click adventure games of the
80s and 90s to run on modern computer hardware.  Technology has changed a
lot since these games were written, and so ScummVM attempts to replicate the
gameplay of the original games in exacting details, without any of the original
code that the game ran on.  ScummVM needs a copy of the original game, in order
to take the graphics, sound, and scripts that made the game work.

ScummVM is written in such a way that it can be 'ported' from one type of
machine to another, and ScummVM DS is a port of ScummVM to the Nintendo DS
handheld games console.


      Features
      ------------------------------------------------------------------------

    * Runs nearly all of Lucasarts' SCUMM games up to and including Sam
      & Max Hit the Road
    * Runs many non-Lucasarts point-and-click adventures too
    * Supports sound
    * Provides a GUI to change settings and choose games
    * Supports using the DS touch screen for controls
    * Suports saving games to compatible flash cards
    * All games run at pretty much full speed





      How to Get ScummVM DS Onto Your DS - Simple Explanation
      ------------------------------------------------------------------------

Nintendo don't want you to run ScummVM on your DS.  They control
which companies can make games on the DS, and there is an expensive
process to go through in order to be licenced.  Having to pay for
this would prevent me from giving away ScummVM for free.

So, the result is that to run ScummVM on your DS you'll need an
unofficial card reader.  There are many of these, and all are different.
Popular models at the time of writing are the R4DS and the M3DS Real,
but many different models work.  You need to buy one of these, and at
MicroSD card to go with it.

There are also slot-2 card readers which fit into the bottom slot on
your DS, usually used for Game Boy Advance games.  These are less common
these days, and although they have certain advantages, the details of
these are beyond the scope of this website.  Information on these is
quite easy to find by searching.

Once you have your card reader and a MicroSD card, you will also need
a copy of the game you want to run.  ScummVM can run a large variety
of games, but you must own a real boxed copy of the game.  These games
are still under copyright, and it is illegal to copy them from a friend
or download them from the Internet without paying.  The exception to
this are the three Revolution Software games.  These are 'Beneath a
Steel Sky', 'Lure of the Temptress' and 'Flight of the Amazon Queen'.
Revolution have kindly allowed us to give these games away for free.
You can download them from the main ScummVM site at
<http://www.scummvm.org/downloads.php>

If you have a modern card reader, like an M3 Real or R4DS, getting
ScummVM to run on your card is as simple as:
 1) Copy the scummvm NDS files on your MicroSD card using your
    PC card reader
 2) Copy the games you want to play onto your MicroSD card in any
    location, one per folder
 3) Boot the DS with the MicroSD card and card reader inserted
 4) Run the relevent build for the game you want to play (see
    'Which games are compatible with ScummVM DS' below.
 5) Click 'Add Game'.  Select the folder where your game is stored.
    Click 'OK'.  Click 'OK' in the settings dialog.
 6) Click 'Start'.

NOTE: Previous version of ScummVM DS supported a method which used a
zip file to run games on unsupported flash card readers.  This method
is no longer supported.



      How to Get ScummVM DS Onto Your DS - Using a CF/SD/Mini SD/Micro
      SD card reader and a DLDI driver
      ------------------------------------------------------------------------

ScummVM DS needs something called a DLDI driver to run on each make
and model of card reader.  Many modern card readers (R4DS, M3 DS Real)
handle this autmatically and for those, you don't have to do anything.
Just running ScummVM on the card will handle this step for you.
For others, you will need to follow the steps in this section before
ScummVM DS will work.

All DS card readers are different in the way that they work. In order to
support many different card readers, ScummVM DS uses a DLDI driver installed
into the ScummVM DS code. This is done using a program called DLDITool
which you can download and run on your computer. Each DLDI driver is
designed to tell ScummVM DS how to use a specific type of card reader.
These drivers can be used with any homebrew program which supports the
DLDI interface.

While each card reader should work with these instructions, there are
some exceptions. Please read the card reader notes
section to see if there is any specific information about your card reader.

Here is what you need to do:

    * Visit the DLDI page <http://dldi.drunkencoders.com/> and
      download the executable for DLDITool for your operating system
      (versions are available for Windows, Linux, and MacOS)
    * Download the DLDI for your card reader. This is the big table at
      the top of the page. The first column marked DLDI is the one you
      want. You should get a single file with a .dldi extension.
    * Extract DLDITool into a folder, and put the DLDI of your choice in
      the same folder.
    * If you're using the command line version of DLDITool enter the
      following at a command prompt:

          dlditool <dldiname> <scummvm nds name>


      If you're using the Windows GUI version, double click on
      dlditool32.exe, select your card reader from the box, drag your
      ScummVM binaries (either the .nds, or the .ds.gba version
      depending on your card reader. I think only Supercards use the
      .ds.gba files) into the lower box, then click patch.

      Either way, you should see 'Patched Successfully'. If you don't,
      you're doing something wrong.

      You need to patch one of the builds labeled A - H depending on
      which game you want to run. See the table on the ScummVM DS
      website to see which games are supported by which build.

    * Put the patched .nds or .ds.gba files on your flash card. If
      you're using the Supercard, you will need to use the .ds.gba
      files, but rename them to .nds.
    * Put your game data in any folder on the card. Do NOT use a zip file.
    * Boot up your DS and run ScummVM.
    * Click 'Add Game', browse to the folder with your game data, click
      'Choose', then 'OK'. Click 'Start' to run the game.

If your copy of ScummVM DS has been successfully patched, you will get a
message on the top screen that looks like this:

        DLDI Device:
        GBA Movie Player (Compact Flash)

The message should show the name of your card reader. If it is wrong,
you have used the wrong DLDI file.

If you haven't patched your .nds file, you will get the following message

        DLDI Driver not patched!
        DLDI Initialise failed.

In this case, you've made a mistake following the above instructions, or
have patched the wrong file.

You may also see the following message:

        DLDI Device:
        GBA Movie Player (Compact Flash)
        DLDI Initialise failed.

In this case, the driver did not start up correctly. The driver is
probably broken, or you've used the wrong one for your card reader.

In the case of the Supercard, M3 Lite and DS Link, there are several
drivers available. You might want to try one of the others.

This version of ScummVM DS will run on any card reader that has a DLDI
driver available. If yours doesn't, you need to pressure your card
reader manufacturer to release one.

DO NOT EMAIL ME TO ASK ME TO CREATE A DRIVER FOR YOUR CARD READER, I
CANNOT DO THIS.


      How to Get ScummVM DS Onto Your DS - Instructions for specific
      card readers
      ------------------------------------------------------------------------

    * *GBAMP CF:* You need to upload replacement firmware to your card
      reader before it will work. You can download the firmware program
      here <http://chishm.drunkencoders.com/NDSMP/index.html>. Name your
      .nds file _BOOT_MP.nds.
    * *M3 CF/SD:* Copy the .nds file to your card with the M3 Game
      Manager in order to avoid an annoying message when you boot your
      M3. Use the default options to copy the file. Be sure to press 'A'
      in the M3 browser to start the .nds file, and not 'Start', or it
      won't work.
    * *M3 CF/SD:* Copy the .nds file to your card with the M3 Game
      Manager in order to avoid an annoying message when you boot your
      M3. Use the default options to copy the file. Be sure to press 'A'
      in the M3 browser to start the .nds file, and not 'Start', or it
      won't work.
    * *Supercard CF/SD (slot-2):* Use the .ds.gba files to run ScummVM
      on the Supercard. Other than that, just follow the instructions as
      normal.
    * *Supercard Lite (slot-2):* It has been reported that only the
      standard Supercard driver and the Moonshell version work with
      ScummVM DS.
    * *Datel Max Media Dock: * If you haven't already, upgrade your
      firmware to the latest version. The firmware that came with my Max
      Media Dock was unable to run ScummVM DS at all. Click here to
      visit Datel's support page and download the latest firmware
      <http://us.codejunkies.com/mpds/support.htm>
    * *NinjaDS*: There are firmware upgrades for this device, but for
      me, ScummVM DS ran straight out of the box. Visit this page
      <http://www.ninjads.com/news.html> to download the latest firmware
      if you want. If you have installed FlashMe on your DS, it will
      make your DS crash on boot when the NinjaDS is inserted. You can
      hold the 'select' button during boot to disable FlashMe, which
      will allow the NinjaDS to work. Due to this, it is not recommended
      to install FlashMe if you use a NinjaDS.
    * *R4DS*: If you upgrade the firmware for your R4DS to version 1.10
      or later, the card will autmatically DLDI patch the game, meaning
      you don't have to use dlditool to patch the .NDS file. This makes
      things a lot easier!
    * *M3DS Real*: This card autmatically DLDI patches the game, meaning
      that you do not need to do this yourself.



      Which games are compatible with ScummVM DS?
      ------------------------------------------------------------------------

I'm glad you asked. Here is a list of the compatible games in version
1.2.0. Demo versions of the games listed should work too.

Flight of the Amazon Queen, Beneath a Steel Sky, and Lure of the
Temptress have generously been released as freeware by the original
authors, Revolution Software <http://www.revolution.co.uk/>. This is a
great thing and we should support Revolution for being so kind to us.
You can download the game data from the official ScummVM download page
<http://www.scummvm.org/downloads.php>.

The other games on this list are commercial, and still under copyright,
which means downloading them without paying for it is illegal. You can
probably find a second-hand copy on eBay. Please don't email me to ask
for a copy, as I am unable to send it to you.

Game                                      Build   Notes

Manic Mansion                             A

Zak McKracken and the Alien Mindbenders   A

Indiana Jones and the Last Crusade        A

Loom                                      A

Passport to Adventure                     A

The Secret of Monkey Island               A

Monkey Island 2: LeChuck's Revenge        A

Indiana Jones and the Fate of Atlantis    A

Day of the Tentacle                       A

Sam & Max Hit the Road                    A     Some slowdown in a few scenes
                                                when MP3 audio is enabled

Bear Stormin' (DOS)                       A

Fatty Bear's Birthday Surprise (DOS)      A

Fatty Bear's Fun Pack (DOS)               A

Putt-Putt's Fun Pack (DOS)                A

Putt-Putt Goes to the Moon (DOS)          A

Putt-Putt Joins the Parade (DOS)          A     Can sometimes crash due to low memory

Beneath a Steel Sky                       B

Flight of the Amazon Queen                B

Simon the Sorcerer 1                      C     Zoomed view does not follow the
                                                speaking character
Simon the Sorcerer 2                      C     Zoomed view does not follow the
                                                speaking character
Elvira 1                                  C

Elvira 2                                  C

Waxworks (Amiga version)                  C

Gobliiins                                 D

Gobliins 2                                D

Goblins 3                                 D

Fascination 				  D     This game is untested on the DS

Ween: The Prophecy                        D

Bargon Attack                             D

Lost in Time                              D

Future Wars                               D

All Sierra AGI games.
For a complete list, see this page
<http://wiki.scummvm.org/index.php/AGI>   D

Inherit the Earth                         E

The Legend of Kyrandia                    F     Zoomed view does not follow the
                                                speaking character

Lure of the Temptress                     G

Nippon Safes                              G

Return to Zork                            I

Leather Goddess of Phobos 2               I

Manhole                                   I

Rodney's Funscreen                        I

Cruise for a Corpse                       K


Full Throttle and The Dig can run on the DS using an external RAM pack
plugged into slot-2 (the GBA slot).  To do this, download the special
build of ScummVM DS from here: http://forums.scummvm.org/viewtopic.php?t=7044

There is no support for Windows Humongous Entertainment games, Broken Sword,
and all other games that run at 640x480 resolution.  The DS cannot cope with
scaling these games down to 256x192, the resolution of the DS screens.


      How to Use ScummVM
      ------------------------------------------------------------------------

Once you've booted up ScummVM, you'll see the start up screen.

 1. Tap the 'Add' button with the pen, then browse to the folder
    containing your game data. Once you have clicked on your folder, you will
    not see any files inside.  This is normal, as the folder selector only shows
    folders!

 2. Click the 'Choose' button.

 3. You will get some options for the game. You can usually just click 'Ok' to
    this.

 4. Now click on the name of the game you want to play from the list and
    click 'Start'. Your game will start!

You can use the B button to skip cutscenes, and the select button to
show an options menu which will let you tweak the DS contols, including
switch between scaled and unscaled video modes. The text is clearer in
the unscaled mode, but the whole game doesn't fit on the screen. To
scroll around, hold either shoulder button and use the D-pad or drag the
screen around with the stylus. Even in scaled mode, a small amount is
missing from the top and bottom of the screen. You can scroll around to
see those areas. The top screen shows a zoomed-in view. This scrolls
around to focus on the character who's speaking, and also follows where
the pen touches the screen. You can change the zoom level by holding one
of the shoulder buttons and pressing B to zoom in and A to zoom out.

Press the start button for the in-game menu where you can load or save
your game (this works in Lucasarts games, other games vary). Saves will
write directly to your flash card. You can choose the folder where they
are stored using the GUI that appears when you boot up. If you're using
a GBA Flash Cartridge, or an unsupported flash card adaptor, you will be
using GBA SRAM to save your game. Four or five save game will fit in
save RAM. If you save more games than will fit, a warning will appear on
the top screen. When you turn your DS off, the new save will be lost,
and only the first ones you saved will be present.

Many of the games use both mouse buttons. Usually the right button often
performs the default action on any object you click on. To simulate this
with the DS pen, you can switch the input into one of three modes. Press
left on the D-pad to enable the left mouse button. Press right on the
D-pad to enable the right mouse button. Press up on the D-pad to enable
hover mode. In this mode, you won't click on anything, just hover the
mouse cursor over it. This lets you pick out active objects in the
scene.

An icon on the top screen will show you which mode you're in.

In hover mode, there are some additional controls. While holding the pen
on the screen, tapping D-pad left or D-pad right (or A/Y in left handed
mode) will click the left or right mouse button.

There is an alternative method of control which doesn't require you to
change modes with the D-pad.  Press 'Select' to bring up the DS options,
and choose 'Tap for left click, double tap for right click'.  In this
mode, you can quickly tap the screen to left click the mouse, and tap
twice to right click the mouse.


Here is a complete list of controls in right-handed mode (the default
setting):
Key                    Usage
Pad Left               Left mouse button
Pad Right              Right mouse button
Pad Up                 Hover mouse (no mouse button)
Pad Down               Skip dialogue line (for some Lucasarts games),
                       Show inventory (for Beneath a Steel Sky), Show
                       active objects (for Simon the Sorceror)
Start                  Pause/game menu (works in some games)
Select                 DS Options
B                      Skip cutscenes
A                      Swap main screen and zoomed screen
Y                      Show/Hide debug console
X                      Show/Hide on-screen keyboard
L + D-pad or L + Pen   Scroll touch screen view
L + B                  Zoom in
L + A                  Zoom out



And here's left-handed mode:
Key                    Usage
Y                      Left mouse button
A                      Right mouse button
X                      Hover mouse (no mouse button)
B                      Skip dialogue line (for some Lucasarts games),
                       Show inventory (for Beneath a Steel Sky), Show
                       active objects (for Simon the Sorceror)
Start                  Pause/game menu (works in some games)
Select                 DS Options
D-pad down             Skip cutscenes
D-pad up               Swap main screen and zoomed screen
D-pad left             Show/Hide debug console
D-pad right            Show/Hide on-screen keyboard
R + D-pad or R + Pen   Scroll touch screen view
R + D-pad down         Zoom in
R + d-pad right        Zoom out



      Game-specific controls
      ------------------------------------------------------------------------

    * Sam and Max Hit the Road: The current cursor mode is displayed on
      the top screen. Use d-pad right to switch mode.
    * Indiana Jones games: If you get into a fight, press Select, and
      check the box marked 'Use Indy Fighting Controls'.
      Return to the game, then use the following controls to fight:

      D-pad left: move left
      D-pad right: move right
      D-pad up: guard up
      D-pad down: guard down
      Y: guard middle
      X: Punch high
      A: Punch middle
      B: Punch low
      Left shoulder: Fight towards the left
      Right shoulder: Fight towards the right

      The icon on the top screen shows which way you're currently
      facing. Remember to turn the option off when the fight ends, or
      the normal controls won't work!
    * Beneath a Steel Sky: Press D-pad down to show your inventory.
    * Simon the Sorcerer 1/2: Press D-pad down to show active objects.
    * AGI games: Press Start to show the menu bar.
    * Bargon Attack: Press Start to hit F1 when you need to start the
      game. Use the on-screen keyboard (hit X) to press other function keys.


      DS Options Screen
      ------------------------------------------------------------------------

Pressing the 'select' button during any game to show the DS options
screen.  This screen shows options specific to the Nintendo DS version
of ScummVM.

Controls tab

Indy Fight Controls - Enable fighting controls for the Indiana Jones
games.  See 'Game Specific Controls' for more information.

Left handed Mode - Switch the controls on the D-pad with the controls
on the A/B/X/Y buttons.

Show mouse cursor - Shows the game's mouse cursor on the bottom screen.

Snap to edges - makes it easier for the mouse controls to reach the edges
of the screen.  Useful for Beneath a Steel Sky and Goblins 3.

Touch X offset - if your screen doesn't perform properly, this setting
allows you to adjust when the cursor appears left or right relative to
the screen's measured touch position.

Touch Y offset - if your screen doesn't perform properly, this setting
allows you to adjust when the cursor appears higher or lower relative to
the screen's measured touch position.

Use Laptop Trackpad-style cursor control - In this mode, use the lower
screen to drag the cursor around, a bit like using a trackpad on a laptop.
When this option is enabled, the following option is also enabled.

Tap for left click, double tap for right click - In this mode, you can
quickly tap on the screen to left click the mouse, or quickly
double tap on the screen to right click the mouse.  If you find clicking
or double-clicking difficult, try and make a firmer touch on the screen,
and keep the pen down for longer.

Sensitivity - this bar adjusts the speed of cursor movement when laptop
trackpad-style cursor control is enabled (see above).

Graphics Tab

Scaling options:

Three scaling options are available for the main screen.

Harware Scale - Scales using the DS hardware scaler using a flicker method.
Produces lower quality graphics but doesn't slow the game down.

Software Scale - Scales using the CPU.  A much higher quality image is
produced, but at the expense of speed in some games.

Unscaled - Allows you to see the graphics as originaly displayed.  This
doesn't fit on the DS screen, but you can scroll the screen around by holding
the left shoulder button and using the D-pad or touch screen.

Top screen zoom - These three options control the zoom level of the top
screen when ScummVM is started up.  Changing this option will set the zoom
to the specified level immediately.

Initial top screen scale:

This option controls the scaling level of the zoomed screen.  In ScummVM
DS, one screen shows a zoomed-in view of the action, and this option controls
how zoomed in it is.  You can also adjust this in the game by holding L and
pressing A/B.

General Tab

High Quality Audio - Enhance the sound quality, at the expense of some
slowdown during some games.

Disable power off - ScummVM DS turns the power off when the game quits.
This option disables that feature.






      Auto completion dictionary for Sierra AGI games
      ------------------------------------------------------------------------

If you are playing a Sierra AGI game, you will be using the on-screen
keyboard quite a lot (press X to show it). To reduce the amount you have
to type, the game can automatically complete long words for you. To use
this feature, simply copy the PRED.DIC file from the ScummVM DS archive
into your game folder on your card. Now, when you use the keyboard,
possible words will be shown underneith it. To type one of those words,
simply double click on it with your stylus.


      Converting your CD audio
      ------------------------------------------------------------------------

ScummVM supports playing CD audio for specific games which came with
music stored as standard music CD tracks. To use this music in ScummVM
DS, they need to be ripped from the CD and stored in a specific format.
This can only be done for the CD versions of certain games, such as
Monkey Island 1, Loom, and Gobliiins. All the floppy games and CD games
that didn't have CD audio tracks for music don't require any conversion,
and will work unmodified on ScummVM DS. MP3 audio files for CD music are
not supported.

Cdex can do the conversion very well and I recommend using it to convert
your audio files, although any CD ripping software can be used, so feel
free to use your favorite program. The format you need to use is
IMA-ADPCM 4-bit Mono. You may use any sample rate. All other formats
will be rejected, including uncompressed WAV files.

Since this sound format is a standard, you should be able to create it
in a variety of software.
Now I will to describe how to rip your CD tracks with Cdex, which can be
found here: Cdex Homepage <http://sourceforge.net/projects/cdexos/>.
If you're using MacOS or Linux, I suggest using Audacity
<http://audacity.sourceforge.net>.

To encode audio in Cdex, select Settings from the Options menu. On the
Encoder tab, select 'WAV Output Encoder'. Under 'Encoder Options',
choose the following:

      Format: WAV
      Compression: IMA ADPCM
      Samplerate: 22050 Hz
      Channels: Mono
      On the fly encoding: On

Next, go to the 'Filenames' tab and select the folder you want to save
your Wav files to. Under 'Filename format', enter 'track%3'. This should
name your WAV files in the correct way. Click OK.

Now select all the tracks on your CD, and click 'Extract CD tracks to a
compressed audio file'. Cdex should rip all the audio off your CD.

Now all you have to do is copy the newly created WAV files into the same
directory that your other game data is stored on your CompactFlash card.
Next time your run ScummVM DS, it should play with music!

*Important Note:* Do not select 'Extract CD tracks to a WAV file'. This
creates uncompressed WAVs only. You want 'Extract CD tracks to a
compressed audio file'.


      Converting Speech files to MP3 format
      ------------------------------------------------------------------------

ScummVM supports playing back speech for talkie games in MP3 format.
Unfortunately, the DS CPU is not quite up to the task, and MP3 audio
will sometimes cause slowdown in your game. However, if your flash card
isn't big enough to fit the audio files on, you will have no choice!

To convert your audio you will need a copy of the ScummVM Tools package
<http://sourceforge.net/project/showfiles.php?group_id=37116&package_id=67433>.
You will also need a copy of the LAME MP3 encoder
<http://www.free-codecs.com/Lame_Encoder_download.htm>.

Once this is all installed and set up, the process to encode your audio
varies from game to game, but the Lucasarts games can all be compressed
using the following command line:

compress_scumm_sou --mp3 monster.sou

This produces a monster.so3 file which you can copy to your flash card
and replaces the original monster.sou. Ogg format (monster.sog) and flac
format files are not currently supported by ScummVM DS, and it is
unlikely they will ever be supported. There is no way to convert .sog or
.so3 files back to .sou files. Just dig out your original CD and copy
the file from that.


      Frequently Asked Questions
      ------------------------------------------------------------------------

I get a lot of email about ScummVM DS. Nearly all of them are exactly
the same. Here I'm going to try and answer the questions that everybody
asks me in the hope that I will spend less time answering questions that
are clearly in the documentation and more time helping people who have a
real problem or have discovered a real bug.

*Q:* I can't see the bottom line of inventory items in Day of the
Tentacle, Monkey Island 2, or a few other games! What do I do?
*A:* Hold down the left shoulder button and use D-pad (or the touch
screen) to scroll the screen around.

*Q:* I don't get speech or sound effects in my Lucasarts games.  What's
wrong?
*A:* Do you have a monster.sog file?  If so, this is the wrong kind of
speech file for ScummVM DS.  Copy the monster.sou file present on your
original CD or floppies, and you will have speech.

*Q:* Can ScummVM take advantage of the DSi?
*A:* At the moment, no.  While some homebrew does run on the DSi, at
the time of writing, it cannot access any of the DSi's new features.
When it does, a version of ScummVM for the DSi may be possible.

*Q:* I dont see a menu when I press Start in Flight of the Amazon Queen
or Simon the Sorcerer. Is ScummVM broken?
*A:* No. To save in Simon the Sorcerer, click 'use', then click on the
postcard in your inventory. In Flight of the Amazon Queen, click 'use',
then click on the journal in your inventory.

*Q:* Why does ScummVM crash when I play Monkey Island 1?
*A:* This happens when MP3 audio tracks are present from the PC version
of ScummVM. Delete the MP3 tracks and reencode them to ADPCM WAV files
as described in the CD audio section.

*Q:* Can't you use the extra RAM in the M3/Supercard or the official
Opera Expansion Pack to support more games like The Dig and Full
Throttle? DS Linux has done it, so why can't you?
*A:* Yes!  The Dig and Full Throttle are playable with some limitations.
See the forum thread here for a download:
http://forums.scummvm.org/viewtopic.php?t=7044

*Q:* ScummVM DS turns off my DS when I hit 'Quit' in the game or quit
from the frontend. Why doesn't it return to the menu?
*A:* To return to the game launcher, hold 'Select' during the game
to access the main ScummVM menu, then click 'Return to Launcher'



      Contributors
      ------------------------------------------------------------------------

ScummVM DS uses chishm's GBA Movie Player FAT driver.
The CPU scaler is by Tramboi and Robin Watts
The ARM code was optimised by Robin Watts
Thanks to highpass for the ScummVM DS icons.
Thanks to zhevon for the Sam & Max cursor code.
Thanks to theNinjaBunny for the M3 Adaptor guide on this site.
Thanks also to everyone on the GBADev Forums.

This program was brought to you by caffiene, sugar, and late nights.


      Donations and Getting Help
      ------------------------------------------------------------------------

If you have problems getting ScummVM to work on your hardware, please
read the FAQ first. /Please/ don't ask me questions which are answered
in the FAQ, I get many emails about this program each day, and I can't
help the people who really need help if I'm answering the same question
all the time which is already answered on this page. Other than that,
feel free to post on the ScummVM DS forum <http://forums.scummvm.org>
for help. Please do your research first though. There is no way of
running this on an out-of-the box DS without extra hardware. Most of
these things are fairly inexpensive though.

If you want to contact me, please email me on scummvm at millstone dot
demon dot co dot uk.

If you want to help with the development of ScummVM DS, great! Download
the source code and get building. There are plenty of things left to do.

You can also help by making a donation if you've particularly enjoyed
ScummVM DS. This uses Paypal, and is completely secure. There's no
pressure though, ScummVM DS is completely free. This is just for those
who would like to make a contribution to further development.



      Building from Sources
      ------------------------------------------------------------------------

ScummVM is an open source project.  This means that anyone is free to
take the source code to the project and make their own additions and fixes,
contributing them back to the authors for consideration for the next version.

To build ScummVM DS from source, it's probably better to checkout the
latest version of the code from the ScummVM SVN repository. The ScummVM
Sourceforge.net homepage <http://sourceforge.net/projects/scummvm> has
all the information about how to do this.

By default, ScummVM DS expects to find libmad, an MP3 compressor library
targeted for the ARM platform.  If you don't have this, you must disable
libmad support by opening 'backends/platform/ds/arm9/makefile' and
commenting out the line which says USE_MAD = 1.

Then, enter the 'backends/platform/ds' folder and type:

  make SCUMM_BUILD=a

The executable nds file will build inside 'backends/platform/ds/arm9/SCUMMVM-A'.

For other builds, substitute the letters b - g in the above line.

To build everything, type:

  make allbuildssafe
