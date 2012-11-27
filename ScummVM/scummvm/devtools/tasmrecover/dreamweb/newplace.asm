;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text
;----------------------------------------------------Choosing a new location----

Newplace	proc	near

	cmp	needtotravel,1
	jz	istravel
	cmp	autolocation,-1
	jnz	isautoloc
	ret

isautoloc:	mov	al,autolocation
	mov	newlocation,al
	mov	autolocation,-1
	ret

istravel:	mov	needtotravel,0
	call	selectlocation
	ret

	endp




Selectlocation	proc	near

	mov	inmaparea,0
	call	clearbeforeload
	mov	getback,0
	mov	pointerframe,22
	
	call	readcitypic
	call	showcity       
	call	getridoftemp
	call	readdesticon
	call	loadtraveltext
	call	showpanel
	call	showman
	call	showarrows
	call	showexit
	call	locationpic
	call	undertextline
	mov	commandtype,255
	call	readmouse
	mov	pointerframe,0
	call	showpointer
	call	worktoscreen
	mov	al,9
	mov	ah,255
	call	playchannel0
	mov	newlocation,255

select:
	cmp quitrequested, 0
	jnz quittravel
	call	delpointer
	call	readmouse
	call	showpointer
	call	vsync
	call	dumppointer
	call	dumptextline
	cmp	getback,1
	jz	quittravel
	mov	bx,offset cs:destlist
	call	checkcoords
	cmp	newlocation,255
	jz	select
	mov	al,newlocation
	cmp	al,location
	jz	quittravel

	call	getridoftemp
	call	getridoftemp2
	call	getridoftemp3
	mov	es,traveltext
	call	deallocatemem
	ret

quittravel:	mov	al,reallocation ; was just location
	mov	newlocation,al
	mov	getback,0
	call	getridoftemp
	call	getridoftemp2
	call	getridoftemp3
	mov	es,traveltext
	call	deallocatemem
	ret

destlist:	dw	238,258,4,44,nextdest
	dw	104,124,4,44,lastdest
	dw	280,308,4,44,lookatplace
	dw	104,216,138,192,destselect
	dw	273,320,157,198,getback1
	dw	0,320,0,200,blank
	dw	0ffffh

	endp





Showcity	proc	near

	call	clearwork
	mov	ds,tempgraphics
	mov	di,57
	mov	bx,32
	mov	al,0
	mov	ah,0
	call	showframe
	mov	ds,tempgraphics
	mov	di,120+57
	mov	bx,32
	mov	al,1
	mov	ah,0
	call	showframe
	ret

	endp


	
	
	
Lookatplace	proc	near

	cmp	commandtype,224
	jz	alreadyinfo
	mov	commandtype,224
	mov	al,27
	call	commandonly
alreadyinfo:	mov	ax,mousebutton
	and	ax,1
	jz	noinfo
	cmp	ax,oldbutton
	jz	noinfo

	mov	bl,destpos
	cmp	bl,15
	jnc	noinfo

	push	bx
	call	delpointer
	call	deltextline
	call	getundercentre
	mov	ds,tempgraphics3
	mov	al,0
	mov	ah,0
	mov	di,60
	mov	bx,72
	call	showframe
	mov	al,4
	mov	ah,0
	mov	di,60
	mov	bx,72+55
	call	showframe
	cmp	foreignrelease, 0
	jz $1
	mov	al,4
	mov	ah,0
	mov	di,60
	mov	bx,72+55+21
	call	showframe
$1:
	pop	bx

	mov	bh,0
	add	bx,bx
	mov	es,traveltext
	mov	si,[es:bx]
	add	si,textstart
	call	findnextcolon

	mov	di,63
	mov	bx,84
	cmp	foreignrelease, 0
	jz $2
	mov	bx,84+4
$2:
	mov	dl,191
	mov	al,0
	mov	ah,0
	call	printdirect 
	call	worktoscreenm

	mov	cx,500
	call	hangonp

afterinfo:	mov	pointermode,0
	mov	pointerframe,0
	call	putundercentre
	call	worktoscreenm

noinfo:	ret

	endp




Getundercentre	proc	near

	mov	di,58
	mov	bx,72
	mov	ds,mapstore
	mov	si,0
	mov	cl,254
	mov	ch,110
	call	multiget
	ret

	endp










Putundercentre	proc	near

	mov	di,58
	mov	bx,72
	mov	ds,mapstore
	mov	si,0
	mov	cl,254
	mov	ch,110
	call	multiput
	ret

	endp







Locationpic	proc	near

	call	getdestinfo
	mov	al,[es:si]
	push	es si
	mov	di,0
	cmp	al,6
	jnc	secondlot
	mov	ds,tempgraphics
	add	al,4
	jmp	gotgraphic
secondlot:	sub	al,6
	mov	ds,tempgraphics2
gotgraphic:	add	di,104
	mov	bx,138+14
	mov	ah,0
	call	showframe
	pop	si es
	mov	al,destpos
	cmp	al,reallocation
	jnz	notinthisone
	mov	al,3
	mov	di,104
	mov	bx,140+14
	mov	ds,tempgraphics
	mov	ah,0
	call	showframe
