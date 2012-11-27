MODULE := backends/platform/wince

MODULE_OBJS := \
	CEActionsPocket.o \
	CEDevice.o \
	CEScaler.o \
	CEActionsSmartphone.o \
	CELauncherDialog.o \
	wince-sdl.o \
	CEgui/GUIElement.o \
	CEgui/Panel.o \
	CEgui/SDL_ImageResource.o \
	CEgui/ItemAction.o \
	CEgui/PanelItem.o \
	CEgui/Toolbar.o \
	CEgui/ItemSwitch.o \
	CEgui/PanelKeyboard.o \
	CEgui/ToolbarHandler.o \
	CEkeys/EventsBuffer.o \
	../../../gui/Actions.o \
	../../../gui/Key.o \
	../../../gui/KeysDialog.o \
	missing/missing.o \
	smartLandScale.o

ifndef DYNAMIC_MODULES
MODULE_OBJS += PocketSCUMM.o
endif

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))

# HACK: The wince backend is based on the SDL one, so we load that, too.
include $(srcdir)/backends/platform/sdl/module.mk
