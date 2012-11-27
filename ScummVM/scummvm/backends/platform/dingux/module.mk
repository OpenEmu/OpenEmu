MODULE := backends/platform/dingux

MODULE_OBJS := \
	main.o \
	dingux.o 

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))

# Hack to ensure the SDL backend is built so we can use OSystem_SDL.
-include $(srcdir)/backends/platform/sdl/module.mk
