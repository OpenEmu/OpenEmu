#!perl
#
# ScummVM - Graphic Adventure Engine
#
# ScummVM is the legal property of its developers, whose names
# are too numerous to list here. Please refer to the COPYRIGHT
# file distributed with this source distribution.
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#

# This script constructs dictionary for use with predictive input
#
# Feed it with list of words. One word per line

%words = ();

while(<>) {
  chomp;

  s/[\x7f-\xff]//g; # remove unprintable characters

  next if /^.$/;    # skip one character words
  next if $_ eq ""; # skip empty words

  $words{$_} = 1;
}

%list = ();

for $_ (sort keys %words) {
  $word = $_;

  s/['-.&_!]/1/g;
  s/[abc]/2/g;
  s/[def]/3/g;
  s/[ghi]/4/g;
  s/[jkl]/5/g;
  s/[mno]/6/g;
  s/[pqrs]/7/g;
  s/[tuv]/8/g;
  s/[wxyz]/9/g;

  $list{$_} .= "$word ";
}

for $k (sort keys %list) {
  chop $list{$k};

  print "$k $list{$k}\n";
}
