;       ----------------------------------------------------

;    galleryNES - version 0.1
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

NewButtons = $41
OldButtons = $42
JustPressed = $43
ScreenNumber = $44
OldScreen = $45
PaletteNumber = $46

;       ----------------------------------------------------

        .ORG $7ff0
Header:                         ; 16 byte .NES header (iNES format)
	.DB "NES", $1a
	.DB $02                 ; size of PRG ROM in 16kb units
	.DB $01			; size of CHR ROM in 8kb units
	.DB #%00000000		; mapper 0
	.DB #%00000000		; mapper 0
        .DB $00
        .DB $00
        .DB $00
        .DB $00
        .DB $00
        .DB $00
        .DB $00
        .DB $00

        .ORG $8000

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

        LDA #$00                ; setting up variables
        STA ScreenNumber
        STA PaletteNumber

;       ----------------------------------------------------

	LDX #$02                ; warm up
WarmUp:
	BIT $2002
	BPL WarmUp
	DEX
	BNE WarmUp

       	LDA #$3F
	STA $2006
	LDA #$00
	STA $2006
        TAX
LoadPal:                        ; load palette
        LDA palette, x
        STA $2007
        INX
        CPX #$10
        BNE LoadPal

	LDA #$20
	STA $2006
	LDA #$00
	STA $2006

	LDY #$04                ; clear nametables
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

        LDA #<pic0              ; load low byte of first picture
        STA $10

        LDA #>pic0              ; load high byte of first picture
        STA $11

;       ----------------------------------------------------

        JSR DrawScreen          ; draw initial nametable
        JSR Vblank              ; turn on screen

;       ----------------------------------------------------

InfLoop:                        ; loop forever, program now controlled by NMI routine

        JMP InfLoop

;       ----------------------------------------------------

LoadNewPalette:
       	LDX PaletteNumber       ; load palette lookup value
        LDY #$00
        LDA #$3F
	STA $2006
	LDA #$00
	STA $2006
LoadNewPal:                     ; load palette
        LDA palette, x
        STA $2007
        INX
        INY
        CPY #$10
        BNE LoadNewPal
        RTS

;       ----------------------------------------------------

DrawScreen:

   	LDA #$20                ; set to beginning of first nametable
    	STA $2006
    	LDA #$00
    	STA $2006

        LDY #$00
        LDX #$04

NameLoop:                       ; loop to draw entire nametable
        LDA ($10),y
        STA $2007
        INY
        BNE NameLoop
        INC $11
        DEX
        BNE NameLoop

        RTS

;       ----------------------------------------------------

Vblank:                         ; turn on the screen and start the party
	BIT $2002
	BPL Vblank

        LDX #$00
        STX $2005
        STX $2005

	LDA #%10001000
	STA $2000
        LDA #%00001110
	STA $2001

        RTS

;       ----------------------------------------------------

LoadScreen:

    	LDA #%00000000          ; disable NMI's and screen display
 	STA $2000
   	LDA #%00000000
   	STA $2001

        LDA ScreenNumber

Test0:
        CMP #0                  ; compare ScreenNumber to find out which picture / palette to load
        BNE Test1
        LDA #<pic0              ; load low byte of picture
        STA $10
        LDA #>pic0              ; load high byte of picture
        STA $11
        LDA #$00
        STA PaletteNumber       ; set palette lookup location
        RTS

Test1:
        CMP #1
        BNE Test2
        LDA #<pic1
        STA $10
        LDA #>pic1
        STA $11
        LDA #$10
        STA PaletteNumber
        RTS

Test2:
        CMP #2
        BNE Test3
        LDA #<pic2
        STA $10
        LDA #>pic2
        STA $11
        LDA #$20
        STA PaletteNumber
        RTS

Test3:
        CMP #3
        BNE Test4
        LDA #<pic3
        STA $10
        LDA #>pic3
        STA $11
        LDA #$30
        STA PaletteNumber
        RTS

Test4:
        CMP #4
        BNE Test5
        LDA #<pic4
        STA $10
        LDA #>pic4
        STA $11
        LDA #$40
        STA PaletteNumber
        RTS

Test5:
        CMP #5
        BNE Test6
        LDA #<pic5
        STA $10
        LDA #>pic5
        STA $11
        LDA #$50
        STA PaletteNumber
        RTS

Test6:
        CMP #6
        BNE Test7
        LDA #<pic6
        STA $10
        LDA #>pic6
        STA $11
        LDA #$60
        STA PaletteNumber
        RTS

Test7:
        CMP #7
        BNE Test8
        LDA #<pic7
        STA $10
        LDA #>pic7
        STA $11
        LDA #$70
        STA PaletteNumber
        RTS

