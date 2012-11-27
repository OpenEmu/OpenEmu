;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text
Usemon	proc	near

	mov	lasttrigger,0

	push	cs			;start off with no file name
	pop	es
	mov	di,offset cs:currentfile+1
	mov	cx,12
	mov	al,32
	rep	stosb
	push	cs			;start off with no file name
	pop	es
	mov	di,offset cs:operand1+1
	mov	cx,12
	mov	al,32
	rep	stosb

	push	cs			;clear all keys
	pop	es
	mov	di,offset cs:keys
	mov	byte ptr [es:di],1
	add	di,26
	mov	cx,3
keyloop:	mov	byte ptr [es:di],0
	add	di,26
	loop	keyloop

	call	createpanel
	call	showpanel
	call	showicon
	call	drawfloor
	call	getridofall ;reels

	mov	dx,offset cs:mongraphicname
	call	loadintotemp
	call	loadpersonal
	call	loadnews
	call	loadcart
	mov	dx,offset cs:characterset2
	call	loadtempcharset

	call	printoutermon
 	call	initialmoncols
	call	printlogo
	call	worktoscreen
	call	turnonpower
	call	fadeupyellows
	call	fadeupmonfirst

	mov	monadx,76
	mov	monady,141
	mov	al,1
	call	monmessage	
	mov	cx,120
	call	hangoncurs
	mov	al,2
	call	monmessage
	mov	cx,60
	call	randomaccess
	mov	al,3
	call	monmessage
	mov	cx,100
	call	hangoncurs
	call	printlogo
	call	scrollmonitor
	mov	bufferin,0
	mov	bufferout,0

moreinput:	mov	di,monadx
	mov	bx,monady
	push	di bx
	call	input
	pop	bx di
	mov	monadx,di
	mov	monady,bx
	call	execcommand
	cmp	al,0
	jz	moreinput

endmon:	call	getridoftemp
	call	getridoftempcharset
	mov	es,textfile1
	call	deallocatemem
	mov	es,textfile2
	call	deallocatemem
	mov	es,textfile3
	call	deallocatemem
	mov	getback,1
	mov	al,26
	call	playchannel1
	mov	manisoffscreen,0
	call	restoreall ;reels
	call	redrawmainscrn
	call	worktoscreenm
	ret

	endp






Printoutermon	proc	near

	mov	di,40
	mov	bx,32
	mov	ds,tempgraphics
	mov	al,1
	mov	ah,0
	call	showframe
	mov	di,264
	mov	bx,32
	mov	ds,tempgraphics
	mov	al,2
	mov	ah,0
	call	showframe
	mov	di,40
	mov	bx,12
	mov	ds,tempgraphics
	mov	al,3
	mov	ah,0
	call	showframe
	mov	di,40
	mov	bx,164
	mov	ds,tempgraphics
	mov	al,4
	mov	ah,0
	call	showframe
	ret

	endp







Loadpersonal	proc	near

	mov	al,location
	mov	dx,offset cs:monitorfile1
	cmp	al,0
	jz	foundpersonal
	cmp	al,42
	jz	foundpersonal
	mov	dx,offset cs:monitorfile2
	cmp	al,2
	jz	foundpersonal

foundpersonal:	call	openfile
	call	readheader
	mov	bx,[es:di]
	push	bx
	mov	cl,4
	shr	bx,cl
	call	allocatemem
	mov	textfile1,ax
	mov	ds,ax
	pop	cx
	mov	dx,0
	call	readfromfile
	call	closefile
	ret

	endp




Loadnews	proc	near		;textfile2 holds information
				;accessable by anyone
	mov	al,newsitem
	mov	dx,offset cs:monitorfile10
	cmp	al,0
	jz	foundnews
	mov	dx,offset cs:monitorfile11
	cmp	al,1
	jz	foundnews
	mov	dx,offset cs:monitorfile12
	cmp	al,2
	jz	foundnews
	mov	dx,offset cs:monitorfile13
foundnews:	call	openfile
	call	readheader
	mov	bx,[es:di]
	push	bx
	mov	cl,4
	shr	bx,cl
	call	allocatemem
	mov	textfile2,ax
	mov	ds,ax
	pop	cx
	mov	dx,0
	call	readfromfile
	call	closefile
	ret

	endp






