;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text
Talk	proc	near
	
	mov	talkpos,0
	mov	inmaparea,0
	mov	al,command
	mov	character,al
	call	createpanel
	call	showpanel
	call	showman
	call	showexit
	call	undertextline
	call	convicons
	call	starttalk
	mov	commandtype,255
	call	readmouse
	call	showpointer
	call	worktoscreen
waittalk:	call	delpointer
	call	readmouse
	call	animpointer
	call	showpointer
	call	vsync
	call	dumppointer
	call	dumptextline
	mov	getback,0
	mov	bx,offset cs:talklist
	call	checkcoords
	cmp quitrequested, 0
	jnz finishtalk
	cmp	getback,0
	jz	waittalk
finishtalk:	mov	bx,persondata
	push	cs
	pop	es
	cmp	talkpos,4
	jc	notnexttalk
	mov	al,[es:bx+7]
	or	al,128
	mov	[es:bx+7],al
notnexttalk:	call	redrawmainscrn
	call	worktoscreenm
	cmp	speechloaded,1
	jnz	nospeech
	call	cancelch1
	mov	volumedirection,-1  	;fade (louder)
	mov	volumeto,0	;up to 0 (max)
nospeech:	ret

talklist:	dw	273,320,157,198,getback1
	dw	240,290,2,44,moretalk
	dw	0,320,0,200,blank
	dw	0ffffh

	endp












Convicons	proc	near

	mov	al,character
	and	al,127
	call	getpersframe
	mov	di,234
	mov	bx,2
	mov	currentframe,ax
	call	findsource
	mov	ax,currentframe
	sub	ax,takeoff
	mov	ah,0
	call	showframe
	ret

	endp








Getpersframe	proc	near

	mov	ah,0
	add	ax,ax
	mov	bx,ax
	mov	es,people
	add	bx,personframes
	mov	ax,[es:bx]
	ret

	endp





Starttalk	proc	near

	mov	talkmode,0
	mov	al,character
	and	al,127
	call	getpersontext
	mov	charshift,91+91
	mov	di,66
	mov	bx,64
	mov	dl,241
	mov	al,0
	mov	ah,79
	call	printdirect
	mov	charshift,0
	mov	di,66
	mov	bx,80
	mov	dl,241
	mov	al,0
	mov	ah,0
	call	printdirect

	if	cd
	mov	speechloaded,0
	mov	al,character
	and	al,127
	mov	ah,0
	mov	cx,64
	mul	cx
	mov	cl,"C"
	mov	dl,"R"
	mov	dh,reallocation
	call	loadspeech
	cmp	speechloaded,1
	jnz	nospeech1
	mov	volumedirection,1 	;quieter
	mov	volumeto,6	;quite quiet!
	mov	al,50+12
	call	playchannel1
	endif
nospeech1:	ret

	endp





Getpersontext	proc	near

	mov	ah,0
	mov	cx,64*2
	mul	cx
	mov	si,ax
	mov	es,people
	add	si,persontxtdat
	mov	cx,persontext
	mov	ax,[es:si]
	add	ax,cx
	mov	si,ax
	ret

	endp







Moretalk	proc	near

	;cmp	ch1playing,255
	;jnz	cantredes
	cmp	talkmode,0
	jz	canmore
	call	redes
	ret

canmore:	cmp	commandtype,215
	jz	alreadymore
	mov	commandtype,215
	mov	al,49
	call	commandonly
alreadymore:	mov	ax,mousebutton
	cmp	ax,oldbutton
	jz	nomore
	and	ax,1
	jnz	domoretalk
nomore: 	ret

domoretalk:	mov	talkmode,2
	mov	talkpos,4
	cmp	character,100
	jc	notsecondpart
              	mov	talkpos,48
notsecondpart:	call	dosometalk
	ret

	endp














Dosometalk	proc	near

	if	cd
dospeech:	mov	al,talkpos
	mov	al,character
	and	al,127
	mov	ah,0
	mov	cx,64
	mul	cx
	mov	cx,ax
	mov	al,talkpos
	mov	ah,0
	add	ax,cx
	add	ax,ax
	mov	si,ax

	mov	es,people
	add	si,persontxtdat
	mov	cx,persontext

	mov	ax,[es:si]
	add	ax,cx
	mov	si,ax
	cmp	byte ptr [es:si],0
	jz	endheartalk
       
	push	es si
	call	createpanel
	call	showpanel
	call	showman
	call	showexit
	call	convicons
	pop	si es

	mov	di,164
	mov	bx,64
	mov	dl,144
	mov	al,0
	mov	ah,0
	call	printdirect

	mov	al,character
	and	al,127
	mov	ah,0
	mov	cx,64
	mul	cx
	mov	cl,talkpos
	mov	ch,0
	add	ax,cx
	mov	cl,"C"
	mov	dl,"R"
	mov	dh,reallocation
	call	loadspeech
	cmp	speechloaded,0
	jz	noplay1
	mov	al,62
	call	playchannel1

noplay1:	mov	pointermode,3
	call	worktoscreenm
	mov	cx,180
	call	hangonpq
	jnc $1
	ret

