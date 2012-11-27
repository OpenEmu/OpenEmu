;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text
Entercode	proc	near

	mov	keypadax,ax
	mov	keypadcx,cx
	call	getridofreels
	call	loadkeypad
	call	createpanel
	call	showicon
	call	showouterpad
	call	showkeypad
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
	mov	presspointer,0
	mov	getback,0

keypadloop:
	cmp quitrequested, 0
	jnz numberright

	call	delpointer
	call	readmouse
	call	showkeypad
	call	showpointer
	call	vsync
	cmp	presscount,0
	jz	nopresses
	dec	presscount
	jmp	afterpress
nopresses:	mov	pressed,255
	mov	graphicpress,255
	call	vsync

afterpress:	call	dumppointer
	call	dumpkeypad
	call	dumptextline
	mov	bx,offset cs:keypadlist
	call	checkcoords
	cmp	getback,1
	jz	numberright

	cmp	lightcount,1
	jnz	notendkey
	cmp	lockstatus,0
	jz	numberright
	jmp	keypadloop

notendkey:	cmp	presscount,40
	jnz	keypadloop
	call	addtopresslist
	cmp	pressed,11
	jnz	keypadloop
	mov	ax,keypadax
	mov	cx,keypadcx
	call	isitright
	jnz	incorrect
	mov	lockstatus,0
	mov	al,11
	call	playchannel1
	mov	lightcount,120
	mov	presspointer,0
	jmp	keypadloop
incorrect:	mov	al,11
	call	playchannel1
	mov	lightcount,120
	mov	presspointer,0
	jmp	keypadloop

numberright:	mov	manisoffscreen,0
	call	getridoftemp
	call	restorereels
	call	redrawmainscrn
	call	worktoscreenm
	ret

keypadlist:	dw	keypadx+9,keypadx+30,keypady+9,keypady+22,buttonone
	dw	keypadx+31,keypadx+52,keypady+9,keypady+22,buttontwo
	dw	keypadx+53,keypadx+74,keypady+9,keypady+22,buttonthree
	dw	keypadx+9,keypadx+30,keypady+23,keypady+40,buttonfour
	dw	keypadx+31,keypadx+52,keypady+23,keypady+40,buttonfive
	dw	keypadx+53,keypadx+74,keypady+23,keypady+40,buttonsix
	dw	keypadx+9,keypadx+30,keypady+41,keypady+58,buttonseven
	dw	keypadx+31,keypadx+52,keypady+41,keypady+58,buttoneight
	dw	keypadx+53,keypadx+74,keypady+41,keypady+58,buttonnine
	dw	keypadx+9,keypadx+30,keypady+59,keypady+73,buttonnought
	dw	keypadx+31,keypadx+74,keypady+59,keypady+73,buttonenter
	dw	keypadx+72,keypadx+86,keypady+80,keypady+94,quitkey
	dw	0,320,0,200,blank
	dw	0ffffh

	endp








Loadkeypad	proc	near
	
	mov	dx,offset cs:extragraphics1
	call	loadintotemp
	ret

	endp




Quitkey	proc	near

	cmp	commandtype,222
	jz	alreadyqk
	mov	commandtype,222
	mov	al,4
	call	commandonly
alreadyqk:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	notqk
	and	ax,1
	jnz	doqk
notqk:	ret

doqk:	 mov	getback,1
	ret

	endp


















Addtopresslist	proc	near

	cmp	presspointer,5
	jz	nomorekeys
	mov	al,pressed
	cmp	al,10
	jnz	not10
	mov	al,0
not10:	mov	bx,presspointer
	mov	dx,seg presslist
	mov	es,dx
	add	bx,offset es:presslist
	mov	[es:bx],al
	inc	presspointer
nomorekeys:	ret

	endp




Buttonone	proc	near

	mov	cl,1
	call	buttonpress
	ret

	endp


Buttontwo	proc	near

	mov	cl,2
	call	buttonpress
	ret

	endp



Buttonthree	proc	near

	mov	cl,3
	call	buttonpress
	ret

	endp



Buttonfour	proc	near

	mov	cl,4
	call	buttonpress
	ret

	endp


