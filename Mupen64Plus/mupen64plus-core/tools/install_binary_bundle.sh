#!/bin/sh
#
# mupen64plus binary bundle install script
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

GINSTALLFLAG=-D

if `which ginstall >/dev/null 2>&1`; then
    INSTALL=ginstall
elif install --help >/dev/null 2>&1; then
    INSTALL=install
elif [ -e "`which install 2>/dev/null`" ]; then 
    printf "warning: GNU install not found, assuming BSD install\n" >&2
    INSTALL=install
    GINSTALLFLAG=
else
    printf "error: install tool not found\n" >&2
    exit 1
fi

INSTALL_STRIP_FLAG="${INSTALL_STRIP_FLAG:=-s}"

usage()
{
printf "usage: $(basename $0) [PREFIX] [SHAREDIR] [BINDIR] [LIBDIR] [PLUGINDIR] [MANDIR]
\tPREFIX    - installation directories prefix (default: /usr/local)
\tSHAREDIR  - path to Mupen64Plus shared data files (default: \$PREFIX/share/mupen64plus)
\tBINDIR    - path to Mupen64Plus binary program files (default: \$PREFIX/bin)
\tLIBDIR    - path to Mupen64Plus core library (default: \$PREFIX/lib)
\tPLUGINDIR - path to Mupen64Plus plugin libraries (default: \$PREFIX/lib/mupen64plus)
\tMANDIR    - path to manual files (default: \$PREFIX/share/man)
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

printf "Installing Mupen64Plus Binary Bundle to ${PREFIX}\n"
# Mupen64Plus-Core
$INSTALL -d -v "${LIBDIR}"
$INSTALL -m 0644 "${INSTALL_STRIP_FLAG}" libmupen64plus.so.2.* "${LIBDIR}"
/sbin/ldconfig
$INSTALL -d -v "${SHAREDIR}"
$INSTALL -m 0644 font.ttf "${SHAREDIR}"
$INSTALL -m 0644 mupen64plus.cht "${SHAREDIR}"
$INSTALL -m 0644 mupen64plus.ini "${SHAREDIR}"
$INSTALL -d -v "${SHAREDIR}/doc"
$INSTALL -m 0644 doc/* "${SHAREDIR}/doc"
# Mupen64Plus-ROM
$INSTALL -m 0644 m64p_test_rom.v64 "${SHAREDIR}"
# Mupen64Plus-UI-Console
$INSTALL -d -v "${BINDIR}"
$INSTALL $GINSTALLFLAG -m 0755 mupen64plus "${BINDIR}"
$INSTALL -d -v "${MANDIR}/man6"
$INSTALL -m 0644 man6/mupen64plus.6 "${MANDIR}/man6"
# Plugins
$INSTALL -d -v "${PLUGINDIR}"
$INSTALL -m 0644 "${INSTALL_STRIP_FLAG}" mupen64plus-audio-sdl.so "${PLUGINDIR}"
$INSTALL -m 0644 "${INSTALL_STRIP_FLAG}" mupen64plus-input-sdl.so "${PLUGINDIR}"
$INSTALL -m 0644 "${INSTALL_STRIP_FLAG}" mupen64plus-rsp-hle.so "${PLUGINDIR}"
$INSTALL -m 0644 "${INSTALL_STRIP_FLAG}" mupen64plus-video-rice.so "${PLUGINDIR}"
$INSTALL -m 0644 RiceVideoLinux.ini "${SHAREDIR}"
$INSTALL -m 0644 InputAutoCfg.ini "${SHAREDIR}"

printf "Installation successful.\n"