$1:
	inc	talkpos
          
	mov	al,talkpos
          	mov	al,character
	and	al,127
	mov	ah,0
	mov	cx,64
	mul	cx
	mov	cx,ax
	mov	al,talkpos
	mov	ah,0
	add	ax,cx
	add	ax,ax
	mov	si,ax

	mov	es,people
	add	si,persontxtdat
	mov	cx,persontext

	mov	ax,[es:si]
	add	ax,cx
	mov	si,ax
	cmp	byte ptr [es:si],0
	jz	endheartalk
	cmp	byte ptr [es:si],":"
	jz	skiptalk2
	cmp	byte ptr [es:si],32
	jz	skiptalk2
	
	push	es si
	call	createpanel
	call	showpanel
	call	showman
	call	showexit
	call	convicons
	pop	si es

	mov	di,48
	mov	bx,128
	mov	dl,144
	mov	al,0
	mov	ah,0
	call	printdirect

	mov	al,character
	and	al,127
	mov	ah,0
	mov	cx,64
	mul	cx
	mov	cl,talkpos
	mov	ch,0
	add	ax,cx
	mov	cl,"C"
	mov	dl,"R"
	mov	dh,reallocation
	call	loadspeech
	cmp	speechloaded,0
	jz	noplay2
	mov	al,62
	call	playchannel1

noplay2:	mov	pointermode,3
	call	worktoscreenm
	mov	cx,180
	call	hangonpq
	jnc skiptalk2
	ret

skiptalk2:	inc	talkpos
	jmp	dospeech

endheartalk:	mov	pointermode,0
	ret
	
	else

watchtalk:	mov	al,talkpos
	mov	al,character
	and	al,127
	mov	ah,0
	mov	cx,64
	mul	cx
	mov	cx,ax
	mov	al,talkpos
	mov	ah,0
	add	ax,cx
	add	ax,ax
	mov	si,ax

	mov	es,people
	add	si,persontxtdat
	mov	cx,persontext

	mov	ax,[es:si]
	add	ax,cx
	mov	si,ax
	cmp	byte ptr [es:si],0
	jz	endwatchtalk
	
	push	es si
	call	createpanel
	call	showpanel
	call	showman
	call	showexit
	call	convicons
	pop	si es

	mov	di,164
	mov	bx,64
	mov	dl,144
	mov	al,0
	mov	ah,0
	call	printdirect

	mov	pointermode,3
	call	worktoscreenm
	mov	cx,180
	call	hangonpq
	jnc $1
	ret
$1:

	inc	talkpos
          
	mov	al,talkpos
          	mov	al,character
	and	al,127
	mov	ah,0
	mov	cx,64
	mul	cx
	mov	cx,ax
	mov	al,talkpos
	mov	ah,0
	add	ax,cx
	add	ax,ax
	mov	si,ax

	mov	es,people
	add	si,persontxtdat
	mov	cx,persontext

	mov	ax,[es:si]
	add	ax,cx
	mov	si,ax
	cmp	byte ptr [es:si],0
	jz	endwatchtalk
	cmp	byte ptr [es:si],":"
	jz	skiptalk
	cmp	byte ptr [es:si],32
	jz	skiptalk
	
	push	es si
	call	createpanel
	call	showpanel
	call	showman
	call	showexit
	call	convicons
	pop	si es

	mov	di,48
	mov	bx,128
	mov	dl,144
	mov	al,0
	mov	ah,0
	call	printdirect

	mov	pointermode,3
	call	worktoscreenm
	mov	cx,180
	call	hangonpq
	jnc skiptalk
	ret

skiptalk:	inc	talkpos
	jmp	watchtalk

endwatchtalk:	mov	pointermode,0
	ret

	endif

	endp







Hangonpq 	proc	near

	mov	getback,0
	mov	bx,0
hangloopq:	push	cx bx
	call	delpointer
	call	readmouse
	call	animpointer
	call	showpointer
	call	vsync
	call	dumppointer
	call	dumptextline
	mov	bx,offset cs:quitlist
	call	checkcoords
	pop	bx cx
	cmp	getback,1
	jz	quitconv
	cmp quitrequested, 0
	jnz quitconv
	cmp	speechloaded,1
	jnz	notspeaking
            	cmp	ch1playing,255
	jnz	notspeaking
	inc	bx
	cmp	bx,40     	;pause after speech ends
	jz	finishconv
notspeaking:	cmp	mousebutton,0
	jz	hangloopq
	cmp	oldbutton,0
	jnz	hangloopq
finishconv:	call	delpointer
	mov	pointermode,0
	clc
	ret
	
quitconv:	call	delpointer
	mov	pointermode,0
	call	cancelch1
	stc
	ret

quitlist:	dw	273,320,157,198,getback1
	dw	0,320,0,200,blank
	dw	0ffffh

	endp









Redes	proc	near

	cmp	ch1playing,255
	jnz	cantredes
	cmp	talkmode,2
	jz	canredes
cantredes:	call	blank
	ret

canredes:	cmp	commandtype,217
	jz	alreadyreds
	mov	commandtype,217
	mov	al,50
	call	commandonly
alreadyreds:	mov	ax,mousebutton
	and	ax,1
	jnz	doredes
	ret

doredes:	call	delpointer
	call	createpanel
	call	showpanel
	call	showman
	call	showexit
	call	convicons
	call	starttalk
	call	readmouse
	call	showpointer
	call	worktoscreen
	call	delpointer
	ret

	endp
