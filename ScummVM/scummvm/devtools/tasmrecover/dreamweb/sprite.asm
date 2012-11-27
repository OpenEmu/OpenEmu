;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text
;------------------------------------------------------------People Routines----



Reelroutines	db	1,44,0 	;Room number and x,y
	dw	20	;reel pointer
	db	2,0,1 	;speed,speed count,convers. no.

	db	1,55,0
	dw	0
	db 	50,20,0

	db	24,22,0
	dw	74
	db	1,0,0

	db	24,33,10
	dw	75
	db	1,0,1

	db	1,44,0
	dw	27
	db	2,0,2

	db	1,44,0
	dw	96
	db	3,0,4

	db	1,44,0
	dw	118
	db	2,0,5

	db	1,44,10
	dw	0
	db	2,0,0

	db	5,22,20
	dw	53
	db	3,0,0

	db	5,22,20
	dw	40
	db	1,0,2

	db	5,22,20
	dw	50
	db	1,0,3

	db	2,11,10
	dw	192
	db	1,0,0

	db	2,11,10
	dw	182
	db	2,0,1

	db	8,11,10
	dw	0
	db	2,0,1

	db	23,0,50
	dw	0
	db	3,0,0
	
	db	28,11,20
	dw	250
	db	4,0,0

	db	23,0,50
	dw	43
	db	2,0,8

	db	23,11,40
	dw	130
	db	2,0,1

	db	23,22,40
	dw	122
	db	2,0,2

	db	23,22,40
	dw	105
	db	2,0,3

	db	23,22,40
	dw	81
	db	2,0,4

	db	23,11,40
	dw	135
	db	2,0,5

	db	23,22,40
	dw	145
	db	2,0,6

	db	4,22,30
	dw	0
	db	2,0,0

	db	45,22,30
	dw	200
	db	0,0,20

	db	45,22,30
	dw	39
	db	2,0,0

	db	45,22,30
	dw	25
	db	2,0,0

	db	8,22,40
	dw	32
	db	2,0,0

	db	7,11,20
	dw	64
	db	2,0,0

	db	22,22,20
	dw	82
	db	2,0,0

	db	27,11,30
	dw	0
	db	2,0,0

	db	20,0,30
	dw	0
	db	2,0,0

	db	14,33,40
	dw	21
	db	1,0,0

	db	29,11,10
	dw	0
	db	1,0,0

	db	2,22,0
	dw	2
	db	2,0,0

	db	25,0,50
	dw	4
	db	2,0,0

	db	50,22,30
	dw	121
	db	2,0,0

	db	50,22,30
	dw	0
	db	20,0,0

	db	52,22,30
	dw	192
	db	2,0,0

	db	52,22,30
	dw	233
	db	2,0,0

	db	50,22,40
	dw	104
	if	cd
	if	german
	db	65,0,0
	else
	db	55,0,0
	endif
	else
	db	55,0,0
	endif

	db	53,33,0
	dw	99
	db	2,0,0

	db	50,22,40
	dw	0
	db	3,0,0

	db	50,22,30
	dw	162
	db	2,0,0

	db	52,22,30
	dw	57
	db	2,0,0

	db	52,22,30
	dw	0
	db	2,0,0

	db	54,0,0
	dw	72
	db	3,0,0

	db	55,44,0
	dw	0
	db	2,0,0

	db	19,0,0
	dw	0
	db	28,0,0

	db	14,22,0
	dw	2
	db	2,0,0

	db	14,22,0
	dw	300
	db	1,0,0

	db	10,22,30
	dw	174
	db	0,0,0

	db	12,22,20
	dw	0
	db	1,0,0

	db	11,11,20
	dw	0
	db 	50,20,0
	
	db	11,11,30
	dw	0
	db 	50,20,0
	
	db	11,22,20
	dw	0
	db 	50,20,0

	db	14,33,40
	dw	0
	db 	50,20,0

	db	255


Lenofreelrouts	equ 457 ; $-reelroutines


Reelcalls	dw	gamer,sparkydrip,eden,edeninbath,sparky,smokebloke
	dw	manasleep,drunk,receptionist,malefan,femalefan
	dw	louis,louischair,soldier1,bossman,interviewer
	dw	heavy,manasleep2,mansatstill,drinker,bartender
	dw	othersmoker,tattooman,attendant,keeper,candles1
	dw	smallcandle,security,copper,poolguard,rockstar
	dw	businessman,train,aide,mugger,helicopter
	dw	intromagic1,intromusic,intromagic2,candles2,gates
	dw	intromagic3,intromonks1,candles,intromonks2
	dw	handclap,monkandryan,endgameseq,priest,madman
	dw	madmanstelly,alleybarksound,foghornsound
	dw	carparkdrip,carparkdrip,carparkdrip,carparkdrip



;---------------------------------------------------------Character updates----



Alleybarksound	proc	near

	mov	ax,[es:bx+3]
	dec	ax
	cmp	ax,0
	jnz	nobark
	push	bx es
    	mov	al,14
	call	playchannel1
	pop	es bx
	mov	ax,1000
nobark:	mov	[es:bx+3],ax
	ret

	endp




Intromusic	proc	near

	ret

	endp


Foghornsound	proc	near

	call	randomnumber
	cmp	al,198
	jnz	nofog
     	mov	al,13
	call	playchannel1
nofog:	ret

	endp




Receptionist	proc	near

	call	checkspeed
	jnz	gotrecep
	cmp	cardpassflag,1
	jnz	notsetcard
           	inc	cardpassflag
	mov	byte ptr [es:bx+7],1
	mov	word ptr [es:bx+3],64
notsetcard:	cmp	word ptr [es:bx+3],58
	jnz	notdes1
	call	randomnumber
	cmp	al,30
	jc	notdes2
	mov	word ptr [es:bx+3],55
	jmp	gotrecep

notdes1:	cmp	word ptr [es:bx+3],60
	jnz	notdes2
	call	randomnumber
	cmp	al,240
	jc	gotrecep
	mov	word ptr [es:bx+3],53
	jmp	gotrecep

notdes2:	cmp	word ptr [es:bx+3],88
	jnz	notendcard
	mov	word ptr [es:bx+3],53
	jmp	gotrecep

notendcard:	inc	word ptr [es:bx+3]
gotrecep:	call	showgamereel
	call	addtopeoplelist

	mov	al,[es:bx+7]
	and	al,128
	jz	nottalkedrecep
	mov	talkedtorecep,1
nottalkedrecep:	ret
	
	endp




Smokebloke	proc	near

	cmp	rockstardead,0
	jnz	notspokento
	mov	al,[es:bx+7]
	and	al,128
	jz	notspokento
	push	es bx
	mov	al,5
	call	setlocation
	pop	bx es
notspokento:	;mov	al,[es:bx+7]
	;and	al,127
	;mov	[es:bx+7],al
	call	checkspeed
	jnz	gotsmokeb
	cmp	word ptr [es:bx+3],100
	jnz	notsmokeb1
	call	randomnumber
	cmp	al,30
	jc	notsmokeb2
	mov	word ptr [es:bx+3],96
	jmp	gotsmokeb

notsmokeb1:	cmp	word ptr [es:bx+3],117
	jnz	notsmokeb2
	mov	word ptr [es:bx+3],96
	jmp	gotsmokeb

notsmokeb2:	inc	word ptr [es:bx+3]
gotsmokeb:	call	showgamereel
	call	addtopeoplelist
	ret

	endp










Attendant	proc	near

	call	showgamereel
	call	addtopeoplelist
	mov	al,[es:bx+7]
	and	al,128
	jz	nottalked
          	mov	talkedtoattendant,1
nottalked:	ret

	endp






Manasleep	proc	near

	mov	al,[es:bx+7]
	and	al,127
	mov	[es:bx+7],al
	call	showgamereel
	call	addtopeoplelist
	ret

	endp



Eden	proc	near

	cmp	generaldead,0
	jnz	notinbed
	call	showgamereel
	call	addtopeoplelist
notinbed:	ret

	endp



Edeninbath	proc	near

	cmp	generaldead,0
	jz	notinbath
	cmp	sartaindead,0
	jnz	notinbath
	call	showgamereel
	call	addtopeoplelist
notinbath:	ret

	endp



Malefan	proc	near

	call	showgamereel
	call	addtopeoplelist
	ret

	endp



Femalefan	proc	near

	call	showgamereel
	call	addtopeoplelist
	ret

	endp




Louis	proc	near

	cmp	rockstardead,0
	jnz	notlouis1
	call	showgamereel
	call	addtopeoplelist
notlouis1:	ret

	endp




Louischair	proc	near
          
	cmp	rockstardead,0
	jz	notlouis2
	call	checkspeed
	jnz	notlouisanim
	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,191
	jz	restartlouis
	cmp	ax,185
	jz	randomlouis
	mov	[es:bx+3],ax
	jmp	notlouisanim
randomlouis:	mov	[es:bx+3],ax
	call	randomnumber
	cmp	al,245
	jnc	notlouisanim
restartlouis:	mov	ax,182
	mov	[es:bx+3],ax
notlouisanim:	call	showgamereel
	call	addtopeoplelist
notlouis2:	ret

	endp



Manasleep2	proc	near

	mov	al,[es:bx+7]
	and	al,127
	mov	[es:bx+7],al
	call	showgamereel
	call	addtopeoplelist
	ret

	endp




Mansatstill	proc	near

	call	showgamereel
	call	addtopeoplelist
	ret

	endp


Tattooman	proc	near

	call	showgamereel
	call	addtopeoplelist
	ret

	endp


Drinker	proc	near

	call	checkspeed
	jnz	gotdrinker
	inc	word ptr [es:bx+3]
	cmp	word ptr [es:bx+3],115
	jnz	notdrinker1
	mov	word ptr [es:bx+3],105
	jmp	gotdrinker

notdrinker1:	cmp	word ptr [es:bx+3],106
	jnz	gotdrinker
	call	randomnumber
	cmp	al,3
	jc	gotdrinker
	mov	word ptr [es:bx+3],105

gotdrinker:	call	showgamereel
	call	addtopeoplelist
	ret

	endp





Bartender	proc	near

	call	checkspeed
	jnz	gotsmoket
	cmp	word ptr [es:bx+3],86
	jnz	notsmoket1
	call	randomnumber
	cmp	al,18
	jc	notsmoket2
	mov	word ptr [es:bx+3],81
	jmp	gotsmoket

notsmoket1:	cmp	word ptr [es:bx+3],103
	jnz	notsmoket2
	mov	word ptr [es:bx+3],81
	jmp	gotsmoket

notsmoket2:	inc	word ptr [es:bx+3]
gotsmoket:	call	showgamereel
	cmp	gunpassflag,1
	jnz	notgotgun
	mov	byte ptr [es:bx+7],9
notgotgun:	call	addtopeoplelist
	ret

	endp







Othersmoker	proc	near

	call	showgamereel
	call	addtopeoplelist
	ret

	endp






Barwoman	proc	near

	call	showgamereel
	call	addtopeoplelist
	ret

	endp









Interviewer	proc	near
	
	cmp	reeltowatch,68
	jnz	notgeneralstart
	inc	word ptr [es:bx+3]
notgeneralstart:	cmp	word ptr [es:bx+3],250
	jz	talking
	call	checkspeed
	jnz	talking
	cmp	word ptr [es:bx+3],259
	jz	talking
	inc	word ptr [es:bx+3]
talking:	call	showgamereel
	ret

	endp





