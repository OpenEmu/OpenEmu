    .TEXT
@--------------------------------sd data--------------------------------
.equ sd_comadd,0x9800000
.equ sd_dataadd,0x9000000
.equ sd_dataradd,0x9100000
@-----------------viod sd_data_write_s(u16 *buff,u16* crc16buff)-------------------
    .ALIGN
    .GLOBAL sd_data_write_s
    .CODE 32
sd_data_write_s:
	stmfd   r13!,{r4-r5}
	mov	r5,#512
	mov	r2,#sd_dataadd
sd_data_write_busy:
	ldrh	r3,[r2]
	tst	r3,#0x100
	beq	sd_data_write_busy

	ldrh	r3,[r2]

	mov	r3,#0 @star bit
	strh	r3,[r2]
sd_data_write_loop:
	ldrh	r3,[r0],#2
	add	r3,r3,r3,lsl #20
	mov	r4,r3,lsl #8
	stmia   r2,{r3-r4}

        subs    r5, r5, #2
        bne     sd_data_write_loop

	cmp	r1,#0
	movne	r0,r1
	movne	r1,#0
	movne	r5,#8
	bne	sd_data_write_loop

	mov	r3,#0xff @end bit
	strh	r3,[r2]
sd_data_write_loop2:
	ldrh	r3,[r2]
	tst	r3,#0x100
	bne	sd_data_write_loop2

	ldmia   r2,{r3-r4}

	ldmfd	r13!,{r4-r5}
	bx      r14
@-----------------end sd_data_write_s-------------------

@----------void sd_data_read_s(u16 *buff)-------------
    .ALIGN
    .GLOBAL sd_data_read_s
    .CODE 32
sd_data_read_s:
	stmfd   r13!,{r4}
	mov	r1,#sd_dataradd
sd_data_read_loop1:
	ldrh	r3,[r1]   @star bit
	tst	r3,#0x100
	bne	sd_data_read_loop1
	mov	r2,#512
sd_data_read_loop:
	ldmia	r1,{r3-r4}
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

	ldmia	r1,{r3-r4}
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

	ldmia	r1,{r3-r4}
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

	ldmia	r1,{r3-r4}
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

	ldmia	r1,{r3-r4}
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

	ldmia	r1,{r3-r4}
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

	ldmia	r1,{r3-r4}
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

	ldmia	r1,{r3-r4}
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

        subs    r2, r2, #16
        bne     sd_data_read_loop

	ldmia	r1,{r3-r4} @crc 16
	ldmia	r1,{r3-r4}
	ldmia	r1,{r3-r4}
	ldmia	r1,{r3-r4}
	ldrh	r3,[r1]    @end bit
	ldmfd	r13!,{r4}
	bx      r14
@----------end sd_data_read_s-------------

@------void sd_com_crc16_s(u16* buff,u16 num,u16* crc16buff)
    .ALIGN
    .GLOBAL	sd_crc16_s
    .CODE 32
sd_crc16_s:
	stmfd   r13!,{r4-r9}
	mov	r9,r2

	mov	r3,#0
	mov	r4,#0
	mov	r5,#0
	mov	r6,#0

	ldr	r7,=0x80808080
	ldr	r8,=0x1021
	mov	r1,r1,lsl #3
sd_crc16_loop:

	tst	r7,#0x80
	ldrneb	r2,[r0],#1

	mov	r3,r3,lsl #1
	tst	r3,#0x10000
	eorne	r3,r3,r8
	tst	r2,r7,lsr #24
	eorne	r3,r3,r8

	mov	r4,r4,lsl #1
	tst	r4,#0x10000
	eorne	r4,r4,r8
	tst	r2,r7,lsr #25
	eorne	r4,r4,r8

	mov	r5,r5,lsl #1
	tst	r5,#0x10000
	eorne	r5,r5,r8
	tst	r2,r7,lsr #26
	eorne	r5,r5,r8

	mov	r6,r6,lsl #1
	tst	r6,#0x10000
	eorne	r6,r6,r8
	tst	r2,r7,lsr #27
	eorne	r6,r6,r8

	mov	r7,r7,ror #4
	subs	r1,r1,#4
        bne     sd_crc16_loop

	mov	r2,r9
	mov	r8,#16
sd_crc16_write_data:
	mov	r7,r7,lsl #4
	tst	r3,#0x8000
	orrne	r7,r7,#8
	tst	r4,#0x8000
	orrne	r7,r7,#4
	tst	r5,#0x8000
	orrne	r7,r7,#2
	tst	r6,#0x8000
	orrne	r7,r7,#1

	mov	r3,r3,lsl #1
	mov	r4,r4,lsl #1
	mov	r5,r5,lsl #1
	mov	r6,r6,lsl #1

	sub	r8,r8,#1
	tst	r8,#1
	streqb	r7,[r2],#1
	cmp	r8,#0
	bne	sd_crc16_write_data

	ldmfd	r13!,{r4-r9}
	bx      r14
