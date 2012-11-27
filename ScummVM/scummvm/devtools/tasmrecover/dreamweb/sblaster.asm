;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text
; Creative Reality Sound Blaster Drivers . (C) 1994 Creative Reality

; Very sparsly commented.
 


;These drivers are not stand alone. We had them as an integral part of the
;game.
;
;Put interupt no. into SOUNDINT, base address (eg 220h) into SOUNDBASEADD.
;If interupt is 255 then no card is assumed.
;
;Call soundstartup at beginning of program to test card and initialise.
;
;This code assumes that EMS has been initialised
;Emm page frame is in variable EMMPAGEFRAME. Handle is in EMMHANDLE.
;
;Call loadsample with a filename in CS:DX (ie. in the code somewhere)
;
;To play a sample call playchannel0 or playchannel1 with sound no. in al.
;
;Call endsample to restore interupts and halt sound.
;
;




;------------------------------------------- Initial sound set up and end ---

Loadspeech	proc	near
	
	cmp	soundint,255
	jz	dontbother8

	call	cancelch1

	mov	speechloaded,0
	call	createname
	
	mov	speechlength,0
	mov	dx,offset cs:speechfilename
	call	openfilenocheck
	jc	dontbother8

	mov	bx,speechemmpage

moreloadspeech:	push	dx bx
	
	push	es di bx
	mov	al,2
	mov	dx,emmhandle
	mov	ah,44h
	int	67h
	cmp	ah,0
	jnz	emmerror
	mov	ds,emmpageframe
	pop	bx di es
 	inc	bx
	push	es di
	mov	al,3
	mov	dx,emmhandle
	mov	ah,44h
	int	67h
	cmp	ah,0
	jnz	emmerror
	mov	ds,emmpageframe
	mov	es,emmpageframe
	mov	di,8000h
	mov	cx,4000h
	mov	ax,0
	rep	stosw
	pop	di es 

	mov	cx,8000h
	mov	dx,8000h
	call	readfromfile
	mov	cl,11
	shr	ax,cl
	add	speechlength,ax
	pop	bx dx
	add	bx,2
	cmp	ax,0 	
	jnz	moreloadspeech
	call	closefile

	mov	es,sounddata2
	mov	di,50*6
	mov	ax,speechemmpage
	mov	[es:di],al
	mov	ax,0
	mov	[es:di+1],ax
	mov	ax,speechlength
	mov	[es:di+3],ax
	mov	speechloaded,1
dontbother8:	ret

speechfilename:	db	"SPEECH\"
speechfile:	db	"R24C0005.RAW",0

	endp



Createname	proc	near

	push	ax
	mov	di,offset cs:speechfile
	mov	byte ptr [cs:di+0],dl ;"R"
	mov	[cs:di+3],cl
	
	mov	al,dh ;reallocation
	mov	ah,"0"-1
findten:	inc	ah
	sub	al,10
	jnc	findten
	mov	[cs:di+1],ah
	add	al,10+"0"
	mov	[cs:di+2],al
	pop	ax
	
	mov	cl,"0"-1
thousandsc:	inc	cl
	sub	ax,1000
	jnc	thousandsc
	add	ax,1000
	mov	[cs:di+4],cl
	mov	cl,"0"-1
hundredsc:	inc	cl
	sub	ax,100
	jnc	hundredsc
	add	ax,100
	mov	[cs:di+5],cl
	mov	cl,"0"-1
tensc:	inc	cl
	sub	ax,10
	jnc	tensc
	add	ax,10
	mov	[cs:di+6],cl
	add	al,"0"
	mov	[cs:di+7],al
	ret

	endp






Loadsample	proc	near

	cmp	soundint,255
	jz	dontbother
      
	call	openfile
	call	readheader
	mov	bx,[es:di]
	push	es di bx
	mov	ds,sounddata
	pop	cx
	mov	dx,0
	call	readfromfile
	pop	di es

	add	di,2
	mov	bx,0
	mov	dx,[es:di]
	add	dx,1
	shr	dx,1
	
	mov	soundemmpage,0

