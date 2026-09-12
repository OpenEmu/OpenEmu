/* Original diagnostic homebrew, CC0-1.0. No commercial ROM or BIOS assets.
 * ARMv4T GBA mode-3 RGB color bars, A-button changes bar color, SRAM boot count.
 * Build with clang --target=armv4t-none-eabi -nostdlib -fuse-ld=lld
 *   -Wl,-Ttext=0x08000000,--oformat=binary,-e,_start gba_diagnostic.s -o test.gba
 */
.syntax unified
.arm
.section .text
.global _start
_start:
    b start
    .space 0x9c
    .ascii "OE LINUX GBA"
    .ascii "OELX00"
    .byte 0x96
    .space 13
start:
    mov r0, #0x04000000
    mov r1, #0x400
    add r1, r1, #3
    strh r1, [r0]
    mov r0, #0x0e000000
    ldrb r1, [r0]
    add r1, r1, #1
    strb r1, [r0]
frame:
    mov r0, #0x04000000
    add r0, r0, #0x130
    ldrh r4, [r0]
    and r4, r4, #1
    mov r0, #0x06000000
    mov r2, #160
row:
    mov r3, #0
pixel:
    cmp r3, #80
    movlt r1, #31
    blt draw
    cmp r3, #160
    movlt r1, #0x3e0
    movge r1, #0x7c00
draw:
    cmp r4, #0
    eoreq r1, r1, #31
    strh r1, [r0], #2
    add r3, r3, #1
    cmp r3, #240
    blt pixel
    subs r2, r2, #1
    bne row
    mov r0, #0x04000000
wait_active:
    ldrh r1, [r0, #6]
    cmp r1, #160
    bge wait_active
wait_vblank:
    ldrh r1, [r0, #6]
    cmp r1, #160
    blt wait_vblank
    b frame
    .ascii "SRAM_V113"
    .space 32768