Buttonfive	proc	near

	mov	cl,5
	call	buttonpress
	ret

	endp



Buttonsix	proc	near

	mov	cl,6
	call	buttonpress
	ret

	endp


Buttonseven	proc	near

	mov	cl,7
	call	buttonpress
	ret

	endp


Buttoneight	proc	near

	mov	cl,8
	call	buttonpress
	ret

	endp

Buttonnine	proc	near

	mov	cl,9
	call	buttonpress
	ret

	endp



Buttonnought	proc	near

	mov	cl,10
	call	buttonpress
	ret

	endp






Buttonenter	proc	near

	mov	cl,11
	call	buttonpress
	ret

	endp



Buttonpress	proc	near

	mov	ch,cl
	add	ch,100
	cmp	commandtype,ch
	jz	alreadyb
	mov	commandtype,ch
	mov	al,cl
	add	al,4
	push	cx
	call	commandonly
	pop	cx
alreadyb:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	notb
	and	ax,1
	jnz	dob
notb:	ret

dob: 	mov	pressed,cl
	add	cl,21
	mov	graphicpress,cl
	mov	presscount,40
	cmp	cl,32
	jz	nonoise
	mov	al,10
	call	playchannel1
nonoise:	ret

	endp














Showouterpad	proc	near

	mov	di,keypadx-3
	mov	bx,keypady-4
	mov	ds,tempgraphics
	mov	al,1
	mov	ah,0
	call	showframe
	mov	di,keypadx+74
	mov	bx,keypady+76
	mov	ds,tempgraphics
	mov	al,37
	mov	ah,0
	call	showframe
	ret

	endp










Showkeypad	proc	near

	mov	al,22
	mov	di,keypadx+9
	mov	bx,keypady+5
	call	singlekey
	mov	al,23
	mov	di,keypadx+31
	mov	bx,keypady+5
	call	singlekey
	mov	al,24
	mov	di,keypadx+53
	mov	bx,keypady+5
	call	singlekey

	mov	al,25
	mov	di,keypadx+9
	mov	bx,keypady+23
	call	singlekey
	mov	al,26
	mov	di,keypadx+31
	mov	bx,keypady+23
	call	singlekey
	mov	al,27
	mov	di,keypadx+53
	mov	bx,keypady+23
	call	singlekey

	mov	al,28
	mov	di,keypadx+9
	mov	bx,keypady+41
	call	singlekey
	mov	al,29
	mov	di,keypadx+31
	mov	bx,keypady+41
	call	singlekey
	mov	al,30
	mov	di,keypadx+53
	mov	bx,keypady+41
	call	singlekey

	mov	al,31
	mov	di,keypadx+9
	mov	bx,keypady+59
	call	singlekey
	mov	al,32
	mov	di,keypadx+31
	mov	bx,keypady+59
	call	singlekey

	cmp	lightcount,0
	jz	notenter
	dec	lightcount
	mov	al,36
	mov	bx,keypady-1+63
	cmp	lockstatus,0
	jnz	changelight
	mov	al,41
	mov	bx,keypady+4+63
changelight:	cmp	lightcount,60
	jc	gotlight
	cmp	lightcount,100
	jnc	gotlight
	dec	al
gotlight:	mov	ds,tempgraphics
	mov	ah,0
	mov	di,keypadx+60
	call	showframe

notenter:	ret

	endp





Singlekey	proc	near

	cmp	graphicpress,al
	jnz	gotkey
	add	al,11
	cmp	presscount,8
	jnc	gotkey
	sub	al,11
;	cmp	presscount,10
;	jnc	gotkey
;	sub	al,11
gotkey:	mov	ds,tempgraphics
	sub	al,20
	mov	ah,0
	call	showframe
	ret

	endp









Dumpkeypad	proc	near

	mov	di,keypadx-3
	mov	bx,keypady-4
	mov	cl,120
	mov	ch,90
	call	multidump
	ret

	endp





;---------------------------------




