;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text
Screenwidth	equ	320		;physical width of screen



Allocatework	proc	near

	mov	bx,1000h
	call	allocatemem
	mov	workspace,ax
	ret

	endp





Showpcx	proc	near

	call	openfile
	mov	bx,handle
	mov	ds,workspace
	mov	ah,3fh
	mov	cx,128
	mov	dx,0
	int	21h

	mov	ds,workspace
	mov	si,16
	mov	cx,48
	mov	es,buffers
	mov	di,maingamepal
pcxpal:	push	cx
	call	readabyte
	shr	al,1
	shr	al,1
	stosb
	pop	cx
	loop	pcxpal
	mov	cx,768-48
	mov	ax,0ffffh
	rep	stosw

	call	readoneblock
	mov	si,0
	mov	di,0
	mov	cx,480
convertpcx:	push	cx
	push	di
	mov	ds,workspace 
	mov	es,buffers
	mov	di,pointerback
	mov	bx,0
sameline:	call	readabyte
	mov	ah,al
	and	ah,11000000b
	cmp	ah,11000000b
	jnz	normal
	mov	cl,al
	and	cl,00111111b
	mov	ch,0
	push	cx
	call	readabyte
	pop	cx
	add	bx,cx
	rep	stosb
	cmp	bx,4*80
	jnz	sameline
	jmp	endline
normal:	stosb
	inc	bx
	cmp	bx,4*80
	jnz	sameline

endline:	pop	di
	push	si
	mov	dx,0a000h
	mov	es,dx
	mov	si,pointerback
	mov	ds,buffers

	mov	dx,03c4h
	mov	al,2
	mov	ah,1
	out	dx,ax
	mov	cx,40
	push	di
	rep	movsw
	pop	di
	mov	ah,2
	out	dx,ax
	mov	cx,40
	push	di
	rep	movsw
	pop	di
	mov	ah,4
	out	dx,ax
	mov	cx,40
	push	di
	rep	movsw
	pop	di
	mov	ah,8
	out	dx,ax
	mov	cx,40
	rep	movsw

	pop	si
	pop	cx
	loop	convertpcx

	mov	bx,handle
	call	closefile
	ret

	endp




Readabyte	proc	near

	cmp	si,30000
	jnz	notendblock
	push	bx es di ds si
	call	readoneblock
	pop	si ds di es bx
	mov	si,0
notendblock:	lodsb
	ret
	
	endp




Readoneblock	proc	near

	mov	bx,handle
	mov	ah,3fh
	mov	ds,workspace
	mov	ah,3fh
	mov	cx,30000
	mov	dx,0
	int	21h
	ret

	endp








Loadpalfromiff	proc	near

	mov	dx,offset cs:palettescreen
	call	openfile
	mov	cx,2000
	mov	ds,mapstore
	mov	dx,0
	call	readfromfile
	call	closefile
	mov	es,buffers
	mov	di,maingamepal
	mov	ds,mapstore
	mov	si,30h
	mov	cx,768
palloop:	lodsb
	shr	al,1
	shr	al,1

	cmp	brightness,1
	jnz	nought
	cmp	al,0
	jz	nought
       	mov	ah,al
	shr	ah,1
	add	al,ah
	shr	ah,1
	add	al,ah
	cmp	al,64
	jc	nought
	mov	al,63

nought:	stosb
	loop	palloop
	ret

	endp