Soldier1	proc	near

	cmp	word ptr [es:bx+3],0
	jz	soldierwait
	mov	watchingtime,10
	cmp	word ptr [es:bx+3],30
	jnz	notaftersshot
	inc	combatcount
	cmp	combatcount,40
	jnz	gotsoldframe
	mov	mandead,2
	jmp	gotsoldframe
notaftersshot:	call	checkspeed
	jnz	gotsoldframe	
	inc	word ptr [es:bx+3]
	jmp	gotsoldframe
soldierwait:	cmp	lastweapon,1
	jnz	gotsoldframe
	mov	watchingtime,10
	cmp	manspath,2
	jnz	gotsoldframe
	cmp	facing,4
	jnz	gotsoldframe
	inc	word ptr [es:bx+3]
	mov	lastweapon,-1
	mov	combatcount,0
gotsoldframe:	call	showgamereel
	call	addtopeoplelist
	ret

	endp








Rockstar	proc	near

	mov	ax,[es:bx+3]
	cmp	ax,303
	jz	rockcombatend
	cmp	ax,118
	jz	rockcombatend
	call	checkspeed
	jnz	rockspeed

	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,118
	jnz	notbeforedead
	mov	mandead,2
	jmp	gotrockframe

notbeforedead:	cmp	ax,79
	jnz	gotrockframe
	dec	ax
	cmp	lastweapon,1
	jnz	notgunonrock
	mov	lastweapon,-1
	mov	ax,123
	jmp	gotrockframe
notgunonrock:	inc	combatcount
	cmp	combatcount,40
	jnz	gotrockframe
	mov	combatcount,0
	mov	ax,79

gotrockframe:	mov	[es:bx+3],ax
rockspeed:	call	showgamereel
	cmp	word ptr [es:bx+3],78
	jnz	notalkrock
	call	addtopeoplelist
	mov	pointermode,2
	mov	watchingtime,0
	ret
	
notalkrock:	mov	watchingtime,2
	mov	pointermode,0
	mov	al,mapy
	mov	[es:bx+2],al
	ret

rockcombatend:	mov	newlocation,45
	call	showgamereel
	ret

	endp









	





Helicopter	proc	near

	mov	ax,[es:bx+3]
	cmp	ax,203
	jz	heliwon
	;cmp	ax,53
	;jz	helicombatend
	call	checkspeed
	jnz	helispeed

	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,53
	jnz	notbeforehdead
	inc	combatcount
	cmp	combatcount,8
	jc	waitabit
	mov	mandead,2
waitabit:	mov	ax,49
	jmp	gotheliframe

notbeforehdead:	cmp	ax,9
	jnz	gotheliframe
	dec	ax
	cmp	lastweapon,1
	jnz	notgunonheli
	mov	lastweapon,-1
	mov	ax,55
	jmp	gotheliframe
notgunonheli:	mov	ax,5
	inc	combatcount
	cmp	combatcount,20
	jnz	gotheliframe
	mov	combatcount,0
	mov	ax,9

gotheliframe:	mov	[es:bx+3],ax
helispeed:	call	showgamereel
	mov	al,mapx
	mov	[es:bx+1],al
helicombatend:	mov	ax,[es:bx+3]
	cmp	ax,9 ;8
	jnc	notwaitingheli
	cmp	combatcount,7
	jc	notwaitingheli
	mov	pointermode,2
	mov	watchingtime,0
	ret
notwaitingheli:	mov	pointermode,0
	mov	watchingtime,2
	ret

heliwon:	mov	pointermode,0
	ret

	endp


Mugger	proc	near

	mov	ax,[es:bx+3]
	cmp	ax,138
	jz	endmugger1
	cmp	ax,176
	jz	endmugger2
	cmp	ax,2
	jnz	havesetwatch
	mov	watchingtime,175*2
havesetwatch:	call	checkspeed
	jnz	notmugger
	inc	word ptr [es:bx+3]
notmugger:	call	showgamereel
	mov	al,mapx
	mov	[es:bx+1],al
	ret

endmugger1:	push	es bx
	call	createpanel2
	call	showicon
	mov	al,41
	call	findpuztext
	mov	di,33+20
	mov	bx,104
	mov	dl,241
	mov	ah,0
	call	printdirect
	call	worktoscreen
	mov	cx,300
	call	hangon
	pop	bx es
	push	es bx
	mov	word ptr [es:bx+3],140
	mov	manspath,2
	mov	finaldest,2
	call	findxyfrompath
	mov	resetmanxy,1
	mov	al,"W"
	mov	ah,"E"
	mov	cl,"T"
	mov	ch,"A"	
	call	findexobject
	mov	command,al
	mov	objecttype,4
	call	removeobfrominv
	mov	al,"W"
	mov	ah,"E"
	mov	cl,"T"
	mov	ch,"B"	
	call	findexobject
	mov	command,al
	mov	objecttype,4
	call	removeobfrominv
	call	makemainscreen
	mov	al,48
	mov	bl,68-32
	mov	bh,54+64
	mov	cx,70	; time on screen
	mov	dx,10	; pause before show
	call	setuptimeduse
	mov	beenmugged,1
	pop	bx es
	ret

endmugger2:	ret
	

	endp








Aide	proc	near

	call	showgamereel
	call	addtopeoplelist
	ret

	endp






Businessman	proc	near

	mov	pointermode,0
	mov	watchingtime,2
	mov	ax,[es:bx+3]
	cmp	ax,2
	jnz	notfirstbiz
	push	ax bx es
	mov	al,49
	mov	cx,30
	mov	dx,1
	mov	bl,68
	mov	bh,174
	call	setuptimeduse
            	pop	es bx ax

notfirstbiz:	cmp	ax,95
	jz	buscombatwonend
	cmp	ax,49
	jz	buscombatend

	call	checkspeed
	jnz	busspeed

	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,48
	jnz	notbeforedeadb
	mov	mandead,2
	jmp	gotbusframe

notbeforedeadb:	cmp	ax,15
	jnz	buscombatwon
	dec	ax
	cmp	lastweapon,3
	jnz	notshieldonbus
	mov	lastweapon,-1
	mov	combatcount,0
	mov	ax,51
	jmp	gotbusframe
notshieldonbus:	inc	combatcount
	cmp	combatcount,20
	jnz	gotbusframe
	mov	combatcount,0
	mov	ax,15
	jmp	gotbusframe

buscombatwon:	cmp	ax,91
	jnz	gotbusframe
	push	bx es
	mov	al,0
	call	turnpathon
	mov	al,1
	call	turnpathon
	mov	al,2
	call	turnpathon
	mov	al,3
	call	turnpathoff
	mov	manspath,5
	mov	finaldest,5
	call	findxyfrompath
	mov	resetmanxy,1
	pop	es bx
	mov	ax,92
	jmp	gotbusframe

gotbusframe:	mov	[es:bx+3],ax
busspeed:	call	showgamereel
	mov	al,mapy
	mov	[es:bx+2],al
	mov	ax,[es:bx+3]
	cmp	ax,14
	jnz	buscombatend
	mov	watchingtime,0
	mov	pointermode,2
	ret

buscombatend:	ret

buscombatwonend:	mov	pointermode,0
	mov	watchingtime,0
	ret

	endp






Poolguard	proc	near

	mov	ax,[es:bx+3]
	cmp	ax,214
	jz	combatover2
	cmp	ax,258
	jz	combatover2
	cmp	ax,185
	jz	combatover1
	cmp	ax,0
	jnz	notfirstpool
	mov	al,0
	call	turnpathon
notfirstpool:	call	checkspeed
	jnz	guardspeed

	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,122
	jnz	notendguard1
	dec	ax
	cmp	lastweapon,2
	jnz	notaxeonpool
	mov	lastweapon,-1
	mov	ax,122
	jmp	gotguardframe
notaxeonpool:	inc	combatcount
	cmp	combatcount,40
	jnz	gotguardframe
	mov	combatcount,0
	mov	ax,195
	jmp	gotguardframe

notendguard1:	cmp	ax,147
	jnz	gotguardframe
	dec	ax
	cmp	lastweapon,1
	jnz	notgunonpool
	mov	lastweapon,-1
	mov	ax,147
	jmp	gotguardframe
notgunonpool:	inc	combatcount
	cmp	combatcount,40
	jnz	gotguardframe
	mov	combatcount,0
	mov	ax,220

gotguardframe:	mov	[es:bx+3],ax
guardspeed:	call	showgamereel
	mov	ax,[es:bx+3]
	cmp	ax,121
	jz	iswaitingpool
	cmp	ax,146
	jz	iswaitingpool
	mov	pointermode,0
	mov	watchingtime,2
	ret
iswaitingpool:	mov	pointermode,2
	mov	watchingtime,0	
	ret

combatover1:	mov	watchingtime,0
	mov	pointermode,0
	mov	al,0
	call	turnpathon
	mov	al,1
	call	turnpathoff
	ret

combatover2:	call	showgamereel
	mov	watchingtime,2
	mov	pointermode,0
	inc	combatcount
	cmp	combatcount,100
	jc	doneover2
	mov	watchingtime,0
	mov	mandead,2
doneover2:	ret

	endp










Security	proc	near
	
	cmp	word ptr [es:bx+3],32
	jz	securwait
	cmp	word ptr [es:bx+3],69
	jnz	notaftersec
	ret
notaftersec:	mov	watchingtime,10
	call	checkspeed
	jnz	gotsecurframe	
	inc	word ptr [es:bx+3]
	jmp	gotsecurframe
securwait:	cmp	lastweapon,1
	jnz	gotsecurframe
	mov	watchingtime,10
	cmp	manspath,9
	jnz	gotsecurframe
	cmp	facing,0
	jnz	gotsecurframe
	mov	lastweapon,-1
	inc	word ptr [es:bx+3]
gotsecurframe:	call	showgamereel
	call	addtopeoplelist
	ret

	endp






Heavy	proc	near

	mov	al,[es:bx+7]
	and	al,127
	mov	[es:bx+7],al
	cmp	word ptr [es:bx+3],43
	jz	heavywait
	mov	watchingtime,10
	cmp	word ptr [es:bx+3],70
	jnz	notafterhshot
	inc	combatcount
	cmp	combatcount,80
	jnz	gotheavyframe
	mov	mandead,2
	jmp	gotheavyframe
notafterhshot:	call	checkspeed
	jnz	gotheavyframe	
	inc	word ptr [es:bx+3]
	jmp	gotheavyframe
heavywait:	cmp	lastweapon,1
	jnz	gotheavyframe
	cmp	manspath,5
	jnz	gotheavyframe
	cmp	facing,4
	jnz	gotheavyframe
	mov	lastweapon,-1
	inc	word ptr [es:bx+3]
	mov	combatcount,0
gotheavyframe:	call	showgamereel
	call	addtopeoplelist
	ret

	endp




Bossman	proc	near

	call	checkspeed
	jnz	notboss
	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,4
	jz	firstdes
	cmp	ax,20
	jz	secdes
	cmp	ax,41
	jnz	gotallboss
	mov	ax,0
	inc	gunpassflag
	mov	byte ptr [es:bx+7],10
	jmp	gotallboss
firstdes:	cmp	gunpassflag,1
	jz	gotallboss
	push	ax
	call	randomnumber
	mov	cl,al
	pop	ax
	cmp	cl,10
	jc	gotallboss
	mov	ax,0
	jmp	gotallboss
secdes:	cmp	gunpassflag,1
	jz	gotallboss
	mov	ax,0
