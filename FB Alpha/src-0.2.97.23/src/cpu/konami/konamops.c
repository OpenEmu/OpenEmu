/*

HNZVC

? = undefined
* = affected
- = unaffected
0 = cleared
1 = set
# = CCr directly affected by instruction
@ = special - carry set if bit 7 is set

*/

#ifdef NEW
static void illegal( void )
#else
KONAMI_INLINE void illegal( void )
#endif
{
	logerror("KONAMI: illegal opcode at %04x\n",PC);
}

/* $00 NEG direct ?**** */
KONAMI_INLINE void neg_di( void )
{
	UINT16 r,t;
	DIRBYTE(t);
	r = -t;
	CLR_NZVC;
	SET_FLAGS8(0,t,r);
	WM(EAD,r);
}

/* $01 ILLEGAL */

/* $02 ILLEGAL */

/* $03 COM direct -**01 */
KONAMI_INLINE void com_di( void )
{
	UINT8 t;
	DIRBYTE(t);
	t = ~t;
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	WM(EAD,t);
}

/* $04 LSR direct -0*-* */
KONAMI_INLINE void lsr_di( void )
{
	UINT8 t;
	DIRBYTE(t);
	CLR_NZC;
	CC |= (t & CC_C);
	t >>= 1;
	SET_Z8(t);
	WM(EAD,t);
}

/* $05 ILLEGAL */