Setmode	proc	near

	call	vsync
	mov	ah,12h
	mov	al,1
	mov	bl,33h
	int	10h

	mov	ah,0
	mov	al,13h
	int	10h
	
	mov	al,6	; sets graphic controller
	mov	dx,3ceh 	; register 6 (MM) to 1 - 64K
	out	dx,al
	inc	dx
	in	al,dx
	and	al,11110011b
	or	al,00000100b
	out	dx,al

	mov	al,4	; sets sequencer
	mov	dx,3c4h 	; register 4 (EM) to 1 - >64K
	out	dx,al
	inc	dx
	in	al,dx
	and	al,11111101b
	or	al,00000010b
	out	dx,al

	mov	al,13h		;give screen 16 extra hidden
	mov	dx,3d4h 		;pixels at one side
	out	dx,al
	inc	dx
	mov	al,screenwidth/8 ; width of screen
	out	dx,al

	mov	al,8h
	mov	dx,3d4h
	out	dx,al
	inc	dx
	mov	al,00000000b
	out	dx,al

	mov	al,11h
	mov	dx,3d4h
	out	dx,al
	inc	dx
	in	al,dx
	or	al,128
	out	dx,al

	mov	al,00
	mov	dx,3d4h
	out	dx,al
	inc	dx
	mov	al,3fh
	out	dx,al
	mov	al,01
	mov	dx,3d4h
	out	dx,al
	inc	dx
	mov	al,3fh
	out	dx,al
	ret
	
	endp



Cls	proc	near

	mov	ax,0a000h
	mov	es,ax
	mov	di,0
	mov	cx,7fffh
	mov	ax,0
	rep	stosw
	ret

	endp



Printundermon	proc	near		;prints workspace through the text

	mov	si,(screenwidth*43)+76
	mov	di,si
	mov	es,workspace
	add	si,8*screenwidth
	mov	dx,0a000h
	mov	ds,dx
	mov	cx,104
scrollmonloop1: push	cx di si
	mov	cx,170
scrollmonloop2: lodsb
	cmp	al,231
	jnc	dontplace
placeit:	stosb
	loop	scrollmonloop2
	jmp	finmonscroll
dontplace:	inc	di
	loop	scrollmonloop2

finmonscroll:	pop	si di cx
	add	si,screenwidth
	add	di,screenwidth
	loop	scrollmonloop1
	ret

	endp








Worktoscreen	proc	near

	call	vsync
	mov	si,0
	mov	di,0
	mov	cx,25
	mov	ds,workspace
	mov	dx,0a000h
	mov	es,dx

dumpallloop:	call	width160
	call	width160
	call	width160
	call	width160
	call	width160
	call	width160
	call	width160
	call	width160
	loop	dumpallloop

	ret

	endp









;
;Worktoscreen2	proc	near
;
;	call	showpointer
;
;	mov	ds,workspace
;	mov	dx,0a000h
;	mov	es,dx
;
;	mov	si,320-16
;	mov	di,320-16
;	mov	bl,33
;	mov	cx,16
;screen2loop1:	push	di si cx
;	call	vsync
;	cmp	bl,21
;	jc	screen2loop2
;	sub	cx,16
;	jz	isoneblock
;screen2loop2:	movsw
;	movsw
;	movsw
;	movsw
;	movsw
;	movsw
;	movsw
;	movsw
;	add	di,320-15
;	add	si,320-15
;	loop	screen2loop2
;isoneblock:	mov	cx,16
;	mov	ax,320-15
;oneblockloop:	push	cx
;	rep	movsb
;	pop	cx
;	add	si,ax
;	add	di,ax
;	inc	ax
;	loop	oneblockloop
;
;	pop	cx si di
;	add	cx,16
;	cmp	cx,200
;	jc	itsallright
;	mov	cx,200
;itsallright:	sub	si,16
;	sub	di,16
;	dec	bl
;	jnz	screen2loop1
;
;	call	delpointer
;	ret
;
;	endp
;
;
;
;




Paneltomap	proc	near

	mov	di,mapxstart
	add	di,mapadx
	mov	bx,mapystart
	add	bx,mapady
	mov	ds,mapstore
	mov	si,0
	mov	cl,mapxsize
	mov	ch,mapysize
	call	multiget
	ret

	endp



Maptopanel	proc	near

	mov	di,mapxstart
	add	di,mapadx
	mov	bx,mapystart
	add	bx,mapady
	mov	ds,mapstore
	mov	si,0
	mov	cl,mapxsize
	mov	ch,mapysize
	call	multiput

	ret

	endp





