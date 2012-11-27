#!/bin/bash

TARGET=$1
BASESIZE=2097152

CARTSIZE=`ls -l $1 | cut -d" " -f5`

REMAINDER=`echo $CARTSIZE % $BASESIZE   | bc`
REMAINDER=`echo $BASESIZE - $REMAINDER  | bc`
CARTSIZE=`echo $CARTSIZE + $REMAINDER  | bc`

ucon64 -q --n64 --v64 --chk --padn=$CARTSIZE  $1