Usemenu	proc	near

	call	getridofreels
	call	loadmenu
	call	createpanel
	call	showpanel
	call	showicon
	mov	newobs,0
	call	drawfloor
	call	printsprites

	mov	al,4
	mov	ah,0
	mov	di,menux-48
	mov	bx,menuy-4
	mov	ds,tempgraphics2
	call	showframe
	call	getundermenu
	mov	al,5
	mov	ah,0
	mov	di,menux+54
	mov	bx,menuy+72
	mov	ds,tempgraphics2
	call	showframe


	call	worktoscreenm
	mov	getback,0

menuloop:	call	delpointer
	call	putundermenu
	call	showmenu
	call	readmouse
	call	showpointer
	call	vsync
	call	dumppointer
	call	dumpmenu
	call	dumptextline
	mov	bx,offset cs:menulist
	call	checkcoords
	cmp	getback,1
	jnz	menuloop

	mov	manisoffscreen,0
	call	redrawmainscrn
	call	getridoftemp
	call	getridoftemp2
	call	restorereels
	call	worktoscreenm
	ret

menulist:	dw	menux+54,menux+68,menuy+72,menuy+88,quitkey
	dw	0,320,0,200,blank
	dw	0ffffh

	ret

	endp







Dumpmenu	proc	near

	mov	di,menux
	mov	bx,menuy
	mov	cl,48
	mov	ch,48
	call	multidump
	ret

	endp






Getundermenu	proc	near

	mov	di,menux
	mov	bx,menuy
	mov	cl,48
	mov	ch,48
	mov	ds,buffers
	mov	si,undertimedtext
	call	multiget
	ret

	endp








Putundermenu	proc	near

	mov	di,menux
	mov	bx,menuy
	mov	cl,48
	mov	ch,48
	mov	ds,buffers
	mov	si,undertimedtext
	call	multiput
	ret

	endp



Showoutermenu	proc	near

	mov	al,40
	mov	ah,0
	mov	di,menux-34
	mov	bx,menuy-40
	mov	ds,tempgraphics
	call	showframe
	mov	al,41
	mov	ah,0
	mov	di,menux+64-34
	mov	bx,menuy-40
	mov	ds,tempgraphics
	call	showframe
	mov	al,42
	mov	ah,0
	mov	di,menux-26
	mov	bx,menuy+57-40
	mov	ds,tempgraphics
	call	showframe
	mov	al,43
	mov	ah,0
	mov	di,menux+64-26
	mov	bx,menuy+57-40
	mov	ds,tempgraphics
	call	showframe
	ret

	endp








Showmenu	proc	near

	inc	menucount
	cmp	menucount,37*2
	jnz	menuframeok
	mov	menucount,0
menuframeok:	mov	al,menucount
	shr	al,1
	mov	ah,0
	mov	di,menux
	mov	bx,menuy
	mov	ds,tempgraphics
	call	showframe
	ret

	endp



Loadmenu	proc	near

	mov	dx,offset cs:spritename3
	call	loadintotemp
	mov	dx,offset cs:mongraphics2
	call	loadintotemp2
	ret

	endp
















Viewfolder	proc	near

	mov	manisoffscreen,1
	call	getridofall
	call	loadfolder
	mov	folderpage,0
	call	showfolder
	call	worktoscreenm
	mov	getback,0

folderloop:	call	delpointer
	call	readmouse
	call	showpointer
	call	vsync
	call	dumppointer
	call	dumptextline
	mov	bx,offset cs:folderlist
	call	checkcoords
	cmp	getback,0
	jz	folderloop

	mov	manisoffscreen,0
	call	getridoftemp
	call	getridoftemp2
	call	getridoftemp3
	call	getridoftempcharset
	call	restoreall
	call	redrawmainscrn
	call	worktoscreenm
	ret

folderlist:	dw	280,320,160,200,quitkey
	dw	143,300,6,194,nextfolder
	dw	0,143,6,194,lastfolder
	dw	0,320,0,200,blank
	dw	0ffffh

	endp




Nextfolder	proc	near

	cmp	folderpage,12
	jnz	cannextf
	call	blank
	ret
cannextf:	cmp	commandtype,201
	jz	alreadynextf
	mov	commandtype,201
	mov	al,16
	call	commandonly
alreadynextf:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	notnextf
	cmp	ax,1
	jz	donextf
