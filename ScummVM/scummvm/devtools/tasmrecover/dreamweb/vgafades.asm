;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text
Fadedos	proc	near

	call	vsync
	mov	es,buffers
	mov	di,startpal
	mov	al,0
	mov	dx,3c7h
	out	dx,al
	mov	dx,3c9h
	mov	cx,768/4
dos1:	in	al,dx
	stosb
	loop	dos1

	mov	cx,64
fadedosloop:	push	cx

	mov	ds,buffers
	mov	si,startpal
	mov	cx,768
dos3:	lodsb
	cmp	al,0
	jz	nodown
	dec	al
nodown:	mov	[si-1],al
	loop	dos3

	call	vsync
	mov	ds,buffers
	mov	si,startpal
	mov	al,0
	mov	dx,3c8h
	out	dx,al
	inc	dx
	mov	cx,768/4
dos2:	lodsb
	out	dx,al
	loop	dos2

	pop	cx
	loop	fadedosloop
	ret

	endp











Dofade	proc	near

	cmp	fadedirection,0
	jz	finishfade
         	mov	cl,numtofade
	mov	ch,0
	mov	al,colourpos
	mov	ah,0
	mov	ds,buffers
	mov	si,startpal
	add	si,ax
	add	si,ax
	add	si,ax
	call	showgroup
	mov	al,numtofade
	add	al,colourpos
	mov	colourpos,al
	cmp	al,0
	jnz	finishfade
	call	fadecalculation
finishfade:	ret

	endp







Clearendpal	proc	near

	mov	es,buffers
	mov	di,endpal
	mov	cx,768
	mov	al,0
	rep	stosb
	ret

	endp




Clearpalette	proc	near
	
	mov	fadedirection,0
	call	clearstartpal
	call	dumpcurrent
	ret
         	
	endp










Fadescreenup	proc	near

	call	clearstartpal
	call	paltoendpal
	mov	fadedirection,1
	mov	fadecount,63
	mov	colourpos,0
	mov	numtofade,128
	ret

	endp




Fadetowhite	proc	near

	mov	es,buffers
	mov	di,endpal
	mov	cx,768
	mov	al,63
	rep	stosb
	mov	di,endpal
	mov	al,0
	stosb
	stosb
	stosb
	call	paltostartpal
	mov	fadedirection,1
	mov	fadecount,63
	mov	colourpos,0
	mov	numtofade,128
	ret

	endp



Fadefromwhite	proc	near

	mov	es,buffers
	mov	di,startpal
	mov	cx,768
	mov	al,63
	rep	stosb
	mov	di,startpal
	mov	al,0
	stosb
	stosb
	stosb
	call	paltoendpal
	mov	fadedirection,1
	mov	fadecount,63
	mov	colourpos,0
	mov	numtofade,128
	ret

	endp









Fadescreenups	proc	near

	call	clearstartpal
	call	paltoendpal
	mov	fadedirection,1
	mov	fadecount,63
	mov	colourpos,0
	mov	numtofade,64
	ret

	endp



Fadescreendownhalf	proc	near

	call	paltostartpal
	call	paltoendpal
	mov	cx,768
	mov	es,buffers
	mov	bx,endpal
halfend:	mov	al,[es:bx]
	shr	al,1
	mov	[es:bx],al
	inc	bx
	loop	halfend

	mov	ds,buffers
	mov	es,buffers
	mov	si,startpal+(56*3)
	mov	di,endpal+(56*3)
	mov	cx,3*5
	rep	movsb
	mov	si,startpal+(77*3)
	mov	di,endpal+(77*3)
	mov	cx,3*2
	rep	movsb

	mov	fadedirection,1
	mov	fadecount,31
	mov	colourpos,0
	mov	numtofade,32
	ret

	endp


Fadescreenuphalf	proc	near

	call	endpaltostart
	call	paltoendpal
	mov	fadedirection,1
	mov	fadecount,31
	mov	colourpos,0
	mov	numtofade,32
	ret

	endp










Fadescreendown	proc	near

	call	paltostartpal
	call	clearendpal
	mov	fadedirection,1
	mov	fadecount,63
	mov	colourpos,0
	mov	numtofade,128
	ret

	endp



Fadescreendowns	proc	near

	call	paltostartpal
	call	clearendpal
	mov	fadedirection,1
	mov	fadecount,63
	mov	colourpos,0
	mov	numtofade,64
	ret

	endp








Clearstartpal	proc	near

	mov	es,buffers
	mov	di,startpal
	mov	cx,256
wholeloop1:	mov	ax,0
	stosw
	mov	al,0
	stosb
	loop	wholeloop1
	ret

	endp