@------end sd_com_crc16_s-----------------------------------

@--------------u8 sd_crc7_s(u16* buff,u16 num)----------------------------
    .ALIGN
    .GLOBAL	sd_crc7_s
    .CODE 32
sd_crc7_s:
	stmfd   r13!,{r4}

	mov	r3,#0
	ldr	r4,=0x80808080
	mov	r1,r1,lsl #3 @ *8
sd_crc7_loop:
	tst	r4,#0x80
	ldrneb	r2,[r0],#1

	mov	r3,r3,lsl #1

	tst	r3,#0x80
	eorne	r3,r3,#0x9

	tst	r2,r4,lsr #24
	eorne	r3,r3,#0x9

	mov	r4,r4,ror #1
	subs	r1,r1,#1
        bne     sd_crc7_loop

	mov	r3,r3,lsl #1
	add	r0,r3,#1
	ldmfd	r13!,{r4}
	bx      r14
@--------------end sd_crc7_s----------------------------

@--------------sd_com_read_s(u16* buff,u32 num)------------------
		.ALIGN
		.GLOBAL	 sd_com_read_s
		.CODE 32

sd_com_read_s:
	stmfd   r13!,{r4-r6}
	mov	r2,#sd_comadd
sd_com_read_loop1:
	ldrh	r3,[r2] @star bit
	tst	r3,#1
	bne	sd_com_read_loop1

sd_com_read_loop:
	ldmia   r2,{r3-r6}
        subs    r1, r1, #1
        bne     sd_com_read_loop
	ldmfd	r13!,{r4-r6}
	bx      r14
@--------------end sd_com_read_s------------------

@-------------------void sd_com_write_s(u16* buff,u32 num)-----------------

		.ALIGN
		.GLOBAL	 sd_com_write_s
		.CODE 32
sd_com_write_s:
	stmfd   r13!,{r4-r6}

	mov	r2,#sd_comadd
sd_com_write_busy:
	ldrh	r3,[r2]
	tst	r3,#0x1
	beq	sd_com_write_busy

	ldrh	r3,[r2]

sd_com_write_loop:
        ldrb   r3,[r0],#1
	add	r3,r3,r3,lsl #17
	mov	r4,r3,lsl #2
	mov	r5,r4,lsl #2
	mov	r6,r5,lsl #2
        stmia   r2,{r3-r6}
        subs    r1, r1, #1
        bne     sd_com_write_loop
	ldmfd   r13!,{r4-r6}

	bx      r14
@-------------------end sd_com_write_s-----------------

@-----------------void send_clk(u32 num)---------
		.ALIGN
		.GLOBAL	 send_clk
		.CODE 32

send_clk:
	mov	r1,#sd_comadd
send_clk_loop1:
	ldrh	r3,[r1]
	subs	r0,r0,#1
	bne	send_clk_loop1
	bx	r14
@-----------------end send_clk---------

@---------------------------void SDCommand(u8 command,u8 num,u32 sector)--------------------
		.ALIGN
		.GLOBAL	 SDCommand
		.CODE 32
