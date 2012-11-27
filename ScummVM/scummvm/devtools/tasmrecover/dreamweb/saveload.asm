;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text


Zoomonoff	proc	near

	cmp	watchingtime,0
	jnz	blank
	cmp	pointermode,2
	jz	blank
	cmp	commandtype,222
	jz	alreadyonoff
	mov	commandtype,222
	mov	al,39
	call	commandonly
alreadyonoff:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	nozoomonoff
	and	ax,1
	jnz	dozoomonoff
nozoomonoff:	ret

dozoomonoff:	mov	al,zoomon
	xor	al,1
	mov	zoomon,al

	call	createpanel
	mov	newobs,0
	call	drawfloor
	call	printsprites
	call	reelsonscreen
	call	showicon
	call	getunderzoom
	call	undertextline
	mov	al,39
	call	commandonly
	call	readmouse
	call	worktoscreenm
	ret

	endp












Saveload	proc	near

	if	demo
	call	dosreturn
	ret
	else
	cmp	watchingtime,0
	jnz	blank
	cmp	pointermode,2
	jz	blank
	cmp	commandtype,253
	jz	alreadyops
	mov	commandtype,253
	mov	al,43
	call	commandonly
alreadyops:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	noops
	and	ax,1
	jz	noops
	call	dosaveload
noops:	ret
	endif

	endp











Dosaveload	proc	near

	mov	pointerframe,0
	mov	textaddressx,70
	mov	textaddressy,182-8
	mov	textlen,181
	mov	manisoffscreen,1
	call	clearwork
	call	createpanel2
	call	undertextline
	call	getridofall ;reels
	call	loadsavebox
	call	showopbox
	call	showmainops
	call	worktoscreen ;2
	jmp	donefirstops

restartops:	call	showopbox
	call	showmainops
	call	worktoscreenm
donefirstops:	mov	getback,0
waitops:	
	cmp quitrequested, 0
	jnz justret

	call	readmouse
	call	showpointer
	call	vsync
	call	dumppointer
	call	dumptextline
	call	delpointer
	mov	bx,offset cs:opslist
	call	checkcoords
	cmp	getback,0
	jz	waitops
	cmp	getback,2
	jz	restartops
	mov	textaddressx,13
	mov	textaddressy,182
	mov	textlen,240
	cmp	getback,4
	jz	justret
	call	getridoftemp
	call	restoreall ;reels
	call	redrawmainscrn
	call	worktoscreenm
	mov	commandtype,200
justret:	mov	manisoffscreen,0
	ret

opslist:	dw	opsx+59,opsx+114,opsy+30,opsy+76,getbackfromops
	dw	opsx+10,opsx+77,opsy+10,opsy+59,dosreturn
	dw	opsx+128,opsx+190,opsy+16,opsy+100,discops
	dw	0,320,0,200,blank
	dw	0ffffh


	endp



Getbackfromops	proc	near

	cmp	mandead,2
	jz	opsblock1
	call	getback1
	ret
opsblock1:	call	blank
	ret

	endp





Showmainops	proc	near

	mov	ds,tempgraphics
	mov	di,opsx+10
	mov	bx,opsy+10
	mov	al,8
	mov	ah,0
	call	showframe
	mov	ds,tempgraphics
	mov	di,opsx+59
	mov	bx,opsy+30
	mov	al,7
	mov	ah,0
	call	showframe
	mov	ds,tempgraphics
	mov	di,opsx+128+4
	mov	bx,opsy+12
	mov	al,1
	mov	ah,0
	call	showframe
	ret

	endp




Showdiscops	proc	near

	mov	ds,tempgraphics
	mov	di,opsx+128+4
	mov	bx,opsy+12
	mov	al,1
	mov	ah,0
	call	showframe
	mov	ds,tempgraphics
	mov	di,opsx+10
	mov	bx,opsy+10
	mov	al,9
	mov	ah,0
	call	showframe
	mov	ds,tempgraphics
	mov	di,opsx+59
	mov	bx,opsy+30
	mov	al,10
	mov	ah,0
	call	showframe
	mov	ds,tempgraphics
	mov	di,opsx+176+2
	mov	bx,opsy+60-4
	mov	al,5
	mov	ah,0
	call	showframe
	ret

	endp