Test8:
        CMP #8
        BNE Test9
        LDA #<pic8
        STA $10
        LDA #>pic8
        STA $11
        LDA #$80
        STA PaletteNumber
        RTS

Test9:
        LDA #<pic9
        STA $10
        LDA #>pic9
        STA $11
        LDA #$90
        STA PaletteNumber
        RTS

;       ----------------------------------------------------

ControllerTest:

        LDA NewButtons
	STA OldButtons

        LDX #$00
	LDA #$01		; strobe joypad
	STA $4016
	LDA #$00
	STA $4016
ConLoop:
	LDA $4016		; check the state of each button
	LSR
	ROR NewButtons
        INX
        CPX #$08
        bne ConLoop

	LDA OldButtons          ; invert bits
	EOR #$FF
	AND NewButtons
	STA JustPressed

	LDA ScreenNumber        ; save old screen number for later compare
	STA OldScreen

CheckLeft:
	LDA #%01000000
	AND JustPressed
	BEQ CheckRight

	DEC ScreenNumber        ; decrement screen number here
        BPL CheckRight
	LDA #9	                ; equal to total # of screens, starting from 0
	STA ScreenNumber

CheckRight:
	LDA #%10000000
	AND JustPressed
	BEQ EndDrawChk

	INC ScreenNumber        ; increment screen number here
        LDA ScreenNumber
	CMP #10	                ; equal to total # of screens +1, starting from 0
	BNE EndDrawChk
	LDA #0
	STA ScreenNumber

EndDrawChk:
	LDA ScreenNumber        ; has screen number changed? if not, skip redraw
	CMP OldScreen
	BEQ CheckOver

        JSR LoadScreen          ; turn off and load new screen data
        JSR LoadNewPalette      ; load new palette
        JSR DrawScreen          ; draw new screen
        JSR Vblank              ; turn the screen back on

CheckOver:

        RTS

;       ----------------------------------------------------

NMI:
        JSR ControllerTest      ; check for user input

        RTI
IRQ:
        RTI

;       ----------------------------------------------------

palette:                        ; palette data
        .byte $0F,$00,$10,$30,$0F,$05,$26,$30,$0F,$13,$23,$33,$0F,$1C,$2B,$39 ; palette 0 - aligns with pic0.nam below
        .byte $0F,$05,$15,$33,$0F,$26,$37,$30,$0F,$0B,$2B,$39,$0F,$02,$21,$3C ; palette 1 - alings with pic1.nam below
        .byte $0F,$00,$10,$30,$0F,$05,$26,$30,$0F,$13,$23,$33,$0F,$1C,$2B,$39 ; palette 2 - aligns with you know what below...
        .byte $0F,$00,$10,$30,$0F,$05,$26,$30,$0F,$13,$23,$33,$0F,$1C,$2B,$39 ; palette 3
        .byte $0F,$00,$10,$30,$0F,$05,$26,$30,$0F,$13,$23,$33,$0F,$1C,$2B,$39 ; palette 4
        .byte $0F,$00,$10,$30,$0F,$05,$26,$30,$0F,$13,$23,$33,$0F,$1C,$2B,$39 ; palette 5
        .byte $0F,$00,$10,$30,$0F,$05,$26,$30,$0F,$13,$23,$33,$0F,$1C,$2B,$39 ; palette 6
        .byte $0F,$00,$10,$30,$0F,$05,$26,$30,$0F,$13,$23,$33,$0F,$1C,$2B,$39 ; palette 7
        .byte $0F,$00,$10,$30,$0F,$05,$26,$30,$0F,$13,$23,$33,$0F,$1C,$2B,$39 ; palette 8
        .byte $0F,$00,$10,$30,$0F,$05,$26,$30,$0F,$13,$23,$33,$0F,$1C,$2B,$39 ; palette 9

;       ----------------------------------------------------

                                ; include picture data

pic0:
        .INCBIN "pic0.nam"      ; Boring galleryNES 0.1 splash screen by NO CARRIER, 2009 - http://www.no-carrier.com
pic1:
        .INCBIN "pic1.nam"      ; Rhino (from 'Color Caves' album) by Alex Mauer - http://www.headlessbarbie.com
pic2:
        .INCBIN "pic2.nam"
pic3:
        .INCBIN "pic3.nam"
pic4:
        .INCBIN "pic4.nam"
pic5:
        .INCBIN "pic5.nam"
pic6:
        .INCBIN "pic6.nam"
pic7:
        .INCBIN "pic7.nam"
pic8:
        .INCBIN "pic8.nam"
pic9:
        .INCBIN "pic9.nam"

;       ----------------------------------------------------

	.ORG $fffa              ; vectors
	.DW NMI
	.DW Reset
	.DW IRQ