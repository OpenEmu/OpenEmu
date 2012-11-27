;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text
;---------------------------------------------------------Inventory printer----

Fillryan	proc	near	
			; cx=what to search for
	mov	es,buffers
	mov	di,ryaninvlist
	call	findallryan
	mov	si,ryaninvlist
	mov	al,ryanpage
	mov	ah,0
	mov	cx,20
	mul	cx
	add	si,ax
	
	mov	di,inventx
	mov	bx,inventy

	mov	cx,2
ryanloop2:	push	cx di bx
	mov	cx,5
ryanloop1:	push	cx di bx
	mov	ax,[es:si]
	add	si,2
	push	si es
	call	obtoinv
	pop	es si
	pop	bx di cx
	add	di,itempicsize
	loop	ryanloop1
	pop	bx di cx
	add	bx,itempicsize
	loop	ryanloop2

	call	showryanpage
	ret

	endp











Fillopen	proc	near	

	call	deltextline

	call	getopenedsize		;find out how many slots
	cmp	ah,4		;ah=slots, al=size holdable
	jc	lessthanapage
	mov	ah,4
lessthanapage:	mov	al,1

	push	ax
	mov	es,buffers
	mov	di,openinvlist
	call	findallopen
	mov	si,openinvlist
	mov	di,inventx
	mov	bx,inventy+96
	pop	cx

openloop1:	push	cx di bx
	mov	ax,[es:si]
	add	si,2
	push	si es
	cmp	ch,cl
	jc	nextopenslot
	call	obtoinv
nextopenslot:	pop	es si
	pop	bx di cx
	add	di,itempicsize
	inc	cl
	cmp	cl,5
	jnz	openloop1

	call	undertextline
	ret

	endp
















Findallryan	proc	near
	
	push	di
	mov	cx,30
	mov	ax,0ffffh
	rep	stosw
	pop	di
	
	mov	cl,4
	mov	ds,extras
	mov	bx,exdata
	mov	ch,0
findryanloop:	cmp	[bx+2],cl
	jnz	notinryaninv
	cmp	byte ptr [bx+3],255
	jnz	notinryaninv
	mov	al,[bx+4]
	mov	ah,0
	push	di
	add	di,ax
	add	di,ax
	mov	al,ch
	mov	ah,4		;means it is an exchanged object ie:not free or set
	stosw
	pop	di
notinryaninv:	add	bx,16
	inc	ch
	cmp	ch,numexobjects
	jnz	findryanloop
	ret

	endp







Findallopen	proc	near
	
	push	di
	mov	cx,16
	mov	ax,0ffffh
	rep	stosw
	pop	di

	mov	cl,openedob
	mov	dl,openedtype
	mov	ds,extras
	mov	bx,exdata
	mov	ch,0
findopen1:	cmp	[bx+3],cl
	jnz	findopen2
	cmp	[bx+2],dl
	jnz	findopen2
	cmp	openedtype,4
	jz	noloccheck
           	mov	al,[bx+5]
	cmp	al,reallocation
	jnz	findopen2
noloccheck:	mov	al,[bx+4]
	mov	ah,0
	push	di
	add	di,ax
	add	di,ax
	mov	al,ch
	mov	ah,4
	stosw
	pop	di
findopen2:	add	bx,16
	inc	ch
	cmp	ch,numexobjects
	jnz	findopen1

	mov	cl,openedob
	mov	dl,openedtype
	push	dx
	mov	ds,freedat
	pop	dx
	mov	bx,0
	mov	ch,0
findopen1a:	cmp	[bx+3],cl
	jnz	findopen2a
	cmp	[bx+2],dl
	jnz	findopen2a
	mov	al,[bx+4]
	mov	ah,0
	push	di
	add	di,ax
	add	di,ax
	mov	al,ch
	mov	ah,2 ; means it's in a free ob
	stosw
	pop	di
findopen2a:	add	bx,16
	inc	ch
	cmp	ch,80
	jnz	findopen1a
	ret

	endp






Obtoinv 	proc	near

	push	bx es si ax

	push	ax di bx
	mov	ds,icons1
	sub	di,2
	sub	bx,1
	mov	al,10
	mov	ah,0
	call	showframe
	pop	bx di ax
	cmp	al,255
	jz	finishfill

	push	bx di ax
	mov	ds,extras
	cmp	ah,4
	jz	isanextra
	mov	ds,freeframes
isanextra:	mov	cl,al
	add	al,al
	add	al,cl
	inc	al
	mov	ah,128
	add	bx,19
	add	di,18
	call	showframe
	pop	ax di bx

	push	bx
	call	getanyaddir
	call	isitworn
	pop	bx
	jnz	finishfill
	mov	ds,icons1	;Print wearing logo
	sub	di,3
	sub	bx,2
	mov	al,7
	mov	ah,0
	call	showframe

finishfill:	pop	ax si es bx
	ret

	endp





Isitworn	proc	near		;zero if yes

	mov	al,[es:bx+12]
	cmp	al,"W"-"A"
	jnz	notworn
	mov	al,[es:bx+13]
	cmp	al,"E"-"A"
notworn:	ret

	endp



Makeworn	proc	near

	mov	byte ptr [es:bx+12],"W"-"A"
	mov	byte ptr [es:bx+13],"E"-"A"
	ret

	endp






;-------------------------------------------------------Examining an object----

Examineob	proc	near

	mov	pointermode,0
	mov	timecount,0

examineagain:	mov	inmaparea,0
	mov	examagain,0
	mov	openedob,255
	mov	openedtype,255
	mov	invopen,0
	mov	al,commandtype
	mov	objecttype,al
	mov	itemframe,0
	mov	pointerframe,0
	
	call	createpanel
	call	showpanel
	call	showman
	call	showexit
	call	obicons
	call	obpicture
	call	describeob
	call	undertextline

	mov	commandtype,255
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer

waitexam:	;call	delpointer
	call	readmouse
	call	showpointer
	call	vsync
	call	dumppointer
	call	dumptextline
	call	delpointer

	mov	getback,0
	mov	bx,offset cs:examlist
	cmp	invopen,0
	jz	notuseinv
	mov	bx,offset cs:invlist1
	cmp	invopen,1
	jz	notuseinv
	mov	bx,offset cs:withlist1
notuseinv:	call	checkcoords

	cmp     quitrequested, 0
	jnz	stopwaiting

	cmp	examagain,0
	jz	norex
	jmp	examineagain
norex:	cmp	getback,0
	jz	waitexam
stopwaiting:

	mov	pickup,0
	cmp	watchingtime,0
	jnz	iswatching
	cmp	newlocation,255
	jnz	justgetback

iswatching:	call	makemainscreen
	mov	invopen,0
	mov	openedob,255
	ret

justgetback:	mov	invopen,0
	mov	openedob,255
	ret

examlist:	dw	273,320,157,198,getbackfromob
	dw	260,300,0,44,useobject
	dw	210,254,0,44,selectopenob
	dw	144,176,64,96,setpickup
	dw	0,50,50,200,examinventory
	dw	0,320,0,200,blank
	dw	0ffffh

invlist1:	dw	273,320,157,198,getbackfromob
	dw	255,294,0,24,dropobject
	dw	inventx+167,inventx+167+(18*3),inventy-18,inventy-2,incryanpage
	dw	inventx
openchangesize: dw	inventx+(4*itempicsize)
invlist1continued:	dw	inventy+100,inventy+100+itempicsize,useopened
	dw	inventx,inventx+(5*itempicsize)
	dw	inventy,inventy+(2*itempicsize),intoinv
	dw	0,320,0,200,blank
	dw	0ffffh

withlist1:	dw	273,320,157,198,getbackfromob
	dw	inventx+167,inventx+167+(18*3),inventy-18,inventy-2,incryanpage
	dw	inventx,inventx+(5*itempicsize)
	dw	inventy,inventy+(2*itempicsize),selectob
	dw	0,320,0,200,blank
	dw	0ffffh

	endp






Makemainscreen	proc	near

	call	createpanel
	mov	newobs,1
	call	drawfloor
	call	spriteupdate
	call	printsprites
	call	reelsonscreen
	call	showicon
	call	getunderzoom
	call	undertextline
	mov	commandtype,255
	call	animpointer
	call	worktoscreenm
	mov	commandtype,200
	mov	manisoffscreen,0
	ret

	endp




Getbackfromob	proc	near

	cmp	pickup,1
	jnz	notheldob
	call	blank
	ret
notheldob:	call	getback1
	ret

	endp
















Incryanpage	proc	near

	cmp	commandtype,222
	jz	alreadyincryan
	mov	commandtype,222
	mov	al,31
	call	commandonly
alreadyincryan:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	noincryan
	and	ax,1
	jnz	doincryan
noincryan:	ret

doincryan:	mov	ax,mousex
	sub	ax,inventx+167
	mov	ryanpage,-1
findnewpage:	inc	ryanpage
	sub	ax,18
	jnc	findnewpage
	call	delpointer
	call	fillryan
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
	ret

	endp







Openinv 	proc	near

	mov	invopen,1
	mov	al,61
	mov	di,inventx
	mov	bx,inventy-10
	mov	dl,240
	call	printmessage
	call	fillryan
	mov	commandtype,255
	ret

	endp







Showryanpage	proc	near

	mov	ds,icons1
	mov	di,inventx+167
	mov	bx,inventy-12
	mov	al,12
	mov	ah,0
	call	showframe

	mov	al,13
	add	al,ryanpage
	push	ax
	mov	al,ryanpage
	mov	ah,0
	mov	cx,18
	mul	cx
	mov	ds,icons1
	mov	di,inventx+167
	add	di,ax
	mov	bx,inventy-12
	pop	ax
	mov	ah,0
	call	showframe
	ret

	endp









Openob	proc	near

	mov	al,openedob
	mov	ah,openedtype
	mov	di,offset cs:commandline
	call	copyname
	
	mov	di,inventx
	mov	bx,inventy+86
	mov	al,62
	mov	dl,240
	call	printmessage
	
	mov	di,lastxpos
	add	di,5
	mov	bx,inventy+86
	push	cs
	pop	es
	mov	si,offset cs:commandline
	mov	dl,220
	mov	al,0
	mov	ah,0
	call	printdirect
	
	call	fillopen
	call	getopenedsize
	mov	al,ah
	mov	ah,0
	mov	cx,itempicsize
	mul	cx
	add	ax,inventx
	mov	bx,offset cs:openchangesize
	mov	[cs:bx],ax
	ret

	endp






Obicons 	proc	near

	mov	al,command
	call	getanyad
	cmp	al,255
	jz	cantopenit

	mov	ds,icons2
	mov	di,210
	mov	bx,1
	mov	al,4
	mov	ah,0
	call	showframe

cantopenit:	mov	ds,icons2
	mov	di,260
	mov	bx,1
	mov	al,1
	mov	ah,0
	call	showframe
	ret

	endp







Examicon	proc	near

	mov	ds,icons2
	mov	di,254
	mov	bx,5
	mov	al,3
	mov	ah,0
	call	showframe
	ret

	endp







Obpicture	proc	near

	mov	al,command
	mov	ah,objecttype
	cmp	ah,1
	jz	setframe
	cmp	ah,4
	jz	exframe

	mov	ds,freeframes
	mov	di,160
	mov	bx,68
	mov	cl,al
	add	al,al
	add	al,cl
	inc	al
	mov	ah,128
	call	showframe
	ret

setframe:	ret