Loadcart	proc	near

	call	lookininterface

	mov	dx,offset cs:monitorfile20
	cmp	al,0
	jz	gotcart
	mov	dx,offset cs:monitorfile21
	cmp	al,1
	jz	gotcart
	mov	dx,offset cs:monitorfile22
	cmp	al,2
	jz	gotcart
	mov	dx,offset cs:monitorfile23
	cmp	al,3
	jz	gotcart
	mov	dx,offset cs:monitorfile24
gotcart:	call	openfile
	call	readheader
	mov	bx,[es:di]
	push	bx
	mov	cl,4
	shr	bx,cl
	call	allocatemem
	mov	textfile3,ax
	mov	ds,ax
	pop	cx
	mov	dx,0
	call	readfromfile
	call	closefile
	ret

	endp






Lookininterface proc	near

	mov	al,"I"
	mov	ah,"N"
	mov	cl,"T"
	mov	ch,"F"
	call	findsetobject	;this bit searches set obs
			;until the interface is found
			;al holds object number
	mov	ah,1			;ah holds type, 1=set object
	call	checkinside		;this searches for any extra
				;object inside the interface..
	cmp	cl,numexobjects
	jz	emptyinterface
	mov	al,[es:bx+15]	;get the last letter of ID code
	inc	al
	ret
emptyinterface: 	mov	al,0
	ret

	endp







Turnonpower	proc	near

	mov	cx,3
powerloop:	push	cx
	call	powerlighton
	mov	cx,30
	call	hangon
	call	powerlightoff
	mov	cx,30
	call	hangon
	pop	cx
	loop	powerloop
	call	powerlighton
	ret

	endp





Randomaccess	proc	near

accessloop:	push	cx
	call	vsync
	call	vsync
	call	randomnum1
	and	al,15
	cmp	al,10
	jc	off
	call	accesslighton
	jmp	chosenaccess
off:	call	accesslightoff
chosenaccess:	pop	cx
	loop	accessloop
	call	accesslightoff
	ret

	endp



Powerlighton	proc	near

	mov	di,257+4
	mov	bx,182
	mov	ds,tempgraphics
	mov	al,6
	mov	ah,0
	push	di bx
	call	showframe
	pop	bx di
	mov	cl,12
	mov	ch,8
	call	multidump
	ret

	endp




Powerlightoff	proc	near

	mov	di,257+4
	mov	bx,182
	mov	ds,tempgraphics
	mov	al,5
	mov	ah,0
	push	di bx
	call	showframe
	pop	bx di
	mov	cl,12
	mov	ch,8
	call	multidump
	ret

	endp




Accesslighton	proc	near

	mov	di,74
	mov	bx,182
	mov	ds,tempgraphics
	mov	al,8
	mov	ah,0
	push	di bx
	call	showframe
	pop	bx di
	mov	cl,12
	mov	ch,8
	call	multidump
	ret

	endp




Accesslightoff	proc	near

	mov	di,74
	mov	bx,182
	mov	ds,tempgraphics
	mov	al,7
	mov	ah,0
	push	di bx
	call	showframe
	pop	bx di
	mov	cl,12
	mov	ch,8
	call	multidump
	ret

	endp





Locklighton	proc	near

	mov	di,56
	mov	bx,182
	mov	ds,tempgraphics
	mov	al,10
	mov	ah,0
	push	di bx
	call	showframe
	pop	bx di
	mov	cl,12
	mov	ch,8
	call	multidump
	ret

	endp




Locklightoff	proc	near

	mov	di,56
	mov	bx,182
	mov	ds,tempgraphics
	mov	al,9
	mov	ah,0
	push	di bx
	call	showframe
	pop	bx di
	mov	cl,12
	mov	ch,8
	call	multidump
	ret

	endp










Input	proc	near

	push	cs
	pop	es
	mov	di,offset cs:inputline
	mov	cx,64
	mov	al,0
	rep	stosb

	mov	curpos,0
	mov	al,">"
	mov	di,monadx
	mov	bx,monady
	mov	ds,tempcharset
	mov	ah,0
	call	printchar
	mov	di,monadx
	mov	bx,monady
	mov	cl,6
	mov	ch,8
	call	multidump
	add	monadx,6
	mov	ax,monadx
	mov	curslocx,ax
	mov	ax,monady
	mov	curslocy,ax

waitkey:	call	printcurs
	call	vsync
	call	delcurs
	call	readkey
	mov	al,currentkey
	cmp	al,0
	jz	waitkey
	cmp	al,13
	jz	endofinput
	cmp	al,8
	jnz	notdel
	cmp	curpos,0
	jz	waitkey
	call	delchar
	jmp	waitkey
