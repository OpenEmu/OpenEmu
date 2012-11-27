#------------------------------------------------------------------------------
#   scummvm.spec
#       This SPEC file controls the building of ScummVM RPM packages.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#   Prologue information
#------------------------------------------------------------------------------
Name		: scummvm
Version		: 1.6.0git
Release		: 1
Summary		: Graphic adventure game interpreter
Group		: Interpreters
License		: GPL

Url             : http://www.scummvm.org

Source		: %{name}-%{version}.tar.bz2
Source1		: libmad-0.15.1b.tar.bz2
Source2		: faad2-2.7.tar.bz2
BuildRoot	: %{_tmppath}/%{name}-%{version}-root

BuildRequires: desktop-file-utils
BuildRequires: libogg-devel
BuildRequires: libvorbis-devel
BuildRequires: flac-devel
BuildRequires: zlib-devel
BuildRequires: nasm
BuildRequires: SDL-devel >= 1.2.2
BuildRequires: freetype-devel

#------------------------------------------------------------------------------
#   Description
#------------------------------------------------------------------------------
%description
ScummVM is an interpreter that will play many graphic adventure games,
including LucasArts SCUMM games (such as Monkey Island 1-3, Day of the
Tentacle, Sam & Max, ...), many of Sierra's AGI and SCI games (such as King's
Quest 1-6, Space Quest 1-5, ...), Discworld 1 and 2, Simon the Sorcerer 1 and
2, Beneath A Steel Sky, Lure of the Temptress, Broken Sword 1 and 2, Flight of
the Amazon Queen, Gobliiins 1-3, The Legend of Kyrandia 1-3, many of Humongous
Entertainment's children's SCUMM games (including Freddi Fish and Putt Putt
games) and many more. See http://www.scummvm.org for a full compatibility list.

#------------------------------------------------------------------------------
#   install scripts
#------------------------------------------------------------------------------
%prep
%setup -q -a 1 -a 2 -n scummvm-%{version}
mkdir tmp

%build
(cd libmad-0.15.1b; ./configure --enable-static --disable-shared --prefix=%{_builddir}/scummvm-%{version}/tmp; make; make install)
(cd faad2-2.7; ./configure --enable-static --disable-shared --prefix=%{_builddir}/scummvm-%{version}/tmp; make; make install)
./configure --with-mad-prefix=%{_builddir}/scummvm-%{version}/tmp --with-faad-prefix=%{_builddir}/scummvm-%{version}/tmp --prefix=%{_prefix} --enable-release
make

%install
install -m755 -D scummvm %{buildroot}%{_bindir}/scummvm
install -m644 -D dists/scummvm.6 %{buildroot}%{_mandir}/man6/scummvm.6
install -m644 -D icons/scummvm.xpm %{buildroot}%{_datadir}/pixmaps/scummvm.xpm
install -m644 -D icons/scummvm.svg %{buildroot}%{_datadir}/icons/hicolor/scalable/apps/scummvm.svg
install -m644 -D dists/redhat/scummvm48.png %{buildroot}%{_datadir}/icons/hicolor/48x48/apps/scummvm.png
install -m644 -D gui/themes/scummclassic.zip %{buildroot}%{_datadir}/scummvm/scummclassic.zip
install -m644 -D gui/themes/scummmodern.zip %{buildroot}%{_datadir}/scummvm/scummmodern.zip
install -m644 -D gui/themes/translations.dat %{buildroot}%{_datadir}/scummvm/translations.dat
install -m644 -D dists/pred.dic %{buildroot}%{_datadir}/scummvm/pred.dic
install -m644 -D dists/engine-data/kyra.dat %{buildroot}%{_datadir}/scummvm/kyra.dat
install -m644 -D dists/engine-data/lure.dat %{buildroot}%{_datadir}/scummvm/lure.dat
install -m644 -D dists/engine-data/queen.tbl %{buildroot}%{_datadir}/scummvm/queen.tbl
install -m644 -D dists/engine-data/sky.cpt %{buildroot}%{_datadir}/scummvm/sky.cpt
install -m644 -D dists/engine-data/drascula.dat %{buildroot}%{_datadir}/scummvm/drascula.dat
install -m644 -D dists/engine-data/teenagent.dat %{buildroot}%{_datadir}/scummvm/teenagent.dat
install -m644 -D dists/engine-data/hugo.dat %{buildroot}%{_datadir}/scummvm/hugo.dat
install -m644 -D dists/engine-data/toon.dat %{buildroot}%{_datadir}/scummvm/toon.dat
desktop-file-install --vendor scummvm --dir=%{buildroot}/%{_datadir}/applications dists/scummvm.desktop

%clean
rm -Rf ${RPM_BUILD_ROOT}

