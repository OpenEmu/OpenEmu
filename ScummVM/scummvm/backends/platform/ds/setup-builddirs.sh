#!/bin/sh

# TODO: Allow specifying (a list of) build ids (a, b, ...) on the command line.
# TODO: Allow overriding the LOGO variable

# TODO: Computer srcdir in a better way
srcdir=../../../`dirname $0`/..

BASE_PARAM="--host=ds --disable-translation --disable-debug --disable-all-engines"

for build in a b c d e f g h i k; do
	echo "--- Setting up build $build ---"
	mkdir -p build-$build && cd build-$build
	case $build in
	a)
		EXTRA_PARAM="--enable-scumm"
		;;
	b)
		EXTRA_PARAM="--enable-sky --enable-queen"
		;;
	c)
		EXTRA_PARAM="--enable-agos"
		;;
	d)
		EXTRA_PARAM="--enable-gob --enable-cine --enable-agi"
		;;
	e)
		EXTRA_PARAM="--enable-saga --disable-mad"
		;;
	f)
		EXTRA_PARAM="--enable-kyra --disable-mad"
		;;
	g)
		EXTRA_PARAM="--enable-lure"
		;;
	h)
		EXTRA_PARAM="--enable-parallaction"
		;;
	i)
		EXTRA_PARAM="--enable-made --disable-mad"
		;;
	k)
		EXTRA_PARAM="--enable-cruise --disable-mad"
		;;
	*)
		echo "Invalid build $build selected"
		exit 1
		;;
	esac

	defname="DS_BUILD_`echo $build | tr '[a-z]' '[A-Z]'`"
	CPPFLAGS="$CPPFLAGS -D$defname"
	$srcdir/configure $BASE_PARAM $EXTRA_PARAM
	cd ..
	echo DONE
	echo
done
