;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text

Useobject	proc	near

	mov	withobject,255

	cmp	commandtype,229
	jz	alreadyuse
	mov	commandtype,229

	mov	bl,command
	mov	bh,objecttype
	mov	al,51
	call	commandwithob
alreadyuse:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	nouse
	and	ax,1
	jnz	douse
nouse:	ret

douse:	call	useroutine
	ret

	endp







Useroutine	proc	near

	cmp	reallocation,50
	jc	nodream7
	cmp	pointerpower,0
	jnz	powerok
	ret
powerok:	mov	pointerpower,0

nodream7:	call	getanyad
	mov	dx,seg uselist
	mov	ds,dx
	mov	si,offset uselist
checkuselist:	push	si
	lodsb
	sub	al,"A"
	cmp	al,[es:bx+12]
	jnz	failed
	lodsb
	sub	al,"A"
	cmp	al,[es:bx+13]
	jnz	failed
	lodsb
	sub	al,"A"
	cmp	al,[es:bx+14]
	jnz	failed
	lodsb
	sub	al,"A"
	cmp	al,[es:bx+15]
	jnz	failed
	lodsw
	pop	si
	call	ax
	ret
failed: 	pop	si
	add	si,6
	cmp	byte ptr [si],140
	jnz	checkuselist

	call	delpointer
	call	getobtextstart
	call	findnextcolon
	cmp	al,0
	jz	cantuse2
	call	findnextcolon
	cmp	al,0
	jz	cantuse2
	mov	al,[es:si]
	cmp	al,0
	jz	cantuse2
	call	usetext
	mov	cx,400
	call	hangonp
	call	putbackobstuff
	ret
cantuse2:	call	createpanel
	call	showpanel
	call	showman
	call	showexit
	call	obicons
	mov	di,33
	mov	bx,100
	mov	al,63
	mov	dl,241
	call	printmessage
	call	worktoscreenm
	mov	cx,50
	call	hangonp
	call	putbackobstuff
	mov	commandtype,255
	ret

Uselist:	db	"NETW"
	dw      usemon
	db	"ELVA"
	dw	useelevator1
	db	"ELVB"
	dw	useelevator2
	db	"ELVC"
	dw	useelevator3
	db	"ELVE"
	dw	useelevator4
	db	"ELVF"
	dw	useelevator5
	db	"CGAT"
	dw	usechurchgate
	db	"REMO"
	dw	usestereo
	db	"BUTA"
	dw	usebuttona
	db	"CBOX"
	dw	usewinch
	db	"LITE"
	dw	uselighter
	db	"PLAT"
	dw	useplate
	db	"LIFT"
	dw	usecontrol
	db	"WIRE"
	dw	usewire
	db	"HNDL"
	dw	usehandle
	db	"HACH"
	dw	usehatch
	db	"DOOR"
	dw	useelvdoor
	db	"CSHR"
	dw	usecashcard
	db	"GUNA"
	dw	usegun
	db	"CRAA"
	dw	usecardreader1
	db	"CRBB"
	dw	usecardreader2
	db	"CRCC"
	dw	usecardreader3
	db	"SEAT"
	dw	sitdowninbar
	db	"MENU"
	dw	usemenu
	db	"COOK"
	dw	usecooker
	db	"ELCA"
	dw	callhotellift
	db	"EDCA"
	dw	calledenslift
	db	"DDCA"
	dw	calledensdlift
	db	"ALTR"
	dw	usealtar
	db	"LOKA"
	dw	openhoteldoor
	db	"LOKB"
	dw	openhoteldoor2
	db	"ENTA"
	dw	openlouis
	db	"ENTB"
	dw	openryan
	db	"ENTE"
	dw	openpoolboss
	db	"ENTC"
	dw	openyourneighbour
	db	"ENTD"
	dw	openeden
	db	"ENTH"
	dw	opensarters
	db	"WWAT"
	dw	wearwatch
	db	"POOL"
	dw	usepoolreader
	db	"WSHD"
	dw	wearshades
	db	"GRAF"
	dw	grafittidoor
	db	"TRAP"
	dw	trapdoor
	db	"CDPE"
	dw	edenscdplayer

	db	"DLOK"
	dw	opentvdoor

	db	"HOLE"
	dw	usehole

	db	"DRYR"
	dw	usedryer

	db	"HOLY"
	dw	usechurchhole
	
	db	"WALL"
	dw	usewall
	db	"BOOK"
	dw	usediary

	db	"AXED"
	dw	useaxe
	db	"SHLD"
	dw	useshield

	db	"BCNY"
	dw	userailing
	db	"LIDC"
	dw	usecoveredbox
	db	"LIDU"
	dw	useclearbox
	db	"LIDO"
	dw	useopenbox
	db	"PIPE"
	dw	usepipe

	db	"BALC"
	dw	usebalcony
	db	"WIND"
	dw	usewindow
	db	"PAPR"
	dw	viewfolder
	
	db	"UWTA"
	dw	usetrainer
	db	"UWTB"
	dw	usetrainer
	
	db	"STAT"
	dw	entersymbol
	db	"TLID"
	dw	opentomb
	db	"SLAB"
	dw	useslab
	db	"CART"
	dw	usecart
	db	"FCAR"
	dw	usefullcart


	db	"SLBA"
	dw	slabdoora
	db	"SLBB"
	dw	slabdoorb
	db	"SLBC"
	dw	slabdoorc
	db	"SLBD"
	dw	slabdoord
	db	"SLBE"
	dw	slabdoore
	db	"SLBF"
	dw	slabdoorf
	db	"PLIN"
	dw	useplinth

	db	"LADD"
	dw	useladder
	db	"LADB"
	dw	useladderb

	db	"GUMA"
	dw	chewy
	
	db	"SQEE"
	dw	wheelsound
	db	"TAPP"
	dw	runtap
	db	"GUIT"
	dw	playguitar
	db	"CONT"
	dw	hotelcontrol

	db	"BELL"
	dw	hotelbell

	db	140,140,140,140

	endp






;-----------------------------------------------------------Puzzle routines----


Wheelsound	proc	near

	mov	al,17
	call	playchannel1
	call	showfirstuse
	call	putbackobstuff
	ret

	endp




Runtap	proc	near

	cmp	withobject,255
	jnz	tapwith
	call	withwhat
	ret
tapwith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"C"
	mov	ch,"U"
	mov	dl,"P"
	mov	dh,"E"
	call	compare
	jz	fillcupfromtap
	mov	al,withobject
	mov	ah,withtype
	mov	cl,"C"
	mov	ch,"U"
	mov	dl,"P"
	mov	dh,"F"
	call	compare
	jz	cupfromtapfull
	mov	cx,300
	mov	al,56
	call	showpuztext
	call	putbackobstuff
	ret

fillcupfromtap:	mov	al,withobject
	call	getexad
	mov	byte ptr [es:bx+15],"F"-"A"
	mov	al,8
	call	playchannel1
	mov	cx,300
	mov	al,57
	call	showpuztext
	call	putbackobstuff
	ret

cupfromtapfull:	mov	cx,300
	mov	al,58
	call	showpuztext
	call	putbackobstuff
	ret


	endp



Playguitar	proc	near

	mov	al,14
	call	playchannel1
	call	showfirstuse
	call	putbackobstuff
	ret

	endp



Hotelcontrol	proc	near

	cmp	reallocation,21
	jnz	notrightcont
	cmp	mapx,33
	jnz	notrightcont
	call	showfirstuse
	call	putbackobstuff
	ret
notrightcont:	call	showseconduse
	call	putbackobstuff
	ret

	endp




Hotelbell	proc	near

	if	demo
	mov	al,24
	else
	mov	al,12
	endif
	call	playchannel1
	call	showfirstuse
	call	putbackobstuff
	ret

	endp





Opentomb	proc	near

	inc	progresspoints
	call	showfirstuse
	mov	watchingtime,35*2
	mov	reeltowatch,1
	mov	endwatchreel,33
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	ret

	endp



Usetrainer	proc	near

	call	getanyad
	cmp	byte ptr [es:bx+2],4
	jnz	notheldtrainer
	inc	progresspoints
	call	makeworn
	call	showseconduse
	call	putbackobstuff
	ret
notheldtrainer:	call	nothelderror
	ret

	endp



