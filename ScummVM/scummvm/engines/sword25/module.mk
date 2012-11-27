MODULE := engines/sword25

MODULE_OBJS := \
	console.o \
	detection.o \
	sword25.o \
	fmv/movieplayer.o \
	fmv/movieplayer_script.o \
	gfx/animation.o \
	gfx/animationdescription.o \
	gfx/animationresource.o \
	gfx/animationtemplate.o \
	gfx/animationtemplateregistry.o \
	gfx/bitmap.o \
	gfx/dynamicbitmap.o \
	gfx/fontresource.o \
	gfx/graphicengine.o \
	gfx/graphicengine_script.o \
	gfx/panel.o \
	gfx/renderobject.o \
	gfx/renderobjectmanager.o \
	gfx/screenshot.o \
	gfx/staticbitmap.o \
	gfx/text.o \
	gfx/timedrenderobject.o \
	gfx/image/art.o \
	gfx/image/imgloader.o \
	gfx/image/renderedimage.o \
	gfx/image/swimage.o \
	gfx/image/vectorimage.o \
	gfx/image/vectorimagerenderer.o \
	input/inputengine.o \
	input/inputengine_script.o \
	kernel/filesystemutil.o \
	kernel/inputpersistenceblock.o \
	kernel/kernel.o \
	kernel/kernel_script.o \
	kernel/outputpersistenceblock.o \
	kernel/persistenceservice.o \
	kernel/resmanager.o \
	kernel/resource.o \
	math/geometry_script.o \
	math/polygon.o \
	math/region.o \
	math/regionregistry.o \
	math/vertex.o \
	math/walkregion.o \
	package/packagemanager.o \
	package/packagemanager_script.o \
	script/luabindhelper.o \
	script/luacallback.o \
	script/luascript.o \
	script/lua_extensions.o \
	sfx/soundengine.o \
	sfx/soundengine_script.o \
	util/lua/lapi.o \
	util/lua/lauxlib.o \
	util/lua/lbaselib.o \
	util/lua/lcode.o \
	util/lua/ldblib.o \
	util/lua/ldebug.o \
	util/lua/ldo.o \
	util/lua/lfunc.o \
	util/lua/lgc.o \
	util/lua/linit.o \
	util/lua/liolib.o \
	util/lua/llex.o \
	util/lua/lmathlib.o \
	util/lua/lmem.o \
	util/lua/loadlib.o \
	util/lua/lobject.o \
	util/lua/lopcodes.o \
	util/lua/loslib.o \
	util/lua/lparser.o \
	util/lua/lstate.o \
	util/lua/lstring.o \
	util/lua/lstrlib.o \
	util/lua/ltable.o \
	util/lua/ltablib.o \
	util/lua/ltm.o \
	util/lua/lvm.o \
	util/lua/lzio.o \
	util/lua/scummvm_file.o \
	util/pluto/pdep.o \
	util/pluto/pluto.o \
	util/pluto/plzio.o

# This module can be built as a plugin
ifeq ($(ENABLE_SWORD25), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