moreload:	push	dx bx
	
	push	es di bx
	mov	al,2
	mov	dx,emmhandle
	mov	ah,44h
	int	67h
	cmp	ah,0
	jnz	emmerror
	mov	ds,emmpageframe
	pop	bx di es
 	inc	bx
	push	es di
	mov	al,3
	mov	dx,emmhandle
	mov	ah,44h
	int	67h
	cmp	ah,0
	jnz	emmerror
	mov	ds,emmpageframe
	pop	di es 

	mov	cx,8000h
	mov	dx,8000h
	call	readfromfile
	pop	bx dx
	add	bx,2
	add	soundemmpage,2
	dec	dx	
	jnz	moreload
	;inc	soundemmpage
	call	closefile
dontbother:	ret

emmerror:	mov	gameerror,7
	jmp	quickquit2

	endp






Loadsecondsample	proc	near

	cmp	soundint,255
	jz	dontbother9

	cmp	ch0playing,12
	jc	ch0oksecond
	cmp	ch0playing,255
	jz	ch0oksecond
	call	cancelch0
	;mov	cx,100
           	;call	hangon
	jmp	ch0oksecond
justcancel:	call	cancelch0
ch0oksecond:	cmp	ch1playing,12
	jc	ch1oksecond
            	call	cancelch1

ch1oksecond:	call	openfile
	call	readheader
	mov	bx,[es:di]
	push	es di bx
	mov	ds,sounddata2
	pop	cx
	mov	dx,0
	call	readfromfile

	mov	cx,100
	mov	di,0
	mov	es,sounddata2
	mov	bx,soundemmpage
adjustemmpage:	mov	al,[es:di]
	add	al,bl
	mov	[es:di],al
	add	di,6
	loop	adjustemmpage

	pop	di es

	add	di,2
	mov	bx,soundemmpage
	mov	speechemmpage,bx
	mov	dx,[es:di]
	add	dx,1
	shr	dx,1

moreload2:	push	dx bx
	
	push	es di bx
	mov	al,2
	mov	dx,emmhandle
	mov	ah,44h
	int	67h
	cmp	ah,0
	jnz	emmerror2
	mov	ds,emmpageframe
	pop	bx di es
 	inc	bx
	push	es di
	mov	al,3
	mov	dx,emmhandle
	mov	ah,44h
	int	67h
	cmp	ah,0
	jnz	emmerror2
	mov	ds,emmpageframe
	pop	di es 

	mov	cx,8000h
	mov	dx,8000h
	call	readfromfile
	pop	bx dx
	add	bx,2
	add	speechemmpage,2
	dec	dx	
	jnz	moreload2
	call	closefile
dontbother9:	ret

emmerror2:	mov	gameerror,7
	jmp	quickquit2

	endp






Soundstartup	proc	near

	cmp	soundint,255
	jz	dontbother2

	mov	dx,soundbaseadd
	add	dx,0eh
	mov	DSP_status,dx
	mov	dx,soundbaseadd
	add	dx,0ch
	mov	DSP_write,dx
	
	mov	al,1
	mov	dx,soundbaseadd
	add	dx,0006h
	out	dx,al
	push	ax ax ax ax ax ax ax ax
	pop	ax ax ax ax ax ax ax ax
	mov	al,0
	out	dx,al

	mov	dx,DSP_status
	mov	cx,2000
waitinit:	in	al,dx
	and	al,128
	jz	waitinit
	mov	dx,soundbaseadd
	add	dx,000ah
	in	al,dx
	cmp	al,0aah
	jz	dspready
	loop	waitinit
	mov	gameerror,2
	jmp	quickquit

dspready:	call	trysoundalloc

	cli
	mov	ah,40h	;set sample rate
	call	out22c
	mov	ah,210	;of 22050Hz
	call	out22c
	sti

	call	checksoundint

	mov	ah,35h
	mov	al,soundint
	add	al,8
	int	21h
	mov	oldsoundintseg,es	; Save es:bx to temp memory
	mov	oldsoundintadd,bx
	push	cs
	pop	ds
	mov	dx,offset cs:dmaend
	mov	ah,25h
	mov	al,soundint
	add	al,8
	int	21h	; Set to new

	call	enablesoundint
	
	mov	al,sounddmachannel
	xor	ah,ah
	mov	bx,offset cs:dmaaddresses
	add	bx,ax
	mov	al,[cs:bx]
	mov	dmaaddress,al

	mov	ah,0d1h	;speaker on
	call	out22c
	mov	ah,0d0h
	call	out22c