Loadsavebox	proc	near

	mov	dx,offset cs:icongraphics8
	call	loadintotemp
	ret

	endp








Loadgame	proc	near

	cmp	commandtype,246
	jz	alreadyload
	mov	commandtype,246
	mov	al,41
	call	commandonly
alreadyload:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	noload
	cmp	ax,1
	jz	doload
noload:	ret

doload: 	mov	loadingorsave,1
	call	showopbox
	call	showloadops
	mov	currentslot,0
	call	showslots
	call	shownames
	mov	pointerframe,0
	call	worktoscreenm
	call	namestoold
	mov	getback,0

loadops:	
	cmp quitrequested, 0
	jnz quitloaded

	call	delpointer
	call	readmouse
	call	showpointer
	call	vsync
	call	dumppointer
	call	dumptextline

	mov	bx,offset cs:loadlist
	call	checkcoords
	cmp	getback,0
	jz	loadops
	cmp	getback,2
	jz	quitloaded
	call	getridoftemp
	;call	clearnoreels
	mov	dx,seg madeuproomdat
	mov	es,dx
	mov	bx,offset es:madeuproomdat
	call	startloading
	call	loadroomssample
	mov	roomloaded,1
	mov	newlocation,255
	call	clearsprites
	call	initman
	call	initrain
	mov	textaddressx,13
	mov	textaddressy,182
	mov	textlen,240
	call	startup
	call	worktoscreen
	mov	getback,4
quitloaded:	ret

loadlist:	dw	opsx+176,opsx+192,opsy+60,opsy+76,getbacktoops
	dw	opsx+128,opsx+190,opsy+12,opsy+100,actualload
	dw	opsx+2,opsx+92,opsy+4,opsy+81,selectslot
	dw	0,320,0,200,blank
	dw	0ffffh

	endp







Getbacktoops	proc	near

	cmp	commandtype,201
	jz	alreadygetops
	mov	commandtype,201
	mov	al,42
	call	commandonly
alreadygetops:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	nogetbackops
	and	ax,1
	jnz	dogetbackops
nogetbackops:	ret

dogetbackops:	call	oldtonames
	mov	getback,2
	ret

	endp







Discops	proc	near

	cmp	commandtype,249
	jz	alreadydiscops
	mov	commandtype,249
	mov	al,43
	call	commandonly
alreadydiscops:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	nodiscops
	and	ax,1
	jnz	dodiscops
nodiscops:	ret
dodiscops:	call	scanfornames
	mov	loadingorsave,2
	call	showopbox
	call	showdiscops
	mov	currentslot,0
	call	worktoscreenm

	mov	getback,0
discopsloop:
	cmp quitrequested, 0
	jnz quitdiscops

	call	delpointer
	call	readmouse
	call	showpointer
	call	vsync
	call	dumppointer
	call	dumptextline
	mov	bx,offset cs:discopslist
	call	checkcoords
	cmp	getback,0
	jz	discopsloop
quitdiscops:
	ret

discopslist:	dw	opsx+59,opsx+114,opsy+30,opsy+76,loadgame
	dw	opsx+10,opsx+79,opsy+10,opsy+59,savegame
	dw	opsx+176,opsx+192,opsy+60,opsy+76,getbacktoops
	dw	0,320,0,200,blank
	dw	0ffffh

	endp








Savegame	proc	near

	cmp	mandead,2
	jnz	cansaveok
	call	blank
	ret

cansaveok:	cmp	commandtype,247
	jz	alreadysave
	mov	commandtype,247
	mov	al,44
	call	commandonly
alreadysave:	mov	ax,mousebutton
	and	ax,1
	jnz	dosave
	ret
dosave: 	mov	loadingorsave,2
	call	showopbox
	call	showsaveops
	mov	currentslot,0
	call	showslots
	call	shownames
	call	worktoscreenm

	call	namestoold
	mov	bufferin,0
	mov	bufferout,0

	mov	getback,0

saveops:
	cmp quitrequested, 0
	jnz quitsavegame

	call	delpointer
	call	checkinput
	call	readmouse
	call	showpointer
	call	vsync
	call	dumppointer
	call	dumptextline

	mov	bx,offset cs:savelist
	call	checkcoords
	cmp	getback,0
	jz	saveops