notdel:	cmp	curpos,28
	jz	waitkey
	cmp	currentkey,32
	jnz	notleadingspace
	cmp	curpos,0
	jz	waitkey
notleadingspace:	call	makecaps
	push	cs
	pop	es
	mov	si,curpos
	add	si,si
	add	si,offset cs:inputline
	mov	[es:si],al

	cmp	al,"Z"+1
	jnc	waitkey

	push	ax es si
	mov	di,monadx
	mov	bx,monady
	mov	ds,mapstore
	mov	ax,curpos
	xchg	al,ah
	mov	si,ax
	mov	cl,8
	mov	ch,8
	call	multiget
	pop	si es ax

	push	es si
	mov	di,monadx
	mov	bx,monady
	mov	ds,tempcharset
	mov	ah,0
	call	printchar
	pop	si es
	;dec	cx
	mov	[es:si+1],cl
	mov	ch,0
	add	monadx,cx
	inc	curpos
	add	curslocx,cx
	jmp	waitkey

endofinput:	ret

	endp








Makecaps	proc	near

	cmp	al,"a"
	jc	notupperc
	sub	al,32
notupperc:	ret

	endp









Delchar 	proc	near

	dec	curpos
	mov	si,curpos
	add	si,si
	push	cs
	pop	es
	add	si,offset cs:inputline
	mov	byte ptr [es:si],0
	mov	al,[es:si+1]
	mov	ah,0
	sub	monadx,ax
	sub	curslocx,ax
	mov	di,monadx
	mov	bx,monady
	mov	ds,mapstore
	mov	ax,curpos
	xchg	al,ah
	mov	si,ax
	mov	cl,8
	mov	ch,8
	call	multiput
	mov	di,monadx
	mov	bx,monady
	mov	cl,al
	mov	ch,8
	call	multidump
	ret

	endp







Execcommand	proc	near

	push	cs
	pop	es
	mov	bx,offset cs:comlist
	push	cs
	pop	ds
	mov	si,offset cs:inputline
	mov	al,[si]
	cmp	al,0
	jnz	notblankinp
	call	scrollmonitor
	ret

notblankinp:	mov	cl,0
comloop:	push	bx si
comloop2:	mov	al,[si]
	add	si,2
	mov	ah,[es:bx]
	inc	bx
	cmp	ah,32
	jz	foundcom
	cmp	al,ah
	jz	comloop2
	pop	si bx
	add	bx,10
	inc	cl
	cmp	cl,6
	jnz	comloop
	call	neterror
	mov	al,0
	ret
foundcom:	pop	si bx
	cmp	cl,1
	jz	testcom
	cmp	cl,2
	jz	directory
	cmp	cl,3
	jz	accesscom
	cmp	cl,4
	jz	signoncom
	cmp	cl,5
	jz	keyscom
	jmp	quitcom

directory:	call	dircom
	mov	al,0
	ret

signoncom:	call	signon
	mov	al,0
	ret

accesscom:	call	read
	mov	al,0
	ret

keyscom:	call	showkeys
	mov	al,0
	ret

testcom:	mov	al,6
	call	monmessage
	mov	al,0
	ret

quitcom:	mov	al,1
	ret

comlist:	db	"EXIT      "
	db	"HELP      "
	db	"LIST      "
	db	"READ      "
	db	"LOGON     "
	db	"KEYS      "

keys:	db	1,0,"PUBLIC      PUBLIC     ",0
	db	0,0,"BLACKDRAGON RYAN       ",0
	db	0,0,"HENDRIX     LOUIS      ",0
	db	0,0,"SEPTIMUS    BECKETT    ",0
	db	255,255

operand1:	db      "             ",0
rootdir:	db	34,"ROOT        ",0
currentfile	db	34,"            ",0

	endp







Neterror	proc	near

	mov	al,5
	call	monmessage
	call	scrollmonitor
	ret

	endp









Dircom	proc	near

	mov	cx,30
	call	randomaccess
	call	parser
	cmp	byte ptr [es:di+1],0
	jz	dirroot
	call	dirfile
	ret

dirroot:	mov	logonum,0
	push	cs
	pop	ds
	mov	si,offset cs:rootdir
	inc	si
	push	cs
	pop	es
	mov	di,offset cs:currentfile
	inc	di
	mov	cx,12
	rep	movsb
	call	monitorlogo
	call	scrollmonitor
	
	mov	al,9
	call	monmessage

	mov	es,textfile1
	call	searchforfiles
	mov	es,textfile2
	call	searchforfiles
	mov	es,textfile3
	call	searchforfiles

	call	scrollmonitor
	ret

	endp