exframe:	mov	ds,extras
	mov	di,160
	mov	bx,68
	mov	cl,al
	add	al,al
	add	al,cl
	inc	al
	mov	ah,128
	call	showframe
	ret

	endp










Describeob	proc	near

	call	getobtextstart

	mov	di,33
	mov	bx,92

	cmp	foreignrelease, 0
	jz notsetd
	cmp	objecttype,1
	jnz	notsetd
	mov	bx,82
notsetd:
	mov	dl,241
	mov	ah,16
	mov	charshift,91+91
	call	printdirect
	mov	charshift,0
	mov	di,36
	mov	bx,104
	cmp	foreignrelease, 0
	jz notsetd2
	cmp	objecttype,1
	jnz	notsetd2
	mov	bx,94
notsetd2:
	mov	dl,241
	mov	ah,0
	call	printdirect
	push	bx
	call	obsthatdothings
	pop	bx
	call	additionaltext
	ret

	
	endp





Additionaltext	proc	near

	add	bx,10
	push	bx
	mov	al,command
	mov	ah,objecttype
	mov	cl,"C"
	mov	ch,"U"
	mov	dl,"P"
	mov	dh,"E"
	call	compare
	jz	emptycup
	mov	al,command
	mov	ah,objecttype
	mov	cl,"C"
	mov	ch,"U"
	mov	dl,"P"
	mov	dh,"F"
	call	compare
	jz	fullcup
	pop	bx
	ret
emptycup:	mov	al,40
	call	findpuztext
	pop	bx
	mov	di,36
	mov	dl,241
	mov	ah,0
	call	printdirect
	ret
fullcup:	mov	al,39
	call	findpuztext
	pop	bx
	mov	di,36
	mov	dl,241
	mov	ah,0
	call	printdirect
	ret

	endp











Obsthatdothings	proc	near

	mov	al,command
	mov	ah,objecttype
	mov	cl,"M"
	mov	ch,"E"
	mov	dl,"M"
	mov	dh,"B"
	call	compare
	jnz	notlouiscard
	mov	al,4
	call	getlocation
	cmp	al,1
	jz	seencard
	mov	al,4
	call	setlocation
	call	lookatcard
seencard:	ret
notlouiscard:	ret

	endp







Getobtextstart	proc	near

	mov	es,freedesc
	mov	si,freetextdat
	mov	cx,freetext
	cmp	objecttype,2
	jz	describe
	mov	es,setdesc
	mov	si,settextdat
	mov	cx,settext
	cmp	objecttype,1
	jz	describe
	mov	es,extras
	mov	si,extextdat
	mov	cx,extext

describe:	mov	al,command
	mov	ah,0
	add	ax,ax
	add	si,ax
	mov	ax,[es:si]
	add	ax,cx
	mov	si,ax
	mov	bx,ax
tryagain:	push	si
	call	findnextcolon
	mov	al,[es:si]
	mov	cx,si
	pop	si
	cmp	objecttype,1
	jnz	cantmakeoneup
	cmp	al,0
	jz	findsometext
	cmp	al,":"
	jz	findsometext
cantmakeoneup:	ret

findsometext:	call	searchforsame
	jmp	tryagain
	ret

	endp






Searchforsame	proc	near
		
	mov	si,cx
searchagain:	inc	si
	mov	al,[es:bx]
search:	cmp	[es:si],al
	jz	gotstartletter
	inc	cx
	inc	si
	cmp	si,8000	;arbitrary give-up
	jc	search	;counter.
	mov	si,bx
	pop	ax
	ret

gotstartletter:	push	bx si
keepchecking:	inc	si
	inc	bx
	mov	al,[es:bx]
	mov	ah,[es:si]
	cmp	al,":"
	jz	foundmatch
	cmp	al,0
	jz	foundmatch
	cmp	al,ah
	jz	keepchecking
	pop	si bx
	jmp	searchagain

foundmatch:	pop	si bx
	ret

	endp







;-----------------------------------------------------------Using an object----





Findnextcolon	proc	near

isntcolon:	mov	al,[es:si]
	inc	si
	cmp	al,0
	jz	endofcolon
	cmp	al,":"
	jnz	isntcolon
endofcolon:	ret

	endp


;------------------------------------------------------Taking, dropping etc----






Inventory	proc	near

	cmp	mandead,1
	jz	iswatchinv
	cmp	watchingtime,0
	jz	notwatchinv
iswatchinv:	call	blank
	ret
notwatchinv:	cmp	commandtype,239
	jz	alreadyopinv
	mov	commandtype,239
	mov	al,32
	call	commandonly
alreadyopinv:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	cantopinv
	and	ax,1
	jnz	doopeninv
cantopinv:	ret

doopeninv:	mov	timecount,0
	mov	pointermode,0
	mov	inmaparea,0
	call	animpointer
	call	createpanel
	call	showpanel
	call	examicon
	call	showman
	call	showexit
	call	undertextline
	mov	pickup,0
	mov	invopen,2
	call	openinv
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
	mov	openedob,255
	jmp	waitexam		; very naughty!

	endp






Setpickup	proc	near

	cmp	objecttype,1
	jz	cantpick
	cmp	objecttype,3
	jz	cantpick
	call	getanyad
	mov	al,[es:bx+2]
	cmp	al,4
	jnz	canpick
cantpick:	call	blank
	ret

canpick:	cmp	commandtype,209
	jz	alreadysp
	mov	commandtype,209

	mov	bl,command
	mov	bh,objecttype
	mov	al,33
	call	commandwithob
alreadysp:	mov	ax,mousebutton
	cmp	ax,1
	jnz	nosetpick
	cmp	ax,oldbutton
	jnz	dosetpick
nosetpick:	ret