Showgun	proc	near

	mov	addtored,0 ;12
	mov	addtogreen,0
	mov	addtoblue,0	
	call	paltostartpal
	call	paltoendpal
	call	greyscalesum
	
;	mov	es,buffers
;	mov	di,endpal+3
;	mov	cx,255
;	mov	ax,0
;reds:	mov	byte ptr [es:di],63
;	inc	di
;	stosw
;	loop	reds
     	
	mov	fadedirection,1
	mov	fadecount,63
	mov	colourpos,0
	mov	numtofade,128
	mov	cx,130
	call	hangon
	call	endpaltostart
	call	clearendpal
	mov	fadedirection,1
	mov	fadecount,63
	mov	colourpos,0
	mov	numtofade,128
	mov	cx,200
	call	hangon
	mov	roomssample,34
	call	loadroomssample
	mov	volume,0
	mov	dx,offset cs:gungraphic
	call	loadintotemp
	call	createpanel2
	mov	ds,tempgraphics
	mov	al,0
	mov	ah,0
	mov	di,100
	mov	bx,4
	call	showframe
	mov	ds,tempgraphics
	mov	al,1
	mov	ah,0
	mov	di,158
	mov	bx,106
	call	showframe
	call	worktoscreen
	call	getridoftemp
	call	fadescreenup
	mov	cx,160
	call	hangon
	mov	al,12
	mov	ah,0
	call	playchannel0
	mov	dx,offset cs:endtextname
	call	loadtemptext
	call	rollendcredits2
	call	getridoftemptext
	ret

	endp





Rollendcredits2	proc	near

	call	rollem
	ret

	endp




Rollem	proc	near

	mov	cl,160
	mov	ch,160
	mov	di,25
	mov	bx,20
	mov	ds,mapstore
	mov	si,0
	call	multiget

	mov	es,textfile1
	mov	si,49*2
	mov	ax,[es:si]
	mov	si,ax
	add	si,textstart

	mov	cx,80
endcredits21:	push	cx

	mov	bx,10
	mov	cx,linespacing
endcredits22:	push	cx si di es bx

	call	vsync
	mov	cl,160
	mov	ch,160
	mov	di,25
	mov	bx,20
	mov	ds,mapstore
	mov	si,0
	call	multiput
	call	vsync
	pop	bx es di si
	push	si di es bx
	
	mov	cx,18
onelot2:	push	cx
	mov	di,25 ;75
	mov	dx,161
	mov	ax,0
	call	printdirect
	add	bx,linespacing
	pop	cx
	loop	onelot2

	call	vsync
	mov	cl,160
	mov	ch,160
	mov	di,25 ;75
	mov	bx,20
	call	multidump

	pop	bx es di si cx
	cmp	lasthardkey,1
	jz	endearly2
	dec	bx
	loop	endcredits22
	pop	cx
looknext2:	mov	al,[es:si]
	inc	si
	cmp	al,":"
	jz	gotnext2
	cmp	al,0
	jz	gotnext2
	jmp	looknext2
gotnext2:	cmp	lasthardkey,1
	jz	endearly
	loop	endcredits21
	
	mov	cx,120
	call	hangone
	ret
endearly2:	pop	cx
endearly:	ret

	endp









Fadecalculation	proc	near

	cmp	fadecount,0
	jz	nomorefading
	mov	bl,fadecount
	mov	es,buffers
	mov	si,startpal
	mov	di,endpal
	mov	cx,768
fadecolloop:	mov	al,[es:si]
	mov	ah,[es:di]
	cmp	al,ah
	jz	gotthere
	jc	lesscolour
	dec	byte ptr [es:si]
	jmp	gotthere
lesscolour:	cmp	bl,ah
	jz	withit
	jnc	gotthere
withit:	inc	byte ptr [es:si]
gotthere:	inc	si
	inc	di
	loop	fadecolloop
	dec	fadecount
	ret
nomorefading:	mov	fadedirection,0
	ret

	endp
























Greyscalesum	proc	near		;converts palette to grey scale
				;summed using formula:
	mov	es,buffers		; .20xred + .59xGreen + .11xBlue
	mov	si,maingamepal
	mov	di,endpal
	mov	cx,256		;convert 256 colours

greysumloop1:	push	cx
	mov	bx,0
	mov	al,[es:si]
	mov	ah,0
	mov	cx,20
	mul	cx
	add	bx,ax
	mov	al,[es:si+1]
	mov	ah,0
	mov	cx,59
	mul	cx
	add	bx,ax
	mov	al,[es:si+2]
	mov	ah,0
	mov	cx,11
	mul	cx
	add	bx,ax		;bx holds equationx100

	mov	al,-1		;divide result by 100