Dumpmap	proc	near

	mov	di,mapxstart
	add	di,mapadx
	mov	bx,mapystart
	add	bx,mapady
	mov	cl,mapxsize
	mov	ch,mapysize
	call	multidump
	ret

	endp

	


Pixelcheckset	proc	near		;al=x, ah=y, es:bx=setlist pos
				;checks exact pixel in a frame
	push	ax
	sub	al,[es:bx]		;for detection.
	sub	ah,[es:bx+1]		;al,ah now holds offset within
				;the frame
	push	es bx cx ax
	mov	al,[es:bx+4]		;object number
	call	getsetad
	mov	al,[es:bx+17]		;finds frame number
	mov	es,setframes
	mov	bx,framedata
	mov	ah,0
	mov	cx,6
	mul	cx
	add	bx,ax		;get data for this frame in es:bx
	pop	ax

	push	ax
	mov	al,ah
	mov	ah,0
	mov	cl,[es:bx]
	mov	ch,0
	mul	cx
	pop	cx
	mov	ch,0
	add	ax,cx		;ax now holds offset from corner
				;of the frame
	add	ax,[es:bx+2]
	mov	bx,ax		;es:bx now holds offset of pixel!
	add	bx,frames

	mov	al,[es:bx]
	mov	dl,al
	pop	cx bx es ax
	cmp	dl,0
	ret

	endp












Createpanel	proc	near

	mov	di,0
	mov	bx,8
	mov	ds,icons2
	mov	al,0
	mov	ah,2
	call	showframe ;spritef
	mov	di,160
	mov	bx,8
	mov	ds,icons2
	mov	al,0
	mov	ah,2
	call	showframe ;spritef
	mov	di,0
	mov	bx,104
	mov	ds,icons2
	mov	al,0
	mov	ah,2
	call	showframe ;spritef
	mov	di,160
	mov	bx,104
	mov	ds,icons2
	mov	al,0
	mov	ah,2
	call	showframe ;spritef
	ret

	endp



Createpanel2	proc	near

	call	createpanel
	mov	di,0
	mov	bx,0
	mov	ds,icons2
	mov	al,5
	mov	ah,2
	call	showframe
	mov	di,160
	mov	bx,0
	mov	ds,icons2
	mov	al,5
	mov	ah,2
	call	showframe
	ret

	endp






;Showspritef	proc	near
;
;	mov	ax,bx
;	mov	bx,screenwidth
;	mul	bx
;	add	di,ax
;	mov	dx,screenwidth
;	mov	es,workspace
;	mov	si,2080
;	mov	ah,0
;	add	ax,ax
;	mov	bx,ax
;	add	ax,ax
;	add	bx,ax
;	add	si,[bx+2]
;	mov	cx,[bx+0]
;spritefloop:	push	cx di
;	call	width80
;	pop	di cx
;	add	di,dx
;	dec	ch
;	jnz	spritefloop
;	ret
;
;	endp
;
;








Clearwork	proc	near

	mov	ax,0h
	mov	es,workspace
	mov	di,0
	mov	cx,(200*320)/64
clearloop:	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	loop	clearloop
	ret

	endp






Vsync	proc	near

	push	ax bx cx dx si di ds es

	mov	dx,03dah
loop2:	in	al,dx
	test	al,8
	jz	loop2
loop1:	in	al,dx
	test	al,8
	jnz	loop1

	call	doshake
	call	dofade
	if	debuglevel2
	call	debugkeys
	endif
	pop	es ds di si dx cx bx ax
	ret
	
	endp



Doshake	proc	near

	cmp	shakecounter,48
	jz	finishshake
            	inc	shakecounter
	mov	bl,shakecounter
	mov	bh,0
	add	bx,offset cs:shaketable
	mov	al,10h
	mov	dx,3d4h
	out	dx,al
	inc	dx
	mov	al,[cs:bx]
	out	dx,al
finishshake:	ret