dosetpick:	call	createpanel
	call	showpanel
	call	showman
	call	showexit
	call	examicon
	mov	pickup,1
	mov	invopen,2
	cmp	objecttype,4
	jz	pickupexob

	mov	al,command
	mov	itemframe,al
	mov	openedob,255
	call	transfertoex
	mov	itemframe,al
	mov	objecttype,4
	call	geteitherad
	mov	byte ptr [es:bx+2],20 ; means it is in transit
	mov	byte ptr [es:bx+3],255
	call	openinv
	call	worktoscreenm
	ret

pickupexob:	mov	al,command
	mov	itemframe,al
	mov	openedob,255
	call	openinv
	call	worktoscreenm
	ret

	endp





Examinventory	proc	near

	cmp	commandtype,249
	jz	alreadyexinv
	mov	commandtype,249
	mov	al,32
	call	commandonly
alreadyexinv:	mov	ax,mousebutton
	and	ax,1
	jnz	doexinv
	ret

doexinv:	call	createpanel
	call	showpanel
	call	showman
	call	showexit
	call	examicon
	mov	pickup,0
	mov	invopen,2
	call	openinv
	call	worktoscreenm
	ret

	endp





Reexfrominv	proc	near

	call	findinvpos
	mov	ax,[es:bx]
	mov	commandtype,ah
	mov	command,al
	mov	examagain,1
	mov	pointermode,0
	ret

	endp







Reexfromopen	proc	near

	ret
	call	findopenpos
	mov	ax,[es:bx]
	mov	commandtype,ah
	mov	command,al
	mov	examagain,1
	mov	pointermode,0
	ret

	endp



















Swapwithinv	proc	near

	mov	al,itemframe
	mov	ah,objecttype
	cmp	ax,oldsubject
	jnz	difsub7
	cmp	commandtype,243
	jz	alreadyswap1
	mov	commandtype,243

difsub7:	mov	oldsubject,ax
	mov	bx,ax
	mov	al,34
	call	commandwithob
alreadyswap1:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	cantswap1
	and	ax,1
	jnz	doswap1
cantswap1:	ret

doswap1:	mov	ah,objecttype
	mov	al,itemframe
	push	ax

	call	findinvpos
	mov	ax,[es:bx]
	mov	itemframe,al
	mov	objecttype,ah
	call	geteitherad
	mov	byte ptr [es:bx+2],20	; means unplaced object
	mov	byte ptr [es:bx+3],255
	mov	bl,itemframe
	mov	bh,objecttype

	pop	ax
	mov	objecttype,ah
	mov	itemframe,al
	push	bx

	call	findinvpos		;NONE OF THIS IS NEEDED
	call	delpointer		;ONLY EXTRAS CAN BE IN
	mov	al,itemframe
	call	geteitherad
	mov	byte ptr [es:bx+2],4
	mov	byte ptr [es:bx+3],255
	mov	al,lastinvpos
	mov	[es:bx+4],al

	pop	ax
	mov	objecttype,ah
	mov	itemframe,al
	call	fillryan
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
	ret

	endp










Swapwithopen	proc	near

	mov	al,itemframe
	mov	ah,objecttype
	cmp	ax,oldsubject
	jnz	difsub8
	cmp	commandtype,242
	jz	alreadyswap2
	mov	commandtype,242

difsub8:	mov	oldsubject,ax
	mov	bx,ax
	mov	al,34
	call	commandwithob
alreadyswap2:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	cantswap2
	and	ax,1
	jnz	doswap2
cantswap2:	ret

doswap2:	call	geteitherad
	call	isitworn
	jnz	notwornswap
	call	wornerror
	ret
notwornswap:	call	delpointer
	mov	al,itemframe
	cmp	al,openedob
	jnz	isntsame2
	mov	al,objecttype
	cmp	al,openedtype
	jnz	isntsame2
	call	errormessage1
	ret

isntsame2:	call	checkobjectsize
	cmp	al,0
	jz	sizeok2
	ret

sizeok2:	mov	ah,objecttype
	mov	al,itemframe
	push	ax

	call	findopenpos
	mov	ax,[es:bx]
	mov	itemframe,al
	mov	objecttype,ah
	
	cmp	ah,4
	jnz	makeswapex
	call	geteitherad
	mov	byte ptr [es:bx+2],20
	mov	byte ptr [es:bx+3],255
	jmp	actuallyswap

makeswapex:	call	transfertoex
	mov	itemframe,al
	mov	objecttype,4
	call	geteitherad
	mov	byte ptr [es:bx+2],20
	mov	byte ptr [es:bx+3],255

actuallyswap:	mov	bl,itemframe
	mov	bh,objecttype
	pop	ax
	mov	objecttype,ah
	mov	itemframe,al
	push	bx

	call	findopenpos
	call	geteitherad
	mov	al,openedtype
	mov	byte ptr [es:bx+2],al
	mov	al,openedob
	mov	byte ptr [es:bx+3],al
	mov	al,lastinvpos
	mov	[es:bx+4],al
	mov	al,reallocation
	mov	[es:bx+5],al

	pop	ax
	mov	objecttype,ah
	mov	itemframe,al
	call	fillopen
	call	fillryan
	call	undertextline
	call	readmouse
	call	useopened
	call	showpointer
	call	worktoscreen
	call	delpointer
	ret

	endp











Intoinv 	proc	near

	cmp	pickup,0
	jnz	notout	
	call	outofinv
	ret

notout: 	call	findinvpos
	mov	ax,[es:bx]
	cmp	al,255
	jz	canplace1
	call	swapwithinv
	ret

canplace1:	mov	al,itemframe
	mov	ah,objecttype
	cmp	ax,oldsubject
	jnz	difsub1
	cmp	commandtype,220
	jz	alreadyplce
	mov	commandtype,220

difsub1:	mov	oldsubject,ax
	mov	bx,ax
	mov	al,35
	call	commandwithob
alreadyplce:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	notletgo2
	and	ax,1
	jnz	doplace
