AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@ @SDL_CFLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl -I$(top_srcdir)

noinst_LIBRARIES	=	libmdfnsdl.a

#libmdfnsdl_a_CXXFLAGS = @SDL_CFLAGS@
libmdfnsdl_a_SOURCES = input.cpp opengl.cpp main.cpp ers.cpp joystick.cpp sound.cpp video.cpp netplay.cpp input-config.cpp
libmdfnsdl_a_SOURCES += hqxx-common.cpp hq2x.cpp hq3x.cpp hq4x.cpp scale2x.c scale3x.c scalebit.c 2xSaI.cpp
libmdfnsdl_a_SOURCES += args.cpp nnx.cpp nongl.cpp overlay.cpp console.cpp cheat.cpp fps.cpp 
libmdfnsdl_a_SOURCES += help.cpp video-state.cpp shader.cpp remote.cpp

# libmdfnsdl_a_SOURCES += netconnection.cpp netconnection_sdlnet.cpp

if WANT_DEBUGGER
libmdfnsdl_a_SOURCES += debugger.cpp gfxdebugger.cpp memdebugger.cpp logdebugger.cpp prompt.cpp
endif
