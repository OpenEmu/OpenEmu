AM_YFLAGS		= -d

EXTRA_DIST		= libcue.pc.in

pkgconfigdir		= $(libdir)/pkgconfig
pkgconfig_DATA		= libcue.pc


INCLUDES		=				\
			-I.				\
			-Wall -Wmissing-declarations

libcue_la_SOURCES	=				\
			cd.c				\
			cdtext.c			\
			rem.c				\
			time.c				\
			cue_parser.y			\
			cue_scanner.l

libcueinclude_HEADERS	= 				\
			libcue.h			\
			cd.h				\
			cdtext.h			\
			rem.h				\
			time.h

lib_LTLIBRARIES		= libcue.la

libcue_la_LDFLAGS	= -version-info @LIBCUE_LIBTOOL_VERSION@
libcueincludedir	= $(includedir)/@LIBCUE_LIBRARY_NAME@-@LIBCUE_API_VERSION@/@LIBCUE_LIBRARY_NAME@
