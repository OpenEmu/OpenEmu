;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text
;----------------------------------------------Code to draw floor and panel----

Blockget	proc	near

	mov	ah,al
	mov	al,0
	mov	ds,backdrop
	mov	si,blocks
	add	si,ax
	ret

	endp
















;--------------------------------------------------------Background display----








Drawfloor	proc	near

	push	es bx      	;in case this was called during
	call	eraseoldobs	;some sprite update.
	call	drawflags
	call	calcmapad
	call	doblocks
	call	showallobs
	call	showallfree
	call	showallex
	call	paneltomap
	call	initrain
	mov	newobs,0
	pop	bx es
	ret

	endp


















Calcmapad	proc	near

	call	getdimension

	push	cx dx

	mov	al,11
	sub	al,dl
	sub	al,cl
	sub	al,cl
	cbw
	mov	bx,8
	mul	bx
	add	ax,mapoffsetx
	mov	mapadx,ax
	pop	dx cx

	mov	al,10
	sub	al,dh
	sub	al,ch
	sub	al,ch
	cbw
	mov	bx,8
	mul	bx
	add	ax,mapoffsety
	mov	mapady,ax
	ret

	endp









Getdimension	proc	near	;Routine finds width, length
			;and top corner of room
	
	mov	es,buffers
	mov	bx,mapflags
	mov	ch,0
dimloop1:	call	addalong
	cmp	al,0
	jnz	finishdim1
	inc	ch
	jmp	dimloop1	;ch holds y of top corner

finishdim1:	mov	bx,mapflags
	mov	cl,0
dimloop2:	push	bx
	call	addlength
	pop	bx
	cmp	al,0
	jnz	finishdim2
	inc	cl
	add	bx,3
	jmp	dimloop2	;cl holds x of top corner

finishdim2:	mov	bx,mapflags+(11*3*9)
	mov	dh,10
dimloop3:	push	bx
	call	addalong
	pop	bx
	cmp	al,0
	jnz	finishdim3
	dec	dh
	sub	bx,11*3
	jmp	dimloop3	;dh holds y of bottom corner

finishdim3:	mov	bx,mapflags+(3*10)
	mov	dl,11
dimloop4:	push	bx
	call	addlength
	pop	bx
	cmp	al,0
	jnz	finishdim4
	dec	dl
	sub	bx,3
	jmp	dimloop4	;dl holds x of bottom corner

finishdim4:	mov	al,cl	;cl holds x start
	mov	ah,0
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	mov	mapxstart,ax
	mov	al,ch	;ch holds y start
	mov	ah,0
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	mov	mapystart,ax

	sub	dl,cl
	sub	dh,ch
				;dx holds x and y size of room
	mov	al,dl	;dl holds x size
	mov	ah,0
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	mov	mapxsize,al
	mov	al,dh	;dh holds y size
	mov	ah,0
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	mov	mapysize,al	;cx still holds top left corner
	ret

	endp












Addalong	proc	near

	mov	ah,11
addloop:	cmp	byte ptr [es:bx],0
	jnz	gotalong
	add	bx,3
	dec	ah
	jnz	addloop
	mov	al,0
	ret
gotalong:	mov	al,1
	ret

	endp





Addlength	proc	near

	mov	ah,10
addloop2:	cmp	byte ptr [es:bx],0
	jnz	gotlength
	add	bx,3*11
	dec	ah
	jnz	addloop2
	mov	al,0
	ret
gotlength:	mov	al,1
	ret

	endp




















Drawflags	proc	near

	mov	es,buffers
	mov	di,mapflags
	mov	al,mapy
	mov	ah,0
	mov	cx,mapwidth
	mul	cx
	mov	bl,mapx
	mov	bh,0
	add	ax,bx
	mov	si,map
	add	si,ax

	mov	cx,10
$28:	push	cx
	mov	cx,11
$28a:	mov	ds,mapdata
	lodsb
	mov	ds,backdrop
	push	si ax
	mov	ah,0
	add	ax,ax
	mov	si,flags
	add	si,ax
	movsw
	pop	ax
	stosb
	pop	si
	loop	$28a
	add	si,mapwidth-11
	pop	cx
	loop	$28
	ret

	endp














;-------------------------------------------------------Set object printing----

Eraseoldobs	proc	near

	cmp	newobs,0
	jz	donterase

	mov	es,buffers
	mov	bx,spritetable
	
	mov	cx,16