shaketable:	db	9ch,9ah,9fh,9ah,9ch,9eh,0a0h,9bh,9dh,99h,9fh,9eh
	db	9ch,9ah,9fh,9ah,9ch,9eh,0a0h,9bh,9dh,99h,9fh,9eh
	db	9ch,9ah,9fh,9ah,9ch,9eh,0a0h,9bh,9dh,99h,9fh,9eh
	db	9ch,9ah,9fh,9ah,9ch,9eh,0a0h,9bh,9dh,99h,9fh,9eh
	db	9ch,9ah,9fh,9ah,9ch,9eh,0a0h,9bh,9dh,99h,9fh,9eh
	db	9ch,9ah,9fh,9ah,9ch,9eh,0a0h,9bh,9dh,99h,9fh,9eh
	db	9ch,9ah,9fh,9ah,9ch,9eh,0a0h,9bh,9dh,99h,9fh,9eh
	db	9ch,9ah,9fh,9ah,9ch,9eh,0a0h,9bh,9dh,99h,9fh,9eh
	db	9ch,9ah,9fh,9ah,9ch,9eh,0a0h,9bh,9dh,99h,9fh,9ch

	endp






Zoom	proc	near

	cmp	watchingtime,0
	jnz	inwatching
	cmp	zoomon,1
	jz	zoomswitch
inwatching:	ret

zoomswitch:	cmp	commandtype,199
	jc	zoomit
cantzoom:	call	putunderzoom
	ret

zoomit:	mov	ax,oldpointery
	sub	ax,9
	mov	cx,screenwidth
	mul	cx
	add	ax,oldpointerx
	sub	ax,11
	mov	si,ax

	mov	ax,zoomy+4
	mov	cx,screenwidth
	mul	cx
	add	ax,zoomx+5
	mov	di,ax
	mov	es,workspace
	mov	ds,workspace

	mov	cx,20
zoomloop:	push	cx
	mov	cx,23
zoomloop2:	lodsb
	mov	ah,al
	stosw
	mov	[es:di+screenwidth-2],ax
	loop	zoomloop2
	add	si,screenwidth-23
	add	di,screenwidth-46+screenwidth
	pop	cx
	loop	zoomloop

	call	crosshair
	mov	didzoom,1
	ret

	endp









Delthisone	proc	near

	push	ax
	push	ax
	mov	al,ah
	mov	ah,0
	add	ax,mapady
	mov	bx,screenwidth
	mul	bx
	pop	bx
	mov	bh,0
	add	bx,mapadx
	add	ax,bx
	mov	di,ax
	pop	ax
	push	ax
	mov	al,ah
	mov	ah,0
	mov	bx,22*8
	mul	bx
	pop	bx
	mov	bh,0
	add	ax,bx
	mov	si,ax

	mov	es,workspace
	mov	ds,mapstore
	mov	dl,cl
	mov	dh,0
	mov	ax,screenwidth
	sub	ax,dx
	neg	dx
	add	dx,22*8
deloneloop:	push	cx
	mov	ch,0
	rep	movsb
	pop	cx
	add	di,ax
	add	si,dx
	dec	ch
	jnz	deloneloop
	ret

	endp







;------------------------------------------------------------Pointer update----


Multiget	proc	near		;di,bx = dest x,y
				;cl,ch = size
	mov	ax,bx		;si,di = storage
	mov	bx,screenwidth
	mul	bx
	add	di,ax

	mov	es,workspace
	push	es ds
	pop	es ds
	xchg	di,si
	mov	al,cl
	mov	ah,0
	mov	dx,screenwidth
	sub	dx,ax

	mov	al,cl
	and	al,1
	jnz	oddwidth2

	mov	bl,cl
	mov	bh,0
	mov	ax,offset cs:width0
	shr	bx,1
	sub	ax,bx
	mov	cl,ch
	mov	ch,0
multiloop3:	call	ax
	add	si,dx
	loop	multiloop3
	ret

oddwidth2:	mov	bl,cl
	mov	bh,0
	shr	bx,1
	mov	ax,offset cs:width0
	sub	ax,bx
	mov	cl,ch
	mov	ch,0
multiloop4:	call	ax
	movsb
	add	si,dx
	loop	multiloop4
	ret

	endp