Nothelderror	proc	near

	call	createpanel
	call	showpanel
	call	showman
	call	showexit
	call	obicons
	mov	di,64
	mov	bx,100
	mov	al,63
	mov	ah,1
	mov	dl,201
	call	printmessage2
	call	worktoscreenm
	mov	cx,50
	call	hangonp
	call	putbackobstuff
	ret

	endp



Usepipe	proc	near
	
	cmp	withobject,255
	jnz	pipewith
	call	withwhat
	ret
pipewith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"C"
	mov	ch,"U"
	mov	dl,"P"
	mov	dh,"E"
	call	compare
	jz	fillcup
	mov	al,withobject
	mov	ah,withtype
	mov	cl,"C"
	mov	ch,"U"
	mov	dl,"P"
	mov	dh,"F"
	call	compare
	jz	alreadyfull
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret

fillcup:	mov	cx,300
	mov	al,36
	call	showpuztext
	call	putbackobstuff
	mov	al,withobject
	call	getexad
	mov	byte ptr [es:bx+15],"F"-"A"
	ret

alreadyfull:	mov	cx,300
	mov	al,35
	call	showpuztext
	call	putbackobstuff
	ret

	endp






Usefullcart	proc	near

	inc	progresspoints
	mov	al,2
	mov	ah,roomnum
	add	ah,6
	call	turnanypathon
	mov	manspath,4
	mov	facing,4
	mov	turntoface,4
	mov	finaldest,4
	call	findxyfrompath
	mov	resetmanxy,1
	call	showfirstuse
	mov	watchingtime,72*2
	mov	reeltowatch,58
	mov	endwatchreel,142
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	ret

	endp







Useplinth	proc	near

	cmp	withobject,255
	jnz	plinthwith
	call	withwhat
	ret

plinthwith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"D"
	mov	ch,"K"
	mov	dl,"E"
	mov	dh,"Y"
	call	compare
	jz	isrightkey
	call	showfirstuse
	call	putbackobstuff
	ret

isrightkey:	inc	progresspoints
	call	showseconduse
	mov	watchingtime,220
	mov	reeltowatch,0
	mov	endwatchreel,104
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	mov	al,roomafterdream
	mov	newlocation,al
	ret

	endp



Chewy	proc	near

	call	showfirstuse
	call	getanyad
	mov	byte ptr [es:bx+2],255
	mov	getback,1
	ret

	endp





Useladder	proc	near

	call	showfirstuse
	sub	mapx,11
	call	findroominloc
	mov	facing,6
	mov	turntoface,6
	mov	manspath,0
	mov	destination,0
	mov	finaldest,0
	call	findxyfrompath
	mov	resetmanxy,1
	mov	getback,1
	ret

	endp





Useladderb	proc	near

	call	showfirstuse
	add	mapx,11
	call	findroominloc
	mov	facing,2
	mov	turntoface,2
	mov	manspath,1
	mov	destination,1
	mov	finaldest,1
	call	findxyfrompath
	mov	resetmanxy,1
	mov	getback,1
	ret

	endp





Slabdoora	proc	near

	call	showfirstuse
	mov	getback,1
	mov	watchspeed,1
	mov	speedcount,1
	mov	reeltowatch,13
	cmp	dreamnumber,3
	jnz	slabawrong
	inc	progresspoints
	mov	watchingtime,60
	mov	endwatchreel,42
	mov	newlocation,47
	ret
slabawrong:	mov	watchingtime,40
	mov	endwatchreel,34
	mov	watchspeed,1
	mov	speedcount,1
	ret

	endp




Slabdoorb	proc	near

	cmp	dreamnumber,1
	jnz	slabbwrong
	mov	al,"S"
	mov	ah,"H"
	mov	cl,"L"
	mov	ch,"D"
	call	isryanholding
	jnz	gotcrystal
	mov	al,44
	mov	cx,200	
	call	showpuztext
	call	putbackobstuff
	ret
gotcrystal:	call	showfirstuse
	inc	progresspoints
	mov	getback,1
	mov	watchspeed,1
	mov	speedcount,1
	mov	reeltowatch,44
	mov	watchingtime,60
	mov	endwatchreel,71
	mov	newlocation,47
	ret
slabbwrong:	call	showfirstuse
	mov	getback,1
	mov	watchspeed,1
	mov	speedcount,1
	mov	reeltowatch,44
	mov	watchingtime,40
	mov	endwatchreel,63
	mov	watchspeed,1
	mov	speedcount,1
	ret

	endp



Slabdoord	proc	near

	call	showfirstuse
	mov	getback,1
	mov	watchspeed,1
	mov	speedcount,1
	mov	reeltowatch,75
	cmp	dreamnumber,0
	jnz	slabcwrong
	inc	progresspoints
	mov	watchingtime,60
	mov	endwatchreel,102
	mov	newlocation,47
	ret
slabcwrong:	mov	watchingtime,40
	mov	endwatchreel,94
	mov	watchspeed,1
	mov	speedcount,1
	ret

	endp



Slabdoorc	proc	near

	call	showfirstuse
	mov	getback,1
	mov	watchspeed,1
	mov	speedcount,1
	mov	reeltowatch,108
	cmp	dreamnumber,4
	jnz	slabdwrong
	inc	progresspoints
	mov	watchingtime,60
	mov	endwatchreel,135
	mov	newlocation,47
	ret
slabdwrong:	mov	watchingtime,40
	mov	endwatchreel,127
	mov	watchspeed,1
	mov	speedcount,1
	ret

	endp



Slabdoore	proc	near

	call	showfirstuse
	mov	getback,1
	mov	watchspeed,1
	mov	speedcount,1
	mov	reeltowatch,141
	cmp	dreamnumber,5
	jnz	slabewrong
	inc	progresspoints
	mov	watchingtime,60
	mov	endwatchreel,168
	mov	newlocation,47
	ret
slabewrong:	mov	watchingtime,40
	mov	endwatchreel,160
	mov	watchspeed,1
	mov	speedcount,1
	ret

	endp



Slabdoorf	proc	near

	call	showfirstuse
	mov	getback,1
	mov	watchspeed,1
	mov	speedcount,1
	mov	reeltowatch,171
	cmp	dreamnumber,2
	jnz	slabfwrong
	inc	progresspoints
	mov	watchingtime,60
	mov	endwatchreel,197
	mov	newlocation,47
	ret
slabfwrong:	mov	watchingtime,40
	mov	endwatchreel,189
	mov	watchspeed,1
	mov	speedcount,1
	ret

	endp






Useslab	proc	near
	
	cmp	withobject,255
	jnz	slabwith
	call	withwhat
	ret
slabwith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"J"
	mov	ch,"E"
	mov	dl,"W"
	mov	dh,"L"
	call	compare
	jz	nextslab
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret
nextslab:	mov	al,withobject
	call	getexad
	mov	byte ptr [es:bx+2],0
	mov	al,command
	push	ax
	call	removesetobject
	pop	ax
	inc	al
	push	ax
	call	placesetobject
	pop	ax
	cmp	al,54
	jnz	notlastslab
	mov	al,0
	call	turnpathon
	mov	watchingtime,22
	mov	reeltowatch,35
	mov	endwatchreel,48
	mov	watchspeed,1
	mov	speedcount,1
notlastslab:	inc	progresspoints
	call	showfirstuse
	mov	getback,1
	ret

	endp





Usecart	proc	near
	
	cmp	withobject,255
	jnz	cartwith
	call	withwhat
	ret
cartwith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"R"
	mov	ch,"O"
	mov	dl,"C"
	mov	dh,"K"
	call	compare
	jz	nextcart
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret
nextcart:	mov	al,withobject
	call	getexad
	mov	byte ptr [es:bx+2],0
	mov	al,command
	push	ax
	call	removesetobject
	pop	ax
	inc	al
	call	placesetobject
	inc	progresspoints
	mov	al,17
	call	playchannel1
	call	showfirstuse
	mov	getback,1
	ret

	endp








Useclearbox	proc	near

	cmp	withobject,255
	jnz	clearboxwith
	call	withwhat
	ret
clearboxwith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"R"
	mov	ch,"A"
	mov	dl,"I"
	mov	dh,"L"
	call	compare
	jz	openbox
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret

openbox:	inc	progresspoints
	call	showfirstuse
	mov	watchingtime,80
	mov	reeltowatch,67
	mov	endwatchreel,105
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	ret
	
	endp





Usecoveredbox	proc	near

	inc	progresspoints
	call	showfirstuse
	mov	watchingtime,50
	mov	reeltowatch,41
	mov	endwatchreel,66
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	ret

	endp



Userailing	proc	near

	call	showfirstuse
	mov	watchingtime,80
	mov	reeltowatch,0
	mov	endwatchreel,30
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	mov	mandead,4
	ret

	endp





Useopenbox	proc	near

	cmp	withobject,255
	jnz	openboxwith
	call	withwhat
	ret
openboxwith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"C"
	mov	ch,"U"
	mov	dl,"P"
	mov	dh,"F"
	call	compare
	jz	destoryopenbox
	mov	al,withobject
	mov	ah,withtype
	mov	cl,"C"
	mov	ch,"U"
	mov	dl,"P"
	mov	dh,"E"
	call	compare
	jz	openboxwrong
	call	showfirstuse
	ret

destoryopenbox:	inc	progresspoints
	mov	cx,300
	mov	al,37
	call	showpuztext
	mov	al,withobject
	call	getexad
	mov	byte ptr [es:bx+15],"E"-"A"
	mov	watchingtime,140
	mov	reeltowatch,105
	mov	endwatchreel,181
	mov	watchspeed,1
	mov	speedcount,1
	mov	al,4
	call	turnpathon
	mov	getback,1
	ret

openboxwrong:	mov	cx,300
	mov	al,38
	call	showpuztext
	call	putbackobstuff
	ret

	endp














Wearwatch	proc	near

	cmp	watchon,1
	jz	wearingwatch
	call	showfirstuse
	mov	watchon,1
	mov	getback,1
	call	getanyad
	call	makeworn
	ret
wearingwatch:	call	showseconduse
	call	putbackobstuff
	ret

	endp




Wearshades	proc	near

	cmp	shadeson,1
	jz	wearingshades
	mov	shadeson,1
	call	showfirstuse
	mov	getback,1
	call	getanyad
	call	makeworn
	ret
wearingshades:	call	showseconduse
	call	putbackobstuff
	ret
	
	endp



Sitdowninbar	proc	near

	cmp	watchmode,-1
	jnz	satdown
	call	showfirstuse
	mov	watchingtime,50
	mov	reeltowatch,55
	mov	endwatchreel,71
	mov	reeltohold,73
	mov	endofholdreel,83
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	ret
satdown:	call	showseconduse
	call	putbackobstuff
	ret

	endp



Usechurchhole	proc	near

	call	showfirstuse
	mov	getback,1
	mov	watchingtime,28
	mov	reeltowatch,13
	mov	endwatchreel,26
	mov	watchspeed,1
	mov	speedcount,1
	ret

	endp



Usehole	proc	near

	cmp	withobject,255
	jnz	holewith
	call	withwhat
	ret
holewith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"H"
	mov	ch,"N"
	mov	dl,"D"
	mov	dh,"A"
	call	compare
	jz	righthand
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret

righthand:	call	showfirstuse
	mov	al,86
	call	removesetobject
	mov	al,withobject
	call	getexad
	mov	byte ptr [es:bx+2],255
	mov	canmovealtar,1
	mov	getback,1
	ret

	endp




Usealtar	proc	near

	mov	al,"C"
	mov	ah,"N"
	mov	cl,"D"
	mov	ch,"A"
	call	findexobject
	cmp	al,numexobjects
	jz	thingsonaltar
	mov	al,"C"
	mov	ah,"N"
	mov	cl,"D"
	mov	ch,"B"
	call	findexobject
	cmp	al,numexobjects
	jz	thingsonaltar
	cmp	canmovealtar,1
	jz	movealtar
	mov	cx,300
	mov	al,23
	call	showpuztext
	mov	getback,1
         	ret

movealtar:	inc	progresspoints
	call	showseconduse
	mov	watchingtime,160
	mov	reeltowatch,81
	mov	endwatchreel,174
	mov	watchspeed,1
	mov	speedcount,1

	mov	al,47		;message number
	mov	bl,52		;x pos of message
	mov	bh,76		;and y pos
	mov	cx,32		;time on screen
	mov	dx,98		;pause before show
	call	setuptimeduse
	mov	getback,1
	ret

thingsonaltar:	call	showfirstuse
	mov	getback,1
	ret

	endp









Opentvdoor	proc	near

	cmp	withobject,255
	jnz	tvdoorwith
	call	withwhat
	ret
tvdoorwith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"U"
	mov	ch,"L"
	mov	dl,"O"
	mov	dh,"K"
	call	compare
	jz	keyontv
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret

keyontv:	call	showfirstuse
	mov	lockstatus,0
	mov	getback,1
	ret

	endp







Usedryer	proc	near

	mov	al,12
	call	playchannel1
	call	showfirstuse
	mov	getback,1
	ret

	endp





Openlouis	proc	near

	mov	al,5
	mov	ah,2
	mov	cl,3
	mov	ch,8
	call	entercode
	mov	getback,1
	ret

	endp




Nextcolon	proc	near

lookcolon:	mov	al,[es:si]
	inc	si
	cmp	al,":"
	jnz	lookcolon
	ret

	endp




Openyourneighbour	proc	near

	mov	al,255
	mov	ah,255
	mov	cl,255
	mov	ch,255
	call	entercode
	mov	getback,1
	ret

	endp



Usewindow	proc	near

	cmp	manspath,6
	jnz	notonbalc
	inc	progresspoints
	call	showfirstuse
	mov	newlocation,29	
	mov	getback,1
	ret
notonbalc:	call	showseconduse
	call	putbackobstuff
	ret

	endp



Usebalcony	proc	near

	call	showfirstuse
	mov	al,6
	call	turnpathon
	mov	al,0
	call	turnpathoff
	mov	al,1
	call	turnpathoff
	mov	al,2
	call	turnpathoff
	mov	al,3
	call	turnpathoff
	mov	al,4
	call	turnpathoff
	mov	al,5
	call	turnpathoff
	inc	progresspoints
	mov	manspath,6
	mov	destination,6
	mov	finaldest,6
	call	findxyfrompath
	call	switchryanoff
	mov	resetmanxy,1

	mov	watchingtime,30*2
	mov	reeltowatch,183
	mov	endwatchreel,212
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	ret

	endp



Openryan	proc	near

	mov	al,5
	mov	ah,1
	mov	cl,0
	mov	ch,6
	call	entercode
	mov	getback,1
	ret

	endp



Openpoolboss	proc	near

	mov	al,5
	mov	ah,2
	mov	cl,2
	mov	ch,2
	call	entercode
	mov	getback,1
	ret

	endp




Openeden	proc	near

	mov	al,2
	mov	ah,8
	mov	cl,6
	mov	ch,5
	call	entercode
	mov	getback,1
	ret

	endp


Opensarters	proc	near

	mov	al,7
	mov	ah,8
	mov	cl,3
	mov	ch,3
	call	entercode
	mov	getback,1
	ret

	endp




Isitright	proc	near

	mov	bx,seg presslist
	mov	es,bx
	mov	bx,offset es:presslist
	cmp	[es:bx+0],al
	jnz	notright
	cmp	[es:bx+1],ah
	jnz	notright
	cmp	[es:bx+2],cl
	jnz	notright
	cmp	[es:bx+3],ch
notright:	ret

	endp




Drawitall	proc	near

	call	createpanel
	call	drawfloor
	;call	dumpallmap
	call	printsprites
	call	showicon
	ret

	endp




Openhoteldoor	proc	near

	cmp	withobject,255
	jnz	hoteldoorwith
	call	withwhat
	ret
hoteldoorwith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"K"
	mov	ch,"E"
	mov	dl,"Y"
	mov	dh,"A"
	call	compare
	jz	keyonhotel1
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret

keyonhotel1:	if	demo
	mov	al,27
	else
	mov	al,16
	endif
	call	playchannel1
	call	showfirstuse
	;mov	destination,1
	;mov	finaldest,1
	;call	autosetwalk
	mov	lockstatus,0
	mov	getback,1
	ret

	endp



Openhoteldoor2	proc	near

	cmp	withobject,255
	jnz	hoteldoorwith2
	call	withwhat
	ret