notinthisone:	mov	bl,destpos
	mov	bh,0
	add	bx,bx
	mov	es,traveltext
	mov	si,[es:bx]
	add	si,textstart
	mov	di,50
	mov	bx,20
	mov	dl,241
	mov	al,0
	mov	ah,0
	call	printdirect
	ret

	endp




Getdestinfo	proc	near

	mov	al,destpos
	mov	ah,0
	push	ax
	mov	dx,seg roomscango
	mov	es,dx
	mov	si,offset es:roomscango
	add	si,ax
	mov	cl,[es:si]
	pop	ax
	push	cx
	mov	dx,seg roompics
	mov	es,dx
	mov	si,offset es:roompics
	add	si,ax
	pop	ax
	ret

	endp






Showarrows	proc	near

	mov	di,116-12
	mov	bx,16
	mov	ds,tempgraphics
	mov	al,0
	mov	ah,0
	call	showframe
	mov	di,226+12
	mov	bx,16
	mov	ds,tempgraphics
	mov	al,1
	mov	ah,0
	call	showframe
	mov	di,280
	mov	bx,14
	mov	ds,tempgraphics
	mov	al,2
	mov	ah,0
	call	showframe
	ret

	endp









Nextdest	proc	near

duok:	cmp	commandtype,218
	jz	alreadydu
	mov	commandtype,218
	mov	al,28
	call	commandonly
alreadydu:	mov	ax,mousebutton
	and	ax,1
	jz	nodu
	cmp	ax,oldbutton
	jz	nodu

searchdestup:	inc	destpos
	cmp	destpos,15
	jnz	notlastdest
	mov	destpos,0
notlastdest:	call	getdestinfo
	cmp	al,0
	jz	searchdestup

	mov	newtextline,1
	call	deltextline
	call	delpointer
	call	showpanel
	call	showman
	call	showarrows
	call	locationpic
	call	undertextline
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
nodu:	ret

	endp







Lastdest	proc	near

ddok:	cmp	commandtype,219
	jz	alreadydd
	mov	commandtype,219
	mov	al,29
	call	commandonly
alreadydd:	mov	ax,mousebutton
	and	ax,1
	jz	nodd
	cmp	ax,oldbutton
	jz	nodd

searchdestdown: dec	destpos
	cmp	destpos,-1
	jnz	notfirstdest
	mov	destpos,15
notfirstdest:	call	getdestinfo
	cmp	al,0
	jz	searchdestdown

	mov	newtextline,1
	call	deltextline
	call	delpointer
	call	showpanel
	call	showman
	call	showarrows
	call	locationpic
	call	undertextline
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
nodd:	ret

	endp








Destselect	proc	near

	cmp	commandtype,222
	jz	alreadytrav
	mov	commandtype,222
	mov	al,30
	call	commandonly
alreadytrav:	mov	ax,mousebutton
	and	ax,1
	jz	notrav
	cmp	ax,oldbutton
	jz	notrav

	call	getdestinfo
	mov	al,destpos
	mov	newlocation,al
notrav: 	ret

	endp



















Getlocation	proc	near

	mov	ah,0
	mov	bx,ax
	mov	dx,seg roomscango
	mov	es,dx
	add	bx,offset es:roomscango
	mov	al,[es:bx]
	ret

	endp


Setlocation	proc	near		;makes a location accessable

	mov	ah,0
	mov	bx,ax
	mov	dx,seg roomscango
	mov	es,dx
	add	bx,offset es:roomscango
	mov	byte ptr [es:bx],1
	ret

	endp




Resetlocation	proc	near		;makes a location inaccessable

	push	ax
	cmp	al,5
	jnz	notdelhotel
	call	purgealocation
	mov	al,21
	call	purgealocation
	mov	al,22
	call	purgealocation
	mov	al,27
	call	purgealocation
	jmp	clearedlocations

notdelhotel:	cmp	al,8
	jnz	notdeltvstud
	call	purgealocation
	mov	al,28
	call	purgealocation
	jmp	clearedlocations

notdeltvstud:	cmp	al,6
	jnz	notdelsarters
	call	purgealocation
	mov	al,20
	call	purgealocation
	mov	al,25
	call	purgealocation
	jmp	clearedlocations

notdelsarters:	cmp	al,13
	jnz	notdelboathouse
	call	purgealocation
	mov	al,29
	call	purgealocation
	jmp	clearedlocations

notdelboathouse:

clearedlocations:	pop	ax
	mov	ah,0
	mov	bx,ax
	mov	dx,seg roomscango
	mov	es,dx
	add	bx,offset es:roomscango
	mov	byte ptr [es:bx],0
	ret

	endp




Readdesticon	proc	near

	mov	dx,offset cs:travelgraphic1
	call	loadintotemp

	mov	dx,offset cs:travelgraphic2
	call	loadintotemp2
	
	mov	dx,offset cs:icongraphics8
	call	loadintotemp3
	ret

	endp




Readcitypic	proc	near

	mov	dx,offset cs:cityname
	call	loadintotemp
	ret

	endp
