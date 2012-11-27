;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text
Printchar	proc	near

	cmp	al,255
	jz	ignoreit
	push	si bx di
	cmp	foreignrelease, 0
	jz $1
	sub	bx,3
$1:
	push	ax
	sub	al,32 ;"A"
	mov	ah,0
	add	ax,charshift
	call	showframe
	pop	ax di bx si
	cmp	kerning,0
	jnz	nokern
	call	kernchars
nokern:	push	cx
	mov	ch,0
	add	di,cx
	pop	cx
	;dec	di
ignoreit:	ret

	endp










Kernchars	proc	near

	;sub	al,13
	cmp	al,"a"
	jz	iskern
	cmp	al,"u"
	jz	iskern
	ret
iskern:	cmp	ah,"n"
	jz	kernit
	cmp	ah,"t"
	jz	kernit
	cmp	ah,"r"
	jz	kernit
	cmp	ah,"i"
	jz	kernit
	cmp	ah,"l"
	jz	kernit
	ret
kernit:	dec	cl
	ret

	endp






;------------------------------------------------Proportional text printing----


;Memprint	proc	near
;
;	call	usecharset1
;
;	push	ax bx cx dx es ds si di
;	call	deltextline
;	pop	di si ds es dx cx bx ax
;
;	pop	si
;	push	cs
;	pop	es
;	inc	si
;	mov	ds,currentset
;	mov	di,textaddressx
;	mov	bx,textaddressy
;	mov	dl,textlen
;	mov	al,0
;	mov	ah,0
;	call	printdirect
;	push	si

;	mov	newtextline,1
;	ret

;	endp






;------------------------------------------------Proportional text printing----

;Print	proc	near
;
;	pop	si
;	mov	bx,[cs:si+2]
;	mov	di,[cs:si+0]
;	mov	dl,[cs:si+4]
;	add	si,6
;	mov	ds,currentset
;
;printloop2:	push	bx di dx
;	push	es cs
;	pop	es
;	call	getnumber
;	pop	es
;	mov	ch,0
;printloop1:	mov	ax,[cs:si]  
;	inc	si
;	cmp	al,0
;	jz	finishprint
;	push	cx es
;	call	modifychar
;	call	printchar
;	pop	es cx
;	loop	printloop1
;	pop	dx di bx
;	add	bx,linespacing
;	jmp	printloop2

;finishprint:	pop	dx di bx
 ;	push	si
  ;	ret

   ;	endp









Printslow	proc	near

	mov	pointerframe,1
	mov	pointermode,3
	mov	ds,charset1
printloopslow6:	push	bx di dx
	call	getnumber

	mov	ch,0
printloopslow5:	push	cx si es
	mov	ax,[es:si]
	push	bx cx es si ds
	if	foreign
	call	modifychar
	endif
	call	printboth
	pop	ds si es cx bx
	mov	ax,[es:si+1]
	inc	si
	cmp	al,0
	jz	finishslow
	cmp	al,":"
	jz	finishslow
	cmp	cl,1
	jz	afterslow
	push	di ds bx cx es si
	if	foreign
	call	modifychar
	endif
	mov	charshift,91
	call	printboth
	mov	charshift,0
	pop	si es cx bx ds di
	call	waitframes
	cmp	ax,0
	jz	keepgoing
	cmp	ax,oldbutton
	jnz	finishslow2
keepgoing:	call	waitframes
noslow:	cmp	ax,0
	jz	afterslow
	cmp	ax,oldbutton
	jnz	finishslow2		;used to finish early
afterslow:	pop	es si cx
	inc	si
	loop	printloopslow5

	pop	dx di bx
	add	bx,10
	jmp	printloopslow6

finishslow:	pop	es si cx dx di bx
	mov	al,0
	ret

finishslow2:	pop	es si cx dx di bx
	mov	al,1
	ret

	endp



Waitframes	proc	near

	push	di bx es si ds
	call	readmouse
	call	showpointer
	call	vsync
	call	dumppointer
	call	delpointer
	mov	ax,mousebutton
	pop	ds si es bx di
	ret

	endp




Printboth	proc	near

	push	ax cx bx
	push	di
	call	printchar
	pop	ax
	push	di
	mov	di,ax
	call	multidump
	pop	di
	pop	bx cx ax
	ret

	endp






Printdirect	proc	near

	mov	lastxpos,di
	mov	ds,currentset
printloop6:	push	bx di dx 
	call	getnumber
	mov	ch,0
printloop5:	mov	ax,[es:si]
	inc	si
	cmp	al,0
	jz	finishdirct
	cmp	al,":"
	jz	finishdirct
	push	cx es
	if	foreign
	call	modifychar
	endif
	call	printchar
	mov	lastxpos,di
	pop	es cx
	loop	printloop5
	pop	dx di bx
	add	bx,linespacing
	jmp	printloop6

finishdirct:	pop	dx di bx
	ret

	endp














Monprint	proc	near

	mov	kerning,1
	mov	si,bx
	mov	dl,166
	mov	di,monadx
	mov	bx,monady
	mov	ds,tempcharset

printloop8:	push	bx di dx
	call	getnumber
	mov	ch,0