hoteldoorwith2:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"K"
	mov	ch,"E"
	mov	dl,"Y"
	mov	dh,"A"
	call	compare
	jz	keyonhotel2
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret

keyonhotel2:	if	demo
	mov	al,27
	else
	mov	al,16
	endif
	call	playchannel1
	call	showfirstuse
	call	putbackobstuff
	ret

	endp






Grafittidoor	proc	near

	cmp	withobject,255
	jnz	grafwith
	call	withwhat
	ret
grafwith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"A"
	mov	ch,"P"
	mov	dl,"E"
	mov	dh,"N"
	call	compare
	jz	dograf
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret

dograf: 	call	showfirstuse
	call	putbackobstuff
	ret

	endp





Trapdoor	proc	near

	inc	progresspoints
	call	showfirstuse
	call	switchryanoff
	mov	watchingtime,20*2
	mov	reeltowatch,181
	mov	endwatchreel,197
	mov	newlocation,26
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	ret

	endp




Callhotellift	proc	near

	if	demo
	mov	al,24
	else
	mov	al,12
	endif
	call	playchannel1
	call	showfirstuse
	mov	counttoopen,8
	mov	getback,1
	mov	destination,5
	mov	finaldest,5
	call	autosetwalk
	mov	al,4
	call	turnpathon
	ret

	endp




Calledenslift	proc	near

	call	showfirstuse
	mov	counttoopen,8
	mov	getback,1
	mov	al,2
	call	turnpathon
	ret

	endp



Calledensdlift	proc	near

	cmp	liftflag,1
	jz	edensdhere
	call	showfirstuse
	mov	counttoopen,8
	mov	getback,1
	mov	al,2
	call	turnpathon
	ret
edensdhere:	call	showseconduse
	call	putbackobstuff
	ret

	endp






Usepoolreader	proc	near

	cmp	withobject,255
	jnz	poolwith
	call	withwhat
	ret
poolwith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"M"
	mov	ch,"E"
	mov	dl,"M"
	mov	dh,"B"
	call	compare
	jz	openpool
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret

openpool:	cmp	talkedtoattendant,1
	jz	canopenpool
	call	showseconduse
	call	putbackobstuff
	ret

canopenpool:	mov	al,17
	call	playchannel1
	call	showfirstuse
	mov	counttoopen,6
	mov	getback,1
	ret

	endp













Uselighter	proc	near

	cmp	withobject,255
	jnz	gotlighterwith
	call	withwhat
	ret
gotlighterwith: 	mov	al,withobject
	mov	ah,withtype
	mov	cl,"S"
	mov	ch,"M"
	mov	dl,"K"
	mov	dh,"E"
	call	compare
	jz	cigarette
	call	showfirstuse
	call	putbackobstuff
	ret
cigarette:	mov	cx,300
	mov	al,9
	call	showpuztext
	mov	al,withobject
	call	getexad
	mov	byte ptr [es:bx+2],255
	mov	getback,1
	ret

	endp













Showseconduse	proc	near

	call	getobtextstart
	call	nextcolon
	call	nextcolon
	call	nextcolon
	call	usetext
	mov	cx,400
	call	hangonp
	ret

	endp






Usecardreader1	proc	near

	cmp	withobject,255
	jnz	gotreader1with
	call	withwhat
	ret
gotreader1with:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"C"
	mov	ch,"S"
	mov	dl,"H"
	mov	dh,"R"
	call	compare
	jz	correctcard
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret
correctcard:	cmp	talkedtosparky,0
	jz	notyet
	cmp	card1money,0
	jz	getscash
	mov	cx,300
	mov	al,17
	call	showpuztext
	call	putbackobstuff
	ret
getscash:	mov	al,16
	call	playchannel1
	mov	cx,300
	mov	al,18
	call	showpuztext
	inc	progresspoints
	mov	card1money,12432
	mov	getback,1
	ret
notyet:	call	showfirstuse
	call	putbackobstuff
	ret

	endp




Usecardreader2	proc	near

	cmp	withobject,255
	jnz	gotreader2with
	call	withwhat
	ret
gotreader2with:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"C"
	mov	ch,"S"
	mov	dl,"H"
	mov	dh,"R"
	call	compare
	jz	correctcard2
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret

correctcard2:	cmp	talkedtoboss,0
	jz	notyetboss
	cmp	card1money,0
	jz	nocash
	cmp	gunpassflag,2
	jz	alreadygotnew
	mov	al,18
	call	playchannel1
	mov	cx,300
	mov	al,19
	call	showpuztext
	mov	al,94
	call	placesetobject
	mov	gunpassflag,1
	sub	card1money,2000
	inc	progresspoints
	mov	getback,1
	ret
nocash: 	mov	cx,300
	mov	al,20
	call	showpuztext
	call	putbackobstuff
	ret
alreadygotnew:	mov	cx,300
	mov	al,22
	call	showpuztext
	call	putbackobstuff
	ret
notyetboss:	call	showfirstuse
	call	putbackobstuff
	ret

	endp







Usecardreader3	proc	near

	cmp	withobject,255
	jnz	gotreader3with
	call	withwhat
	ret
gotreader3with:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"C"
	mov	ch,"S"
	mov	dl,"H"
	mov	dh,"R"
	call	compare
	jz	rightcard
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret

rightcard:	cmp	talkedtorecep,0
	jz	notyetrecep
	cmp	cardpassflag,0
	jnz	alreadyusedit
	if	demo
	mov	al,27
	else
	mov	al,16
	endif
	call	playchannel1
	mov	cx,300
	mov	al,25
	call	showpuztext
	inc	progresspoints
	sub	card1money,8300
	mov	cardpassflag,1
	mov	getback,1
	ret
alreadyusedit:	mov	cx,300
	mov	al,26
	call	showpuztext
	call	putbackobstuff
	ret
notyetrecep:	call	showfirstuse
	call	putbackobstuff
	ret

	endp








Usecashcard	proc	near

	call	getridofreels
	call	loadkeypad
	call	createpanel
	call	showpanel
	call	showexit
	call	showman

	mov	di,114
	mov	bx,120
	cmp	foreignrelease, 0
	jz $1
	mov	bx,120-3
$1:
	mov	ds,tempgraphics
	mov	al,39
	mov	ah,0
	call	showframe

	mov	ax,card1money
	call	moneypoke

	call	getobtextstart
	call	nextcolon
	call	nextcolon

	mov	di,36
	mov	bx,98
	mov	dl,241
	mov	al,0
	mov	ah,0
	call	printdirect

	mov	di,160
	mov	bx,155
	push	cs
	pop	es
	mov	si,offset cs:money1poke
	mov	charshift,91*2+75 
	mov	al,0
	mov	ah,0
	mov	dl,240
	call	printdirect
	mov	di,187
	mov	bx,155
	push	cs
	pop	es
	mov	si,offset cs:money2poke
	mov	charshift,91*2+85
	mov	al,0
	mov	ah,0
	mov	dl,240
	call	printdirect
	mov	charshift,0
	call	worktoscreenm
	mov	cx,400
	call	hangonp
	call	getridoftemp
	call	restorereels
	call	putbackobstuff
	ret

money1poke:	db	"0000",0
money2poke:	db	"00",0
		
	endp




Lookatcard	proc	near

	mov	manisoffscreen,1
	call	getridofreels
	call	loadkeypad

	call	createpanel2
	mov	di,160
	mov	bx,80
	mov	ds,tempgraphics
	mov	al,42
	mov	ah,128
	call	showframe

	call	getobtextstart
	call	findnextcolon
	call	findnextcolon
	call	findnextcolon
	mov	di,36
	mov	bx,124
	mov	dl,241
	mov	al,0
	mov	ah,0
	call	printdirect

	push	es si
	call	worktoscreenm
	mov	cx,280
	call	hangonw
	call	createpanel2
	mov	di,160
	mov	bx,80
	mov	ds,tempgraphics
	mov	al,42
	mov	ah,128
	call	showframe
	pop	si es

	mov	di,36
	mov	bx,130
	mov	dl,241
	mov	al,0
	mov	ah,0
	call	printdirect
	call	worktoscreenm

	mov	cx,200
	call	hangonw
	mov	manisoffscreen,0
	call	getridoftemp
	call	restorereels
	call	putbackobstuff
	ret

	endp