%post
touch --no-create %{_datadir}/icons/hicolor || :
if [ -x %{_bindir}/gtk-update-icon-cache ]; then
        %{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/hicolor || :
fi

%postun
touch --no-create %{_datadir}/icons/hicolor || :
if [ -x %{_bindir}/gtk-update-icon-cache ]; then
        %{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/hicolor || :
fi

#------------------------------------------------------------------------------
#   Files listing.
#------------------------------------------------------------------------------
%files
%defattr(0644,root,root,0755)
%doc AUTHORS README NEWS COPYING COPYING.LGPL COPYING.FREEFONT COPYING.BSD COPYRIGHT
%attr(0755,root,root)%{_bindir}/scummvm
%{_datadir}/applications/*
%{_datadir}/pixmaps/scummvm.xpm
%{_datadir}/icons/hicolor/48x48/apps/scummvm.png
%{_datadir}/icons/hicolor/scalable/apps/scummvm.svg
%{_datadir}/scummvm/scumm*.zip
%{_datadir}/scummvm/translations.dat
%{_datadir}/scummvm/pred.dic
%{_datadir}/scummvm/kyra.dat
%{_datadir}/scummvm/queen.tbl
%{_datadir}/scummvm/sky.cpt
%{_datadir}/scummvm/lure.dat
%{_datadir}/scummvm/drascula.dat
%{_datadir}/scummvm/teenagent.dat
%{_datadir}/scummvm/hugo.dat
%{_datadir}/scummvm/toon.dat
%{_mandir}/man6/scummvm.6*

#------------------------------------------------------------------------------
#   Change Log
#------------------------------------------------------------------------------
%changelog
* Thu Dec 29 2011 (1.4.0-2)
  - include libfaad2
* Fri Sep 17 2010 (1.2.0)
  - include png/svg icons
  - remove libmpeg2
* Thu Sep 21 2006 (0.9.1)
  - include modern theme
* Mon Dec 20 2004 (0.7.0)
  - added AUTHORS file
* Thu Jul 15 2004 (0.6.0-2)
  - added .desktop file (modified from patch 891909)
  - used some elements of the .spec file for scummvm from http://livna.org/
* Mon Mar 15 2004 (0.6.0)
  - included libmad and libmpeg2
* Sat Aug 02 2003 (0.5.0)
  - Enhanced versions of Maniac Mansion and Zak McKracken are now supported and completable
  - Beneath A Steel Sky is now supported and completable
  - Added support for Amiga version of Monkey Island 1
  - Initial unplayable support for V1 version of Maniac Mansion/Zak McKracken
  - Curse of Monkey Island (COMI) support for playing from CD improved on Mac OS X
  - Loading COMI savegames for disk 2 doesn't anymore require disk 1 first
  - Rewritten iMUSE enginee, and many Music fixes (exp. Monkey Island 2)
  - Support for music in Humongous games and simon2dos/simon2talkie (XMIDI format)
  - Support for music in simon1demo (Proprietary format)
  - Complete music support for Simon the Sorcerer 2
  - Improved music and sound support in Zak256
  - Added Aspect Ratio option
  - Many other bug fixes, improvements and optimisations
* Sun May 25 2003 (0.4.1)
  - Added AdvMame3x filter
  - Fixed crash in Curse of Monkey Island (and possibly other games as well)
  - Fixed airport doors in Zak256
  - Fixed crash in SDL backend
  - Fixed various iMuse bugs
* Sun May 11 2003 (0.4.0)
  - Curse of Monkey Island (comi) support (experimental)
  - Added support for the EGA versions of Loom, Monkey Island and Indy3
  - Improved music support in Indy3 and the floppy versions of Monkey Islands
  - Many Simon the Sorcerer 1 & 2 improvements and fixes
  - Very pre-alpha Beneath a Steel Sky code. Don't expect it to do anything.
  - Even more pre-alpha support for V2 SCUMM games (Maniac Mansion and Zak)
  - Preliminary support for early Humongous Entertainment titles (very experimental)
  - New debug console and several GUI/Launcher enhancements
  - New Save/Load code (easier to expand while retaining compatibility)
  - DreamCast port now works with new games added for 0.3.0b
  - New official PalmOS port
  - Various minor and not so minor SCUMM game fixes
  - Large memory leak fixed for The Dig/ComI
  - SMUSH code optimised, frame dropping added for slower machines
  - Code cleanups
* Sun Dec 01 2002 (0.3.0)
  - massive cleanup work for iMUSE. Sam and Max music now plays correctly
  - many bugfixes for Zak256, + sound and music support
  - music support for Simon the Sorcerer on any platform with real MIDI
  - experimental support for Indy3 (VGA) - Indiana Jones + Last Crusade
  - completed support for Monkey1 VGA Floppy, The Dig
  - added akos16 implementation for The Dig and Full Throttle costumes
  - added digital iMUSE implementation for The Dig and Full Throttle music.
  - Loom CD speech+music syncronisation improved greatly
  - added midi-emulation via adlib, for platforms without sequencer support
  - code separation of various engine parts into several libraries
  - several fixes to prevent Simon the Sorcerer crashing and hanging
  - hundreds of bugfixes for many other games
  - new SMUSH video engine, for Full Throttle and The Dig
  - new in-game GUI
  - launcher dialog
* Sun Apr 14 2002 (0.2.0)
  - core engine rewrite
  - enhanced ingame GUI, including options/volume settings.
  - auto-save feature
  - added more command-line options, and configuration file
  - new ports and platforms (MorphOS, Macintosh, Dreamcast, Solaris, IRIX, etc)
  - graphics filtering added (2xSAI, Super2xSAI, SuperEagle, AdvMame2x)
  - support for MAD MP3 compressed audio
  - support for first non-SCUMM games (Simon the Sorcerer)
  - support for V4 games (Loom CD)
  - enhanced V6 game support (Sam and Max is now completable)
  - experimental support for V7 games (Full Throttle/The Dig)
  - experimental support for V3 games (Zak256/Indy3)
* Sun Jan 13 2002 (0.1.0)
  - loads of changes
* Fri Oct 12 2001 (0.0.2)
  - bug fixes
  - save & load support
* Mon Oct 8 2001 (0.0.1)
  - initial version
