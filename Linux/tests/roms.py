"""Original, redistributable diagnostic ROM generator (CC0-1.0).

No commercial ROM or BIOS data is used. The Game Boy ROM runs without a boot
ROM, draws a text/tile test, changes palette while A is held, and increments
cartridge RAM on each boot. This tests video, input and battery persistence.
"""
from pathlib import Path

FONT = {
" ":[0]*7,"O":[14,17,17,17,17,17,14],"P":[30,17,17,30,16,16,16],
"E":[31,16,16,30,16,16,31],"N":[17,25,25,21,19,19,17],
"M":[17,27,21,21,17,17,17],"U":[17,17,17,17,17,17,14],
"L":[16,16,16,16,16,16,31],"I":[31,4,4,4,4,4,31],
"X":[17,17,10,4,10,17,17],"R":[30,17,17,30,20,18,17],
"S":[15,16,16,14,1,1,30],"A":[14,17,17,31,17,17,17],
"T":[31,4,4,4,4,4,4],"C":[14,17,16,16,16,17,14],
"D":[30,17,17,17,17,17,30],"G":[14,17,16,23,17,17,15],
"B":[30,17,17,30,17,17,30],"Y":[17,17,10,4,4,4,4],
"F":[31,16,16,30,16,16,16],"V":[17,17,17,17,17,10,4],
"W":[17,17,17,21,21,27,17],"H":[17,17,17,31,17,17,17],
"K":[17,18,20,24,20,18,17],"Z":[31,1,2,4,8,16,31],
}


def gameboy(color=False):
    rom = bytearray(32768)
    rom[0x100:0x104] = b"\x00\xc3\x50\x01"
    # Standard cartridge validation bytes, required by boot-accurate cores.
    rom[0x104:0x134] = bytes.fromhex(
        "CEED6666CC0D000B03730083000C000D0008111F8889000EDCCC6EE6DDDDD999"
        "BBBB67636E0EECCCDDDC999FBBB9333E")
    title = b"OE LINUX COLOR" if color else b"OE LINUX TEST"
    rom[0x134:0x134+len(title)] = title
    rom[0x143] = 0x80 if color else 0
    rom[0x147:0x14A] = bytes([3, 0, 2])  # MBC1 + RAM + battery; 32 KiB ROM, 8 KiB RAM.
    code, labels, patches = bytearray(), {}, []
    def emit(*b): code.extend(b)
    def label(name): labels[name] = len(code) + 0x150
    def address(op, name):
        emit(op)
        patches.append((len(code), name))
        emit(0, 0)
    emit(0xF3, 0x31, 0xFE, 0xDF)  # DI, LD SP
    label("wait_lcd")
    emit(0xF0, 0x44, 0xFE, 144)
    address(0xDA, "wait_lcd")
    emit(0xAF, 0xE0, 0x40)  # LCD off
    emit(0x3E, 0x0A, 0xEA, 0, 0)  # enable cartridge RAM
    emit(0xFA, 0, 0xA0, 0x3C, 0xEA, 0, 0xA0)  # increment persistent boot counter
    emit(0x3E, 0x80, 0xE0, 0x26, 0x3E, 0x77, 0xE0, 0x24, 0x3E, 0x11, 0xE0, 0x25)
    emit(0x3E, 0x80, 0xE0, 0x11, 0x3E, 0xA0, 0xE0, 0x12, 0x3E, 0, 0xE0, 0x13, 0x3E, 0x87, 0xE0, 0x14)
    tiles = bytearray()
    chars = list(FONT)
    for char in chars:
        for row in FONT[char] + [0]:
            tiles.extend((row << 2, row << 2))
    tilemap = bytearray(1024)
    for row, text in [(3, "OPENEMU"), (5, "LINUX"), (9, "VIDEO TEST"), (11, "PRESS X"), (14, "GAME BOY COLOR" if color else "GAME BOY")]:
        col = (20-len(text))//2
        for i, ch in enumerate(text): tilemap[row*32+col+i] = chars.index(ch)
    for destination, data_label, length in [(0x8000, "tiles", len(tiles)), (0x9800, "map", len(tilemap))]:
        address(0x11, data_label)  # LD DE, source
        emit(0x21, destination & 255, destination >> 8, 0x01, length & 255, length >> 8)
        label(data_label + "_copy")
        emit(0x1A, 0x22, 0x13, 0x0B, 0x78, 0xB1)
        address(0xC2, data_label + "_copy")
    if color:
        emit(0x3E, 0x80, 0xE0, 0x68)
        for v in [0xDF, 0x7F, 0x16, 0x56, 0x0A, 0x35, 0x43, 0x10]:
            emit(0x3E, v, 0xE0, 0x69)
    emit(0x3E, 0xE4, 0xE0, 0x47, 0x3E, 0x91, 0xE0, 0x40)
    label("frame")
    emit(0xF0, 0x44, 0xFE, 144)
    address(0xDA, "frame")
    emit(0x3E, 0x10, 0xE0, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0xE6, 1)
    address(0xCA, "pressed")
    emit(0x3E, 0xE4)
    address(0xC3, "palette")
    label("pressed")
    emit(0x3E, 0x1B)
    label("palette")
    emit(0xE0, 0x47)
    if color:
        # CGB ignores BGP; change palette 0's first component on input too.
        emit(0x47, 0x3E, 0, 0xE0, 0x68, 0x78, 0xE0, 0x69)
    label("wait_end")
    emit(0xF0, 0x44, 0xFE, 144)
    address(0xD2, "wait_end")
    address(0xC3, "frame")
    label("tiles")
    code.extend(tiles)
    label("map")
    code.extend(tilemap)
    for offset, name in patches:
        code[offset:offset+2] = labels[name].to_bytes(2, "little")
    rom[0x150:0x150+len(code)] = code
    checksum = 0
    for x in rom[0x134:0x14D]: checksum = (checksum-x-1) & 255
    rom[0x14D] = checksum
    rom[0x14E:0x150] = (sum(rom) & 65535).to_bytes(2, "big")
    return bytes(rom)


if __name__ == "__main__":
    folder = Path(__file__).resolve().parents[1] / "demos"
    folder.mkdir(exist_ok=True)
    (folder / "OpenEmu Linux - Game Boy.gb").write_bytes(gameboy())
    (folder / "OpenEmu Linux - Game Boy Color.gbc").write_bytes(gameboy(True))
