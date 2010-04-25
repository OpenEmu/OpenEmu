#/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# *   Mupen64plus - MAKEFILE.MAK                                            *
# *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
# *   Copyright (C) 2008 Marshallh                                          *
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

sm.bin: demo.lnk snmain.obj \
                 demo.obj \
                 gfxlib.obj \
                 cfb.obj \
                 stack.obj \
                 zbuf.obj \
                 fifo.obj 


        psylink /p /m /c /s @demo.lnk,demo.bin,demo.sym,demo.map

        ../util/runcart -f -a 0x800F1800 -h ../util/ultra1.v64 -n 'Mupen64Plus' demo.bin
        ias E008 demo.bin
	


snmain.obj: snmain.s

.c.obj:
	
	ccn64  -w -O2 -I. -IC:/n64/include -IC:/n64/include/PR -c $*.c
.s.obj:
	asmn6432 /zd /l /ow- /g $*,$*
