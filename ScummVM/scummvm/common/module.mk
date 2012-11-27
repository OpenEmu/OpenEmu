MODULE := common

MODULE_OBJS := \
	archive.o \
	config-file.o \
	config-manager.o \
	coroutines.o \
	dcl.o \
	debug.o \
	error.o \
	EventDispatcher.o \
	EventMapper.o \
	EventRecorder.o \
	file.o \
	fs.o \
	gui_options.o \
	hashmap.o \
	iff_container.o \
	installshield_cab.o \
	language.o \
	localization.o \
	macresman.o \
	memorypool.o \
	md5.o \
	mutex.o \
	platform.o \
	quicktime.o \
	random.o \
	rational.o \
	rendermode.o \
	str.o \
	stream.o \
	system.o \
	textconsole.o \
	tokenizer.o \
	translation.o \
	unarj.o \
	unzip.o \
	util.o \
	winexe.o \
	winexe_ne.o \
	winexe_pe.o \
	xmlparser.o \
	zlib.o

MODULE_OBJS += \
	cosinetables.o \
	dct.o \
	fft.o \
	huffman.o \
	rdft.o \
	sinetables.o

# Include common rules
include $(srcdir)/rules.mk