greysumloop2:	inc	al
	sub	bx,100
	jnc	greysumloop2		;ah holds grey scale number
	mov	bl,al

	mov	al,bl
	mov	ah,addtored
	cmp	al,0
	;jz	noaddr
	add	al,ah
noaddr:	stosb
	mov	ah,addtogreen
	mov	al,bl
	cmp	al,0
	jz	noaddg
	add	al,ah
noaddg:	stosb			;store result in red, green and
	mov	ah,addtoblue
	mov	al,bl
	cmp	al,0
	jz	noaddb
	add	al,ah
noaddb:	stosb			;blue portions of palette.

	add	si,3
	pop	cx
	loop	greysumloop1
	ret

	endp






















Showgroup	proc	near

	mov	dx,3c8h
	out	dx,al
	mov	dx,3c9h
showgroup1:	lodsb
	out	dx,al
	lodsb
	out	dx,al
	lodsb
	out	dx,al
	loop	showgroup1
	ret

	endp





Paltostartpal	proc	near

	mov	es,buffers
	mov	ds,buffers
	mov	si,maingamepal
	mov	di,startpal
	mov	cx,768/2
	rep	movsw
	ret

	endp



Endpaltostart	proc	near

	mov	es,buffers
	mov	ds,buffers
	mov	si,endpal
	mov	di,startpal
	mov	cx,768/2
	rep	movsw
	ret

	endp


Startpaltoend	proc	near

	mov	es,buffers
	mov	ds,buffers
	mov	di,endpal
	mov	si,startpal
	mov	cx,768/2
	rep	movsw
	ret

	endp








Paltoendpal	proc	near

	mov	es,buffers
	mov	ds,buffers
	mov	di,endpal
	mov	si,maingamepal
	mov	cx,768/2
	rep	movsw
	ret

	endp








	




Allpalette	proc	near

	mov	es,buffers
	mov	ds,buffers
	mov	di,startpal
	mov	si,maingamepal
	mov	cx,768/2
	rep	movsw
	call	dumpcurrent
	ret

	endp





Dumpcurrent	proc	near

	mov	si,startpal
	mov	ds,buffers
	call	vsync
	mov	al,0
	mov	cx,128
	call	showgroup
	call	vsync
	mov	al,128
	mov	cx,128
	call	showgroup
	ret

	endp











Fadedownmon	proc	near

	call	paltostartpal
	call	paltoendpal
	mov	es,buffers
	mov	di,endpal+(231*3)
	mov	cx,3*8
	mov	ax,0
	rep	stosb
	mov	di,endpal+(246*3)
	stosb
	stosw
	mov	fadedirection,1
	mov	fadecount,63
	mov	colourpos,0
	mov	numtofade,128
	mov	cx,64 ;100
	call	hangon ;curs
	ret

	endp





Fadeupmon	proc	near

	call	paltostartpal
	call	paltoendpal
	mov	es,buffers
	mov	di,startpal+(231*3)
	mov	cx,3*8
	mov	ax,0
	rep	stosb
	mov	di,startpal+(246*3)
	stosb
	stosw
	mov	fadedirection,1
	mov	fadecount,63
	mov	colourpos,0
	mov	numtofade,128
	mov	cx,128
	call	hangon ;curs
	ret

	endp





Fadeupmonfirst	proc	near

	call	paltostartpal
	call	paltoendpal
	mov	es,buffers
	mov	di,startpal+(231*3)
	mov	cx,3*8
	mov	ax,0
	rep	stosb
	mov	di,startpal+(246*3)
	stosb
	stosw
	mov	fadedirection,1
	mov	fadecount,63
	mov	colourpos,0
	mov	numtofade,128
	mov	cx,64
	call	hangon
	mov	al,26
	call	playchannel1
	mov	cx,64
	call	hangon

	ret

	endp







Fadeupyellows	proc	near

	;call	startpaltoend
	call	paltoendpal
	mov	es,buffers
	mov	di,endpal+(231*3)
	mov	cx,3*8
	mov	ax,0
	rep	stosb
	mov	di,endpal+(246*3)
	stosb
	stosw
	mov	fadedirection,1
	mov	fadecount,63
	mov	colourpos,0
	mov	numtofade,128	
	mov	cx,128
	call	hangon
	ret

	endp



Initialmoncols	proc	near

	call	paltostartpal
	mov	es,buffers
	mov	di,startpal+(230*3)
	mov	cx,3*9
	mov	ax,0
	rep	stosb
	mov	di,startpal+(246*3)
	stosb
	stosw
	mov	ds,buffers
	mov	si,startpal+(230*3)
	mov	al,230
	mov	cx,18
	call	showgroup
	ret

	endp