quitsavegame:
	ret

savelist:	dw	opsx+176,opsx+192,opsy+60,opsy+76,getbacktoops
	dw	opsx+128,opsx+190,opsy+12,opsy+100,actualsave
	dw	opsx+2,opsx+92,opsy+4,opsy+81,selectslot
	dw	0,320,0,200,blank
	dw	0ffffh

	endp





Actualsave	proc	near

	cmp	commandtype,222
	jz	alreadyactsave
	mov	commandtype,222
	mov	al,44
	call	commandonly
alreadyactsave: 	mov	ax,mousebutton
	and	ax,1
	jz	noactsave

	mov	dx,seg savenames
	mov	ds,dx
	mov	si,offset savenames
	mov	al,currentslot
	mov	ah,0
	mov	cx,17
	mul	cx
	add	si,ax
	inc	si
	cmp	byte ptr [si],0
	jz	noactsave

	mov	al,location
	mov	ah,0
	mov	cx,32
	mul	cx
	push	cs
	pop	ds
	mov	si,offset cs:roomdata
	add	si,ax

	mov	di,offset cs:madeuproomdat
	mov	bx,di
	push	cs
	pop	es
	mov	cx,16
	rep	movsw

	mov	al,roomssample
	mov	[es:bx+13],al
	mov	al,mapx
	mov	[es:bx+15],al
	mov	al,mapy
	mov	[es:bx+16],al
	mov	al,liftflag
	mov	[es:bx+20],al
	mov	al,manspath
	mov	[es:bx+21],al
	mov	al,facing
	mov	[es:bx+22],al
	mov	al,255
	mov	[es:bx+27],al
	call	saveposition
	call	getridoftemp
	call	restoreall ;reels
	mov	textaddressx,13
	mov	textaddressy,182
	mov	textlen,240
	call	redrawmainscrn
	call	worktoscreenm
	mov	getback,4
noactsave:	ret

	endp




Actualload	proc	near

	cmp	commandtype,221
	jz	alreadyactload
	mov	commandtype,221
	mov	al,41
	call	commandonly
alreadyactload:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	notactload
	cmp	ax,1
	jnz	notactload

	mov	dx,seg savenames
	mov	ds,dx
	mov	si,offset savenames
	mov	al,currentslot
	mov	ah,0
	mov	cx,17
	mul	cx
	add	si,ax
	inc	si
	cmp	byte ptr [si],0
	jz	notactload
	call	loadposition
	mov	getback,1
notactload:	ret

	endp




Selectslot2	proc	near

	cmp	mousebutton,0
	jz	noselslot2
	mov	loadingorsave,2
noselslot2:	call	selectslot
	ret

	endp





Checkinput	proc	near

	cmp	loadingorsave,3
	jz	nokeypress
	call	readkey
	mov	al,currentkey
	cmp	al,0
	jz	nokeypress
	cmp	al,13
	jnz	notret
	mov	loadingorsave,3
	jmp	afterkey
notret: 	cmp	al,8
	jnz	nodel2
	cmp	cursorpos,0
	jz	nokeypress
	call	getnamepos
	dec	cursorpos
	mov	byte ptr [es:bx],0
	mov	byte ptr [es:bx+1],1
	jmp	afterkey
nodel2: 	;cmp	al,32
	;jz	spacepress
	;cmp	al,"A"
	;jc	nokeypress
	;cmp	al,"Z"+1
	;jnc	nokeypress
spacepress:	cmp	cursorpos,14
	jz	nokeypress
	call	getnamepos
	inc	cursorpos
	mov	al,currentkey
	mov	[es:bx+1],al
	mov	byte ptr [es:bx+2],0
	mov	byte ptr [es:bx+3],1
	jmp	afterkey

nokeypress:	ret

afterkey:	call	showopbox
	call	shownames
	call	showslots
	call	showsaveops
	call	worktoscreenm
	ret

	endp





Getnamepos	proc	near

	mov	al,currentslot
	mov	ah,0
	mov	cx,17
	mul	cx
	mov	dx,seg savenames
	mov	es,dx
	mov	bx,offset es:savenames
	add	bx,ax
	mov	al,cursorpos
	mov	ah,0
	add	bx,ax
	ret

	endp








