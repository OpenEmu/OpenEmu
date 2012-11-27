;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text

	if	debuglevel2


Debugkeys	proc	near

	ret

	endp



Debugstart	proc	near

	call	allpalette
	mov	reeltohold,-1
	mov	newlocation,23
	mov	dreamnumber,0
	mov	al,"W"
	mov	ah,"S"
	mov	cl,"H"
	mov	ch,"D"
	call	findexobject
	mov	byte ptr [es:bx+12],"S"-"A"
	mov	byte ptr [es:bx+13],"C"-"A"
	mov	byte ptr [es:bx+14],"R"-"A"
	mov	byte ptr [es:bx+15],"W"-"A"
	mov	al,"W"
	mov	ah,"E"
	mov	cl,"T"
	mov	ch,"A"
	call	findexobject
	mov	byte ptr [es:bx+12],"G"-"A"
	mov	byte ptr [es:bx+13],"U"-"A"
	mov	byte ptr [es:bx+14],"N"-"A"
	mov	byte ptr [es:bx+15],"A"-"A"
	mov	al,"W"
	mov	ah,"E"
	mov	cl,"T"
	mov	ch,"B"
	call	findexobject
	mov	byte ptr [es:bx+12],"S"-"A"
	mov	byte ptr [es:bx+13],"H"-"A"
	mov	byte ptr [es:bx+14],"L"-"A"
	mov	byte ptr [es:bx+15],"D"-"A"
	mov	card1money,12342

	ret

	endp





Debuggreen	proc	near

	push	ax dx
	mov	al,0
	mov	dx,3c8h
	out	dx,al
	mov	dx,3c9h
	mov	al,0
	out	dx,al
	mov	al,63
	out	dx,al
	mov	al,0
	out	dx,al
	pop	dx ax
	ret
	
	endp








Debugred	proc	near

	push	ax dx
	mov	al,0
	mov	dx,3c8h
	out	dx,al
	mov	dx,3c9h
	mov	al,63
	out	dx,al
	mov	al,0
	out	dx,al
	mov	al,0
	out	dx,al
	pop	dx ax
	ret
	
	endp




Debugblue	proc	near

	push	ax dx
	mov	al,0
	mov	dx,3c8h
	out	dx,al
	mov	dx,3c9h
	mov	al,0
	out	dx,al
	mov	al,0
	out	dx,al
	mov	al,63
	out	dx,al
	pop	dx ax
	ret
	
	endp





Debugblack	proc	near

	push	dx ax
	mov	al,0
	mov	dx,3c8h
	out	dx,al
	mov	dx,3c9h
	mov	al,0
	out	dx,al
	mov	al,0
	out	dx,al
	mov	al,0
	out	dx,al
	pop	ax dx 
	ret
	
	endp











Debug	proc	near

	push	ds dx cx
	mov	ah,3ch
	mov	cx,0
	mov	dx,seg filenamed
	mov	ds,dx
	mov	dx,offset filenamed
	int	21h
	mov	bx,ax
	pop	cx dx ds
	push	bx
	mov	ah,40h
	int	21h
	pop	bx
	mov	ah,3eh
	int	21h
	ret

filenamed	db	"DREAMWEB.TXT",0

	endp








Shout	proc	near

	push	ax bx cx dx si di es ds
	call	debugblue
	mov	cx,50
	call	hangon
	call	debugblack
	mov	cx,10
	call	hangon
	pop	ds es di si dx cx bx ax
	ret

	endp


Shoutred	proc	near

	push	ax bx cx dx si di es ds
	call	debugred
	mov	cx,4
	call	hangon
	call	debugblack
	mov	cx,40
	call	hangon
	pop	ds es di si dx cx bx ax
	ret

	endp



Shoutgreen	proc	near

	push	ax bx cx dx si di es ds
	call	debuggreen
	mov	cx,4
	call	hangon
	call	debugblack
	mov	cx,40
	call	hangon
	pop	ds es di si dx cx bx ax
	ret

	endp









;Checkmemingame	proc	near

;	cmp	charset1,0
;	jz	nodebug
;	mov	bx,60000
;	mov	ah,48h
;	int	21h
;	mov	ax,bx
;	mov	cl,6
;	shr	ax,cl
;	mov	di,offset cs:debugtextig
;	call	showword

;	mov	ax,soundbufferwrite
;	;mov	ax,exframepos
;	mov	di,offset cs:debugtextex
;	call	showword

;	;mov	ax,extextpos
;	;mov	di,offset cs:debugtextex2
;	;call	showword

;	push	cs
;	pop	es
;	mov	si,offset cs:debugtextig
;	mov	al,0
;	mov	ah,0
;	mov	dl,100
;	mov	di,204
;	mov	bx,14
;	call	printdirect
;	push	cs
;	pop	es
;	mov	si,offset cs:debugtextex
;	mov	al,0
;	mov	ah,0
;	mov	dl,100
;	mov	di,204
;	mov	bx,22
;	call	printdirect
;	push	cs
;	pop	es
;	mov	si,offset cs:debugtextex2
;	mov	al,0
;	mov	ah,0
;	mov	dl,100
;	mov	di,204
;	mov	bx,30
;	call	printdirect
;	mov	di,204
;	mov	bx,14
;	mov	cl,40
;	mov	ch,24
;	call	multidump
;nodebug:	ret

	endp

debugtextig:	db	"00000K",0

debugtextex:	db	"00000b",0

debugtextex2:	db	"00000b",0

	




	if	recording

	mov	ax,recordpos
	mov	di,offset cs:debugtextr
	call	showword

	mov	al,0
	call	print
	dw	4,4,100
debugtextr:	db	"00000",0

	mov	si,0
	mov	di,0
	mov	cl,40
	mov	ch,12
	call	multidump

	endif

	ret

	endp


















;Debugax	proc	near
;
;	push	ax
;	call	showpanel
;	pop	ax
;	mov	di,offset cs:debugaxtext
;	call	showword
;	
;	mov	di,204
;	mov	bx,14
;	mov	al,0
;	mov	ah,0
;	mov	dl,100
;	push	cs
;	pop	es
;	mov	si,offset cs:debugaxtext
;	call	printdirect
;	mov	di,204
;	mov	bx,14
;	mov	cl,40
;	mov	ch,24
;	call	multidump
;	ret
;
;debugaxtext	db	"00000  ",0
;	
;	endp







	endif
