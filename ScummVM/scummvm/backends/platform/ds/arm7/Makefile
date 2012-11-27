#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
# TARGET is the name of the output, if this ends with _mb generates a multiboot image
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET	:=	arm7
BUILD		:=	build
SOURCES	:=	gfx source source/libcartreset data
INCLUDES	:=	include build


# Enable support for debugger (must be the same as in the arm9 makefile)
#USE_DEBUGGER = 1

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb-interwork

# note: arm9tdmi isn't the correct CPU arch, but anything newer and LD
# *insists* it has a FPU or VFP, and it won't take no for an answer!
CFLAGS	:=	-g -Wall -O2\
		-mcpu=arm7tdmi -mtune=arm7tdmi -fomit-frame-pointer\
		-ffast-math \
		$(ARCH)

CFLAGS	+=	$(INCLUDE) -DARM7

ifdef USE_DEBUGGER
	CFLAGS += -DUSE_DEBUGGER
endif


CXXFLAGS	:=	$(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS	:=	-g $(ARCH)
LDFLAGS	:=	-g $(ARCH) -mno-fpu

#---------------------------------------------------------------------------------
# path to tools - this can be deleted if you set the path in windows
#---------------------------------------------------------------------------------
# export PATH		:=	/d/dev/ds/devkitARM_r11/bin:/bin

#---------------------------------------------------------------------------------
# PATH to ndslib - just make a system variable called NDSLIBPATH and be done with it
#---------------------------------------------------------------------------------
# NDSLIBPATH	:=	/d/dev/ds/ndslib/

#---------------------------------------------------------------------------------
# the prefix on the compiler executables
#---------------------------------------------------------------------------------
PREFIX			:=	arm-eabi-
#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:= -lnds7

ifdef USE_DEBUGGER
	LIBS += -ldswifi7
endif


#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
#LIBDIRS	:=	/home/neil/devkitpror21/libnds home/neil/devkitpror21/libnds/nds
LIBDIRS	:=	$(DEVKITPRO)/libnds


#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export CC		:=	$(PREFIX)gcc
export CXX		:=	$(PREFIX)g++
export AR		:=	$(PREFIX)ar
export OBJCOPY	:=	$(PREFIX)objcopy
#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
export LD		:=	$(CXX)
#export LD		:=	$(CC)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
PCXFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.pcx)))
BINFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.bin)))
PALFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.pal)))
RAWFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.raw)))
MAPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.map)))



export OFILES	:=	$(MAPFILES:.map=.o) $(RAWFILES:.raw=.o) $(PALFILES:.pal=.o) $(BINFILES:.bin=.o) $(PCXFILES:.pcx=.o)\
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) -Iquote -I$(CURDIR)/../commoninclude\
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include/nds)\
					-I$(CURDIR)/$(BUILD) -I$(CURDIR)/source/libcartreset

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@echo $(OFILES)
	@echo $(CFILES)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...$(TARGET)
	@rm -fr $(BUILD) *.bin

#---------------------------------------------------------------------------------
semiclean:
	@echo semiclean ...$(TARGET)


#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------

$(OUTPUT).bin	:	$(OUTPUT).elf

$(OUTPUT).elf	:	$(OFILES)



#---------------------------------------------------------------------------------
%.bin: %.elf
	@echo built ... $(notdir $@)
	$(OBJCOPY) -O binary  $(TARGET).elf $@

#---------------------------------------------------------------------------------
%.elf:
	echo ELF
	$(LD)  $(LDFLAGS) -specs=ds_arm7.specs $(OFILES) $(LIBPATHS) $(LIBS) -o $(TARGET).elf



#---------------------------------------------------------------------------------
# Compile Targets for C/C++
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
%.o : %.cpp
	@echo $(notdir $<)
	@echo $(CXX) -MM $(CXXFLAGS) -o $*.d $<
	@$(CXX) -MM $(CXXFLAGS) -o $*.d $<
	@$(CXX) $(CXXFLAGS) -c $< -o$@

#---------------------------------------------------------------------------------
%.o : %.c
	@echo $(notdir $<)
	@echo $(CC)  $(CFLAGS) -c $< -o$@
	@$(CC) -MM $(CFLAGS) -o $*.d $<
	@$(CC)  $(CFLAGS) -c $< -o$@

#---------------------------------------------------------------------------------
%.o : %.s
	@echo $(notdir $<)
	@$(CC) -MM $(CFLAGS) -o $*.d $<
	@$(CC)  $(ASFLAGS) -c $< -o$@

define bin2o
	cp $(<) $(*).tmp
	$(OBJCOPY) -I binary -O elf32-littlearm -B arm \
	--rename-section .data=.rodata \
	--redefine-sym _binary_$*_tmp_start=$*\
	--redefine-sym _binary_$*_tmp_end=$*_end\
	--redefine-sym _binary_$*_tmp_size=$*_size\
	$(*).tmp $(@)
	echo "extern const u8" $(*)"[];" > $(*).h
	echo "extern const u32" $(*)_size[]";" >> $(*).h
	rm $(*).tmp
endef

#---------------------------------------------------------------------------------
%.o	:	%.pcx
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.o	:	%.raw
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.o	:	%.pal
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.o	:	%.map
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.o	:	%.mdl
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)


-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