Showopbox	proc	near

	mov	ds,tempgraphics
	mov	di,opsx
	mov	bx,opsy
	mov	al,0
	mov	ah,0
	call	showframe
	
	mov	ds,tempgraphics
	mov	di,opsx
	mov	bx,opsy+55
	mov	al,4
	mov	ah,0
	call	showframe
	ret

	endp








Showloadops	proc	near

	mov	ds,tempgraphics
	mov	di,opsx+128+4
	mov	bx,opsy+12
	mov	al,1
	mov	ah,0
	call	showframe
	mov	ds,tempgraphics
	mov	di,opsx+176+2
	mov	bx,opsy+60-4
	mov	al,5
	mov	ah,0
	call	showframe

	mov	di,opsx+104
	mov	bx,opsy+14
	mov	al,55
	mov	dl,101
	call	printmessage
	ret

	endp




Showsaveops	proc	near

	mov	ds,tempgraphics
	mov	di,opsx+128+4
	mov	bx,opsy+12
	mov	al,1
	mov	ah,0
	call	showframe
	mov	ds,tempgraphics
	mov	di,opsx+176+2
	mov	bx,opsy+60-4
	mov	al,5
	mov	ah,0
	call	showframe

	mov	di,opsx+104
	mov	bx,opsy+14
	mov	al,54
	mov	dl,101
	call	printmessage
	ret

	endp





Selectslot	proc	near

	cmp	commandtype,244
	jz	alreadysel
	mov	commandtype,244
	mov	al,45
	call	commandonly
alreadysel:	mov	ax,mousebutton
	cmp	ax,1
	jnz	noselslot
	cmp	ax,oldbutton
	jz	noselslot

	cmp	loadingorsave,3
	jnz	notnocurs
          	dec	loadingorsave
notnocurs:	call	oldtonames
	mov	ax,mousey
	sub	ax,opsy+4
	mov	cl,-1
getslotnum:	inc	cl
	sub	ax,11
	jnc	getslotnum
	mov	currentslot,cl
	call	delpointer
	call	showopbox
	call	showslots
	call	shownames
	cmp	loadingorsave,1
	jz	isloadmode
	call	showsaveops
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
	ret
isloadmode:	call	showloadops
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
	ret

noselslot:	ret

	endp










Showslots	proc	near

	mov	di,opsx+7
	mov	bx,opsy+8
	mov	al,2
	mov	ds,tempgraphics
	mov	ah,0
	call	showframe

	mov	di,opsx+10
	mov	bx,opsy+11
	mov	cl,0
slotloop:	push	cx di bx

	cmp	cl,currentslot
	jnz	nomatchslot
	mov	al,3
	mov	ds,tempgraphics
	mov	ah,0
	call	showframe

nomatchslot:	pop	bx di cx
	add	bx,10
	inc	cl
	cmp	cl,7
	jnz	slotloop
	ret

	endp





Shownames	proc	near

	mov	dx,seg savenames
	mov	es,dx
	mov	si,offset es:savenames+1
	mov	di,opsx+21
	mov	bx,opsy+10
	mov	cl,0

shownameloop:	push	cx di es bx si
	mov	al,4
	cmp	cl,currentslot
	jnz	nomatchslot2

	cmp	loadingorsave,2
	jnz	loadmode

	mov	dx,si
	mov	cx,15
	add	si,15
zerostill:	dec	si
	dec	cl
	cmp	byte ptr [es:si],1
	jnz	foundcharacter
	jmp	zerostill
foundcharacter:	mov	cursorpos,cl
	mov	byte ptr [es:si],"/"
	mov	byte ptr [es:si+1],0
	push	si
	mov	si,dx
	mov	dl,200
	mov	ah,0
	call	printdirect
	pop	si
	mov	byte ptr [es:si],0
	mov	byte ptr [es:si+1],1
	jmp	afterprintname

loadmode:	mov	al,0
	mov	dl,200
	mov	ah,0
	mov	charshift,91
	call	printdirect
	mov	charshift,0
	jmp	afterprintname

nomatchslot2:	mov	dl,200
	mov	ah,0
	call	printdirect

afterprintname:	pop	si bx es di cx
	add	si,17
	add	bx,10
	inc	cl
	cmp	cl,7
	jnz	shownameloop
	ret

	endp


















