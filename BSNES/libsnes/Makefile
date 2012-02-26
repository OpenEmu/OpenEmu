include nall/Makefile
snes := snes
gameboy := gameboy
profile := performance
link :=

ifneq ($(platform),win)
   fpic = -fPIC
endif

ifeq ($(platform),win)
   CC = gcc
   CXX = g++
endif

ifeq ($(DEBUG),1)
   extraflags = -O0 -g -I. -I$(snes) $(fpic)
else
   extraflags = -O3 -fomit-frame-pointer -I. -I$(snes) $(fpic)
endif

ifeq ($(PROFILING),gen)
   extraflags += -fprofile-generate --coverage
   link += -fprofile-generate --coverage
endif

ifeq ($(PROFILING),use)
   extraflags += -fprofile-use
   link += -fprofile-use
endif

# implicit rules
compile = \
  $(strip \
    $(if $(filter %.c,$<), \
      $(CC) $(CFLAGS) $(extraflags) $1 -c $< -o $@, \
      $(if $(filter %.cpp,$<), \
        $(CXX) $(CXXFLAGS) $(extraflags) $1 -c $< -o $@, \
		  $(if $(filter %.s,$<), \
		    $(AS) $(ASFLAGS) -o $@ $<, \
      	) \
		 ) \
    ) \
  )

all: library;

include gameboy/Makefile


set-static:
ifneq ($(platform),win)
	$(eval fpic := )
endif

static: set-static static-library;

install: library-install;

uninstall: library-uninstall;

%.o: $<; $(call compile)
include $(snes)/Makefile

clean: 
	-@$(call delete,obj/*.o)
	-@$(call delete,obj/*.a)
	-@$(call delete,obj/*.so)
	-@$(call delete,obj/*.dylib)
	-@$(call delete,obj/*.dll)
	-@$(call delete,out/*.a)
	-@$(call delete,out/*.so)
	-@$(call delete,*.res)
	-@$(call delete,*.pgd)
	-@$(call delete,*.pgc)
	-@$(call delete,*.ilk)
	-@$(call delete,*.pdb)
	-@$(call delete,*.manifest)

archive-all:
	tar -cjf libsnes.tar.bz2 libco nall obj out snes Makefile cc.bat clean.bat sync.sh

help:;