notnextf:	ret
donextf:	inc	folderpage
	call	folderhints
	call	delpointer
	call	showfolder
	mov	mousebutton,0
	mov	bx,offset cs:folderlist
	call	checkcoords
	call	worktoscreenm
	ret

	endp




Folderhints	proc	near

	cmp	folderpage,5
	jnz	notaideadd
	cmp	aidedead,1
	jz	notaideadd
	mov	al,13
	call	getlocation
	cmp	al,1
	jz	notaideadd
	mov	al,13
	call	setlocation
	call	showfolder
	mov	al,30
	call	findtext1
	mov	di,0
	mov	bx,86
	mov	dl,141
	mov	ah,16
	call	printdirect
	call	worktoscreenm
	mov	cx,200
	call	hangonp
	ret

notaideadd:	cmp	folderpage,9
	jnz	notaristoadd
	mov	al,7
	call	getlocation
	cmp	al,1
	jz	notaristoadd
	mov	al,7
	call	setlocation
	call	showfolder
	mov	al,31
	call	findtext1
	mov	di,0
	mov	bx,86
	mov	dl,141
	mov	ah,16
	call	printdirect
	call	worktoscreenm
	mov	cx,200
	call	hangonp
notaristoadd:	ret

	endp



Lastfolder	proc	near
	
	cmp	folderpage,0
	jnz	canlastf
	call	blank
	ret
canlastf:	cmp	commandtype,202
	jz	alreadylastf
	mov	commandtype,202
	mov	al,17
	call	commandonly
alreadylastf:	cmp	folderpage,0
	jz	notlastf
	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	notlastf
	cmp	ax,1
	jz	dolastf
notlastf:	ret
dolastf:	dec	folderpage
	call	delpointer
	call	showfolder
	mov	mousebutton,0
	mov	bx,offset cs:folderlist
	call	checkcoords
	call	worktoscreenm
	ret

	endp



Loadfolder	proc	near

	mov	dx,offset cs:foldergraphic1
	call	loadintotemp
	mov	dx,offset cs:foldergraphic2
	call	loadintotemp2
	mov	dx,offset cs:foldergraphic3
	call	loadintotemp3
	mov	dx,offset cs:characterset3
	call	loadtempcharset
	mov	dx,offset cs:foldertext
	call	loadtemptext
	ret

	endp




Showfolder	proc	near

	mov	commandtype,255
	cmp	folderpage,0
	jz	closedfolder
	call	usetempcharset
	call	createpanel2
	mov	ds,tempgraphics
	mov	di,0
	mov	bx,0
	mov	al,0
	mov	ah,0
	call	showframe
	mov	ds,tempgraphics
	mov	di,143
	mov	bx,0
	mov	al,1
	mov	ah,0
	call	showframe
	mov	ds,tempgraphics
	mov	di,0
	mov	bx,92
	mov	al,2
	mov	ah,0
	call	showframe
	mov	ds,tempgraphics
	mov	di,143
	mov	bx,92
	mov	al,3
	mov	ah,0
	call	showframe
	call	folderexit
	
	cmp	folderpage,1
	jz	noleftpage
	call	showleftpage
noleftpage:	cmp	folderpage,12
	jz	norightpage
	call	showrightpage
norightpage:	call	usecharset1
	call	undertextline
	ret

closedfolder:	call	createpanel2
	mov	ds,tempgraphics3
	mov	di,143-28
	mov	bx,0
	mov	al,0
	mov	ah,0
	call	showframe
	mov	ds,tempgraphics3
	mov	di,143-28
	mov	bx,92
	mov	al,1
	mov	ah,0
	call	showframe
	call	folderexit
	call	undertextline
	ret

	endp








Folderexit	proc	near

	mov	ds,tempgraphics2
	mov	di,296
	mov	bx,178
	mov	al,6
	mov	ah,0
	call	showframe
	ret

	endp





Showleftpage	proc	near

	mov	ds,tempgraphics2
	mov	di,0
	mov	bx,12
	mov	al,3
	mov	ah,0
	call	showframe

	mov	bx,12+5
	mov	cx,9