Dosreturn	proc	near

	cmp	commandtype,250
	jz	alreadydos
	mov	commandtype,250
	mov	al,46
	call	commandonly
alreadydos:	mov	ax,mousebutton
	and	ax,1
	jz	nodos

quickquit2:	call	soundend
	call	removeemm

quickquit:	if	recording
	call	saverec
	mov	bx,rechandle
	mov	ah,3eh
	int	21h
	endif
	if	playback
	mov	bx,rechandle
	mov	ah,3eh
	int	21h
	endif

	call	resetkeyboard
	mov	bl,31h
	mov	al,0
	mov	ah,12h
	int	10h
	call	vsync

	mov	ah,0
	mov	al,3
	int	10h
	call	error
	mov	ax,4c00h
	int	21h
	ret
      	
	endp



Error	proc	near
	
	cmp	gameerror,1
	jz	error1
	cmp	gameerror,2
	jz	error2
	cmp	gameerror,3
	jz	error3
	cmp	gameerror,4
	jz	error4
	cmp	gameerror,5
	jz	error5
	cmp	gameerror,6
	jz	error6
	cmp	gameerror,7
	jz	error7
	cmp	gameerror,8
	jz	error8
	ret
	
error1:	mov	dx,offset cs:gameerror1
	jmp	generalerror

error2:	mov	ax,soundbaseadd
	sub	ax,200h
	mov	cl,4
	shr	ax,cl
	add	al,"0"
	mov	bx,offset cs:error2patch
	mov	[cs:bx+1],al
	mov	dx,offset cs:gameerror2
	call	generalerror
	mov	dx,offset cs:gameinfo
	jmp	generalerror

error3:	mov	dx,offset cs:gameerror3
	jmp	generalerror

error4:	mov	dx,offset cs:gameerror4
	jmp	generalerror
	
error5:	mov	dx,offset cs:gameerror5
	jmp	generalerror
	
error6:	mov	al,soundint
	add	al,"0"
	mov	bx,offset cs:error6patch
	mov	[cs:bx],al
	mov	dx,offset cs:gameerror6
       	call	generalerror
	mov	dx,offset cs:gameinfo
	jmp	generalerror

error7:	mov	dx,offset cs:gameerror7
       	jmp	generalerror

error8:	mov	dx,offset cs:gameerror8
       	jmp	generalerror

generalerror:	mov	ah,9h
	push	cs
	pop	ds
	int	21h
	ret

nodos:	ret
	
gameerror1:	db	13,10,13,10
	db	"Dreamweb has an Error:",13,10
	db	"Unable to allocate Expanded Memory."
	db	13,10,13,10
	db	24h
gameerror2:	db	13,10,13,10
	db	"Dreamweb has an Error:",13,10
	db	"Sound Blaster card not found at address "
error2patch:	db	"220 Hex."
	db	13,10,13,10
	db	24h
gameerror3:	db	13,10,13,10
	db	"Dreamweb has an Error:",13,10
	db	"Out of Base Memory."
	db	13,10,13,10
	db	24h
gameerror4:	db	13,10,13,10
	db	"Dreamweb has an Error:",13,10
	db	"Memory Deallocation problem."
	db	13,10,13,10
	db	24h
gameerror5:	db	13,10,13,10
	db	"Dreamweb has an Error:",13,10
	db	"At least 590K of base memory is required."
	db	13,10,13,10
	db	24h
gameerror6:	db	13,10,13,10
	db	"Dreamweb has an Error:",13,10
	db	"Sound Blaster not found on interupt "
error6patch:	db	"0"
	db	13,10,13,10
	db	24h
gameerror7:	db	13,10,13,10
	db	"Dreamweb has an Error:",13,10
	db	"Unable to select EMM page."
	db	13,10,13,10
	db	24h
gameerror8:	db	13,10,13,10
	db	"Dreamweb has an Error:",13,10
	db	"File not found.c"
error8patch:	db	13,10,13,10
	db	24h

