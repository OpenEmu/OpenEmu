MODULE := gui

MODULE_OBJS := \
	about.o \
	chooser.o \
	console.o \
	debugger.o \
	dialog.o \
	error.o \
	gui-manager.o \
	launcher.o \
	massadd.o \
	message.o \
	object.o \
	options.o \
	predictivedialog.o \
	saveload.o \
	saveload-dialog.o \
	themebrowser.o \
	ThemeEngine.o \
	ThemeEval.o \
	ThemeLayout.o \
	ThemeParser.o \
	Tooltip.o \
	widget.o \
	widgets/editable.o \
	widgets/edittext.o \
	widgets/list.o \
	widgets/popup.o \
	widgets/scrollbar.o \
	widgets/tab.o

ifdef MACOSX
MODULE_OBJS += \
	browser_osx.o
else
MODULE_OBJS += \
	browser.o
endif

# Include common rules
include $(srcdir)/rules.mk