Multiput	proc	near		;di,bx = dest x,y
				;cl,ch = size
	mov	ax,bx		;si,di = storage
	mov	bx,screenwidth
	mul	bx
	add	di,ax

	mov	es,workspace
	mov	al,cl
	mov	ah,0
	mov	dx,screenwidth
	sub	dx,ax

	mov	al,cl
	and	al,1
	jnz	oddwidth3

	mov	bl,cl
	mov	bh,0
	shr	bx,1
	mov	ax,offset cs:width0
	sub	ax,bx
	mov	cl,ch
	mov	ch,0
multiloop5:	call	ax
	add	di,dx
	loop	multiloop5
	ret

oddwidth3:	mov	bl,cl
	mov	bh,0
	shr	bx,1
	mov	ax,offset cs:width0
	sub	ax,bx
	mov	cl,ch
	mov	ch,0
multiloop6:	call	ax
	movsb
	add	di,dx
	loop	multiloop6
	ret


	endp









Multidump	proc	near		;di,bx = dest x,y
				;cl,ch = size
	mov	dx,0a000h
	mov	es,dx
	mov	ds,workspace

	mov	ax,bx
	mov	bx,screenwidth
	mul	bx
	add	di,ax
	mov	dx,screenwidth
	mov	si,di

	mov	al,cl
	and	al,1
	jnz	oddwidth

	mov	bl,cl
	mov	bh,0
	shr	bx,1
	mov	ax,offset cs:width0
	sub	ax,bx
	mov	bl,cl
	mov	bh,0
	neg	bx
	add	bx,dx
	mov	cl,ch
	mov	ch,0
multiloop1:	call	ax
	add	di,bx
	add	si,bx
	loop	multiloop1
	ret

oddwidth:	mov	bl,cl
	mov	bh,0
	shr	bx,1
	mov	ax,offset cs:width0
	sub	ax,bx
	mov	bl,cl
	mov	bh,0
	neg	bx
	add	bx,screenwidth
	mov	cl,ch
	mov	ch,0
multiloop2:	call	ax
	movsb
	add	di,bx
	add	si,bx
	loop	multiloop2
	ret

	endp





Width160	proc	near

	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
width128:	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
width110:	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
width88:	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
width80:	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
width63:	movsw
width62:	movsw
width61:	movsw
width60:	movsw
width59:	movsw
width58:	movsw
width57:	movsw
width56:	movsw
width55:	movsw
width54:	movsw
width53:	movsw
width52:	movsw
width51:	movsw
width50:	movsw
width49:	movsw
width48:	movsw
width47:	movsw
width46:	movsw
width45:	movsw
width44:	movsw
width43:	movsw
width42:	movsw
width41:	movsw
width40:	movsw
width39:	movsw
width38:	movsw	   
width37:	movsw
width36:	movsw
width35:	movsw
width34:	movsw
width33:	movsw
width32:	movsw
width31:	movsw
width30:	movsw
width29:	movsw
width28:	movsw
width27:	movsw
width26:	movsw
width25:	movsw
width24:	movsw
width23:	movsw
width22:	movsw
width21:	movsw
width20:	movsw
width19:	movsw
width18:	movsw
width17:	movsw
width16:	movsw
width15:	movsw
width14:	movsw
width13:	movsw
width12:	movsw
width11:	movsw
width10:	movsw
width9: 	movsw
width8: 	movsw
width7: 	movsw
width6: 	movsw
width5: 	movsw
width4: 	movsw
width3: 	movsw
width2: 	movsw
width1: 	movsw
width0: 	ret

	endp







	

Doblocks	proc	near

	mov	es,workspace
	mov	ax,mapady
	mov	cx,screenwidth
	mul	cx
	mov	di,mapadx
	add	di,ax

	mov	al,mapy
	mov	ah,0
	mov	bx,mapwidth
	mul	bx
	mov	bl,mapx
	mov	bh,0
	add	ax,bx

	mov	si,map
	add	si,ax

	mov	cx,10
loop120:	push	di cx
	mov	cx,11
