	.TEXT
@	AREA M3SD, CODE, READONLY
@	ENTRY
@-----------------------------------
@	EXPORT SD_crc16
@	EXPORT SD_data_write

@CSYNC	EQU	0x9800000
@SDDIR	EQU 0x8800000
@SDCON	EQU	0x9800000
@SDODA	EQU 0x9000000
.equ CSYNC,0x9800000
.equ SDDIR,0x8800000
.equ SDCON,0x9800000
.equ SDODA,0x9000000

	.ALIGN
	.CODE 32

clkout:
	stmfd   r13!,{r0-r1}
	ldr     r1,=SDDIR
	mov     r0,#0x4
	strh    r0,[r1]
	mov     r0,r0
	mov     r0,r0
	mov     r0,#0xc
	strh    r0,[r1]
	ldmfd	r13!,{r0-r1}
	bx      r14

clkin:
	stmfd   r13!,{r0-r1}
	ldr     r1,=SDDIR
	mov     r0,#0x0
	strh    r0,[r1]
	mov     r0,r0
	mov     r0,r0
	mov     r0,#0x8
	strh    r0,[r1]
	ldmfd	r13!,{r0-r1}
	bx      r14

wait_ready:
	stmfd   r13!,{r0-r2}
	mov     r2,#32
	mov	    r1,#SDODA
sd_write_loop2:
	mov	    r0,#0xff @end bit
	strh    r0,[r1]
	bl      clkout
	subs    r2, r2, #1
	bne     sd_write_loop2

sd_write_busy:
	bl      clkin
	ldrh    r0,[r1]
	tst	    r0,#0x100
	beq	    sd_write_busy
	ldmfd	r13!,{r0-r1}
	bx      r14

@------void SD_crc16(u16* buff,u16 num,u16* crc16buff)

    .GLOBAL SD_crc16

SD_crc16:
	stmfd   r13!,{r4-r9}
	mov	    r9,r2

	mov	    r3,#0
	mov	    r4,#0
	mov	    r5,#0
	mov	    r6,#0

	ldr	    r7,=0x80808080
	ldr	    r8,=0x1021
	mov	    r1,r1,lsl #3
sd_crc16_loop:

	tst	    r7,#0x80
	ldrneb	r2,[r0],#1

	mov	    r3,r3,lsl #1
	tst	    r3,#0x10000
	eorne	r3,r3,r8
	tst	    r2,r7,lsr #24
	eorne	r3,r3,r8

	mov	    r4,r4,lsl #1
	tst	    r4,#0x10000
	eorne	r4,r4,r8
	tst	    r2,r7,lsr #25
	eorne	r4,r4,r8

	mov	    r5,r5,lsl #1
	tst	    r5,#0x10000
	eorne	r5,r5,r8
	tst	    r2,r7,lsr #26
	eorne	r5,r5,r8

	mov	    r6,r6,lsl #1
	tst	    r6,#0x10000
	eorne	r6,r6,r8
	tst	    r2,r7,lsr #27
	eorne	r6,r6,r8

	mov	    r7,r7,ror #4
	subs	r1,r1,#4
	bne     sd_crc16_loop

	mov	    r2,r9
	mov	    r8,#16
sd_crc16_write_data:
	mov	    r7,r7,lsl #4
	tst	    r3,#0x8000
	orrne   r7,r7,#8
	tst	    r4,#0x8000
	orrne   r7,r7,#4
	tst	    r5,#0x8000
	orrne   r7,r7,#2
	tst	    r6,#0x8000
	orrne   r7,r7,#1

	mov	    r3,r3,lsl #1
	mov	    r4,r4,lsl #1
	mov	    r5,r5,lsl #1
	mov	    r6,r6,lsl #1

	sub	    r8,r8,#1
	tst	    r8,#1
	streqb  r7,[r2],#1
	cmp	    r8,#0
	bne	    sd_crc16_write_data

	ldmfd   r13!,{r4-r9}
	bx      r14
@------end-----------------------------------

@-----------------viod SD_data_write(u16 *buff,u16* crc16buff)-------------------
    .GLOBAL SD_data_write
SD_data_write:
	stmfd   r13!,{r4-r5,r14}
	mov     r5,#512
	mov	    r2,#SDODA
sd_data_write_busy:
	bl      clkin
	ldrh    r3,[r2]
	tst	    r3,#0x100
	beq	    sd_data_write_busy

	mov	    r3,#0 @star bit
	strh    r3,[r2]
	bl      clkout

sd_data_write_loop:
	ldrh    r4,[r0],#2
	mov     r3,r4,lsr#4
	strh    r3,[r2]
	bl      clkout
	mov     r3,r4
	strh    r3,[r2]
	bl      clkout
	mov     r3,r4,lsr#12
	strh    r3,[r2]
	bl      clkout
	mov     r3,r4,lsr#8
	strh    r3,[r2]
	bl      clkout

	subs    r5, r5, #2
	bne     sd_data_write_loop

	cmp	    r1,#0
	movne   r0,r1
	movne   r1,#0
	movne   r5,#8
	bne	    sd_data_write_loop

	mov     r5,#32
sd_data_write_loop2:
	mov	    r3,#0xff @end bit
	strh    r3,[r2]
	bl      clkout
	subs    r5, r5, #1
	bne     sd_data_write_loop2

sd_data_write_busy2:
	bl      clkin
	ldrh    r3,[r2]
	tst	    r3,#0x100
	beq	    sd_data_write_busy2

	ldmfd   r13!,{r4-r5,r15}
@-----------------end-------------------
