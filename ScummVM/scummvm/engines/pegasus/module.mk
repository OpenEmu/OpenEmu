MODULE := engines/pegasus

MODULE_OBJS = \
	compass.o \
	console.o \
	cursor.o \
	detection.o \
	elements.o \
	energymonitor.o \
	fader.o \
	gamestate.o \
	graphics.o \
	hotspot.o \
	input.o \
	interface.o \
	menu.o \
	movie.o \
	notification.o \
	pegasus.o \
	sound.o \
	surface.o \
	timers.o \
	transition.o \
	util.o \
	ai/ai_action.o \
	ai/ai_area.o \
	ai/ai_condition.o \
	ai/ai_rule.o \
	items/autodragger.o \
	items/inventory.o \
	items/inventorypicture.o \
	items/item.o \
	items/itemdragger.o \
	items/itemlist.o \
	items/biochips/aichip.o \
	items/biochips/biochipitem.o \
	items/biochips/mapchip.o \
	items/biochips/mapimage.o \
	items/biochips/opticalchip.o \
	items/biochips/pegasuschip.o \
	items/biochips/retscanchip.o \
	items/biochips/shieldchip.o \
	items/inventory/airmask.o \
	items/inventory/gascanister.o \
	items/inventory/inventoryitem.o \
	items/inventory/keycard.o \
	neighborhood/door.o \
	neighborhood/exit.o \
	neighborhood/extra.o \
	neighborhood/hotspotinfo.o \
	neighborhood/neighborhood.o \
	neighborhood/spot.o \
	neighborhood/turn.o \
	neighborhood/view.o \
	neighborhood/zoom.o \
	neighborhood/caldoria/caldoria.o \
	neighborhood/caldoria/caldoria4dsystem.o \
	neighborhood/caldoria/caldoriabomb.o \
	neighborhood/caldoria/caldoriamessages.o \
	neighborhood/caldoria/caldoriamirror.o \
	neighborhood/mars/energybeam.o \
	neighborhood/mars/gravitoncannon.o \
	neighborhood/mars/hermite.o \
	neighborhood/mars/mars.o \
	neighborhood/mars/planetmover.o \
	neighborhood/mars/reactor.o \
	neighborhood/mars/robotship.o \
	neighborhood/mars/shuttleenergymeter.o \
	neighborhood/mars/shuttlehud.o \
	neighborhood/mars/shuttleweapon.o \
	neighborhood/mars/spacechase3d.o \
	neighborhood/mars/spacejunk.o \
	neighborhood/mars/tractorbeam.o \
	neighborhood/norad/norad.o \
	neighborhood/norad/noradelevator.o \
	neighborhood/norad/pressuredoor.o \
	neighborhood/norad/pressuretracker.o \
	neighborhood/norad/subcontrolroom.o \
	neighborhood/norad/subplatform.o \
	neighborhood/norad/alpha/ecrmonitor.o \
	neighborhood/norad/alpha/fillingstation.o \
	neighborhood/norad/alpha/noradalpha.o \
	neighborhood/norad/alpha/panorama.o \
	neighborhood/norad/alpha/panoramascroll.o \
	neighborhood/norad/delta/globegame.o \
	neighborhood/norad/delta/noraddelta.o \
	neighborhood/prehistoric/prehistoric.o \
	neighborhood/tsa/fulltsa.o \
	neighborhood/tsa/tinytsa.o \
	neighborhood/wsc/moleculebin.o \
	neighborhood/wsc/wsc.o


# This module can be built as a plugin
ifeq ($(ENABLE_PEGASUS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