leftpageloop:	push	cx bx	
	mov	ds,tempgraphics2
	mov	di,0
	mov	al,4
	mov	ah,0
	call	showframe
	pop	bx cx
	add	bx,16
	loop	leftpageloop

	mov	ds,tempgraphics2
	mov	di,0
	mov	al,5
	mov	ah,0
	call	showframe

	mov	linespacing,8
	mov	charshift,91
	mov	kerning,1
	mov	bl,folderpage
	dec	bl
	dec	bl
	add	bl,bl
	mov	bh,0
	add	bx,bx
	mov	es,textfile1
	mov	si,[es:bx]
	add	si,66*2
	mov	di,2
	mov	bx,48
	mov	dl,140
	mov	cx,2
twolotsleft:	push	cx
contleftpage:	call	printdirect
	add	bx,linespacing
	cmp	al,0
	jnz	contleftpage
	pop	cx
	loop	twolotsleft

	mov	kerning,0
	mov	charshift,0
	mov	linespacing,10

	mov	es,workspace
	mov	ds,workspace
	mov	di,(48*320)+2
	mov	si,(48*320)+2+130
	mov	cx,120
flipfolder:	push	cx di si
	mov	cx,65
flipfolderline:	mov	al,[es:di]
	mov	ah,[es:si]
	mov	[es:di],ah
	mov	[es:si],al
	dec	si
	inc	di
	loop	flipfolderline
	pop	si di cx
	add	si,320
	add	di,320
	loop	flipfolder
	ret
	
	endp



Showrightpage	proc	near

	mov	ds,tempgraphics2
	mov	di,143
	mov	bx,12
	mov	al,0
	mov	ah,0
	call	showframe

	mov	bx,12+37
	mov	cx,7
rightpageloop:	push	cx bx	
	mov	ds,tempgraphics2
	mov	di,143
	mov	al,1
	mov	ah,0
	call	showframe
	pop	bx cx
	add	bx,16
	loop	rightpageloop

	mov	ds,tempgraphics2
	mov	di,143
	mov	al,2
	mov	ah,0
	call	showframe

	mov	linespacing,8
	mov	kerning,1
	mov	bl,folderpage
	dec	bl
	add	bl,bl
	mov	bh,0
	add	bx,bx
	mov	es,textfile1
	mov	si,[es:bx]
	add	si,66*2
	mov	di,152
	mov	bx,48
	mov	dl,140
	mov	cx,2
twolotsright:	push	cx
contrightpage:	call	printdirect
	add	bx,linespacing
	cmp	al,0
	jnz	contrightpage
	pop	cx
	loop	twolotsright

	mov	kerning,0
	mov	linespacing,10
	ret

	endp









Entersymbol	proc	near

	mov	manisoffscreen,1
	call	getridofreels
	mov	dx,offset cs:symbolgraphic
	call	loadintotemp
	mov	symboltopx,24
	mov	symboltopdir,0
	mov	symbolbotx,24
	mov	symbolbotdir,0
	call	redrawmainscrn
	call	showsymbol
	call	undertextline
	call	worktoscreenm
	mov	getback,0

symbolloop:	call	delpointer
	call	updatesymboltop
	call	updatesymbolbot
	call	showsymbol
	call	readmouse
	call	showpointer
	call	vsync
	call	dumppointer
	call	dumptextline
	call	dumpsymbol
	mov	bx,offset cs:symbollist
	call	checkcoords
	cmp	getback,0
	jz	symbolloop

	cmp	symbolbotnum,3
	jnz	symbolwrong
	cmp	symboltopnum,5
	jnz	symbolwrong
	mov	al,43
	call	removesetobject
	mov	al,46
	call	placesetobject
	mov	ah,roomnum
	add	ah,12
	mov	al,0
	call	turnanypathon
	mov	manisoffscreen,0
	call	redrawmainscrn
	call	getridoftemp
	call	restorereels
	call	worktoscreenm
	mov	al,13
	call	playchannel1
	ret

symbolwrong:	mov	al,46
	call	removesetobject
	mov	al,43
	call	placesetobject
	mov	ah,roomnum
	add	ah,12
	mov	al,0
	call	turnanypathoff
	mov	manisoffscreen,0
	call	redrawmainscrn
	call	getridoftemp
	call	restorereels
	call	worktoscreenm
	ret