dontbother2:	ret

dmaaddresses	db	87h,83h,81h,82h

	endp





Trysoundalloc	proc	near

	cmp	needsoundbuff,1
	jz	gotsoundbuff
	inc	soundtimes
	mov	bx,(16384+2048)/16
	call	allocatemem
	mov	soundbuffer,ax
	push	ax
	mov	al,ah
	mov	cl,4
	shr	al,cl
	mov	soundbufferpage,al
	pop	ax
	mov	cl,4
	shl	ax,cl
	mov	soundbufferad,ax
	cmp	ax,0b7ffh
	jnc	soundfail

	mov	es,soundbuffer
	mov	di,0
	mov	cx,16384/2
	mov	ax,7f7fh
	rep	stosw
	mov	needsoundbuff,1
	ret

soundfail:	mov	es,soundbuffer
	call	deallocatemem
gotsoundbuff:    	ret

	endp






Setsoundoff	proc	near

	cmp	soundint,255
	jz	dontbother28
	mov	soundbufferwrite,0
	cli
	call	setupPIT
	mov	soundbufferwrite,4096
	call	startdmablock
	sti
dontbother28:	ret	        

	endp






Checksoundint	proc	near
	
	mov	ah,0d3h	;speaker off
	call	out22c

	mov	testresult,0
	mov	ah,35h
	mov	al,soundint
	add	al,8
	int	21h
	mov	oldsoundintseg,es
	mov	oldsoundintadd,bx
	push	cs
	pop	ds
	mov	dx,offset cs:interupttest
	mov	ah,25h
	mov	al,soundint
	add	al,8
	int	21h

	call	enablesoundint
	
	mov	ah,0f2h
	call	out22c

	mov	cx,20
	call	hangon

	call	disablesoundint

	mov	dx,oldsoundintseg	
	mov	ds,dx
	mov	dx,oldsoundintadd	;Restore old interupt vector
	mov	ah,25h
	mov	al,soundint
	add	al,8
	int	21h

	cmp	testresult,1
	jz	interuptworked
	mov	gameerror,6	;interupt wrong
	jmp	quickquit	;exit to DOS with error

interuptworked:	ret

	endp





Enablesoundint	proc	near

	mov	dx,21h	; Enable int?
	in	al,dx
	mov	currentirq,al
	mov	ah,11111110b
	mov	cl,soundint
	rol	ah,cl
	and	al,ah
	out	dx,al
	ret

	endp





Disablesoundint	proc	near

	mov	al,soundint
	mov	dx,21h
	mov	al,currentirq
	out	dx,al
	ret

	endp




Interupttest	proc	near

	cli
	push	ax dx
	mov	testresult,1
	mov	dx,DSP_status
	in	al,dx
	mov	al,20h
	out	20h,al
	pop	dx ax
	iret
	
	endp





Soundend	proc	near

	cmp	soundint,255
	jz	dontbother3

	call	getridofPIT

	mov	ah,0d0h
	call	out22c
	
	call	disablesoundint

	mov	ds,oldsoundintseg		;for keys
	mov	dx,oldsoundintadd		;Restore old interupt vector
	mov	ah,25h
	mov	al,soundint
	add	al,8
	int	21h

dontbother3:	ret

	endp





Out22c	proc	near

    	mov	dx,DSP_write
notclear:	in	al,dx
	or	al,al
	js	notclear
	mov	al,ah
	out	dx,al
	ret

	endp





;---------------------------------------------------------------------------




Playchannel0	proc	near	;al=sound no
			;ah=times to repeat
	cmp	soundint,255
	jz	dontbother4
	
	push	es ds bx cx di si

	mov	ch0playing,al
	mov	es,sounddata
	cmp	al,12
	jc	notsecondbank
	mov	es,sounddata2
	sub	al,12
