#!/bin/sh

. /usr/local/angstrom/arm/environment-setup

CROSS_COMPILE=arm-angstrom-linux-gnueabi-
export CROSS_COMPILE

echo Quick script to make building a distribution of the OpenPanodra backend more consistent.

cd ../../../..

make op-bundle
make op-pnd