oberase:	push	cx bx
	mov	ax,[es:bx+20]
	cmp	ax,0ffffh
	jz	notthisob
	mov	di,bx
	mov	al,255
	mov	cx,tablesize
	rep	stosb
notthisob:	pop	bx cx
	add	bx,tablesize
	loop	oberase

donterase:	ret

	endp




















Showallobs	proc	near

	mov	es,buffers
	mov	bx,setlist
	mov	listpos,bx
	mov	di,bx
	mov	cx,128*5
	mov	al,255
	rep	stosb
		
	mov	es,setframes
	mov	frsegment,es
	mov	ax,framedata
	mov	dataad,ax
	mov	ax,frames
	mov	framesad,ax
	mov	currentob,0

	mov	ds,setdat
	mov	si,0
	
	mov	cx,128
showobsloop:	push	cx si

	push	si
	add	si,58
	mov	es,setdat
	call	getmapad
	pop	si
	cmp	ch,0
	jz	blankframe

	mov	al,[es:si+18]
	mov	ah,0
	mov	currentframe,ax
	cmp	al,255
	jz	blankframe

	push	es si
	call	calcfrframe
	call	finalframe
	pop	si es

	mov	al,[es:si+18]
	mov	[es:si+17],al
	cmp	byte ptr [es:si+8],0
	jnz	animating
	cmp	byte ptr [es:si+5],5
	jz	animating
	cmp	byte ptr [es:si+5],6
	jz	animating
	mov	ax,currentframe
	mov	ah,0
	add	di,mapadx
	add	bx,mapady
	call	showframe
	jmp	drawnsetob

animating:	call	makebackob

drawnsetob:	mov	si,listpos
	mov	es,buffers
	mov	al,savex
	mov	ah,savey
	mov	[es:si],ax
	mov	cx,ax
	mov	ax,savesize
	add	al,cl
	add	ah,ch
	mov	[es:si+2],ax
	mov	al,currentob
	mov	[es:si+4],al
	add	si,5
	mov	listpos,si

blankframe:	inc	currentob
	pop	si cx
	add	si,64
	dec	cx
	jz	finishedsetobs
	jmp	showobsloop

finishedsetobs: ret

	endp














Makebackob	proc	near

	cmp	newobs,0
	jz	nomake

	mov	al,[es:si+5]	 ; priority
	mov	ah,[es:si+8]	 ; type - steady, constant,random,door etc.
	push	si ax si
	mov	ax,objectx
	mov	bx,objecty
	mov	ah,bl
	mov	si,ax
	mov	cx,offset cs:backobject
	mov	dx,setframes
	mov	di,framedata
	call	makesprite
	pop	ax
	mov	[es:bx+20],ax
	pop	ax
	cmp	al,255
	jnz	usedpriority	 ; forgotten to specify priority
	mov	al,0
usedpriority:	mov	[es:bx+23],al
	mov	[es:bx+30],ah
	mov	byte ptr [es:bx+16],0
	mov	byte ptr [es:bx+18],0
	mov	byte ptr [es:bx+19],0
	pop	si
nomake: 	ret

	endp




;------------------------------------------------------Free object printing----

Showallfree	proc	near

	mov	es,buffers
	mov	bx,freelist
	mov	listpos,bx
	mov	di,freelist
	mov	cx,80*5
	mov	al,255
	rep	stosb

	mov	es,freeframes
	mov	frsegment,es
	mov	ax,frframedata
	mov	dataad,ax
	mov	ax,frframes
	mov	framesad,ax
	mov	al,0
	mov	currentfree,al

	mov	ds,freedat
	mov	si,2
	
	mov	cx,0
loop127:	push	cx si

	push	si
	mov	es,freedat
	call	getmapad
	pop	si
	cmp	ch,0
	jz	over138

	mov	al,currentfree
	mov	ah,0
	mov	dx,ax
	add	ax,ax
	add	ax,dx
	mov	currentframe,ax
		
	push	es si
	call	calcfrframe
	mov	es,mapstore
	mov	ds,frsegment
	call	finalframe
	pop	si es
	cmp	cx,0
	jz	over138

	mov	ax,currentframe
	mov	ah,0
	add	di,mapadx
	add	bx,mapady
	call	showframe
	mov	si,listpos
	mov	es,buffers
	mov	al,savex
	mov	ah,savey
	mov	[es:si],ax
	mov	cx,ax
	mov	ax,savesize
	add	al,cl
	add	ah,ch
	mov	[es:si+2],ax
	pop	ax cx
	push	cx ax
	mov	[es:si+4],cl
	add	si,5
	mov	listpos,si