symbollist:	dw	symbolx+40,symbolx+64,symboly+2,symboly+16,quitsymbol
	dw	symbolx,symbolx+52,symboly+20,symboly+50,settopleft
	dw	symbolx+52,symbolx+104,symboly+20,symboly+50,settopright
	dw	symbolx,symbolx+52,symboly+50,symboly+80,setbotleft
	dw	symbolx+52,symbolx+104,symboly+50,symboly+80,setbotright
	dw	0,320,0,200,blank
	dw	0ffffh

	endp
















Quitsymbol	proc	near

	cmp	symboltopx,24
	jnz	blank
	cmp	symbolbotx,24
	jnz	blank
	cmp	commandtype,222
	jz	alreadyqs
	mov	commandtype,222
	mov	al,18
	call	commandonly
alreadyqs:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	notqs
	and	ax,1
	jnz	doqs
notqs:	ret

doqs:	 mov	getback,1
	ret

	endp




Settopleft	proc	near

	cmp	symboltopdir,0
	jnz	blank
	cmp	commandtype,210
	jz	alreadytopl
	mov	commandtype,210
	mov	al,19
	call	commandonly
alreadytopl:	cmp	mousebutton,0
	jz	notopleft
	mov	symboltopdir,-1
notopleft:	ret

	endp



Settopright	proc	near

	cmp	symboltopdir,0
	jnz	blank
	cmp	commandtype,211
	jz	alreadytopr
	mov	commandtype,211
	mov	al,20
	call	commandonly
alreadytopr:	cmp	mousebutton,0
	jz	notopright
	mov	symboltopdir,1
notopright:	ret

	endp




Setbotleft	proc	near

	cmp	symbolbotdir,0
	jnz	blank
	cmp	commandtype,212
	jz	alreadybotl
	mov	commandtype,212
	mov	al,21
	call	commandonly
alreadybotl:	cmp	mousebutton,0
	jz	nobotleft
	mov	symbolbotdir,-1
nobotleft:	ret

	endp



Setbotright	proc	near

	cmp	symbolbotdir,0
	jnz	blank
	cmp	commandtype,213
	jz	alreadybotr
	mov	commandtype,213
	mov	al,22
	call	commandonly
alreadybotr:	cmp	mousebutton,0
	jz	nobotright
	mov	symbolbotdir,1
nobotright:	ret

	endp







Dumpsymbol	proc	near

	mov	newtextline,0
	mov	di,symbolx
	mov	bx,symboly+20
	mov	cl,104
	mov	ch,60
	call	multidump
	ret

	endp




Showsymbol	proc	near

	mov	al,12
	mov	ah,0
	mov	di,symbolx
	mov	bx,symboly
	mov	ds,tempgraphics
	call	showframe

	mov	al,symboltopx
	mov	ah,0
	mov	di,ax
	add	di,symbolx-44
	mov	al,symboltopnum
	mov	bx,symboly+20
	mov	ds,tempgraphics
	mov	ah,32
	push	ax di bx ds
	call	showframe
	pop	ds bx di ax
	call	nextsymbol
	add	di,49
	push	ax di bx ds
	call	showframe
	pop	ds bx di ax
	call	nextsymbol
	add	di,49
	call	showframe

	mov	al,symbolbotx
	mov	ah,0
	mov	di,ax
	add	di,symbolx-44
	mov	al,symbolbotnum
	add	al,6
	mov	bx,symboly+49
	mov	ds,tempgraphics
	mov	ah,32
	push	ax di bx ds
	call	showframe
	pop	ds bx di ax
	call	nextsymbol
	add	di,49
	push	ax di bx ds
	call	showframe
	pop	ds bx di ax
	call	nextsymbol
	add	di,49
	call	showframe
	ret

	endp






Nextsymbol	proc	near

	inc	al
	cmp	al,6
	jz	topwrap
	cmp	al,12
	jz	botwrap
	ret
topwrap:	mov	al,0
	ret
botwrap:	mov	al,6
	ret
	
	endp