Searchforfiles	proc	near

	mov	bx,textstart
directloop1:	mov	al,[es:bx]
	inc	bx
	cmp	al,"*"
	jz	endofdir
	cmp	al,34
	jnz	directloop1
	call	monprint
	jmp	directloop1
endofdir:	ret

	endp












Signon	proc	near

	call	parser
	inc	di
	push	cs
	pop	ds
	mov	si,offset cs:keys
	mov	cx,4
signonloop:	push	cx si di
	add	si,14
	mov	cx,11
signonloop2:	lodsb
	cmp	al,32
	jz	foundsign
	call	makecaps
	mov	ah,[es:di]
	inc	di
	cmp	al,ah
	jnz	nomatch
	loop	signonloop2
nomatch:	pop	di si cx
	add	si,26
	loop	signonloop
	mov	al,13
	call	monmessage
	ret

foundsign:	pop	di si cx		;ds:si contains ad of key matched
	mov	bx,si
	push	ds
	pop	es		;now ds:si is in es:bx

	cmp	byte ptr [es:bx],0
	jz	notyetassigned

	mov	al,17
	call	monmessage
	ret

notyetassigned: push	es bx
	call	scrollmonitor
	mov	al,15
	call	monmessage
	mov	di,monadx
	mov	bx,monady
	push	di bx
	call	input
	pop	bx di
	mov	monadx,di
	mov	monady,bx
	pop	bx es
	push	es bx

	add	bx,2
	push	cs
	pop	ds
	mov	si,offset cs:inputline
checkpass:	lodsw
	mov	ah,[es:bx]
	inc	bx
	;cmp	al,0
	cmp	ah,32
	jz	passpassed
	cmp	al,ah
	jz	checkpass
passerror:	pop	bx es
	call	scrollmonitor
	mov	al,16
	call	monmessage
	ret

passpassed:	mov	al,14
	call	monmessage
	pop	bx es
	push	es bx
	add	bx,14
	call	monprint
	call	scrollmonitor
	pop	bx es
	mov	byte ptr [es:bx],1
	ret

	endp







Showkeys	proc	near

	mov	cx,10
	call	randomaccess
	call	scrollmonitor
	mov	al,18
	call	monmessage

	push	cs
	pop	es
	mov	bx,offset cs:keys
	mov	cx,4
keysloop:	push	cx bx
	cmp	byte ptr [es:bx],0
	jz	notheld
	add	bx,14
	call	monprint
notheld:	pop	bx cx
	add	bx,26
	loop	keysloop
	call	scrollmonitor
	ret

	endp











Read	proc	near

	mov	cx,40
	call	randomaccess
	call	parser
	cmp	byte ptr [es:di+1],0
	jnz	okcom
	call	neterror
	ret
okcom:	push	cs
	pop	es
	mov	di,offset cs:currentfile
	mov	ax,textfile1
	mov	monsource,ax
	mov	ds,ax
	mov	si,textstart
	call	searchforstring
	cmp	al,0
	jz	foundfile2
	mov	ax,textfile2
	mov	monsource,ax
	mov	ds,ax
	mov	si,textstart
	call	searchforstring
	cmp	al,0
	jz	foundfile2
	mov	ax,textfile3
	mov	monsource,ax
	mov	ds,ax
	mov	si,textstart
	call	searchforstring
	cmp	al,0
	jz	foundfile2
	mov	al,7
	call	monmessage
	ret
foundfile2:	call	getkeyandlogo
	cmp	al,0
	jz	keyok1
	ret
keyok1: 	push	cs
	pop	es
	mov	di,offset cs:operand1
	mov	ds,monsource
	call	searchforstring
	cmp	al,0
	jz	findtopictext
	mov	al,oldlogonum
	mov	logonum,al
	mov	al,11
	call	monmessage
	ret

findtopictext:	inc	bx
	push	es bx
	call	monitorlogo
	call	scrollmonitor
	pop	bx es
moretopic:	call	monprint
	mov	al,[es:bx]
	cmp	al,34
	jz	endoftopic
	cmp	al,"="
	jz	endoftopic
	cmp	al,"*"
	jz	endoftopic
	push	es bx
	call	processtrigger
	mov	cx,24
	call	randomaccess
	pop	bx es
	jmp	moretopic