notsecondbank:	mov	ch0repeat,ah
	mov	ah,0
	add	ax,ax
	mov	bx,ax
	add	ax,ax
	add	bx,ax

	mov	al,[es:bx]
	mov	ah,0
	mov	ch0emmpage,ax
	mov	ax,[es:bx+1]
	mov	ch0offset,ax
	mov	ax,[es:bx+3]
	mov	ch0blockstocopy,ax
	
	cmp	ch0repeat,0
	jz	nosetloop
	mov	ax,ch0emmpage
	mov	ch0oldemmpage,ax
	mov	ax,ch0offset
	mov	ch0oldoffset,ax
	mov	ax,ch0blockstocopy
	mov	ch0oldblockstocopy,ax

nosetloop:	pop	si di cx bx ds es

dontbother4:	ret

	endp







Playchannel1	proc	near	;al=sound no

	cmp	soundint,255
	jz	dontbother5
	cmp	ch1playing,7
	jz	dontbother5
                  	push	es ds bx cx di si

	mov	ch1playing,al
	mov	es,sounddata
	cmp	al,12
	jc	notsecondbank1
	mov	es,sounddata2
	sub	al,12
notsecondbank1:	mov	ah,0
	add	ax,ax
	mov	bx,ax
	add	ax,ax
	add	bx,ax

	mov	al,[es:bx]
	mov	ah,0
	mov	ch1emmpage,ax
	mov	ax,[es:bx+1]
	mov	ch1offset,ax
	mov	ax,[es:bx+3]
	mov	ch1blockstocopy,ax
	
	pop	si di cx bx ds es

dontbother5:	ret

	endp








Makenextblock	proc	near
		              
	call	volumeadjust

	call	loopchannel0
	cmp	ch1blockstocopy,0
	jz	mightbeonlych0
	cmp	ch0blockstocopy,0
	jz	mightbeonlych1

	dec	ch0blockstocopy
	dec	ch1blockstocopy
	call	bothchannels
	ret

mightbeonlych1:	mov	ch0playing,255
	cmp	ch1blockstocopy,0
	jz	notch1only
	dec	ch1blockstocopy
	call	channel1only
notch1only:	ret

mightbeonlych0:	mov	ch1playing,255
	cmp	ch0blockstocopy,0
	jz	notch0only
	dec	ch0blockstocopy
	call	channel0only
	ret
notch0only:	mov	es,soundbuffer
	mov	di,soundbufferwrite
	mov	cx,1024
	mov	ax,7f7fh
	rep	stosw
	and	di,16384-1
	mov	soundbufferwrite,di
	ret
		
	endp




Volumeadjust	proc	near

	mov	al,volumedirection
	cmp	al,0
	jz	volok
	mov	al,volume
	cmp	al,volumeto
	jz	volfinish
	add	volumecount,64 
	jnz	volok
	mov	al,volume
	add	al,volumedirection
	mov	volume,al
	ret
volfinish:	mov	volumedirection,0
volok:	ret

	endp



Loopchannel0	proc	near

	cmp	ch0blockstocopy,0
	jnz	notloop
	cmp	ch0repeat,0
	jz	notloop
	cmp	ch0repeat,255
	jz	endlessloop
            	dec	ch0repeat
endlessloop:	mov	ax,ch0oldemmpage
	mov	ch0emmpage,ax
	mov	ax,ch0oldoffset
	mov	ch0offset,ax
	mov	ax,ch0blockstocopy
	add	ax,ch0oldblockstocopy
	mov	ch0blockstocopy,ax
	ret		
notloop:	ret

	endp







Cancelch0	proc	near

	mov	ch0repeat,0
	mov	ch0blockstocopy,0
	mov	ch0playing,255
	ret

	endp



Cancelch1	proc	near

	mov	ch1blockstocopy,0
	mov	ch1playing,255
	ret

	endp