Moneypoke	proc	near
	
	mov	bx,offset cs:money1poke
	mov	cl,48-1
numberpoke0:	inc	cl
	sub	ax,10000
	jnc	numberpoke0
	add	ax,10000
	mov	[cs:bx],cl
	inc	bx

	mov	cl,48-1
numberpoke1:	inc	cl
	sub	ax,1000
	jnc	numberpoke1
	add	ax,1000
	mov	[cs:bx],cl
	inc	bx

	mov	cl,48-1
numberpoke2:	inc	cl
	sub	ax,100
	jnc	numberpoke2
	add	ax,100
	mov	[cs:bx],cl
	inc	bx

	mov	cl,48-1
numberpoke3:	inc	cl
	sub	ax,10
	jnc	numberpoke3
	add	ax,10
	mov	[cs:bx],cl

	mov	bx,offset cs:money2poke
	add	al,48
	mov	[cs:bx],al
	ret

	endp







Usecontrol	proc	near

	cmp	withobject,255
	jnz	gotcontrolwith
	call	withwhat
	ret
gotcontrolwith: 	mov	al,withobject
	mov	ah,withtype
	mov	cl,"K"
	mov	ch,"E"
	mov	dl,"Y"
	mov	dh,"A"
	call	compare
	jz	rightkey
	cmp	reallocation,21
	jnz	balls
	mov	al,withobject
	mov	ah,withtype
	mov	cl,"K"
	mov	ch,"N"
	mov	dl,"F"
	mov	dh,"E"
	call	compare
	jz	jimmycontrols
	mov	al,withobject
	mov	ah,withtype
	mov	cl,"A"
	mov	ch,"X"
	mov	dl,"E"
	mov	dh,"D"
	call	compare
	jz	axeoncontrols

balls:	call	showfirstuse
	call	putbackobstuff
	ret

rightkey:	mov	al,16
	call	playchannel1
	cmp	location,21
	jz	goingdown
	mov	cx,300
	mov	al,0
	call	showpuztext
	mov	newlocation,21
	mov	counttoclose,8
	mov	counttoopen,0
	mov	watchingtime,80
	mov	getback,1
	ret

goingdown:	mov	cx,300
	mov	al,3
	call	showpuztext
	mov	newlocation,30
	mov	counttoclose,8
	mov	counttoopen,0
	mov	watchingtime,80
	mov	getback,1
	ret

jimmycontrols:	mov	al,50
	call	placesetobject
	mov	al,51
	call	placesetobject
	mov	al,26
	call	placesetobject
	mov	al,30
	call	placesetobject
	mov	al,16
	call	removesetobject
	mov	al,17
	call	removesetobject
	if	demo
	mov	al,26
	else
	mov	al,14
	endif
	call	playchannel1
	mov	cx,300
	mov	al,10
	call	showpuztext
	inc	progresspoints
	mov	getback,1
	ret

axeoncontrols:	mov	cx,300
	mov	al,16
	call	showpuztext
	inc	progresspoints
	call	putbackobstuff
	ret

	endp






Usehatch	proc	near

	call	showfirstuse
	mov	newlocation,40
	mov	getback,1
	ret

	endp





Usewire 	proc	near

	cmp	withobject,255
	jnz	gotwirewith
	call	withwhat
	ret
gotwirewith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"K"
	mov	ch,"N"
	mov	dl,"F"
	mov	dh,"E"
	call	compare
	jz	wireknife
	mov	al,withobject
	mov	ah,withtype
	mov	cl,"A"
	mov	ch,"X"
	mov	dl,"E"
	mov	dh,"D"
	call	compare
	jz	wireaxe

	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret

wireaxe:	mov	cx,300
	mov	al,16
	call	showpuztext
	call	putbackobstuff
	ret

wireknife:	mov	al,51
	call	removesetobject
	mov	al,52
	call	placesetobject
	mov	cx,300
	mov	al,11
	call	showpuztext
	inc	progresspoints
	mov	getback,1
	ret

	endp







Usehandle	proc	near

	mov	al,"C"
	mov	ah,"U"
	mov	cl,"T"
	mov	ch,"W"
	call	findsetobject
	mov	al,[es:bx+58]
	cmp	al,255
	jnz	havecutwire
	mov	cx,300
	mov	al,12
	call	showpuztext
	mov	getback,1
	ret

havecutwire:	mov	cx,300
	mov	al,13
	call	showpuztext
	mov	newlocation,22
	mov	getback,1
	ret

	endp








Useelevator1	proc	near

	call	showfirstuse
	call	selectlocation
	mov	getback,1
	ret

	endp






Showfirstuse	proc	near		;shows but does not delete the
				;first bit of text after the
	call	getobtextstart		;description
	call	findnextcolon
	call	findnextcolon
	call	usetext
	mov	cx,400
	call	hangonp
	ret

	endp





Useelevator3	proc	near

	call	showfirstuse
	mov	counttoclose,20
	mov	newlocation,34
	mov	reeltowatch,46
	mov	endwatchreel,63
	mov	watchspeed,1
	mov	speedcount,1
	mov	watchingtime,80 ;40
	mov	getback,1
	ret

	endp




Useelevator4	proc	near

	call	showfirstuse
	mov	reeltowatch,0
	mov	endwatchreel,11
	mov	watchspeed,1
	mov	speedcount,1
	mov	counttoclose,20
	mov	watchingtime,80 ;40
	mov	getback,1
	mov	newlocation,24
	ret

	endp



Useelevator2	proc	near

	cmp	location,23
	jz	inpoolhall
	call	showfirstuse
	mov	newlocation,23
	mov	counttoclose,20
	mov	counttoopen,0
	mov	watchingtime,80
	mov	getback,1
	ret
inpoolhall:	call	showfirstuse
	mov	newlocation,31
	mov	counttoclose,20
	mov	counttoopen,0
	mov	watchingtime,80
	mov	getback,1
	ret

	endp




Useelevator5	proc	near

	mov	al,4
	call	placesetobject
	mov	al,0
	call	removesetobject
	mov	newlocation,20
	mov	watchingtime,80
	mov	liftflag,1
	mov	counttoclose,8
	mov	getback,1
	ret

	endp





Usekey	proc	near

	cmp	location,5
	jz	usekey1
	cmp	location,30
	jz	usekey1
	cmp	location,21
	jz	usekey2
	mov	cx,200
	mov	al,1
	call	showpuztext
	call	putbackobstuff
	ret

usekey1:	cmp	mapx,22
	jnz	wrongroom1
	cmp	mapy,10
	jnz	wrongroom1
	mov	cx,300
	mov	al,0
	call	showpuztext
	mov	counttoclose,100
	mov	getback,1
	ret

usekey2:	cmp	mapx,11
	jnz	wrongroom1
	cmp	mapy,10
	jnz	wrongroom1
	mov	cx,300
	mov	al,3
	call	showpuztext
	mov	newlocation,30
	mov	al,2
	call	fadescreendown
	call	showfirstuse
	call	putbackobstuff
	ret

wrongroom1:	mov	cx,200
	mov	al,2
	call	showpuztext
	call	putbackobstuff
	ret

	endp







Usestereo	proc	near

	cmp	location,0
	jz	stereook
	mov	cx,400		;Ryan isn't in his flat.
	mov	al,4
	call	showpuztext
	call	putbackobstuff
	ret

stereook:	cmp	mapx,11
	jnz	stereonotok
	cmp	mapy,0
	jz	stereook2
stereonotok:	mov	cx,400		;Ryan isn't in his bedroom.
	mov	al,5
	call	showpuztext
	call	putbackobstuff
	ret

stereook2:	mov	al,"C"
	mov	ah,"D"
	mov	cl,"P"
	mov	ch,"L"
	call	findsetobject	;find object number of CD player
	mov	ah,1		;searching for inside a set ob
	call	checkinside	;see if there is anything inside
	cmp	cl,numexobjects
	jnz	cdinside
	mov	al,6		;Need a CD inside
	mov	cx,400
	call	showpuztext
	call	putbackobstuff
	call	getanyad		;if the CD's been taken out,
	mov	al,255		;make sure the player isn't still
	mov	[es:bx+10],al	;playing, ie:reset the puzzle
	ret			;flag for the remote.

