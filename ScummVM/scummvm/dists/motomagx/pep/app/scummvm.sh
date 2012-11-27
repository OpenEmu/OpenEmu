#!/bin/sh
mypath=${0%/*}
LIBDIR1=/ezxlocal/download/mystuff/games/lib
LIBDIR2=/mmc/mmca1/games/lib
LIBDIR3=/mmc/mmca1/.Games/.lib
#LIBDIR3=/mmc/mmca1/.system/lib
LIBDIR4=$mypath/lib
#export SDL_QT_INVERT_ROTATION=1
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIBDIR1:$LIBDIR2:$LIBDIR3:$LIBDIR4
export HOME=$mypath
cd $mypath
exec $mypath/scummvm --gfx-mode=1x > $mypath/scummvm.log