Channel0only	proc	near

	call	saveems
	mov	al,0
	mov	bx,ch0emmpage
	mov	dx,emmhandle
	mov	ah,44h
	int	67h
	
	mov	es,soundbuffer
	mov	ds,emmpageframe
	mov	di,soundbufferwrite
	mov	si,ch0offset
	
	call	channel0tran
	call	restoreems

	and	di,16384-1
	mov	soundbufferwrite,di
           	and	si,16384-1
	mov	ch0offset,si
	cmp	si,0
	jnz	notch0endofpage0
	inc	ch0emmpage
notch0endofpage0:	ret

	endp




Channel1only	proc	near

	call	saveems
	mov	al,1
	mov	bx,ch1emmpage
	mov	dx,emmhandle
	mov	ah,44h
	int	67h
	
	mov	es,soundbuffer
	mov	ds,emmpageframe
	mov	di,soundbufferwrite
	mov	si,ch1offset
	add	si,16384

	mov	cx,1024
	rep	movsw
	call	restoreems

	and	di,16384-1
	mov	soundbufferwrite,di
           	and	si,16384-1
	mov	ch1offset,si
	cmp	si,0
	jnz	notch1endofpage1
	inc	ch1emmpage
notch1endofpage1:	ret

	endp





Channel0tran	proc	near

	cmp	volume,0
	jnz	lowvolumetran
	mov	cx,1024
	rep	movsw
	ret

lowvolumetran:	mov	cx,1024
	mov	bh,volume
	mov	bl,0
	add	bx,16384-256
volloop:	lodsw
	mov	bl,al
	mov	al,[es:bx]
	mov	bl,ah
	mov	ah,[es:bx]
	stosw
	loop	volloop
	ret
	

	endp








Bothchannels	proc	near 	;rather slow routine
			;to mix two channels

	call	saveems
	mov	al,0
	mov	bx,ch0emmpage
	mov	dx,emmhandle
	mov	ah,44h
	int	67h
	mov	al,1
	mov	bx,ch1emmpage
	mov	dx,emmhandle
	mov	ah,44h
	int	67h

	mov	es,soundbuffer
	mov	ds,emmpageframe
	mov	di,soundbufferwrite
	mov	si,ch0offset
	mov	bx,ch1offset
	add	bx,16384
	mov	cx,2048
	mov	dh,128
	mov	dl,255

	call	domix
	call	restoreems
	
	and	di,16384-1
	mov	soundbufferwrite,di

	mov	si,ch0offset
	add	si,2048
           	and	si,16384-1
	mov	ch0offset,si
	cmp	si,0
	jnz	notbothendofpage0
	inc	ch0emmpage
notbothendofpage0:	mov	si,ch1offset
	add	si,2048
           	and	si,16384-1
	mov	ch1offset,si
	cmp	si,0
	jnz	notbothendofpage1
	inc	ch1emmpage
notbothendofpage1:	ret

	endp



Saveems	proc	near

	mov	ah,4eh
	mov	al,0
	mov	es,soundbuffer
	mov	di,16384+2048-256
	int	67h
	ret

	endp


Restoreems	proc	near

	push	si di
	mov	ah,4eh
	mov	al,1
	mov	ds,soundbuffer
	mov	si,16384+2048-256
	int	67h
	pop	di si
	ret

	endp



Domix	proc	near

	cmp	volume,0
	jnz	lowvolumemix

slow:	lodsb
	mov	ah,[bx]
	inc	bx       
	cmp	al,dh
	jnc	toplot

botlot:	cmp	ah,dh
	jnc	nodistort
  	add	al,ah
	js	botok
	xor	al,al
	stosb
    	loop	slow	
     	jmp	doneit
botok:	xor	al,dh
   	stosb
    	loop	slow	
     	jmp	doneit

toplot:	cmp	ah,dh
	jc	nodistort
  	add	al,ah
	jns	topok
	mov	al,dl
	stosb
    	loop	slow	
     	jmp	doneit
topok:	xor	al,dh
   	stosb
    	loop	slow	
     	jmp	doneit
	
nodistort:	add	al,ah
	xor	al,dh
	stosb
	loop	slow
	jmp	doneit