endoftopic:	call	scrollmonitor
	ret

	endp









Dirfile	proc	near

	mov	al,34
	mov	[es:di],al
	push	es di		;save start point held in es:di
	mov	ds,textfile1
	mov	si,textstart
	call	searchforstring
	cmp	al,0
	jz	foundfile
	pop	di es
	push	es di
	mov	ds,textfile2
	mov	si,textstart
	call	searchforstring
	cmp	al,0
	jz	foundfile
	pop	di es
	push	es di
	mov	ds,textfile3
	mov	si,textstart
	call	searchforstring
	cmp	al,0
	jz	foundfile

	pop	di es
	mov	al,7
	call	monmessage
	ret

foundfile:	pop	ax ax		;discard old values of es:di
	call	getkeyandlogo
	cmp	al,0
	jz	keyok2
	ret

keyok2: 	push	es bx
	push	cs
	pop	ds
	mov	si,offset cs:operand1+1
	push	cs
	pop	es
	mov	di,offset cs:currentfile+1
	mov	cx,12
	rep	movsb
	call	monitorlogo
	call	scrollmonitor
	mov	al,10
	call	monmessage
	pop	bx es


directloop2:	mov	al,[es:bx]
	inc	bx
	cmp	al,34
	jz	endofdir2
	cmp	al,"*"
	jz	endofdir2
	cmp	al,"="
	jnz	directloop2
	call	monprint
	jmp	directloop2

endofdir2:	call	scrollmonitor
	ret

	endp






Getkeyandlogo	proc	near

	inc	bx
	mov	al,[es:bx]
	sub	al,48
	mov	newlogonum,al
	add	bx,2
	mov	al,[es:bx]
	sub	al,48
	mov	keynum,al
	inc	bx
	push	es bx
	mov	al,keynum
	mov	ah,0
	mov	cx,26
	mul	cx
	push	cs
	pop	es
	mov	bx,offset cs:keys
	add	bx,ax
	mov	al,[es:bx]
	cmp	al,1
	jz	keyok
	push	bx es
	mov	al,12
	call	monmessage
	pop	es bx
	add	bx,14
	call	monprint
	call	scrollmonitor
	pop	bx es
	mov	al,1
	ret

keyok:	pop	bx es
	mov	al,newlogonum
	mov	logonum,al
	mov	al,0
	ret

	endp









Searchforstring	proc	near	;finds string at es:di in
			;text at ds:si

	mov	dl,[es:di]	;needs to know first character
			;if it's an equals then we must
			;stop at the next quotation mark
	mov	cx,di	;need to remember starting point

restartlook:	mov	di,cx
	mov	bx,si

	mov	dh,0	;dh holds count of brackets
			;brackets are either " or =
keeplooking:	lodsb
	call	makecaps
	cmp	al,"*"
	jz	notfound
	cmp	dl,"="
	jnz	nofindingtopic	;are we looking for a topic?
	cmp	al,34	;if YES, check we haven't reached
	jz	notfound	;the end of this file.
nofindingtopic: mov	ah,[es:di]
	cmp	al,dl	;all searches bracketed by same thing
	jnz	notbracket
	inc	dh
	cmp	dh,2
	jz	complete
notbracket:	cmp	al,ah
	jnz	restartlook
	inc	di
	jmp	keeplooking
complete:	push	ds	;es:bx returns found string
	pop	es
	mov	al,0
	mov	bx,si
	ret
notfound:	mov	al,1
	ret

	endp






Parser	proc	near

	push	cs
	pop	es
	mov	di,offset cs:operand1
	mov	cx,13
	mov	al,0
	rep	stosb

	mov	di,offset cs:operand1
	mov	al,"="
	stosb

	push	cs
	pop	ds
	mov	si,offset cs:inputline

notspace1:	lodsw
	cmp	al,32
	jz	stillspace1
	cmp	al,0
	jnz	notspace1
	jmp	finishpars

stillspace1:	lodsw
	cmp	al,32
	jz	stillspace1

copyin1:	stosb
	lodsw
	cmp	al,0
	jz	finishpars
	cmp	al,32
	jnz	copyin1

finishpars:	mov	di,offset cs:operand1
	ret

	endp






Scrollmonitor	proc	near

	push	ax bx cx dx di si es ds

	call	printlogo
	mov	di,monadx
	mov	bx,monady
	call	printundermon
	mov	ax,monady
	call	worktoscreen
	mov	al,25
	call	playchannel1

	pop	ds es si di dx cx bx ax
	ret

	endp