loop124:	push	cx di

	mov	ds,mapdata
	lodsb
	mov	ds,backdrop

	push	si
	cmp	al,0
	jz	zeroblock
	mov	ah,al
	mov	al,0
	mov	si,blocks
	add	si,ax
	mov	bh,14


	mov	bh,4
firstbitofblock:	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	add	di,screenwidth-16
	dec	bh
	jnz	firstbitofblock

	mov	bh,12
loop125:	movsw
	movsw  
	movsw
	movsw
	movsw 
	movsw
	movsw
	movsw
	mov	ax,0dfdfh
	stosw
	stosw

	add	di,screenwidth-20
	dec	bh
	jnz	loop125
	
	add	di,4
	mov	ax,0dfdfh
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	add	di,screenwidth-16
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	add	di,screenwidth-16
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	add	di,screenwidth-16
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw
	stosw


zeroblock:	pop	si
	
	pop	di cx
	add	di,16
	loop	loop124
	add	si,mapwidth-11
	pop	cx di
	add	di,screenwidth*16
	loop	loop120
	ret
	
	endp







;----------------------------------------------General sprite output routine----

Showframe	proc	near 	; shows a frame from sprites

	push	dx ax	; es=destination
	mov	cx,ax	; ds=source
	and	cx,511	; di=x, bx=y
	add	cx,cx	; al=frame number
	mov	si,cx	; ah=effects flag
	add	cx,cx
	add	si,cx
	cmp	word ptr [si],0
	jnz	notblankshow
	pop ax dx
	mov	cx,0
	ret

notblankshow:	test	ah,128
	jnz	skipoffsets
	mov	al,[si+4]
	mov	ah,0
	add	di,ax
	mov	al,[si+5]
	mov	ah,0
	add	bx,ax
skipoffsets:	mov	cx,[si+0]
	mov	ax,[si+2]
	add	ax,2080
	mov	si,ax
	pop	ax dx
	cmp	ah,0
	jz	noeffects

	test	ah,128
	jz	notcentred
	push	ax
	mov	al,cl
	mov	ah,0
	shr	ax,1
	sub	di,ax
	mov	al,ch
	mov	ah,0
	shr	ax,1
	sub	bx,ax
	pop	ax

notcentred:	test	ah,64
	jz	notdiffdest
	push	cx
	call	frameoutfx
	pop	cx
	ret

notdiffdest:	test	ah,8
	jz	notprintlist
	push	ax
	mov	ax,di
	sub	ax,mapadx
	push	bx
	sub	bx,mapady
	mov	ah,bl
	pop	bx
	;call	addtoprintlist
	pop	ax

notprintlist:	test	ah,4
	jz	notflippedx
	mov	dx,screenwidth
	mov	es,workspace
	push	cx
	call	frameoutfx
	pop	cx
	ret

notflippedx:	test	ah,2
	jz	notnomask
	mov	dx,screenwidth
	mov	es,workspace
	push	cx
	call	frameoutnm
	pop	cx
	ret

notnomask:	test	ah,32
	jz	noeffects
	mov	dx,screenwidth
	mov	es,workspace
	push	cx
	call	frameoutbh
	pop	cx
	ret

noeffects:	mov	dx,screenwidth
	mov	es,workspace
	push	cx
	call	frameoutv
	pop	cx		; returns size printed in cx
	ret

	endp










Frameoutv	proc	near

	push	dx
	mov	ax,bx
	mov	bx,dx
	mul	bx
	add	di,ax
	pop	dx

	push	cx
	mov	ch,0
	sub	dx,cx
	pop	cx

frameloop1:	push	cx
	mov	ch,0

frameloop2:	lodsb
	cmp	al,0
	jnz	backtosolid
backtoother:	inc	di
	loop	frameloop2
	pop	cx
	add	di,dx
	dec	ch
	jnz	frameloop1
	ret

frameloop3:	lodsb
	cmp	al,0
	jz	backtoother
backtosolid:	stosb
	loop	frameloop3
	pop	cx
	add	di,dx
	dec	ch
	jnz	frameloop1
	ret

	endp