notletgo2:	ret

doplace:	call	delpointer
	mov	al,itemframe
	call	getexad
	mov	byte ptr [es:bx+2],4
	mov	byte ptr [es:bx+3],255
	mov	al,lastinvpos
	mov	[es:bx+4],al

	mov	pickup,0
	call	fillryan
	call	readmouse
	call	showpointer
	call	outofinv
	call	worktoscreen
	call	delpointer
	ret

	endp







Deletetaken	proc	near		;gets rid of objects that were
				;transfered to exlist ages ago
	mov	es,freedat
	mov	ah,reallocation
	mov	ds,extras
	mov	si,exdata

	mov	cx,numexobjects
takenloop:	mov	al,[si+11]
	cmp	al,ah
	jnz	notinhere
	mov	bl,[si+1]
	mov	bh,0
	add	bx,bx
	add	bx,bx
	add	bx,bx
	add	bx,bx
	mov	byte ptr [es:bx+2],254 ; was 255

notinhere:	add	si,16
	loop	takenloop

	ret
	
	endp






Outofinv	proc	near

	call	findinvpos
	mov	ax,[es:bx]
	cmp	al,255
	jnz	canpick2
	call	blank
	ret

canpick2:	mov	bx,mousebutton
	cmp	bx,2
	jnz	canpick2a
	call	reexfrominv
	ret

canpick2a:	cmp	ax,oldsubject
	jnz	difsub3
	cmp	commandtype,221
	jz	alreadygrab
	mov	commandtype,221

difsub3:	mov	oldsubject,ax
	mov	bx,ax
	mov	al,36
	call	commandwithob
alreadygrab:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	notletgo
	and	ax,1
	jnz	dograb
notletgo:	ret

dograb: 	call	delpointer
	mov	pickup,1
	call	findinvpos
	mov	ax,[es:bx]
	mov	itemframe,al
	mov	objecttype,ah
	call	getexad
	mov	byte ptr [es:bx+2],20	; means unplaced object
	mov	byte ptr [es:bx+3],255
	call	fillryan
	call	readmouse
	call	showpointer
	call	intoinv
	call	worktoscreen
	call	delpointer
	ret
	
	endp





Getfreead	proc	near

	mov	ah,0
	mov	cl,4
	shl	ax,cl
	mov	bx,ax
	mov	es,freedat
	ret
	
	endp








Getexad 	proc	near

	mov	ah,0
	mov	bx,16
	mul	bx
	mov	bx,ax
	mov	es,extras
	add	bx,exdata
	ret
	
	endp






Geteitherad	proc	near

	cmp	objecttype,4
	jz	isinexlist
	mov	al,itemframe
	call	getfreead
	ret
isinexlist:	mov	al,itemframe
	call	getexad
	ret

	endp






Getanyad	proc	near		;nearly same as above
				;but uses command
	cmp	objecttype,4
	jz	isex
	cmp	objecttype,2
	jz	isfree
	mov	al,command
	call	getsetad
	mov	ax,[es:bx+4]
	ret
isfree: 	mov	al,command
	call	getfreead
	mov	ax,[es:bx+7]
	ret
isex:	mov	al,command
	call	getexad
	mov	ax,[es:bx+7]
	ret

	endp



Getanyaddir	proc	near		;nearly same as above
				;but uses ax
	cmp	ah,4
	jz	isex3
	cmp	ah,2
	jz	isfree3
	call	getsetad
	ret
isfree3:	call	getfreead
	ret
isex3:	call	getexad
	ret

	endp






Getopenedsize	proc	near		;nearly same as above again
				;but finds ad of opened ob
	cmp	openedtype,4
	jz	isex2
	cmp	openedtype,2
	jz	isfree2
	mov	al,openedob
	call	getsetad
	mov	ax,[es:bx+3]
	ret
isfree2:	mov	al,openedob
	call	getfreead
	mov	ax,[es:bx+7]
	ret
isex2:	mov	al,openedob
	call	getexad
	mov	ax,[es:bx+7]
	ret

	endp








Getsetad	proc	near

	mov	ah,0
	mov	bx,64
	mul	bx
	mov	bx,ax
	mov	es,setdat
	ret
	
	endp






Findinvpos	proc	near

	mov	cx,mousex
	sub	cx,inventx
	mov	bx,-1
findinv1:	inc	bx
	sub	cx,itempicsize
	jnc	findinv1

	mov	cx,mousey
	sub	cx,inventy
	sub	bx,5
findinv2:	add	bx,5
	sub	cx,itempicsize
	jnc	findinv2

	mov	al,ryanpage
	mov	ah,0
	mov	cx,10
	mul	cx
	add	bx,ax

	mov	al,bl
	mov	lastinvpos,al
	add	bx,bx

	mov	es,buffers
	add	bx,ryaninvlist
	ret

	endp









Findopenpos	proc	near

	mov	cx,mousex
	sub	cx,inventx
	mov	bx,-1
findopenp1:	inc	bx
	sub	cx,itempicsize
	jnc	findopenp1

	mov	al,bl
	mov	lastinvpos,al

	add	bx,bx
	mov	es,buffers
	add	bx,openinvlist
	ret

	endp












;--------------------------------------------------------Dropping an object----

Dropobject	proc	near

 	cmp	commandtype,223
	jz	alreadydrop
	mov	commandtype,223
	cmp	pickup,0
	jz	blank

	mov	bl,itemframe
	mov	bh,objecttype
	mov	al,37
	call	commandwithob
alreadydrop:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	nodrop
	and	ax,1
	jnz	dodrop
nodrop: 	ret

dodrop: 	call	geteitherad
	call	isitworn
	jnz	nowornerror
	call	wornerror
	ret
nowornerror:	cmp	reallocation,47
	jz	nodrop2
	mov	cl,ryanx
	add	cl,12
	mov	ch,ryany
	add	ch,12
	call	checkone
	cmp	cl,2
	jc	nodroperror
