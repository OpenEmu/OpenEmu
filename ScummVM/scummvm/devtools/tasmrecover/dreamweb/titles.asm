;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text

Titles	proc	near
	
	if	demo
	ret
	else
	call	clearpalette
	call	biblequote
	cmp quitrequested, 0
	jnz titlesearly
	call	intro
titlesearly:
	ret
	endif

	endp




Endgame	proc	near

	mov	dx,offset cs:endtextname
	call	loadtemptext
	call	monkspeaking
	call	gettingshot
	call	getridoftemptext
	mov	volumeto,7
	mov	volumedirection,1
	mov	cx,200
	call	hangon
	ret

	endp


	if	cd

Monkspeaking	proc	near

	mov	roomssample,35
	call	loadroomssample
	mov	dx,offset cs:monkface
	call	loadintotemp
	call	clearwork ;createpanel2
	call	showmonk
	call	worktoscreen
	mov	volume,7
	mov	volumedirection,-1
	mov	volumeto,5
	mov	al,12
	mov	ah,255
	call	playchannel0
	call	fadescreenups
	mov	cx,300
	call	hangon

	mov	al,40
loadspeech2:	push	ax
	mov	dl,"T"
	mov	dh,83
               	mov	cl,"T"
	mov	ah,0
	call	loadspeech
	mov	al,50+12
	call	playchannel1
notloadspeech2:	
	call vsync
	cmp	ch1playing,255
	jnz	notloadspeech2
	pop	ax
	inc	al
	cmp	al,48
	jnz	loadspeech2

	mov	volumedirection,1
	mov	volumeto,7
	call	fadescreendowns
	mov	cx,300
	call	hangon
	call	getridoftemp
	ret

	endp
	
	else

Monkspeaking	proc	near

	mov	roomssample,35
	call	loadroomssample
	mov	dx,offset cs:monkface
	call	loadintotemp
	call	clearwork ;createpanel2
	call	showmonk
	call	worktoscreen
	mov	volume,7
	mov	volumedirection,-1
	mov	volumeto,0
	mov	al,12
	mov	ah,255
	call	playchannel0
	call	fadescreenups
	mov	cx,300
	call	hangon

	mov	al,40
nextmonkspeak:	push	ax
	mov	ah,0
	mov	si,ax
	add	si,si
	mov	es,textfile1
	mov	ax,[es:si]
	add	ax,textstart
	mov	si,ax
nextbit:	mov	di,36
	mov	bx,140
	mov	dl,239
	call	printdirect
	push	ax si es
	call	worktoscreen
	call	clearwork
	call	showmonk
	mov	cx,240
	call	hangon
	pop	es si ax
	cmp	al,0
	jnz	nextbit
	pop	ax
	inc	al
	cmp	al,44
	jnz	nextmonkspeak

	mov	volumedirection,1
	mov	volumeto,7
	call	fadescreendowns
	mov	cx,300
	call	hangon
	call	getridoftemp
	ret

	endp

	endif




Showmonk	proc	near

	mov	al,0
	mov	ah,128
	mov	di,160
	mov	bx,72
	mov	ds,tempgraphics
	call	showframe
	ret

	endp


Gettingshot	proc	near

	mov	newlocation,55
	call	clearpalette
          	call	loadintroroom
         	call	fadescreenups
	mov	volumeto,0
	mov	volumedirection,-1	
        	call	runendseq
	call	clearbeforeload
	ret

	endp


	
	
	
	
Credits	proc	near

	call	clearpalette
	call	realcredits
	ret	
	
	endp



Biblequote	proc	near

	call	mode640x480
	mov	dx,offset cs:title0graphics
	call	showpcx
	call	fadescreenups
	mov	cx,80
	call	hangone
	cmp	lasthardkey,1
	jz	biblequotearly
	mov	cx,560
	call	hangone
	cmp	lasthardkey,1
	jz	biblequotearly
	call	fadescreendowns
	mov	cx,200 ;128
	call	hangone
	cmp	lasthardkey,1
	jz	biblequotearly
	call	cancelch0	
biblequotearly:
	mov lasthardkey,0
	ret

	endp




Hangone	proc	near

hangonloope:	push	cx
	call	vsync
	pop	cx
	cmp	lasthardkey,1
	jz	hangonearly
	loop	hangonloope
hangonearly:
	ret

	endp






Intro	proc	near

	mov	dx,offset cs:introtextname
	call	loadtemptext

	call	loadpalfromiff
	call	setmode

	mov	newlocation,50
	call	clearpalette
          	call	loadintroroom
	mov	volume,7
	mov	volumedirection,-1
	if	cd
	mov	volumeto,4
	else
	mov	volumeto,0
	endif
	mov	al,12 ;4
	mov	ah,255
	call	playchannel0
         	call	fadescreenups
        	call	runintroseq
		cmp	lasthardkey,1
		jz	introearly