Frameoutnm	proc	near

	push	dx
	mov	ax,bx
	mov	bx,dx
	mul	bx
	add	di,ax
	pop	dx

	push	cx
	mov	ch,0
	sub	dx,cx
	pop	cx

	mov	al,cl
	and	al,1
	jnz	oddwidthframe

	mov	bl,cl
	mov	bh,0
	mov	ax,offset cs:width0
	shr	bx,1
	sub	ax,bx
	mov	cl,ch
	mov	ch,0
nmloop1:	call	ax
	add	di,dx
	loop		nmloop1
	ret

oddwidthframe:	mov	bl,cl
	mov	bh,0
	shr	bx,1
	mov	ax,offset cs:width0
	sub	ax,bx
	mov	cl,ch
	mov	ch,0
nmloop2:	call	ax
	movsb
	add	di,dx
	loop	nmloop2
	ret

	endp






Frameoutbh	proc	near

	push	dx
	mov	ax,bx
	mov	bx,dx
	mul	bx
	add	di,ax
	pop	dx

	push	cx
	mov	ch,0
	sub	dx,cx
	pop	cx

bhloop2:	push	cx
	mov	ch,0
	mov	ah,255
bhloop1:	cmp	[es:di],ah
	jnz	nofill
	movsb
	loop	bhloop1
	jmp	nextline
nofill:	inc	di
	inc	si
	loop	bhloop1
nextline:	add	di,dx
	pop	cx
	dec	ch
	jnz	bhloop2
	ret
	
	endp







Frameoutfx	proc	near

	push	dx
	mov	ax,bx
	mov	bx,dx
	mul	bx
	add	di,ax
	pop	dx

	push	cx
	mov	ch,0
	add	dx,cx
	pop	cx

frameloopfx1:	push	cx
	mov	ch,0

frameloopfx2:	lodsb
	cmp	al,0
	jnz	backtosolidfx
backtootherfx:	dec	di
	loop	frameloopfx2
	pop	cx
	add	di,dx
	dec	ch
	jnz	frameloopfx1
	ret

frameloopfx3:	lodsb
	cmp	al,0
	jz	backtootherfx
backtosolidfx:	mov	[es:di],al
	dec	di
	loop	frameloopfx3
	pop	cx
	add	di,dx
	dec	ch
	jnz	frameloopfx1
	ret

	endp




;---------------------------------------------------Transfers for extra data----

Transferinv	proc	near

	mov	di,exframepos
	push	di
	mov	al,expos
	mov	ah,0
	mov	bx,ax
	add	ax,ax
	add	ax,bx
	inc	ax
	mov	cx,6
	mul	cx
	mov	es,extras
	mov	bx,exframedata
	add	bx,ax
	add	di,exframes
	
	push	bx
	mov	al,itemtotran
	mov	ah,0
	mov	bx,ax
	add	ax,ax
	add	ax,bx
	inc	ax
	mov	cx,6
	mul	cx
	mov	ds,freeframes
	mov	bx,frframedata
	add	bx,ax
	mov	si,frframes
	mov	al,[bx]
	mov	ah,0
	mov	cl,[bx+1]
	mov	ch,0
	add	si,[bx+2]		;we have si, and length
	mov	dx,[bx+4]
	pop	bx
	mov	[es:bx+0],al
	mov	[es:bx+1],cl
	mov	[es:bx+4],dx

	mul	cx
	mov	cx,ax
	push	cx
	rep	movsb
	pop	cx
	pop	ax
	mov	[es:bx+2],ax
	add	exframepos,cx
	ret

	endp










