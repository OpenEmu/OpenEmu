;       ----------------------------------------------------

;    glitchNES - version 0.1
;    Copyright 2009 Don Miller
;    For more information, visit: http://www.no-carrier.com

;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.

;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.

;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.

;       ----------------------------------------------------

buttons EQU $c0
tilenum EQU $c1
scroll_h EQU $c2
scroll_v EQU $c3
PPU_ADDR EQU $c4

;       ----------------------------------------------------

        .ORG $7ff0
Header:                         ; 16 byte .NES header (iNES format)
	.db "NES", $1a
	.db $02                 ; size of PRG ROM in 16kb units
	.db $01			; size of CHR ROM in 8kb units
	.db #%00000000		; mapper 0
	.db #%00000000		; mapper 0
        .db $00                 ; size of PRG RAM in 8kb RAM
        .db $00
        .db $00
        .db $00
        .db $00
        .db $00
        .db $00
        .db $00

        .org $8000

;       ----------------------------------------------------

Reset:                          ; reset routine
        SEI
        CLD
	LDX #$00
	STX $2000
	STX $2001
	DEX
	TXS
  	LDX #0
  	TXA
ClearMemory:
	STA 0, X
	STA $100, X
	STA $200, X
	STA $300, X
	STA $400, X
	STA $500, X
	STA $600, X
	STA $700, X
        STA $800, X
        STA $900, X
        INX
	BNE ClearMemory

;       ----------------------------------------------------

        lda #$00                ; setting up variables
        sta scroll_h
        sta scroll_v
        lda #$20
        sta PPU_ADDR+0
        lda #$00
        sta PPU_ADDR+1

;       ----------------------------------------------------

	LDX #$02                ; warm up
WarmUp:
	bit $2002
	bpl WarmUp
	dex
	BNE WarmUp

       	LDA #$3F
	STA $2006
	LDA #$00
	STA $2006
load_pal:                       ; load palette
        LDA palette,x
        sta $2007
        inx
        cpx #$20
        bne load_pal

	LDA #$20
	STA $2006
	LDA #$00
	STA $2006

	ldy #$04                ; clear nametables
ClearName:
	LDX #$00
	LDA #$00
PPULoop:
	STA $2007
	DEX
	BNE PPULoop

	DEY
	BNE ClearName

;       ----------------------------------------------------

        lda #$20                ; write the welcome message
        sta $2006
        lda #$82
        sta $2006
        ldx #$00
WriteWelcome:
        lda WelcomeText,x
        cmp #$0d
        beq Vblank
        sta $2007
        INX
        JMP WriteWelcome

;       ----------------------------------------------------

Vblank:                         ; turn on the screen and start the party
	bit $2002
	bpl Vblank

	LDA #%10001000
	STA $2000
        LDA #%00001110
	STA $2001

        ldx scroll_h
        stx $2005
        ldx scroll_v
        stx $2005

;       ----------------------------------------------------

InfLoop:                        ; loop forever
        jsr controller_test
        JMP InfLoop

;       ----------------------------------------------------

controller_test:

        ldx #$00
	LDA #$01		; strobe joypad
	STA $4016
	LDA #$00
	STA $4016
con_loop:
	LDA $4016		; check the state of each button
	LSR
	ROR buttons
        INX
        CPX #$08
        bne con_loop

CheckUp:
	LDA #%00010000
	AND buttons
	BEQ CheckDown

        dec scroll_h            ; decrease horz scroll
        lda scroll_h
        sta $2005
        inc scroll_v            ; increase vert scroll
        inc scroll_v
        lda scroll_v
        sta $2005

CheckDown:
	LDA #%00100000
	AND buttons
	BEQ CheckLeft

        inc scroll_h
        lda scroll_h
        sta $2005
        dec scroll_v
        lda scroll_v
        sta $2005

CheckLeft:
	LDA #%01000000
	AND buttons
	BEQ CheckRight

        inc scroll_h
        lda scroll_h
        sta $2005
        lda scroll_v
        sta $2005

CheckRight:
	LDA #%10000000
	AND buttons
	BEQ CheckSel

        inc scroll_h
        inc scroll_h
        lda scroll_h
        sta $2005
        inc scroll_v
        lda scroll_v
        sta $2005

CheckSel:
	LDA #%00000100
	AND buttons
	BEQ CheckStart

        inc tilenum             ; increase tile number
        lda tilenum
        sta $2007               ; write to screen

CheckStart
	LDA #%00001000
	AND buttons
	BEQ CheckB

        jsr writer              ; jump to character writing routine

CheckB:
	LDA #%00000010
	AND buttons
	BEQ CheckA

        lda #$01                ; write tile #1 to screen
        sta $2007

       	LDA #%10001000          ; switches to the first pattern table
	STA $2000

CheckA:
	LDA #%00000001
	AND buttons
	BEQ CheckOver

        lda #$00                ; write tile #0 to screen (blank by default)
        sta $2007

       	LDA #%10011000          ; switches to the second pattern table
	STA $2000

CheckOver:

        RTS

;       ----------------------------------------------------

writer:                         ; this routine increases tile number and screen location
        ldy #$ff                ; then draws to the screen
write_tile:
        LDA PPU_ADDR+0
    	STA $2006
    	LDA PPU_ADDR+1
    	STA $2006

        inc tilenum
        lda tilenum
        sta $2007

        inc PPU_ADDR+1
        lda PPU_ADDR+1
        bne end_write
        inc PPU_ADDR+0

end_write:
        dey
        bne write_tile
        RTS

;       ----------------------------------------------------

NMI:
        RTI
IRQ:
        RTI

;       ----------------------------------------------------

palette:
	;BG
	.byte $0F,$10,$20,$30,$0F,$10,$20,$30,$0F,$10,$20,$30,$0F,$10,$20,$30
	;SPR
	.byte $0F,$10,$20,$30,$0F,$10,$20,$30,$0F,$10,$20,$30,$0F,$10,$20,$30

WelcomeText:
        .db "GLITCHNES 0.1 BY NO CARRIER",$0D

	.ORG $fffa
	.dw NMI
	.dw Reset
	.dw IRQ

;       ----------------------------------------------------