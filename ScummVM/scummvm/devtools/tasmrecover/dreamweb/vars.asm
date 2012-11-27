;Copyright (c) 1990-2012 by Neil Dodwell
;Released with permission from Neil Dodwell under GPLv2
;See LICENSE file for full license text
;---------------------------------------------------Equates and definitions----

Inputport	equ	63h
Mapwidth	equ	66 ;132/2
Maplength	equ	60 ;6/2
Tablesize	equ	32		;size of each entry in spritetable
Itempicsize	equ	44		;size of inventory slots
Opsy	equ	52
Opsx	equ	60
Inventx	equ	80
Inventy 	equ	58
Zoomx	equ	8
Zoomy	equ	132
Keypadx	equ	36+112
Keypady	equ	72
Diaryx	equ	68+24
Diaryy	equ	48+12
Symbolx	equ	64
Symboly	equ	56
Menux	equ	80+40
Menuy	equ	60

	if	foreign
Undertextsizex	equ	228
Undertextsizey	equ	13
Undertimedysize	equ	30
	else
Undertextsizex	equ	180
Undertextsizey	equ	10
Undertimedysize	equ	24
	endif

Numchanges	equ	250

Textunder	equ	0		;offsets for items in buffer segment
Openinvlist	equ	textunder+(undertextsizex*undertextsizey)
Ryaninvlist	equ	openinvlist+32
Pointerback	equ	ryaninvlist+60
Mapflags	equ	pointerback+(32*32)
Startpal	equ	mapflags+(11*10*3)
Endpal	equ	startpal+768
Maingamepal	equ	endpal+768
Spritetable	equ	maingamepal+768
Setlist 	equ	spritetable+(32*tablesize)
Freelist	equ	setlist+(128*5)
Exlist	equ	freelist+(80*5)
Peoplelist	equ	exlist+(100*5)
Zoomspace	equ	peoplelist+(12*5)
Printedlist	equ	zoomspace+(46*40)
Listofchanges	equ	printedlist+(5*80)
Undertimedtext	equ	listofchanges+(numchanges*4)
Rainlist	equ	undertimedtext+(256*undertimedysize)
Initialreelrouts	equ	rainlist+(6*64)
Initialvars	equ	initialreelrouts+lenofreelrouts
Lengthofbuffer	equ	initialvars+lengthofvars
		               
Flags	equ	0		;offsets of items in backdrop segment
Blocks	equ	flags+192
Map	equ	0
Lengthofmap	equ	map+(mapwidth*maplength)

Intextdat	equ	0
Intext	equ	intextdat+(38*2)
Blocktextdat	equ	0
Blocktext	equ	blocktextdat+(98*2)
Settextdat	equ	0
Settext 	equ	settextdat+(130*2)
Freetextdat	equ	0
Freetext	equ	freetextdat+(82*2)

Numexobjects	equ	114
Exframeslen	equ	30000
Extextlen	equ	18000

Exframedata	equ	0
Exframes	equ	exframedata+2080
Exdata	equ	exframes+exframeslen
Extextdat	equ	exdata+(16*numexobjects)
Extext	equ	extextdat+((numexobjects+2)*2)
Lengthofextra	equ	extext+extextlen

Framedata	equ	0
Frames	equ	framedata+2080

Frframedata	equ	0
Frframes	equ	frframedata+2080

Personframes	equ	0
Persontxtdat	equ	personframes+24
Persontext	equ	persontxtdat+(1026*2)

Pathdata	equ	0
Reellist	equ	pathdata+(36*144)

Lenofmapstore	equ	22*8*20*8
Maplen	equ	mapwidth*maplength
Freedatlen	equ	16*80
Setdatlen	equ	64*128
Textstart	equ	66*2

;-----------------------------------------------------------------Variables----


startvars	db	0
progresspoints	db	0
watchon	db	0
shadeson	db	0
secondcount	db	0
minutecount	db	30
hourcount	db	19
zoomon	db	1
location	db	0
expos	db	0
exframepos	dw	0
extextpos	dw	0
card1money	dw	0
listpos	dw	0
ryanpage	db	0


watchingtime	dw	0
reeltowatch	dw	-1		;reel plays from here in mode 0
endwatchreel	dw	0		;and stops here. Mode set to 1
speedcount	db	0
watchspeed	db	0
reeltohold	dw	-1		;if mode is 1 hold on this reel
endofholdreel	dw	-1		;if mode is 2 then play to end of
watchmode	db	-1		;hold reel. Set mode back to -1
destafterhold	db	0		;set walking destination.

newsitem	db	0

liftflag	db	0
liftpath	db	0
lockstatus	db	1
doorpath	db	0
counttoopen	db	0
counttoclose	db	0	
rockstardead	db	0
generaldead	db	0
sartaindead	db	0
aidedead	db	0
beenmugged	db	0