over138:	inc	currentfree
	pop	si cx
	add	si,16
	inc	cx
	cmp	cx,80
	jz	finfree
	jmp	loop127

finfree:	ret

	endp












Showallex	proc	near

	mov	es,buffers
	mov	bx,exlist
	mov	listpos,bx
	mov	di,exlist
	mov	cx,100*5
	mov	al,255
	rep	stosb

	mov	es,extras
	mov	frsegment,es
	mov	ax,exframedata
	mov	dataad,ax
	mov	ax,exframes
	mov	framesad,ax
	mov	currentex,0

	mov	si,exdata+2
	
	mov	cx,0
exloop:	push	cx si

	mov	es,extras

	push	si
	mov	ch,0
 	cmp	byte ptr [es:si],255
	jz	notinroom
	mov	al,[es:si-2]
	cmp	al,reallocation
	jnz	notinroom
	call	getmapad
notinroom:	pop	si
	cmp	ch,0
	jz	blankex

	mov	al,currentex
	mov	ah,0
	mov	dx,ax
	add	ax,ax
	add	ax,dx
	mov	currentframe,ax
		
	push	es si
	call	calcfrframe
	mov	es,mapstore
	mov	ds,frsegment
	call	finalframe
	pop	si es
	cmp	cx,0
	jz	blankex

	mov	ax,currentframe
	mov	ah,0
	add	di,mapadx
	add	bx,mapady
	call	showframe
	mov	si,listpos
	mov	es,buffers
	mov	al,savex
	mov	ah,savey
	mov	[es:si],ax
	mov	cx,ax
	mov	ax,savesize
	add	al,cl
	add	ah,ch
	mov	[es:si+2],ax
	pop	ax cx
	push	cx ax
	mov	[es:si+4],cl
	add	si,5
	mov	listpos,si

blankex:	inc	currentex
	pop	si cx
	add	si,16
	inc	cx
	cmp	cx,100
	jz	finex
	jmp	exloop

finex:	ret

	endp







Calcfrframe	proc	near
	
	mov	dx,frsegment
	mov	ax,framesad
	push	ax
	mov	cx,dataad
	mov	ax,currentframe
	mov	ds,dx
	mov	bx,6
	mul	bx
	add	ax,cx
	mov	bx,ax
	mov	cx,[bx]
	mov	ax,[bx+2]
	mov	dx,[bx+4]
	pop	bx
	push	dx
	add	ax,bx		 ;ax=source add, cx=x,y
				;need this later
	mov	savesource,ax
	mov	savesize,cx
	pop	ax
	push	ax
	mov	ah,0
	mov	offsetx,ax
	pop	ax
	mov	al,ah
	mov	ah,0
	mov	offsety,ax
	ret
nullframe:	pop	ax
	mov	cx,0
	mov	savesize,cx
	ret

	endp


















Finalframe	proc	near
	
	mov	ax,objecty
	add	ax,offsety
	mov	bx,objectx
	add	bx,offsetx
	mov	savex,bl
	mov	savey,al
	mov	di,objectx
	mov	bx,objecty
	ret

	endp


	










Adjustlen	proc	near

	mov	ah,al
	add	al,ch
	cmp	al,100
	jc	over242
	mov	al,224
	sub	al,ch
	mov	ch,al
over242:	ret

	endp









Getmapad	proc	near

	call	getxad
	cmp	ch,0
	jz	over146
	mov	objectx,ax
	call	getyad
	cmp	ch,0
	jz	over146
	mov	objecty,ax
	mov	ch,1
over146:	ret
	
	endp









Getxad	proc	near

	mov	cl,[es:si]
	inc	si
	mov	al,[es:si]
	inc	si
	mov	ah,[es:si]
	inc	si
	cmp	cl,0
	jnz	over148
	sub	al,mapx
	jc	over148
	cmp	al,11
	jnc	over148
	mov	cl,4
	shl	al,cl
	or	al,ah
	mov	ah,0
	mov	ch,1
	ret
over148:	mov	ch,0
	ret

	endp









Getyad	proc	near

	mov	al,[es:si]
	inc	si
	mov	ah,[es:si]
	inc	si
	sub	al,mapy
	jc	over147
	cmp	al,10
	jnc	over147
	mov	cl,4
	shl	al,cl
	or	al,ah
	mov	ah,0
	mov	ch,1
	ret

over147:	mov	ch,0
	ret

	endp
