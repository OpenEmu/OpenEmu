#!/bin/sh
FORMAT=libsnes
START_DIR=`pwd`
BUILD_DIR_EXECUTABLES=`pwd`/wii/pkg

#******************
# PROGRAM FUNCTIONS
#******************

function clean()
{
	make -f Makefile.libsnes clean
}

function make_libsnes()
{
	make -f Makefile.libsnes
}

function make_libsnes_debug()
{
	make -f Makefile.libsnes DEBUG=1
}

#******************
# DISPLAY FUNCTIONS
#******************

function title()
{
	echo ""
	echo "***********************"
	echo "COMPILER SCRIPT FOR $FORMAT"
	echo "***********************"
}

function display_clean()
{
	echo "clean          Clean the object files"
}

function display_make()
{
	echo "make           Compile libsnes library"
}


function display_make_debug()
{
	echo "make_debug     Compile DEBUG libsnes library "
}

function display_all_options()
{
	display_clean
	display_make
	display_make_debug
}

function display_usage()
{
	echo "Usage: compile_libsnes.sh [options]"
	echo "Options:"
	display_all_options
}

#***********************
# MAIN CONTROL FLOW LOOP
#***********************

title
if [ ! -n "$1" ]; then
	display_usage
else
	for i in "$@"
	do
		if [ "$i" = "help" ]; then
			display_usage
		fi
		if [ "$i" = "clean" ]; then
			echo ""
			echo "*************************************"
			echo "DOING:"
			display_clean
			echo "*************************************"
			clean
		fi
		if [ "$i" = "make" ]; then
			echo ""
			echo "*************************************"
			echo "DOING:"
			display_make
			echo "*************************************"
			make_libsnes
		fi
		if [ "$i" = "make_debug" ]; then
			echo ""
			echo "*************************************"
			echo "DOING:"
			display_make_debug
			echo "*************************************"
			make_libsnes_debug
		fi
	done
fi
