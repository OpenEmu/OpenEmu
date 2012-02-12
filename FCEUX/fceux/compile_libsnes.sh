#!/bin/sh
FORMAT=libsnes
START_DIR=`pwd`

#******************
# PROGRAM FUNCTIONS
#******************

function clean_fceux()
{
	make -f Makefile.libsnes-fceux clean
}

function clean_fceumm()
{
	make -f Makefile.libsnes-fceumm clean
}

function make_libsnes_fceumm()
{
	make -f Makefile.libsnes-fceumm
}

function make_libsnes_fceumm_debug()
{
	make -f Makefile.libsnes-fceumm DEBUG=1
}

function make_libsnes_fceux()
{
	make -f Makefile.libsnes-fceux
}

function make_libsnes_fceux_debug()
{
	make -f Makefile.libsnes-fceux DEBUG=1
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

function display_clean_fceumm()
{
	echo "clean_fceumm          Clean the object files"
}

function display_clean_fceux()
{
	echo "clean_fceux           Clean the object files"
}

function display_make_fceux()
{
	echo "make_fceux            Compile libsnes library for FCEUx"
}

function display_make_fceux_debug()
{
	echo "make_fceux_debug      Compile DEBUG libsnes library for FCEUx"
}

function display_make_fceumm()
{
	echo "make_fceumm           Compile libsnes library for FCEUmm"
}

function display_make_fceumm_debug()
{
	echo "make_fceumm_debug     Compile DEBUG libsnes library for FCEUmm"
}

function display_all_options()
{
	display_clean_fceux
	display_clean_fceumm
	display_make_fceux
	display_make_fceux_debug
	display_make_fceumm
	display_make_fceumm_debug
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
		if [ "$i" = "clean_fceux" ]; then
			echo ""
			echo "*************************************"
			echo "DOING:"
			display_clean_fceux
			echo "*************************************"
			clean_fceux
		fi
		if [ "$i" = "clean_fceumm" ]; then
			echo ""
			echo "*************************************"
			echo "DOING:"
			display_clean_fceumm
			echo "*************************************"
			clean_fceumm
		fi
		if [ "$i" = "make_fceux" ]; then
			echo ""
			echo "*************************************"
			echo "DOING:"
			display_make_fceux
			echo "*************************************"
			make_libsnes_fceux
		fi
		if [ "$i" = "make_fceux_debug" ]; then
			echo ""
			echo "*************************************"
			echo "DOING:"
			display_make_fceux_debug
			echo "*************************************"
			make_libsnes_fceux_debug
		fi
		if [ "$i" = "make_fceumm" ]; then
			echo ""
			echo "*************************************"
			echo "DOING:"
			display_make_fceumm
			echo "*************************************"
			make_libsnes_fceumm
		fi
		if [ "$i" = "make_fceumm_debug" ]; then
			echo ""
			echo "*************************************"
			echo "DOING:"
			display_make_fceumm_debug
			echo "*************************************"
			make_libsnes_fceumm_debug
		fi
	done
fi