/* $06 ROR direct -**-* */
KONAMI_INLINE void ror_di( void )
{
	UINT8 t,r;
	DIRBYTE(t);
	r= (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	r |= t>>1;
	SET_NZ8(r);
	WM(EAD,r);
}

/* $07 ASR direct ?**-* */
KONAMI_INLINE void asr_di( void )
{
	UINT8 t;
	DIRBYTE(t);
	CLR_NZC;
	CC |= (t & CC_C);
	t = (t & 0x80) | (t >> 1);
	SET_NZ8(t);
	WM(EAD,t);
}

/* $08 ASL direct ?**** */
KONAMI_INLINE void asl_di( void )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = t << 1;
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $09 ROL direct -**** */
KONAMI_INLINE void rol_di( void )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = (CC & CC_C) | (t << 1);
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $0A DEC direct -***- */
KONAMI_INLINE void dec_di( void )
{
	UINT8 t;
	DIRBYTE(t);
	--t;
	CLR_NZV;
	SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $0B ILLEGAL */

/* $OC INC direct -***- */
KONAMI_INLINE void inc_di( void )
{
	UINT8 t;
	DIRBYTE(t);
	++t;
	CLR_NZV;
	SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $OD TST direct -**0- */
KONAMI_INLINE void tst_di( void )
{
	UINT8 t;
	DIRBYTE(t);
	CLR_NZV;
	SET_NZ8(t);
}

/* $0E JMP direct ----- */
KONAMI_INLINE void jmp_di( void )
{
    DIRECT;
	PCD=EAD;
	change_pc(PCD);
}

/* $0F CLR direct -0100 */
KONAMI_INLINE void clr_di( void )
{
	DIRECT;
	WM(EAD,0);
	CLR_NZVC;
	SEZ;
}

/* $10 FLAG */

/* $11 FLAG */

/* $12 NOP inherent ----- */
KONAMI_INLINE void nop( void )
{
	;
}

/* $13 SYNC inherent ----- */
KONAMI_INLINE void sync( void )
{
	/* SYNC stops processing instructions until an interrupt request happens. */
	/* This doesn't require the corresponding interrupt to be enabled: if it */
	/* is disabled, execution continues with the next instruction. */
	konami.int_state |= KONAMI_SYNC;
	CHECK_IRQ_LINES;
	/* if KONAMI_SYNC has not been cleared by CHECK_IRQ_LINES,
     * stop execution until the interrupt lines change. */
	if( (konami.int_state & KONAMI_SYNC) && konami_ICount > 0 )
		konami_ICount = 0;
}

/* $14 ILLEGAL */

/* $15 ILLEGAL */

/* $16 LBRA relative ----- */
KONAMI_INLINE void lbra( void )
{
	IMMWORD(ea);
	PC += EA;
	change_pc(PCD);

	/* EHC 980508 speed up busy loop */
	if( EA == 0xfffd && konami_ICount > 0 )
		konami_ICount = 0;
}

/* $17 LBSR relative ----- */
KONAMI_INLINE void lbsr( void )
{
	IMMWORD(ea);
	PUSHWORD(pPC);
	PC += EA;
	change_pc(PCD);
}

/* $18 ILLEGAL */

#if 1
/* $19 DAA inherent (A) -**0* */
KONAMI_INLINE void daa( void )
{
	UINT8 msn, lsn;
	UINT16 t, cf = 0;
	msn = A & 0xf0; lsn = A & 0x0f;
	if( lsn>0x09 || CC & CC_H) cf |= 0x06;
	if( msn>0x80 && lsn>0x09 ) cf |= 0x60;
	if( msn>0x90 || CC & CC_C) cf |= 0x60;
	t = cf + A;
	CLR_NZV; /* keep carry from previous operation */
	SET_NZ8((UINT8)t); SET_C8(t);
	A = t;
}
#else
/* $19 DAA inherent (A) -**0* */
KONAMI_INLINE void daa( void )
{
	UINT16 t;
	t = A;
	if (CC & CC_H) t+=0x06;
	if ((t&0x0f)>9) t+=0x06;		/* ASG -- this code is broken! $66+$99=$FF -> DAA should = $65, we get $05! */
	if (CC & CC_C) t+=0x60;
	if ((t&0xf0)>0x90) t+=0x60;
	if (t&0x100) SEC;
	A = t;
}
#endif

/* $1A ORCC immediate ##### */
KONAMI_INLINE void orcc( void )
{
	UINT8 t;
	IMMBYTE(t);
	CC |= t;
	CHECK_IRQ_LINES;
}

/* $1B ILLEGAL */

/* $1C ANDCC immediate ##### */
KONAMI_INLINE void andcc( void )
{
	UINT8 t;
	IMMBYTE(t);
	CC &= t;
	CHECK_IRQ_LINES;
}

/* $1D SEX inherent -**0- */
KONAMI_INLINE void sex( void )
{
	UINT16 t;
	t = SIGNED(B);
	D = t;
//  CLR_NZV;    NS 20020905: applying the same fix that was applied to 6809 and 6309
	CLR_NZ;
	SET_NZ16(t);
}

/* $1E EXG inherent ----- */
KONAMI_INLINE void exg( void )
{
	UINT16 t1 = 0, t2 = 0;
	UINT8 tb;

	IMMBYTE(tb);

	GETREG( t1, tb >> 4 );
	GETREG( t2, tb & 0x0f );

	SETREG( t2, tb >> 4 );
	SETREG( t1, tb & 0x0f );
}

/* $1F TFR inherent ----- */
KONAMI_INLINE void tfr( void )
{
	UINT8 tb;
	UINT16 t = 0;

	IMMBYTE(tb);

	GETREG( t, tb & 0x0f );
	SETREG( t, ( tb >> 4 ) & 0x07 );
}

/* $20 BRA relative ----- */
KONAMI_INLINE void bra( void )
{
	UINT8 t;
	IMMBYTE(t);
	PC += SIGNED(t);
	change_pc(PCD);
	/* JB 970823 - speed up busy loops */
	if( t == 0xfe && konami_ICount > 0 )
		konami_ICount = 0;
}

/* $21 BRN relative ----- */
KONAMI_INLINE void brn( void )
{
	UINT8 t;
	IMMBYTE(t);
}

/* $1021 LBRN relative ----- */
KONAMI_INLINE void lbrn( void )
{
	IMMWORD(ea);
}

/* $22 BHI relative ----- */
KONAMI_INLINE void bhi( void )
{
	BRANCH( !(CC & (CC_Z|CC_C)) );
}

/* $1022 LBHI relative ----- */
KONAMI_INLINE void lbhi( void )
{
	LBRANCH( !(CC & (CC_Z|CC_C)) );
}

/* $23 BLS relative ----- */
KONAMI_INLINE void bls( void )
{
	BRANCH( (CC & (CC_Z|CC_C)) );
}

/* $1023 LBLS relative ----- */
KONAMI_INLINE void lbls( void )
{
	LBRANCH( (CC&(CC_Z|CC_C)) );
}

/* $24 BCC relative ----- */
KONAMI_INLINE void bcc( void )
{
	BRANCH( !(CC&CC_C) );
}

/* $1024 LBCC relative ----- */
KONAMI_INLINE void lbcc( void )
{
	LBRANCH( !(CC&CC_C) );
}

/* $25 BCS relative ----- */
KONAMI_INLINE void bcs( void )
{
	BRANCH( (CC&CC_C) );
}

/* $1025 LBCS relative ----- */
KONAMI_INLINE void lbcs( void )
{
	LBRANCH( (CC&CC_C) );
}

/* $26 BNE relative ----- */
KONAMI_INLINE void bne( void )
{
	BRANCH( !(CC&CC_Z) );
}

/* $1026 LBNE relative ----- */
KONAMI_INLINE void lbne( void )
{
	LBRANCH( !(CC&CC_Z) );
}

/* $27 BEQ relative ----- */
KONAMI_INLINE void beq( void )
{
	BRANCH( (CC&CC_Z) );
}

/* $1027 LBEQ relative ----- */
KONAMI_INLINE void lbeq( void )
{
	LBRANCH( (CC&CC_Z) );
}

/* $28 BVC relative ----- */
KONAMI_INLINE void bvc( void )
{
	BRANCH( !(CC&CC_V) );
}

/* $1028 LBVC relative ----- */
KONAMI_INLINE void lbvc( void )
{
	LBRANCH( !(CC&CC_V) );
}

/* $29 BVS relative ----- */
KONAMI_INLINE void bvs( void )
{
	BRANCH( (CC&CC_V) );
}

/* $1029 LBVS relative ----- */
KONAMI_INLINE void lbvs( void )
{
	LBRANCH( (CC&CC_V) );
}

/* $2A BPL relative ----- */
KONAMI_INLINE void bpl( void )
{
	BRANCH( !(CC&CC_N) );
}

/* $102A LBPL relative ----- */
KONAMI_INLINE void lbpl( void )
{
	LBRANCH( !(CC&CC_N) );
}

/* $2B BMI relative ----- */
KONAMI_INLINE void bmi( void )
{
	BRANCH( (CC&CC_N) );
}

/* $102B LBMI relative ----- */
KONAMI_INLINE void lbmi( void )
{
	LBRANCH( (CC&CC_N) );
}

/* $2C BGE relative ----- */
KONAMI_INLINE void bge( void )
{
	BRANCH( !NXORV );
}

/* $102C LBGE relative ----- */
KONAMI_INLINE void lbge( void )
{
	LBRANCH( !NXORV );
}

/* $2D BLT relative ----- */
KONAMI_INLINE void blt( void )
{
	BRANCH( NXORV );
}

/* $102D LBLT relative ----- */
KONAMI_INLINE void lblt( void )
{
	LBRANCH( NXORV );
}

/* $2E BGT relative ----- */
KONAMI_INLINE void bgt( void )
{
	BRANCH( !(NXORV || (CC&CC_Z)) );
}

/* $102E LBGT relative ----- */
KONAMI_INLINE void lbgt( void )
{
	LBRANCH( !(NXORV || (CC&CC_Z)) );
}

/* $2F BLE relative ----- */
KONAMI_INLINE void ble( void )
{
	BRANCH( (NXORV || (CC&CC_Z)) );
}

/* $102F LBLE relative ----- */
KONAMI_INLINE void lble( void )
{
	LBRANCH( (NXORV || (CC&CC_Z)) );
}

/* $30 LEAX indexed --*-- */
KONAMI_INLINE void leax( void )
{
	X = EA;
	CLR_Z;
	SET_Z(X);
}

/* $31 LEAY indexed --*-- */
KONAMI_INLINE void leay( void )
{
	Y = EA;
	CLR_Z;
	SET_Z(Y);
}

/* $32 LEAS indexed ----- */
KONAMI_INLINE void leas( void )
{
	S = EA;
	konami.int_state |= KONAMI_LDS;
}

/* $33 LEAU indexed ----- */
KONAMI_INLINE void leau( void )
{
	U = EA;
}

/* $34 PSHS inherent ----- */
KONAMI_INLINE void pshs( void )
{
	UINT8 t;
	IMMBYTE(t);
	if( t&0x80 ) { PUSHWORD(pPC); konami_ICount -= 2; }
	if( t&0x40 ) { PUSHWORD(pU);  konami_ICount -= 2; }
	if( t&0x20 ) { PUSHWORD(pY);  konami_ICount -= 2; }
	if( t&0x10 ) { PUSHWORD(pX);  konami_ICount -= 2; }
	if( t&0x08 ) { PUSHBYTE(DP);  konami_ICount -= 1; }
	if( t&0x04 ) { PUSHBYTE(B);   konami_ICount -= 1; }
	if( t&0x02 ) { PUSHBYTE(A);   konami_ICount -= 1; }
	if( t&0x01 ) { PUSHBYTE(CC);  konami_ICount -= 1; }
}

/* 35 PULS inherent ----- */
KONAMI_INLINE void puls( void )
{
	UINT8 t;
	IMMBYTE(t);
	if( t&0x01 ) { PULLBYTE(CC); konami_ICount -= 1; }
	if( t&0x02 ) { PULLBYTE(A);  konami_ICount -= 1; }
	if( t&0x04 ) { PULLBYTE(B);  konami_ICount -= 1; }
	if( t&0x08 ) { PULLBYTE(DP); konami_ICount -= 1; }
	if( t&0x10 ) { PULLWORD(XD); konami_ICount -= 2; }
	if( t&0x20 ) { PULLWORD(YD); konami_ICount -= 2; }
	if( t&0x40 ) { PULLWORD(UD); konami_ICount -= 2; }
	if( t&0x80 ) { PULLWORD(PCD); change_pc(PCD); konami_ICount -= 2; }

	/* check after all PULLs */
	if( t&0x01 ) { CHECK_IRQ_LINES; }
}

/* $36 PSHU inherent ----- */
KONAMI_INLINE void pshu( void )
{
	UINT8 t;
	IMMBYTE(t);
	if( t&0x80 ) { PSHUWORD(pPC); konami_ICount -= 2; }
	if( t&0x40 ) { PSHUWORD(pS);  konami_ICount -= 2; }
	if( t&0x20 ) { PSHUWORD(pY);  konami_ICount -= 2; }
	if( t&0x10 ) { PSHUWORD(pX);  konami_ICount -= 2; }
	if( t&0x08 ) { PSHUBYTE(DP);  konami_ICount -= 1; }
	if( t&0x04 ) { PSHUBYTE(B);   konami_ICount -= 1; }
	if( t&0x02 ) { PSHUBYTE(A);   konami_ICount -= 1; }
	if( t&0x01 ) { PSHUBYTE(CC);  konami_ICount -= 1; }
}

/* 37 PULU inherent ----- */
KONAMI_INLINE void pulu( void )
{
	UINT8 t;
	IMMBYTE(t);
	if( t&0x01 ) { PULUBYTE(CC); konami_ICount -= 1; }
	if( t&0x02 ) { PULUBYTE(A);  konami_ICount -= 1; }
	if( t&0x04 ) { PULUBYTE(B);  konami_ICount -= 1; }
	if( t&0x08 ) { PULUBYTE(DP); konami_ICount -= 1; }
	if( t&0x10 ) { PULUWORD(XD); konami_ICount -= 2; }
	if( t&0x20 ) { PULUWORD(YD); konami_ICount -= 2; }
	if( t&0x40 ) { PULUWORD(SD); konami_ICount -= 2; }
	if( t&0x80 ) { PULUWORD(PCD); change_pc(PCD); konami_ICount -= 2; }

	/* check after all PULLs */
	if( t&0x01 ) { CHECK_IRQ_LINES; }
}

/* $38 ILLEGAL */

/* $39 RTS inherent ----- */
KONAMI_INLINE void rts( void )
{
	PULLWORD(PCD);
	change_pc(PCD);
}

/* $3A ABX inherent ----- */
KONAMI_INLINE void abx( void )
{
	X += B;
}

/* $3B RTI inherent ##### */
KONAMI_INLINE void rti( void )
{
	PULLBYTE(CC);
	if( CC & CC_E ) /* entire state saved? */
	{
        konami_ICount -= 9;
		PULLBYTE(A);
		PULLBYTE(B);
		PULLBYTE(DP);
		PULLWORD(XD);
		PULLWORD(YD);
		PULLWORD(UD);
	}
	PULLWORD(PCD);
	change_pc(PCD);
	CHECK_IRQ_LINES;
}

/* $3C CWAI inherent ----1 */
KONAMI_INLINE void cwai( void )
{
	UINT8 t;
	IMMBYTE(t);
	CC &= t;
	/*
     * CWAI stacks the entire machine state on the hardware stack,
     * then waits for an interrupt; when the interrupt is taken
     * later, the state is *not* saved again after CWAI.
     */
	CC |= CC_E; 		/* HJB 990225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	konami.int_state |= KONAMI_CWAI;
	CHECK_IRQ_LINES;
	if( (konami.int_state & KONAMI_CWAI) && konami_ICount > 0 )
		konami_ICount = 0;
}

/* $3D MUL inherent --*-@ */
KONAMI_INLINE void mul( void )
{
	UINT16 t;
	t = A * B;
	CLR_ZC; SET_Z16(t); if(t&0x80) SEC;
	D = t;
}

/* $3E ILLEGAL */

/* $3F SWI (SWI2 SWI3) absolute indirect ----- */
KONAMI_INLINE void swi( void )
{
	CC |= CC_E; 			/* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	CC |= CC_IF | CC_II;	/* inhibit FIRQ and IRQ */
	PCD=RM16(0xfffa);
	change_pc(PCD);
}

/* $103F SWI2 absolute indirect ----- */
KONAMI_INLINE void swi2( void )
{
	CC |= CC_E; 			/* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
    PUSHBYTE(CC);
	PCD=RM16(0xfff4);
	change_pc(PCD);
}

/* $113F SWI3 absolute indirect ----- */
KONAMI_INLINE void swi3( void )
{
	CC |= CC_E; 			/* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
    PUSHBYTE(CC);
	PCD=RM16(0xfff2);
	change_pc(PCD);
}

/* $40 NEGA inherent ?**** */
KONAMI_INLINE void nega( void )
{
	UINT16 r;
	r = -A;
	CLR_NZVC;
	SET_FLAGS8(0,A,r);
	A = r;
}

/* $41 ILLEGAL */

/* $42 ILLEGAL */

/* $43 COMA inherent -**01 */
KONAMI_INLINE void coma( void )
{
	A = ~A;
	CLR_NZV;
	SET_NZ8(A);
	SEC;
}

/* $44 LSRA inherent -0*-* */
KONAMI_INLINE void lsra( void )
{
	CLR_NZC;
	CC |= (A & CC_C);
	A >>= 1;
	SET_Z8(A);
}

/* $45 ILLEGAL */

/* $46 RORA inherent -**-* */
KONAMI_INLINE void rora( void )
{
	UINT8 r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (A & CC_C);
	r |= A >> 1;
	SET_NZ8(r);
	A = r;
}

/* $47 ASRA inherent ?**-* */
KONAMI_INLINE void asra( void )
{
	CLR_NZC;
	CC |= (A & CC_C);
	A = (A & 0x80) | (A >> 1);
	SET_NZ8(A);
}

/* $48 ASLA inherent ?**** */
KONAMI_INLINE void asla( void )
{
	UINT16 r;
	r = A << 1;
	CLR_NZVC;
	SET_FLAGS8(A,A,r);
	A = r;
}

/* $49 ROLA inherent -**** */
KONAMI_INLINE void rola( void )
{
	UINT16 t,r;
	t = A;
	r = (CC & CC_C) | (t<<1);
	CLR_NZVC; SET_FLAGS8(t,t,r);
	A = r;
}

/* $4A DECA inherent -***- */
KONAMI_INLINE void deca( void )
{
	--A;
	CLR_NZV;
	SET_FLAGS8D(A);
}

/* $4B ILLEGAL */

/* $4C INCA inherent -***- */
KONAMI_INLINE void inca( void )
{
	++A;
	CLR_NZV;
	SET_FLAGS8I(A);
}

/* $4D TSTA inherent -**0- */
KONAMI_INLINE void tsta( void )
{
	CLR_NZV;
	SET_NZ8(A);
}

/* $4E ILLEGAL */

/* $4F CLRA inherent -0100 */
KONAMI_INLINE void clra( void )
{
	A = 0;
	CLR_NZVC; SEZ;
}

/* $50 NEGB inherent ?**** */
KONAMI_INLINE void negb( void )
{
	UINT16 r;
	r = -B;
	CLR_NZVC;
	SET_FLAGS8(0,B,r);
	B = r;
}

/* $51 ILLEGAL */

/* $52 ILLEGAL */

/* $53 COMB inherent -**01 */
KONAMI_INLINE void comb( void )
{
	B = ~B;
	CLR_NZV;
	SET_NZ8(B);
	SEC;
}

/* $54 LSRB inherent -0*-* */
KONAMI_INLINE void lsrb( void )
{
	CLR_NZC;
	CC |= (B & CC_C);
	B >>= 1;
	SET_Z8(B);
}

/* $55 ILLEGAL */

/* $56 RORB inherent -**-* */
KONAMI_INLINE void rorb( void )
{
	UINT8 r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (B & CC_C);
	r |= B >> 1;
	SET_NZ8(r);
	B = r;
}

/* $57 ASRB inherent ?**-* */
KONAMI_INLINE void asrb( void )
{
	CLR_NZC;
	CC |= (B & CC_C);
	B= (B & 0x80) | (B >> 1);
	SET_NZ8(B);
}

/* $58 ASLB inherent ?**** */
KONAMI_INLINE void aslb( void )
{
	UINT16 r;
	r = B << 1;
	CLR_NZVC;
	SET_FLAGS8(B,B,r);
	B = r;
}

/* $59 ROLB inherent -**** */
KONAMI_INLINE void rolb( void )
{
	UINT16 t,r;
	t = B;
	r = CC & CC_C;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	B = r;
}

/* $5A DECB inherent -***- */
KONAMI_INLINE void decb( void )
{
	--B;
	CLR_NZV;
	SET_FLAGS8D(B);
}

/* $5B ILLEGAL */

/* $5C INCB inherent -***- */
KONAMI_INLINE void incb( void )
{
	++B;
	CLR_NZV;
	SET_FLAGS8I(B);
}

/* $5D TSTB inherent -**0- */
KONAMI_INLINE void tstb( void )
{
	CLR_NZV;
	SET_NZ8(B);
}

/* $5E ILLEGAL */

/* $5F CLRB inherent -0100 */
KONAMI_INLINE void clrb( void )
{
	B = 0;
	CLR_NZVC; SEZ;
}

/* $60 NEG indexed ?**** */
KONAMI_INLINE void neg_ix( void )
{
	UINT16 r,t;
	t = RM(EAD);
	r = -t;
	CLR_NZVC;
	SET_FLAGS8(0,t,r);
	WM(EAD,r);
}

/* $61 ILLEGAL */

/* $62 ILLEGAL */

/* $63 COM indexed -**01 */
KONAMI_INLINE void com_ix( void )
{
	UINT8 t;
	t = ~RM(EAD);
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	WM(EAD,t);
}

/* $64 LSR indexed -0*-* */
KONAMI_INLINE void lsr_ix( void )
{
	UINT8 t;
	t = RM(EAD);
	CLR_NZC;
	CC |= (t & CC_C);
	t>>=1; SET_Z8(t);
	WM(EAD,t);
}

/* $65 ILLEGAL */

/* $66 ROR indexed -**-* */
KONAMI_INLINE void ror_ix( void )
{
	UINT8 t,r;
	t = RM(EAD);
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	r |= t>>1; SET_NZ8(r);
	WM(EAD,r);
}

/* $67 ASR indexed ?**-* */
KONAMI_INLINE void asr_ix( void )
{
	UINT8 t;
	t = RM(EAD);
	CLR_NZC;
	CC |= (t & CC_C);
	t=(t&0x80)|(t>>1);
	SET_NZ8(t);
	WM(EAD,t);
}

/* $68 ASL indexed ?**** */
KONAMI_INLINE void asl_ix( void )
{
	UINT16 t,r;
	t = RM(EAD);
	r = t << 1;
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $69 ROL indexed -**** */
KONAMI_INLINE void rol_ix( void )
{
	UINT16 t,r;
	t = RM(EAD);
	r = CC & CC_C;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $6A DEC indexed -***- */
KONAMI_INLINE void dec_ix( void )
{
	UINT8 t;
	t = RM(EAD) - 1;
	CLR_NZV; SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $6B ILLEGAL */

/* $6C INC indexed -***- */
KONAMI_INLINE void inc_ix( void )
{
	UINT8 t;
	t = RM(EAD) + 1;
	CLR_NZV; SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $6D TST indexed -**0- */
KONAMI_INLINE void tst_ix( void )
{
	UINT8 t;
	t = RM(EAD);
	CLR_NZV;
	SET_NZ8(t);
}

/* $6E JMP indexed ----- */
KONAMI_INLINE void jmp_ix( void )
{
	PCD=EAD;
	change_pc(PCD);
}

/* $6F CLR indexed -0100 */
KONAMI_INLINE void clr_ix( void )
{
	WM(EAD,0);
	CLR_NZVC; SEZ;
}

/* $70 NEG extended ?**** */
KONAMI_INLINE void neg_ex( void )
{
	UINT16 r,t;
	EXTBYTE(t); r=-t;
	CLR_NZVC; SET_FLAGS8(0,t,r);
	WM(EAD,r);
}

/* $71 ILLEGAL */

/* $72 ILLEGAL */

/* $73 COM extended -**01 */
KONAMI_INLINE void com_ex( void )
{
	UINT8 t;
	EXTBYTE(t); t = ~t;
	CLR_NZV; SET_NZ8(t); SEC;
	WM(EAD,t);
}

/* $74 LSR extended -0*-* */
KONAMI_INLINE void lsr_ex( void )
{
	UINT8 t;
	EXTBYTE(t); CLR_NZC; CC |= (t & CC_C);
	t>>=1; SET_Z8(t);
	WM(EAD,t);
}

/* $75 ILLEGAL */

/* $76 ROR extended -**-* */
KONAMI_INLINE void ror_ex( void )
{
	UINT8 t,r;
	EXTBYTE(t); r=(CC & CC_C) << 7;
	CLR_NZC; CC |= (t & CC_C);
	r |= t>>1; SET_NZ8(r);
	WM(EAD,r);
}

/* $77 ASR extended ?**-* */
KONAMI_INLINE void asr_ex( void )
{
	UINT8 t;
	EXTBYTE(t); CLR_NZC; CC |= (t & CC_C);
	t=(t&0x80)|(t>>1);
	SET_NZ8(t);
	WM(EAD,t);
}

/* $78 ASL extended ?**** */
KONAMI_INLINE void asl_ex( void )
{
	UINT16 t,r;
	EXTBYTE(t); r=t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $79 ROL extended -**** */
KONAMI_INLINE void rol_ex( void )
{
	UINT16 t,r;
	EXTBYTE(t); r = (CC & CC_C) | (t << 1);
	CLR_NZVC; SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $7A DEC extended -***- */
KONAMI_INLINE void dec_ex( void )
{
	UINT8 t;
	EXTBYTE(t); --t;
	CLR_NZV; SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $7B ILLEGAL */

/* $7C INC extended -***- */
KONAMI_INLINE void inc_ex( void )
{
	UINT8 t;
	EXTBYTE(t); ++t;
	CLR_NZV; SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $7D TST extended -**0- */
KONAMI_INLINE void tst_ex( void )
{
	UINT8 t;
	EXTBYTE(t); CLR_NZV; SET_NZ8(t);
}

/* $7E JMP extended ----- */
KONAMI_INLINE void jmp_ex( void )
{
	EXTENDED;
	PCD=EAD;
	change_pc(PCD);
}

/* $7F CLR extended -0100 */
KONAMI_INLINE void clr_ex( void )
{
	EXTENDED;
	WM(EAD,0);
	CLR_NZVC; SEZ;
}

/* $80 SUBA immediate ?**** */
KONAMI_INLINE void suba_im( void )
{
	UINT16 t,r;
	IMMBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $81 CMPA immediate ?**** */
KONAMI_INLINE void cmpa_im( void )
{
	UINT16	  t,r;
	IMMBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
}

/* $82 SBCA immediate ?**** */
KONAMI_INLINE void sbca_im( void )
{
	UINT16	  t,r;
	IMMBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $83 SUBD (CMPD CMPU) immediate -**** */
KONAMI_INLINE void subd_im( void )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $1083 CMPD immediate -**** */
KONAMI_INLINE void cmpd_im( void )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $1183 CMPU immediate -**** */
KONAMI_INLINE void cmpu_im( void )
{
	UINT32 r, d;
	PAIR b;
	IMMWORD(b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $84 ANDA immediate -**0- */
KONAMI_INLINE void anda_im( void )
{
	UINT8 t;
	IMMBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $85 BITA immediate -**0- */
KONAMI_INLINE void bita_im( void )
{
	UINT8 t,r;
	IMMBYTE(t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $86 LDA immediate -**0- */
KONAMI_INLINE void lda_im( void )
{
	IMMBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* is this a legal instruction? */
/* $87 STA immediate -**0- */
KONAMI_INLINE void sta_im( void )
{
	CLR_NZV;
	SET_NZ8(A);
	IMM8;
	WM(EAD,A);
}

/* $88 EORA immediate -**0- */
KONAMI_INLINE void eora_im( void )
{
	UINT8 t;
	IMMBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $89 ADCA immediate ***** */
KONAMI_INLINE void adca_im( void )
{
	UINT16 t,r;
	IMMBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $8A ORA immediate -**0- */
KONAMI_INLINE void ora_im( void )
{
	UINT8 t;
	IMMBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $8B ADDA immediate ***** */
KONAMI_INLINE void adda_im( void )
{
	UINT16 t,r;
	IMMBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $8C CMPX (CMPY CMPS) immediate -**** */
KONAMI_INLINE void cmpx_im( void )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $108C CMPY immediate -**** */
KONAMI_INLINE void cmpy_im( void )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $118C CMPS immediate -**** */
KONAMI_INLINE void cmps_im( void )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $8D BSR ----- */
KONAMI_INLINE void bsr( void )
{
	UINT8 t;
	IMMBYTE(t);
	PUSHWORD(pPC);
	PC += SIGNED(t);
	change_pc(PCD);
}

/* $8E LDX (LDY) immediate -**0- */
KONAMI_INLINE void ldx_im( void )
{
	IMMWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $108E LDY immediate -**0- */
KONAMI_INLINE void ldy_im( void )
{
	IMMWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* is this a legal instruction? */
/* $8F STX (STY) immediate -**0- */
KONAMI_INLINE void stx_im( void )
{
	CLR_NZV;
	SET_NZ16(X);
	IMM16;
	WM16(EAD,&pX);
}

/* is this a legal instruction? */
/* $108F STY immediate -**0- */
KONAMI_INLINE void sty_im( void )
{
	CLR_NZV;
	SET_NZ16(Y);
	IMM16;
	WM16(EAD,&pY);
}

/* $90 SUBA direct ?**** */
KONAMI_INLINE void suba_di( void )
{
	UINT16	  t,r;
	DIRBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $91 CMPA direct ?**** */
KONAMI_INLINE void cmpa_di( void )
{
	UINT16	  t,r;
	DIRBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
}

/* $92 SBCA direct ?**** */
KONAMI_INLINE void sbca_di( void )
{
	UINT16	  t,r;
	DIRBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $93 SUBD (CMPD CMPU) direct -**** */
KONAMI_INLINE void subd_di( void )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $1093 CMPD direct -**** */
KONAMI_INLINE void cmpd_di( void )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $1193 CMPU direct -**** */
KONAMI_INLINE void cmpu_di( void )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(U,b.d,r);
}

/* $94 ANDA direct -**0- */
KONAMI_INLINE void anda_di( void )
{
	UINT8 t;
	DIRBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $95 BITA direct -**0- */
KONAMI_INLINE void bita_di( void )
{
	UINT8 t,r;
	DIRBYTE(t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $96 LDA direct -**0- */
KONAMI_INLINE void lda_di( void )
{
	DIRBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $97 STA direct -**0- */
KONAMI_INLINE void sta_di( void )
{
	CLR_NZV;
	SET_NZ8(A);
	DIRECT;
	WM(EAD,A);
}

/* $98 EORA direct -**0- */
KONAMI_INLINE void eora_di( void )
{
	UINT8 t;
	DIRBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $99 ADCA direct ***** */
KONAMI_INLINE void adca_di( void )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $9A ORA direct -**0- */
KONAMI_INLINE void ora_di( void )
{
	UINT8 t;
	DIRBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $9B ADDA direct ***** */
KONAMI_INLINE void adda_di( void )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $9C CMPX (CMPY CMPS) direct -**** */
KONAMI_INLINE void cmpx_di( void )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $109C CMPY direct -**** */
KONAMI_INLINE void cmpy_di( void )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $119C CMPS direct -**** */
KONAMI_INLINE void cmps_di( void )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $9D JSR direct ----- */
KONAMI_INLINE void jsr_di( void )
{
	DIRECT;
	PUSHWORD(pPC);
	PCD=EAD;
	change_pc(PCD);
}

/* $9E LDX (LDY) direct -**0- */
KONAMI_INLINE void ldx_di( void )
{
	DIRWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $109E LDY direct -**0- */
KONAMI_INLINE void ldy_di( void )
{
	DIRWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $9F STX (STY) direct -**0- */
KONAMI_INLINE void stx_di( void )
{
	CLR_NZV;
	SET_NZ16(X);
	DIRECT;
	WM16(EAD,&pX);
}

/* $109F STY direct -**0- */
KONAMI_INLINE void sty_di( void )
{
	CLR_NZV;
	SET_NZ16(Y);
	DIRECT;
	WM16(EAD,&pY);
}

/* $a0 SUBA indexed ?**** */
KONAMI_INLINE void suba_ix( void )
{
	UINT16 t,r;
	t = RM(EAD);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $a1 CMPA indexed ?**** */
KONAMI_INLINE void cmpa_ix( void )
{
	UINT16 t,r;
	t = RM(EAD);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
}

/* $a2 SBCA indexed ?**** */
KONAMI_INLINE void sbca_ix( void )
{
	UINT16	  t,r;
	t = RM(EAD);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $a3 SUBD (CMPD CMPU) indexed -**** */
KONAMI_INLINE void subd_ix( void )
{
	UINT32 r,d;
	PAIR b;
	b.d=RM16(EAD);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $10a3 CMPD indexed -**** */
KONAMI_INLINE void cmpd_ix( void )
{
	UINT32 r,d;
	PAIR b;
	b.d=RM16(EAD);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $11a3 CMPU indexed -**** */
KONAMI_INLINE void cmpu_ix( void )
{
	UINT32 r;
	PAIR b;
	b.d=RM16(EAD);
	r = U - b.d;
	CLR_NZVC;
	SET_FLAGS16(U,b.d,r);
}

/* $a4 ANDA indexed -**0- */
KONAMI_INLINE void anda_ix( void )
{
	A &= RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a5 BITA indexed -**0- */
KONAMI_INLINE void bita_ix( void )
{
	UINT8 r;
	r = A & RM(EAD);
	CLR_NZV;
	SET_NZ8(r);
}

/* $a6 LDA indexed -**0- */
KONAMI_INLINE void lda_ix( void )
{
	A = RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a7 STA indexed -**0- */
KONAMI_INLINE void sta_ix( void )
{
	CLR_NZV;
	SET_NZ8(A);
	WM(EAD,A);
}

/* $a8 EORA indexed -**0- */
KONAMI_INLINE void eora_ix( void )
{
	A ^= RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a9 ADCA indexed ***** */
KONAMI_INLINE void adca_ix( void )
{
	UINT16 t,r;
	t = RM(EAD);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $aA ORA indexed -**0- */
KONAMI_INLINE void ora_ix( void )
{
	A |= RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $aB ADDA indexed ***** */
KONAMI_INLINE void adda_ix( void )
{
	UINT16 t,r;
	t = RM(EAD);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $aC CMPX (CMPY CMPS) indexed -**** */
KONAMI_INLINE void cmpx_ix( void )
{
	UINT32 r,d;
	PAIR b;
	b.d=RM16(EAD);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $10aC CMPY indexed -**** */
KONAMI_INLINE void cmpy_ix( void )
{
	UINT32 r,d;
	PAIR b;
	b.d=RM16(EAD);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $11aC CMPS indexed -**** */
KONAMI_INLINE void cmps_ix( void )
{
	UINT32 r,d;
	PAIR b;
	b.d=RM16(EAD);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $aD JSR indexed ----- */
KONAMI_INLINE void jsr_ix( void )
{
	PUSHWORD(pPC);
	PCD=EAD;
	change_pc(PCD);
}

/* $aE LDX (LDY) indexed -**0- */
KONAMI_INLINE void ldx_ix( void )
{
	X=RM16(EAD);
	CLR_NZV;
	SET_NZ16(X);
}

/* $10aE LDY indexed -**0- */
KONAMI_INLINE void ldy_ix( void )
{
	Y=RM16(EAD);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $aF STX (STY) indexed -**0- */
KONAMI_INLINE void stx_ix( void )
{
	CLR_NZV;
	SET_NZ16(X);
	WM16(EAD,&pX);
}

/* $10aF STY indexed -**0- */
KONAMI_INLINE void sty_ix( void )
{
	CLR_NZV;
	SET_NZ16(Y);
	WM16(EAD,&pY);
}

/* $b0 SUBA extended ?**** */
KONAMI_INLINE void suba_ex( void )
{
	UINT16	  t,r;
	EXTBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $b1 CMPA extended ?**** */
KONAMI_INLINE void cmpa_ex( void )
{
	UINT16	  t,r;
	EXTBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
}

/* $b2 SBCA extended ?**** */
KONAMI_INLINE void sbca_ex( void )
{
	UINT16	  t,r;
	EXTBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $b3 SUBD (CMPD CMPU) extended -**** */
KONAMI_INLINE void subd_ex( void )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $10b3 CMPD extended -**** */
KONAMI_INLINE void cmpd_ex( void )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $11b3 CMPU extended -**** */
KONAMI_INLINE void cmpu_ex( void )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $b4 ANDA extended -**0- */
KONAMI_INLINE void anda_ex( void )
{
	UINT8 t;
	EXTBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $b5 BITA extended -**0- */
KONAMI_INLINE void bita_ex( void )
{
	UINT8 t,r;
	EXTBYTE(t);
	r = A & t;
	CLR_NZV; SET_NZ8(r);
}

/* $b6 LDA extended -**0- */
KONAMI_INLINE void lda_ex( void )
{
	EXTBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $b7 STA extended -**0- */
KONAMI_INLINE void sta_ex( void )
{
	CLR_NZV;
	SET_NZ8(A);
	EXTENDED;
	WM(EAD,A);
}

/* $b8 EORA extended -**0- */
KONAMI_INLINE void eora_ex( void )
{
	UINT8 t;
	EXTBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $b9 ADCA extended ***** */
KONAMI_INLINE void adca_ex( void )
{
	UINT16 t,r;
	EXTBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $bA ORA extended -**0- */
KONAMI_INLINE void ora_ex( void )
{
	UINT8 t;
	EXTBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $bB ADDA extended ***** */
KONAMI_INLINE void adda_ex( void )
{
	UINT16 t,r;
	EXTBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $bC CMPX (CMPY CMPS) extended -**** */
KONAMI_INLINE void cmpx_ex( void )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $10bC CMPY extended -**** */
KONAMI_INLINE void cmpy_ex( void )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $11bC CMPS extended -**** */
KONAMI_INLINE void cmps_ex( void )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $bD JSR extended ----- */
KONAMI_INLINE void jsr_ex( void )
{
	EXTENDED;
	PUSHWORD(pPC);
	PCD=EAD;
	change_pc(PCD);
}

/* $bE LDX (LDY) extended -**0- */
KONAMI_INLINE void ldx_ex( void )
{
	EXTWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $10bE LDY extended -**0- */
KONAMI_INLINE void ldy_ex( void )
{
	EXTWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $bF STX (STY) extended -**0- */
KONAMI_INLINE void stx_ex( void )
{
	CLR_NZV;
	SET_NZ16(X);
	EXTENDED;
	WM16(EAD,&pX);
}

/* $10bF STY extended -**0- */
KONAMI_INLINE void sty_ex( void )
{
	CLR_NZV;
	SET_NZ16(Y);
	EXTENDED;
	WM16(EAD,&pY);
}

/* $c0 SUBB immediate ?**** */
KONAMI_INLINE void subb_im( void )
{
	UINT16	  t,r;
	IMMBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $c1 CMPB immediate ?**** */
KONAMI_INLINE void cmpb_im( void )
{
	UINT16	  t,r;
	IMMBYTE(t);
	r = B - t;
	CLR_NZVC; SET_FLAGS8(B,t,r);
}

/* $c2 SBCB immediate ?**** */
KONAMI_INLINE void sbcb_im( void )
{
	UINT16	  t,r;
	IMMBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $c3 ADDD immediate -**** */
KONAMI_INLINE void addd_im( void )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $c4 ANDB immediate -**0- */
KONAMI_INLINE void andb_im( void )
{
	UINT8 t;
	IMMBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $c5 BITB immediate -**0- */
KONAMI_INLINE void bitb_im( void )
{
	UINT8 t,r;
	IMMBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $c6 LDB immediate -**0- */
KONAMI_INLINE void ldb_im( void )
{
	IMMBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* is this a legal instruction? */
/* $c7 STB immediate -**0- */
KONAMI_INLINE void stb_im( void )
{
	CLR_NZV;
	SET_NZ8(B);
	IMM8;
	WM(EAD,B);
}

/* $c8 EORB immediate -**0- */
KONAMI_INLINE void eorb_im( void )
{
	UINT8 t;
	IMMBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $c9 ADCB immediate ***** */
KONAMI_INLINE void adcb_im( void )
{
	UINT16 t,r;
	IMMBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $cA ORB immediate -**0- */
KONAMI_INLINE void orb_im( void )
{
	UINT8 t;
	IMMBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $cB ADDB immediate ***** */
KONAMI_INLINE void addb_im( void )
{
	UINT16 t,r;
	IMMBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $cC LDD immediate -**0- */
KONAMI_INLINE void ldd_im( void )
{
	IMMWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* is this a legal instruction? */
/* $cD STD immediate -**0- */
KONAMI_INLINE void std_im( void )
{
	CLR_NZV;
	SET_NZ16(D);
    IMM16;
	WM16(EAD,&pD);
}

/* $cE LDU (LDS) immediate -**0- */
KONAMI_INLINE void ldu_im( void )
{
	IMMWORD(pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10cE LDS immediate -**0- */
KONAMI_INLINE void lds_im( void )
{
	IMMWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
	konami.int_state |= KONAMI_LDS;
}

/* is this a legal instruction? */
/* $cF STU (STS) immediate -**0- */
KONAMI_INLINE void stu_im( void )
{
	CLR_NZV;
	SET_NZ16(U);
    IMM16;
	WM16(EAD,&pU);
}

/* is this a legal instruction? */
/* $10cF STS immediate -**0- */
KONAMI_INLINE void sts_im( void )
{
	CLR_NZV;
	SET_NZ16(S);
    IMM16;
	WM16(EAD,&pS);
}

/* $d0 SUBB direct ?**** */
KONAMI_INLINE void subb_di( void )
{
	UINT16	  t,r;
	DIRBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $d1 CMPB direct ?**** */
KONAMI_INLINE void cmpb_di( void )
{
	UINT16	  t,r;
	DIRBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
}

/* $d2 SBCB direct ?**** */
KONAMI_INLINE void sbcb_di( void )
{
	UINT16	  t,r;
	DIRBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $d3 ADDD direct -**** */
KONAMI_INLINE void addd_di( void )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $d4 ANDB direct -**0- */
KONAMI_INLINE void andb_di( void )
{
	UINT8 t;
	DIRBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $d5 BITB direct -**0- */
KONAMI_INLINE void bitb_di( void )
{
	UINT8 t,r;
	DIRBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $d6 LDB direct -**0- */
KONAMI_INLINE void ldb_di( void )
{
	DIRBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $d7 STB direct -**0- */
KONAMI_INLINE void stb_di( void )
{
	CLR_NZV;
	SET_NZ8(B);
	DIRECT;
	WM(EAD,B);
}

/* $d8 EORB direct -**0- */
KONAMI_INLINE void eorb_di( void )
{
	UINT8 t;
	DIRBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $d9 ADCB direct ***** */
KONAMI_INLINE void adcb_di( void )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $dA ORB direct -**0- */
KONAMI_INLINE void orb_di( void )
{
	UINT8 t;
	DIRBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $dB ADDB direct ***** */
KONAMI_INLINE void addb_di( void )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $dC LDD direct -**0- */
KONAMI_INLINE void ldd_di( void )
{
	DIRWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $dD STD direct -**0- */
KONAMI_INLINE void std_di( void )
{
	CLR_NZV;
	SET_NZ16(D);
    DIRECT;
	WM16(EAD,&pD);
}

/* $dE LDU (LDS) direct -**0- */
KONAMI_INLINE void ldu_di( void )
{
	DIRWORD(pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10dE LDS direct -**0- */
KONAMI_INLINE void lds_di( void )
{
	DIRWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
	konami.int_state |= KONAMI_LDS;
}

/* $dF STU (STS) direct -**0- */
KONAMI_INLINE void stu_di( void )
{
	CLR_NZV;
	SET_NZ16(U);
	DIRECT;
	WM16(EAD,&pU);
}

/* $10dF STS direct -**0- */
KONAMI_INLINE void sts_di( void )
{
	CLR_NZV;
	SET_NZ16(S);
	DIRECT;
	WM16(EAD,&pS);
}

/* $e0 SUBB indexed ?**** */
KONAMI_INLINE void subb_ix( void )
{
	UINT16	  t,r;
	t = RM(EAD);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $e1 CMPB indexed ?**** */
KONAMI_INLINE void cmpb_ix( void )
{
	UINT16	  t,r;
	t = RM(EAD);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
}

/* $e2 SBCB indexed ?**** */
KONAMI_INLINE void sbcb_ix( void )
{
	UINT16	  t,r;
	t = RM(EAD);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $e3 ADDD indexed -**** */
KONAMI_INLINE void addd_ix( void )
{
	UINT32 r,d;
	PAIR b;
	b.d=RM16(EAD);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $e4 ANDB indexed -**0- */
KONAMI_INLINE void andb_ix( void )
{
	B &= RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $e5 BITB indexed -**0- */
KONAMI_INLINE void bitb_ix( void )
{
	UINT8 r;
	r = B & RM(EAD);
	CLR_NZV;
	SET_NZ8(r);
}

/* $e6 LDB indexed -**0- */
KONAMI_INLINE void ldb_ix( void )
{
	B = RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $e7 STB indexed -**0- */
KONAMI_INLINE void stb_ix( void )
{
	CLR_NZV;
	SET_NZ8(B);
	WM(EAD,B);
}

/* $e8 EORB indexed -**0- */
KONAMI_INLINE void eorb_ix( void )
{
	B ^= RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $e9 ADCB indexed ***** */
KONAMI_INLINE void adcb_ix( void )
{
	UINT16 t,r;
	t = RM(EAD);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $eA ORB indexed -**0- */
KONAMI_INLINE void orb_ix( void )
{
	B |= RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $eb ADDB indexed ***** */
KONAMI_INLINE void addb_ix( void )
{
	UINT16 t,r;
	t = RM(EAD);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $ec LDD indexed -**0- */
KONAMI_INLINE void ldd_ix( void )
{
	D=RM16(EAD);
	CLR_NZV; SET_NZ16(D);
}

/* $eD STD indexed -**0- */
KONAMI_INLINE void std_ix( void )
{
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD,&pD);
}

/* $eE LDU (LDS) indexed -**0- */
KONAMI_INLINE void ldu_ix( void )
{
	U=RM16(EAD);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10eE LDS indexed -**0- */
KONAMI_INLINE void lds_ix( void )
{
	S=RM16(EAD);
	CLR_NZV;
	SET_NZ16(S);
	konami.int_state |= KONAMI_LDS;
}

/* $eF STU (STS) indexed -**0- */
KONAMI_INLINE void stu_ix( void )
{
	CLR_NZV;
	SET_NZ16(U);
	WM16(EAD,&pU);
}

/* $10eF STS indexed -**0- */
KONAMI_INLINE void sts_ix( void )
{
	CLR_NZV;
	SET_NZ16(S);
	WM16(EAD,&pS);
}

/* $f0 SUBB extended ?**** */
KONAMI_INLINE void subb_ex( void )
{
	UINT16	  t,r;
	EXTBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $f1 CMPB extended ?**** */
KONAMI_INLINE void cmpb_ex( void )
{
	UINT16	  t,r;
	EXTBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
}

/* $f2 SBCB extended ?**** */
KONAMI_INLINE void sbcb_ex( void )
{
	UINT16	  t,r;
	EXTBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $f3 ADDD extended -**** */
KONAMI_INLINE void addd_ex( void )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $f4 ANDB extended -**0- */
KONAMI_INLINE void andb_ex( void )
{
	UINT8 t;
	EXTBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $f5 BITB extended -**0- */
KONAMI_INLINE void bitb_ex( void )
{
	UINT8 t,r;
	EXTBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $f6 LDB extended -**0- */
KONAMI_INLINE void ldb_ex( void )
{
	EXTBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $f7 STB extended -**0- */
KONAMI_INLINE void stb_ex( void )
{
	CLR_NZV;
	SET_NZ8(B);
	EXTENDED;
	WM(EAD,B);
}

/* $f8 EORB extended -**0- */
KONAMI_INLINE void eorb_ex( void )
{
	UINT8 t;
	EXTBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $f9 ADCB extended ***** */
KONAMI_INLINE void adcb_ex( void )
{
	UINT16 t,r;
	EXTBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $fA ORB extended -**0- */
KONAMI_INLINE void orb_ex( void )
{
	UINT8 t;
	EXTBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $fB ADDB extended ***** */
KONAMI_INLINE void addb_ex( void )
{
	UINT16 t,r;
	EXTBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $fC LDD extended -**0- */
KONAMI_INLINE void ldd_ex( void )
{
	EXTWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $fD STD extended -**0- */
KONAMI_INLINE void std_ex( void )
{
	CLR_NZV;
	SET_NZ16(D);
    EXTENDED;
	WM16(EAD,&pD);
}

/* $fE LDU (LDS) extended -**0- */
KONAMI_INLINE void ldu_ex( void )
{
	EXTWORD(pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10fE LDS extended -**0- */
KONAMI_INLINE void lds_ex( void )
{
	EXTWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
	konami.int_state |= KONAMI_LDS;
}

/* $fF STU (STS) extended -**0- */
KONAMI_INLINE void stu_ex( void )
{
	CLR_NZV;
	SET_NZ16(U);
	EXTENDED;
	WM16(EAD,&pU);
}

/* $10fF STS extended -**0- */
KONAMI_INLINE void sts_ex( void )
{
	CLR_NZV;
	SET_NZ16(S);
	EXTENDED;
	WM16(EAD,&pS);
}

KONAMI_INLINE void setline_im( void )
{
	UINT8 t;
	IMMBYTE(t);

	if ( konami.setlines_callback )
		(*konami.setlines_callback)( t );
}

KONAMI_INLINE void setline_ix( void )
{
	UINT8 t;
	t = RM(EA);

	if ( konami.setlines_callback )
		(*konami.setlines_callback)( t );
}

KONAMI_INLINE void setline_di( void )
{
	UINT8 t;
	DIRBYTE(t);

	if ( konami.setlines_callback )
		(*konami.setlines_callback)( t );
}

KONAMI_INLINE void setline_ex( void )
{
	UINT8 t;
	EXTBYTE(t);

	if ( konami.setlines_callback )
		(*konami.setlines_callback)( t );
}

KONAMI_INLINE void bmove( void )
{
	UINT8	t;

	while( U != 0 ) {
		t = RM(Y);
		WM(X,t);
		Y++;
		X++;
		U--;
		konami_ICount -= 2;
	}
}

KONAMI_INLINE void move( void )
{
	UINT8	t;

	t = RM(Y);
	WM(X,t);
	Y++;
	X++;
	U--;
}

/* CLRD inherent -0100 */
KONAMI_INLINE void clrd( void )
{
	D = 0;
	CLR_NZVC; SEZ;
}

/* CLRW indexed -0100 */
KONAMI_INLINE void clrw_ix( void )
{
	PAIR t;
	t.d = 0;
	WM16(EAD,&t);
	CLR_NZVC; SEZ;
}

/* CLRW direct -0100 */
KONAMI_INLINE void clrw_di( void )
{
	PAIR t;
	t.d = 0;
	DIRECT;
	WM16(EAD,&t);
	CLR_NZVC;
	SEZ;
}

/* CLRW extended -0100 */
KONAMI_INLINE void clrw_ex( void )
{
	PAIR t;
	t.d = 0;
	EXTENDED;
	WM16(EAD,&t);
	CLR_NZVC; SEZ;
}

/* LSRD immediate -0*-* */
KONAMI_INLINE void lsrd( void )
{
	UINT8 t;

	IMMBYTE( t );

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D >>= 1;
		SET_Z16(D);
	}
}

/* RORD immediate -**-* */
KONAMI_INLINE void rord( void )
{
	UINT16 r;
	UINT8  t;

	IMMBYTE(t);

	while ( t-- ) {
		r = (CC & CC_C) << 15;
		CLR_NZC;
		CC |= (D & CC_C);
		r |= D >> 1;
		SET_NZ16(r);
		D = r;
	}
}

/* ASRD immediate ?**-* */
KONAMI_INLINE void asrd( void )
{
	UINT8 t;

	IMMBYTE(t);

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D = (D & 0x8000) | (D >> 1);
		SET_NZ16(D);
	}
}

/* ASLD immediate ?**** */
KONAMI_INLINE void asld( void )
{
	UINT32	r;
	UINT8	t;

	IMMBYTE( t );

	while ( t-- ) {
		r = D << 1;
		CLR_NZVC;
		SET_FLAGS16(D,D,r);
		D = r;
	}
}

/* ROLD immediate -**-* */
KONAMI_INLINE void rold( void )
{
	UINT16 r;
	UINT8  t;

	IMMBYTE(t);

	while ( t-- ) {
		CLR_NZC;
		if ( D & 0x8000 ) SEC;
		r = CC & CC_C;
		r |= D << 1;
		SET_NZ16(r);
		D = r;
	}
}

/* DECB,JNZ relative ----- */
KONAMI_INLINE void decbjnz( void )
{
	--B;
	CLR_NZV;
	SET_FLAGS8D(B);
	BRANCH( !(CC&CC_Z) );
}

/* DECX,JNZ relative ----- */
KONAMI_INLINE void decxjnz( void )
{
	--X;
	CLR_NZV;
	SET_NZ16(X);	/* should affect V as well? */
	BRANCH( !(CC&CC_Z) );
}

KONAMI_INLINE void bset( void )
{
	UINT8	t;

	while( U != 0 ) {
		t = A;
		WM(XD,t);
		X++;
		U--;
		konami_ICount -= 2;
	}
}

KONAMI_INLINE void bset2( void )
{
	while( U != 0 ) {
		WM16(XD,&pD);
		X += 2;
		U--;
		konami_ICount -= 3;
	}
}

/* LMUL inherent --*-@ */
KONAMI_INLINE void lmul( void )
{
	UINT32 t;
	t = X * Y;
	X = (t >> 16);
	Y = (t & 0xffff);
	CLR_ZC; SET_Z(t); if( t & 0x8000 ) SEC;
}

/* DIVX inherent --*-@ */
KONAMI_INLINE void divx( void )
{
	UINT16 t;
	UINT8 r;
	if ( B != 0 )
	{
		t = X / B;
		r = X % B;
	}
	else
	{
		/* ?? */
		t = 0;
		r = 0;
	}
	CLR_ZC; SET_Z16(t); if ( t & 0x80 ) SEC;
	X = t;
	B = r;
}

/* INCD inherent -***- */
KONAMI_INLINE void incd( void )
{
	UINT32 r;
	r = D + 1;
	CLR_NZV;
	SET_FLAGS16(D,D,r);
	D = r;
}

/* INCW direct -***- */
KONAMI_INLINE void incw_di( void )
{
	PAIR t,r;
	DIRWORD(t);
	r = t;
	++r.d;
	CLR_NZV;
	SET_FLAGS16(t.d, t.d, r.d);
	WM16(EAD,&r);
}

/* INCW indexed -***- */
KONAMI_INLINE void incw_ix( void )
{
	PAIR t,r;
	t.d=RM16(EAD);
	r = t;
	++r.d;
	CLR_NZV;
	SET_FLAGS16(t.d, t.d, r.d);
	WM16(EAD,&r);
}

/* INCW extended -***- */
KONAMI_INLINE void incw_ex( void )
{
	PAIR t, r;
	EXTWORD(t);
	r = t;
	++r.d;
	CLR_NZV; SET_FLAGS16(t.d,t.d,r.d);
	WM16(EAD,&r);
}

/* DECD inherent -***- */
KONAMI_INLINE void decd( void )
{
	UINT32 r;
	r = D - 1;
	CLR_NZV;
	SET_FLAGS16(D,D,r);
	D = r;
}

/* DECW direct -***- */
KONAMI_INLINE void decw_di( void )
{
	PAIR t,r;
	DIRWORD(t);
	r = t;
	--r.d;
	CLR_NZV;
	SET_FLAGS16(t.d, t.d, r.d);
	WM16(EAD,&r);
}

/* DECW indexed -***- */
KONAMI_INLINE void decw_ix( void )
{
	PAIR t, r;
	t.d=RM16(EAD);
	r = t;
	--r.d;
	CLR_NZV; SET_FLAGS16(t.d, t.d, r.d);
	WM16(EAD,&r);
}

/* DECW extended -***- */
KONAMI_INLINE void decw_ex( void )
{
	PAIR t, r;
	EXTWORD(t);
	r = t;
	--r.d;
	CLR_NZV; SET_FLAGS16(t.d,t.d,r.d);
	WM16(EAD,&r);
}

/* TSTD inherent -**0- */
KONAMI_INLINE void tstd( void )
{
	CLR_NZV;
	SET_NZ16(D);
}

/* TSTW direct -**0- */
KONAMI_INLINE void tstw_di( void )
{
	PAIR t;
	CLR_NZV;
	DIRWORD(t);
	SET_NZ16(t.d);
}

/* TSTW indexed -**0- */
KONAMI_INLINE void tstw_ix( void )
{
	PAIR t;
	CLR_NZV;
	t.d=RM16(EAD);
	SET_NZ16(t.d);
}

/* TSTW extended -**0- */
KONAMI_INLINE void tstw_ex( void )
{
	PAIR t;
	CLR_NZV;
	EXTWORD(t);
	SET_NZ16(t.d);
}

/* LSRW direct -0*-* */
KONAMI_INLINE void lsrw_di( void )
{
	PAIR t;
	DIRWORD(t);
	CLR_NZC;
	CC |= (t.d & CC_C);
	t.d >>= 1;
	SET_Z16(t.d);
	WM16(EAD,&t);
}

/* LSRW indexed -0*-* */
KONAMI_INLINE void lsrw_ix( void )
{
	PAIR t;
	t.d=RM16(EAD);
	CLR_NZC;
	CC |= (t.d & CC_C);
	t.d >>= 1;
	SET_Z16(t.d);
	WM16(EAD,&t);
}

/* LSRW extended -0*-* */
KONAMI_INLINE void lsrw_ex( void )
{
	PAIR t;
	EXTWORD(t);
	CLR_NZC;
	CC |= (t.d & CC_C);
	t.d >>= 1;
	SET_Z16(t.d);
	WM16(EAD,&t);
}

/* RORW direct -**-* */
KONAMI_INLINE void rorw_di( void )
{
	PAIR t,r;
	DIRWORD(t);
	r.d = (CC & CC_C) << 15;
	CLR_NZC;
	CC |= (t.d & CC_C);
	r.d |= t.d>>1;
	SET_NZ16(r.d);
	WM16(EAD,&r);
}

/* RORW indexed -**-* */
KONAMI_INLINE void rorw_ix( void )
{
	PAIR t,r;
	t.d=RM16(EAD);
	r.d = (CC & CC_C) << 15;
	CLR_NZC;
	CC |= (t.d & CC_C);
	r.d |= t.d>>1;
	SET_NZ16(r.d);
	WM16(EAD,&r);
}

/* RORW extended -**-* */
KONAMI_INLINE void rorw_ex( void )
{
	PAIR t,r;
	EXTWORD(t);
	r.d = (CC & CC_C) << 15;
	CLR_NZC;
	CC |= (t.d & CC_C);
	r.d |= t.d>>1;
	SET_NZ16(r.d);
	WM16(EAD,&r);
}

/* ASRW direct ?**-* */
KONAMI_INLINE void asrw_di( void )
{
	PAIR t;
	DIRWORD(t);
	CLR_NZC;
	CC |= (t.d & CC_C);
	t.d = (t.d & 0x8000) | (t.d >> 1);
	SET_NZ16(t.d);
	WM16(EAD,&t);
}

/* ASRW indexed ?**-* */
KONAMI_INLINE void asrw_ix( void )
{
	PAIR t;
	t.d=RM16(EAD);
	CLR_NZC;
	CC |= (t.d & CC_C);
	t.d = (t.d & 0x8000) | (t.d >> 1);
	SET_NZ16(t.d);
	WM16(EAD,&t);
}

/* ASRW extended ?**-* */
KONAMI_INLINE void asrw_ex( void )
{
	PAIR t;
	EXTWORD(t);
	CLR_NZC;
	CC |= (t.d & CC_C);
	t.d = (t.d & 0x8000) | (t.d >> 1);
	SET_NZ16(t.d);
	WM16(EAD,&t);
}

/* ASLW direct ?**** */
KONAMI_INLINE void aslw_di( void )
{
	PAIR t,r;
	DIRWORD(t);
	r.d = t.d << 1;
	CLR_NZVC;
	SET_FLAGS16(t.d,t.d,r.d);
	WM16(EAD,&r);
}

/* ASLW indexed ?**** */
KONAMI_INLINE void aslw_ix( void )
{
	PAIR t,r;
	t.d=RM16(EAD);
	r.d = t.d << 1;
	CLR_NZVC;
	SET_FLAGS16(t.d,t.d,r.d);
	WM16(EAD,&r);
}

/* ASLW extended ?**** */
KONAMI_INLINE void aslw_ex( void )
{
	PAIR t,r;
	EXTWORD(t);
	r.d = t.d << 1;
	CLR_NZVC;
	SET_FLAGS16(t.d,t.d,r.d);
	WM16(EAD,&r);
}

/* ROLW direct -**** */
KONAMI_INLINE void rolw_di( void )
{
	PAIR t,r;
	DIRWORD(t);
	r.d = (CC & CC_C) | (t.d << 1);
	CLR_NZVC;
	SET_FLAGS16(t.d,t.d,r.d);
	WM16(EAD,&r);
}

/* ROLW indexed -**** */
KONAMI_INLINE void rolw_ix( void )
{
	PAIR t,r;
	t.d=RM16(EAD);
	r.d = (CC & CC_C) | (t.d << 1);
	CLR_NZVC;
	SET_FLAGS16(t.d,t.d,r.d);
	WM16(EAD,&r);
}

/* ROLW extended -**** */
KONAMI_INLINE void rolw_ex( void )
{
	PAIR t,r;
	EXTWORD(t);
	r.d = (CC & CC_C) | (t.d << 1);
	CLR_NZVC;
	SET_FLAGS16(t.d,t.d,r.d);
	WM16(EAD,&r);
}

/* NEGD inherent ?**** */
KONAMI_INLINE void negd( void )
{
	UINT32 r;
	r = -D;
	CLR_NZVC;
	SET_FLAGS16(0,D,r);
	D = r;
}

/* NEGW direct ?**** */
KONAMI_INLINE void negw_di( void )
{
	PAIR r,t;
	DIRWORD(t);
	r.d = -t.d;
	CLR_NZVC;
	SET_FLAGS16(0,t.d,r.d);
	WM16(EAD,&r);
}

/* NEGW indexed ?**** */
KONAMI_INLINE void negw_ix( void )
{
	PAIR r,t;
	t.d=RM16(EAD);
	r.d = -t.d;
	CLR_NZVC;
	SET_FLAGS16(0,t.d,r.d);
	WM16(EAD,&r);
}

/* NEGW extended ?**** */
KONAMI_INLINE void negw_ex( void )
{
	PAIR r,t;
	EXTWORD(t);
	r.d = -t.d;
	CLR_NZVC;
	SET_FLAGS16(0,t.d,r.d);
	WM16(EAD,&r);
}

/* ABSA inherent ?**** */
KONAMI_INLINE void absa( void )
{
	UINT16 r;
	if (A & 0x80)
		r = -A;
	else
		r = A;
	CLR_NZVC;
	SET_FLAGS8(0,A,r);
	A = r;
}

/* ABSB inherent ?**** */
KONAMI_INLINE void absb( void )
{
	UINT16 r;
	if (B & 0x80)
		r = -B;
	else
		r = B;
	CLR_NZVC;
	SET_FLAGS8(0,B,r);
	B = r;
}

/* ABSD inherent ?**** */
KONAMI_INLINE void absd( void )
{
	UINT32 r;
	if (D & 0x8000)
		r = -D;
	else
		r = D;
	CLR_NZVC;
	SET_FLAGS16(0,D,r);
	D = r;
}

/* LSRD direct -0*-* */
KONAMI_INLINE void lsrd_di( void )
{
	UINT8 t;

	DIRBYTE( t );

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D >>= 1;
		SET_Z16(D);
	}
}

/* RORD direct -**-* */
KONAMI_INLINE void rord_di( void )
{
	UINT16 r;
	UINT8  t;

	DIRBYTE(t);

	while ( t-- ) {
		r = (CC & CC_C) << 15;
		CLR_NZC;
		CC |= (D & CC_C);
		r |= D >> 1;
		SET_NZ16(r);
		D = r;
	}
}

/* ASRD direct ?**-* */
KONAMI_INLINE void asrd_di( void )
{
	UINT8 t;

	DIRBYTE(t);

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D = (D & 0x8000) | (D >> 1);
		SET_NZ16(D);
	}
}

/* ASLD direct ?**** */
KONAMI_INLINE void asld_di( void )
{
	UINT32	r;
	UINT8	t;

	DIRBYTE( t );

	while ( t-- ) {
		r = D << 1;
		CLR_NZVC;
		SET_FLAGS16(D,D,r);
		D = r;
	}
}

/* ROLD direct -**-* */
KONAMI_INLINE void rold_di( void )
{
	UINT16 r;
	UINT8  t;

	DIRBYTE(t);

	while ( t-- ) {
		CLR_NZC;
		if ( D & 0x8000 ) SEC;
		r = CC & CC_C;
		r |= D << 1;
		SET_NZ16(r);
		D = r;
	}
}

/* LSRD indexed -0*-* */
KONAMI_INLINE void lsrd_ix( void )
{
	UINT8 t;

	t=RM(EA);

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D >>= 1;
		SET_Z16(D);
	}
}

/* RORD indexed -**-* */
KONAMI_INLINE void rord_ix( void )
{
	UINT16 r;
	UINT8  t;

	t=RM(EA);

	while ( t-- ) {
		r = (CC & CC_C) << 15;
		CLR_NZC;
		CC |= (D & CC_C);
		r |= D >> 1;
		SET_NZ16(r);
		D = r;
	}
}

/* ASRD indexed ?**-* */
KONAMI_INLINE void asrd_ix( void )
{
	UINT8 t;

	t=RM(EA);

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D = (D & 0x8000) | (D >> 1);
		SET_NZ16(D);
	}
}

/* ASLD indexed ?**** */
KONAMI_INLINE void asld_ix( void )
{
	UINT32	r;
	UINT8	t;

	t=RM(EA);

	while ( t-- ) {
		r = D << 1;
		CLR_NZVC;
		SET_FLAGS16(D,D,r);
		D = r;
	}
}

/* ROLD indexed -**-* */
KONAMI_INLINE void rold_ix( void )
{
	UINT16 r;
	UINT8  t;

	t=RM(EA);

	while ( t-- ) {
		CLR_NZC;
		if ( D & 0x8000 ) SEC;
		r = CC & CC_C;
		r |= D << 1;
		SET_NZ16(r);
		D = r;
	}
}

/* LSRD extended -0*-* */
KONAMI_INLINE void lsrd_ex( void )
{
	UINT8 t;

	EXTBYTE(t);

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D >>= 1;
		SET_Z16(D);
	}
}

/* RORD extended -**-* */
KONAMI_INLINE void rord_ex( void )
{
	UINT16 r;
	UINT8  t;

	EXTBYTE(t);

	while ( t-- ) {
		r = (CC & CC_C) << 15;
		CLR_NZC;
		CC |= (D & CC_C);
		r |= D >> 1;
		SET_NZ16(r);
		D = r;
	}
}

/* ASRD extended ?**-* */
KONAMI_INLINE void asrd_ex( void )
{
	UINT8 t;

	EXTBYTE(t);

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D = (D & 0x8000) | (D >> 1);
		SET_NZ16(D);
	}
}

/* ASLD extended ?**** */
KONAMI_INLINE void asld_ex( void )
{
	UINT32	r;
	UINT8	t;

	EXTBYTE(t);

	while ( t-- ) {
		r = D << 1;
		CLR_NZVC;
		SET_FLAGS16(D,D,r);
		D = r;
	}
}

/* ROLD extended -**-* */
KONAMI_INLINE void rold_ex( void )
{
	UINT16 r;
	UINT8  t;

	EXTBYTE(t);

	while ( t-- ) {
		CLR_NZC;
		if ( D & 0x8000 ) SEC;
		r = CC & CC_C;
		r |= D << 1;
		SET_NZ16(r);
		D = r;
	}
}

KONAMI_INLINE void opcode2( void )
{
	UINT8 ireg2 = ROP_ARG(PCD);
	PC++;

	switch ( ireg2 ) {
//  case 0x00: EA=0; break; /* auto increment */
//  case 0x01: EA=0; break; /* double auto increment */
//  case 0x02: EA=0; break; /* auto decrement */
//  case 0x03: EA=0; break; /* double auto decrement */
//  case 0x04: EA=0; break; /* postbyte offs */
//  case 0x05: EA=0; break; /* postword offs */
//  case 0x06: EA=0; break; /* normal */
	case 0x07:
		EAD=0;
		(*konami_extended[konami.ireg])();
        konami_ICount -= 2;
		return;
//  case 0x08: EA=0; break; /* indirect - auto increment */
//  case 0x09: EA=0; break; /* indirect - double auto increment */
//  case 0x0a: EA=0; break; /* indirect - auto decrement */
//  case 0x0b: EA=0; break; /* indirect - double auto decrement */
//  case 0x0c: EA=0; break; /* indirect - postbyte offs */
//  case 0x0d: EA=0; break; /* indirect - postword offs */
//  case 0x0e: EA=0; break; /* indirect - normal */
	case 0x0f:				/* indirect - extended */
		IMMWORD(ea);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
//  case 0x10: EA=0; break; /* auto increment */
//  case 0x11: EA=0; break; /* double auto increment */
//  case 0x12: EA=0; break; /* auto decrement */
//  case 0x13: EA=0; break; /* double auto decrement */
//  case 0x14: EA=0; break; /* postbyte offs */
//  case 0x15: EA=0; break; /* postword offs */
//  case 0x16: EA=0; break; /* normal */
//  case 0x17: EA=0; break; /* extended */
//  case 0x18: EA=0; break; /* indirect - auto increment */
//  case 0x19: EA=0; break; /* indirect - double auto increment */
//  case 0x1a: EA=0; break; /* indirect - auto decrement */
//  case 0x1b: EA=0; break; /* indirect - double auto decrement */
//  case 0x1c: EA=0; break; /* indirect - postbyte offs */
//  case 0x1d: EA=0; break; /* indirect - postword offs */
//  case 0x1e: EA=0; break; /* indirect - normal */
//  case 0x1f: EA=0; break; /* indirect - extended */

/* base X */
    case 0x20:              /* auto increment */
		EA=X;
		X++;
        konami_ICount-=2;
		break;
	case 0x21:				/* double auto increment */
		EA=X;
		X+=2;
        konami_ICount-=3;
        break;
	case 0x22:				/* auto decrement */
		X--;
		EA=X;
        konami_ICount-=2;
        break;
	case 0x23:				/* double auto decrement */
		X-=2;
		EA=X;
        konami_ICount-=3;
		break;
	case 0x24:				/* postbyte offs */
		IMMBYTE(EA);
		EA=X+SIGNED(EA);
        konami_ICount-=2;
		break;
	case 0x25:				/* postword offs */
		IMMWORD(ea);
		EA+=X;
        konami_ICount-=4;
		break;
	case 0x26:				/* normal */
		EA=X;
		break;
//  case 0x27: EA=0; break; /* extended */
	case 0x28:				/* indirect - auto increment */
		EA=X;
		X++;
		EA=RM16(EAD);
        konami_ICount-=5;
		break;
	case 0x29:				/* indirect - double auto increment */
		EA=X;
		X+=2;
		EA=RM16(EAD);
        konami_ICount-=6;
		break;
	case 0x2a:				/* indirect - auto decrement */
		X--;
		EA=X;
		EA=RM16(EAD);
        konami_ICount-=5;
		break;
	case 0x2b:				/* indirect - double auto decrement */
		X-=2;
		EA=X;
		EA=RM16(EAD);
        konami_ICount-=6;
		break;
	case 0x2c:				/* indirect - postbyte offs */
		IMMBYTE(EA);
		EA=X+SIGNED(EA);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
	case 0x2d:				/* indirect - postword offs */
		IMMWORD(ea);
		EA+=X;
		EA=RM16(EAD);
        konami_ICount-=7;
		break;
	case 0x2e:				/* indirect - normal */
		EA=X;
		EA=RM16(EAD);
        konami_ICount-=3;
		break;
//  case 0x2f: EA=0; break; /* indirect - extended */

/* base Y */
    case 0x30:              /* auto increment */
		EA=Y;
		Y++;
        konami_ICount-=2;
		break;
	case 0x31:				/* double auto increment */
		EA=Y;
		Y+=2;
        konami_ICount-=3;
		break;
	case 0x32:				/* auto decrement */
		Y--;
		EA=Y;
        konami_ICount-=2;
		break;
	case 0x33:				/* double auto decrement */
		Y-=2;
		EA=Y;
        konami_ICount-=3;
		break;
	case 0x34:				/* postbyte offs */
		IMMBYTE(EA);
		EA=Y+SIGNED(EA);
        konami_ICount-=2;
		break;
	case 0x35:				/* postword offs */
		IMMWORD(ea);
		EA+=Y;
        konami_ICount-=4;
		break;
	case 0x36:				/* normal */
		EA=Y;
		break;
//  case 0x37: EA=0; break; /* extended */
	case 0x38:				/* indirect - auto increment */
		EA=Y;
		Y++;
		EA=RM16(EAD);
        konami_ICount-=5;
		break;
	case 0x39:				/* indirect - double auto increment */
		EA=Y;
		Y+=2;
		EA=RM16(EAD);
        konami_ICount-=6;
		break;
	case 0x3a:				/* indirect - auto decrement */
		Y--;
		EA=Y;
		EA=RM16(EAD);
        konami_ICount-=5;
		break;
	case 0x3b:				/* indirect - double auto decrement */
		Y-=2;
		EA=Y;
		EA=RM16(EAD);
        konami_ICount-=6;
		break;
	case 0x3c:				/* indirect - postbyte offs */
		IMMBYTE(EA);
		EA=Y+SIGNED(EA);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
	case 0x3d:				/* indirect - postword offs */
		IMMWORD(ea);
		EA+=Y;
		EA=RM16(EAD);
        konami_ICount-=7;
		break;
	case 0x3e:				/* indirect - normal */
		EA=Y;
		EA=RM16(EAD);
        konami_ICount-=3;
		break;
//  case 0x3f: EA=0; break; /* indirect - extended */

//  case 0x40: EA=0; break; /* auto increment */
//  case 0x41: EA=0; break; /* double auto increment */
//  case 0x42: EA=0; break; /* auto decrement */
//  case 0x43: EA=0; break; /* double auto decrement */
//  case 0x44: EA=0; break; /* postbyte offs */
//  case 0x45: EA=0; break; /* postword offs */
//  case 0x46: EA=0; break; /* normal */
//  case 0x47: EA=0; break; /* extended */
//  case 0x48: EA=0; break; /* indirect - auto increment */
//  case 0x49: EA=0; break; /* indirect - double auto increment */
//  case 0x4a: EA=0; break; /* indirect - auto decrement */
//  case 0x4b: EA=0; break; /* indirect - double auto decrement */
//  case 0x4c: EA=0; break; /* indirect - postbyte offs */
//  case 0x4d: EA=0; break; /* indirect - postword offs */
//  case 0x4e: EA=0; break; /* indirect - normal */
//  case 0x4f: EA=0; break; /* indirect - extended */

/* base U */
    case 0x50:              /* auto increment */
		EA=U;
		U++;
        konami_ICount-=2;
		break;
	case 0x51:				/* double auto increment */
		EA=U;
		U+=2;
        konami_ICount-=3;
		break;
	case 0x52:				/* auto decrement */
		U--;
		EA=U;
        konami_ICount-=2;
		break;
	case 0x53:				/* double auto decrement */
		U-=2;
		EA=U;
        konami_ICount-=3;
		break;
	case 0x54:				/* postbyte offs */
		IMMBYTE(EA);
		EA=U+SIGNED(EA);
        konami_ICount-=2;
		break;
	case 0x55:				/* postword offs */
		IMMWORD(ea);
		EA+=U;
        konami_ICount-=4;
		break;
	case 0x56:				/* normal */
		EA=U;
		break;
//  case 0x57: EA=0; break; /* extended */
	case 0x58:				/* indirect - auto increment */
		EA=U;
		U++;
		EA=RM16(EAD);
        konami_ICount-=5;
		break;
	case 0x59:				/* indirect - double auto increment */
		EA=U;
		U+=2;
		EA=RM16(EAD);
        konami_ICount-=6;
		break;
	case 0x5a:				/* indirect - auto decrement */
		U--;
		EA=U;
		EA=RM16(EAD);
        konami_ICount-=5;
		break;
	case 0x5b:				/* indirect - double auto decrement */
		U-=2;
		EA=U;
		EA=RM16(EAD);
        konami_ICount-=6;
		break;
	case 0x5c:				/* indirect - postbyte offs */
		IMMBYTE(EA);
		EA=U+SIGNED(EA);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
	case 0x5d:				/* indirect - postword offs */
		IMMWORD(ea);
		EA+=U;
		EA=RM16(EAD);
        konami_ICount-=7;
		break;
	case 0x5e:				/* indirect - normal */
		EA=U;
		EA=RM16(EAD);
        konami_ICount-=3;
		break;
//  case 0x5f: EA=0; break; /* indirect - extended */

/* base S */
    case 0x60:              /* auto increment */
		EAD=SD;
		S++;
        konami_ICount-=2;
		break;
	case 0x61:				/* double auto increment */
		EAD=SD;
		S+=2;
        konami_ICount-=3;
		break;
	case 0x62:				/* auto decrement */
		S--;
		EAD=SD;
        konami_ICount-=2;
		break;
	case 0x63:				/* double auto decrement */
		S-=2;
		EAD=SD;
        konami_ICount-=3;
		break;
	case 0x64:				/* postbyte offs */
		IMMBYTE(EA);
		EA=S+SIGNED(EA);
        konami_ICount-=2;
		break;
	case 0x65:				/* postword offs */
		IMMWORD(ea);
		EA+=S;
        konami_ICount-=4;
		break;
	case 0x66:				/* normal */
		EAD=SD;
		break;
//  case 0x67: EA=0; break; /* extended */
	case 0x68:				/* indirect - auto increment */
		EAD=SD;
		S++;
		EA=RM16(EAD);
        konami_ICount-=5;
		break;
	case 0x69:				/* indirect - double auto increment */
		EAD=SD;
		S+=2;
		EA=RM16(EAD);
        konami_ICount-=6;
		break;
	case 0x6a:				/* indirect - auto decrement */
		S--;
		EAD=SD;
		EA=RM16(EAD);
        konami_ICount-=5;
		break;
	case 0x6b:				/* indirect - double auto decrement */
		S-=2;
		EAD=SD;
		EA=RM16(EAD);
        konami_ICount-=6;
		break;
	case 0x6c:				/* indirect - postbyte offs */
		IMMBYTE(EA);
		EA=S+SIGNED(EA);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
	case 0x6d:				/* indirect - postword offs */
		IMMWORD(ea);
		EA+=S;
		EA=RM16(EAD);
        konami_ICount-=7;
		break;
	case 0x6e:				/* indirect - normal */
		EAD=SD;
		EA=RM16(EAD);
        konami_ICount-=3;
		break;
//  case 0x6f: EA=0; break; /* indirect - extended */

/* base PC */
    case 0x70:              /* auto increment */
		EAD=PCD;
		PC++;
        konami_ICount-=2;
		break;
	case 0x71:				/* double auto increment */
		EAD=PCD;
		PC+=2;
        konami_ICount-=3;
		break;
	case 0x72:				/* auto decrement */
		PC--;
		EAD=PCD;
        konami_ICount-=2;
		break;
	case 0x73:				/* double auto decrement */
		PC-=2;
		EAD=PCD;
        konami_ICount-=3;
		break;
	case 0x74:				/* postbyte offs */
		IMMBYTE(EA);
		EA=PC-1+SIGNED(EA);
        konami_ICount-=2;
		break;
	case 0x75:				/* postword offs */
		IMMWORD(ea);
		EA+=PC-2;
        konami_ICount-=4;
		break;
	case 0x76:				/* normal */
		EAD=PCD;
		break;
//  case 0x77: EA=0; break; /* extended */
	case 0x78:				/* indirect - auto increment */
		EAD=PCD;
		PC++;
		EA=RM16(EAD);
        konami_ICount-=5;
		break;
	case 0x79:				/* indirect - double auto increment */
		EAD=PCD;
		PC+=2;
		EA=RM16(EAD);
        konami_ICount-=6;
		break;
	case 0x7a:				/* indirect - auto decrement */
		PC--;
		EAD=PCD;
		EA=RM16(EAD);
        konami_ICount-=5;
		break;
	case 0x7b:				/* indirect - double auto decrement */
		PC-=2;
		EAD=PCD;
		EA=RM16(EAD);
        konami_ICount-=6;
		break;
	case 0x7c:				/* indirect - postbyte offs */
		IMMBYTE(EA);
		EA=PC-1+SIGNED(EA);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
	case 0x7d:				/* indirect - postword offs */
		IMMWORD(ea);
		EA+=PC-2;
		EA=RM16(EAD);
        konami_ICount-=7;
		break;
	case 0x7e:				/* indirect - normal */
		EAD=PCD;
		EA=RM16(EAD);
        konami_ICount-=3;
		break;
//  case 0x7f: EA=0; break; /* indirect - extended */

//  case 0x80: EA=0; break; /* register a */
//  case 0x81: EA=0; break; /* register b */
//  case 0x82: EA=0; break; /* ???? */
//  case 0x83: EA=0; break; /* ???? */
//  case 0x84: EA=0; break; /* ???? */
//  case 0x85: EA=0; break; /* ???? */
//  case 0x86: EA=0; break; /* ???? */
//  case 0x87: EA=0; break; /* register d */
//  case 0x88: EA=0; break; /* indirect - register a */
//  case 0x89: EA=0; break; /* indirect - register b */
//  case 0x8a: EA=0; break; /* indirect - ???? */
//  case 0x8b: EA=0; break; /* indirect - ???? */
//  case 0x8c: EA=0; break; /* indirect - ???? */
//  case 0x8d: EA=0; break; /* indirect - ???? */
//  case 0x8e: EA=0; break; /* indirect - register d */
//  case 0x8f: EA=0; break; /* indirect - ???? */
//  case 0x90: EA=0; break; /* register a */
//  case 0x91: EA=0; break; /* register b */
//  case 0x92: EA=0; break; /* ???? */
//  case 0x93: EA=0; break; /* ???? */
//  case 0x94: EA=0; break; /* ???? */
//  case 0x95: EA=0; break; /* ???? */
//  case 0x96: EA=0; break; /* ???? */
//  case 0x97: EA=0; break; /* register d */
//  case 0x98: EA=0; break; /* indirect - register a */
//  case 0x99: EA=0; break; /* indirect - register b */
//  case 0x9a: EA=0; break; /* indirect - ???? */
//  case 0x9b: EA=0; break; /* indirect - ???? */
//  case 0x9c: EA=0; break; /* indirect - ???? */
//  case 0x9d: EA=0; break; /* indirect - ???? */
//  case 0x9e: EA=0; break; /* indirect - register d */
//  case 0x9f: EA=0; break; /* indirect - ???? */
	case 0xa0:				/* register a */
		EA=X+SIGNED(A);
        konami_ICount-=1;
		break;
	case 0xa1:				/* register b */
		EA=X+SIGNED(B);
        konami_ICount-=1;
		break;
//  case 0xa2: EA=0; break; /* ???? */
//  case 0xa3: EA=0; break; /* ???? */
//  case 0xa4: EA=0; break; /* ???? */
//  case 0xa5: EA=0; break; /* ???? */
//  case 0xa6: EA=0; break; /* ???? */
	case 0xa7:				/* register d */
		EA=X+D;
        konami_ICount-=4;
		break;
	case 0xa8:				/* indirect - register a */
		EA=X+SIGNED(A);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
	case 0xa9:				/* indirect - register b */
		EA=X+SIGNED(B);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
//  case 0xaa: EA=0; break; /* indirect - ???? */
//  case 0xab: EA=0; break; /* indirect - ???? */
//  case 0xac: EA=0; break; /* indirect - ???? */
//  case 0xad: EA=0; break; /* indirect - ???? */
//  case 0xae: EA=0; break; /* indirect - ???? */
	case 0xaf:				/* indirect - register d */
		EA=X+D;
		EA=RM16(EAD);
        konami_ICount-=7;
		break;
	case 0xb0:				/* register a */
		EA=Y+SIGNED(A);
        konami_ICount-=1;
		break;
	case 0xb1:				/* register b */
		EA=Y+SIGNED(B);
        konami_ICount-=1;
		break;
//  case 0xb2: EA=0; break; /* ???? */
//  case 0xb3: EA=0; break; /* ???? */
//  case 0xb4: EA=0; break; /* ???? */
//  case 0xb5: EA=0; break; /* ???? */
//  case 0xb6: EA=0; break; /* ???? */
	case 0xb7:				/* register d */
		EA=Y+D;
        konami_ICount-=4;
		break;
	case 0xb8:				/* indirect - register a */
		EA=Y+SIGNED(A);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
	case 0xb9:				/* indirect - register b */
		EA=Y+SIGNED(B);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
//  case 0xba: EA=0; break; /* indirect - ???? */
//  case 0xbb: EA=0; break; /* indirect - ???? */
//  case 0xbc: EA=0; break; /* indirect - ???? */
//  case 0xbd: EA=0; break; /* indirect - ???? */
//  case 0xbe: EA=0; break; /* indirect - ???? */
	case 0xbf:				/* indirect - register d */
		EA=Y+D;
		EA=RM16(EAD);
        konami_ICount-=7;
		break;
//  case 0xc0: EA=0; break; /* register a */
//  case 0xc1: EA=0; break; /* register b */
//  case 0xc2: EA=0; break; /* ???? */
//  case 0xc3: EA=0; break; /* ???? */
	case 0xc4:
		EAD=0;
		(*konami_direct[konami.ireg])();
        konami_ICount -= 1;
		return;
//  case 0xc5: EA=0; break; /* ???? */
//  case 0xc6: EA=0; break; /* ???? */
//  case 0xc7: EA=0; break; /* register d */
//  case 0xc8: EA=0; break; /* indirect - register a */
//  case 0xc9: EA=0; break; /* indirect - register b */
//  case 0xca: EA=0; break; /* indirect - ???? */
//  case 0xcb: EA=0; break; /* indirect - ???? */
	case 0xcc:				/* indirect - direct */
		DIRWORD(ea);
        konami_ICount-=4;
		break;
//  case 0xcd: EA=0; break; /* indirect - ???? */
//  case 0xce: EA=0; break; /* indirect - register d */
//  case 0xcf: EA=0; break; /* indirect - ???? */
	case 0xd0:				/* register a */
		EA=U+SIGNED(A);
        konami_ICount-=1;
		break;
	case 0xd1:				/* register b */
		EA=U+SIGNED(B);
        konami_ICount-=1;
		break;
//  case 0xd2: EA=0; break; /* ???? */
//  case 0xd3: EA=0; break; /* ???? */
//  case 0xd4: EA=0; break; /* ???? */
//  case 0xd5: EA=0; break; /* ???? */
//  case 0xd6: EA=0; break; /* ???? */
	case 0xd7:				/* register d */
		EA=U+D;
        konami_ICount-=4;
		break;
	case 0xd8:				/* indirect - register a */
		EA=U+SIGNED(A);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
	case 0xd9:				/* indirect - register b */
		EA=U+SIGNED(B);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
//  case 0xda: EA=0; break; /* indirect - ???? */
//  case 0xdb: EA=0; break; /* indirect - ???? */
//  case 0xdc: EA=0; break; /* indirect - ???? */
//  case 0xdd: EA=0; break; /* indirect - ???? */
//  case 0xde: EA=0; break; /* indirect - ???? */
	case 0xdf:				/* indirect - register d */
		EA=U+D;
		EA=RM16(EAD);
        konami_ICount-=7;
        break;
	case 0xe0:				/* register a */
		EA=S+SIGNED(A);
        konami_ICount-=1;
		break;
	case 0xe1:				/* register b */
		EA=S+SIGNED(B);
        konami_ICount-=1;
		break;
//  case 0xe2: EA=0; break; /* ???? */
//  case 0xe3: EA=0; break; /* ???? */
//  case 0xe4: EA=0; break; /* ???? */
//  case 0xe5: EA=0; break; /* ???? */
//  case 0xe6: EA=0; break; /* ???? */
	case 0xe7:				/* register d */
		EA=S+D;
        konami_ICount-=4;
		break;
	case 0xe8:				/* indirect - register a */
		EA=S+SIGNED(A);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
	case 0xe9:				/* indirect - register b */
		EA=S+SIGNED(B);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
//  case 0xea: EA=0; break; /* indirect - ???? */
//  case 0xeb: EA=0; break; /* indirect - ???? */
//  case 0xec: EA=0; break; /* indirect - ???? */
//  case 0xed: EA=0; break; /* indirect - ???? */
//  case 0xee: EA=0; break; /* indirect - ???? */
	case 0xef:				/* indirect - register d */
		EA=S+D;
		EA=RM16(EAD);
        konami_ICount-=7;
		break;
	case 0xf0:				/* register a */
		EA=PC+SIGNED(A);
        konami_ICount-=1;
		break;
	case 0xf1:				/* register b */
		EA=PC+SIGNED(B);
        konami_ICount-=1;
		break;
//  case 0xf2: EA=0; break; /* ???? */
//  case 0xf3: EA=0; break; /* ???? */
//  case 0xf4: EA=0; break; /* ???? */
//  case 0xf5: EA=0; break; /* ???? */
//  case 0xf6: EA=0; break; /* ???? */
	case 0xf7:				/* register d */
		EA=PC+D;
        konami_ICount-=4;
		break;
	case 0xf8:				/* indirect - register a */
		EA=PC+SIGNED(A);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
	case 0xf9:				/* indirect - register b */
		EA=PC+SIGNED(B);
		EA=RM16(EAD);
        konami_ICount-=4;
		break;
//  case 0xfa: EA=0; break; /* indirect - ???? */
//  case 0xfb: EA=0; break; /* indirect - ???? */
//  case 0xfc: EA=0; break; /* indirect - ???? */
//  case 0xfd: EA=0; break; /* indirect - ???? */
//  case 0xfe: EA=0; break; /* indirect - ???? */
	case 0xff:				/* indirect - register d */
		EA=PC+D;
		EA=RM16(EAD);
        konami_ICount-=7;
		break;
	default:
		logerror("KONAMI: Unknown/Invalid postbyte at PC = %04x\n", PC -1 );
        EAD = 0;
	}
	(*konami_indexed[konami.ireg])();
}