nodrop2:	call	droperror
	ret
nodroperror:	cmp	mapxsize,64
	jnz	notinlift
	cmp	mapysize,64
	jnz	notinlift
	call	droperror
	ret
notinlift:	mov	al,itemframe
	mov	ah,4
	mov	cl,"G"
	mov	ch,"U"
	mov	dl,"N"
	mov	dh,"A"
	call	compare
	jz	cantdrop
	mov	al,itemframe
	mov	ah,4
	mov	cl,"S"
	mov	ch,"H"
	mov	dl,"L"
	mov	dh,"D"
	call	compare
	jz	cantdrop
	mov	objecttype,4
	mov	al,itemframe
	call	getexad
	mov	byte ptr [es:bx+2],0
	mov	al,ryanx
	add	al,4
	mov	cl,4
	shr	al,cl
	add	al,mapx
	mov	ah,ryany
	add	ah,8
	mov	cl,4
	shr	ah,cl
	add	ah,mapy
	mov	byte ptr [es:bx+3],al
	mov	byte ptr [es:bx+5],ah
	mov	al,ryanx
	add	al,4
	and	al,15
	mov	ah,ryany
	add	ah,8
	and	ah,15
	mov	byte ptr [es:bx+4],al
	mov	byte ptr [es:bx+6],ah
	mov	pickup,0
	mov	al,reallocation
	mov	[es:bx],al
	ret

	endp




Droperror	proc	near

	mov	commandtype,255
	call	delpointer
	mov	di,76
	mov	bx,21
	mov	al,56
	mov	dl,240
	call	printmessage
	call	worktoscreenm
	mov	cx,50
	call	hangonp
	call	showpanel
	call	showman
	call	examicon
	mov	commandtype,255
	call	worktoscreenm
	ret

	endp




Cantdrop	proc	near

	mov	commandtype,255
	call	delpointer
	mov	di,76
	mov	bx,21
	mov	al,24
	mov	dl,240
	call	printmessage
	call	worktoscreenm
	mov	cx,50
	call	hangonp
	call	showpanel
	call	showman
	call	examicon
	mov	commandtype,255
	call	worktoscreenm
	ret

	endp



Wornerror	proc	near

	mov	commandtype,255
	call	delpointer
	mov	di,76
	mov	bx,21
	mov	al,57
	mov	dl,240
	call	printmessage
	call	worktoscreenm
	mov	cx,50
	call	hangonp
	call	showpanel
	call	showman
	call	examicon
	mov	commandtype,255
	call	worktoscreenm
	ret

	endp










Removeobfrominv	proc	near

	cmp	command,100
	jz	obnotexist
	call	getanyad
	mov	di,bx
	mov	cl,command
	mov	ch,0
	call	deleteexobject
	;mov	byte ptr [es:bx+2],0
obnotexist:	ret

	endp




;---------------------------------------------------------Opening an object----

Selectopenob	proc	near

	mov	al,command
	call	getanyad
	cmp	al,255
	jnz	canopenit1
	call	blank
	ret

canopenit1:	cmp	commandtype,224
	jz	alreadyopob
	mov	commandtype,224

	mov	bl,command
	mov	bh,objecttype
	mov	al,38
	call	commandwithob
alreadyopob:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	noopenob
	and	ax,1
	jnz	doopenob
noopenob:	ret

doopenob:	mov	al,command
	mov	openedob,al
	mov	al,objecttype
	mov	openedtype,al

	call	createpanel
	call	showpanel
	call	showman
	call	examicon
	call	showexit
	call	openinv
	call	openob
	call	undertextline
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
	ret

	endp










Useopened	proc	near

	cmp	openedob,255
	jz	cannotuseopen
	cmp	pickup,0
	jnz	notout2	
	call	outofopen
	ret

notout2:	call	findopenpos
	mov	ax,[es:bx]
	cmp	al,255
	jz	canplace3
	call	swapwithopen
cannotuseopen:	ret

canplace3:	cmp	pickup,1
	jz	intoopen
	call	blank
	ret

intoopen:	mov	al,itemframe
	mov	ah,objecttype
	cmp	ax,oldsubject
	jnz	difsub2
	cmp	commandtype,227
	jz	alreadyplc2
	mov	commandtype,227

difsub2:	mov	oldsubject,ax
	mov	bx,ax
	mov	al,35
	call	commandwithob
alreadyplc2:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	notletgo3
	cmp	ax,1
	jz	doplace2
notletgo3:	ret

doplace2:	call	geteitherad
	call	isitworn
	jnz	notworntoopen
	call	wornerror
	ret
notworntoopen:	call	delpointer
	mov	al,itemframe
	cmp	al,openedob
	jnz	isntsame
	mov	al,objecttype
	cmp	al,openedtype
	jnz	isntsame
	call	errormessage1
	ret

isntsame:	call	checkobjectsize
	cmp	al,0
	jz	sizeok1
	ret

sizeok1:	mov	pickup,0
	mov	al,itemframe
	call	geteitherad
	mov	al,openedtype
	mov	byte ptr [es:bx+2],al
	mov	al,openedob
	mov	byte ptr [es:bx+3],al
	mov	al,lastinvpos
	mov	[es:bx+4],al
	mov	al,reallocation
	mov	[es:bx+5],al
	call	fillopen
	call	undertextline
	call	readmouse
	call	useopened
	call	showpointer
	call	worktoscreen
	call	delpointer
	ret

	endp








Errormessage1	proc	near

	call	delpointer
	mov	di,76
	mov	bx,21
	mov	al,58
	mov	dl,240
	call	printmessage
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
	mov	cx,50
	call	hangonp
	call	showpanel
	call	showman
	call	examicon
	call	readmouse
	call	useopened
	call	showpointer
	call	worktoscreen
	call	delpointer
	ret

	endp