gotallboss:	mov	[es:bx+3],ax
notboss:	call	showgamereel
	call	addtopeoplelist

	mov	al,[es:bx+7]
	and	al,128
	jz	nottalkedboss
	mov	talkedtoboss,1
nottalkedboss:	ret
	
	endp





Gamer	proc	near

	call	checkspeed
	jnz	gamerfin
gameragain:	call	randomnum1
	and	al,7
	cmp	al,5
	jnc	gameragain
	add	al,20
	cmp	al,[es:bx+3]
	jz	gameragain
	mov	ah,0
	mov	[es:bx+3],ax
gamerfin:	call	showgamereel
	call	addtopeoplelist
	ret

	endp





Sparkydrip	proc	near

	call	checkspeed
	jnz	cantdrip
         	mov	al,14
	mov	ah,0
	call	playchannel0
cantdrip:	ret

	endp



Carparkdrip	proc	near

	call	checkspeed
	jnz	cantdrip2
         	mov	al,14
	call	playchannel1
cantdrip2:	ret

	endp



Keeper	proc	near

	cmp	keeperflag,0
	jnz	notwaiting
	cmp	reeltowatch,190
	jc	waiting
	inc	keeperflag
	mov	ah,[es:bx+7]
	and	ah,127
	cmp	ah,dreamnumber
	jz	notdiff
	mov	al,dreamnumber
	mov	[es:bx+7],al
notdiff:	ret
notwaiting:	call	addtopeoplelist
	call	showgamereel
waiting:	ret

	endp



Candles1	proc	near

	call	checkspeed
	jnz	candle1
        	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,44
	jnz	notendcandle1
	mov	ax,39
notendcandle1:	mov	[es:bx+3],ax
candle1:	call	showgamereel
	ret

	endp



Smallcandle	proc	near

	call	checkspeed
	jnz	smallcandlef
        	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,37
	jnz	notendsmallcandle
	mov	ax,25
notendsmallcandle:	mov	[es:bx+3],ax
smallcandlef:	call	showgamereel
	ret

	endp











Intromagic1	proc	near

	call	checkspeed
	jnz	introm1fin
	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,145
	jnz	gotintrom1
	mov	ax,121
gotintrom1:	mov	[es:bx+3],ax
	cmp	ax,121
	jnz	introm1fin
	inc	introcount
	push	es bx
	call	intro1text
	pop	bx es
	cmp	introcount,8 ; was 7
	jnz	introm1fin
	add	mapy,10
	mov	nowinnewroom,1
introm1fin:	call	showgamereel
	ret

	endp




Candles	proc	near

	call	checkspeed
	jnz	candlesfin
	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,167
	jnz	gotcandles
	mov	ax,162
gotcandles:	mov	[es:bx+3],ax
candlesfin:	call	showgamereel
	ret

	endp



Candles2	proc	near

	call	checkspeed
	jnz	candles2fin
	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,238
	jnz	gotcandles2
	mov	ax,233
gotcandles2:	mov	[es:bx+3],ax
candles2fin:	call	showgamereel
	ret

	endp



Gates	proc	near

	call	checkspeed
	jnz	gatesfin
	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,116
	jnz	notbang
	push	ax bx es
	mov	al,17 ;12
	call	playchannel1
	pop	es bx ax
notbang:	cmp	ax,110
	jc	slowgates
	mov	byte ptr [es:bx+5],2
slowgates:	cmp	ax,120
	jnz	gotgates
	mov	getback,1
	mov	ax,119
gotgates:	mov	[es:bx+3],ax
	push	es bx
	call	intro3text
	pop	bx es
gatesfin:	call	showgamereel
	ret

	endp




Intromagic2	proc	near

	call	checkspeed
	jnz	introm2fin
	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,216
	jnz	gotintrom2
	mov	ax,192
gotintrom2:	mov	[es:bx+3],ax
introm2fin:	call	showgamereel
	ret

	endp




Intromagic3	proc	near

	call	checkspeed
	jnz	introm3fin
	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,218
	jnz	gotintrom3
	mov	getback,1
gotintrom3:	mov	[es:bx+3],ax
introm3fin:	call	showgamereel
	mov	al,mapx
	mov	[es:bx+1],al
	ret

	endp







Intromonks1	proc	near

	call	checkspeed
	jnz	intromonk1fin
	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,80
	jnz	notendmonk1
	add	mapy,10
	mov	nowinnewroom,1
	call	showgamereel
	ret
notendmonk1:	cmp	ax,30
	jnz	gotintromonk1
	sub	mapy,10
	mov	nowinnewroom,1
	mov	ax,51
gotintromonk1:	mov	[es:bx+3],ax
	cmp	ax,5
	jz	waitstep
	cmp	ax,15
	jz	waitstep
	cmp	ax,25
	jz	waitstep
	cmp	ax,61
	jz	waitstep
	cmp	ax,71
	jz	waitstep
	jmp	intromonk1fin
waitstep:	push	es bx
	call	intro2text
	pop	bx es
	mov	byte ptr [es:bx+6],-20
intromonk1fin:	call	showgamereel
	mov	al,mapy
	mov	[es:bx+2],al
	ret

	endp




Intromonks2	proc	near

	call	checkspeed
	jnz	intromonk2fin
	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,87
	jnz	nottalk1
	inc	introcount
	push	es bx
	call	monks2text
	pop	bx es
	cmp	introcount,19
	jnz	notlasttalk1
	mov	ax,87
	jmp	gotintromonk2
notlasttalk1:	mov	ax,74
	jmp	gotintromonk2

nottalk1:	cmp	ax,110
	jnz	notraisearm
	inc	introcount
	push	es bx
	call	monks2text
	pop	bx es
	if	cd
	if	german
	cmp	introcount,42
	else
	cmp	introcount,35
	endif
	else
	cmp	introcount,35
	endif
	jnz	notlastraise
	mov	ax,111
	jmp	gotintromonk2
notlastraise:	mov	ax,98
	jmp	gotintromonk2

notraisearm:	cmp	ax,176
	jnz	notendmonk2
	mov	getback,1
	jmp	gotintromonk2
notendmonk2:	cmp	ax,125
	jnz	gotintromonk2
	mov	ax,140
gotintromonk2:	mov	[es:bx+3],ax
intromonk2fin:	call	showgamereel
	ret

	endp





Handclap	proc	near

	ret

	endp




	if	german
	if	cd

Monks2text	proc	near

	cmp	introcount,1
	jnz	notmonk2text1
	mov	al,8
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text1:	cmp	introcount,5
	jnz	notmonk2text2
	mov	al,9
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text2:	cmp	introcount,9
	jnz	notmonk2text3
	mov	al,10
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text3:	cmp	introcount,13
	jnz	notmonk2text4
	mov	introcount,14
	mov	al,11
	mov	bl,0
	mov	bh,105
	mov	cx,100
	jmp	gotmonks2text
notmonk2text4:	cmp	introcount,19
	jnz	notmonk2text7
	mov	al,14
	mov	bl,36
	mov	bh,160
	mov	cx,100 ;32
	mov	dx,1
	mov	ah,82
	jmp	setuptimedtemp
notmonk2text7:	cmp	introcount,23
	jnz	notmonk2text8
	mov	al,15
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text8:	cmp	introcount,27
	jnz	notmonk2text9
	mov	al,16
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text9:	cmp	introcount,30
	jnz	notmonk2text10
	mov	al,17
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text10:	cmp	introcount,35
	jnz	notmonk2text11
	mov	al,18
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text11:	ret

gotmonks2text:	mov	dx,1
	mov	cx,120
	mov	ah,82
	call	setuptimedtemp
	ret

	endp

	else

Monks2text	proc	near

	cmp	introcount,1
	jnz	notmonk2text1
	mov	al,8
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text1:	cmp	introcount,4
	jnz	notmonk2text2
	mov	al,9
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text2:	cmp	introcount,7
	jnz	notmonk2text3
	mov	al,10
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text3:	cmp	introcount,10
	jnz	notmonk2text4
	if	cd
	mov	introcount,12
	endif
	mov	al,11
	mov	bl,0
	mov	bh,105
	mov	cx,100
	jmp	gotmonks2text
notmonk2text4:	cmp	introcount,13
	jnz	notmonk2text5
	if	cd
	mov	introcount,17; 18
	ret
	endif
	mov	al,12
	mov	bl,0
	mov	bh,120
	mov	cx,100
	jmp	gotmonks2text
notmonk2text5:	cmp	introcount,16
	jnz	notmonk2text6
	mov	al,13
	mov	bl,0
	mov	bh,135
	mov	cx,100
	jmp	gotmonks2text
notmonk2text6:	cmp	introcount,19
	jnz	notmonk2text7
	mov	al,14
	mov	bl,36
	mov	bh,160
	mov	cx,100 ;32
	mov	dx,1
	mov	ah,82
	jmp	setuptimedtemp
notmonk2text7:	cmp	introcount,22
	jnz	notmonk2text8
	mov	al,15
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text8:	cmp	introcount,25
	jnz	notmonk2text9
	mov	al,16
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text9:	if	cd
	cmp	introcount,27
	else
	cmp	introcount,28
	endif
	jnz	notmonk2text10
	mov	al,17
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text10:	cmp	introcount,31
	jnz	notmonk2text11
	mov	al,18
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text11:	ret

gotmonks2text:	mov	dx,1
	mov	cx,120
	mov	ah,82
	call	setuptimedtemp
	ret

	endp

	
	endif
	else

Monks2text	proc	near

	cmp	introcount,1
	jnz	notmonk2text1
	mov	al,8
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text1:	cmp	introcount,4
	jnz	notmonk2text2
	mov	al,9
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text2:	cmp	introcount,7
	jnz	notmonk2text3
	mov	al,10
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text3:	cmp	introcount,10
	jnz	notmonk2text4
	if	cd
	mov	introcount,12
	endif
	mov	al,11
	mov	bl,0
	mov	bh,105
	mov	cx,100
	jmp	gotmonks2text
notmonk2text4:	cmp	introcount,13
	jnz	notmonk2text5
	if	cd
	mov	introcount,17; 18
	ret
	endif
	mov	al,12
	mov	bl,0
	mov	bh,120
	mov	cx,100
	jmp	gotmonks2text
notmonk2text5:	cmp	introcount,16
	jnz	notmonk2text6
	mov	al,13
	mov	bl,0
	mov	bh,135
	mov	cx,100
	jmp	gotmonks2text
notmonk2text6:	cmp	introcount,19
	jnz	notmonk2text7
	mov	al,14
	mov	bl,36
	mov	bh,160
	mov	cx,100 ;32
	mov	dx,1
	mov	ah,82
	jmp	setuptimedtemp
notmonk2text7:	cmp	introcount,22
	jnz	notmonk2text8
	mov	al,15
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text8:	cmp	introcount,25
	jnz	notmonk2text9
	mov	al,16
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text9:	if	cd
	cmp	introcount,27
	else
	cmp	introcount,28
	endif
	jnz	notmonk2text10
	mov	al,17
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text10:	cmp	introcount,31
	jnz	notmonk2text11
	mov	al,18
	mov	bl,36
	mov	bh,160
	mov	cx,100
	jmp	gotmonks2text
notmonk2text11:	ret

gotmonks2text:	mov	dx,1
	mov	cx,120
	mov	ah,82
	call	setuptimedtemp
	ret

	endp

	endif






Intro1text	proc	near

	cmp	introcount,2
	jnz	notintro1text1
	mov	al,40
	mov	bl,34
	mov	bh,130
	mov	cx,90
	jmp	gotintro1text
