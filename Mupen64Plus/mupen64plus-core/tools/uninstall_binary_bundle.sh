#!/bin/sh
#
# mupen64plus binary bundle uninstall script
#
# Copyright 2007-2009 The Mupen64Plus Development Team
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#

set -e

export PATH=/bin:/usr/bin

usage()
{
printf "usage: $(basename $0) [PREFIX] [SHAREDIR] [BINDIR] [LIBDIR] [PLUGINDIR] [MANDIR]
\tPREFIX    - installation directories prefix (default: /usr/local)
\tSHAREDIR  - path to Mupen64Plus shared data files (default: \$PREFIX/share/mupen64plus)
\tBINDIR    - path to Mupen64Plus binary program files (default: \$PREFIX/bin)
\tLIBDIR    - path to Mupen64Plus core library (default: \$PREFIX/lib)
\tPLUGINDIR - path to Mupen64Plus plugin libraries (default: \$PREFIX/lib/mupen64plus)
\tMANDIR    - path to manual files (default: \$PREFIX/man)
"
}

if [ $# -gt 6 ]; then
	usage
	exit 1
fi

PREFIX="${1:-/usr/local}"
SHAREDIR="${2:-${PREFIX}/share/mupen64plus}"
BINDIR="${3:-${PREFIX}/bin}"
LIBDIR="${4:-${PREFIX}/lib}"
PLUGINDIR="${5:-${PREFIX}/lib/mupen64plus}"
MANDIR="${6:-${PREFIX}/share/man}"

printf "Uninstalling Mupen64Plus Binary Bundle from ${PREFIX}\n"
# Mupen64Plus-Core
rm -f "${LIBDIR}"/libmupen64plus.so*
/sbin/ldconfig
rm -f "${SHAREDIR}/font.ttf"
rm -f "${SHAREDIR}/mupen64plus.cht"
rm -f "${SHAREDIR}/mupen64plus.ini"
rm -f "${SHAREDIR}"/doc/*
# Mupen64Plus-ROM
rm -f "${SHAREDIR}/m64p_test_rom.v64"
# Mupen64Plus-UI-Console
rm -f "${BINDIR}/mupen64plus"
rm -f "${MANDIR}/man6/mupen64plus.6"
# Plugins
rm -f "${PLUGINDIR}/mupen64plus-audio-sdl.so"
rm -f "${PLUGINDIR}/mupen64plus-input-sdl.so"
rm -f "${PLUGINDIR}/mupen64plus-rsp-hle.so"
rm -f "${PLUGINDIR}/mupen64plus-video-rice.so"
rm -f "${SHAREDIR}/RiceVideoLinux.ini"
rm -f "${SHAREDIR}/InputAutoCfg.ini"
# get rid of the empty dirs
rmdir --ignore-fail-on-non-empty "${SHAREDIR}/doc"
rmdir --ignore-fail-on-non-empty "${SHAREDIR}"
rmdir --ignore-fail-on-non-empty "${BINDIR}"
rmdir --ignore-fail-on-non-empty "${LIBDIR}"
rmdir --ignore-fail-on-non-empty "${PLUGINDIR}"
rmdir --ignore-fail-on-non-empty "${MANDIR}/man6"
rmdir --ignore-fail-on-non-empty "${MANDIR}"

printf "Done.\n"