cdinside:	call	getanyad
	mov	al,[es:bx+10]
	xor	al,1
	mov	[es:bx+10],al
	cmp	al,255
	jz	stereoon
	mov	al,7		;The stereo works
	mov	cx,400
	call	showpuztext
	call	putbackobstuff
	ret

stereoon:	mov	al,8		;Stereo was already on,
	mov	cx,400		;so switch it off
	call	showpuztext
	call	putbackobstuff
	ret

	endp















Usecooker	proc	near

	mov	al,command
	mov	ah,objecttype
	call	checkinside	;see if there is anything inside
	cmp	cl,numexobjects
	jnz	foodinside
	call	showfirstuse
	call	putbackobstuff
	ret			;flag for the remote.

foodinside:	call	showseconduse
	call	putbackobstuff
	ret

	endp






Useaxe	proc	near

	cmp	reallocation,22
	jnz	notinpool
	cmp	mapy,10
	jz	axeondoor
	call	showseconduse
	inc	progresspoints
	mov	lastweapon,2
	mov	getback,1
	call	removeobfrominv
	ret

notinpool:	call	showfirstuse
	ret

	endp








Useelvdoor	proc	near

	cmp	withobject,255
	jnz	gotdoorwith
	call	withwhat
	ret
gotdoorwith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"A"
	mov	ch,"X"
	mov	dl,"E"
	mov	dh,"D"
	call	compare
	jz	axeondoor
	mov	al,14
	mov	cx,300
	call	showpuztext
	call	putbackobstuff
	ret

axeondoor:	mov	al,15
	mov	cx,300
	call	showpuztext
	inc	progresspoints

	mov	watchingtime,46*2
	mov	reeltowatch,31
	mov	endwatchreel,77
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	ret

	endp

;------------------------------------------------------------------------------

Withwhat	proc	near		;Gets player to identify object
				;to use selected item with.
	call	createpanel
	call	showpanel
	call	showman
	call	showexit
	mov	al,command
	mov	ah,objecttype
	push	cs
	pop	es
	mov	di,offset cs:commandline
	call	copyname

	mov	di,100
	mov	bx,21
	mov	dl,200
	mov	al,63
	mov	ah,2
	call	printmessage2

	mov	di,lastxpos
	add	di,5
	mov	bx,21
	push	cs
	pop	es
	mov	si,offset cs:commandline
	mov	dl,220
	mov	al,0
	mov	ah,0
	call	printdirect

	mov	di,lastxpos
	add	di,5
	mov	bx,21
	mov	dl,200
	mov	al,63
	mov	ah,3
	call	printmessage2

	call	fillryan
	mov	commandtype,255
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
	mov	invopen,2
	ret

	endp





Selectob	proc	near

	call	findinvpos
	mov	ax,[es:bx]
	cmp	al,255
	jnz	canselectob
	call	blank
	ret

canselectob:	mov	withobject,al
	mov	withtype,ah
	cmp	ax,oldsubject
	jnz	diffsub3
	cmp	commandtype,221
	jz	alreadyselob
	mov	commandtype,221

diffsub3:	mov	oldsubject,ax
	mov	bx,ax
	mov	al,0
	call	commandwithob
alreadyselob:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	notselob
	and	ax,1
	jnz	doselob
notselob:	ret

doselob:	call	delpointer
	mov	invopen,0
	call	useroutine
	ret

	endp








Compare 	proc	near

	sub	dl,"A"
	sub	dh,"A"
	sub	cl,"A"
	sub	ch,"A"
	push	cx dx
	call	getanyaddir
	pop	dx cx
	cmp	[es:bx+12],cx
	jnz	comparefin
	cmp	[es:bx+14],dx
comparefin:	ret

	endp











Findsetobject	proc	near		;searches set object ID's
				;for contents of ax,cx
	sub	al,"A"	 ;returns number in al and data
	sub	ah,"A"	 ;start point in es:bx
	sub	cl,"A"
	sub	ch,"A"
	mov	es,setdat
	mov	bx,0
	mov	dl,0		;dl counts object number
findsetloop:	cmp	al,[es:bx+12]
	jnz	nofind
	cmp	ah,[es:bx+13]
	jnz	nofind
	cmp	cl,[es:bx+14]
	jnz	nofind
	cmp	ch,[es:bx+15]
	jnz	nofind
	mov	al,dl
	ret
nofind: 	add	bx,64
	inc	dl
	cmp	dl,128		;number of objects to search
	jnz	findsetloop
	mov	al,dl
	ret

	endp






Findexobject	proc	near			;searches ex object ID's
					;for contents of ax,cx
	sub	al,"A"	 	;returns number in al and data
	sub	ah,"A"	 	;start point in es:bx
	sub	cl,"A"
	sub	ch,"A"
	mov	es,extras
	mov	bx,exdata
	mov	dl,0			;dl counts object number
findexloop:	cmp	al,[es:bx+12]
	jnz	nofindex
	cmp	ah,[es:bx+13]
	jnz	nofindex
	cmp	cl,[es:bx+14]
	jnz	nofindex
	cmp	ch,[es:bx+15]
	jnz	nofindex
	mov	al,dl
	ret
nofindex:	add	bx,16
	inc	dl
	cmp	dl,numexobjects	 	;number of objects to search
	jnz	findexloop
	mov	al,dl
	ret

	endp



Isryanholding	proc	near

	sub	al,"A"	 	;returns number in al and data
	sub	ah,"A"	 	;start point in es:bx
	sub	cl,"A"
	sub	ch,"A"
	mov	es,extras
	mov	bx,exdata
	mov	dl,0			;dl counts object number
searchinv:	cmp	byte ptr [es:bx+2],4
	jnz	nofindininv
	cmp	al,[es:bx+12]
	jnz	nofindininv
	cmp	ah,[es:bx+13]
	jnz	nofindininv
	cmp	cl,[es:bx+14]
	jnz	nofindininv
	cmp	ch,[es:bx+15]
	jnz	nofindininv
	mov	al,dl
	cmp	al,numexobjects
	ret
nofindininv:	add	bx,16
	inc	dl
	cmp	dl,numexobjects	 	;number of objects to search
	jnz	searchinv
	mov	al,dl
	cmp	al,numexobjects		;if not zero he is holding
	ret			;if zero, he is not holding

	endp




Checkinside	proc	near		;finds an extra object inside
		  	;object number al, type ah

	mov	es,extras
	mov	bx,exdata
	mov	cl,0
insideloop:	cmp	al,[es:bx+3]	 ;OI! might need to check room number!!!
	jnz	notfoundinside
	cmp	ah,[es:bx+2]
	jnz	notfoundinside
	ret
notfoundinside: 	add	bx,16
	inc	cl
	cmp	cl,numexobjects
	jnz	insideloop
	ret			;ch returns the object number
				;in the extras list
	endp












Usetext 	proc	near

	push	es si
	call	createpanel
	call	showpanel
	call	showman
	call	showexit
	call	obicons
	pop	si es

	mov	di,36
	mov	bx,104
	mov	dl,241
	mov	al,0
	mov	ah,0
	call	printdirect

	call	worktoscreenm
	ret

	endp





Putbackobstuff	proc	near

	call	createpanel
	call	showpanel
	call	showman
	call	obicons
	call	showexit
	call	obpicture
	call	describeob
	call	undertextline
	mov	commandtype,255
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
	ret

	endp







Showpuztext	proc	near

	push	cx
	call	findpuztext	
	push	es si
	call	createpanel
	call	showpanel
	call	showman
	call	showexit
	call	obicons
	pop	si es
	mov	di,36
	mov	bx,104
	mov	dl,241
	mov	ah,0
	call	printdirect
	call	worktoscreenm
	pop	cx
	call	hangonp
	ret

	endp



Findpuztext	proc	near

	mov	ah,0
	mov	si,ax
	add	si,si
	mov	es,puzzletext
	mov	ax,[es:si]
	add	ax,textstart
	mov	si,ax
	ret

	endp



;-------------------------------------------------------------------------------

Placesetobject	proc	near

	push	es bx
	mov	cl,0
	mov	ch,0
	call	findormake
	call	getsetad
	mov	byte ptr [es:bx+58],0
	pop	bx es
	ret

	endp




