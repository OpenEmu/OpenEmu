"""Original SNES diagnostic cartridge, dedicated to the public domain (CC0).

Uses a small 65816 program and an SPC700 tone program assembled here. This
contains no commercial cartridge or BIOS bytes. Header country/video settings
can be varied to test PAL timing and high-resolution/interlaced output.

Register references: SNESdev's Controller_reading and Booting_the_SPC700 pages.
"""
from pathlib import Path
from roms import FONT


def snes(pal=False, video_mode=0):
    code, labels, fixes = bytearray(), {}, []

    def emit(*values): code.extend(values)
    def label(name): labels[name] = 0x8000 + len(code)
    def absolute(op, target):
        emit(op)
        fixes.append((len(code), target, False))
        emit(0, 0)
    def branch(op, target):
        emit(op)
        fixes.append((len(code), target, True))
        emit(0)
    def store(address, value):
        emit(0xA9, value, 0x8D, address & 255, address >> 8)
    def zero(address): emit(0x9C, address & 255, address >> 8)
    def vram(address):
        store(0x2116, address & 255)
        store(0x2117, address >> 8)

    # Native mode, 8-bit accumulator and 16-bit indexes/stack.
    emit(0x78, 0x18, 0xFB, 0xC2, 0x10, 0xA2, 0xFF, 0x1F, 0x9A, 0x4B, 0xAB)
    zero(0x4200)
    zero(0x420C)
    zero(0x4016)
    store(0x2100, 0x80)
    emit(0xA2, 0x33, 0)
    label("clear_ppu")
    emit(0x9E, 0x00, 0x21, 0xCA)
    branch(0xD0, "clear_ppu")
    # Battery boot counter at LoROM SRAM $70:0000.
    emit(0xAF, 0, 0, 0x70, 0x1A, 0x8F, 0, 0, 0x70)

    # Upload the original SPC700 program through the standard IPL handshake.
    label("spc_ready")
    emit(0xAD, 0x40, 0x21, 0xC9, 0xAA)
    branch(0xD0, "spc_ready")
    store(0x2142, 0)
    store(0x2143, 2)
    store(0x2141, 1)
    store(0x2140, 0xCC)
    label("spc_begin")
    emit(0xCD, 0x40, 0x21)
    branch(0xD0, "spc_begin")
    emit(0xA2, 0, 0)
    label("spc_send")
    absolute(0xBD, "spc_data")
    emit(0x8D, 0x41, 0x21, 0x8A, 0x8D, 0x40, 0x21)
    label("spc_ack")
    emit(0xCD, 0x40, 0x21)
    branch(0xD0, "spc_ack")
    emit(0xE8, 0xE0, 0x19, 0x01)  # 281 bytes: $0200 through $0318
    branch(0xD0, "spc_send")
    store(0x2142, 0)
    store(0x2143, 2)
    zero(0x2141)
    emit(0xAD, 0x40, 0x21, 0x18, 0x69, 2, 0x8D, 0x40, 0x21)

    chars = list(FONT)
    tiles = bytearray()
    for ch in chars:
        for row in FONT[ch] + [0]:
            tiles.extend((row << 2, row << 2))
    solid = len(chars)
    tiles.extend(bytes([255] * 16))
    tilemap = bytearray(32 * 32 * 2)
    for row, text in [(4, "OPENEMU LINUX"), (7, "SNES CORE TEST"),
                      (10, "VIDEO INPUT AUDIO"), (14, "A B X Y L R"),
                      (18, "U D L R S T"), (24, "PRESS THE BUTTONS")]:
        column = (32 - len(text)) // 2
        for i, ch in enumerate(text): tilemap[(row * 32 + column + i) * 2] = chars.index(ch)
    # Colored edge to reveal stride/cropping mistakes in the Linux adapter.
    for row in range(28):
        for column in (0, 31): tilemap[(row * 32 + column) * 2] = solid
    store(0x2115, 0x80)
    for address, name, data in [(0, "tiles", tiles), (0x1000, "tilemap", tilemap)]:
        vram(address)
        emit(0xA2, 0, 0)
        label(name + "_copy")
        absolute(0xBD, name)
        emit(0x8D, 0x18, 0x21, 0xE8)
        absolute(0xBD, name)
        emit(0x8D, 0x19, 0x21, 0xE8, 0xE0, len(data) & 255, len(data) >> 8)
        branch(0xD0, name + "_copy")
    store(0x2121, 0)
    for rgb in [0x1443, 0x3810, 0x51B6, 0x7FBF]:
        store(0x2122, rgb & 255)
        store(0x2122, rgb >> 8)
    store(0x2107, 0x10)  # BG1 map at $1000 words
    store(0x212C, 1)     # BG1 enabled
    store(0x2133, video_mode)
    store(0x4200, 1)     # Automatic joypad reading (no NMI).
    store(0x2100, 15)    # Full brightness, display enabled.

    label("wait_active")
    emit(0xAD, 0x12, 0x42)
    branch(0x30, "wait_active")
    label("wait_vblank")
    emit(0xAD, 0x12, 0x42)
    branch(0x10, "wait_vblank")
    # Wait past the tiny gap before the auto-read busy flag asserts.
    emit(0xA2, 100, 0)
    label("delay")
    emit(0xCA)
    branch(0xD0, "delay")
    label("joy_busy")
    emit(0xAD, 0x12, 0x42, 0x29, 1)
    branch(0xD0, "joy_busy")
    emit(0xAD, 0x18, 0x42, 0x85, 0, 0xAD, 0x19, 0x42, 0x85, 1)
    buttons = [(0, 0x80), (1, 0x80), (0, 0x40), (1, 0x40), (0, 0x20), (0, 0x10),
               (1, 0x08), (1, 0x04), (1, 0x02), (1, 0x01), (1, 0x20), (1, 0x10)]
    for i, (address, mask) in enumerate(buttons):
        vram(0x1000 + (16 if i < 6 else 20) * 32 + 10 + (i % 6) * 2)
        emit(0xA5, address, 0x29, mask)
        branch(0xF0, f"off{i}")
        emit(0xA9, solid)
        branch(0x80, f"write{i}")
        label(f"off{i}")
        emit(0xA9, 0)
        label(f"write{i}")
        emit(0x8D, 0x18, 0x21)
        zero(0x2119)
    absolute(0x4C, "wait_active")
    label("rti")
    emit(0x40)
    label("tiles")
    code.extend(tiles)
    label("tilemap")
    code.extend(tilemap)
    label("spc_data")
    spc = bytearray()
    # MOV dp,#imm (8F imm dp); configure a quiet looping BRR tone on voice 0.
    for register, value in [(0x6C, 0x20), (0x5C, 0), (0x0C, 0x40), (0x1C, 0x40),
                            (0x00, 0x30), (0x01, 0x30), (0x02, 0), (0x03, 2),
                            (0x04, 0), (0x05, 0), (0x06, 0), (0x07, 0x40),
                            (0x5D, 3), (0x3D, 0), (0x4D, 0), (0x2D, 0), (0x4C, 1)]:
        spc.extend((0x8F, register, 0xF2, 0x8F, value, 0xF3))
    spc.extend((0x2F, 0xFE))
    spc.extend(bytes(0x100 - len(spc)))
    spc.extend((0x10, 3, 0x10, 3))  # Directory entry: sample start and loop $0310.
    spc.extend(bytes(12))
    spc.extend((0xB3, 0x77, 0x77, 0x77, 0x77, 0x99, 0x99, 0x99, 0x99))
    assert len(spc) == 0x119
    code.extend(spc)

    for position, target, relative in fixes:
        address = labels[target]
        if relative:
            delta = address - (0x8000 + position + 1)
            assert -128 <= delta <= 127
            code[position] = delta & 255
        else:
            code[position:position+2] = address.to_bytes(2, "little")
    assert len(code) < 0x7FC0
    rom = bytearray(131072)
    rom[:len(code)] = code
    rom[0x7FC0:0x7FD5] = b"OPENEMU LINUX TEST   ".ljust(21)
    rom[0x7FD5:0x7FDC] = bytes([0x20, 0x02, 7, 3, 2 if pal else 1, 0x33, 0])
    for vector in (0x7FE4, 0x7FE6, 0x7FE8, 0x7FEA, 0x7FEE, 0x7FF4, 0x7FFA, 0x7FFE):
        rom[vector:vector+2] = labels["rti"].to_bytes(2, "little")
    rom[0x7FFC:0x7FFE] = b"\x00\x80"
    checksum = (sum(rom) + 510) & 0xFFFF
    rom[0x7FDC:0x7FE0] = (checksum ^ 0xFFFF).to_bytes(2, "little") + checksum.to_bytes(2, "little")
    return bytes(rom)


if __name__ == "__main__":
    target = Path(__file__).resolve().parents[1] / "demos" / "OpenEmu Linux - SNES.sfc"
    target.write_bytes(snes())
    print(target)