Updatesymboltop	proc	near

	cmp	symboltopdir,0
	jz	topfinished
	cmp	symboltopdir,-1
	jz	backwards

	inc	symboltopx
	cmp	symboltopx,49
	jnz	notwrapfor
	mov	symboltopx,0
	dec	symboltopnum
	cmp	symboltopnum,-1
	jnz	topfinished
	mov	symboltopnum,5
	ret
notwrapfor:	cmp	symboltopx,24
	jnz	topfinished
	mov	symboltopdir,0
	ret

backwards:	dec	symboltopx
	cmp	symboltopx,-1
	jnz	notwrapback
	mov	symboltopx,48
	inc	symboltopnum
	cmp	symboltopnum,6
	jnz	topfinished
	mov	symboltopnum,0
	ret
notwrapback:	cmp	symboltopx,24
	jnz	topfinished
	mov	symboltopdir,0
topfinished:	ret

	endp



Updatesymbolbot	proc	near

	cmp	symbolbotdir,0
	jz	botfinished
	cmp	symbolbotdir,-1
	jz	backwardsbot

	inc	symbolbotx
	cmp	symbolbotx,49
	jnz	notwrapforb
	mov	symbolbotx,0
	dec	symbolbotnum
	cmp	symbolbotnum,-1
	jnz	botfinished
	mov	symbolbotnum,5
	ret
notwrapforb:	cmp	symbolbotx,24
	jnz	botfinished
	mov	symbolbotdir,0
	ret

backwardsbot:	dec	symbolbotx
	cmp	symbolbotx,-1
	jnz	notwrapbackb
	mov	symbolbotx,48
	inc	symbolbotnum
	cmp	symbolbotnum,6
	jnz	botfinished
	mov	symbolbotnum,0
	ret
notwrapbackb:	cmp	symbolbotx,24
	jnz	botfinished
	mov	symbolbotdir,0
botfinished:	ret

	endp










Dumpsymbox	proc	near

	cmp	dumpx,-1
	jz	nodumpsym
	mov	di,dumpx
	mov	bx,dumpy
	mov	cl,30
	mov	ch,77;30
	call	multidump
	mov	dumpx,-1
nodumpsym:	ret

	endp







Usediary	proc	near

	call	getridofreels
	mov	dx,offset cs:diarygraphic
	call	loadintotemp
	mov	dx,offset cs:diarytext
	call	loadtemptext

	mov	dx,offset cs:characterset3
	call	loadtempcharset
	call	createpanel
	call	showicon
	call	showdiary
	call	undertextline
	call	showdiarypage
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
	mov	getback,0

diaryloop:	call	delpointer
	call	readmouse
	call	showdiarykeys
	call	showpointer
	call	vsync
	call	dumppointer
	call	dumpdiarykeys
	call	dumptextline
	mov	bx,offset cs:diarylist
	call	checkcoords
	cmp	getback,0
	jz	diaryloop

	call	getridoftemp
	call	getridoftemptext
	call	getridoftempcharset
	call	restorereels
	mov	manisoffscreen,0
	call	redrawmainscrn
	call	worktoscreenm
	ret

diarylist:	dw	diaryx+94,diaryx+110,diaryy+97,diaryy+113,diarykeyn
	dw	diaryx+151,diaryx+167,diaryy+71,diaryy+87,diarykeyp
	dw	diaryx+176,diaryx+192,diaryy+108,diaryy+124,quitkey
	dw	0,320,0,200,blank
	dw	0ffffh

	ret

	endp







Showdiary	proc	near

	mov	al,1
	mov	ah,0
	mov	di,diaryx
	mov	bx,diaryy+37
	mov	ds,tempgraphics
	call	showframe
	mov	al,2
	mov	ah,0
	mov	di,diaryx+176
	mov	bx,diaryy+108
	mov	ds,tempgraphics
	call	showframe
	ret

	endp




Showdiarykeys	proc	near

	cmp	presscount,0
	jz	nokeyatall
	dec	presscount
	cmp	presscount,0
	jz	nokeyatall
	cmp	pressed,"N"
	jnz	nokeyn
	mov	al,3
	cmp	presscount,1
	jz	gotkeyn
	mov	al,4