Removesetobject proc	near

	push	es bx
	mov	cl,255
	mov	ch,0
	call	findormake
	call	getsetad
	mov	byte ptr [es:bx+58],255
	pop	bx es
	ret

	endp




Issetobonmap	proc	near

	push	es bx
	call	getsetad
	mov	al,[es:bx+58]
	pop	bx es
	cmp	al,0
	ret

	endp






Placefreeobject	proc	near

	push	es bx
	mov	cl,0
	mov	ch,1
	call	findormake
	call	getfreead
	mov	byte ptr [es:bx+2],0
	pop	bx es
	ret

	endp





Removefreeobject	proc	near

	push	es bx
	;mov	cl,255
	;mov	ch,1
	;call	findormake
	call	getfreead
	mov	byte ptr [es:bx+2],255
	pop	bx es
	ret

	endp










Findormake	proc	near

	mov	bx,listofchanges
	push	ax
	mov	es,buffers
	mov	ah,reallocation
changeloop:	cmp	byte ptr [es:bx],255
	jz	haventfound
	cmp	ax,[es:bx]
	jnz	nofoundchange
              	cmp	ch,[es:bx+3]
	jz	foundchange
nofoundchange:	add	bx,4
	jmp	changeloop
foundchange:	pop	ax
	mov	[es:bx+2],cl
	ret
haventfound:	mov	[es:bx],ax
	mov	[es:bx+2],cx
	pop	ax
	ret

	endp








Switchryanon	proc	near

	mov	ryanon,255
	ret

	endp





Switchryanoff	proc	near

	mov	ryanon,1
	ret

	endp



Setallchanges	proc	near

	mov	es,buffers
	mov	bx,listofchanges
setallloop:	mov	ax,[es:bx]
	cmp	al,255
	jz	endsetloop
	mov	cx,[es:bx+2]
	add	bx,4
	cmp	ah,reallocation
	jnz	setallloop
	push	es bx
	call	dochange
	pop	bx es
	jmp	setallloop
endsetloop:	ret

	endp






Dochange	proc	near

	cmp	ch,0
	jz	object
	cmp	ch,1
	jz	freeobject

path:	push	cx
	mov	ah,0
	add	ax,ax
	add	ax,ax
	add	ax,ax
	push	ax
	mov	al,ch
	sub	al,100
	mov	ah,0
	mov	cx,144
	mul	cx
	pop	bx
	add	bx,ax
	add	bx,pathdata
	mov	es,reels
	pop	cx
	mov	byte ptr [es:bx+6],cl
nopath:	ret

object:	push	cx
	call	getsetad
	pop	cx
	mov	[es:bx+58],cl
	ret

freeobject:	push	cx
	call	getfreead
	pop	cx
	cmp	byte ptr [es:bx+2],255
	jnz	beenpickedup
	mov	[es:bx+2],cl
beenpickedup:	ret

	endp



Autoappear	proc	near				;places objects that appear
			;in rooms after certain
	cmp	location,32			;conditions are met.
	jnz	notinalley
	mov	al,5	;switch off travel to
	call	resetlocation	;hotel after kill
	mov	al,10
	call	setlocation
	mov	destpos,10
	ret
notinalley:	cmp	reallocation,24
	jnz	notinedens
	cmp	generaldead,1
	jnz	edenspart2
	inc	generaldead
	mov	al,44
	call	placesetobject
	mov	al,18
	call	placesetobject
	mov	al,93
	call	placesetobject
	mov	al,92
	call	removesetobject
	mov	al,55
	call	removesetobject
	mov	al,75
	call	removesetobject
	mov	al,84
	call	removesetobject
	mov	al,85
	call	removesetobject
	ret
edenspart2:	cmp	sartaindead,1
	jnz	notedens2
	mov	al,44
	call	removesetobject
	mov	al,93
	call	removesetobject
	mov	al,55
	call	placesetobject
	inc	sartaindead
notedens2:	ret	
notinedens:	cmp	reallocation,25
	jnz	notonsartroof
	mov	newsitem,3
	mov	al,6
	call	resetlocation	 ;turn off Sartain Industries
	mov	al,11
	call	setlocation		;turn on carpark for later
	mov	destpos,11
	ret
notonsartroof:	cmp	reallocation,2
	jnz	notinlouiss
	cmp	rockstardead,0
	jz	notinlouiss
	mov	al,23
	call	placesetobject
notinlouiss:	ret

	endp










;--------------------------------------------------------- Timed text stuff ----



Getundertimed	proc	near

	mov	al,timedy
	cmp	foreignrelease, 0
	jz $1
	sub	al,3
$1:
	mov	ah,0
	mov	bx,ax
	mov	al,timedx
	mov	ah,0
	mov	di,ax
	mov	ch,undertimedysize
	mov	cl,240
	mov	ds,buffers
	mov	si,undertimedtext
	call	multiget
	ret

	endp




Putundertimed	proc	near

	mov	al,timedy
	cmp	foreignrelease, 0
	jz $1
	sub	al,3
$1:
	mov	ah,0
	mov	bx,ax
	mov	al,timedx
	mov	ah,0
	mov	di,ax
	mov	ch,undertimedysize
	mov	cl,240
	mov	ds,buffers
	mov	si,undertimedtext
	call	multiput
	ret

	endp






Dumptimedtext	proc	near

	cmp	needtodumptimed,1
	jnz	nodumptimed
	mov	al,timedy
	cmp	foreignrelease, 0
	jz $1
	sub	al,3
$1:
	mov	ah,0
	mov	bx,ax
	mov	al,timedx
	mov	ah,0
	mov	di,ax
	mov	cl,240
	mov	ch,undertimedysize
	call	multidump
	mov	needtodumptimed,0
nodumptimed:	ret

	endp








Setuptimeduse	proc	near

	cmp	timecount,0
	jnz	cantsetup

	mov	timedy,bh
	mov	timedx,bl
	mov	counttotimed,cx
	add	dx,cx
	mov	timecount,dx
	mov	bl,al
	mov	bh,0
	add	bx,bx
	mov	es,puzzletext
	mov	cx,textstart
	mov	ax,[es:bx]
	add	ax,cx
	mov	bx,ax
	mov	timedseg,es
	mov	timedoffset,bx
cantsetup:	ret

	endp



Setuptimedtemp	proc	near

	if	cd
	cmp	ah,0
	jz	notloadspeech3
	push ax
	push bx
	push cx
	push dx
	mov	dl,"T"
	mov	dh,ah
	mov	cl,"T"
	mov	ah,0
	call	loadspeech
	cmp	speechloaded,1
	jnz	$1
	mov	al,50+12
	call	playchannel1
$1:
	pop dx
	pop cx
	pop bx
	pop ax

	cmp	speechloaded,1
	jnz	notloadspeech3 ; failed to load speech
	cmp	subtitles, 1
	jz	notloadspeech3
	ret

notloadspeech3:
	endif	;if cd

	cmp	timecount,0
	jnz	cantsetup2
	mov	timedy,bh
	mov	timedx,bl
	mov	counttotimed,cx
	add	dx,cx
	mov	timecount,dx
	mov	bl,al
	mov	bh,0
	add	bx,bx
	mov	es,textfile1
	mov	cx,textstart
	mov	ax,[es:bx]
	add	ax,cx
	mov	bx,ax
	mov	timedseg,es
	mov	timedoffset,bx
cantsetup2:	ret

	endp







Usetimedtext	proc	near

	cmp	timecount,0
	jz	notext
	dec	timecount
	cmp	timecount,0
	jz	deltimedtext
	mov	ax,timecount
	cmp	ax,counttotimed
	jz	firsttimed
	jnc	notext
	jmp	notfirsttimed
firsttimed:	call	getundertimed
notfirsttimed:	mov	bl,timedy
	mov	bh,0
	mov	al,timedx
	mov	ah,0
	mov	di,ax
	mov	es,timedseg
	mov	si,timedoffset
	mov	dl,237
	mov	ah,0
	call	printdirect
	mov	needtodumptimed,1
notext: 	ret

deltimedtext:	call	putundertimed
	mov	needtodumptimed,1
	ret

	endp












Edenscdplayer	proc	near

	call	showfirstuse
	mov	watchingtime,18*2
	mov	reeltowatch,25
	mov	endwatchreel,42
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	ret

	endp





