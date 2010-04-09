sexyal_SOURCES	=      sexyal/sexyal.cpp sexyal/convert.cpp

if HAVE_DIRECTSOUND
TMP_DIRECTSOUND	=	sexyal/drivers/dsound.cpp
endif

if HAVE_OSSDSP
TMP_OSSDSP	=	sexyal/drivers/oss.cpp
endif

if HAVE_JACK
TMP_JACK	=	sexyal/drivers/jack.cpp
endif

if HAVE_SDL
TMP_SDL		=	sexyal/drivers/sdl.cpp
endif

if HAVE_ALSA
TMP_ALSA	=	sexyal/drivers/alsa.cpp
endif

sexyal_SOURCES	+=	sexyal/drivers/dummy.cpp $(TMP_DIRECTSOUND) $(TMP_OSSDSP) $(TMP_JACK) $(TMP_SDL) $(TMP_ALSA)