;waitsound:	cmp	ch1blockstoplay,0
;	jnz	waitsound
	call	clearbeforeload
	
	mov	newlocation,52
	call	loadintroroom
	call	runintroseq
	cmp	lasthardkey,1
	jz	introearly
	call	clearbeforeload
	
	mov	newlocation,53
	call	loadintroroom
	call	runintroseq
	cmp	lasthardkey,1
	jz	introearly
	call	clearbeforeload
	
	call	allpalette
	mov	newlocation,54
	call	loadintroroom
	;mov	al,12
	;mov	ah,255
	;call	playchannel0
	call	runintroseq
	cmp	lasthardkey,1
	jz	introearly

	call	getridoftemptext
	call	clearbeforeload
introearly:
	mov lasthardkey, 0
	ret

	endp








Runintroseq	proc	near

	mov	getback,0

moreintroseq:	call	vsync
	cmp	lasthardkey,1
	jz	earlyendrun
	call	spriteupdate
	call	vsync
	cmp	lasthardkey,1
	jz	earlyendrun
	call	deleverything
	call	printsprites
	call	reelsonscreen
	call	afterintroroom
	call	usetimedtext
	call	vsync
	cmp	lasthardkey,1
	jz	earlyendrun
	call	dumpmap
	call	dumptimedtext
	call	vsync
	cmp	lasthardkey,1
	jz	earlyendrun
	cmp	getback,1
	jnz	moreintroseq
	ret
earlyendrun:
	call	getridoftemptext
	call	clearbeforeload
	ret

	endp





Runendseq	proc	near
	
	call	atmospheres
	mov	getback,0
moreendseq:	call	vsync
	call	spriteupdate
	call	vsync
	call	deleverything
	call	printsprites
	call	reelsonscreen
	call	afterintroroom
	call	usetimedtext
	call	vsync
	call	dumpmap
	call	dumptimedtext
	call	vsync
	cmp	getback,1
	jnz	moreendseq
	ret

	endp




Loadintroroom	proc	near

	mov	introcount,0
	mov	location,255
	call	loadroom
	mov	mapoffsetx,72
	mov	mapoffsety,16
	call	clearsprites
	mov	throughdoor,0
	mov	currentkey,"0"
	mov	mainmode,0
	call	clearwork
	mov	newobs,1
	call	drawfloor
	call	reelsonscreen
	call	spriteupdate
	call	printsprites
	call	worktoscreen
	ret

	endp






Mode640x480	proc	near

	mov	al,12h+128
	mov	ah,0
	int	10h
	;call	clearpalette
	ret

	endp



Set16colpalette	proc	near

	mov	cx,16
	mov	bl,0
	mov	bh,0
	mov	al,0
	mov	ah,10h
set16palloop2:	push	ax bx cx
	int	10h
	pop	cx bx ax
	inc	bl
	inc	bh
	loop	set16palloop2
	
	mov	bl,31h
	mov	al,1
	mov	ah,12h
	int	10h
	ret

	endp





RealCredits	proc	near

	mov	roomssample,33
	call	loadroomssample
	mov	volume,0

	call	mode640x480
	mov	cx,35
	call	hangon

	mov	dx,offset cs:title1graphics
	call	showpcx
	mov	al,12
	mov	ah,0
	call	playchannel0
	mov	cx,2
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly
	call	allpalette
	mov	cx,80
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly
	call	fadescreendowns
	mov	cx,256
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly

	mov	dx,offset cs:title2graphics
	call	showpcx
	mov	al,12
	mov	ah,0
	call	playchannel0
	mov	cx,2
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly
	call	allpalette
	mov	cx,80
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly
	call	fadescreendowns
	mov	cx,256
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly

	if	demo
	else
	mov	dx,offset cs:title3graphics
	call	showpcx
	mov	al,12
	mov	ah,0
	call	playchannel0
	mov	cx,2
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly
	call	allpalette
	mov	cx,80
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly
	call	fadescreendowns
	mov	cx,256
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly

	mov	dx,offset cs:title4graphics
	call	showpcx
	mov	al,12
	mov	ah,0
	call	playchannel0
	mov	cx,2
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly
	call	allpalette
	mov	cx,80
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly
	call	fadescreendowns
	mov	cx,256
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly

	mov	dx,offset cs:title5graphics
	call	showpcx
	mov	al,12
	mov	ah,0
	call	playchannel0
	mov	cx,2
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly
	call	allpalette
	mov	cx,80
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly
	call	fadescreendowns
	mov	cx,256
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly
	endif

	mov	dx,offset cs:title6graphics
	call	showpcx
	call	fadescreenups
	mov	cx,60
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly
	mov	al,13
	mov	ah,0
	call	playchannel0
	mov	cx,350
	call	hangone
	cmp	lasthardkey,1
	jz	realcreditsearly
	call	fadescreendowns
	mov	cx,256
	call	hangone
realcreditsearly:
	mov lasthardkey, 0
	ret

	endp
