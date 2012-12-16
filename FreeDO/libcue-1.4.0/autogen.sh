#!/bin/sh

echo "aclocal.."
aclocal --force			|| exit 1
echo "libtoolize.."
libtoolize --force		|| exit 1
echo "autoheader.."
autoheader --force		|| exit 1
if [ -f config.h.in~ ]; then
	rm config.h.in~
fi
echo "automake.."
automake --force --add-missing	|| exit 1
echo "autoconf.."
autoconf --force		|| exit 1

if [ -f configure ]; then
	./configure $*
fi
