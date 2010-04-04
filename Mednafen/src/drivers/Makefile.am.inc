mednafen_SOURCES += drivers/input.cpp drivers/opengl.cpp drivers/main.cpp drivers/joystick.cpp drivers/sound.cpp drivers/video.cpp drivers/netplay.cpp drivers/input-config.cpp
mednafen_SOURCES += drivers/hqxx-common.cpp drivers/hq2x.cpp drivers/hq3x.cpp drivers/hq4x.cpp drivers/scale2x.c drivers/scale3x.c drivers/scalebit.c
mednafen_SOURCES += drivers/config.cpp drivers/args.cpp drivers/nnx.cpp drivers/nongl.cpp drivers/console.cpp drivers/cheat.cpp drivers/fps.cpp 
mednafen_SOURCES += drivers/help.cpp drivers/video-state.cpp drivers/shader.cpp drivers/remote.cpp

if WANT_DEBUGGER
mednafen_SOURCES += drivers/debugger.cpp drivers/gfxdebugger.cpp drivers/memdebugger.cpp drivers/logdebugger.cpp drivers/prompt.cpp
endif