Errormessage2	proc	near

	mov	commandtype,255
	call	delpointer
	mov	di,76
	mov	bx,21
	mov	al,59
	mov	dl,240
	call	printmessage
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
	mov	cx,50
	call	hangonp
	call	showpanel
	call	showman
	call	examicon
	call	readmouse
	call	useopened
	call	showpointer
	call	worktoscreen
	call	delpointer
	ret

	endp










Errormessage3	proc	near

	call	delpointer
	mov	di,76
	mov	bx,21
	mov	al,60
	mov	dl,240
	call	printmessage
	call	worktoscreenm
	mov	cx,50
	call	hangonp
	call	showpanel
	call	showman
	call	examicon
	call	readmouse
	call	useopened
	call	showpointer
	call	worktoscreen
	call	delpointer
	ret

	endp




Checkobjectsize proc	near

	call	getopenedsize
	push	ax
	mov	al,itemframe
	call	geteitherad
	mov	al,[es:bx+9]
	pop	cx

	cmp	al,255	 	;gives a size of 6 if no
	jnz	notunsized	;size was defined in the editor
	mov	al,6			;could be a bad idea
notunsized:


	cmp	al,100
	jnc	specialcase
	cmp	cl,100
	jc	isntspecial
	call	errormessage3
	jmp	sizewrong
isntspecial:	cmp	cl,al
	jnc	sizeok
specialcase:	sub	al,100
	cmp	cl,100
	jnc	bothspecial
	cmp	cl,al
	jnc	sizeok
	call	errormessage2
	jmp	sizewrong
bothspecial:	sub	cl,100
	cmp	al,cl
	jz	sizeok
	call	errormessage3
sizewrong:	mov	al,1
	ret
sizeok: 	mov	al,0
	ret

	endp








Outofopen	proc	near

	cmp	openedob,255
	jz	cantuseopen
	call	findopenpos
	mov	ax,[es:bx]
	cmp	al,255
	jnz	canpick4
cantuseopen:	call	blank
	ret

canpick4:	cmp	ax,oldsubject
	jnz	difsub4
	cmp	commandtype,228
	jz	alreadygrb
	mov	commandtype,228

difsub4:	mov	oldsubject,ax
	mov	bx,ax
	mov	al,36
	call	commandwithob
alreadygrb:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	notletgo4
	cmp	ax,1
	jz	dogrb
	cmp	ax,2
	jnz	notletgo4
	call	reexfromopen
notletgo4:	ret

dogrb:	call	delpointer
	mov	pickup,1
	call	findopenpos
	mov	ax,[es:bx]
	mov	itemframe,al
	mov	objecttype,ah
	
	cmp	ah,4
	jnz	makeintoex
	call	geteitherad
	mov	byte ptr [es:bx+2],20	; means unplaced object
	mov	byte ptr [es:bx+3],255
	jmp	actuallyout

makeintoex:	call	transfertoex
	mov	itemframe,al
	mov	objecttype,4
	call	geteitherad
	mov	byte ptr [es:bx+2],20 ; means it is in transit
	mov	byte ptr [es:bx+3],255
	
actuallyout:	call	fillopen
	call	undertextline
	call	readmouse
	call	useopened
	call	showpointer
	call	worktoscreen
	call	delpointer
	ret

	endp





;All Extra object code - adding and removing plus purge routines -------------






Transfertoex	proc	near

	call	emergencypurge

	call	getexpos
	mov	al,expos
	push	ax

	push	di
	mov	al,itemframe
	mov	ah,0
	mov	bx,16
	mul	bx
	mov	ds,freedat
	mov	si,ax
	mov	cx,8
	rep	movsw
	pop	di

	mov	al,reallocation
	mov	[es:di],al
	mov	[es:di+11],al
	mov	al,itemframe
	mov	[es:di+1],al
	mov	byte ptr [es:di+2],4
	mov	byte ptr [es:di+3],255
	mov	al,lastinvpos
	mov	[es:di+4],al

	mov	al,itemframe
	mov	itemtotran,al
	call	transfermap
	call	transferinv
	call	transfertext

	mov	al,itemframe
	mov	ah,0
	mov	bx,16
	mul	bx
	mov	ds,freedat
	mov	si,ax
	mov	byte ptr [si+2],254 ; was 255
	call	pickupconts

	pop	ax
	ret

	endp














Pickupconts	proc	near

	mov	al,[si+7]
	cmp	al,255
	jz	notopenable

	mov	al,itemframe
	mov	ah,objecttype
	mov	dl,expos
	;dec	dl
	mov	es,freedat
	mov	bx,0
	mov	cx,0
pickupcontloop:	push	cx es bx dx ax

	cmp	byte ptr [es:bx+2],ah
	jnz	notinsidethis
	cmp	byte ptr [es:bx+3],al
	jnz	notinsidethis

	mov	itemtotran,cl
	call	transfercontoex

notinsidethis:	pop	ax dx bx es cx
	add	bx,16
	inc	cx
	cmp	cx,80
	jnz	pickupcontloop

notopenable:	ret

	endp







Transfercontoex	proc	near

	push	es bx

	push	dx es bx
	call	getexpos
	pop	si ds

	push	di
	mov	cx,8
	rep	movsw
	pop	di
	pop	dx

	mov	al,reallocation
	mov	[es:di],al
	mov	[es:di+11],al
	mov	al,itemtotran
	mov	[es:di+1],al
	mov	[es:di+3],dl
	mov	byte ptr [es:di+2],4

	call	transfermap
	call	transferinv
	call	transfertext
	;inc	expos

	pop	si ds
	mov	byte ptr [si+2],255
	ret

	endp


















