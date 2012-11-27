#!/bin/bash

echo Quick script to make building a distribution of the GP2X Caanoo backend more consistent.

cd ../../../..

echo Building ScummVM for GP2X Caanoo.

make caanoo-bundle