gunpassflag	db	0
canmovealtar	db	0
talkedtoattendant	db	0
talkedtosparky	db	0
talkedtoboss	db	0
talkedtorecep	db	0
cardpassflag	db	0
madmanflag	db	0
keeperflag	db	0
lasttrigger	db	0
mandead	db	0
seed	db	1,2,3
needtotravel	db	0
throughdoor	db	0
newobs	db	0
ryanon	db	255
combatcount	db	0
lastweapon	db	-1

dreamnumber	db	0
roomafterdream	db	0

shakecounter	db	48

lengthofvars	equ	$-startvars


speechcount	db	0

charshift	dw	0
kerning	db	0

brightness	db	0

roomloaded	db	0

didzoom	db	0

linespacing	dw	10
textaddressx	dw	13
textaddressy	dw	182		;address on screen for text
textlen	db	0
lastxpos	dw	0

icontop	dw	0
iconleft	dw	0
itemframe	db	0
itemtotran	db	0
roomad	dw	0
oldsubject	dw	0

withobject	db	0
withtype	db	0

lookcounter	dw	0

command	db	0
commandtype	db	0
oldcommandtype	db	0
objecttype	db	0
getback	db	0
invopen	db	0
mainmode	db	0
pickup	db	0
lastinvpos	db	0
examagain	db	0
newtextline	db	0

openedob	db	0
openedtype	db	0

oldmapadx	dw	0
oldmapady	dw	0
mapadx	dw	0
mapady	dw	0
mapoffsetx	dw	104
mapoffsety	dw	38

mapxstart	dw	0
mapystart	dw	0
mapxsize	db	0
mapysize	db	0

havedoneobs	db	0
manisoffscreen	db	0
rainspace	db	0

facing	db	0
leavedirection	db	0
turntoface	db	0
turndirection	db	0

maintimer	dw	0
introcount	db	0
arrowad	dw	0
currentkey	db	0
oldkey	db	0
useddirection	db	0
currentkey2	db	0

timercount	db	0
oldtimercount	db	0

mapx	db	0
mapy	db	0
newscreen	db	0
ryanx	db	0
ryany	db	0
lastflag	db	0
lastflagex	db	0
flagx	db	0
flagy	db	0

currentex	db	0
currentfree	db	0
currentframe	dw	0
framesad	dw	0
dataad	dw	0
frsegment	dw	0
objectx	dw	0
objecty	dw	0
offsetx	dw	0
offsety	dw	0
savesize	dw	0
savesource	dw	0
savex	db	0
savey	db	0
currentob	db	0
priorityDep	db	0	; dep for deprecated, I leave a byte here to minimize changes to the generated code

destpos	db	0

reallocation	db	0	 ;----------;some rooms have more than one
roomnum	db	0		;place in the Roomdata list, to
				;account for different start points
nowinnewroom	db	0		;this variable holds the rooms
resetmanxy	db	0		;real value - ie:which file it's in
newlocation	db	-1		;if set then room is loaded at end of watch mode, or straight away if not in watch mode
autolocation	db	-1
mustload	db	0
answered	db	0
saidno	db	0

doorcheck1	db	0
doorcheck2	db	0
doorcheck3	db	0
doorcheck4	db	0

mousex	dw	0
mousey	dw	0
mousebutton	dw	0
mousebutton1	dw	0
mousebutton2	dw	0
mousebutton3	dw	0
mousebutton4	dw	0
oldbutton	dw	0
oldx	dw	0
oldy	dw	0
lastbutton	dw	0
oldpointerx	dw	0
oldpointery	dw	0
delherex	dw	0
delherey	dw	0
pointerxs	db	32
pointerys	db	32
delxs	db	0
delys	db	0
pointerframe	db	0
pointerpower	db	0
auxpointerframe	db	0
pointermode	db	0
pointerspeed	db	0
pointercount	db	0
inmaparea	db	0

reelpointer	dw	0
slotdata	db	0
thisslot	db	0
slotflags	db	0
takeoff	dw	0

talkmode	db	0
talkpos	db	0
character	db	0
persondata	dw	0
talknum	db	0
numberinroom	db	0

currentcel	db	0
oldselection	db	0

stopwalking	db	0

mouseon	db	0
played	dw	0
timer1	db	0
timer2	db	0
timer3	db	0
wholetimer	dw	0
timer1to	db	0
timer2to	db	0
timer3to	db	0

watchdump	db	0

currentset	dw	0

logonum	db	0
oldlogonum	db	0
newlogonum	db	0
netseg	dw	0
netpoint	dw	0
keynum	db	0
cursorstate	db	0

pressed	db	0
presspointer	dw	0
graphicpress	db	0
presscount	db	0
keypadax	dw	0
keypadcx	dw	0
lightcount	db	0
folderpage	db	0
diarypage	db	0
menucount	db	0
symboltopx	db	0
symboltopnum	db	0
symboltopdir	db	0
symbolbotx	db	0
symbolbotnum	db	0
symbolbotdir	db	0

symboltolight	db	0
symbol1	db	0
symbol2	db	0
symbol3	db	0
symbolnum	db	0
dumpx	dw	0
dumpy	dw	0

walkandexam	db	0
walkexamtype	db	0
walkexamnum	db	0

cursloc	dw	0
curslocx	dw	0
curslocy	dw	0
curpos	dw	0
monadx	dw	0
monady	dw	0
gotfrom	dw	0

monsource	dw	0
numtodo	dw	0

timecount	dw	0
counttotimed	dw	0
timedseg	dw	0
timedoffset	dw	0
timedy	db	0
timedx	db	0
needtodumptimed	db	0

;recordpos	dw	0
;rechandle	dw	0
handle	dw	0

loadingorsave	db	0		;1 if load 2 if save
currentslot	db	0
cursorpos	db	0

colourpos	db	0
fadedirection	db	0
numtofade	db	0
fadecount	db	0
addtogreen	db	0
addtored	db	0
addtoblue	db	0


lastsoundreel	dw	0

soundbuffer	dw	0
soundbufferad	dw	0
soundbufferpage	db	0
soundtimes	db	0
needsoundbuff	db	0

oldint9seg	dw	-1
oldint9add	dw	-1
oldint8seg	dw	-1
oldint8add	dw	-1
oldsoundintseg	dw	0
oldsoundintadd	dw	0
soundbaseadd	dw	0
dsp_status	dw	0
dsp_write	dw	0
dmaaddress	db	0
soundint	db	5
sounddmachannel	db	1
sampleplaying	db	255
testresult	db	0
currentirq	db	0
speechloaded	db	0
speechlength	dw	0
volume	db	0
volumeto	db	0
volumedirection	db	0
volumecount	db	0

playblock	db	0

wongame	db	0

lasthardkey	db	0
bufferin	dw	0
bufferout	dw	0

extras	dw	0	;for allocated memory
workspace	dw	0	;allocated mem for screen buffer
mapstore	dw	0	;allocated mem for copy of room
charset1	dw	0	;allocated mem for normal charset
tempcharset	dw	0	;monitor char set
icons1	dw	0	;allocated mem for on screen stuff
icons2	dw	0
buffers	dw	0	;allocated mem for buffers
mainsprites	dw	0	;allocated mem for Ryan sprites
backdrop	dw	0
mapdata	dw	0

sounddata	dw	0
sounddata2	dw	0

recordspace	dw	0

freedat	dw	0
setdat	dw	0

reel1	dw	-1
reel2	dw	-1
reel3	dw	-1
roomdesc	dw	-1
freedesc	dw	-1
setdesc	dw	-1
blockdesc	dw	-1
setframes	dw	-1
freeframes	dw	-1
people	dw	-1
reels	dw	-1
commandtext	dw	-1
puzzletext	dw	-1
traveltext	dw	-1
tempgraphics	dw	-1
tempgraphics2	dw	-1
tempgraphics3	dw	-1
tempsprites	dw	-1

textfile1	dw	-1
textfile2	dw	-1
textfile3	dw	-1

blinkframe	db	23
blinkcount	db	0


reasseschanges	db	0	; if it's a 1 then obname will assume that
pointerspath	db	0	;the command has changed.
manspath	db	0	;ie. from "walk to" to "Examine"
pointerfirstpath	db	0
finaldest	db	0
destination	db	0
linestartx	dw	0
linestarty	dw	0
lineendx	dw	0
lineendy	dw	0
increment1	dw	0
increment2	dw	0
lineroutine	db	0
linepointer	db	0
linedirection	db	0
linelength	db	0

liftsoundcount	db	0

emmhandle	dw	0
emmpageframe	dw	0
emmhardwarepage	db	0

ch0emmpage	dw	0
ch0offset	dw	0
ch0blockstocopy	dw	0

ch0playing	db	0
ch0repeat	db	0
ch0oldemmpage	dw	0
ch0oldoffset	dw	0
ch0oldblockstocopy	dw	0

ch1playing	db	255
ch1emmpage	dw	0
ch1offset	dw	0
ch1blockstocopy	dw	0
ch1blocksplayed	dw	0

soundbufferwrite	dw	0

soundemmpage	dw	0
speechemmpage	dw	0

currentsample	db	-1
roomssample	db	0

gameerror	db	0

howmuchalloc	dw	0