printloop7:	mov	al,[es:si]
	inc	si

	cmp	al,":"
	jz	finishmon2
	cmp	al,0
	jz	finishmon
	cmp	al,34
	jz	finishmon
	cmp	al,"="
	jz	finishmon
	cmp	al,"%"
	jnz	nottrigger
	mov	ah,[es:si]
	inc	si
	inc	si
	jmp	finishmon
nottrigger:	push	cx es
	if	foreign
	call	modifychar
	endif
	call	printchar
	mov	curslocx,di
	mov	curslocy,bx
	mov	maintimer,1
	call	printcurs

	call	vsync
	push	si dx ds es bx di
	call	lockmon
	pop	di bx es ds dx si
	call	delcurs
	pop	es cx
	loop	printloop7

finishmon2:	pop	dx di bx
	call	scrollmonitor
	mov	curslocx,di
	jmp	printloop8

finishmon:	pop	dx di bx
	cmp	al,"%"
	jnz	nottrigger2
	mov	lasttrigger,ah
nottrigger2:	mov	curslocx,di
	call	scrollmonitor
	mov	bx,si
	mov	kerning,0
	ret

	endp









Getnumber	proc	near

	mov	cx,0
	push	si bx di ds es
	mov	di,si

wordloop:	push	cx dx
	call	getnextword
	pop	dx cx
	cmp	al,1
	jz	endoftext
	mov	al,cl
	mov	ah,0
	push	bx
	mov	bh,0
	add	ax,bx
	pop	bx
	sub	ax,10
	mov	dh,0
	cmp	ax,dx
	jnc	gotoverend
	add	cl,bl
	add	ch,bh
	jmp	wordloop

gotoverend:	mov	al,dl
	and	al,1
	jz	notcentre
	push	cx
	mov	al,dl
	and	al,11111110b
	mov	ah,0
	mov	ch,0
	sub	ax,cx
	add	ax,20
	shr	ax,1
	pop	cx
	pop	es ds di bx si
	add	di,ax
	mov	cl,ch
	ret
notcentre:	pop	es ds di bx si
	mov	cl,ch
	ret



endoftext:	mov	al,cl
	mov	ah,0
	push	bx
	mov	bh,0
	add	ax,bx
	pop	bx
	sub	ax,10
	mov	dh,0
	cmp	ax,dx
	jnc	gotoverend2
	add	cl,bl
	add	ch,bh

gotoverend2:	mov	al,dl
	and	al,1
	jz	notcent2
	push	cx
	mov	al,dl
	and	al,11111110b
	add	al,2
	mov	ah,0
	mov	ch,0
	add	ax,20
	sub	ax,cx
	shr	ax,1
	pop	cx
	pop	es ds di bx si
	add	di,ax
	mov	cl,ch
	ret
notcent2:	pop	es ds di bx si
	mov	cl,ch
	ret

	endp





Getnextword	proc	near

	mov	bx,0
getloop:	mov	ax,[es:di]
	inc	di
	inc	bh
	cmp	al,":"
	jz	endall
	cmp	al,0
	jz	endall
	cmp	al,32
	jz	endword
	if	foreign
	call	modifychar
	endif
	cmp	al,255
	jz	getloop
	push	ax
	sub	al,32 ;"A"
	mov	ah,0
	add	ax,charshift
	add	ax,ax
	mov	si,ax
	add	ax,ax
	add	si,ax
	mov	cl,[si+0]
	pop	ax
	call	kernchars
	add	bl,cl
	;dec	bl
	jmp	getloop

endword:	add	bl,6
	mov	al,0
	ret

endall: 	add	bl,6
	mov	al,1
	ret

	endp





	if	german

Modifychar	proc	near

	cmp	al,128
	jc	nomod
	cmp	al,129
	jnz	not129
	mov	al,"Z"+3
	ret
not129:	cmp	al,132
	jnz	not132
	mov	al,"Z"+1
	ret
not132:	cmp	al,142
	jnz	not142
	mov	al,"Z"+4
	ret
not142:	cmp	al,154
	jnz	not154
	mov	al,"Z"+6
	ret
not154:	cmp	al,225
	jnz	not225
	mov	al,"A"-1
	ret
not225:	cmp	al,153
	jnz	not153
	mov	al,"Z"+5
	ret
not153:	cmp	al,148
	jnz	not148
	mov	al,"Z"+2
	ret
not148:	ret

nomod:	ret

	endp

	endif




	if	spanish

Modifychar	proc	near

	cmp	al,128
	jc	nomod
	cmp	al,160
	jnz	not160
	mov	al,"Z"+1
	ret
not160:	cmp	al,130
	jnz	not130
	mov	al,"Z"+2
	ret
not130:	cmp	al,161
	jnz	not161
	mov	al,"Z"+3
	ret
not161:	cmp	al,162
	jnz	not162
	mov	al,"Z"+4
	ret
not162:	cmp	al,163
	jnz	not163
	mov	al,"Z"+5
	ret
not163:	cmp	al,164
	jnz	not164
	mov	al,"Z"+6
	ret
not164:	cmp	al,165
	jnz	not165
	mov	al,","-1
	ret
not165:	cmp	al,168
	jnz	not168
	mov	al,"A"-1
	ret
not168:	cmp	al,173
	jnz	not173
	mov	al,"A"-4
	ret
not173:	cmp	al,129
	jnz	not129
	mov	al,"A"-5
not129:	ret

nomod:	ret
	
	endp

	endif