Transfertext	proc	near

	mov	es,extras
	mov	al,expos
	mov	ah,0
	add	ax,ax
	mov	bx,extextdat
	add	bx,ax
	mov	di,extextpos
	mov	[es:bx],di
	add	di,extext

	mov	al,itemtotran
	mov	ah,0
	add	ax,ax
	mov	ds,freedesc
	mov	bx,freetextdat
	add	bx,ax
	mov	si,freetext
	mov	ax,[bx]
	add	si,ax

moretext:	lodsb
	stosb
	inc	extextpos
	cmp	al,0
	jnz	moretext
	ret

	endp










Getexpos	proc	near


	mov	es,extras
	mov	al,0
	mov	di,exdata
tryanotherex:	cmp	byte ptr [es:di+2],255
	jz	foundnewex
	add	di,16
	inc	al
	cmp	al,numexobjects
	jnz	tryanotherex
foundnewex:	mov	expos,al
	ret

	endp









Purgealocation	proc	near

	push	ax
	mov	es,extras
	mov	di,exdata
	pop	bx
	mov	cx,0
purgeloc:	cmp	bl,[es:di+0]
	jnz	dontpurge
	cmp	byte ptr [es:di+2],0
	jnz	dontpurge
	push	di es bx cx
	call	deleteexobject
	pop	cx bx es di
dontpurge:	add	di,16
	inc	cx
	cmp	cx,numexobjects
	jnz	purgeloc
              	ret

	endp



	

Emergencypurge	proc	near

checkpurgeagain:	mov	ax,exframepos
	add	ax,4000
	cmp	ax,exframeslen
	jc	notnearframeend
	call	purgeanitem
	jmp	checkpurgeagain
notnearframeend:	mov	ax,extextpos
	add	ax,400
	cmp	ax,extextlen
	jc	notneartextend
	call	purgeanitem
	jmp	checkpurgeagain
notneartextend:	ret

	endp






Purgeanitem	proc	near

	mov	es,extras	;first time try and
	mov	di,exdata	;find an object in a
	mov	bl,reallocation	;location other than
	mov	cx,0	;the one the player is
lookforpurge:	mov	al,[es:di+2]	;in
	cmp	al,0
	jnz	cantpurge
	cmp	byte ptr [es:di+12],2
	jz	iscup
	cmp	byte ptr [es:di+12],255
	jnz	cantpurge
iscup:	cmp	byte ptr [es:di+11],bl
	jz	cantpurge
	call	deleteexobject
	ret
cantpurge:	add	di,16
	inc	cx
	cmp	cx,numexobjects
	jnz	lookforpurge
	
	mov	di,exdata
	mov	bl,reallocation
	mov	cx,0
lookforpurge2:	mov	al,[es:di+2]
	cmp	al,0
	jnz	cantpurge2
	cmp	byte ptr [es:di+12],255
	jnz	cantpurge2
	call	deleteexobject
	ret
cantpurge2:	add	di,16
	inc	cx
	cmp	cx,numexobjects
	jnz	lookforpurge2
	ret

	endp





Deleteexobject	proc 	near	;es:di holds data ad
			;cx holds number
	push	cx cx cx cx
	mov	al,255
	mov	cx,16
	rep	stosb
         	pop	ax
	mov	cl,al
	add	al,al
	add	al,cl
	call	deleteexframe
	pop	ax
	mov	cl,al
	add	al,al
	add	al,cl
	inc	al
	call	deleteexframe
	pop	ax
	call	deleteextext

	pop	bx
	mov	bh,bl
	mov	bl,4
	mov	di,exdata
	mov	cx,0
deleteconts:	cmp	[es:di+2],bx
	jnz	notinsideex	
	push	bx cx di
	call	deleteexobject	;Oooh missus!
	pop	di cx bx	;Recursive code!
notinsideex:	add	di,16
	inc	cx
	cmp	cx,numexobjects
	jnz	deleteconts
	ret

	endp





Deleteexframe	proc	near	;al holds frame to delete

	mov	di,exframedata
	mov	ah,0
	add	ax,ax
	add	di,ax
	add	ax,ax
	add	di,ax
	mov	al,[es:di]
	mov	ah,0
	mov	cl,[es:di+1]
	mov	ch,0
	mul	cx	;ax holds size of this
			;frame in bytes
	mov	si,[es:di+2]
	push	si
	add	si,exframes 
	mov	cx,exframeslen
	sub	cx,[es:di+2]
	mov	di,si	;di/si hold start of frame
	add	si,ax	;si holds end of frame
	push	ax
	push	es
	pop	ds
	rep	movsb
	pop	bx	;bx holds size now
	sub	exframepos,bx
	pop	si	;si holds start of frame 
			;(offset only)
	mov	cx,numexobjects*3
	mov	di,exframedata
shuffleadsdown:	mov	ax,[es:di+2]
	cmp	ax,si
	jc	beforethisone
	sub	ax,bx
beforethisone:	mov	[es:di+2],ax
               	add	di,6
	loop	shuffleadsdown
	ret

	endp




Deleteextext	proc	near

	mov	di,extextdat
	mov	ah,0
	add	ax,ax
	add	di,ax
	mov	ax,[es:di]
	mov	si,ax
	mov	di,ax
	add	si,extext
	add	di,extext
	mov	ax,0
findlenextext:	mov	cl,[es:si]
	inc	ax
	inc	si
	cmp	cl,0
	jnz	findlenextext	

	mov	cx,extextlen
	mov	bx,si
	sub	bx,extext
	push	bx ax
	sub	cx,bx
	rep	movsb
	pop	bx
	sub	extextpos,bx
	
	pop	si
	mov	cx,numexobjects
	mov	di,extextdat
shuffletextads:	mov	ax,[es:di]
	cmp	ax,si
	jc	beforethistext
	sub	ax,bx
beforethistext:	mov	[es:di],ax
               	add	di,2
	loop	shuffletextads
	ret

	endp