lowvolumemix:	lodsb
	push	bx
	mov	bh,volume
	add	bh,63
	mov	bl,al
	mov	al,[es:bx]
	pop	bx

	mov	ah,[bx]
	inc	bx       
	cmp	al,dh
	jnc	toplotv

botlotv:	cmp	ah,dh
	jnc	nodistortv
  	add	al,ah
	js	botokv
	xor	al,al
	stosb
    	loop	lowvolumemix	
     	jmp	doneit
botokv:	xor	al,dh
   	stosb
    	loop	lowvolumemix	
     	jmp	doneit

toplotv:	cmp	ah,dh
	jc	nodistortv
  	add	al,ah
	jns	topokv
	mov	al,dl
	stosb
    	loop	lowvolumemix	
     	jmp	doneit
topokv:	xor	al,dh
   	stosb
    	loop	lowvolumemix	
     	jmp	doneit
	
nodistortv:	add	al,ah
	xor	al,dh
	stosb
	loop	lowvolumemix
doneit:	ret


	endp







Dmaend	proc	near
	
	cli
	push	ax cx dx
	call	startdmablock
	mov	dx,DSP_status
	in	al,dx
	mov	al,20h
	out	20h,al
	pop	dx cx ax
	iret

	endp
















Startdmablock	proc	near

	mov	al,sounddmachannel	;cx=length
	or	al,4	;bx=offset
	out	0ah,al
	xor	al,al
	out	0ch,al
	
	mov	al,48h
	or	al,sounddmachannel
	out	0bh,al

	mov	cx,soundbufferad
	xor	dh,dh
	mov	dl,sounddmachannel
	shl	dl,1
	mov	al,cl
	out	dx,al
	mov	al,ch
	out	dx,al

	mov	dl,dmaaddress
	mov	al,soundbufferpage	;hardware page
	out	dx,al

	mov	dl,sounddmachannel
	shl	dl,1
	inc	dl
	mov	cx,16384-1
	mov	al,cl
	out	dx,al
	mov	al,ch
	out	dx,al

	mov	al,sounddmachannel
	out	0ah,al	;dmac programmed
	
    	mov	dx,DSP_write
notclear1:	in	al,dx
	or	al,al
	js	notclear1
	mov	al,14h
	out	dx,al
notclear2:	in	al,dx
	or	al,al
	js	notclear2
	mov	al,cl
	out	dx,al
notclear3:	in	al,dx
	or	al,al
	js	notclear3
	mov	al,ch
	out	dx,al

	ret


	endp















SetupPIT	proc	near

	mov	ah,35h
	mov	al,8
	int	21h
	mov	oldint8seg,es		; Save es:bx to temp memory
	mov	oldint8add,bx
	push	cs
	pop	ds
	mov	dx,offset cs:PITinterupt
	mov	ah,25h
	mov	al,8
	int	21h		; Set to new
	
	mov	al,34h
	out	43h,al
	mov	al,0h
	out	40h,al
	mov	al,0dah
	out	40h,al
	ret

	endp






Getridofpit	proc	near

	cmp	oldint8seg,-1
	jz   	noresetPIT
	mov	dx,oldint8add
	mov	ax,oldint8seg		
	mov	ds,ax			
	mov	ah,25h
	mov	al,8
	int	21h
	mov	al,34h
	out	43h,al
	mov	al,0
	out	40h,al
	mov	al,0
	out	40h,al
noresetPIT:	ret

	endp






PITinterupt	proc	near

	cli
	push	ax dx cx
	
	xor	dh,dh
	mov	dl,sounddmachannel
	shl	dl,1
	in	al,dx
	mov	cl,al
	in	al,dx
	mov	ch,al
	sub	cx,soundbufferad
	mov	ax,soundbufferwrite
	sub	ax,cx
	and	ax,3fffh
	sti
	cmp	ax,8192
	jnc	mustgo
	cmp	ax,2048
	jnc	nopitflip
	
mustgo:	push	bx si di es ds
	call	makenextblock
	pop	ds es di si bx

nopitflip:	cli
	mov	al,20h
	out	20h,al
	pop	cx dx ax
	iret

	endp