gameinfo:	db	"Dreamweb looks for Sound Blaster information in",13,10
	db	"the BLASTER environment variable (in your AUTOEXEC.BAT)",13,10
	db	13,10,"If this is not found then IRQ 7, DMA channel 1 and base",13,10
	db	"address 220h are assumed.",13,10,13,10

	db	"To alter any or all of these settings you can specify them",13,10
	db	"on the command line. For example:",13,10,13,10
	db	"Type    DREAMWEB I7 A220 D1    to run Dreamweb on IRQ 7, DMA",13,10
	db	"                               channel 1 and base address 220h"
	db	13,10
	db	"        DREAMWEB I5            to run Dreamweb on IRQ 5 and",13,10
	db	"                               default address of 220h, DMA 1",13,10
	db	13,10
	db	24h

endgametext1:	db	13,10,13,10
	db	"Try the Dreamweb CD in your stereo....",13,10
	db	13,10,13,10
	db	24h

	endp








Namestoold	proc	near

	push	cs
	pop	ds
	mov	si,offset cs:savenames
	mov	di,zoomspace
	mov	es,buffers
	mov	cx,17*4
	rep	movsb
	ret

	endp



Oldtonames	proc	near

	push	cs
	pop	es
	mov	di,offset cs:savenames
	mov	si,zoomspace
	mov	ds,buffers
	mov	cx,17*4
	rep	movsb
	ret

	endp



Savefilewrite	proc	near
	mov	bx,handle
	mov	ah,40h
	int	21h
	ret
	endp

Savefileread	proc	near
	mov	bx,handle
	mov	ah,3fh
	int	21h
	ret
	endp

Saveposition	proc	near

	call	makeheader

	mov	al,currentslot
	mov	ah,0
	push	ax
	mov	cx,13
	mul	cx
	mov	dx,seg savefiles
	mov	ds,dx
	mov	dx,offset savefiles
	add	dx,ax
	call	openforsave

	mov	dx,seg fileheader
	mov	ds,dx
	mov	dx,offset fileheader
	mov	cx,headerlen
	call savefilewrite
	mov	dx,seg fileheader
	mov	es,dx
	mov	di,offset es:filedata

	pop	ax
	mov	cx,17
	mul	cx
	mov	dx,seg savenames
	mov	ds,dx
	mov	dx,offset savenames
	add	dx,ax
	call	saveseg

	mov	dx,seg startvars
	mov	ds,dx
	mov	dx,offset startvars
	call	saveseg
	
	mov	ds,extras
	mov	dx,exframedata
	call	saveseg

	mov	ds,buffers
	mov	dx,listofchanges
	call	saveseg

	mov	dx,seg madeuproomdat
	mov	ds,dx
	mov	dx,offset madeuproomdat
	call	saveseg

	mov	dx,seg reelroutines
	mov	ds,dx
	mov	dx,offset reelroutines
	call	saveseg

fquit:	call	closefile
	ret

	endp






Loadposition	proc	near

	mov	timecount,0
	call	clearchanges

	mov	al,currentslot
	mov	ah,0
	push	ax
	mov	cx,13
	mul	cx
	mov	dx,seg savefiles
	mov	ds,dx
	mov	dx,offset savefiles
	add	dx,ax
	if	cd
	call	openfilefromc
	else
	call	openfile
	endif

	push	cs
	pop	ds
	mov	dx,offset cs:fileheader
	mov	cx,headerlen
	call savefileread
	push	cs
	pop	es
	mov	di,offset cs:filedata
	
	pop	ax
	mov	cx,17
	mul	cx
	mov	dx,seg savenames
	mov	ds,dx
	mov	dx,offset savenames
	add	dx,ax
	call	loadseg
	
	mov	dx,seg startvars
	mov	ds,dx
	mov	dx,offset startvars
	call	loadseg

	mov	ds,extras
	mov	dx,exframedata
	call	loadseg

	mov	ds,buffers
	mov	dx,listofchanges
	call	loadseg

	mov	dx,seg madeuproomdat
	mov	ds,dx
	mov	dx,offset madeuproomdat
	call	loadseg

	push	cs
	pop	ds
	mov	dx,offset cs:reelroutines
	call	loadseg

	call	closefile
	ret

	endp





Loadseg 	proc	near

	mov	bx,handle
	mov	ax,[es:di]
	add	di,2
	push	di
	push	es
	mov	cx,ax
	mov	ah,3fh
	int	21h
	pop	es
	pop	di
	ret

	endp





Makeheader	proc	near

	mov	dx,seg fileheader
	mov	es,dx
	mov	di,offset es:filedata
	mov	ax,17
	call	storeit
	mov	ax,lengthofvars
	call	storeit
	mov	ax,lengthofextra
	call	storeit
	mov	ax,numchanges*4
	call	storeit
	mov	ax,48
	call	storeit
	mov	ax,lenofreelrouts
	call	storeit
	ret

	endp





Storeit 	proc near

	cmp	ax,0
	jnz	isntblank
	inc	ax
isntblank:	stosw
	ret

	endp




Saveseg 	proc	near

	mov	cx,[es:di]
	add	di,2
	push	di es
	mov	bx,handle
	mov	ah,40h
	int	21h
	pop	es di 
	ret

	endp



Findlen 	proc	near

	dec	bx
	add	bx,ax
nextone:	cmp	cl,[bx]
	jnz	foundlen
	dec	bx
	dec	ax
	cmp	ax,0
	jnz	nextone
foundlen:	ret

	endp





Scanfornames	proc	near

	mov	dx,seg savenames
	mov	es,dx
	mov	di,offset es:savenames
	mov	dx,seg savefiles
	mov	ds,dx
	mov	dx,offset savefiles
	mov	cx,7
scanloop:	push	es ds di dx cx

	if	cd
	call	openfilefromc
	else
	call	openfilenocheck
	endif
	jc	notexist
	pop	cx
	inc	ch
	push	cx
	push	di es
	mov	dx,seg fileheader
	mov	ds,dx
	mov	dx,offset fileheader
	mov	cx,headerlen
	call savefileread
	mov	dx,seg fileheader
	mov	es,dx
	mov	di,offset es:filedata
	pop	ds dx
	call	loadseg
	mov	bx,handle
	call	closefile

notexist:	pop	cx dx di ds es
	add	dx,13
	add	di,17
	dec	cl
	jnz	scanloop
	mov	al,ch
	ret

	endp





Decide	proc	near

	call	setmode
	call	loadpalfromiff
	call	clearpalette	
	mov	pointermode,0
	mov	watchingtime,0
	mov	pointerframe,0
	mov	textaddressx,70
	mov	textaddressy,182-8
	mov	textlen,181
	mov	manisoffscreen,1
	call	loadsavebox
	call	showdecisions
	call	worktoscreen
	call	fadescreenup
	mov	getback,0

waitdecide:	
	cmp quitrequested, 0
	jz $1
	ret
$1:
	call	readmouse
	call	showpointer
	call	vsync
	call	dumppointer
	call	dumptextline
	call	delpointer
	mov	bx,offset cs:decidelist
	call	checkcoords
	cmp	getback,0
	jz	waitdecide
	cmp	getback,4
	jz	hasloadedroom
	call	getridoftemp
hasloadedroom:	mov	textaddressx,13
	mov	textaddressy,182
	mov	textlen,240
	ret

decidelist:	dw	opsx+69,opsx+124,opsy+30,opsy+76,newgame
	dw	opsx+20,opsx+87,opsy+10,opsy+59,dosreturn
	dw	opsx+123,opsx+190,opsy+10,opsy+59,loadold
	dw	0,320,0,200,blank
	dw	0ffffh

	endp






Showdecisions	proc	near

	call	createpanel2
	call	showopbox
	mov	ds,tempgraphics
	mov	di,opsx+17
	mov	bx,opsy+13
	mov	al,6
	mov	ah,0
	call	showframe
	call	undertextline
	ret

	endp





Newgame	proc	near

	cmp	commandtype,251
	jz	alreadynewgame
	mov	commandtype,251
	mov	al,47
	call	commandonly
alreadynewgame:	mov	ax,mousebutton
	cmp	ax,1
	jnz	nonewgame
	mov	getback,3
nonewgame:	ret

	endp






	
Loadold	proc	near

	cmp	commandtype,252
	jz	alreadyloadold
	mov	commandtype,252
	mov	al,48
	call	commandonly
alreadyloadold:	mov	ax,mousebutton
	and	ax,1
	jz	noloadold
	call	doload
	cmp	getback,4
	jz	noloadold
	cmp quitrequested, 0
	jnz noloadold
	call	showdecisions
	call	worktoscreenm
	mov	getback,0
noloadold:	ret

	endp
