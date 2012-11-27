#!/bin/bash

echo Quick script to make building a distribution of the GP2X Wiz backend more consistent.

cd ../../../..

echo Building ScummVM for GP2X Wiz.

make gp2xwiz-bundle-debug
