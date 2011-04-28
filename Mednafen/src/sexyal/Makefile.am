AUTOMAKE_OPTIONS = subdir-objects
DEFS = -DLOCALEDIR=\"$(datadir)/locale\" @DEFS@ @MATH_OPTIMIZER_FLAGS@ @SDL_CFLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/intl

noinst_LIBRARIES        =       libsexyal.a

libsexyal_a_SOURCES	=      sexyal.cpp convert.cpp drivers/dummy.cpp

if HAVE_DIRECTSOUND
TMP_DIRECTSOUND	=	drivers/dsound.cpp
endif

if HAVE_OSSDSP
TMP_OSSDSP	=	drivers/oss.cpp
endif

if HAVE_JACK
TMP_JACK	=	drivers/jack.cpp
endif

if HAVE_SDL
TMP_SDL		=	drivers/sdl.cpp
libsexyal_a_CFLAGS = @SDL_CFLAGS@
endif

if HAVE_ALSA
TMP_ALSA	=	drivers/alsa.cpp
endif

libsexyal_a_SOURCES	+=	$(TMP_DIRECTSOUND) $(TMP_OSSDSP) $(TMP_JACK) $(TMP_SDL) $(TMP_ALSA)