notintro1text1:	cmp	introcount,4
	jnz	notintro1text2
	mov	al,41
	mov	bl,34
	mov	bh,130
	mov	cx,90
	jmp	gotintro1text
notintro1text2:	cmp	introcount,6
	jnz	notintro1text3
	mov	al,42
	mov	bl,34
	mov	bh,130
	mov	cx,90
	jmp	gotintro1text
notintro1text3:	ret

gotintro1text:	mov	dx,1
	mov	ah,82
	if	cd
	cmp	ch1playing,255
	jz	oktalk2
	dec	introcount
	ret
	endif
oktalk2:	call	setuptimedtemp
	ret

	endp



Intro2text	proc	near

               	cmp	ax,5
	jnz	notintro2text1
	mov	al,43
	mov	bl,34
	mov	bh,40
	mov	cx,90
	jmp	gotintro2text
notintro2text1:	cmp	ax,15
	jnz	notintro2text2
	mov	al,44
	mov	bl,34
	mov	bh,40
	mov	cx,90
	jmp	gotintro2text
notintro2text2:	ret

gotintro2text:	mov	dx,1
	mov	ah,82
	call	setuptimedtemp
	ret

	endp






Intro3text	proc	near

	cmp	ax,107
	jnz	notintro3text1
	mov	al,45
	mov	bl,36
	mov	bh,56
	mov	cx,100
	jmp	gotintro3text
notintro3text1:	if	cd
	cmp	ax,108
	else
	cmp	ax,109
	endif
	jnz	notintro3text2
	mov	al,46
	mov	bl,36
	mov	bh,56
	mov	cx,100
	jmp	gotintro3text
notintro3text2:	ret

gotintro3text:	mov	dx,1
	mov	ah,82
	call	setuptimedtemp
	ret

	endp







Monkandryan	proc	near

	call	checkspeed
	jnz	notmonkryan
	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,83
	jnz	gotmonkryan
	inc	introcount
	push	es bx
	call	textformonk
	pop	bx es
	mov	ax,77
	cmp	introcount,57
	jnz	gotmonkryan
	mov	getback,1
	ret
gotmonkryan:	mov	[es:bx+3],ax
notmonkryan:	call	showgamereel
	ret

	endp





Endgameseq	proc	near

	call	checkspeed
	jnz	notendseq
	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,51
	jnz	gotendseq
	cmp	introcount,140
	jz	gotendseq
	inc	introcount
	push	es bx
	call	textforend
	pop	bx es
	mov	ax,50
gotendseq:	mov	[es:bx+3],ax
	cmp	ax,134
	jnz	notfadedown
	push	es bx ax
	call	fadescreendownhalf	
	pop	ax bx es
	jmp	notendseq
notfadedown:	cmp	ax,324
	jnz	notfadeend
	push	es bx ax
	call	fadescreendowns
	mov	volumeto,7
	mov	volumedirection,1
	pop	ax bx es
notfadeend:	cmp	ax,340
	jnz	notendseq
	mov	getback,1
notendseq:	call	showgamereel
	mov	al,mapy
	mov	[es:bx+2],al
            	mov	ax,[es:bx+3]
	cmp	ax,145
	jnz	notendcreds
	mov	word ptr [es:bx+3],146
	call	rollendcredits
notendcreds:	ret
	
	endp






Rollendcredits	proc	near

	mov	al,16
	mov	ah,255
	call	playchannel0
	mov	volume,7
	mov	volumeto,0
	mov	volumedirection,-1

	mov	cl,160
	mov	ch,160
	mov	di,75
	mov	bx,20
	mov	ds,mapstore
	mov	si,0
	call	multiget

	mov	es,textfile1
	mov	si,3*2
	mov	ax,[es:si]
	mov	si,ax
	add	si,textstart

	mov	cx,254
endcredits1:	push	cx

	mov	bx,10
	mov	cx,linespacing
endcredits2:	push	cx si di es bx

	call	vsync
	mov	cl,160
	mov	ch,160
	mov	di,75
	mov	bx,20
	mov	ds,mapstore
	mov	si,0
	call	multiput
	call	vsync
	pop	bx es di si
	push	si di es bx
	
	mov	cx,18
onelot:	push	cx
	mov	di,75
	mov	dx,161
	mov	ax,0
	call	printdirect
	add	bx,linespacing
	pop	cx
	loop	onelot

	call	vsync
	mov	cl,160
	mov	ch,160
	mov	di,75
	mov	bx,20
	call	multidump

	pop	bx es di si cx
	dec	bx
	loop	endcredits2
	pop	cx
looknext:	mov	al,[es:si]
	inc	si
	cmp	al,":"
	jz	gotnext
	cmp	al,0
	jz	gotnext
	jmp	looknext
gotnext:	loop	endcredits1

	mov	cx,100
	call	hangon
	call	paneltomap
	call	fadescreenuphalf
	ret

	endp






Priest	proc	near

	cmp	word ptr [es:bx+3],8
	jz	priestspoken
	mov	pointermode,0
	mov	watchingtime,2
	call	checkspeed
	jnz	priestwait
	inc	word ptr [es:bx+3]
	push	es bx
	call	priesttext
	pop	bx es
priestwait:	ret

priestspoken:	ret

	endp






Madmanstelly	proc	near

	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,307
	jnz	notendtelly
	mov	ax,300
notendtelly:	mov	[es:bx+3],ax
	call	showgamereel
	ret

	endp





Madman	proc	near

	mov	watchingtime,2
	call	checkspeed
	jnz	nomadspeed
	mov	ax,[es:bx+3]
	cmp	ax,364
	jnc	ryansded
	cmp	ax,10
	jnz	notfirstmad
	push	es bx ax
	mov	dx,offset cs:introtextname
	call	loadtemptext
	pop	ax bx es
	mov	combatcount,-1
	mov	speechcount,0
notfirstmad:	inc	ax
	cmp	ax,294
	jz	madmanspoken
	cmp	ax,66
	jnz	nomadspeak
	inc	combatcount
	push	es bx
	call	madmantext
	pop	bx es
	mov	ax,53
	if	cd
	cmp	combatcount,64
	else
	cmp	combatcount,62
	endif
	jc	nomadspeak
	if	cd
	cmp	combatcount,70
	else
	cmp	combatcount,68
	endif
	jz	killryan
	cmp	lastweapon,8
	jnz	nomadspeak
	if	cd
	mov	combatcount,72
	else
	mov	combatcount,70
	endif
	mov	lastweapon,-1
	mov	madmanflag,1
	mov	ax,67
	jmp	nomadspeak
killryan:	mov	ax,310
nomadspeak:	mov	[es:bx+3],ax
nomadspeed:	call	showgamereel
	mov	al,mapx
	mov	[es:bx+1],al
	call	madmode
	ret
madmanspoken:	cmp	wongame,1
	jz	alreadywon
	mov	wongame,1
	push	es bx
	call	getridoftemptext
	pop	bx es
alreadywon:	ret

ryansded:	mov	mandead,2
	call	showgamereel
	ret

	endp









	if	cd
Madmantext	proc	near
		         
	cmp	speechcount,63
	jnc	nomadtext
	cmp	ch1playing,255
	jnz	nomadtext

	mov	al,speechcount
	inc	speechcount
	add	al,47
	mov	bl,72
	mov	bh,80
	mov	cx,90
	mov	dx,1
	mov	ah,82
	call	setuptimedtemp
nomadtext:	ret

	endp
	
	else

Madmantext	proc	near
		         
	cmp	combatcount,61
	jnc	nomadtext
	mov	al,combatcount
	and	al,3
	jnz	nomadtext
	mov	al,combatcount
	shr	al,1
	shr	al,1
	add	al,47
	mov	bl,72
	mov	bh,80
	mov	cx,90
	mov	dx,1
	mov	ah,82
	call	setuptimedtemp
nomadtext:	ret

	endp
	endif




Madmode	proc	near

	mov	watchingtime,2
	mov	pointermode,0
	if	cd
	cmp	combatcount,65
	else
	cmp	combatcount,63
	endif
	jc	iswatchmad
	if	cd
	cmp	combatcount,70
	else
	cmp	combatcount,68
	endif
	jnc	iswatchmad
	mov	pointermode,2	
iswatchmad:	ret

	endp





Priesttext	proc	near
	
	cmp	word ptr [es:bx+3],2
	jc	nopriesttext
	cmp	word ptr [es:bx+3],7
	jnc	nopriesttext
	mov	al,[es:bx+3]
	and	al,1
	jnz	nopriesttext
	mov	al,[es:bx+3]
	shr	al,1
	add	al,50
	mov	bl,72
	mov	bh,80
	mov	cx,54
	mov	dx,1
	call	setuptimeduse
nopriesttext:	ret

	endp




Textforend	proc	near

	cmp	introcount,20
	jnz	notendtext1
	mov	al,0
	mov	bl,34
	mov	bh,20
	mov	cx,60
	jmp	gotendtext
notendtext1:	if	cd
	cmp	introcount,50
	else
	cmp	introcount,65
	endif
	jnz	notendtext2
	mov	al,1
	mov	bl,34
	mov	bh,20
	mov	cx,60
	jmp	gotendtext
notendtext2:	if	cd
	cmp	introcount,85
	else
	cmp	introcount,110
	endif
	jnz	notendtext3
	mov	al,2
	mov	bl,34
	mov	bh,20
	mov	cx,60
	jmp	gotendtext
notendtext3:	ret

gotendtext:	mov	dx,1
	mov	ah,83
	call	setuptimedtemp
	ret

	endp








Textformonk	proc	near

	cmp	introcount,1
	jnz	notmonktext1
	mov	al,19
	mov	bl,68
	mov	bh,154
	mov	cx,120
	jmp	gotmonktext
notmonktext1:	cmp	introcount,5
	jnz	notmonktext2
	mov	al,20
	mov	bl,68
	mov	bh,38
	mov	cx,120
	jmp	gotmonktext
notmonktext2:	cmp	introcount,9
	jnz	notmonktext3
	mov	al,21
	mov	bl,48
	mov	bh,154
	mov	cx,120
	jmp	gotmonktext
notmonktext3:	cmp	introcount,13
	jnz	notmonktext4
	mov	al,22
	mov	bl,68
	mov	bh,38
	mov	cx,120
	jmp	gotmonktext
notmonktext4:	if	cd
	cmp	introcount,15
	else
	cmp	introcount,17
	endif
	jnz	notmonktext5
	mov	al,23
	mov	bl,68
	mov	bh,154
	mov	cx,120
	jmp	gotmonktext
notmonktext5:	cmp	introcount,21
	jnz	notmonktext6
	mov	al,24
	mov	bl,68
	mov	bh,38
	mov	cx,120
	jmp	gotmonktext
notmonktext6:	cmp	introcount,25
	jnz	notmonktext7
	mov	al,25
	mov	bl,68
	mov	bh,154
	mov	cx,120
	jmp	gotmonktext
notmonktext7:	cmp	introcount,29
	jnz	notmonktext8
	mov	al,26
	mov	bl,68
	mov	bh,38
	mov	cx,120
	jmp	gotmonktext
notmonktext8:	cmp	introcount,33
	jnz	notmonktext9
	mov	al,27
	mov	bl,68
	mov	bh,154
	mov	cx,120
	jmp	gotmonktext
notmonktext9:	cmp	introcount,37
	jnz	notmonktext10
	mov	al,28
	mov	bl,68
	mov	bh,154
	mov	cx,120
	jmp	gotmonktext
notmonktext10:	cmp	introcount,41
	jnz	notmonktext11
	mov	al,29
	mov	bl,68
	mov	bh,38
	mov	cx,120
	jmp	gotmonktext
