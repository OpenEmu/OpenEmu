cmake_minimum_required(VERSION 3.20)
project(OpenEmuLinux VERSION 0.4.0 LANGUAGES C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/cores")
get_filename_component(OE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

# Compile the same pinned core sources used by the macOS OpenEmu targets.
# Only the Cocoa adapters are replaced. No RetroArch executable is involved.
set(GB "${OE_ROOT}/Gambatte")
file(GLOB_RECURSE GB_SOURCES CONFIGURE_DEPENDS "${GB}/src/*.cpp")
list(REMOVE_ITEM GB_SOURCES "${GB}/src/libgambatte/statesaver.cpp")
add_library(oe_gambatte SHARED native/gambatte.cpp ${GB_SOURCES} "${GB}/statesaver.cpp")
target_include_directories(oe_gambatte PRIVATE "${GB}/src" "${GB}/src/libgambatte" "${GB}/src/resample")
target_link_libraries(oe_gambatte PRIVATE m)
target_link_options(oe_gambatte PRIVATE -Wl,--no-undefined)

set(MG "${OE_ROOT}/mGBA")
file(GLOB MG_ARM CONFIGURE_DEPENDS "${MG}/src/arm/*.c")
file(GLOB_RECURSE MG_GBA CONFIGURE_DEPENDS "${MG}/src/gba/*.c")
list(FILTER MG_GBA EXCLUDE REGEX "/(test|debugger|extra)/|/sio/(dolphin|lockstep)\\.c$")
file(GLOB MG_UTIL CONFIGURE_DEPENDS "${MG}/src/util/*.c")
list(FILTER MG_UTIL EXCLUDE REGEX "/(gui|memory)\\.c$")
file(GLOB MG_CORE CONFIGURE_DEPENDS "${MG}/src/core/*.c")
list(FILTER MG_CORE EXCLUDE REGEX "/(scripting|thread)\\.c$")
file(GLOB MG_VFS CONFIGURE_DEPENDS "${MG}/src/util/vfs/*.c")
list(FILTER MG_VFS EXCLUDE REGEX "/vfs-(devlist|file|zip|lzma)\\.c$")
add_library(oe_mgba SHARED native/mgba.c ${MG_ARM} ${MG_GBA} ${MG_UTIL} ${MG_CORE} ${MG_VFS}
    "${MG}/src/gb/audio.c" "${MG}/src/platform/posix/memory.c"
    "${MG}/src/third-party/blip_buf/blip_buf.c" "${MG}/src/third-party/inih/ini.c")
target_include_directories(oe_mgba PRIVATE "${MG}/include" "${MG}/src")
target_compile_definitions(oe_mgba PRIVATE M_CORE_GBA DISABLE_THREADING MINIMAL_CORE=1
    MGBA_STANDALONE HAVE_STRDUP HAVE_STRNDUP HAVE_LOCALTIME_R HAVE_LOCALE
    HAVE_USELOCALE HAVE_STRTOF_L _GNU_SOURCE)
target_link_libraries(oe_mgba PRIVATE m)
target_link_options(oe_mgba PRIVATE -Wl,--no-undefined)

# Snes9x's portable sources, matching SNES9x.xcodeproj's source build phase.
# Included implementation fragments (e.g. SPC_DSP.cpp) are not compiled twice.
set(SN "${OE_ROOT}/SNES9x/src")
set(SN_FILES
    bml bsx c4 c4emu cheats cheats2 clip conffile controls cpu cpuexec cpuops
    crosshairs debug dma dsp dsp1 dsp2 dsp3 dsp4 fxdbg fxemu fxinst gfx globals
    logger memmap movie msu1 netplay obc1 ppu fscompat stream sa1 sa1cpu sdd1
    sdd1emu server seta seta010 seta011 seta018 sha256 snapshot snes9x spc7110
    srtc tile tileimpl-h2x1 tileimpl-n1x1 tileimpl-n2x1)
list(TRANSFORM SN_FILES PREPEND "${SN}/")
list(TRANSFORM SN_FILES APPEND ".cpp")
add_library(oe_snes9x SHARED native/snes9x.cpp ${SN_FILES}
    "${SN}/apu/apu.cpp" "${SN}/apu/bapu/dsp/sdsp.cpp"
    "${SN}/apu/bapu/smp/smp.cpp" "${SN}/apu/bapu/smp/smp_state.cpp")
target_include_directories(oe_snes9x PRIVATE "${SN}" "${SN}/apu/bapu")
target_compile_definitions(oe_snes9x PRIVATE HAVE_STRINGS_H HAVE_INTTYPES_H HAVE_STDINT_H RIGHTSHIFT_IS_SAR)
target_link_libraries(oe_snes9x PRIVATE m)
target_link_options(oe_snes9x PRIVATE -Wl,--no-undefined)
add_library(oe_libretro SHARED native/libretro.cpp)
target_link_libraries(oe_libretro PRIVATE dl)
target_link_options(oe_libretro PRIVATE -Wl,--no-undefined)
install(TARGETS oe_gambatte oe_mgba oe_snes9x oe_libretro LIBRARY DESTINATION lib/openemu-linux/cores)