Transfermap	proc	near

	mov	di,exframepos
	push	di
	mov	al,expos
	mov	ah,0
	mov	bx,ax
	add	ax,ax
	add	ax,bx
	mov	cx,6
	mul	cx
	mov	es,extras
	mov	bx,exframedata
	add	bx,ax
	add	di,exframes
	
	push	bx
	mov	al,itemtotran
	mov	ah,0
	mov	bx,ax
	add	ax,ax
	add	ax,bx
	mov	cx,6
	mul	cx
	mov	ds,freeframes
	mov	bx,frframedata
	add	bx,ax
	mov	si,frframes
	mov	al,[bx]
	mov	ah,0
	mov	cl,[bx+1]
	mov	ch,0
	add	si,[bx+2]		;we have si, and length
	mov	dx,[bx+4]
	pop	bx
	mov	[es:bx+0],al
	mov	[es:bx+1],cl
	mov	[es:bx+4],dx

	mul	cx
	mov	cx,ax
	push	cx
	rep	movsb
	pop	cx
	pop	ax
	mov	[es:bx+2],ax
	add	exframepos,cx
	ret

	endp


;------------------------------------------------------------------Filenames----


Spritename1	db	"DREAMWEB.S00",0
Spritename3	db	"DREAMWEB.S02",0

Idname	db	"INSTALL.DAT",0

Characterset1	db	"DREAMWEB.C00",0
Characterset2	db	"DREAMWEB.C01",0
Characterset3	db	"DREAMWEB.C02",0

Samplename	db	"DREAMWEB.V00",0

Basicsample	db	"DREAMWEB.V99",0

Icongraphics0	db	"DREAMWEB.G00",0
Icongraphics1	db	"DREAMWEB.G01",0
Extragraphics1	db	"DREAMWEB.G02",0
Icongraphics8	db	"DREAMWEB.G08",0
Mongraphicname	db	"DREAMWEB.G03",0
Mongraphics2	db	"DREAMWEB.G07",0
Cityname	db	"DREAMWEB.G04",0
Travelgraphic1	db	"DREAMWEB.G05",0
Travelgraphic2	db	"DREAMWEB.G06",0
Diarygraphic	db	"DREAMWEB.G14",0

Monitorfile1	db	"DREAMWEB.T01",0
Monitorfile2	db	"DREAMWEB.T02",0
Monitorfile10	db	"DREAMWEB.T10",0	;News items 10-13
Monitorfile11	db	"DREAMWEB.T11",0
Monitorfile12	db	"DREAMWEB.T12",0
Monitorfile13	db	"DREAMWEB.T13",0
Monitorfile20	db	"DREAMWEB.T20",0
Monitorfile21	db	"DREAMWEB.T21",0	;Ryan's private stuff
Monitorfile22	db	"DREAMWEB.T22",0	;Use for blank carts
Monitorfile23	db	"DREAMWEB.T23",0	;Use for edens cart
Monitorfile24	db	"DREAMWEB.T24",0	;Use for church cart
Foldertext	db	"DREAMWEB.T50",0
Diarytext	db	"DREAMWEB.T51",0
Puzzletextname	db	"DREAMWEB.T80",0	;puzzle text
Traveltextname	db	"DREAMWEB.T81",0	;location descriptions
Introtextname	db	"DREAMWEB.T82",0	;intro sequence
Endtextname	db	"DREAMWEB.T83",0	;end sequence/credits
Commandtextname	db	"DREAMWEB.T84",0	;commands

Volumetabname	db	"DREAMWEB.VOL",0

Foldergraphic1	db	"DREAMWEB.G09",0
Foldergraphic2	db	"DREAMWEB.G10",0
Foldergraphic3	db	"DREAMWEB.G11",0
Symbolgraphic	db	"DREAMWEB.G12",0
Gungraphic	db	"DREAMWEB.G13",0
Monkface	db	"DREAMWEB.G15",0

Title0graphics	db	"DREAMWEB.I00",0
Title1graphics	db	"DREAMWEB.I01",0
Title2graphics	db	"DREAMWEB.I02",0
Title3graphics	db	"DREAMWEB.I03",0
Title4graphics	db	"DREAMWEB.I04",0
Title5graphics	db	"DREAMWEB.I05",0
Title6graphics	db	"DREAMWEB.I06",0
Title7graphics	db	"DREAMWEB.I07",0

Palettescreen	db	"DREAMWEB.PAL",0
