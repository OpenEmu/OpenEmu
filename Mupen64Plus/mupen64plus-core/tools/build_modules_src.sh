#!/bin/sh
#/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# *   Mupen64plus - build_modules_src.sh                                    *
# *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
# *   Copyright (C) 2009 Richard Goedeken                                   *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# *   This program is distributed in the hope that it will be useful,       *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU General Public License for more details.                          *
# *                                                                         *
# *   You should have received a copy of the GNU General Public License     *
# *   along with this program; if not, write to the                         *
# *   Free Software Foundation, Inc.,                                       *
# *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

# terminate the script if any commands return a non-zero error code
set -e

if [ $# -lt 2 ]; then
    echo "Usage: build_modules_src.sh <tag-name> <build-name>"
    exit 1
fi

modules='mupen64plus-core mupen64plus-rom mupen64plus-ui-console mupen64plus-audio-sdl mupen64plus-input-sdl mupen64plus-rsp-hle mupen64plus-video-rice'
for modname in ${modules}; do
  echo "************************************ Downloading and packaging module source code: ${modname}"
  rm -rf "tmp"
  EXCLUDE="--exclude .hgtags --exclude .hg_archival.txt --exclude .hgignore"
  TARTAG=""
  OUTPUTDIR="${modname}-$2"
  hg clone --noupdate "http://bitbucket.org/richard42/$modname" "tmp"
  cd tmp
  hg archive --no-decode --type tar --prefix "${OUTPUTDIR}/" ${EXCLUDE} -r $1 "../${OUTPUTDIR}${TARTAG}.tar"
  cd ..
  gzip -n -f "${OUTPUTDIR}${TARTAG}.tar"
  rm -rf "tmp"
done