notmonktext11:	cmp	introcount,45
	jnz	notmonktext12
	mov	al,30
	mov	bl,68
	mov	bh,154
	mov	cx,120
	jmp	gotmonktext
notmonktext12:	if	cd
	cmp	introcount,52
	else
	cmp	introcount,49
	endif
	jnz	notmonktext13
	mov	al,31
	mov	bl,68
	mov	bh,154
	mov	cx,220 ;132
	jmp	gotmonktext
notmonktext13:	cmp	introcount,53
	jnz	notendtitles
	call	fadescreendowns
	if	cd
	mov	volumeto,7
	mov	volumedirection,1
	endif
notendtitles:	ret

gotmonktext:	mov	dx,1
	mov	ah,82
	if	cd
	cmp	ch1playing,255
	jz	oktalk
	dec	introcount
	ret
	endif
oktalk:	call	setuptimedtemp
	ret

	endp










Drunk	proc	near

	cmp	generaldead,0
	jnz	trampgone
	mov	al,[es:bx+7]
	and	al,127
	mov	[es:bx+7],al
	call	showgamereel
	call	addtopeoplelist
trampgone:	ret

	endp


Advisor	proc	near

	call	checkspeed
	jnz	noadvisor
	jmp	noadvisor
	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,123
	jnz	notendadvis
	mov	ax,106
	jmp	gotadvframe
notendadvis:	cmp	ax,108
	jnz	gotadvframe
	push	ax
	call	randomnumber
	mov	cl,al
	pop	ax
	cmp	cl,3
	jc	gotadvframe
	mov	ax,106
gotadvframe:	mov	[es:bx+3],ax
noadvisor:	call	showgamereel
	call	addtopeoplelist
	ret
		    
	endp



Copper	proc	near

	call	checkspeed
	jnz	nocopper
	mov	ax,[es:bx+3]
	inc	ax
	cmp	ax,94
	jnz	notendcopper
	mov	ax,64
	jmp	gotcopframe
notendcopper:	cmp	ax,81
	jz	mightwait
	cmp	ax,66
	jnz	gotcopframe
mightwait:	push	ax
	call	randomnumber
	mov	cl,al
	pop	ax
	cmp	cl,7
	jc	gotcopframe
	dec	ax
gotcopframe:	mov	[es:bx+3],ax
nocopper:	call	showgamereel
	call	addtopeoplelist
	ret

	endp










Sparky	proc	near

	cmp	card1money,0
	jz	animsparky
	mov	byte ptr [es:bx+7],3
	jmp	animsparky

animsparky:	call	checkspeed
	jnz	finishsparky
	cmp	word ptr [es:bx+3],34
	jnz	notsparky1
	call	randomnumber
	cmp	al,30
	jc	dosparky
	mov	word ptr [es:bx+3],27
	jmp	finishsparky

notsparky1:	cmp	word ptr [es:bx+3],48
	jnz	dosparky
	mov	word ptr [es:bx+3],27
	jmp	finishsparky

dosparky:	inc	word ptr [es:bx+3]
finishsparky:	call	showgamereel
	call	addtopeoplelist

	mov	al,[es:bx+7]
	and	al,128
	jz	nottalkedsparky
          	mov	talkedtosparky,1
nottalkedsparky:	ret
	
	endp





Train	proc	near

	ret
	mov	ax,[es:bx+3]
	cmp	ax,21
	jnc	notrainyet
	inc	ax
	jmp	gottrainframe
notrainyet:	call	randomnumber
	cmp	al,253
	jc	notrainatall
	cmp	manspath,5
	jnz	notrainatall
	cmp	finaldest,5
	jnz	notrainatall
	mov	ax,5
gottrainframe:	mov	[es:bx+3],ax
	call	showgamereel
notrainatall:	ret

	endp







Addtopeoplelist 	proc	near

	push	es bx bx
	mov	cl,[es:bx+7]
	mov	ax,[es:bx+3]
	mov	bx,listpos
	mov	es,buffers
	mov	[es:bx],ax	;reel pointer position
	pop	ax
	mov	[es:bx+2],ax	                      
	mov	[es:bx+4],cl		;coversation number
	pop	bx es
	add	listpos,5
	ret

	endp



Showgamereel	proc	near

	mov	ax,[es:bx+3]
	cmp	ax,512
	jnc	noshow
	mov	reelpointer,ax
	push	es bx
	call	plotreel
	pop	bx es
	mov	ax,reelpointer
	mov	[es:bx+3],ax
noshow: 	ret

	endp






Checkspeed	proc	near

	cmp	lastweapon,-1
	jnz	forcenext
	inc	byte ptr [es:bx+6]
	mov	al,[es:bx+6]
	cmp	al,[es:bx+5]
	jnz	notspeed
	mov	al,0
	mov	[es:bx+6],al
	cmp	al,al
notspeed:	ret

forcenext:	cmp	al,al
	ret

	endp










;------------------------------------------------------------Sprite Routines----



Clearsprites	proc	near

	mov	es,buffers
	mov	di,spritetable
	mov	al,255
	mov	cx,tablesize*16
	rep	stosb
	ret

	endp




Makesprite	proc	near		;si holds x,y	cx holds update
				;di,dx holds data offset,seg
	mov	es,buffers
	mov	bx,spritetable
$17:	cmp	byte ptr [es:bx+15],255
	jz	$17a
	add	bx,tablesize
	jmp	$17

$17a:	mov	[es:bx],cx
	mov	[es:bx+10],si
	mov	[es:bx+6],dx
	mov	[es:bx+8],di
	mov	[es:bx+2],0ffffh
	mov	byte ptr [es:bx+15],0
	mov	byte ptr [es:bx+18],0
	ret

	endp










Delsprite	proc	near

	mov	di,bx
	mov	cx,tablesize
	mov	al,255
	rep	stosb
	ret

	endp










Spriteupdate	proc	near

	mov	es,buffers
	mov	bx,spritetable
	mov	al,ryanon
	mov	byte ptr [es:bx+31],al

	mov	es,buffers
	mov	bx,spritetable
	mov	cx,16
$18:	push	cx bx
	mov	ax,[es:bx]
	cmp	ax,0ffffh
	jz	$18a
	push	es ds
	mov	cx,[es:bx+2]
	mov	[es:bx+24],cx
	call	ax
	pop	ds es
$18a:	pop	bx cx
	cmp	nowinnewroom,1
	jz	$18b
	add	bx,tablesize
	loop	$18

$18b:	ret

	endp




























Printsprites	proc	near

	mov	es,buffers
	mov	cx,0
priorityloop:	push	cx
	mov	priority,cl
	mov	bx,spritetable
	mov	cx,16
prtspriteloop:	push	cx bx
	mov	ax,[es:bx]
	cmp	ax,0ffffh
	jz	skipsprite
	mov	al,priority
	cmp	al,[es:bx+23]
	jnz	skipsprite
	cmp	byte ptr [es:bx+31],1
	jz	skipsprite
	call	printasprite
skipsprite:	pop	bx cx
	add	bx,tablesize
	loop	prtspriteloop
	pop	cx
	inc	cx
	cmp	cx,7
	jnz	priorityloop
	ret

	endp










Printasprite	proc	near

	push	es bx
	mov	si,bx
	mov	ds,[es:si+6]
	mov	al,[es:si+11]
	mov	ah,0
	cmp	al,220
	jc	notnegative1
	mov	ah,255
notnegative1:	mov	bx,ax
	add	bx,mapady
	mov	al,[es:si+10]
	mov	ah,0
	cmp	al,220
	jc	notnegative2
	mov	ah,255
notnegative2:	mov	di,ax
	add	di,mapadx
	mov	al,[es:si+15]
	mov	ah,0
	cmp	byte ptr [es:si+30],0
	jz	steadyframe
	mov	ah,8
steadyframe:	cmp	priority,6
	jnz	notquickp
notquickp:	call	showframe
	pop	bx es
	ret

	endp




;cmp	priority,6
;	;jz	quicksprite	; WIll NEED TO DO THIS LATER!!!!!
;Quicksprite:	mov	cl,al
;	mov	ch,0
;	mov	dx,[es:bx+6]
;	mov	es,workspace
;	mov	ds,dx
;	mov	bx,ax
;	mov	dx,vgawidth
;
;printquickloop: push	di si
;
;	push	si
;	add	si,cx
;	dec	si
;	jmp	startzero
;zeroloop:	dec	si
;	dec	cl
;startzero:	cmp	[si],ch
;	jz	zeroloop
;	pop	si
;
;;printquickline: cmp	[si],ch
;	jnz	foundfirstpix
;	inc	si
;	inc	di
;	dec	cl
;	jnz	printquickline
;
;foundfirstpix:	cmp	cl,0
;	jz	finquickspr
;	rep	movsb
;
;finquickspr:	pop	si di
;	mov	cl,bl
;	add	si,cx
;	add	di,dx
;	dec	bh
;	jnz	printquickloop
;
;	pop	bx ds es
;	ret







;Calcframe	proc	near
;
;	ret
;
;	mov	al,[es:bx+15]
;	mov	ah,0
;	mov	cx,6
;	mul	cx
;	add	ax,[es:bx+8]
;
;	mov	dx,[es:bx+6]
;	push	bx
;	mov	ds,dx
;	mov	bx,ax
;	mov	ax,[bx]
;	mov	cx,[bx+2]
;	mov	dx,[bx+4]
;	pop	bx
;	mov	[es:bx+2],ax
;	add	cx,[es:bx+8]
;	add	cx,2080
;	mov	[es:bx+4],cx	 ;calculates frame data
;
;	mov	al,[es:bx+10]	;this bit calculates the actual
;	add	al,dl		;x and y (including any offset)
;	mov	[es:bx+12],al
;	mov	al,[es:bx+11]
;	add	al,dh
;	mov	[es:bx+13],al
;	ret
;
;	endp













		






Checkone	proc	near		;cx=x,y to check

	push	cx
	mov	al,ch
	mov	ah,0
	mov	cl,4
	shr	ax,cl
	mov	dl,al
	pop	cx
	mov	al,cl
	mov	ah,0
	mov	cl,4
	shr	ax,cl
	mov	ah,dl		; al,ah holds x,y in blocks
	
	push	ax
	mov	ch,0
	mov	cl,al
	push	cx
	mov	al,ah
	mov	ah,0
	mov	cx,11
	mul	cx
	pop	cx
	add	ax,cx

	mov	cx,3
	mul	cx
	mov	si,ax

	mov	ds,buffers
	add	si,mapflags
	lodsw
	mov	cx,ax
	lodsb
	pop	dx
	ret


	endp


















Findsource	proc	near

	mov	ax,currentframe
	cmp	ax,160
	jnc	over1000
	mov	ds,reel1
	mov	takeoff,0
	ret
over1000:	cmp	ax,320
	jnc	over1001
	mov	ds,reel2
	mov	takeoff,160
	ret
over1001:	mov	ds,reel3
	mov	takeoff,320
	ret

	endp











;---------------------------------------------------------Routines for Ryan----


Initman 	proc	near

	;mov	linepointer,254
	mov	al,ryanx
	mov	ah,ryany
	mov	si,ax
	mov	cx,offset cs:mainman
	mov	dx,mainsprites
	mov	di,0
	call	makesprite
	mov	byte ptr [es:bx+23],4
	mov	byte ptr [es:bx+22],0
	mov	byte ptr [es:bx+29],0
	ret

	endp