@void SDCommand(u8 command,u8 num,u32 sector )
@{
@	u8 databuff[6];
@	register u8 *char1;
@	register u8 *char2;
@
@	char1=(u8*)(((u32)&sector)+3);
@	char2=(u8*)databuff;
@	*char2++=coma+0x40;
@	*char2++=*char1--;
@	*char2++=*char1--;
@	*char2++=*char1--;
@	*char2++=*char1;
@	*char2=sd_crc7_s((u32)databuff,5);
@
@	sd_com_write_s((u32)databuff,6);
@
@}
SDCommand:
	stmfd   r13!,{r14}

	sub	r13,r13,#16
	add	r0,r0,#0x40
	strb	r0,[r13,#4]

	mov	r1,r2,lsr #24
	strb	r1,[r13,#5]
	mov	r1,r2,lsr #16
	strb	r1,[r13,#6]
	mov	r1,r2,lsr #8
	strb	r1,[r13,#7]
	strb	r2,[r13,#8]
	add	r0,r13,#4
	mov	r1,#5
	bl	sd_crc7_s
	strb	r0,[r13,#9]
	add	r0,r13,#4
	mov	r1,#6
	bl	sd_com_write_s

	add	r13,r13,#16
	ldmfd   r13!,{r15}
@	bx	r14
@---------------------------end SDCommand--------------------

@----------void ReadSector(u16 *buff,u32 sector,u8 readnum)-------------
		.ALIGN
		.GLOBAL	 ReadSector @ r0:Srcp r1:num ok
		.CODE 32

@void ReadSector(u16 *buff,u32 sector,u8 readnum)
@{
@	register u16 i,j;
@	i=readnum;
@	sectno<<=9;
@	SDCommand(18,0,sector);
@	for (j=0;j<i ; j++)
@	{
@		sd_data_read_s((u32)buff+j*512);
@	}
@	SDCommand(12,0,0);
@	get_resp();
@	send_clk(0x10);
@
@}
ReadSector:
	stmfd   r13!,{r4-r6,r14}

	mov	r4,r0
	mov	r5,r2

	mov	r2,r1,lsl #9
	mov	r0,#18
	mov	r1,#0
	bl	SDCommand
	mov	r6,#0
beginforj_ReadSector:
	cmp	r6,r5
	bge	endforj_ReadSector
	mov	r0,r4
	add	r0,r0,r6,lsl #9
	bl	sd_data_read_s
	add	r6,r6,#1
	b	beginforj_ReadSector
endforj_ReadSector:
	mov	r0,#12
	mov	r1,#0
	mov	r2,#0
	bl	SDCommand
	bl	get_resp
	mov	r0,#0x10
	bl	send_clk
	mov	r0,#1
	ldmfd   r13!,{r4-r6,r15}
@	bx	r14
@----------end ReadSector------------

@-----------void get_resp(void)-------------------


		.ALIGN
		.GLOBAL	 get_resp @ r0:Srcp r1:num ok
		.CODE 32
get_resp:

	stmfd   r13!,{r14}
	mov	r1,#6
	bl	sd_com_read_s
	ldmfd   r13!,{r15}
@-----------end get_resp-------------------


@---------------void WriteSector(u16 *buff,u32 sector,u8 writenum)---------------------
		.ALIGN
		.GLOBAL	 WriteSector @ r0:Srcp r1:num ok
		.CODE 32

@void WriteSector(u16 *buff,u32 sector,u8 writenum)
@{
@	register u16 i,j;
@	u16 crc16[5];
@	i=writenum;
@
@	sectno<<=9;
@
@	SDCommand(25,0,sector);
@	get_resp();
@	send_clk(0x10);
@
@	for (j=0;j<i ; j++)
@	{
@		sd_crc16_s((u32)(u32)buff+j*512,512,(u32)crc16);
@		sd_data_write_s((u32)buff+j*512,(u32)crc16);
@		send_clk(0x10);
@	}
@	SDCommand(12,0,0);
@	get_resp();
@	send_clk(0x10);
@	while((*(u16*)sd_dataadd &0x0100)==0);
@
@}
@
WriteSector:
	stmfd   r13!,{r4-r6,r14}

	sub	r13,r13,#20

	mov	r4,r0
	mov	r5,r2

	mov	r2,r1,lsl #9
	mov	r0,#25
	mov	r1,#0
	bl	SDCommand
	bl	get_resp
	mov	r0,#0x10
	bl	send_clk
	mov	r6,#0

beginforj_WriteSector:
	cmp	r6,r5
	bge	endforj_WriteSector
	mov	r0,r4
	add	r0,r0,r6,lsl #9
	mov	r1,#512
	add	r2,r13,#4
	bl	sd_crc16_s
	mov	r0,r4
	add	r0,r0,r6,lsl #9
	add	r1,r13,#4
	bl	sd_data_write_s
	mov	r0,#0x10
	bl	send_clk
	add	r6,r6,#1
	b	beginforj_WriteSector
endforj_WriteSector:
	mov	r0,#12
	mov	r1,#0
	mov	r2,#0
	bl	SDCommand
	bl	get_resp
	mov	r0,#0x10
	bl	send_clk
	ldr	r0,=sd_dataadd
beginwhile_WriteSector:
	ldrh	r1,[r0]
	tst	r1,#0x0100
	beq	beginwhile_WriteSector
	mov	r0,#1
	add	r13,r13,#20
	ldmfd   r13!,{r4-r6,r15}
@---------------end WriteSector--------------------

@----------------void InitSCMode(void)---------------
		.ALIGN
		.GLOBAL	 InitSCMode
		.CODE 32
InitSCMode:
	mvn     r0,#0x0F6000000
	sub     r0,r0,#0x01
	mov     r1,#0x0A500
	add     r1,r1,#0x5A
	strh    r1,[r0]
	strh    r1,[r0]
	mov	r2,#3
	strh    r2,[r0]
	strh    r2,[r0]
	bx	r14
@----------------end InitSCMode ---------------

@----------------bool MemoryCard_IsInserted(void)---------------
		.ALIGN
		.GLOBAL	 MemoryCard_IsInserted
		.CODE 32

MemoryCard_IsInserted:
	ldr	r0,=sd_comadd
	ldrh	r1,[r0]
	tst	r1,#0x300
	moveq	r0,#1
	movne	r0,#0
	bx	r14
@----------------end MemoryCard_IsInserted---------------

    .END