gotkeyn:	mov	ah,0
	mov	di,diaryx+94
	mov	bx,diaryy+97
	mov	ds,tempgraphics
	call	showframe
	cmp	presscount,1
	jnz	notshown
	call	showdiarypage	
notshown:	ret

nokeyn:	mov	al,5
	cmp	presscount,1
	jz	gotkeyp
	mov	al,6
gotkeyp:	mov	ah,0
	mov	di,diaryx+151
	mov	bx,diaryy+71
	mov	ds,tempgraphics
	call	showframe
	cmp	presscount,1
	jnz	notshowp
	call	showdiarypage
notshowp:	ret

nokeyatall:	ret

	endp






Dumpdiarykeys	proc	near

	cmp	presscount,1
	jnz	notdumpdiary
	cmp	sartaindead,1
	jz	notsartadd
	cmp	diarypage,5
	jnz	notsartadd
	cmp	diarypage,5
	jnz	notsartadd
	mov	al,6
	call	getlocation
	cmp	al,1
	jz	notsartadd
	mov	al,6
	call	setlocation
	
	call	delpointer
	mov	al,12
	call	findtext1
	mov	di,70  ;diaryx+48
	mov	bx,106 ;diaryy+16
	mov	dl,241
	mov	ah,16
	call	printdirect

	call	worktoscreenm
	mov	cx,200
	call	hangonp
	call	createpanel
	call	showicon
	call	showdiary
	call	showdiarypage
	call	worktoscreenm
	call	showpointer
	ret
		             
notsartadd:	mov	di,diaryx+48
	mov	bx,diaryy+15
	mov	cl,200
	mov	ch,16
	call	multidump
notdumpdiary:	mov	di,diaryx+94
	mov	bx,diaryy+97
	mov	cl,16
	mov	ch,16
	call	multidump
	mov	di,diaryx+151
	mov	bx,diaryy+71
	mov	cl,16
	mov	ch,16
	call	multidump
	ret

	endp



Diarykeyp	proc	near

	cmp	commandtype,214
	jz	alreadykeyp
	mov	commandtype,214
	mov	al,23
	call	commandonly
alreadykeyp:	cmp	mousebutton,0
	jz	notkeyp
	mov	ax,oldbutton
	cmp	ax,mousebutton
	jz	notkeyp
	cmp	presscount,0
	jnz	notkeyp
	mov	al,16
	call	playchannel1
	mov	presscount,12
	mov	pressed,"P"
	dec	diarypage
	cmp	diarypage,-1
	jnz	notkeyp
	mov	diarypage,11
notkeyp:	ret

	endp



Diarykeyn	proc	near

	cmp	commandtype,213
	jz	alreadykeyn
	mov	commandtype,213
	mov	al,23
	call	commandonly
alreadykeyn:	cmp	mousebutton,0
	jz	notkeyn
	mov	ax,oldbutton
	cmp	ax,mousebutton
	jz	notkeyn
	cmp	presscount,0
	jnz	notkeyn
	mov	al,16
	call	playchannel1
	mov	presscount,12
	mov	pressed,"N"
	inc	diarypage
	cmp	diarypage,12
	jnz	notkeyn
	mov	diarypage,0
notkeyn:	ret

	endp






Showdiarypage	proc	near

	mov	al,0
	mov	ah,0
	mov	di,diaryx
	mov	bx,diaryy
	mov	ds,tempgraphics
	call	showframe

	mov	al,diarypage
	call	findtext1

	mov	kerning,1
	call	usetempcharset
	mov	di,diaryx+48
	mov	bx,diaryy+16
	mov	dl,240
	mov	ah,16
	mov	charshift,91+91
	call	printdirect

	mov	di,diaryx+129
	mov	bx,diaryy+16
	mov	dl,240
	mov	ah,16
	call	printdirect

	mov	di,diaryx+48
	mov	bx,diaryy+23
	mov	dl,240
	mov	ah,16
	call	printdirect

	mov	kerning,0
	mov	charshift,0
	call	usecharset1
	ret

	endp





Findtext1	proc	near

	mov	ah,0
	mov	si,ax
	add	si,si
	mov	es,textfile1
	mov	ax,[es:si]
	add	ax,textstart
	mov	si,ax
	ret

	endp