Mainman 	proc	near

	cmp	resetmanxy,1
	jnz	notinnewroom
	mov	resetmanxy,0
	mov	al,ryanx
	mov	ah,ryany
	mov	[es:bx+10],ax
	mov	byte ptr [es:bx+29],0
	jmp	executewalk
notinnewroom:	dec	byte ptr [es:bx+22]
	cmp	byte ptr [es:bx+22],-1
	jz	executewalk
	ret


executewalk:	mov	byte ptr [es:bx+22],0 ; speed
	mov	al,turntoface
	cmp	al,facing
	jz	facingok
	call	aboutturn
	jmp	notwalk

facingok:	cmp	turndirection,0
	jz	alreadyturned
	cmp	linepointer,254
	jnz	alreadyturned
	mov	reasseschanges,1
	mov	al,facing
	cmp	al,leavedirection
	jnz	alreadyturned
	call	checkforexit
alreadyturned:	mov	turndirection,0
	cmp	linepointer,254
	jnz	walkman
	mov	byte ptr [es:bx+29],0
	jmp	notwalk

walkman:	mov	al,[es:bx+29]
	inc	al
	cmp	al,11
	jnz	notanimend1
	mov	al,1
notanimend1:	mov	[es:bx+29],al

	call	walking
	cmp	linepointer,254
	jz	afterwalk

	mov	al,facing
	and	al,1
	jz	isdouble
	mov	al,[es:bx+29]
	cmp	al,2
	jz	afterwalk
	cmp	al,7
	jz	afterwalk
isdouble:	call	walking
afterwalk:	cmp	linepointer,254
	jnz	notwalk
	mov	al,turntoface
	cmp	al,facing
	jnz	notwalk
	mov	reasseschanges,1
	mov	al,facing
	cmp	al,leavedirection
	jnz	notwalk
	call	checkforexit

notwalk:	mov	al,facing
	mov	ah,0
	mov	di,offset cs:facelist
	add	di,ax
	mov	al,[cs:di]
	add	al,[es:bx+29]
	mov	[es:bx+15],al
	mov	ax,[es:bx+10]
	mov	ryanx,al
	mov	ryany,ah
	ret

facelist:	db	0,60,33,71,11,82,22,93

	endp










Aboutturn	proc	near

	cmp	turndirection,1
	jz	incdir
	cmp	turndirection,-1
	jz	decdir
	mov	al,facing
	sub	al,turntoface
	jnc	higher
	neg	al
	cmp	al,4
	jnc	decdir
	jmp	incdir
higher: 	cmp	al,4
	jnc	incdir
	jmp	decdir

incdir:	mov	turndirection,1
	mov	al,facing
	inc	al
	and	al,7
	mov	facing,al
	mov	byte ptr [es:bx+29],0
	ret

decdir: 	mov	turndirection,-1
	mov	al,facing
	dec	al
	and	al,7
	mov	facing,al
	mov	byte ptr [es:bx+29],0
	ret

	endp








Walking 	proc	near

	cmp	linedirection,0
	jz	normalwalk
	mov	al,linepointer
	dec	al
	mov	linepointer,al
	cmp	al,200
	jnc	endofline
	jmp	continuewalk

normalwalk:	mov	al,linepointer
	inc	al
	mov	linepointer,al
	cmp	al,linelength
	jnc	endofline

continuewalk:	mov	ah,0
	add	ax,ax
	push	es bx
	mov	dx,seg linedata
	mov	es,dx
	mov	bx,offset es:linedata
	add	bx,ax
	mov	ax,[es:bx]
	pop	bx es
stillline:	mov	[es:bx+10],ax
	ret

endofline:	mov	linepointer,254
	mov	al,destination
	mov	manspath,al
	cmp	al,finaldest
	jz	finishedwalk
	mov	al,finaldest
	mov	destination,al
	push	es bx
	call	autosetwalk
	pop	bx es
	ret

finishedwalk:	call	facerightway
	ret

	endp







Facerightway	proc	near

	push	es bx		;Face object when finished
	call	getroomspaths		;walking
	mov	al,manspath
	mov	ah,0
	add	ax,ax
	add	ax,ax
	add	ax,ax
	add	bx,ax
	mov	al,[es:bx+7]
	mov	turntoface,al
	mov	leavedirection,al
	pop	bx es
	ret

	endp







Checkforexit	proc	near

	mov	cl,ryanx		;look under feet to see if
	add	cl,12		;any flags are there
	mov	ch,ryany
	add	ch,12
	call	checkone
	mov	lastflag,cl
	mov	lastflagex,ch
	mov	flagx,dl
	mov	flagy,dh
	mov	al,lastflag
	
	test	al,64
	jz	notnewdirect
	mov	al,lastflagex
	mov	autolocation,al
	ret

notnewdirect:	test	al,32
	jz	notleave
	push	es bx
	cmp	reallocation,2
	jnz	notlouis
	mov	bl,0
	push	bx
	mov	al,"W"
	mov	ah,"E"
	mov	cl,"T"
	mov	ch,"A"
	call	isryanholding
	pop	bx
	jz 	noshoe1
	inc	bl
noshoe1:	push	bx	
	mov	al,"W"
	mov	ah,"E"
	mov	cl,"T"
	mov	ch,"B"
	call	isryanholding
	pop	bx
	jz	noshoe2
	inc	bl
noshoe2:	cmp	bl,2
	jz	notlouis	
	mov	al,42
	cmp	bl,0
	jz	notravmessage
	inc	al
notravmessage:	mov	cx,80
	mov	dx,10
	mov	bl,68
	mov	bh,64
	call	setuptimeduse
	mov	al,facing
	add	al,4
	and	al,7
	mov	turntoface,al
	pop	bx es
	ret

notlouis:	pop	bx es
	mov	needtotravel,1
	ret



notleave:	test	al,4
	jz	notaleft
	call	adjustleft
	ret

notaleft:	test	al,2
	jz	notaright
	call	adjustright
	ret

notaright:	test	al,8
	jz	notadown
	call	adjustdown
	ret

notadown:	test	al,16
	jz	notanup
	call	adjustup
	ret

notanup:	ret

	endp





Adjustdown	proc	near

	push	es bx	
	add	mapy,10
	mov	al,lastflagex
	mov	cl,16
	mul	cl
	mov	[es:bx+11],al
	mov	nowinnewroom,1
	pop	bx es
	ret

	endp




Adjustup	proc	near
 
	push	es bx
	sub	mapy,10
	mov	al,lastflagex
	mov	cl,16
	mul	cl
	mov	[es:bx+11],al
	mov	nowinnewroom,1
	pop	bx es
	ret

	endp





Adjustleft	proc	near

	push	es bx
	mov	lastflag,0
	sub	mapx,11
	mov	al,lastflagex
	mov	cl,16
	mul	cl
	mov	[es:bx+10],al
	mov	nowinnewroom,1
	pop	bx es
	ret

	endp






Adjustright	proc	near
	
	push	es bx
	add	mapx,11
	mov	al,lastflagex
	mov	cl,16
	mul	cl
	sub	al,2
	mov	[es:bx+10],al
	mov	nowinnewroom,1
	pop	bx es
	ret

	endp





Reminders	proc	nar

	cmp	reallocation,24
	jnz	notinedenslift
	cmp	mapx,44
	jnz	notinedenslift
	cmp	progresspoints,0
	jnz	notfirst
	mov	al,"D"
	mov	ah,"K"
	mov	cl,"E"
	mov	ch,"Y"
	call	isryanholding
	jz	forgotone
	mov	al,"C"
	mov	ah,"S"
	mov	cl,"H"
	mov	ch,"R"
	call	findexobject
	cmp	al,numexobjects
	jz	forgotone
	mov	ax,[es:bx+2]
	cmp	al,4
	jnz	forgotone	;card is in inventory
	cmp	ah,255
	jz	havegotcard 	;card must be in an ex
	mov	cl,"P"	;object
	mov	ch,"U"
	mov	dl,"R"
	mov	dh,"S"
	xchg	al,ah
	call	compare
	jnz	forgotone	;is it in wallet?		
havegotcard:	inc	progresspoints
notfirst:	ret

forgotone:	mov	al,50	;message number
	mov	bl,54	;x pos of message
	mov	bh,70	;and y pos
	mov	cx,48	;time on screen
	mov	dx,8	;pause before show
	call	setuptimeduse
	ret
notinedenslift:	ret

	endp




	
;---------------------------------------------------------------------------
;
;	Sprite update routines for rain effect
;
;---------------------------------------------------------------------------


	

Initrain 	proc	near

	mov	es,buffers
	mov	di,rainlist
	mov	bx,offset cs:rainlocations
checkmorerain:	mov	al,[cs:bx]
	cmp	al,255
	jz	finishinitrain
	cmp	al,reallocation
	jnz	checkrain
	mov	al,[cs:bx+1]
	cmp	al,mapx
	jnz	checkrain
	mov	al,[cs:bx+2]
	cmp	al,mapy
	jnz	checkrain
	mov	al,[cs:bx+3]
	mov	rainspace,al
	jmp	dorain
checkrain:	add	bx,4
	jmp	checkmorerain

dorain:	mov	cx,4
initraintop:	call	randomnumber
	and	al,31
	add	al,3
	cmp	al,rainspace
	jnc	initraintop
	add	cl,al
	cmp	cl,mapxsize
	jnc	initrainside
	push	cx
	call	splitintolines
	pop	cx
	jmp	initraintop

initrainside:	mov	cl,mapxsize
	dec	cl
initrainside2:	call	randomnumber
	and	al,31
	add	al,3
	cmp	al,rainspace 
	jnc	initrainside2	
	add	ch,al
	cmp	ch,mapysize
	jnc	finishinitrain
	push	cx
	call	splitintolines
	pop	cx
	jmp	initrainside2
finishinitrain:	mov	al,255
	stosb
	ret

rainlocations:	db	1,44,10,16	;location,map x,y,seed
	db	4,11,30,14
	db	4,22,30,14
	db	3,33,10,14
	db	10,33,30,14
	db	10,22,30,24
	db	9,22,10,14
	db	2,33,0,14
	db	2,22,0,14
	db	6,11,30,14
	db	7,11,20,18
	db	7,0,20,18
	db	7,0,30,18
	db	55,44,0,14
	db	5,22,30,14

	db	8,0,10,18
	db	8,11,10,18
	db	8,22,10,18
	db	8,33,10,18
	db	8,33,20,18
	db	8,33,30,18
	db	8,33,40,18
	db	8,22,40,18
	db	8,11,40,18
    
	db	21,44,20,18
	db	255

	endp


	

	





Splitintolines	proc	near


lookforlinestart:	call	getblockofpixel
	cmp	al,0
	jnz	foundlinestart
              	dec	cl
	inc	ch
	cmp	cl,0
	jz	endofthisline	
	cmp	ch,mapysize
	jnc	endofthisline
	jmp	lookforlinestart

foundlinestart:	mov	[es:di],cx
	mov	bh,1
lookforlineend:	call	getblockofpixel
	cmp	al,0
	jz	foundlineend
              	dec	cl
	inc	ch
	cmp	cl,0
	jz	foundlineend
	cmp	ch,mapysize
	jnc	foundlineend
	inc	bh
	jmp	lookforlineend

foundlineend:	push	cx	
	mov	[es:di+2],bh
	call	randomnumber
	mov	[es:di+3],al
	call	randomnumber
	mov	[es:di+4],al
	call	randomnumber
	and	al,3
	add	al,4
	mov	[es:di+5],al
	add	di,6
	pop	cx
	cmp	cl,0
	jz	endofthisline	
	cmp	ch,mapysize
	jnc	endofthisline
	jmp	lookforlinestart

