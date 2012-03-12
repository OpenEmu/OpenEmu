#	Main Makefile for FB Alpha, execute an appropriate system-specific makefile

export

#
#	Declare variables
#

# Make a special build, pass the quoted text as comment (use FORCE_UPDATE declaration below to force recompilation of resources)
# SPECIALBUILD = "This text will appear in the property sheet of the .exe file"



#
#	Flags. Uncomment any of these declarations to enable their function.
#

# Include Unicode support
UNICODE = 1

# Build A68K ASM 68000 core
BUILD_A68K = 1

# Include x86 Assembly routines
BUILD_X86_ASM = 1

# Build for x64 targets (MinGW64 and MSVC only, this will undefine BUILD_A68K and BUILD_X86_ASM)
#BUILD_X64_EXE = 1

# Include 7-zip support
INCLUDE_7Z_SUPPORT = 1

# Include symbols and other debug information in the executable
#SYMBOL = 1

# Include features for debugging drivers
#DEBUG	= 1

# Include rom set verifying features (comment this for release builds)
#ROM_VERIFY = 1

# Force recompilation of files that need it (i.e. use __TIME__, __DATE__, SPECIALBUILD).
FORCE_UPDATE = 1

# Use the __fastcall calling convention when interfacing with A68K/Musashi/Doze
FASTCALL = 1

# Compress executable with upx (the DEBUG option ignores this)
# COMPRESS = 1

# Perl is available
PERL = 1

# Endianness
LSB_FIRST = 1

# Include png.h from burner.h
INCLUDE_LIB_PNGH = 1


#
#	execute an appropriate system-specific makefile
#

mingw345: FORCE
	@$(MAKE) -s -f makefile.mingw GCC345=1

mingw452: FORCE
	@$(MAKE) -s -f makefile.mingw GCC452=1
	
mingw461: FORCE
	@$(MAKE) -s -f makefile.mingw GCC461=1

sdl: FORCE
	@$(MAKE) -s -f makefile.sdl

vc: FORCE
	@$(MAKE) -s -f makefile.vc

FORCE: