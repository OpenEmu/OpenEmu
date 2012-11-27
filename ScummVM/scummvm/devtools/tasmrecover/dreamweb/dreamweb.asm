;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text









;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³			              ³
;³	DREAMWEB	              ³
;³			              ³
;³			              ³
;³			              ³
;³			              ³
;³	Written by Neil Dodwell. Graphics by Dave Dew.    ³
;³			              ³
;³	Started on Friday 28 December 1990 at 1:20 pm     ³
;³			              ³
;³	Copyright 1990/1991 Creative Reality              ³	³
;³			              ³
;³			              ³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ















;----------------------------------------------------------Assembly options----

JUMPS

playback	equ	0
recording	equ	0
debuglevel1	equ	0	;memory debug on
debuglevel2	equ	0	;debug keys on+shouts
demo	equ	0
CD	equ	1
Foreign	equ	1
Spanish	equ	0
German	equ	0

;----------------------------------------------------------------Code start----

DREAMWEBPROG	segment para public 'CODE'

	assume	cs:DREAMWEBPROG,ss:STACKSPACE




;------------------------------------------------------------------Includes----

include \pc\dreamweb\vars.asm	;variables and equates
include \pc\dreamweb\sprite.asm	;sprite routines
include \pc\dreamweb\vgagrafx.asm	;screen routines for vga
include \pc\dreamweb\vgafades.asm	;fade routines
include \pc\dreamweb\titles.asm	;guess!
include \pc\dreamweb\print.asm	;text printing routines
include \pc\dreamweb\object.asm	;pickup
include \pc\dreamweb\backdrop.asm	;draws floor etc.
include \pc\dreamweb\look.asm	;look command
include \pc\dreamweb\talk.asm	;conversations
include \pc\dreamweb\newplace.asm	;travel
include \pc\dreamweb\monitor.asm	;network machine
include \pc\dreamweb\use.asm	;individual use routines
include \pc\dreamweb\keypad.asm	;accept code with keypad
include \pc\dreamweb\saveload.asm	;in game options
include \pc\dreamweb\sblaster.asm
	if	debuglevel1
include \pc\dreamweb\debug.asm
	else
	if	debuglevel2
include \pc\dreamweb\debug.asm
	endif
	endif

;-----------------------------------------------------------------Main loop----


Dreamweb	proc	near

	call	seecommandtail

	call	checkbasemem
	call	soundstartup
	call	setkeyboardint
	call	setupemm
	call	allocatebuffers
	call	setmouse
	call	fadedos
	call	gettime

	call	clearbuffers
	call	clearpalette
	call	set16colpalette
	call	readsetdata
	if	demo
	call	changeroomnums
	endif
	mov	wongame,0

	mov	dx,offset cs:basicsample
	call	loadsample
	call	setsoundoff

	if	demo
	else
	call	scanfornames
           	cmp	al,0
	jnz	dodecisions
	endif
	
	call	setmode
	call	loadpalfromiff

	call	titles
	call	credits
	jmp	playgame

dodecisions:	call	cls
	call	setmode
	call	decide

	cmp quitrequested, 0
	jnz exitgame

	cmp	getback,4
	jz	mainloop

	call	titles

	cmp quitrequested, 0
	jnz exitgame

	call	credits

playgame:
	cmp quitrequested, 0
	jnz exitgame
	call	clearchanges
	call	setmode
	call	loadpalfromiff
	mov	location,255
	mov	roomafterdream,1
	if	demo
	mov	newlocation,5
	else
	mov	newlocation,35
	endif
	mov	volume,7
	call	loadroom
	call	clearsprites
	call	initman
	call	entrytexts
	call	entryanims
	mov	destpos,3
	call	initialinv
	mov	lastflag,32
	call	startup1
	mov	volumeto,0
	mov	volumedirection,-1
	mov	commandtype,255
	jmp	mainloop

loadnew:	if	demo
	cmp	newlocation,27
	jnz	not27
      	call	fadescreendowns
	mov	cx,260
	call	hangon
	call	clearbeforeload
	jmp	playgame
not27:
	endif
	call	clearbeforeload
	call	loadroom
	call	clearsprites
	call	initman
	call	entrytexts
	call	entryanims
	mov	newlocation,255
	call	startup
	mov	commandtype,255
	call	worktoscreenm
	jmp	mainloop

alreadyloaded:	mov	newlocation,255
	call	clearsprites
	call	initman
	call	startup
	mov	commandtype,255

mainloop:
	cmp quitrequested, 0
	jnz exitgame

	call	screenupdate
	cmp	wongame,0
	jnz	endofgame
	cmp	mandead,1
	jz	gameover
	cmp	mandead,2
	jz	gameover
	cmp	watchingtime,0
	jz	notwatching
	mov	al,finaldest
	cmp	al,manspath
	jnz	mainloop
	dec	watchingtime
	jnz	mainloop

notwatching:	cmp	mandead,4
	jz	gameover
	cmp	newlocation,255
	jnz	loadnew
	jmp	mainloop

gameover:	if	demo
      	call	fadescreendowns
	mov	cx,260
	call	hangon
	call	clearbeforeload
	jmp	playgame
	endif
	call	clearbeforeload
	call	showgun
	call	fadescreendown
	mov	cx,100
	call	hangon
	jmp	dodecisions

endofgame:	call	clearbeforeload
	call	fadescreendowns
	mov	cx,200
	call	hangon
	call	endgame
	jmp	quickquit2

exitgame:
	ret
	endp




	endp



	if	demo

Changeroomnums	proc	near
	
	mov	di,offset cs:roomdata+10
	mov	cx,50
changenumloop:	mov	al,[cs:di]
	cmp	al,"0"
	jnz	nochange
	mov	al,[cs:di+1]
	cmp	al,"5"
	jnz	nochange
	mov	al,"6"
	mov	ah,"0"
	mov	[cs:di],ax
nochange: 	add	di,32
	loop	changenumloop
	ret

	endp

	endif




Entrytexts	proc	near

	cmp	location,21
	jnz	notloc15
	mov	al,28
	mov	cx,60
	mov	dx,11
	mov	bl,68
	mov	bh,64
	call	setuptimeduse
	ret
notloc15:	cmp	location,30
	jnz	notloc43
	mov	al,27
	mov	cx,60
	mov	dx,11
	mov	bl,68
	mov	bh,64
	call	setuptimeduse
	ret
notloc43:	cmp	location,23
	jnz	notloc23
	mov	al,29
	mov	cx,60
	mov	dx,11
	mov	bl,68
	mov	bh,64
	call	setuptimeduse
	ret
notloc23:	cmp	location,31
	jnz	notloc44
	mov	al,30
	mov	cx,60
	mov	dx,11
	mov	bl,68
	mov	bh,64
	call	setuptimeduse
	ret
notloc44:	cmp	location,20
	jnz	notsarters2
	mov	al,31
	mov	cx,60
	mov	dx,11
	mov	bl,68
	mov	bh,64
	call	setuptimeduse
	ret
notsarters2:	cmp	location,24
	jnz	notedenlob
	mov	al,32
	mov	cx,60
	mov	dx,3
	mov	bl,68
	mov	bh,64
	call	setuptimeduse
	ret
notedenlob:	cmp	location,34
	jnz	noteden2
	mov	al,33
	mov	cx,60
	mov	dx,3
	mov	bl,68
	mov	bh,64
	call	setuptimeduse
	ret
noteden2:	ret

	endp








Entryanims	proc	near

	mov	reeltowatch,-1
	mov	watchmode,-1
	cmp	location,33
	jnz	notinthebeach
	call	switchryanoff
	mov	watchingtime,76*2
	mov	reeltowatch,0
	mov	endwatchreel,76
	mov	watchspeed,1
	mov	speedcount,1
	ret
notinthebeach:	cmp	location,44
	jnz	notsparkys
	mov	al,8
	call	resetlocation
	mov	watchingtime,50*2
	mov	reeltowatch,247
	mov	endwatchreel,297
	mov	watchspeed,1
	mov	speedcount,1
	call	switchryanoff
	ret
notsparkys:	cmp	location,22
	jnz	notinthelift
	mov	watchingtime,31*2
	mov	reeltowatch,0
	mov	endwatchreel,30
	mov	watchspeed,1
	mov	speedcount,1
	call	switchryanoff
	ret
notinthelift:	cmp	location,26
	jnz	notunderchurch
	mov	symboltopnum,2
	mov	symbolbotnum,1
	ret
notunderchurch:	cmp	location,45
	jnz	notenterdream
	mov	keeperflag,0
	mov	watchingtime,296
	mov	reeltowatch,45
	mov	endwatchreel,198
	mov	watchspeed,1
	mov	speedcount,1
	call	switchryanoff
	ret
notenterdream:	cmp	reallocation,46
	jnz	notcrystal
	cmp	sartaindead,1
	jnz	notcrystal
           	mov	al,0
	call	removefreeobject
	ret
notcrystal:	cmp	location,9
	jnz	nottopchurch
	mov	al,2
	call	checkifpathison
	jz	nottopchurch
	cmp	aidedead,0
	jz	nottopchurch
	mov	al,3
	call	checkifpathison
	jnz	makedoorsopen
	mov	al,2
	call	turnpathon
makedoorsopen:	mov	al,4
	call	removesetobject
	mov	al,5
	call	placesetobject
	ret
nottopchurch:	cmp	location,47
	jnz	notdreamcentre	
	mov	al,4
	call	placesetobject
	mov	al,5
	call	placesetobject
	ret
notdreamcentre:	cmp	location,38
	jnz	notcarpark
	mov	watchingtime,57*2
	mov	reeltowatch,4
	mov	endwatchreel,57
	mov	watchspeed,1
	mov	speedcount,1
	call	switchryanoff
	ret
notcarpark:	cmp	location,32
	jnz	notalley
	mov	watchingtime,66*2
	mov	reeltowatch,0
	mov	endwatchreel,66
	mov	watchspeed,1
	mov	speedcount,1
	call	switchryanoff
	ret
notalley:	cmp	location,24
	jnz	notedensagain
	mov	al,2
	mov	ah,roomnum
	dec	ah
	call	turnanypathon
notedensagain:	ret

	endp






	if	demo
Initialinv	proc	near

	mov	al,11
	mov	ah,5
	call	pickupob
	mov	al,12
	mov	ah,6
	call	pickupob
	mov	al,13
	mov	ah,7
	call	pickupob
	mov	al,14
	mov	ah,8
	call	pickupob
	mov	al,18
	mov	ah,0
	call	pickupob
	mov	al,19
	mov	ah,1
	call	pickupob
	mov	al,20
	mov	ah,9
	call	pickupob
	mov	al,16
	mov	ah,2
	call	pickupob
	
	mov	al,2
	mov	ah,4
	call	pickupob

	mov	al,29
	mov	ah,10
	call	pickupob
	mov	al,33
	mov	ah,11
	call	pickupob
	mov	al,44
	mov	ah,12
	call	pickupob
	mov	card1money,12342
	ret

	endp
	else
Initialinv	proc	near

	cmp	reallocation,24
	jz	isedens
	ret

isedens:	mov	al,11
	mov	ah,5
	call	pickupob
	mov	al,12
	mov	ah,6
	call	pickupob
	mov	al,13
	mov	ah,7
	call	pickupob
	mov	al,14
	mov	ah,8
	call	pickupob
	mov	al,18
	mov	al,18
	mov	ah,0
	call	pickupob
	mov	al,19
	mov	ah,1
	call	pickupob
	mov	al,20
	mov	ah,9
	call	pickupob
	mov	al,16
	mov	ah,2
	call	pickupob

	mov	watchmode,1
	mov	reeltohold,0
	mov	endofholdreel,6
	mov	watchspeed,1
	mov	speedcount,1
	call	switchryanoff
	ret

	endp

	endif 











Pickupob	proc	near

	mov	lastinvpos,ah
	mov	objecttype,2
	mov	itemframe,al
	mov	command,al
	call	getanyad
	call	transfertoex
	ret

	endp















;---------------------------------------------------------Memory allocation----




Setupemm	proc	near
	
	cmp	soundint,255
	jz	noneedforemm
	call	checkforemm
	
	mov	ah,43h	;allocate handle and 160 pages
	mov	bx,176 	;was 176	                      
	int	67h
	cmp	ah,0
	jnz	emmerror1	;if there's an error drop to DOS
	mov	emmhandle,dx
	mov	ah,41h	;get the page frame base address
	int	67h
	cmp	ah,0
	jnz	emmerror1
	mov	emmpageframe,bx
 	mov	ax,bx
	mov	cl,12
	shr	ax,cl
	mov	emmhardwarepage,al
noneedforemm:	ret
emmerror1:	mov	gameerror,1
	jmp	quickquit2

	endp








Removeemm	proc	near
             
	cmp	soundint,255
	jz	noneedtoremove
	mov	ah,45h
	mov	dx,emmhandle
	int	67h
noneedtoremove:	ret

	endp





Checkforemm	proc	near

	ret

	endp




Checkbasemem	proc	near

	mov	bx,howmuchalloc
	cmp	bx,9360h
	jnc	enoughmem
          	mov	gameerror,5
	jmp	quickquit
enoughmem:	ret

	endp



Allocatebuffers	proc	near
	
	mov	bx,lengthofextra/16
	call	allocatemem
	mov	extras,ax

	call	trysoundalloc
	mov	bx,lengthofmap/16
	call	allocatemem
	mov	mapdata,ax

	call	trysoundalloc
	mov	bx,lengthofbuffer/16
	call	allocatemem
	mov	buffers,ax

	call	trysoundalloc
	mov	bx,freedatlen/16
	call	allocatemem
	mov	freedat,ax

	call	trysoundalloc
	mov	bx,setdatlen/16
	call	allocatemem
	mov	setdat,ax

	call	trysoundalloc
	mov	bx,lenofmapstore/16
	call	allocatemem
	mov	mapstore,ax

	if	recording
	mov	bx,1028
	call	allocatemem
	mov	recordspace,ax
	endif

	if	playback
	mov	bx,1028
	call	allocatemem
	mov	recordspace,ax
	endif

	call	allocatework
	
	mov	bx,2048/16
	call	allocatemem
	mov	sounddata,ax

	mov	bx,2048/16
	call	allocatemem
	mov	sounddata2,ax
	ret

	endp










Clearbuffers	proc	near

	mov	es,buffers
	mov	cx,lengthofbuffer/2
	mov	ax,0
	mov	di,0
	rep	stosw

	mov	es,extras
	mov	cx,lengthofextra/2
	mov	ax,0ffffh
	mov	di,0
	rep	stosw

	mov	es,buffers
	mov	di,initialreelrouts
	push	cs
	pop	ds
	mov	si,offset cs:reelroutines
	mov	cx,lenofreelrouts
	rep	movsb
	
	mov	es,buffers
	mov	di,initialvars
	push	cs
	pop	ds
	mov	si,offset cs:startvars
	mov	cx,lengthofvars
	rep	movsb
	call	clearchanges
	ret

	endp






Clearchanges	proc	near

	mov	es,buffers
	mov	cx,numchanges*2
	mov	ax,0ffffh
	mov	di,listofchanges
	rep	stosw
	mov	ds,buffers
	mov	si,initialreelrouts
	push	cs
	pop	es
	mov	di,offset cs:reelroutines
	mov	cx,lenofreelrouts
	rep	movsb

	mov	ds,buffers
	mov	si,initialvars
	push	cs
	pop	es
	mov	di,offset cs:startvars
	mov	cx,lengthofvars
	rep	movsb
	
	mov	expos,0
	mov	exframepos,0
	mov	extextpos,0
	mov	es,extras
	mov	cx,lengthofextra/2
	mov	ax,0ffffh
	mov	di,0
	rep	stosw

	push	cs
	pop	es
	mov	di,offset cs:roomscango
	mov	al,1
	stosb
	stosb
	mov	al,0
	stosb
	mov	al,1
	stosb
	mov	ax,0
	mov	cx,6
	rep	stosw
	ret

	endp







Clearbeforeload	proc	near	;deallocates variable buffers
			;and clears out fixed ones
	cmp	roomloaded,1
	jnz	noclear
	call	clearreels
	call	clearrest
	mov	roomloaded,0
noclear:	ret

	endp



;Clearnoreels	proc	near
;
;	cmp	roomloaded,1
;	jnz	noclear2
;	call	clearrest
;	mov	roomloaded,0
;noclear2:	ret
;
;	endp






Clearreels	proc	near

	mov	es,reel1
	call	deallocatemem
	mov	es,reel2
	call	deallocatemem
	mov	es,reel3
	call	deallocatemem
	ret

	endp



Clearrest	proc	near

	mov	es,mapdata
	mov	cx,maplen/2
	mov	ax,0
	mov	di,map
	rep	stosw

	mov	es,backdrop
	call	deallocatemem
	mov	es,setframes
	call	deallocatemem
	mov	es,reels
	call	deallocatemem
	mov	es,people
	call	deallocatemem
	mov	es,setdesc
	call	deallocatemem
	mov	es,blockdesc
	call	deallocatemem
	mov	es,roomdesc
	call	deallocatemem
	mov	es,freeframes
	call	deallocatemem
	mov	es,freedesc
	call	deallocatemem
	ret

	endp






Deallocatemem	proc	near

	mov	ah,49h
	int	21h
	jc	deallerror
	ret
deallerror:	mov	gameerror,4
	jmp	quickquit2
	ret

	endp








Allocatemem	proc	near

	add	bx,2
	mov	ah,48h
	int	21h
	jc	memerror
	ret

memerror:	mov	gameerror,3
	jmp	quickquit2

	endp




Seecommandtail	proc	near

	mov	soundbaseadd,220h
	mov	soundint,5
	mov	sounddmachannel,1
	mov	brightness,0
	
	mov	bx,2
	mov	ax,[es:bx]
	mov	dx,es
	sub	ax,dx
	mov	howmuchalloc,ax
	
	mov	bx,02ch
	mov	ax,[es:bx]
	push	es bx
	
	mov	es,ax
	mov	bx,0
findblaster:	mov	ax,[es:bx]
	cmp	ax,0
	jz	endofenvironment
	cmp	al,"B"
	jnz	notblast
	cmp	ah,"L"
	jnz	notblast
	cmp	byte ptr [es:bx+2],"A"
	jnz	notblast
	cmp	byte ptr [es:bx+3],"S"
	jnz	notblast
	cmp	byte ptr [es:bx+4],"T"
	jnz	notblast
	cmp	byte ptr [es:bx+5],"E"
	jnz	notblast
	cmp	byte ptr [es:bx+6],"R"
	jnz	notblast
	add	bx,7
	call	parseblaster
	jmp	endofenvironment
notblast:	inc	bx
	jmp	findblaster
	
endofenvironment:	pop	bx es
	mov	bx,81h
	call	parseblaster
	ret

	endp



Parseblaster	proc	near
	
lookattail:	mov	al,[es:bx]
	cmp	al,0
	jz	endtail
	cmp	al,13
	jz	endtail
	cmp	al,"i"
	jz	issoundint
	cmp	al,"I"
	jz	issoundint
	cmp	al,"b"
	jz	isbright
	cmp	al,"B"
	jz	isbright
	cmp	al,"a"
	jz	isbaseadd
	cmp	al,"A"
	jz	isbaseadd
	cmp	al,"n"
	jz	isnosound
	cmp	al,"N"
	jz	isnosound
	cmp	al,"d"
	jz	isdma
	cmp	al,"D"
	jz	isdma
	inc	bx
	loop	lookattail
	ret

issoundint:	mov	al,[es:bx+1]
	sub	al,"0"
	mov	soundint,al
	inc	bx
	jmp	lookattail
isdma:	mov	al,[es:bx+1]
	sub	al,"0"
	mov	sounddmachannel,al
	inc	bx
	jmp	lookattail
isbaseadd:	push 	cx
	mov	al,[es:bx+2]
	sub	al,"0"
	mov	ah,0
	mov	cl,4
	shl	ax,cl
	add	ax,200h
	mov	soundbaseadd,ax
	pop	cx
	inc	bx
	jmp	lookattail
isbright:	mov	brightness,1
	inc	bx
	jmp	lookattail
isnosound:	mov	soundint,255
	inc	bx
	jmp	lookattail
endtail: 	ret

	endp




;-------------------------------------------------------High level routines----

Startup	proc	near

	mov	currentkey,0
	mov	mainmode,0
	call	createpanel
	mov	newobs,1
	call	drawfloor
	call	showicon
	call	getunderzoom
	call	spriteupdate
	call	printsprites
	call	undertextline
	call	reelsonscreen
	call	atmospheres
	ret

	endp




Startup1	proc	near


	call	clearpalette
	mov	throughdoor,0
	mov	currentkey,"0"
	mov	mainmode,0
	call	createpanel
	mov	newobs,1
	call	drawfloor

	call	showicon
	call	getunderzoom
	call	spriteupdate
	call	printsprites
	call	undertextline
	call	reelsonscreen
	call	atmospheres
	call	worktoscreen
	call	fadescreenup
	ret

	endp








;--------------------------------------------------Scroll location routines----





Screenupdate	proc	near

	call	newplace
	call	mainscreen
	cmp quitrequested, 0
	jnz finishearly
	call	animpointer
	call	showpointer
	cmp	watchingtime,0
	jnz	iswatchingmode
	cmp	newlocation,255
	jnz	finishearly
iswatchingmode:	call	vsync
	call	readmouse1
	call	dumppointer
	call	dumptextline
	call	delpointer
	call	autolook
	call	spriteupdate
	call	watchcount
	call	zoom
	call	showpointer
	cmp	wongame,0
	jnz	finishearly

	call	vsync
	call	readmouse2
	call	dumppointer
	call	dumpzoom
	call	delpointer
	call	deleverything
	call	printsprites
	call	reelsonscreen
	call	afternewroom
	call	showpointer

	call	vsync
	call	readmouse3
	call	dumppointer
	call	dumpmap
	call	dumptimedtext
	call	delpointer
	call	showpointer

	call	vsync
	call	readmouse4
	call	dumppointer
	call	dumpwatch
	call	delpointer

finishearly:	ret

	endp















Watchreel	proc	near

	cmp	reeltowatch,-1
	jz	notplayingreel
	mov	al,manspath
	cmp	al,finaldest
	jnz	waitstopwalk
	mov	al,turntoface
	cmp	al,facing
	jz	notwatchpath
waitstopwalk:	ret

notwatchpath:	dec	speedcount
	cmp	speedcount,-1
	jnz	showwatchreel
	mov	al,watchspeed
	mov	speedcount,al
	mov	ax,reeltowatch
	cmp	ax,endwatchreel
	jnz	ismorereel
	cmp	watchingtime,0
	jnz	showwatchreel
	mov	reeltowatch,-1
	mov	watchmode,-1
	cmp	reeltohold,-1
	jz	nomorereel
	mov	watchmode,1
	jmp	notplayingreel
ismorereel:	inc	reeltowatch
showwatchreel:	mov	ax,reeltowatch
	mov	reelpointer,ax
	call	plotreel
	mov	ax,reelpointer
	mov	reeltowatch,ax
	call	checkforshake
nomorereel:	ret


notplayingreel: cmp	watchmode,1
	jnz	notholdingreel
	mov	ax,reeltohold
	mov	reelpointer,ax
	call	plotreel
	ret

notholdingreel: cmp	watchmode,2
	jnz	notreleasehold
	dec	speedcount
	cmp	speedcount,-1
	jnz	notlastspeed2
	mov	al,watchspeed
	mov	speedcount,al
	inc	reeltohold
notlastspeed2:	mov	ax,reeltohold
	cmp	ax,endofholdreel
	jnz	ismorereel2
	mov	reeltohold,-1
	mov	watchmode,-1
	mov	al,destafterhold
	mov	destination,al
	mov	finaldest,al
	call	autosetwalk
	ret
ismorereel2:	mov	ax,reeltohold
	mov	reelpointer,ax
	call	plotreel
	ret

notreleasehold:	ret

	endp





Checkforshake	proc	near

	cmp	reallocation,26
	jnz	notstartshake
          	cmp	ax,104
	jnz	notstartshake
	mov	shakecounter,-1
notstartshake:	ret

	endp





Watchcount	proc	near

	cmp	watchon,0
	jz	nowatchworn
	inc	timercount
	cmp	timercount,9
	jz	flashdots
	cmp	timercount,18
	jz	uptime
nowatchworn:	ret

flashdots:	mov	ax,91*3+21
	mov	di,268+4
	mov	bx,21
	mov	ds,charset1
	call	showframe
	jmp	finishwatch

uptime: 	mov	timercount,0
	add	secondcount,1
	cmp	secondcount,60
	jnz	finishtime
	mov	secondcount,0
	inc	minutecount
	cmp	minutecount,60
	jnz	finishtime
	mov	minutecount,0
	inc	hourcount
	cmp	hourcount,24
	jnz	finishtime
	mov	hourcount,0

finishtime:	call	showtime
finishwatch:	mov	watchdump,1
	ret

	endp



Showtime	proc	near

	cmp	watchon,0
	jz	nowatch

	mov	al,secondcount
	mov	cl,0 
	call	twodigitnum
	push	ax
	mov	al,ah
	mov	ah,0
	add	ax,91*3+10
	mov	ds,charset1
	mov	di,282+5
	mov	bx,21
	call	showframe
	pop	ax
	mov	ah,0
	add	ax,91*3+10
	mov	ds,charset1
	mov	di,282+9
	mov	bx,21
	call	showframe

	mov	al,minutecount
	mov	cl,0
	call	twodigitnum
	push	ax
	mov	al,ah
	mov	ah,0
	add	ax,91*3
	mov	ds,charset1
	mov	di,270+5
	mov	bx,21
	call	showframe
	pop	ax
	mov	ah,0
	add	ax,91*3
	mov	ds,charset1
	mov	di,270+11
	mov	bx,21
	call	showframe

	mov	al,hourcount
	mov	cl,0
	call	twodigitnum
	push	ax
	mov	al,ah
	mov	ah,0   
	add	ax,91*3
	mov	ds,charset1
	mov	di,256+5
	mov	bx,21
	call	showframe
	pop	ax
	mov	ah,0
	add	ax,91*3
	mov	ds,charset1
	mov	di,256+11
	mov	bx,21
	call	showframe

	mov	ax,91*3+20
	mov	ds,charset1
	mov	di,267+5
	mov	bx,21
	call	showframe
nowatch:	ret

	
	endp




Dumpwatch	proc	near

	cmp	watchdump,1
	jnz	nodumpwatch
	mov	di,256
	mov	bx,21
	mov	cl,40
	mov	ch,12
	call	multidump
	mov	watchdump,0
nodumpwatch:	ret

	endp




Showbyte	proc	near

	mov	dl,al
	shr	dl,1
	shr	dl,1
	shr	dl,1
	shr	dl,1
	call	onedigit
	mov	[es:di],dl
	mov	dl,al
	and	dl,15
	call	onedigit
	mov	[es:di+1],dl
	add	di,3
	ret

	endp


Onedigit	proc	near

	cmp	dl,10
	jnc	morethan10
	add	dl,"0"
	ret
morethan10:	sub	dl,10
	add	dl,"A"
	ret

	endp





Twodigitnum 	proc	near

	mov	ah,cl
	dec	ah
numloop1:	inc	ah
	sub	al,10
	jnc	numloop1
	add	al,10
	add	al,cl
	ret

	endp





Showword	proc	near

	mov	ch,0
	mov	bx,10000
	mov	cl,47
word1:	inc	cl
	sub	ax,bx
	jnc	word1
	add	ax,bx
	call	convnum
	mov	[cs:di],cl
	mov	bx,1000
	mov	cl,47
word2:	inc	cl
	sub	ax,bx
	jnc	word2
	add	ax,bx
	call	convnum
	mov	[cs:di+1],cl
	mov	bx,100
	mov	cl,47
word3:	inc	cl
	sub	ax,bx
	jnc	word3
	add	ax,bx
	call	convnum
	mov	[cs:di+2],cl
	mov	bx,10
	mov	cl,47
word4:	inc	cl
	sub	ax,bx
	jnc	word4
	add	ax,bx
	call	convnum
	mov	[cs:di+3],cl
	add	al,48
	mov	cl,al
	call	convnum
	mov	[cs:di+4],cl
	ret

	endp




Convnum 	proc	near

	cmp	ch,0
	jnz	noconvnum
	cmp	cl,"0"
	jnz	notzeronum
	mov	cl,32
	jmp	noconvnum
notzeronum:	mov	ch,1
noconvnum:	ret

	endp








;---------------------------------------------Handling of pointer on screen----

Mainscreen	proc	near

	mov	inmaparea,0
	mov	bx,offset cs:mainlist
	cmp	watchon,1
	jz	checkmain
	mov	bx,offset cs:mainlist2
checkmain:	call	checkcoords
	cmp	walkandexam,0
	jz	finishmain
	call	walkandexamine
finishmain:	ret

mainlist:	dw	44,70,32,46,look
	dw	0,50,0,180,inventory
	dw	226,244,10,26,zoomonoff
	dw	226,244,26,40,saveload
	dw	240,260,100,124,madmanrun
	dw	0,320,0,200,identifyob
	dw	0ffffh

mainlist2:	dw	44,70,32,46,look
	dw	0,50,0,180,inventory
	dw	226+48,244+48,10,26,zoomonoff
	dw	226+48,244+48,26,40,saveload
	dw	240,260,100,124,madmanrun
	dw	0,320,0,200,identifyob
	dw	0ffffh

	endp






Madmanrun	proc	near

	cmp	location,14
	jnz	identifyob
	cmp	mapx,22
	jnz	identifyob
	cmp	pointermode,2
	jnz	identifyob
	cmp	madmanflag,0
	jnz	identifyob

	cmp	commandtype,211
	jz	alreadyrun
	mov	commandtype,211
	mov	al,52
	call	commandonly
alreadyrun:	cmp	mousebutton,1
	jnz	norun
	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	norun
	mov	lastweapon,8
norun:	ret

	endp






Checkcoords	proc	near
	cmp newlocation,255		;objects keep enumerated even in loading state, fixme
	jz loop048
	ret

loop048:	mov	ax,[cs:bx]
	cmp	ax,0ffffh
	jz	nonefound
	push	bx
	cmp	mousex,ax
	jl	over045
	mov	ax,[cs:bx+2]
	cmp	mousex,ax
	jge	over045
	mov	ax,[cs:bx+4]
	cmp	mousey,ax
	jl	over045
	mov	ax,[cs:bx+6]
	cmp	mousey,ax
	jge	over045
	mov	ax,[cs:bx+8]
	call	ax
finished:	pop	ax
	ret
over045:	pop	bx
	add	bx,10
	jmp	loop048
nonefound:	ret

	endp





;-------------------------------------------Printing of icons during scroll----





Identifyob	proc	near

	cmp	watchingtime,0
	jnz	blank
	
	mov	ax,mousex
	sub	ax,mapadx
	cmp	ax,22*8
	jc	notover1
	call	blank
	ret

notover1:	mov	bx,mousey
	sub	bx,mapady
	cmp	bx,20*8
	jc	notover2
	call	blank
	ret

notover2:	mov	inmaparea,1
	mov	ah,bl
	push	ax
	call	findpathofpoint
	mov	pointerspath,dl
	pop	ax
	push	ax
	call	findfirstpath
	mov	pointerfirstpath,al
	pop	ax

	call	checkifex
	jnz	finishidentify
	call	checkiffree
	jnz	finishidentify
	call	checkifperson
	jnz	finishidentify
	call	checkifset
	jnz	finishidentify

	mov	ax,mousex
	sub	ax,mapadx
	mov	cl,al
	mov	ax,mousey
	sub	ax,mapady
	mov	ch,al
	call	checkone
	cmp	al,0
	jz	nothingund
	;cmp	watchingtime,0
	;jnz	nothingund
	cmp	mandead,1
	jz	nothingund
	mov	ah,3
	call	obname
finishidentify:	ret

nothingund:	call	blank
	ret

	endp








Checkifperson	proc	near

	mov	es,buffers
	mov	bx,peoplelist
	mov	cx,12
identifyreel:	push	cx

	cmp	byte ptr [es:bx+4],255
	jz	notareelid

	push	es bx ax
	mov	ax,[es:bx+0]
	mov	reelpointer,ax
	call	getreelstart
	cmp	[es:si+2],0ffffh
	jnz	notblankpers
	add	si,5
notblankpers:	mov	cx,[es:si+2]	 ;x,y of reel slot
	mov	ax,[es:si+0]	 ;frame number
	push	cx
	call	getreelframeax
	pop	cx
	add	cl,[es:bx+4]
	add	ch,[es:bx+5]
	mov	dx,cx
	add	dl,[es:bx+0]
	add	dh,[es:bx+1]
	pop	ax bx es

	cmp	al,cl
	jc	notareelid
	cmp	ah,ch
	jc	notareelid
	cmp	al,dl
	jnc	notareelid
	cmp	ah,dh
	jnc	notareelid

	pop	cx
	mov	ax,[es:bx+2]
	mov	persondata,ax
	mov	al,[es:bx+4]
	mov	ah,5
	call	obname
	mov	al,0
	cmp	al,1
	ret

notareelid:	pop	cx
	add	bx,5
	dec	cx
	jnz	identifyreel
	ret

	endp








Checkifset	proc	near

	mov	es,buffers
	mov	bx,setlist+(127*5)
	mov	cx,127
identifyset:	cmp	byte ptr [es:bx+4],255
	jz	notasetid
	cmp	al,[es:bx]
	jc	notasetid
	cmp	al,[es:bx+2]
	jnc	notasetid
	cmp	ah,[es:bx+1]
	jc	notasetid
	cmp	ah,[es:bx+3]
	jnc	notasetid
	call	pixelcheckset
	jz	notasetid
	call	isitdescribed
	jz	notasetid
	mov	al,[es:bx+4]
	mov	ah,1
	call	obname
	mov	al,0
	cmp	al,1
	ret
notasetid:	sub	bx,5
	dec	cx
	cmp	cx,-1
	jnz	identifyset
	ret

	endp










Checkifex	proc	near

	mov	es,buffers
	mov	bx,exlist+(99*5)
	mov	cx,99
identifyex:	cmp	byte ptr [es:bx+4],255
	jz	notanexid
	cmp	al,[es:bx]
	jc	notanexid
	cmp	al,[es:bx+2]
	jnc	notanexid
	cmp	ah,[es:bx+1]
	jc	notanexid
	cmp	ah,[es:bx+3]
	jnc	notanexid
	mov	al,[es:bx+4]
	mov	ah,4
	call	obname
	mov	al,1
	cmp	al,0
	ret
notanexid:	sub	bx,5
	dec	cx
	cmp	cx,-1
	jnz	identifyex
	ret

	endp






Checkiffree	proc	near

	mov	es,buffers
	mov	bx,freelist+(79*5)
	mov	cx,79
identifyfree:	cmp	byte ptr [es:bx+4],255
	jz	notafreeid
	cmp	al,[es:bx]
	jc	notafreeid
	cmp	al,[es:bx+2]
	jnc	notafreeid
	cmp	ah,[es:bx+1]
	jc	notafreeid
	cmp	ah,[es:bx+3]
	jnc	notafreeid
	mov	al,[es:bx+4]
	mov	ah,2
	call	obname
	mov	al,0
	cmp	al,1
	ret
notafreeid:	sub	bx,5
	dec	cx
	cmp	cx,-1
	jnz	identifyfree
	ret

	endp






Isitdescribed	proc	near

	push	ax cx es bx
	mov	al,[es:bx+4]		;get object number
	mov	ah,0
	add	ax,ax
	mov	bx,ax
	mov	es,setdesc
	add	bx,settextdat
	mov	ax,[es:bx]
	add	ax,settext
	mov	bx,ax
	mov	dl,[es:bx]
	pop	bx es cx ax
	cmp	dl,0
	ret

	endp








;Getcurrentpath	proc	near		;routine finds out which path
;				;block the pointer is in.
;	push	ax		;used to see if an object is
;	call	findpathofpoint	;close or not
;	pop	ax
;	mov	pointerspath,dl
;	ret
;
;	endp





Findpathofpoint	proc	near

	push	ax
	mov	bx,pathdata
	mov	es,reels
	mov	al,roomnum
	mov	ah,0
	mov	cx,144
	mul	cx
	add	bx,ax
	pop	cx

	mov	dl,0
pathloop:	mov	al,[es:bx+6]
	cmp	al,255
	jnz	flunkedit
	mov	ax,[es:bx+2]
	cmp	ax,0ffffh
	jz	flunkedit
	cmp	cl,al
	jc	flunkedit
	cmp	ch,ah
	jc	flunkedit
	mov	ax,[es:bx+4]
	cmp	cl,al
	jnc	flunkedit
	cmp	ch,ah
	jnc	flunkedit
	jmp	gotvalidpath
flunkedit:	add	bx,8
	inc	dl
	cmp	dl,12
	jnz	pathloop
	mov	dl,255
gotvalidpath:	ret

	endp





Findfirstpath	proc	near	;similar to last routine, but it
			;searches each path to see if
	push	ax	;pointer is within it, regardless
	mov	bx,pathdata	;of whether the path is on or off
	mov	es,reels	;it returns the on or off state in
	mov	al,roomnum	;al (255=on 0=off) 0 if no path
	mov	ah,0
	mov	cx,144
	mul	cx
	add	bx,ax
	pop	cx

	mov	dl,0
fpathloop:	mov	ax,[es:bx+2]
	cmp	ax,0ffffh
	jz	nofirst
	cmp	cl,al
	jc	nofirst
	cmp	ch,ah
	jc	nofirst
	mov	ax,[es:bx+4]
	cmp	cl,al
	jnc	nofirst
	cmp	ch,ah
	jnc	nofirst
	jmp	gotfirst
nofirst:	add	bx,8
	inc	dl
	cmp	dl,12
	jnz	fpathloop
	mov	al,0
	ret
gotfirst:	mov	al,[es:bx+6]
	ret

	endp








Turnpathon	proc	near		;turns path on permanently

	push	ax ax
	mov	cl,255
	mov	ch,roomnum	
	add	ch,100
	call	findormake
	pop	ax
	call	getroomspaths
	pop	ax
	cmp	al,255
	jz	nopathon
	mov	ah,0
	add	ax,ax
	add	ax,ax
	add	ax,ax
	add	bx,ax
	mov	al,255
	mov	[es:bx+6],al
nopathon:	ret

	endp







Turnpathoff	proc	near		;turns path on permanently

	push	ax ax
	mov	cl,0
	mov	ch,roomnum
	add	ch,100
	call	findormake
	pop	ax
	call	getroomspaths
	pop	ax
	cmp	al,255
	jz	nopathoff
	mov	ah,0
	add	ax,ax
	add	ax,ax
	add	ax,ax
	add	bx,ax
	mov	al,0
	mov	[es:bx+6],al
nopathoff:	ret

	endp















Turnanypathon	proc	near

	push	ax ax
	mov	cl,255
	mov	ch,ah
	add	ch,100
	call	findormake
	pop	ax
	mov	al,ah
	mov	ah,0
	mov	cx,144
	mul	cx
	mov	es,reels
	mov	bx,pathdata
	add	bx,ax
	pop	ax
	mov	ah,0
	add	ax,ax
	add	ax,ax
	add	ax,ax
	add	bx,ax
	mov	al,255
	mov	[es:bx+6],al
	ret

	endp






Turnanypathoff	proc	near

	push	ax ax
	mov	cl,0
	mov	ch,ah
	add	ch,100
	call	findormake
	pop	ax
	mov	al,ah
	mov	ah,0
	mov	cx,144
	mul	cx
	mov	es,reels
	mov	bx,pathdata
	add	bx,ax
	pop	ax
	mov	ah,0
	add	ax,ax
	add	ax,ax
	add	ax,ax
	add	bx,ax
	mov	al,0
	mov	[es:bx+6],al
	ret

	endp





Checkifpathison	proc	near

	push	ax
	call	getroomspaths
	pop	ax
	mov	ah,0
	add	ax,ax
	add	ax,ax
	add	ax,ax
	add	bx,ax
	mov	al,[es:bx+6]
	cmp	al,255
	ret

	endp





Afternewroom	proc	near

	cmp	nowinnewroom,0
	jz	notnew
	mov	timecount,0
	call	createpanel
	mov	commandtype,0
	call	findroominloc

	cmp	ryanon,1
	jz	ryansoff

	mov	al,ryanx
	add	al,12
	mov	ah,ryany
	add	ah,12
	call	findpathofpoint
	mov	manspath,dl
	call	findxyfrompath
	mov	resetmanxy,1

ryansoff:	mov	newobs,1
	call	drawfloor
	mov	lookcounter,160
	mov	nowinnewroom,0

	call	showicon
	call	spriteupdate
	call	printsprites
	call	undertextline
	call	reelsonscreen
	call	mainscreen
	call	getunderzoom
	call	zoom
	call	worktoscreenm
	call	walkintoroom
	call	reminders
	call	atmospheres
notnew:	ret

	endp






Atmospheres	proc	near

	mov	cl,mapx
	mov	ch,mapy
	mov	bx,offset cs:atmospherelist
nextatmos:	mov	al,[cs:bx]
	cmp	al,255
	jz	nomoreatmos
	cmp	al,reallocation
	jnz	wrongatmos
	mov	ax,[cs:bx+1]
	cmp	ax,cx
	jnz	wrongatmos
	mov	ax,[cs:bx+3]
	cmp	al,ch0playing
	jz	playingalready
	cmp	location,45
	jnz	notweb
	cmp	reeltowatch,45
	jz	wrongatmos
notweb:	call	playchannel0
	cmp	reallocation,2
	cmp	mapy,0
	jz	fullvol
	jnz	notlouisvol
	cmp	mapy,10
	jnz	notlouisvol
	cmp	mapx,22
            	jnz	notlouisvol
	mov	volume,5
notlouisvol:	if	cd
	cmp	reallocation,14
	jnz	notmad1
	cmp	mapx,33
	jz	ismad2
	cmp	mapx,22
	jnz	notmad1
	mov	volume,5
	ret
ismad2:	mov	volume,0
	ret
notmad1:	endif
playingalready:	cmp	reallocation,2
	jnz	notlouisvol2
	cmp	mapx,22
	jz	louisvol
             	cmp	mapx,11
	jnz	notlouisvol2
fullvol:	mov	volume,0
notlouisvol2:	ret
louisvol:	mov	volume,5
	ret
wrongatmos:	add	bx,5
	jmp	nextatmos
nomoreatmos:	call	cancelch0
	ret

atmospherelist:	db	0,33,10,15,255
	db	0,22,10,15,255
	db	0,22,0,15,255
	db	0,11,0,15,255
	db	0,11,10,15,255
	db	0,0,10,15,255

	db	1,44,10,6,255	;location,map x,y,sound,repeat
	db	1,44,0,13,255

	db	2,33,0,6,255
	db	2,22,0,5,255
	db	2,22,10,16,255
	db	2,11,10,16,255

	db	3,44,0,15,255
	db	3,33,10,6,255
	db	3,33,0,5,255

	db	4,11,30,6,255
	db	4,22,30,5,255
	db	4,22,20,13,255

	db	10,33,30,6,255
	db	10,22,30,6,255
	
	db	9,22,10,6,255
	db	9,22,20,16,255
	db	9,22,30,16,255
	db	9,22,40,16,255
	db	9,22,50,16,255
	
	db	6,11,30,6,255
	db	6,0,10,15,255
	db	6,0,20,15,255
	db	6,11,20,15,255
	db	6,22,20,15,255
	
	db	7,11,20,6,255
	db	7,0,20,6,255
	db	7,0,30,6,255
	
	db	55,44,0,5,255
	db	55,44,10,5,255

	db	5,22,30,6,255
	if	demo
	db	5,22,20,16,255
	db	5,22,10,16,255
	else
	db	5,22,20,15,255
	db	5,22,10,15,255
	endif

	db	24,22,0,15,255
	db	24,33,0,15,255
	db	24,44,0,15,255
	db	24,33,10,15,255

	db	8,0,10,6,255
	db	8,11,10,6,255
	db	8,22,10,6,255
	db	8,33,10,6,255
	db	8,33,20,6,255
	db	8,33,30,6,255
	db	8,33,40,6,255
	db	8,22,40,6,255
	db	8,11,40,6,255
    
	db	11,11,20,12,255
	db	11,11,30,12,255
	db	11,22,20,12,255
	db	11,22,30,12,255
	
	db	12,22,20,12,255
	db	13,22,20,12,255
	db	13,33,20,12,255

	db	14,44,20,12,255
	db	14,33,0,12,255
	db	14,33,10,12,255
	db	14,33,20,12,255
	db	14,33,30,12,255
	db	14,33,40,12,255
	db	14,22,0,16,255
	
	db	19,0,0,12,255

	db	20,0,20,16,255
	db	20,0,30,16,255
	db	20,11,30,16,255
	db	20,0,40,16,255
	db	20,11,40,16,255

	if	demo
	db	21,11,10,16,255
	db	21,11,20,16,255
	db	21,0,20,16,255
	db	21,22,20,16,255
	db	21,33,20,16,255
	db	21,44,20,16,255
	db	21,44,10,16,255
	else
	db	21,11,10,15,255
	db	21,11,20,15,255
	db	21,0,20,15,255
	db	21,22,20,15,255
	db	21,33,20,15,255
	db	21,44,20,15,255
	db	21,44,10,15,255
	endif

	db	22,22,10,16,255
	db	22,22,20,16,255

	db	23,22,30,13,255
	db	23,22,40,13,255
	db	23,33,40,13,255
	db	23,11,40,13,255
	db	23,0,40,13,255
	db	23,0,50,13,255

	db	25,11,40,16,255
	db	25,11,50,16,255
	db	25,0,50,16,255

	db	27,11,20,16,255
	db	27,11,30,16,255

	db	29,11,10,16,255

	db	45,22,30,12,255
	db	45,22,40,12,255
	db	45,22,50,12,255

	db	46,22,40,12,255
	db	46,11,50,12,255
	db	46,22,50,12,255
	db	46,33,50,12,255

	db	47,0,0,12,255

	db	26,22,20,16,255
	db	26,33,10,16,255
	db	26,33,20,16,255
	db	26,33,30,16,255
	db	26,44,30,16,255
	db	26,22,30,16,255
	db	26,11,30,16,255
	db	26,11,20,16,255
	db	26,0,20,16,255
	db	26,11,40,16,255
	db	26,0,40,16,255
	db	26,22,40,16,255
	db	26,11,50,16,255

	db	28,0,30,15,255
	db	28,0,20,15,255
	db	28,0,40,15,255
	db	28,11,30,15,255
	db	28,11,20,15,255
	db	28,22,30,15,255
	db	28,22,20,15,255

	db	255

	endp






Walkintoroom	proc	near
	
	cmp	location,14
	jnz	notlair
	cmp	mapx,22
	jnz	notlair
	mov	destination,1
	mov	finaldest,1
	call	autosetwalk
notlair:	ret

	endp












Afterintroroom	proc	near

	cmp	nowinnewroom,0
	jz	notnewintro
	call	clearwork
	call	findroominloc
	mov	newobs,1
	call	drawfloor
	call	reelsonscreen
	call	spriteupdate
	call	printsprites
	call	worktoscreen
	mov	nowinnewroom,0
notnewintro:	ret

	endp











Obname	proc	near

	cmp	reasseschanges,0
	jz	notnewpath
	mov	reasseschanges,0
	jmp	diff

notnewpath:	cmp	ah,commandtype
	jz	notdiffob
	jmp	diff
notdiffob:	cmp	al,command
	jnz	diff
	cmp	walkandexam,1
	jz	walkandexamine
	cmp	mousebutton,0
	jz	noobselect
	cmp	commandtype,3
	jnz	isntblock
	cmp	lastflag,2
	jc	noobselect
isntblock:	mov	bl,manspath
	cmp	bl,pointerspath
	jnz	wantstowalk
	cmp	commandtype,3
	jz	wantstowalk
	call	finishedwalking
	jnz	noobselect
	cmp	commandtype,5
	jz	wantstotalk
	cmp	watchingtime,0
	jnz	noobselect
	call	examineob
	ret
wantstotalk:	cmp	watchingtime,0
	jnz	noobselect
	call	talk
	ret
walkandexamine:	call	finishedwalking
	jnz	noobselect
	mov	al,walkexamtype
	mov	commandtype,al
	mov	al,walkexamnum
	mov	command,al
	mov	walkandexam,0
	cmp	commandtype,5
	jz	noobselect
	call	examineob
	ret
wantstowalk:	call	setwalk
	mov	reasseschanges,1
noobselect:	ret


diff:	mov	command,al
	mov	commandtype,ah
diff2:	cmp	linepointer,254
	jnz	middleofwalk
	cmp	watchingtime,0
	jnz	middleofwalk
	mov	al,facing
	cmp	al,turntoface
	jnz	middleofwalk
	cmp	commandtype,3
	jnz	notblock
	mov	bl,manspath
	cmp	bl,pointerspath
	jnz	dontcheck
	mov	cl,ryanx		;look under feet to see if
	add	cl,12		;any flags are there
	mov	ch,ryany
	add	ch,12
	call	checkone
	cmp	cl,2
	jc	isblock
dontcheck:	call	getflagunderp
	cmp	lastflag,2
	jc	isblock
	cmp	lastflag,128
	jnc	isblock
	jmp	toofaraway ; only here for turning on doorstep
notblock:	mov	bl,manspath
	cmp	bl,pointerspath
	jnz	toofaraway
	cmp	commandtype,3
	jz	isblock
	cmp	commandtype,5
	jz	isaperson
	call	examineobtext
	ret
middleofwalk:	call	blocknametext
	ret
isblock:	call	blocknametext
	ret
isaperson:	call	personnametext
	ret
toofaraway:	call	walktotext
	ret

	endp







Finishedwalking proc	near

	cmp	linepointer,254
	jnz	iswalking
	mov	al,facing
	cmp	al,turntoface
iswalking:	ret

	endp







Examineobtext	proc	near

	mov	bl,command
	mov	bh,commandtype
	mov	al,1
	call	commandwithob
	ret

	endp





Commandwithob	proc	near

	push	ax
	push	ax bx cx dx es ds si di
	call	deltextline
	pop	di si ds es dx cx bx ax
	
	push	bx
	mov	ah,0
	add	ax,ax
	mov	bx,ax
	mov	es,commandtext
	mov	ax,[es:bx]
	add	ax,textstart
	mov	si,ax
	
	mov	di,textaddressx
	mov	bx,textaddressy
	mov	dl,textlen
	mov	al,0
	mov	ah,0
	call	printdirect

	pop	ax
	mov	di,offset cs:commandline
	call	copyname
	pop	ax

	mov	di,lastxpos
	cmp	al,0
	jz	noadd
	add	di,5
noadd:	mov	bx,textaddressy
	push	cs
	pop	es
	mov	si,offset cs:commandline
	mov	dl,textlen
	mov	al,0
	mov	ah,0
	call	printdirect
	mov	newtextline,1
	ret	             

commandline:	db	"OBJECT NAME ONE                         ",0
	
	endp




Commandonly	proc	near

	push	ax bx cx dx es ds si di
	call	deltextline
	pop	di si ds es dx cx bx ax
	
	mov	ah,0
	add	ax,ax
	mov	bx,ax
	mov	es,commandtext
	mov	ax,[es:bx]
	add	ax,textstart
	mov	si,ax
	
	mov	di,textaddressx
	mov	bx,textaddressy
	mov	dl,textlen
	mov	al,0
	mov	ah,0
	call	printdirect
	mov	newtextline,1
	ret

	endp





Printmessage	proc	near

	push 	dx bx di
	mov	ah,0
	add	ax,ax
	mov	bx,ax
	mov	es,commandtext
	mov	ax,[es:bx]
	add	ax,textstart
	mov	si,ax
	pop	di bx dx
	mov	al,0
	mov	ah,0
	call	printdirect
	ret

	endp



Printmessage2	proc	near

	push 	dx bx di
	push	ax
	mov	ah,0
	add	ax,ax
	mov	bx,ax
	mov	es,commandtext
	mov	ax,[es:bx]
	add	ax,textstart
	mov	si,ax
	pop	ax

searchmess:	push	ax
	call	findnextcolon
	pop	ax
	dec	ah
	jnz	searchmess

	pop	di bx dx
	mov	al,0
	mov	ah,0
	call	printdirect
	ret

	endp









Blocknametext	proc	near

	mov	bl,command
	mov	bh,commandtype
	mov	al,0
	call	commandwithob
	ret

	endp




Personnametext	proc	near

	mov	bl,command
	and	bl,127
	mov	bh,commandtype
	mov	al,2
	call	commandwithob
	ret

	endp







Walktotext	proc	near

	mov	bl,command
	mov	bh,commandtype
	mov	al,3
	call	commandwithob
	ret

	endp





Getflagunderp	proc	near

	mov	cx,mousex
	sub	cx,mapadx
	mov	ax,mousey
	sub	ax,mapady
	mov	ch,al
	call	checkone
	mov	lastflag,cl
	mov	lastflagex,ch
	ret

	endp





Setwalk 	proc	near

	cmp	linepointer,254
	jnz	alreadywalking
	mov	al,pointerspath
	cmp	al,manspath
	jz	cantwalk2
	cmp	watchmode,1
	jz	holdingreel
	cmp	watchmode,2
	jz	cantwalk
	mov	destination,al
	mov	finaldest,al
	cmp	mousebutton,2
	jnz	notwalkandexam
	cmp	commandtype,3
	jz	notwalkandexam
	mov	walkandexam,1
	mov	al,commandtype
	mov	walkexamtype,al
	mov	al,command
	mov	walkexamnum,al
notwalkandexam:	call	autosetwalk
cantwalk:	ret
cantwalk2:	call	facerightway
	ret
alreadywalking:	mov	al,pointerspath
	mov	finaldest,al
	ret

holdingreel:	mov	destafterhold,al
	mov	watchmode,2
	ret

	endp







Autosetwalk	proc	near

	mov	al,manspath
	cmp	finaldest,al
	jnz	notsamealready
	ret
notsamealready:	call	getroomspaths
	call	checkdest
	push	bx
	mov	al,manspath
	mov	ah,0
	add	ax,ax
	add	ax,ax
	add	ax,ax
	add	bx,ax
	mov	al,[es:bx]
	mov	ah,0
	sub	ax,12
	mov	linestartx,ax
	mov	al,[es:bx+1]
	mov	ah,0
	sub	ax,12
	mov	linestarty,ax
	pop	bx

	mov	al,destination
	mov	ah,0
	add	ax,ax
	add	ax,ax
	add	ax,ax
	add	bx,ax
	mov	al,[es:bx]
	mov	ah,0
	sub	ax,12
	mov	lineendx,ax
	mov	al,[es:bx+1]
	mov	ah,0
	sub	ax,12
	mov	lineendy,ax
	call	bresenhams

	cmp	linedirection,0
	jz	normalline
	mov	al,linelength
	dec	al
	mov	linepointer,al
	mov	linedirection,1
	ret

normalline:	mov	linepointer,0
	ret

	endp








Checkdest	proc	near

	push	bx
	add	bx,12*8
	mov	ah,manspath
	mov	cl,4
	shl	ah,cl
	mov	al,destination

	mov	cl,24
	mov	ch,destination
checkdestloop:	mov	dh,[es:bx]
	and	dh,11110000b
	mov	dl,[es:bx]
	and	dl,00001111b
	cmp	ax,dx
	jnz	nextcheck
	mov	al,[es:bx+1]
	and	al,15
	mov	destination,al
	pop	bx
	ret
nextcheck:	mov	dl,[es:bx]
	and	dl,11110000b
	shr	dl,1
	shr	dl,1
	shr	dl,1
	shr	dl,1
	mov	dh,[es:bx]
	and	dh,00001111b
	shl	dh,1
	shl	dh,1
	shl	dh,1
	shl	dh,1
	cmp	ax,dx
	jnz	nextcheck2
	mov	ch,[es:bx+1]
	and	ch,15
nextcheck2:	add	bx,2
	dec	cl
	jnz	checkdestloop
	mov	destination,ch
	pop	bx
	ret

	endp














Bresenhams	proc	near

	call	workoutframes

	mov	dx,seg linedata
	mov	es,dx
	mov	di,offset es:linedata
	mov	si,1
	mov	linedirection,0

	mov	cx,lineendx
	sub	cx,linestartx
	jz	vertline
	jns	line1

	neg	cx
	mov	bx,lineendx
	xchg	bx,linestartx
	mov	lineendx,bx

	mov	bx,lineendy
	xchg	bx,linestarty
	mov	lineendy,bx
	mov	linedirection,1

line1:	mov	bx,lineendy
	sub	bx,linestarty
	jz	horizline
	jns	line3

	neg	bx
	neg	si

line3:	push	si
	mov	lineroutine,0 ; means lo slope
	cmp	bx,cx
	jle	line4
	mov	lineroutine,1 ; means hi slope
	xchg	bx,cx

line4:	shl	bx,1
	mov	increment1,bx
	sub	bx,cx
	mov	si,bx
	sub	bx,cx
	mov	increment2,bx

	mov	ax,linestartx
	mov	bx,linestarty
	mov	ah,bl
	inc	cx
	pop	bx
	cmp	lineroutine,1
	jz	hislope
	jmp	loslope

vertline:	mov	ax,linestarty
	mov	bx,lineendy
	mov	cx,bx
	sub	cx,ax
	jge	line31

	neg	cx
	mov	ax,bx
	mov	linedirection,1

line31: 	inc	cx
	mov	bx,linestartx
	xchg	ax,bx
	mov	ah,bl
	mov	bx,si
line32: 	stosw
	add	ah,bl
	loop	line32
	jmp	lineexit


horizline:	mov	ax,linestartx
	mov	bx,linestarty
	mov	ah,bl
	inc	cx
horizloop:	stosw
	inc	al
	loop	horizloop
	jmp	lineexit


loslope:
loloop: 	stosw
	inc	al
	or	si,si
	jns	line12
	add	si,increment1
	loop	loloop
	jmp	lineexit

line12: 	add	si,increment2
	add	ah,bl
	loop	loloop
	jmp	lineexit



hislope:
hiloop: 	stosw
	add	ah,bl
	or	si,si
	jns	line23
	add	si,increment1
	loop	hiloop
	jmp	lineexit

line23: 	add	si,increment2
	inc	al
	loop	hiloop

lineexit:	sub	di,offset es:linedata
	mov	ax,di
	shr	ax,1
	mov	linelength,al
	ret

	endp







Workoutframes	proc	near

	mov	bx,linestartx
	add	bx,32
	mov	ax,lineendx
	add	ax,32
	sub	bx,ax
	jnc	notneg1
	neg	bx
notneg1:	mov	cx,linestarty
	add	cx,32
	mov	ax,lineendy
	add	ax,32
	sub	cx,ax
	jnc	notneg2
	neg	cx
notneg2:	cmp	bx,cx
	jnc	tendstohoriz
	mov	dl,2
	mov	ax,cx
	shr	ax,1
	cmp	bx,ax
	jc	gotquad
	mov	dl,1
	jmp	gotquad
tendstohoriz:	mov	dl,0
	mov	ax,bx
	shr	ax,1
	cmp	cx,ax
	jc	gotquad
	mov	dl,1
	jmp	gotquad

gotquad:	mov	bx,linestartx
	add	bx,32
	mov	ax,lineendx
	add	ax,32
	sub	bx,ax
	jc	isinright
isinleft:	mov	cx,linestarty
	add	cx,32
	mov	ax,lineendy
	add	ax,32
	sub	cx,ax
	jnc	topleft
	cmp	dl,1
	jz	noswap1
	xor	dl,2
noswap1:	add	dl,4
	jmp	success
topleft:	add	dl,6
	jmp	success

isinright:	mov	cx,linestarty
	add	cx,32
	mov	ax,lineendy
	add	ax,32
	sub	cx,ax
	jnc	botright
	add	dl,2
	jmp	success
botright:	cmp	dl,1
	jz	noswap2
	xor	dl,2
noswap2:

success:	and	dl,7
	mov	turntoface,dl
	mov	turndirection,0
	ret

	endp


















;Multiply8	proc	near
;
;	mov	ah,0
;	mov	cx,8
;	mul	cx
;	ret
;
;	endp






Getroomspaths	proc	near

	mov	al,roomnum
	mov	ah,0
	mov	cx,144
	mul	cx
	mov	es,reels
	mov	bx,pathdata
	add	bx,ax
	ret

	endp












	



Copyname	proc	near

	push	di
	call	findobname
	pop	di
	push	cs
	pop	es

copytext:	mov	cx,28
make:	lodsb
	cmp	al,":"
	jz	finishmakename
	cmp	al,0
	jz	finishmakename
	stosb
	loop	make

finishmakename:	inc	cx
	mov	al,0
	stosb
	ret
	mov	al,255
	rep	stosb
	ret

	endp









Findobname	proc	near

	push	ax
	mov	ah,0
	add	ax,ax
	mov	bx,ax
	pop	ax

	cmp	ah,5
	jnz	notpersonname

	push	ax
	and	al,127
	mov	ah,0
	mov	bx,64*2
	mul	bx
	mov	si,ax
	mov	ds,people
	add	si,persontxtdat
	mov	cx,persontext
	mov	ax,[si]
	add	ax,cx
	mov	si,ax
	pop	ax
	ret

notpersonname:	cmp	ah,4
	jnz	notextraname
	mov	ds,extras
	add	bx,extextdat
	mov	ax,[bx]
	add	ax,extext
	mov	si,ax
	ret

notextraname:	cmp	ah,2
	jnz	notfreename
	mov	ds,freedesc
	add	bx,freetextdat
	mov	ax,[bx]
	add	ax,freetext
	mov	si,ax
	ret

notfreename:	cmp	ah,1
	jnz	notsetname
	mov	ds,setdesc
	add	bx,settextdat
	mov	ax,[bx]
	add	ax,settext
	mov	si,ax
	ret
	
notsetname:	mov	ds,blockdesc
	add	bx,blocktextdat
	mov	ax,[bx]
	add	ax,blocktext
	mov	si,ax
	ret

	endp













;-------------------------------------------Printing of non scrolling icons----

Showicon	proc	near

	cmp	reallocation,50
	jnc	isdream1
	call	showpanel
	call	showman
	call	roomname
	call	panelicons1
	call	zoomicon
	ret

isdream1:	mov	ds,tempsprites
	mov	di,72
	mov	bx,2
	mov	al,45
	mov	ah,0
	call	showframe
	mov	ds,tempsprites
	mov	di,72+47
	mov	bx,2
	mov	al,46
	mov	ah,0
	call	showframe
	mov	ds,tempsprites
	mov	di,69-10
	mov	bx,21
	mov	al,49
	mov	ah,0
	call	showframe

	mov	ds,tempsprites
	mov	di,160+88
	mov	bx,2
	mov	al,45
	mov	ah,4
	call	showframe
	mov	ds,tempsprites
	mov	di,160+43
	mov	bx,2
	mov	al,46
	mov	ah,4
	call	showframe
	mov	ds,tempsprites
	mov	di,160+101
	mov	bx,21
	mov	al,49
	mov	ah,4
	call	showframe
	call	middlepanel
	ret

	endp





Middlepanel	proc	near

	mov	ds,tempsprites
	mov	di,72+47+20
	mov	bx,0
	mov	al,48
	mov	ah,0
	call	showframe
	mov	ds,tempsprites
	mov	di,72+19
	mov	bx,21
	mov	al,47
	mov	ah,0
	call	showframe
	mov	ds,tempsprites
	mov	di,160+23
	mov	bx,0
	mov	al,48
	mov	ah,4
	call	showframe
	mov	ds,tempsprites
	mov	di,160+71
	mov	bx,21
	mov	al,47
	mov	ah,4
	call	showframe
	ret

	endp










Showman 	proc	near

	mov	ds,icons1
	mov	di,0
	mov	bx,0
	mov	al,0
	mov	ah,0
	call	showframe
	mov	ds,icons1
	mov	di,0
	mov	bx,114
	mov	al,1
	mov	ah,0
	call	showframe

	cmp	shadeson,0
	jz	notverycool

	mov	ds,icons1
	mov	di,28
	mov	bx,25
	mov	al,2
	mov	ah,0
	call	showframe
notverycool:	ret

	endp



















Showpanel	proc	near

	mov	ds,icons1
	mov	di,72
	mov	bx,0
	mov	al,19
	mov	ah,0
	call	showframe
	mov	ds,icons1
	mov	di,192
	mov	bx,0
	mov	al,19
	mov	ah,0
	call	showframe
	ret

	endp






















Roomname	proc	near

	mov	di,88
	mov	bx,18
	mov	al,53
	mov	dl,240
	call	printmessage

	mov	bl,roomnum
	cmp	bl,32
	jc	notover32
	sub	bl,32

notover32:	mov	bh,0
	add	bx,bx
	mov	es,roomdesc
	add	bx,intextdat
	mov	ax,[es:bx]
	add	ax,intext
	mov	si,ax

	mov	linespacing,7
	mov	di,88
	mov	bx,25
	mov	dl,120
	cmp	watchon,1
	jz	gotpl
	mov	dl,160
gotpl:	mov	al,0
	mov	ah,0
	call	printdirect
	mov	linespacing,10

	call	usecharset1
	ret

	endp






Usecharset1	proc	near

	mov	ax,charset1
	mov	currentset,ax
	ret

	endp











Usetempcharset	proc	near

	mov	ax,tempcharset
	mov	currentset,ax
	ret

	endp





Showexit	proc	near

	mov	ds,icons1
	mov	di,274
	mov	bx,154
	mov	al,11
	mov	ah,0
	call	showframe
	ret

	endp




Panelicons1	proc	near

	mov	di,0
	cmp	watchon,1
	jz	watchison
	mov	di,48
watchison:	push	di
	mov	ds,icons2
	add	di,204
	mov	bx,4
	mov	al,2
	mov	ah,0
	call	showframe
	pop	di
	push	di
	cmp	zoomon,1
	jz	zoomisoff
	mov	ds,icons1
	add	di,228
	mov	bx,8
	mov	al,5
	mov	ah,0
	call	showframe
zoomisoff:	pop	di
	call	showwatch
	ret

	endp












Showwatch	proc	near

	cmp	watchon,0
	jz	nowristwatch
	mov	ds,icons1
	mov	di,250
	mov	bx,1
	mov	al,6
	mov	ah,0
	call	showframe
	call	showtime
nowristwatch:	ret

	endp


Gettime	proc	near
	
	mov	ah,2ch
	int	21h
	mov	secondcount,dh
	mov	minutecount,cl
	mov	hourcount,ch
	ret

	endp





Zoomicon	proc	near

	cmp	zoomon,0
	jz	nozoom1
	mov	ds,icons1
	mov	di,zoomx
	mov	bx,zoomy-1
	mov	al,8
	mov	ah,0
	call	showframe
nozoom1:	ret

	endp






Showblink	proc	near

	cmp	manisoffscreen,1
	jz	finblink1
	inc	blinkcount
	cmp	shadeson,0
	jnz	finblink1
	cmp	reallocation,50
	jnc	eyesshut
	mov	al,blinkcount
	cmp	al,3
	jnz	finblink1
	mov	blinkcount,0
	mov	al,blinkframe
	inc	al
	mov	blinkframe,al
	cmp	al,6
	jc	nomorethan6
	mov	al,6
nomorethan6:	mov	ah,0
	mov	bx,offset cs:blinktab
	add	bx,ax

	mov	al,[cs:bx]
	mov	ds,icons1
	mov	di,44
	mov	bx,32
	mov	ah,0
	call	showframe
finblink1:	ret

eyesshut:	;mov	al,32
	;mov	ds,icons1
	;mov	di,44
	;mov	bx,32
	;mov	ah,0
	;call	showframe
	ret

blinktab:	db	16,18,18,17,16,16,16

	endp






Dumpblink	proc	near

	cmp	shadeson,0
	jnz	nodumpeye
	cmp	blinkcount,0
	jnz	nodumpeye
	mov	al,blinkframe
	cmp	al,6
	jnc	nodumpeye
	push	ds
	mov	di,44
	mov	bx,32
	mov	cl,16
	mov	ch,12
	call	multidump
	pop	ds
nodumpeye:	ret

	endp


















Worktoscreenm	proc	near

	call	animpointer
	call	readmouse
	call	showpointer
	call	vsync
	call	worktoscreen
	call	delpointer
	ret

	endp













;-------------------------------------------------------------Blank routine----




Blank	proc	near

	cmp	commandtype,199
	jz	alreadyblnk
	mov	commandtype,199
	mov	al,0
	call	commandonly
alreadyblnk:	ret

	endp






















;---------------------------------------------------------Standard routines----













Allpointer	proc	near

	call	readmouse
	call	showpointer
	call	dumppointer
	ret

	endp







Hangonp 	proc	near

	push	cx
	add	cx,cx
	pop	ax
	add	cx,ax
	mov	maintimer,0
	mov	al,pointerframe
	mov	ah,pickup
	push	ax
	mov	pointermode,3
	mov	pickup,0
	push	cx
	mov	commandtype,255
	call	readmouse
	call	animpointer
	call	showpointer
	call	vsync
	call	dumppointer
	pop	cx

hangloop:	push	cx
	call	delpointer
	call	readmouse
	call	animpointer
	call	showpointer
	call	vsync
	call	dumppointer
	pop	cx
	mov	ax,mousebutton
	cmp	ax,0
	jz	notpressed
	cmp	ax,oldbutton
	jnz	getoutofit
notpressed:	loop	hangloop
	
getoutofit:	call	delpointer
	pop	ax
	mov	pointerframe,al
	mov	pickup,ah
	mov	pointermode,0
	ret

	endp





Hangonw	proc	near

hangloopw:	push	cx
	call	delpointer
	call	readmouse
	call	animpointer
	call	showpointer
	call	vsync
	call	dumppointer
	pop	cx
	loop	hangloopw
	ret

	endp




Hangoncurs	proc	near

monloop1:	push	cx
	call	printcurs
	call	vsync
	call	delcurs
	pop	cx
	loop	monloop1
	ret

	endp








Getunderzoom	proc	near

	mov	di,zoomx+5
	mov	bx,zoomy+4
	mov	ds,buffers
	mov	si,zoomspace
	mov	cl,46
	mov	ch,40
	call	multiget
	ret

	endp





Dumpzoom	proc	near

	cmp	zoomon,1
	jnz	notzoomon
	mov	di,zoomx+5
	mov	bx,zoomy+4
	mov	cl,46
	mov	ch,40
	call	multidump
notzoomon:	ret

	endp







Putunderzoom	proc	near

	mov	di,zoomx+5
	mov	bx,zoomy+4
	mov	ds,buffers
	mov	si,zoomspace
	mov	cl,46
	mov	ch,40
	call	multiput
	ret

	endp





Crosshair	proc	near

	cmp	commandtype,3
	jz	nocross
	cmp	commandtype,10
	jnc	nocross

	mov	es,workspace
	mov	ds,icons1
	mov	di,zoomx+24
	mov	bx,zoomy+19
	mov	al,9
	mov	ah,0
	call	showframe
	ret

nocross:	mov	es,workspace
	mov	ds,icons1
	mov	di,zoomx+24
	mov	bx,zoomy+19
	mov	al,29
	mov	ah,0
	call	showframe
	ret

	endp






Showpointer	proc	near

	call	showblink
	mov	di,mousex
	mov	oldpointerx,di
	mov	bx,mousey
	mov	oldpointery,bx
	cmp	pickup,1
	jz	itsanobject

	push	bx di
	mov	ds,icons1
	mov	al,pointerframe
	add	al,20
	mov	ah,0
	add	ax,ax
	mov	si,ax
	add	ax,ax
	add	si,ax
	mov	cx,[si]
	cmp	cl,12
	jnc	notsmallx
	mov	cl,12
notsmallx:	cmp	ch,12
	jnc	notsmally
	mov	ch,12
notsmally:	mov	pointerxs,cl
	mov	pointerys,ch
	push	ds
	mov	ds,buffers
	mov	si,pointerback
	call	multiget
	pop	ds di bx
	push	di bx
	mov	al,pointerframe
	add	al,20
	mov	ah,0
	call	showframe
	pop	bx di
	ret

itsanobject:	mov	al,itemframe
	mov	ds,extras
	cmp	objecttype,4
	jz	itsfrominv
	mov	ds,freeframes
itsfrominv:	mov	cl,al
	add	al,al
	add	al,cl
	inc	al
	mov	ah,0

	push	ax
	add	ax,ax
	mov	si,ax
	add	ax,ax
	add	si,ax
	mov	ax,2080
	mov	cx,[si]
	cmp	cl,12
	jnc	notsmallx2
	mov	cl,12
notsmallx2:	cmp	ch,12
	jnc	notsmally2
	mov	ch,12
notsmally2:	mov	pointerxs,cl
	mov	pointerys,ch
	pop	ax

	push	di bx
	push	ax bx di ds
	mov	al,cl
	mov	ah,0
	shr	ax,1
	sub	oldpointerx,ax
	sub	di,ax
	mov	al,ch
	shr	ax,1
	sub	oldpointery,ax
	sub	bx,ax
	mov	ds,buffers
	mov	si,pointerback
	call	multiget
	pop	ds di bx ax
	mov	ah,128
	call	showframe
	pop	bx di
	mov	ds,icons1
	mov	al,3
	mov	ah,128
	call	showframe
	ret
	
	endp







Delpointer	proc		near

	mov	ax,oldpointerx
	cmp	ax,0ffffh
	jz	nevershown
	mov	delherex,ax
	mov	ax,oldpointery
	mov	delherey,ax
	mov	cl,pointerxs
	mov	delxs,cl
	mov	ch,pointerys
	mov	delys,ch
	mov	ds,buffers
	mov	si,pointerback
	mov	di,delherex
	mov	bx,delherey
	call	multiput
nevershown:	ret

	endp









Dumppointer	proc	near

	call	dumpblink
	mov	cl,delxs
	mov	ch,delys
	mov	di,delherex
	mov	bx,delherey
	call	multidump

	mov	bx,oldpointery
	mov	di,oldpointerx
	cmp	di,delherex
	jnz	difffound
	cmp	bx,delherey
	jz	notboth
difffound:	mov	cl,pointerxs
	mov	ch,pointerys
	call	multidump
notboth:	ret

	endp











Undertextline	proc	near

	mov	di,textaddressx
	mov	bx,textaddressy
	cmp	foreignrelease, 0
	jz $1
	sub	bx,3
$1:
	mov	ds,buffers
	mov	si,textunder
	mov	cl,undertextsizex
	mov	ch,undertextsizey
	call	multiget
	ret

	endp








Deltextline	proc	near

	mov	di,textaddressx
	mov	bx,textaddressy
	cmp	foreignrelease, 0
	jz $1
	sub	bx,3
$1:
	mov	ds,buffers
	mov	si,textunder
	mov	cl,undertextsizex
	mov	ch,undertextsizey
	call	multiput
	ret

	endp





Dumptextline	proc	near

	cmp	newtextline,1
	jnz	nodumptextline
	mov	newtextline,0
	mov	di,textaddressx
	mov	bx,textaddressy
	cmp	foreignrelease, 0
	jz $1
	sub	bx,3
$1:
	mov	cl,undertextsizex
	mov	ch,undertextsizey
	call	multidump
nodumptextline:	ret

	endp

















Animpointer	proc	near

	cmp	pointermode,2
	jz	combathand
	cmp	pointermode,3
	jz	mousehand

	cmp	watchingtime,0
	jz	notwatchpoint
	mov	pointerframe,11
	ret
notwatchpoint:	mov	pointerframe,0
	cmp	inmaparea,0
	jz	gothand
	cmp	pointerfirstpath,0
	jz	gothand
arrow:	call	getflagunderp
	cmp	cl,2
	jc	gothand
	cmp	cl,128
	jnc	gothand
	mov	pointerframe,3
	test	cl,4
	jnz	gothand
	mov	pointerframe,4
	test	cl,16
	jnz	gothand
	mov	pointerframe,5
	test	cl,2
	jnz	gothand
	mov	pointerframe,6
	test	cl,8
	jnz	gothand
	mov	pointerframe,8
gothand:	ret

mousehand:	cmp	pointerspeed,0
	jz	rightspeed3
	dec	pointerspeed
	jmp	finflashmouse
rightspeed3:	mov	pointerspeed,5
	inc	pointercount
	cmp	pointercount,16
	jnz	finflashmouse
	mov	pointercount,0
finflashmouse:	mov	al,pointercount
	mov	ah,0
	mov	bx,offset cs:flashmousetab
	add	bx,ax
	mov	al,[cs:bx]
	mov	pointerframe,al
	ret

combathand:	mov	pointerframe,0
	cmp	reallocation,14
	jnz	notarrow
         	cmp	commandtype,211
	jnz	notarrow
	mov	pointerframe,5
notarrow:	ret

flashmousetab:	db	1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2

	endp




;------------------------------------------------Mouse and keyboard-readers----

Setmouse	proc	near

	if	recording
	mov	recordpos,-8
	mov	dx,seg recname
	mov	ds,dx
	mov	dx,offset recname
	mov	cx,0
	mov	ah,3ch
	mov	al,2
	int	21h
	mov	rechandle,ax
	endif

	if	playback
	mov	dx,seg recname
	mov	ds,dx
	mov	dx,offset recname
	mov	ah,3dh
	mov	al,2
	int	21h
	mov	rechandle,ax
	call	loadrec
	endif

	mov	oldpointerx,0ffffh

	mov	ax,0
	int	33h
	mov	ax,8
	mov	cx,15
	mov	dx,184
	int	33h
	mov	ax,7
	mov	cx,15
	mov	dx,298*2
	int	33h
	ret

	endp





Readmouse	proc	near

	mov	ax,mousebutton
	mov	oldbutton,ax
	mov	ax,mousex
	mov	oldx,ax
	mov	ax,mousey
	mov	oldy,ax
	call	mousecall
	mov	mousex,cx
	mov	mousey,dx
	mov	mousebutton,bx
	ret

	endp







Mousecall	proc	near

	if	playback
	call	playmouse
	ret
	endif

	mov	ax,3
	int	33h
	shr	cx,1
	cmp	cx,298
	jc	notxover
	mov	cx,298
notxover:	cmp	cx,15
	jnc	notxover2
	mov	cx,15
notxover2:	cmp	dx,184
	jc	notyover
	mov	dx,184
notyover:	cmp	dx,15
	jnc	notyover2
	mov	dx,15
notyover2:
	if	recording
	call	recmouse
	endif
	ret

	endp






	if	playback

Playmouse	proc	near

	mov	es,recordspace
	mov	di,recordpos
	cmp	word ptr [es:di+6],0
	jnz	isthisplay
	add	di,8
	add	recordpos,8
	cmp	di,16384
	jnz	isthisplay
	call	loadrec
isthisplay:	mov	cx,[es:di]
	mov	dx,[es:di+2]
	mov	bx,[es:di+4]
	dec	word ptr [es:di+6]
	ret

	endp

	endif

	if	recording

Recmouse	proc	near

	mov	es,recordspace
	mov	di,recordpos
	cmp	di,-8
	jz	diffrec
	cmp	[es:di],cx
	jnz	diffrec
	cmp	[es:di+2],dx
	jnz	diffrec
	cmp	[es:di+4],bx
	jnz	diffrec
	inc	word ptr [es:di+6]
	cmp	word ptr [es:di+5],0ffffh
	jz	diffrec
	ret
diffrec:	add	recordpos,8
	add	di,8
	cmp	di,16384
	jnz	notsaverec
	push	cx dx bx
	call	saverec
	pop	bx dx cx
notsaverec:	mov	[es:di],cx
	mov	[es:di+2],dx
	mov	[es:di+4],bx
	mov	word ptr [es:di+6],1
	ret

	endp





Saverec	proc	near

	mov	bx,rechandle
	mov	ds,recordspace
	mov	dx,0
	mov	cx,recordpos
	add	cx,8
	mov	ah,40h
	int	21h
	mov	di,0
	mov	recordpos,0
	ret

	endp






Loadrec	proc	near

	mov	bx,rechandle
	mov	ds,recordspace
	mov	dx,0
	mov	cx,16384+8
	mov	ah,3fh
	int	21h
	mov	di,0
	mov	recordpos,0
	ret

	endp


	endif







Readmouse1	proc	near

	mov	ax,mousex
	mov	oldx,ax
	mov	ax,mousey
	mov	oldy,ax
	call	mousecall
	mov	mousex,cx
	mov	mousey,dx
	mov	mousebutton1,bx
	ret

	endp



Readmouse2	proc	near

	mov	ax,mousex
	mov	oldx,ax
	mov	ax,mousey
	mov	oldy,ax
	call	mousecall
	mov	mousex,cx
	mov	mousey,dx
	mov	mousebutton2,bx
	ret

	endp


Readmouse3	proc	near

	mov	ax,mousex
	mov	oldx,ax
	mov	ax,mousey
	mov	oldy,ax
	call	mousecall
	mov	mousex,cx
	mov	mousey,dx
	mov	mousebutton3,bx
	ret

	endp






Readmouse4	proc	near

	mov	ax,mousebutton
	mov	oldbutton,ax
	mov	ax,mousex
	mov	oldx,ax
	mov	ax,mousey
	mov	oldy,ax
	call	mousecall
	mov	mousex,cx
	mov	mousey,dx
	mov	ax,mousebutton1
	or	ax,mousebutton2
	or	ax,mousebutton3
	or	bx,ax
	mov	mousebutton,bx
	ret

	endp





Readkey 	proc	near

	mov	bx,bufferout
	cmp	bx,bufferin
	jz	nokey
	inc	bx
	and	bx,15
	mov	bufferout,bx
      	mov	di,offset cs:keybuffer
	add	di,bx
	mov	al,[cs:di]
	mov	currentkey,al
	ret
nokey:	mov	currentkey,0
	ret


	endp

keybuffer:	db	16 dup (0)



Convertkey	proc	near

	and	al,127
	mov	ah,0
	mov	di,offset cs:keyconverttab
	add	di,ax
	mov	al,[cs:di]
	ret

keyconverttab:	db	0,0,"1","2","3","4","5","6","7","8","9","0","-",0,8,0
	db	"Q","W","E","R","T","Y","U","I","O","P",0,0,13,0,"A","S"
	db	"D","F","G","H","J","K","L",0,0,0,0,0,"Z","X","C","V","B","N","M"
	db	0,0,0,0,0,0,32,0,0,0,0,0,0,0,0,0
	db	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	db	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	db	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	db	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

	endp




;-------------------------------------------------------------Miscellaneous----

Randomnum1	proc	near

	push	ds es di bx cx
	call	randomnumber
	pop	cx bx di es ds
	ret

	endp





Randomnum2	proc	near

	push	ds es di bx ax
	call	randomnumber
	mov	cl,al
	pop	ax bx di es ds
	ret

	endp





Randomnumber	proc	near

	mov	al,seed 	
	and	al,48h	
	add	al,38h		
	sal	al,1			
	sal	al,1				
	rcl	seed+2,1			
	rcl	seed+1,1			
	rcl	seed+0,1	
	mov	al,seed 	
	and	al,48h	
	add	al,38h		
	sal	al,1			
	sal	al,1				
	rcl	seed+2,1			
	rcl	seed+1,1			
	rcl	seed+0,1			
	mov	al,seed 	
	and	al,48h	
	add	al,38h		
	sal	al,1			
	sal	al,1				
	rcl	seed+2,1			
	rcl	seed+1,1			
	rcl	seed+0,1			
	mov	al,seed 	
	and	al,48h	
	add	al,38h		
	sal	al,1			
	sal	al,1				
	rcl	seed+2,1			
	rcl	seed+1,1			
	rcl	seed+0,1			
	mov	al,seed 	
	and	al,48h	
	add	al,38h		
	sal	al,1			
	sal	al,1				
	rcl	seed+2,1			
	rcl	seed+1,1			
	rcl	seed+0,1			
	mov	al,seed 	
	and	al,48h	
	add	al,38h		
	sal	al,1			
	sal	al,1				
	rcl	seed+2,1			
	rcl	seed+1,1			
	rcl	seed+0,1			
	mov	al,seed 	
	and	al,48h	
	add	al,38h		
	sal	al,1			
	sal	al,1				
	rcl	seed+2,1			
	rcl	seed+1,1			
	rcl	seed+0,1			
	mov	al,seed 	
	and	al,48h	
	add	al,38h		
	sal	al,1			
	sal	al,1				
	rcl	seed+2,1			
	rcl	seed+1,1			
	rcl	seed+0,1			
	
	mov	al,seed	
	ret

	endp






Hangon	proc	near

hangonloop:	push	cx
	call	vsync
	pop	cx
	loop	hangonloop
	ret

	endp





;-------------------------------------------------------------Disc handling----


Loadtraveltext	proc	near

	mov	dx,offset cs:traveltextname
	call	standardload
	mov	traveltext,ax
	ret

	endp






Loadintotemp	proc	near

	push	cs
	pop	ds
	call	standardload
	mov	tempgraphics,ax
	ret

	endp






Loadintotemp2	proc	near

	push	cs
	pop	ds
	call	standardload
	mov	tempgraphics2,ax
	ret

	endp




Loadintotemp3	proc	near

	push	cs
	pop	ds
	call	standardload
	mov	tempgraphics3,ax
	ret

	endp



Loadtempcharset	proc	near

	call	standardload
	mov	tempcharset,ax
	ret
	
	endp






Standardload	proc	near

	call	openfile
	call	readheader
	mov	bx,[es:di]
	push	bx
	mov	cl,4
	shr	bx,cl
	call	allocatemem
	mov	ds,ax
	pop	cx
	push	ax
	mov	dx,0
	call	readfromfile
	call	closefile
	pop	ax
	ret

	endp






Loadtemptext	proc	near

	call	standardload
	mov	textfile1,ax
	ret

	endp







Loadroom	proc	near

	mov	roomloaded,1
	mov	timecount,0
	mov	maintimer,0
	mov	mapoffsetx,104
	mov	mapoffsety,38
	mov	textaddressx,13
	mov	textaddressy,182
	mov	textlen,240
	mov	al,newlocation
	mov	location,al
	call	getroomdata
	call	startloading
	call	loadroomssample
	call	switchryanon
	call	drawflags
	call	getdimension
	ret

	endp




Loadroomssample	proc	near

	mov	al,roomssample
	cmp	al,255
	jz	loadedalready
	cmp	al,currentsample
	jz	loadedalready
	mov	currentsample,al
	mov	al,currentsample
	mov	cl,"0"
	call	twodigitnum
	mov	di,offset cs:samplename
	xchg	al,ah
	mov	[cs:di+10],ax
	mov	dx,di
	call	loadsecondsample
loadedalready:	ret

	endp





Getridofreels	proc	near

	cmp	roomloaded,0
	jz	dontgetrid
	mov	es,reel1
	call	deallocatemem
	mov	es,reel2
	call	deallocatemem
	mov	es,reel3
	call	deallocatemem
dontgetrid:	ret

	endp





Getridofall	proc	near

	mov	es,backdrop
	call	deallocatemem
	mov	es,setframes
	call	deallocatemem
	mov	es,reel1
	call	deallocatemem
	mov	es,reel2
	call	deallocatemem
	mov	es,reel3
	call	deallocatemem
	mov	es,reels
	call	deallocatemem
	mov	es,people
	call	deallocatemem
	mov	es,setdesc
	call	deallocatemem
	mov	es,blockdesc
	call	deallocatemem
	mov	es,roomdesc
	call	deallocatemem
	mov	es,freeframes
	call	deallocatemem
	mov	es,freedesc
	call	deallocatemem
	ret

	endp





Restorereels	proc	near

	cmp	roomloaded,0
	jz	dontrestore
	mov	al,reallocation
	call	getroomdata
	mov	dx,bx
	call	openfile
	call	readheader

	call	dontloadseg
	call	dontloadseg
	call	dontloadseg
	call	dontloadseg

	call	allocateload
	mov	reel1,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg
	call	allocateload
	mov	reel2,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg
	call	allocateload
	mov	reel3,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg
	call	closefile
dontrestore:	ret

	endp







Restoreall	proc	near
	
	mov	al,location
	call	getroomdata
	mov	dx,bx
	call	openfile
	call	readheader

	call	allocateload
	mov	ds,ax
	mov	backdrop,ax
	mov	dx,flags
	call	loadseg

	mov	ds,workspace ;mapdata
	mov	dx,map
	mov	cx,132*66 ;maplen
	mov	al,0
	call	fillspace
	call	loadseg
	call	sortoutmap

	call	allocateload
	mov	setframes,ax
	mov	ds,ax
	mov	dx,framedata
	call	loadseg

	;mov	ds,setdat
	;mov	dx,0
	;mov	cx,setdatlen
	;mov	al,255
	;call	fillspace
	call	dontloadseg

	call	allocateload
	mov	reel1,ax
	mov	ds,ax
	mov	dx,0
	;call	bloc
	;BIG FIXME: undefined bloc, replaced with loadseg. dunno!
	call	loadseg
	call	allocateload
	mov	reel2,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg
	call	allocateload
	mov	reel3,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg

	call	allocateload
	mov	reels,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg

	call	allocateload
	mov	people,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg

	call	allocateload
	mov	setdesc,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg

	call	allocateload
	mov	blockdesc,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg

	call	allocateload
	mov	roomdesc,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg

	call	allocateload
	mov	freeframes,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg

	;mov	ds,freedat
	;mov	dx,0
	;mov	cx,freedatlen
	;mov	al,255
	;call	fillspace
	call	dontloadseg

	call	allocateload
	mov	freedesc,ax
	mov	ds,ax
	mov	dx,freetextdat
	call	loadseg

	call	closefile
	
	call	setallchanges
	ret

	endp



Sortoutmap	proc	near
	
	push	es di
	mov	ds,workspace
	mov	si,0
	mov	es,mapdata
	mov	di,0

	mov	cx,maplength
blimey:	push	cx si
	mov	cx,mapwidth
	rep	movsb
	pop	si cx
	add	si,132
	loop	blimey
	pop	di es
	ret

	endp




Startloading	proc	near

	mov	combatcount,0
	mov	al,[cs:bx+13]
	mov	roomssample,al
	mov	al,[cs:bx+15]
	mov	mapx,al
	mov	al,[cs:bx+16]
	mov	mapy,al

	mov	al,[cs:bx+20]		; start path pos
	mov	liftflag,al
	mov	al,[cs:bx+21]		; start path pos
	mov	manspath,al
	mov	destination,al
	mov	finaldest,al
	mov	al,[cs:bx+22]
	mov	facing,al
	mov	turntoface,al
	mov	al,[cs:bx+23]
	mov	counttoopen,al
	mov	al,[cs:bx+24]
	mov	liftpath,al
	mov	al,[cs:bx+25]
	mov	doorpath,al
	mov	lastweapon,-1
	mov	al,[cs:bx+27]
	push	ax

	mov	al,[cs:bx+31]
	mov	ah,reallocation
	mov	reallocation,al

	mov	dx,bx
	call	openfile
	call	readheader

	call	allocateload
	mov	ds,ax
	mov	backdrop,ax
	mov	dx,flags
	call	loadseg

	mov	ds,workspace ;mapdata
	mov	dx,map
	mov	cx,132*66 ;maplen
	mov	al,0
	call	fillspace
	call	loadseg
	call	sortoutmap

	call	allocateload
	mov	setframes,ax
	mov	ds,ax
	mov	dx,framedata
	call	loadseg

	mov	ds,setdat
	mov	dx,0
	mov	cx,setdatlen
	mov	al,255
	call	fillspace
	call	loadseg

	call	allocateload
	mov	reel1,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg
	call	allocateload
	mov	reel2,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg
	call	allocateload
	mov	reel3,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg

	call	allocateload
	mov	reels,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg

	call	allocateload
	mov	people,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg

	call	allocateload
	mov	setdesc,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg

	call	allocateload
	mov	blockdesc,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg

	call	allocateload
	mov	roomdesc,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg

	call	allocateload
	mov	freeframes,ax
	mov	ds,ax
	mov	dx,0
	call	loadseg

	mov	ds,freedat
	mov	dx,0
	mov	cx,freedatlen
	mov	al,255
	call	fillspace
	call	loadseg

	call	allocateload
	mov	freedesc,ax
	mov	ds,ax
	mov	dx,freetextdat
	call	loadseg

	call	closefile


	call	findroominloc
	call	deletetaken
	call	setallchanges
	call	autoappear
	mov	al,newlocation
	call	getroomdata
	mov	lastweapon,-1
	mov	mandead,0
	mov	lookcounter,160
	mov	newlocation,255
	mov	linepointer,254
	pop	ax
	cmp	al,255
	jz	dontwalkin
           	mov	manspath,al
	push	bx
	call	autosetwalk
	pop	bx
dontwalkin:	call	findxyfrompath
	ret

	endp










Disablepath	proc	near		;needs al,ah map x,y cl=path

	push	cx
	xchg	al,ah
	mov	cx,-6
looky2: 	add	cx,6
	sub	al,10
	jnc	looky2
	mov	al,ah
	dec	cx
lookx2: 	inc	cx
	sub	al,11
	jnc	lookx2
	mov	al,cl
	mov	ah,0
	mov	cx,144
	mul	cx
	mov	es,reels
	mov	bx,pathdata
	add	bx,ax
	pop	ax
	mov	ah,0
	add	ax,ax
	add	ax,ax
	add	ax,ax
	add	bx,ax
	mov	al,0
	mov	[es:bx+6],al
	ret

	endp








Findxyfrompath	proc	near		;path number was found from
				;room data. Fill ryanxy from
	call	getroomspaths		;the pathdata.
	mov	al,manspath
	mov	ah,0
	add	ax,ax
	add	ax,ax
	add	ax,ax
	add	bx,ax
	mov	ax,[es:bx]
	sub	al,12
	sub	ah,12
	mov	ryanx,al
	mov	ryany,ah
	ret

	endp





Findroominloc	proc	near

	mov	al,mapy
	mov	cx,-6
looky:	add	cx,6
	sub	al,10
	jnc	looky
	mov	al,mapx
	dec	cx
lookx:	inc	cx
	sub	al,11
	jnc	lookx
	mov	roomnum,cl
	ret

	endp






Getroomdata	proc	near

	mov	ah,0
	mov	cx,32
	mul	cx
	mov	bx,offset cs:roomdata
	add	bx,ax
	ret

	endp










	


Readheader	proc	near

	push	cs
	pop	ds
	mov	dx,offset cs:fileheader
	mov	cx,headerlen
	call	readfromfile
	push	cs
	pop	es
	mov	di,offset cs:filedata
	ret

	endp
	
	






Dontloadseg	proc	neqr

	mov	ax,[es:di]
	add	di,2
	push	bx di es
	mov	cx,0
	mov	dx,ax
	mov	al,1
	mov	ah,42h
	int	21h
	pop	es di bx
	ret

	endp






Allocateload	proc	near

	push	es di
	mov	bx,[es:di]
	mov	cl,4
	shr	bx,cl
	call	allocatemem
	pop	di es
	ret

	endp




Fillspace	proc	near

	push	es ds dx di bx
	mov	di,dx
	push	ds
	pop	es
	rep	stosb
	pop	bx di dx ds es
	ret

	endp







Getridoftemp	proc	near

	mov	es,tempgraphics
	call	deallocatemem
	ret

	endp





Getridoftemptext	proc	near

	mov	es,textfile1
	call	deallocatemem
	ret

	endp





Getridoftemp2	proc	near

	mov	es,tempgraphics2
	call	deallocatemem
	ret

	endp



Getridoftemp3	proc	near

	mov	es,tempgraphics3
	call	deallocatemem
	ret

	endp



Getridoftempcharset	proc	near

	mov	es,tempcharset
	call	deallocatemem
	ret

	endp



Getridoftempsp	proc	near

	mov	es,tempsprites
	call	deallocatemem
	ret

	endp






















	
Readsetdata	proc	near

	mov	dx,offset cs:characterset1
	call	standardload
	mov	charset1,ax

	mov	dx,offset cs:icongraphics0
	call	standardload
	mov	icons1,ax

	mov	dx,offset cs:icongraphics1
	call	standardload
	mov	icons2,ax

	mov	dx,offset cs:spritename1
	call	standardload
	mov	mainsprites,ax

	mov	dx,offset cs:puzzletextname
	call	standardload
	mov	puzzletext,ax

	mov	dx,offset cs:commandtextname
	call	standardload
	mov	commandtext,ax

	mov	ax,charset1
	mov	currentset,ax

	cmp	soundint,255
	jz	novolumeload
	mov	dx,offset cs:volumetabname
	call	openfile
	mov	cx,2048-256
	mov	ds,soundbuffer
	mov	dx,16384
	call	readfromfile
	call	closefile
novolumeload:	ret

	endp











Createfile	proc	near

	mov	ah,3ch
	mov	cx,0
	int	21h
	mov	bx,ax
	ret

	endp







Openfile	proc	near

	if	cd
	call	makename
	endif
	push	cs
	pop	ds
	mov	ah,3dh
	mov	al,2
	push	dx
	int	21h
	pop	dx
	jc	fileerror
	mov	handle,ax
	ret
fileerror:	mov	gameerror,8
	jmp	quickquit2

	endp


	if	cd
Openfilefromc	proc	near

	push	cs
	pop	ds
	mov	ah,3dh
	mov	al,2
	push	dx
	int	21h
	pop	dx
	mov	handle,ax
	ret
	
	endp
	endif


	if	cd
Makename	proc	near

	if	demo
	ret
	endif
	mov	si,dx
	mov	di,offset cs:place
transfer:	mov	al,[cs:si]
	mov	[cs:di],al
	inc	si
	inc	di
	cmp	al,0
	jnz	transfer
	mov	dx,offset cs:id
	ret
id:	db	"D:\"
place:	db	30 dup (0)

	endp
	endif





Openfilenocheck	proc	near

	if	cd
	call	makename
	endif
	push	cs
	pop	ds
	mov	ah,3dh
	mov	al,2
	int	21h
	mov	handle,ax
	ret

	endp



Openforsave	proc	near

	mov	cx,0
	mov	ah,3ch
	mov	al,2
	int	21h
	mov	handle,ax
	ret

	endp



Closefile	proc	near

	mov	bx,handle
	mov	ah,3eh
	int	21h
	ret

	endp




Readfromfile	proc	near

	mov	bx,handle
	mov	ah,3fh
	int	21h
	ret

	endp




Setkeyboardint	proc	near

	mov	ah,35h
	mov	al,9
	int	21h
	mov	oldint9seg,es		; Save es:bx to temp memory
	mov	oldint9add,bx
	push	cs
	pop	ds
	mov	dx,offset cs:keyboardread
	mov	ah,25h
	mov	al,9
	int	21h		; Set to new
	ret

	endp




Resetkeyboard	proc	near

	cmp	oldint9add,-1
	jz	noreset   
	mov	dx,oldint9add		;Restore old interupt vector
	mov	ax,oldint9seg		;for keys
	mov	ds,ax
	mov	ah,25h
	mov	al,9
	int	21h
noreset:	ret

	endp






















Keyboardread	proc	near

	push	ax dx di ds es
	in	al,60h
	cmp	al,lasthardkey
	jz	same
	mov	lasthardkey,al
	cmp	al,128
	jnc	same
	mov	dx,bufferin
	inc	dx
	and	dx,15
	cmp	dx,bufferout
	jz	same	;buffer is full
	mov	bufferin,dx
	call	convertkey
	mov	di,offset cs:keybuffer
	mov	dx,bufferin
	add	di,dx
	mov	[cs:di],al	
same:	in	al,61h
	mov	ah,al
	or	al,80h		; Mask for Akn
	out	61h,al		; Set Akn.
	and	al,7fh
	out	61h,al
	cli
	mov	al,20h		; 8259 end of interrupt
	out	20h,al
	pop	es ds di dx ax
	iret

	endp



;------------------------------------------------------Text and tables data----



Fileheader	db	"DREAMWEB DATA FILE "
	db	"COPYRIGHT 1992 "
	db	"CREATIVE REALITY"
Filedata	dw	20 dup (0)
Extradata	db	6 dup (0)
Headerlen	equ	96   ; $-Fileheader


Roomdata	db	"DREAMWEB.R00",0	;Ryan's apartment
	db	5,255,33,10
	db	255,255,255,0
	db	1,6,2,255,3,255,255,255,255,255,0

	db	"DREAMWEB.R01",0
	db	1,255,44,10
	db	255,255,255,0
	db	7,2,255,255,255,255,6,255,255,255,1

	db	"DREAMWEB.R02",0
	db	2,255,33,0
	db	255,255,255,0
	db	1,0,255,255,1,255,3,255,255,255,2

	db	"DREAMWEB.R03",0
	db	5,255,33,10
	db	255,255,255,0
	db	2,2,0,2,4,255,0,255,255,255,3

	db	"DREAMWEB.R04",0
	db	23,255,11,30
	db	255,255,255,0
	db	1,4,0,5,255,255,3,255,255,255,4

	db	"DREAMWEB.R05",0
	if	demo
	db	22,255,22,30
	else
	db	5,255,22,30
	endif
	db	255,255,255,0
	db	1,2,0,4,255,255,3,255,255,255,5

	db	"DREAMWEB.R06",0
	db	5,255,11,30
	db	255,255,255,0
	db	1,0,0,1,2,255,0,255,255,255,6

	db	"DREAMWEB.R07",0
	db	255,255,0,20
	db	255,255,255,0
	db	2,2,255,255,255,255,0,255,255,255,7

	db	"DREAMWEB.R08",0
	db	8,255,0,10
	db	255,255,255,0
	db	1,2,255,255,255,255,0,11,40,0,8

	db	"DREAMWEB.R09",0
	db	9,255,22,10
	db	255,255,255,0
	db	4,6,255,255,255,255,0,255,255,255,9

	db	"DREAMWEB.R10",0
	db	10,255,33,30
	db	255,255,255,0
	db	2,0,255,255,2,2,4,22,30,255,10	;22,30,0 switches
				;off path 0 in skip
	db	"DREAMWEB.R11",0
	db	11,255,11,20
	db	255,255,255,0
	db	0,4,255,255,255,255,255,255,255,255,11

	db	"DREAMWEB.R12",0
	db	12,255,22,20
	db	255,255,255,0
	db	1,4,255,255,255,255,255,255,255,255,12

	db	"DREAMWEB.R13",0
	db	12,255,22,20
	db	255,255,255,0
	db	1,4,255,255,255,255,255,255,255,255,13

	db	"DREAMWEB.R14",0
	db	14,255,44,20
	db	255,255,255,0
	db	0,6,255,255,255,255,255,255,255,255,14

	db	32 dup (0)
	db	32 dup (0)
	db	32 dup (0)
	db	32 dup (0)
	
	db	"DREAMWEB.R19",0
	db	19,255,0,0
	db	255,255,255,0
	db	0,4,255,255,255,255,255,255,255,255,19

	db	"DREAMWEB.R20",0
	db	22,255,0,20
	db	255,255,255,0
	db	1,4,2,15,255,255,255,255,255,255,20

	db	"DREAMWEB.R21",0
	if	demo
	db	22,255,11,10
	else
	db	5,255,11,10
	endif
	db	255,255,255,0
	db	1,4,2,15,1,255,255,255,255,255,21

	db	"DREAMWEB.R22",0
	db	22,255,22,10
	db	255,255,255,0
	db	0,4,255,255,1,255,255,255,255,255,22

	db	"DREAMWEB.R23",0
	db	23,255,22,30
	db	255,255,255,0
	db	1,4,2,15,3,255,255,255,255,255,23

	db	"DREAMWEB.R24",0
	db	5,255,44,0
	db	255,255,255,0
	db	1,6,2,15,255,255,255,255,255,255,24

	db	"DREAMWEB.R25",0
	db	22,255,11,40
	db	255,255,255,0
	db	1,0,255,255,255,255,255,255,255,255,25

	db	"DREAMWEB.R26",0
	db	9,255,22,20
	db	255,255,255,0
	db	4,2,255,255,255,255,255,255,255,255,26

	db	"DREAMWEB.R27",0
	db	22,255,11,20
	db	255,255,255,0
	db	0,6,255,255,255,255,255,255,255,255,27

	db	"DREAMWEB.R28",0
	db	5,255,11,30
	db	255,255,255,0
	db	0,0,255,255,2,255,255,255,255,255,28

	db	"DREAMWEB.R29",0
	db	22,255,11,10
	db	255,255,255,0
	db	0,2,255,255,255,255,255,255,255,255,29



	db	"DREAMWEB.R05",0	;Duplicate of hotel lobby,
	if	demo
	db	22,255,22,10 	;but emerging from the lift.
	else
	db	5,255,22,10
	endif
	db	255,255,255,0
	db	1,4,1,15,255,255,255,255,255,255,5

	db	"DREAMWEB.R04",0	;Duplicate of pool hall lobby,
	db	23,255,22,20 	;but emerging from the lift.
	db	255,255,255,0
	db	1,4,2,15,255,255,255,255,255,255,4

	db	"DREAMWEB.R10",0	;entering alley via skip
	db	10,255,22,30
	db	255,255,255,0
	db	3,6,255,255,255,255,255,255,255,255,10

	db	"DREAMWEB.R12",0	;on the beach, getting up.
	db	12,255,22,20
	db	255,255,255,0
	db	0,2,255,255,255,255,255,255,255,255,12

	db	"DREAMWEB.R03",0	;Duplicate of Eden's lobby
	db	5,255,44,0   	;but emerging from the lift
	db	255,255,255,0
	db	1,6,2,255,4,255,255,255,255,255,3

	db	"DREAMWEB.R24",0	;Duplicate of Eden's flat
	db	5,255,22,0	 ;but starting on the bed
	db	255,255,255,0
	db	3,6,0,255,255,255,255,33,0,3,24	; 33,0,3 turns off
					; path for lift
	db	"DREAMWEB.R22",0	;Duplicate
	db	22,255,22,20	;of hotel but in pool room
	db	255,255,255,0
	db	1,4,255,255,255,255,255,255,255,255,22

	db	"DREAMWEB.R22",0	;Duplicate
	db	22,255,22,20	;of hotel but in pool room
	db	255,255,255,0	;coming out of bedroom
	db	0,2,255,255,255,255,255,255,255,255,22

	db	"DREAMWEB.R11",0	;Duplicate
	db	11,255,22,30	;of carpark but getting
	db	255,255,255,0	;up off the floor
	db	0,0,255,255,255,255,255,255,255,255,11

	db	"DREAMWEB.R28",0
	db	5,255,11,20
	db	255,255,255,0
	db	0,6,255,255,2,255,255,255,255,255,28

	db	"DREAMWEB.R21",0
	if	demo
	db	22,255,11,10
	else
	db	5,255,11,10
	endif
	db	255,255,255,0
	db	1,4,2,15,1,255,255,255,255,255,21

	db	"DREAMWEB.R26",0
	db	9,255,0,40
	db	255,255,255,0
	db	0,0,255,255,255,255,255,255,255,255,26

	db	"DREAMWEB.R19",0
	db	19,255,0,0
	db	255,255,255,0
	db	2,2,255,255,255,255,255,255,255,255,19

	db	"DREAMWEB.R08",0		;leaving tvstudio into street
	db	8,255,11,40
	db	255,255,255,0
	db	0,4,255,255,255,255,255,255,255,255,8

	db	"DREAMWEB.R01",0
	db	1,255,44,10
	db	255,255,255,0
	db	3,6,255,255,255,255,255,255,255,255,1



	db	"DREAMWEB.R45",0	;Dream room
 	db	35,255,22,30
	db	255,255,255,0
	db	0,6,255,255,255,255,255,255,255,255,45

	db	"DREAMWEB.R46",0	;Dream room
	db	35,255,22,40
	db	255,255,255,0
	db	0,4,255,255,255,255,255,255,255,255,46

	db	"DREAMWEB.R47",0	;Dream room
	db	35,255,0,0
	db	255,255,255,0
	db	0,0,255,255,255,255,255,255,255,255,47

	db	"DREAMWEB.R45",0	;Dream room
	db	35,255,22,30
	db	255,255,255,0
	db	4,0,255,255,255,255,255,255,255,255,45

	db	"DREAMWEB.R46",0	;Dream room
	db	35,255,22,50
	db	255,255,255,0
	db	0,4,255,255,255,255,255,255,255,255,46



	db	"DREAMWEB.R50",0 ; Intro sequence one
	db	35,255,22,30
	db	255,255,255,0
	db	0,0,255,255,255,255,255,255,255,255,50

	db	"DREAMWEB.R51",0 ; Intro sequence two
	db	35,255,11,30
	db	255,255,255,0
	db	0,0,255,255,255,255,255,255,255,255,51

	db	"DREAMWEB.R52",0 ; Intro sequence three
	db	35,255,22,30
	db	255,255,255,0
	db	0,0,255,255,255,255,255,255,255,255,52

	db	"DREAMWEB.R53",0 ; Intro sequence four
	db	35,255,33,0
	db	255,255,255,0
	db	0,0,255,255,255,255,255,255,255,255,53

	db	"DREAMWEB.R54",0 ; Intro sequence five - wasteland
	db	35,255,0,0
	db	255,255,255,0
	db	0,0,255,255,255,255,255,255,255,255,54

	db	"DREAMWEB.R55",0 ; End sequence
	db	14,255,44,0
	db	255,255,255,0
	db	0,0,255,255,255,255,255,255,255,255,55

	
Madeuproomdat	db	32 dup (0)

Roomscango	db	1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0

Roompics	db	5,0,3,2,4,1,10,9,8,6,11,4,7,7,0

Oplist	db	3 dup (0)

Inputline	db	128 dup (0)

linedata	dw	200 dup (0ffffh)

presslist	db	6 dup (255)

savenames	db	2,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
	db	2,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
	db	2,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
	db	2,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
	db	2,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
	db	2,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
	db	2,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1



savefiles	db	"DREAMWEB.D00",0
	db	"DREAMWEB.D01",0
	db	"DREAMWEB.D02",0
	db	"DREAMWEB.D03",0
	db	"DREAMWEB.D04",0
	db	"DREAMWEB.D05",0
	db	"DREAMWEB.D06",0

Recname	db	"DREAMWEB.DEM",0

Quitrequested	db 0
Subtitles		db 0
ForeignRelease	db 0


;-------------------------------------------------------End of code segment----

DREAMWEBPROG	ends




;---------------------------------------------------------------Stack space----s

STACKSPACE	segment para stack 'STACK'

stak	db	256 dup (?)

STACKSPACE	ends



;-----------------------------------------------------------End of all code----

	end	Dreamweb
