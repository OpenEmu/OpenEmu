include_directories(
	${AMeteor_INCLUDE_DIRS}
)

link_directories(
	${AMeteor_LIBRARY_DIRS}
)

set(CMAKE_CXX_FLAGS_RELEASE
	"${CMAKE_CXX_FLAGS_RELEASE} -fno-exceptions -fno-rtti")

set(AMeteor_SRC
	audio/dsound.cpp
	audio/sound1.cpp
	audio/sound2.cpp
	audio/sound4.cpp
	audio/speaker.cpp
	disassembler/argimmediate.cpp
	disassembler/argmulregisters.cpp
	disassembler/argpsr.cpp
	disassembler/argregister.cpp
	disassembler/argrelative.cpp
	disassembler/argshift.cpp
	disassembler/arguimmediate.cpp
	disassembler/arguments.cpp
	disassembler/instruction.cpp
	graphics/bglayer.cpp
	graphics/object.cpp
	graphics/objects.cpp
	graphics/renderer.cpp
	graphics/screen.cpp
	ameteor.cpp
	bios.cpp
	cartmem.cpp
	clock.cpp
	cpu.cpp
	debug.cpp
	dma.cpp
	eeprom.cpp
	flash.cpp
	interpreter.cpp
	interpreter_arm.cpp
	interpreter_thumb.cpp
	io.cpp
	keypad.cpp
	lcd.cpp
	memory.cpp
	sound.cpp
	sram.cpp
	timer.cpp
)

add_library(ameteor ${AMeteor_SRC})

target_link_libraries(ameteor
	${AMeteor_LIBRARIES}
)