endofthisline:	ret

	endp




Getblockofpixel	proc	near
		
	push	cx es di
	mov	ax,mapxstart
	add	cl,al
	mov	ax,mapystart
	add	ch,al
	call	checkone
	and	cl,1
	jnz	failrain
	pop	di es cx
	ret
failrain:	pop	di es cx
	mov	al,0
	ret

	endp




Showrain	proc	near

	mov	ds,mainsprites
	mov	si,6*58
	mov	ax,[si+2]
	mov	si,ax
	add	si,2080

	mov	bx,rainlist
	mov	es,buffers
	cmp	byte ptr [es:bx],255
	jz	nothunder

morerain:	mov	es,buffers
	cmp	byte ptr [es:bx],255
	jz	finishrain

	mov	al,[es:bx+1]
	mov	ah,0
	add	ax,mapady
	add	ax,mapystart
	mov	cx,320
	mul	cx
	mov	cl,[es:bx]
	mov	ch,0
	add	ax,cx
	add	ax,mapadx
	add	ax,mapxstart
	mov	di,ax

	mov	cl,[es:bx+2]
	mov	ch,0
	mov	ax,[es:bx+3]
	mov	dl,[es:bx+5]
	mov	dh,0
	sub	ax,dx
	and	ax,511
	mov	[es:bx+3],ax
	add	bx,6

	push	si
	add	si,ax
	mov	es,workspace
	mov	ah,0
	mov	dx,320-2
rainloop:	lodsb
	cmp	al,ah
	jz	noplot
	stosb
	add	di,dx
	loop	rainloop
	pop	si
	jmp	morerain
noplot:	add	di,320-1
	loop	rainloop
	pop	si	
	jmp	morerain

finishrain:	cmp	ch1blockstocopy,0
	jnz	nothunder
	cmp	reallocation,2
	jnz	notlouisthund
	cmp	beenmugged,1
	jnz	nothunder
notlouisthund:	cmp	reallocation,55
	jz	nothunder
	call	randomnum1
	cmp	al,1
	jnc	nothunder
	mov	al,7
	cmp	ch0playing,6
	jz	isthunder1
   	mov	al,4
isthunder1:	call	playchannel1
nothunder:	ret

	endp



	

	
	
;---------------------------------------------------------------------------
;
;	Sprite update routines for background objects
;
;---------------------------------------------------------------------------






Backobject	proc	near
	
	mov	ds,setdat
	mov	di,[es:bx+20]

	mov	al,[es:bx+18]
	cmp	al,0
	jz	$48z
	dec	al
	mov	[es:bx+18],al
	jmp	finishback

$48z:	mov	al,[di+7]
	mov	[es:bx+18],al
	mov	al,[di+8]
	cmp	al,6
	jnz	notwidedoor
	call	widedoor
	jmp	finishback

notwidedoor:	cmp	al,5
	jnz	notrandom
	call	random
	jmp	finishback

notrandom:	cmp	al,4
	jnz	notlockdoor
	call	lockeddoorway
	jmp	finishback

notlockdoor:	cmp	al,3
	jnz	notlift
	call	liftsprite
	jmp	finishback

notlift:	cmp	al,2
	jnz	notdoor
	call	doorway
	jmp	finishback

notdoor:	cmp	al,1
	jnz	steadyob
	call	constant
	jmp	finishback

steadyob:	call	steady

finishback:	;call	calcframe
	ret

	endp









Liftsprite	proc	near

	mov	al,liftflag
	cmp	al,0
	jz	liftclosed
	cmp	al,1
	jz	liftopen
	cmp	al,3
	jz	openlift

	mov	al,[es:bx+19]
	cmp	al,0
	jz	finishclose
	dec	al
	cmp	al,11
	jnz	pokelift
	push	ax
	mov	al,3
	call	liftnoise
	pop	ax
	jmp	pokelift
finishclose:	mov	liftflag,0
	ret

openlift:	mov	al,[es:bx+19]
	cmp	al,12
	jz	endoflist
	inc	al
	cmp	al,1
	jnz	pokelift
              	push	ax
	mov	al,2
	call	liftnoise
	pop	ax
pokelift:	mov	[es:bx+19],al
	mov	ah,0
	push	di
	add	di,ax
	mov	al,[di+18]
	pop	di
	mov	[es:bx+15],al
	mov	[di+17],al
	ret

endoflist:	mov	liftflag,1
	ret

liftopen:	mov	al,liftpath
	push	es bx
	call	turnpathon
	pop	bx es
	cmp	counttoclose,0
	jz	nocountclose
	dec	counttoclose
	cmp	counttoclose,0
	jnz	nocountclose
	mov	liftflag,2
nocountclose:	mov	al,12
	jmp	pokelift

liftclosed:	mov	al,liftpath
	push	es bx
	call	turnpathoff
	pop	bx es
	cmp	counttoopen,0
	jz	nocountopen
	dec	counttoopen
	cmp	counttoopen,0
	jnz	nocountopen
	mov	liftflag,3
nocountopen:	mov	al,0
	jmp	pokelift

	endp


Liftnoise	proc	near

	cmp	reallocation,5
	jz	hissnoise
	cmp	reallocation,21
	jz	hissnoise
	call	playchannel1
	ret
hissnoise:	if	demo
	mov	al,25
	else
	mov	al,13
	endif
	call	playchannel1
	ret
	
	endp




Random	proc	near

	call	randomnum1
	push	di
	and	ax,7
	add	di,18
	add	di,ax
	mov	al,[di]
	pop	di
	mov	[es:bx+15],al
	ret

	endp








Steady	proc	near

	mov	al,[di+18]
	mov	[di+17],al
	mov	[es:bx+15],al
	ret

	endp





Constant	proc	near

	inc	byte ptr [es:bx+19]
	mov	cl,[es:bx+19]
	mov	ch,0
	add	di,cx
	cmp	byte ptr [di+18],255
	jnz	gotconst
	sub	di,cx
	mov	cx,0
	mov	[es:bx+19],cl
gotconst:	mov	al,[di+18]
	sub	di,cx
	mov	[es:bx+15],al
	mov	[di+17],al
	ret

	endp














Doorway 	proc	near

	mov	doorcheck1,-24
	mov	doorcheck2,10
	mov	doorcheck3,-30
	mov	doorcheck4,10
	call	dodoor
	ret

	endp



Widedoor	proc	near

	mov	doorcheck1,-24
	mov	doorcheck2,24
	mov	doorcheck3,-30
	mov	doorcheck4,24
	call	dodoor
	ret

	endp









Dodoor	proc	near

	mov	al,ryanx
	mov	ah,ryany
	mov	cl,[es:bx+10]
	mov	ch,[es:bx+11]

	cmp	al,cl
	jnc	rtofdoor
	sub	al,cl
	cmp	al,doorcheck1
	jnc	upordown
	jmp	shutdoor
rtofdoor:	sub	al,cl
	cmp	al,doorcheck2
	jnc	shutdoor

upordown:	cmp	ah,ch
	jnc	botofdoor
	sub	ah,ch
	cmp	ah,doorcheck3
	jc	shutdoor
	jmp	opendoor
botofdoor:	sub	ah,ch
	cmp	ah,doorcheck4
	jnc	shutdoor

opendoor:	mov	cl,[es:bx+19]
	cmp	throughdoor,1
	jnz	notthrough
	cmp	cl,0
	jnz	notthrough
	mov	cl,6
notthrough:	inc	cl
	cmp	cl,1
	jnz	notdoorsound2
	mov	al,0
	cmp	reallocation,5
	jnz	nothoteldoor2
	if	demo
	mov	al,25
	else
	mov	al,13
              	endif
nothoteldoor2:	call	playchannel1
notdoorsound2:	mov	ch,0

	push	di
	add	di,cx
	mov	al,[di+18] ; must be a better way than this
	cmp	al,255
	jnz	atlast1
	dec	di
	dec	cl
atlast1:	mov	[es:bx+19],cl
	mov	al,[di+18]
	pop	di
	mov	[es:bx+15],al
	mov	[di+17],al
	mov	throughdoor,1
	ret


shutdoor:	mov	cl,[es:bx+19]
	cmp	cl,5
	jnz	notdoorsound1
              	mov	al,1
	cmp	reallocation,5
	jnz	nothoteldoor1
	if	demo
	mov	al,25
	else
	mov	al,13
	endif
nothoteldoor1:	call	playchannel1
notdoorsound1:	cmp	cl,0
	jz	atlast2
	dec	cl
	mov	[es:bx+19],cl
atlast2:	mov	ch,0
	push	di
	add	di,cx
	mov	al,[di+18]
	pop	di
	mov	[es:bx+15],al
	mov	[di+17],al
	cmp	cl,5
	jnz	notnearly
	mov	throughdoor,0
notnearly:	ret

	endp








Lockeddoorway	proc	near

	mov	al,ryanx
	mov	ah,ryany
	mov	cl,[es:bx+10]
	mov	ch,[es:bx+11]

	cmp	al,cl
	jnc	rtofdoor2
	sub	al,cl
	cmp	al,-24
	jnc	upordown2
	jmp	shutdoor2
rtofdoor2:	sub	al,cl
	cmp	al,10
	jnc	shutdoor2

upordown2:	cmp	ah,ch
	jnc	botofdoor2
	sub	ah,ch
	cmp	ah,-30
	jc	shutdoor2
	jmp	opendoor2
botofdoor2:	sub	ah,ch
	cmp	ah,12
	jnc	shutdoor2

opendoor2:	cmp	throughdoor,1
	jz	mustbeopen
	cmp	lockstatus,1
	jz	shutdoor2
mustbeopen:	mov	cl,[es:bx+19]
              	cmp	cl,1
	jnz	notdoorsound4
	mov	al,0
	call	playchannel1
notdoorsound4:	cmp	cl,6 ; was 3
	jnz	noturnonyet
	mov	al,doorpath
	push	es bx
	call	turnpathon
	pop	bx es

noturnonyet:	mov	cl,[es:bx+19]
	cmp	throughdoor,1
	jnz	notthrough2
	cmp	cl,0
	jnz	notthrough2
	mov	cl,6
notthrough2:	inc	cl
	mov	ch,0

	push	di
	add	di,cx
	mov	al,[di+18]
	cmp	al,255
	jnz	atlast3
	dec	di
	dec	cl
atlast3:	mov	[es:bx+19],cl
	mov	al,[di+18]
	pop	di
	mov	[es:bx+15],al
	mov	[di+17],al
	cmp	cl,5
	jnz	justshutting
	mov	throughdoor,1
justshutting:	ret



shutdoor2:	mov	cl,[es:bx+19]
	cmp	cl,5
	jnz	notdoorsound3
              	mov	al,1
	call	playchannel1
notdoorsound3:	cmp	cl,0
	jz	atlast4
	dec	cl
	mov	[es:bx+19],cl
atlast4:	mov	ch,0
	mov	throughdoor,0
	push	di
	add	di,cx
	mov	al,[di+18]
	pop	di
	mov	[es:bx+15],al
	mov	[di+17],al
	cmp	cl,0 ;1
	jnz	notlocky
	mov	al,doorpath
	push	es bx
	call	turnpathoff
	pop	bx es
	mov	lockstatus,1
notlocky:	ret

	endp










;------------------------------------------------------------People handler----

