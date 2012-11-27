#!/bin/sh -e
#
# dist-module.sh - make release tarballs for one SVN module
#
# Largely based on dist-fink.sh, Copyright (c) 2001 Christoph Pfisterer.
# Modified to use Subversion instead of CVS by Max Horn in 2007.
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

### configuration

svnroot='https://scummvm.svn.sourceforge.net/svnroot/scummvm'

### init

if [ $# -lt 2 ]; then
  echo "Usage: $0 <module> <version-number> [<temporary-directory> [<tag>]]"
  exit 1
fi

module=$1
version=$2
tmpdir=${3:-/tmp}
tag=$4
if [ -z "$tag" ]; then
  tag=release-`echo $version | sed 's/\./-/g'`
fi
fullname="$module-$version"

echo "packaging $module release $version, SVN tag $tag"

### setup temp directory

mkdir -p $tmpdir
cd $tmpdir
umask 022

if [ -d $fullname ]; then
  echo "There is a left-over directory in $tmpdir."
  echo "Remove $fullname, then try again."
  exit 1
fi

### check code out from SVN
# TODO: Add support for making tarballs from trunk / branches?

echo "Exporting module $module, tag $tag from SVN:"
svn export "$svnroot/$module/tags/$tag" $fullname
if [ ! -d $fullname ]; then
  echo "SVN export failed, directory $fullname doesn't exist!"
  exit 1
fi

### roll the tarball

echo "Creating tarball $fullname.tar:"
rm -f $fullname.tar $fullname.tar.gz
tar -cvf $fullname.tar $fullname

echo "Compressing (using gzip) tarball $fullname.tar.gz..."
gzip -c9 $fullname.tar > $fullname.tar.gz
if [ ! -f $fullname.tar.gz ]; then
  echo "Packaging failed, $fullname.tar.gz doesn't exist!"
  exit 1
fi

echo "Compressing (using bzip2) tarball $fullname.tar.bz2..."
bzip2 -c9 $fullname.tar > $fullname.tar.bz2
if [ ! -f $fullname.tar.bz2 ]; then
  echo "Packaging failed, $fullname.tar.bz2 doesn't exist!"
  exit 1
fi

echo "Zipping $fullname.zip..."
zip -r9 $fullname.zip $fullname
if [ ! -f $fullname.zip ]; then
  echo "Packaging failed, $fullname.zip doesn't exist!"
  exit 1
fi


### finish up

echo "Done:"
ls -l $fullname.tar.gz $fullname.tar.bz2 $fullname.zip

exit 0