Lockmon 	proc	near

	cmp	lasthardkey,57
	jnz	notlock
	call	locklighton
lockloop:	cmp	lasthardkey,57
	jz	lockloop
	call	locklightoff
notlock:	ret

	endp





Monitorlogo	proc	near

	mov	al,logonum
	cmp	al,oldlogonum
	jz	notnewlogo
	mov	oldlogonum,al
	;call	fadedownmon
	call	printlogo
	call	printundermon
	call	worktoscreen
	call	printlogo
	;call	fadeupmon
	call	printlogo
	mov	al,26
	call	playchannel1
	mov	cx,20
	call	randomaccess
	ret
notnewlogo:	call	printlogo
	ret

	endp








Printlogo	proc	near

	mov	di,56
	mov	bx,32
	mov	ds,tempgraphics
	mov	al,0
	mov	ah,0
	call	showframe
	call	showcurrentfile
	ret

	endp






Showcurrentfile proc	near

	mov	di,178 ;99
	mov	bx,37
	mov	si,offset cs:currentfile+1
curfileloop:	mov	al,[cs:si]
	cmp	al,0
	jz	finishfile
	inc	si
	push	si
	if	foreign
	call	modifychar
	endif
	mov	ds,tempcharset
	mov	ah,0
	call	printchar
	pop	si
	jmp	curfileloop
finishfile:	ret

	endp









Monmessage	proc	near

	mov	es,textfile1
	mov	bx,textstart
	mov	cl,al
	mov	ch,0
monmessageloop:	mov	al,[es:bx]
	inc	bx
	cmp	al,"+"
	jnz	monmessageloop
	loop	monmessageloop
	call	monprint
	ret

	endp






Processtrigger	proc	near

	cmp	lasttrigger,"1"
	jnz	notfirsttrigger
	mov	al,8
	call	setlocation
	mov	al,45
	call	triggermessage
	ret

notfirsttrigger:	cmp	lasttrigger,"2"
	jnz	notsecondtrigger
	mov	al,9
	call	setlocation
	mov	al,55
	call	triggermessage
	ret

notsecondtrigger:	cmp	lasttrigger,"3"
	jnz	notthirdtrigger
	mov	al,2
	call	setlocation
	mov	al,59
	call	triggermessage

notthirdtrigger:	ret

	endp




Triggermessage	proc	near

	push	ax
	mov	di,174
	mov	bx,153
	mov	cl,200
	mov	ch,63
	mov	ds,mapstore
	mov	si,0
	call	multiget
	pop	ax
	call	findpuztext
	mov	di,174
	mov	bx,156
	mov	dl,141
	mov	ah,16
	call	printdirect
	mov	cx,140
	call	hangon
	call	worktoscreen
	mov	cx,340
	call	hangon
	mov	di,174
	mov	bx,153
	mov	cl,200
	mov	ch,63
	mov	ds,mapstore
	mov	si,0
	call	multiput
	call	worktoscreen
	mov	lasttrigger,0
	ret

	endp






Printcurs	proc	near

	push	si di ds dx bx es
	mov	di,curslocx
	mov	bx,curslocy
	mov	cl,6
	mov	ch,8
	cmp	foreignrelease, 0
	jz $1
	sub	bx,3
	mov	ch,11
$1:
	mov	ds,buffers
	mov	si,textunder
	push	di bx
	call	multiget
	pop	bx di

	push	bx di
	inc	maintimer
	mov	ax,maintimer
	and	al,16
	jnz	flashcurs
	mov	al,"/"
	sub	al,32
	mov	ah,0
	mov	ds,tempcharset
	call	showframe

flashcurs:	pop	di bx
	sub	di,6
	mov	cl,12
	mov	ch,8
	cmp	foreignrelease, 0
	jz $2
	mov	ch,11
$2:
	call	multidump

	pop	es bx dx ds di si
	ret

	endp






Delcurs 	proc	near

	push	es bx di ds dx si
	mov	di,curslocx
	mov	bx,curslocy
	mov	cl,6
	mov	ch,8
	cmp	foreignrelease, 0
	jz $1
	sub	bx,3
	mov	ch,11
$1:
	push	di bx cx
	mov	ds,buffers
	mov	si,textunder
	call	multiput
	pop	cx bx di
	call	multidump
finishcurdel:
	pop	si dx ds di bx es
	ret

	endp
