;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text
;---------------------------------------------------------------Look-routine----

Autolook	proc	near

	mov	ax,mousex
	cmp	ax,oldx
	jnz	diffmouse
	mov	ax,mousey
	cmp	ax,oldy
	jnz	diffmouse

	dec	lookcounter
	cmp	lookcounter,0
	jnz	noautolook
	cmp	watchingtime,0
	jnz	noautolook
	call	dolook
noautolook:	ret

diffmouse:	mov	lookcounter,1000
	ret

	endp




Look	proc	near

	cmp	watchingtime,0
	jnz	blank
	cmp	pointermode,2
	jz	blank

	cmp	commandtype,241
	jz	alreadylook
	mov	commandtype,241
	mov	al,25
	call	commandonly
alreadylook:	cmp	mousebutton,1
	jnz	nolook
	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	nolook
	call	dolook
nolook:	ret

	endp





Dolook	proc	near

	call	createpanel
	call	showicon
	call	undertextline
	call	worktoscreenm

	mov	commandtype,255
	call	dumptextline

	mov	bl,roomnum
	and	bl,31
	mov	bh,0
	add	bx,bx

	mov	es,roomdesc
	add	bx,intextdat

	mov	si,[es:bx]
	add	si,intext

	call	findnextcolon

	mov	di,66
	cmp	reallocation,50
	jc	notdream3
	mov	di,40
notdream3:	mov	bx,80
	mov	dl,241
	call	printslow

	cmp	al,1
	jz	afterlook
	mov	cx,400
	call	hangonp

afterlook:	mov	pointermode,0
	mov	commandtype,0
	call	redrawmainscrn
	call	worktoscreenm
	ret

	endp






Redrawmainscrn	proc	near

	mov	timecount,0
	call	createpanel
	mov	newobs,0
	call	drawfloor
	call	printsprites
	call	reelsonscreen
	call	showicon
	call	getunderzoom
	call	undertextline
	call	readmouse
	mov	commandtype,255
	ret

	endp















Getback1	proc	near

	cmp	pickup,0
	jz	notgotobject
	call	blank
	ret

notgotobject:	cmp	commandtype,202
	jz	alreadyget
	mov	commandtype,202
	mov	al,26
	call	commandonly
alreadyget:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	nogetback
	and	ax,1
	jnz	dogetback
nogetback:	ret

dogetback:	mov	getback,1
	mov	pickup,0
	ret

	endp
