#!/usr/bin/python
'''* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - code_convert.c                                          *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2010 Rhett Osborne                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Usage:

python code_convert.py > ../data/mupencheat.txt < ../data/mupen64plus.cht

'''

from sys import stdin

class cheat:
    def __init__(self):
        self.n=""
        self.d=""
        self.c=[]
        self.v=0
        self.hb='00'
    def add(self, l):
        if(self.n == ""):
            return
        l.append(" cn %s"%(self.n))
        if(self.d != ""): l.append("  cd %s"%(self.d))
        for code in self.c:
            l.append("  "+code)
    def clear(self):
        self.n=""
        self.d=""
        self.c=[]
        self.v=0
        

l=[]
cCount=0
_cs = []
for i in range(225):
 _cs.append(cheat())
cs = _cs[:]

def print_l():
    global l, cs
    for cheat in cs:
        cheat.add(l)
    for line in l:
        print line.replace("\x00", "")
    l=[]
    cCount=0
    for i in range(225):
        cs[i].clear()

lines = stdin.read().split("\n")

for line in lines:
    if len(line) < 2: continue
    elif(line[:2] == "//" and line != "//----" and line != "//---" ):
        l.append(line)
    elif len(line) < 4: continue
    elif(line[0] == '[' and line[-1] == ']' and len(line) > 23):
        print_l()
        l.append("\ncrc %s" % line[1:-1])
    elif(line[:5] == "Name="):
        l.append("gn %s" % (line[5:]))
    elif(line[:5] == "Cheat"):
        t = line[5:].split('=')[0]
        if (len(t)>1 and t[-2] == '_'):
            n = int(t[:-2])
            if(t[-1] == 'N'):
                cs[n].d = line.split("=")[1]
            else:
                for option in line.split("=")[1].split("$")[1:]:
                    if(len(option) < 4):
                        break;
                    if(option[-1]==','): end =-1
                    else: end = None
                    if(option[2] == " "):
                        cs[n].c[cs[n].v] += "%s%s:\"%s\""%(cs[n].hb,option[:2],option[3:end].replace("\"", "\\\""))
                    else:
                        cs[n].c[cs[n].v] += "%s:\"%s\""%(option[:4],option[5:end].replace("\"", "\\\""))
                    cs[n].c[cs[n].v]+=','
                cs[n].c[cs[n].v] = cs[n].c[cs[n].v][:-1]
        else:
            n = int(t)
            cn = line.split('"')
            cs[n].c = cn[2][1:].split(',')
            cs[n].n = cn[1];
            i=0
            for cheat in cs[n].c:
                if(cheat[-1] == '?'):
                    if(cheat[-2:] == '??' and cheat[-4:-2] != '??'):
                        cs[n].hb = cheat[-4:-2]
                    else:
                        cs[n].hb = '00'
                    cs[n].c[i] = cheat[:9] + "???? ";
                    cs[n].v=i
                i+=1
        if(n > cCount):
            cCount = n
    elif(line != "//----" and line != "//---" ):
        l.append("//%s" %line)