Usewall	proc	near

	call	showfirstuse
	cmp	manspath,3
	jz	gobackover
	mov	watchingtime,30*2
	mov	reeltowatch,2
	mov	endwatchreel,31
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	mov	al,3
	call	turnpathon
	mov	al,4
	call	turnpathon
	mov	al,0
	call	turnpathoff
	mov	al,1
	call	turnpathoff
	mov	al,2
	call	turnpathoff
	mov	al,5
	call	turnpathoff
	mov	manspath,3
	mov	finaldest,3
	call	findxyfrompath
	mov	resetmanxy,1
	call	switchryanoff
	ret
gobackover:	mov	watchingtime,30*2
	mov	reeltowatch,34
	mov	endwatchreel,60
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	mov	al,3
	call	turnpathoff
	mov	al,4
	call	turnpathoff
	mov	al,0
	call	turnpathon
	mov	al,1
	call	turnpathon
	mov	al,2
	call	turnpathon
	mov	al,5
	call	turnpathon
	mov	manspath,5
	mov	finaldest,5
	call	findxyfrompath
	mov	resetmanxy,1
	call	switchryanoff
	ret

	endp







Usechurchgate	proc	near

	cmp	withobject,255
	jnz	gatewith
	call	withwhat
	ret
gatewith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"C"
	mov	ch,"U"
	mov	dl,"T"
	mov	dh,"T"
	call	compare
	jz	cutgate
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret	
	
cutgate:	call	showfirstuse
	mov	watchingtime,64*2
	mov	reeltowatch,4
	mov	endwatchreel,70
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	inc	progresspoints
	mov	al,3
	call	turnpathon
	cmp	aidedead,0
	jz	notopenchurch
	mov	al,2
	call	turnpathon
notopenchurch:	ret

	endp





Usegun	proc	near

	cmp	objecttype,4
	jz	istakengun
	call	showseconduse
	call	putbackobstuff
	ret
istakengun:	cmp	reallocation,22
	jnz	notinpoolroom
	mov	cx,300
	mov	al,34
	call	showpuztext
	mov	lastweapon,1
	mov	combatcount,39
	mov	getback,1
	inc	progresspoints
	ret
notinpoolroom:	cmp	reallocation,25
	jnz	nothelicopter
	mov	cx,300
	mov	al,34
	call	showpuztext
	mov	lastweapon,1
	mov	combatcount,19
	mov	getback,1
	mov	dreamnumber,2
	mov	roomafterdream,38
	mov	sartaindead,1
	inc	progresspoints
	ret
nothelicopter:	cmp	reallocation,27
	jnz	notinrockroom
	mov	cx,300
	mov	al,46
	call	showpuztext
	mov	pointermode,2 ;0
	mov	rockstardead,1
	mov	lastweapon,1
	mov	newsitem,1
	mov	getback,1
	mov	roomafterdream,32 ; skip
	mov	dreamnumber,0
	inc	progresspoints
	ret
notinrockroom:	cmp	reallocation,8
	jnz	notbystudio
	cmp	mapx,22
	jnz	notbystudio
	cmp	mapy,40
	jnz	notbystudio
	mov	al,92
	call	issetobonmap
	jz	notbystudio
	cmp	manspath,9
	jz	notbystudio
	mov	destination,9
	mov	finaldest,9
	call	autosetwalk
	mov	lastweapon,1
	mov	getback,1
	inc	progresspoints
	ret
notbystudio:	cmp	reallocation,6
	jnz	notsarters
	cmp	mapx,11
	jnz	notsarters
	cmp	mapy,20
	jnz	notsarters
	mov	al,5
	call	issetobonmap
	jnz	notsarters
	mov	destination,1
	mov	finaldest,1
	call	autosetwalk
	mov	al,5
	call	removesetobject
	mov	al,6
	call	placesetobject
	mov	al,1
	mov	ah,roomnum
	dec	ah
	call	turnanypathon
	mov	liftflag,1
	mov	watchingtime,40*2
	mov	reeltowatch,4
	mov	endwatchreel,43
	mov	watchspeed,1
	mov	speedcount,1
	mov	getback,1
	inc	progresspoints
	ret
notsarters:	cmp	reallocation,29
	jnz	notaide
	mov	getback,1
	mov	al,13
	call	resetlocation
	mov	al,12
	call	setlocation
	mov	destpos,12
	mov	destination,2
	mov	finaldest,2
	call	autosetwalk
	mov	watchingtime,164*2
	mov	reeltowatch,3
	mov	endwatchreel,164
	mov	watchspeed,1
	mov	speedcount,1
	mov	aidedead,1
	mov	dreamnumber,3
	mov	roomafterdream,33
	inc	progresspoints
	ret
notaide:	cmp	reallocation,23
	jnz	notwithboss
	cmp	mapx,0
	jnz	notwithboss
	cmp	mapy,50
	jnz	notwithboss
	cmp	manspath,5
	jz	pathokboss
	mov	destination,5
	mov	finaldest,5
	call	autosetwalk
pathokboss:	mov	lastweapon,1
	mov	getback,1
	ret
notwithboss:	cmp	reallocation,8
	jnz	nottvsoldier
	cmp	mapx,11
	jnz	nottvsoldier
	cmp	mapy,10
	jnz	nottvsoldier
	cmp	manspath,2
	jz	pathoktv
	mov	destination,2
	mov	finaldest,2
	call	autosetwalk
pathoktv:	mov	lastweapon,1
	mov	getback,1
	ret
nottvsoldier:	call	showfirstuse
	call	putbackobstuff
	ret

	endp







Useshield	proc	near

	cmp	reallocation,20
	jnz	notinsartroom
	cmp	combatcount,0
	jz	notinsartroom
	mov	lastweapon,3
	call	showseconduse
	mov	getback,1
	inc	progresspoints
	call	removeobfrominv
	ret
notinsartroom:	call	showfirstuse
	call	putbackobstuff
	ret

	endp






Usebuttona	proc	near

	mov	al,95
	call	issetobonmap
	jz	donethisbit

	call	showfirstuse
	mov	al,0
	mov	ah,roomnum
	dec	ah
	call	turnanypathon
	mov	al,9
	call	removesetobject
	mov	al,95
	call	placesetobject

	mov	watchingtime,15*2
	mov	reeltowatch,71
	mov	endwatchreel,85
	mov	watchspeed,1
	mov	speedcount,1

	mov	getback,1
	inc	progresspoints
	ret
donethisbit:	call	showseconduse
	call	putbackobstuff
	ret

	endp




Useplate	proc	near

	cmp	withobject,255
	jnz	platewith
	call	withwhat
	ret
platewith:	mov	al,withobject
	mov	ah,withtype
	mov	cl,"S"
	mov	ch,"C"
	mov	dl,"R"
	mov	dh,"W"
	call	compare
	jz	unscrewplate
	mov	al,withobject
	mov	ah,withtype
	mov	cl,"K"
	mov	ch,"N"
	mov	dl,"F"
	mov	dh,"E"
	call	compare
	jz	triedknife
	mov	cx,300
	mov	al,14
	call	showpuztext
	call	putbackobstuff
	ret

unscrewplate:	mov	al,20
	call	playchannel1
	call	showfirstuse
	mov	al,28
	call	placesetobject
	mov	al,24
	call	placesetobject
	mov	al,25
	call	removesetobject
	mov	al,0
	call	placefreeobject
	inc	progresspoints
	mov	getback,1
	ret

triedknife:	mov	cx,300
	mov	al,54
	call	showpuztext
	call	putbackobstuff
	ret

	endp





Usewinch	proc	near

	mov	al,40
	mov	ah,1
	call	checkinside
	cmp	cl,numexobjects
	jz	nowinch
	mov	al,cl
	mov	ah,4
	mov	cl,"F"
	mov	ch,"U"
	mov	dl,"S"
	mov	dh,"E"
	call	compare
	jnz	nowinch

	mov	watchingtime,217*2
	mov	reeltowatch,0
	mov	endwatchreel,217
	mov	watchspeed,1
	mov	speedcount,1
	mov	destpos,1
	mov	newlocation,45
	mov	dreamnumber,1
	mov	roomafterdream,44
	mov	generaldead,1
	mov	newsitem,2
	mov	getback,1
	inc	progresspoints
	ret

nowinch:	call	showfirstuse
   	call	putbackobstuff
	ret

	endp