Updatepeople	proc	near

	mov	es,buffers
	mov	di,peoplelist
	mov	listpos,di
	mov	cx,12*5
	mov	al,255
	rep	stosb

	inc	maintimer
	push	cs
	pop	es
	mov	bx,offset cs:reelroutines
	mov	di,offset cs:reelcalls
updateloop:	mov	al,[es:bx]
	cmp	al,255
	jz	endupdate
	cmp	al,reallocation
	jnz	notinthisroom
	mov	cx,[es:bx+1]
	cmp	cl,mapx
	jnz	notinthisroom
	cmp	ch,mapy
	jnz	notinthisroom
	push	di
	mov	ax,[cs:di]
	call	ax
	pop	di
notinthisroom:	add	bx,8
	add	di,2
	jmp	updateloop
endupdate:	ret

	endp














Getreelframeax	proc	near

	push	ds
	mov	currentframe,ax
	call	findsource
	push	ds
	pop	es
	pop	ds
	mov	ax,currentframe
	sub	ax,takeoff
	add	ax,ax
	mov	cx,ax
	add	ax,ax
	add	ax,cx
	mov	bx,ax
	ret

	endp










Reelsonscreen	proc	near

	call	reconstruct
	call	updatepeople
	call	watchreel
	call	showrain
	call	usetimedtext
	ret

	endp






Plotreel	proc	near

	call	getreelstart
retryreel:	push	es si
	mov	ax,[es:si+2]
	cmp	al,220
	jc	normalreel
	cmp	al,255
	jz	normalreel
	call	dealwithspecial
	inc	reelpointer
	pop	si es
	add	si,40
	jmp	retryreel

normalreel:	mov	cx,8
plotloop:	push	cx es si
	mov	ax,[es:si]
	cmp	ax,0ffffh
	jz	notplot
	call	showreelframe
notplot:	pop	si es cx
	add	si,5
	loop	plotloop
	call	soundonreels
	pop	bx es
	ret

	endp





Soundonreels	proc	near
	
	mov	bl,reallocation
	add	bl,bl
	xor	bh,bh
	add	bx,offset cs:roombyroom
	mov	si,[cs:bx]

reelsoundloop:	mov	al,[cs:si]
	cmp	al,255
	jz	endreelsound
             	mov	ax,[cs:si+1]
	cmp	ax,reelpointer
	jnz	skipreelsound
	
	cmp	ax,lastsoundreel
	jz	skipreelsound
	
	mov	lastsoundreel,ax
	mov	al,[cs:si]
	cmp	al,64
	jc	playchannel1
	cmp	al,128
	jc	channel0once
	and	al,63
	mov	ah,255
	jmp	playchannel0
channel0once:	and	al,63
	mov	ah,0
	jmp	playchannel0
skipreelsound:	add	si,3
	jmp	reelsoundloop
endreelsound:	mov	ax,lastsoundreel
	cmp	ax,reelpointer
	jz	nochange2
         	mov	lastsoundreel,-1
nochange2:	ret

roombyroom	dw	r0,r1,r2,r0,r0,r0,r6,r0,r8,r9,r10,r11,r12,r13,r14,r0,r0,r0,r0,r0
	dw	r20,r0,r22,r23,r0,r25,r26,r27,r28,r29,r0,r0,r0,r0,r0
	dw	r0,r0,r0,r0,r0,r0,r0,r0,r0,r0,r45,r46,r47,r0,r0,r0,r0,r52,r53,r0,r55

r0	db	255

r1	db	15
	dw	257
	db	255

r2	db	12
	dw	5
	db	13
	dw	21
	db	15	;hitting floor?
	dw	35
	db	17
	dw	50
	db	18
	dw	103
	db	19
	dw	108
	db	255

r6	db	18
	dw	19
	db	19
	dw	23
  	db	255
	
r8	db	12
	dw	51
	db	13
	dw	53
	db	14
	dw	14
	db	15
	dw	20
	db	0
	dw	78
	db	255

r9	db	12
	dw	119
	db	12
	dw	145
	db	255

r10	db	13
	dw	16
	db	255
	
r11	db	13
	dw	20
	db	255

r12	db	14
	dw	16
   	db	255

r13	db	15
	dw	4
	db	16
	dw	8
	db	17
	dw	134
	db	18
	dw	153
	db	255

r14	db	13
	dw	108
	db	15
	dw	326
	db	15
	dw	331
	db	15
	dw	336
	db	15
	dw	342
	db	15
	dw	348
	db	15
	dw	354
	db	18
	dw	159
	db	18
	dw	178
	db	19+128
	dw	217
	db	20+64
	dw	228
   	db	255

r20	db	13
	dw	20
	db	13
	dw	21
	db	15
	dw	34
	db	13
	dw	52
	db	13
	dw	55
	db	25
	dw	57
	db	21
	dw	73
   	db	255

r22	db	13	;room,sample
	dw	196	;reelpointer
	db	13
	dw	234
	db	13
	dw	156
	db	14
	dw	129
	db	13
	dw	124
	db	15
	dw	162
	db	15
	dw	200
	db	15
	dw	239
	db	17
	dw	99
	db	12
	dw	52
   	db	255

r23	db	15
	dw	56
	db	16
	dw	64
	db	19
	dw	22
	db	20
	dw	33
	db	255

r25	db	20
	dw	11
	db	20
	dw	15
	db	15
	dw	28
	db	13
	dw	80
	db	21
	dw	82
	db	19+128
	dw	87
	db	23+64
	dw	128
	db	255

r26	db	12
	dw	13
	db	14
	dw	39
	db	12
	dw	67
	db	12
	dw	75
	db	12
	dw	83
	db	12
	dw	91
	db	15
	dw	102 ; was 90, should be mine cart
	db	255
	
r27	db	22
	dw	36
	db	13
	dw	125
	db	18
	dw	88
	db	15
	dw	107
	db	14
	dw	127
	db	14
	dw	154
	db	19+128
	dw	170
	db	23+64
	dw	232
	db	255

r28	db	21
	dw	16
	db	21
	dw	72
	db	21
	dw	205
	db	22
	dw	63 ;65
	db	23+128
	dw	99
	db	24+64
	dw	158
	db	255

r29	db	13
	dw	21
	db	14
	dw	24
	db	19+128
	dw	50
	db	23+64
	dw	75
	if	german
	else
	db	24
	dw	128
	endif
	db	255

r45	db	19+64
	dw	46
	db	16
	dw	167
	db	255

r46	db	16
	dw	19
	db	14
	dw	36
	db	16
	dw	50
	db	14
	dw	65
	db	16
	dw	81
	db	14
	dw	96
	db	16
	dw	114
	db	14
	dw	129
	db	16
	dw	147
	db	14
	dw	162
	db	16
	dw	177
	db	14
	dw	191
	db	255

r47	db	13
	dw	48
	db	14
	dw	41
	db	15
	dw	78
	db	16
	dw	92
	db	255

r52	db	16
	dw	115
	db	255

r53	db	21
	dw	103
	db	20
	dw	199
	db	255

r55	db	17
	dw	53
	db	17
	dw	54
	db	17
	dw	55
	db	17
	dw	56
	db	17
	dw	57
	db	17
	dw	58
	db	17
	dw	59
	db	17
	dw	61
	db	17
	dw	63
	db	17
	dw	64
	db	17
	dw	65
	db	255

	endp









Reconstruct	proc	near

	cmp	havedoneobs,0
	jz	noneedtorecon
	mov	newobs,1
	call	drawfloor
	call	spriteupdate
	call	printsprites
	cmp	foreignrelease, 0
	jz notfudge
	cmp	reallocation,20
	jnz	notfudge
	call	undertextline
notfudge:
	mov	havedoneobs,0
noneedtorecon:	ret

	endp










Dealwithspecial	proc	near

	sub	al,220
	cmp	al,0
	jnz	notplset
	mov	al,ah
	call	placesetobject
	mov	havedoneobs,1
	ret
notplset:	cmp	al,1
	jnz	notremset
	mov	al,ah
	call	removesetobject
	mov	havedoneobs,1
	ret
notremset:	cmp	al,2
	jnz	notplfree
	mov	al,ah
	call	placefreeobject
	mov	havedoneobs,1
	ret
notplfree:	cmp	al,3
	jnz	notremfree
	mov	al,ah
	call	removefreeobject
	mov	havedoneobs,1
	ret
notremfree:	cmp	al,4
	jnz	notryanoff
	call	switchryanoff
	ret
notryanoff:	cmp	al,5
	jnz	notryanon
	mov	turntoface,ah
	mov	facing,ah
	call	switchryanon
	ret
notryanon:	cmp	al,6
	jnz	notchangeloc
	mov	newlocation,ah ; was new loc in watch
	;call	switchryanon
	ret
notchangeloc:	call	movemap
	ret

	endp



Movemap	proc	near

	cmp	ah,32
	jnz	notmapup2
	sub	mapy,20
	mov	nowinnewroom,1
	ret

notmapup2:	cmp	ah,16
	jnz	notmapupspec
	sub	mapy,10
	mov	nowinnewroom,1
	ret	        

notmapupspec:	cmp	ah,8
	jnz	notmapdownspec
	add	mapy,10
	mov	nowinnewroom,1
	ret

notmapdownspec:	cmp	ah,2
	jnz	notmaprightspec
	add	mapx,11
	mov	nowinnewroom,1
	ret

notmaprightspec:	sub	mapx,11
	mov	nowinnewroom,1
	ret

	endp




Getreelstart	proc	near

	mov	ax,reelpointer
	mov	cx,40
	mul	cx
	mov	es,reels
	mov	si,ax
	add	si,reellist
	ret

	endp





;------------------------------------------------------Printing a reel frame----



Showreelframe	proc	near

	mov	al,[es:si+2]
	mov	ah,0
	mov	di,ax
	add	di,mapadx
	mov	al,[es:si+3]
	mov	bx,ax
	add	bx,mapady
	mov	ax,[es:si]
	mov	currentframe,ax
	call	findsource
	mov	ax,currentframe
	sub	ax,takeoff
	mov	ah,8
	call	showframe
	ret

	endp






;-------------------------------------------------------------------------------









Deleverything	proc	near

	mov	al,mapysize
	mov	ah,0
	add	ax,mapoffsety
	cmp	ax,182
	jnc	bigroom
	call	maptopanel
	ret
bigroom:	sub	mapysize,8
	call	maptopanel
	add	mapysize,8
	ret
	
	endp











Dumpeverything	proc	near

	mov	es,buffers
	mov	bx,printedlist
dumpevery1:	mov	ax,[es:bx]
	mov	cx,[es:bx+2]
	cmp	ax,0ffffh
	jz	finishevery1
	cmp	ax,[es:bx+(40*5)]
	jnz	notskip1
	cmp	cx,[es:bx+(40*5)+2]
	jz	skip1

notskip1:	push	bx es ds
	mov	bl,ah
	mov	bh,0
	mov	ah,0
	mov	di,ax
	add	di,mapadx
	add	bx,mapady
	call	multidump
	pop	ds es bx

skip1:	add	bx,5
	jmp	dumpevery1

finishevery1:	mov	bx,printedlist+(40*5)
dumpevery2:	mov	ax,[es:bx]
	mov	cx,[es:bx+2]
	cmp	ax,0ffffh
	jz	finishevery2

	push	bx es ds
	mov	bl,ah
	mov	bh,0
	mov	ah,0
	mov	di,ax
	add	di,mapadx
	add	bx,mapady
	call	multidump
	pop	ds es bx
	add	bx,5
	jmp	dumpevery2

finishevery2:	ret

	endp
