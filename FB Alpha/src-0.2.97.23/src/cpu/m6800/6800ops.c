
/*

HNZVC

? = undefined
* = affected
- = unaffected
0 = cleared
1 = set
# = ccr directly affected by instruction
@ = special - carry set if bit 7 is set

*/

//M6800_INLINE void illegal( void )
static void illegal( void )
{
//	logerror("M6808: illegal opcode: address %04X, op %02X\n",PC,(int) M_RDOP_ARG(PC)&0xFF);
}

/* HD63701 only */
#if (HAS_HD63701)
//M6800_INLINE void trap( void )
static void trap( void )
{
//	logerror("M6808: illegal opcode: address %04X, op %02X\n",PC,(int) M_RDOP_ARG(PC)&0xFF);
	TAKE_TRAP;
}
#endif

/* $00 ILLEGAL */

/* $01 NOP */
M6800_INLINE void nop( void )
{
}

/* $02 ILLEGAL */

/* $03 ILLEGAL */

/* $04 LSRD inherent -0*-* */
M6800_INLINE void lsrd (void)
{
	UINT16 t;
	CLR_NZC; t = D; CC|=(t&0x0001);
	t>>=1; SET_Z16(t); D=t;
}

/* $05 ASLD inherent ?**** */
M6800_INLINE void asld (void)
{
	int r;
	UINT16 t;
	t = D; r=t<<1;
	CLR_NZVC; SET_FLAGS16(t,t,r);
	D=r;
}

/* $06 TAP inherent ##### */
M6800_INLINE void tap (void)
{
	CC=A;
	ONE_MORE_INSN();
	CHECK_IRQ_LINES(); /* HJB 990417 */
}

/* $07 TPA inherent ----- */
M6800_INLINE void tpa (void)
{
	A=CC;
}

/* $08 INX inherent --*-- */
M6800_INLINE void inx (void)
{
	++X;
	CLR_Z; SET_Z16(X);
}

/* $09 DEX inherent --*-- */
M6800_INLINE void dex (void)
{
	--X;
	CLR_Z; SET_Z16(X);
}

/* $0a CLV */
M6800_INLINE void clv (void)
{
	CLV;
}

/* $0b SEV */
M6800_INLINE void sev (void)
{
	SEV;
}

/* $0c CLC */
M6800_INLINE void clc (void)
{
	CLC;
}

/* $0d SEC */
M6800_INLINE void sec (void)
{
	SEC;
}

/* $0e CLI */
M6800_INLINE void cli (void)
{
	CLI;
	ONE_MORE_INSN();
	CHECK_IRQ_LINES(); /* HJB 990417 */
}

/* $0f SEI */
M6800_INLINE void sei (void)
{
	SEI;
	ONE_MORE_INSN();
	CHECK_IRQ_LINES(); /* HJB 990417 */
}

/* $10 SBA inherent -**** */
M6800_INLINE void sba (void)
{
	UINT16 t;
	t=A-B;
	CLR_NZVC; SET_FLAGS8(A,B,t);
	A=t;
}

/* $11 CBA inherent -**** */
M6800_INLINE void cba (void)
{
	UINT16 t;
	t=A-B;
	CLR_NZVC; SET_FLAGS8(A,B,t);
}

/* $12 ILLEGAL */
M6800_INLINE void undoc1 (void)
{
	X += RM( S + 1 );
}

/* $13 ILLEGAL */
M6800_INLINE void undoc2 (void)
{
	X += RM( S + 1 );
}


/* $14 ILLEGAL */

/* $15 ILLEGAL */

/* $16 TAB inherent -**0- */
M6800_INLINE void tab (void)
{
	B=A;
	CLR_NZV; SET_NZ8(B);
}

/* $17 TBA inherent -**0- */
M6800_INLINE void tba (void)
{
	A=B;
	CLR_NZV; SET_NZ8(A);
}

/* $18 XGDX inherent ----- */ /* HD63701YO only */
M6800_INLINE void xgdx( void )
{
	UINT16 t = X;
	X = D;
	D=t;
}

/* $19 DAA inherent (A) -**0* */
M6800_INLINE void daa( void )
{
	UINT8 msn, lsn;
	UINT16 t, cf = 0;
	msn=A & 0xf0; lsn=A & 0x0f;
	if( lsn>0x09 || CC&0x20 ) cf |= 0x06;
	if( msn>0x80 && lsn>0x09 ) cf |= 0x60;
	if( msn>0x90 || CC&0x01 ) cf |= 0x60;
	t = cf + A;
	CLR_NZV; /* keep carry from previous operation */
	SET_NZ8((UINT8)t); SET_C8(t);
	A = t;
}

/* $1a ILLEGAL */

#if (HAS_HD63701)
/* $1a SLP */ /* HD63701YO only */
M6800_INLINE void slp (void)
{
	/* wait for next IRQ (same as waiting of wai) */
	m6808.wai_state |= HD63701_SLP;
	EAT_CYCLES;
}
#endif

/* $1b ABA inherent ***** */
M6800_INLINE void aba (void)
{
	UINT16 t;
	t=A+B;
	CLR_HNZVC; SET_FLAGS8(A,B,t); SET_H(A,B,t);
	A=t;
}

/* $1c ILLEGAL */

/* $1d ILLEGAL */

/* $1e ILLEGAL */

/* $1f ILLEGAL */

/* $20 BRA relative ----- */
M6800_INLINE void bra( void )
{
	UINT8 t;
	IMMBYTE(t);PC+=SIGNED(t);CHANGE_PC();
	/* speed up busy loops */
	if (t==0xfe) EAT_CYCLES;
}

/* $21 BRN relative ----- */
M6800_INLINE void brn( void )
{
	UINT8 t;
	IMMBYTE(t);
}

/* $22 BHI relative ----- */
M6800_INLINE void bhi( void )
{
	UINT8 t;
	BRANCH(!(CC&0x05));
}

/* $23 BLS relative ----- */
M6800_INLINE void bls( void )
{
	UINT8 t;
	BRANCH(CC&0x05);
}

/* $24 BCC relative ----- */
M6800_INLINE void bcc( void )
{
	UINT8 t;
	BRANCH(!(CC&0x01));
}

/* $25 BCS relative ----- */
M6800_INLINE void bcs( void )
{
	UINT8 t;
	BRANCH(CC&0x01);
}

/* $26 BNE relative ----- */
M6800_INLINE void bne( void )
{
	UINT8 t;
	BRANCH(!(CC&0x04));
}

/* $27 BEQ relative ----- */
M6800_INLINE void beq( void )
{
	UINT8 t;
	BRANCH(CC&0x04);
}

/* $28 BVC relative ----- */
M6800_INLINE void bvc( void )
{
	UINT8 t;
	BRANCH(!(CC&0x02));
}

/* $29 BVS relative ----- */
M6800_INLINE void bvs( void )
{
	UINT8 t;
	BRANCH(CC&0x02);
}

/* $2a BPL relative ----- */
M6800_INLINE void bpl( void )
{
	UINT8 t;
	BRANCH(!(CC&0x08));
}

/* $2b BMI relative ----- */
M6800_INLINE void bmi( void )
{
	UINT8 t;
	BRANCH(CC&0x08);
}

/* $2c BGE relative ----- */
M6800_INLINE void bge( void )
{
	UINT8 t;
	BRANCH(!NXORV);
}

/* $2d BLT relative ----- */
M6800_INLINE void blt( void )
{
	UINT8 t;
	BRANCH(NXORV);
}

/* $2e BGT relative ----- */
M6800_INLINE void bgt( void )
{
	UINT8 t;
	BRANCH(!(NXORV||CC&0x04));
}

/* $2f BLE relative ----- */
M6800_INLINE void ble( void )
{
	UINT8 t;
	BRANCH(NXORV||CC&0x04);
}

/* $30 TSX inherent ----- */
M6800_INLINE void tsx (void)
{
	X = ( S + 1 );
}

/* $31 INS inherent ----- */
M6800_INLINE void ins (void)
{
	++S;
}

/* $32 PULA inherent ----- */
M6800_INLINE void pula (void)
{
	PULLBYTE(m6808.d.b.h);
}

/* $33 PULB inherent ----- */
M6800_INLINE void pulb (void)
{
	PULLBYTE(m6808.d.b.l);
}

/* $34 DES inherent ----- */
M6800_INLINE void des (void)
{
	--S;
}

/* $35 TXS inherent ----- */
M6800_INLINE void txs (void)
{
	S = ( X - 1 );
}

/* $36 PSHA inherent ----- */
M6800_INLINE void psha (void)
{
	PUSHBYTE(m6808.d.b.h);
}

/* $37 PSHB inherent ----- */
M6800_INLINE void pshb (void)
{
	PUSHBYTE(m6808.d.b.l);
}

/* $38 PULX inherent ----- */
M6800_INLINE void pulx (void)
{
	PULLWORD(pX);
}

/* $39 RTS inherent ----- */
M6800_INLINE void rts( void )
{
	PULLWORD(pPC);
	CHANGE_PC();
}

/* $3a ABX inherent ----- */
M6800_INLINE void abx( void )
{
	X += B;
}

/* $3b RTI inherent ##### */
M6800_INLINE void rti( void )
{
	PULLBYTE(CC);
	PULLBYTE(B);
	PULLBYTE(A);
	PULLWORD(pX);
	PULLWORD(pPC);
	CHANGE_PC();
	CHECK_IRQ_LINES(); /* HJB 990417 */
}

/* $3c PSHX inherent ----- */
M6800_INLINE void pshx (void)
{
	PUSHWORD(pX);
}

/* $3d MUL inherent --*-@ */
M6800_INLINE void mul( void )
{
	UINT16 t;
	t=A*B;
	CLR_C; if(t&0x80) SEC;
	D=t;
}

/* $3e WAI inherent ----- */
M6800_INLINE void wai( void )
{
	/*
     * WAI stacks the entire machine state on the
     * hardware stack, then waits for an interrupt.
     */
	m6808.wai_state |= M6800_WAI;
	PUSHWORD(pPC);
	PUSHWORD(pX);
	PUSHBYTE(A);
	PUSHBYTE(B);
	PUSHBYTE(CC);
	CHECK_IRQ_LINES();
	if (m6808.wai_state & M6800_WAI) EAT_CYCLES;
}

/* $3f SWI absolute indirect ----- */
M6800_INLINE void swi( void )
{
	PUSHWORD(pPC);
	PUSHWORD(pX);
	PUSHBYTE(A);
	PUSHBYTE(B);
    PUSHBYTE(CC);
    SEI;
	PCD = RM16(0xfffa);
	CHANGE_PC();
}

/* $40 NEGA inherent ?**** */
M6800_INLINE void nega( void )
{
	UINT16 r;
	r=-A;
	CLR_NZVC; SET_FLAGS8(0,A,r);
	A=r;
}

/* $41 ILLEGAL */

/* $42 ILLEGAL */

/* $43 COMA inherent -**01 */
M6800_INLINE void coma( void )
{
	A = ~A;
	CLR_NZV; SET_NZ8(A); SEC;
}

/* $44 LSRA inherent -0*-* */
M6800_INLINE void lsra( void )
{
	CLR_NZC; CC|=(A&0x01);
	A>>=1; SET_Z8(A);
}

/* $45 ILLEGAL */

/* $46 RORA inherent -**-* */
M6800_INLINE void rora( void )
{
	UINT8 r;
	r=(CC&0x01)<<7;
	CLR_NZC; CC|=(A&0x01);
	r |= A>>1; SET_NZ8(r);
	A=r;
}

/* $47 ASRA inherent ?**-* */
M6800_INLINE void asra( void )
{
	CLR_NZC; CC|=(A&0x01);
	A>>=1; A|=((A&0x40)<<1);
	SET_NZ8(A);
}

/* $48 ASLA inherent ?**** */
M6800_INLINE void asla( void )
{
	UINT16 r;
	r=A<<1;
	CLR_NZVC; SET_FLAGS8(A,A,r);
	A=r;
}

/* $49 ROLA inherent -**** */
M6800_INLINE void rola( void )
{
	UINT16 t,r;
	t = A; r = CC&0x01; r |= t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	A=r;
}

/* $4a DECA inherent -***- */
M6800_INLINE void deca( void )
{
	--A;
	CLR_NZV; SET_FLAGS8D(A);
}

/* $4b ILLEGAL */

/* $4c INCA inherent -***- */
M6800_INLINE void inca( void )
{
	++A;
	CLR_NZV; SET_FLAGS8I(A);
}

/* $4d TSTA inherent -**0- */
M6800_INLINE void tsta( void )
{
	CLR_NZVC; SET_NZ8(A);
}

/* $4e ILLEGAL */

/* $4f CLRA inherent -0100 */
M6800_INLINE void clra( void )
{
	A=0;
	CLR_NZVC; SEZ;
}

/* $50 NEGB inherent ?**** */
M6800_INLINE void negb( void )
{
	UINT16 r;
	r=-B;
	CLR_NZVC; SET_FLAGS8(0,B,r);
	B=r;
}

/* $51 ILLEGAL */

/* $52 ILLEGAL */

/* $53 COMB inherent -**01 */
M6800_INLINE void comb( void )
{
	B = ~B;
	CLR_NZV; SET_NZ8(B); SEC;
}

/* $54 LSRB inherent -0*-* */
M6800_INLINE void lsrb( void )
{
	CLR_NZC; CC|=(B&0x01);
	B>>=1; SET_Z8(B);
}

/* $55 ILLEGAL */

/* $56 RORB inherent -**-* */
M6800_INLINE void rorb( void )
{
	UINT8 r;
	r=(CC&0x01)<<7;
	CLR_NZC; CC|=(B&0x01);
	r |= B>>1; SET_NZ8(r);
	B=r;
}

/* $57 ASRB inherent ?**-* */
M6800_INLINE void asrb( void )
{
	CLR_NZC; CC|=(B&0x01);
	B>>=1; B|=((B&0x40)<<1);
	SET_NZ8(B);
}

/* $58 ASLB inherent ?**** */
M6800_INLINE void aslb( void )
{
	UINT16 r;
	r=B<<1;
	CLR_NZVC; SET_FLAGS8(B,B,r);
	B=r;
}

/* $59 ROLB inherent -**** */
M6800_INLINE void rolb( void )
{
	UINT16 t,r;
	t = B; r = CC&0x01; r |= t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	B=r;
}

/* $5a DECB inherent -***- */
M6800_INLINE void decb( void )
{
	--B;
	CLR_NZV; SET_FLAGS8D(B);
}

/* $5b ILLEGAL */

/* $5c INCB inherent -***- */
M6800_INLINE void incb( void )
{
	++B;
	CLR_NZV; SET_FLAGS8I(B);
}

/* $5d TSTB inherent -**0- */
M6800_INLINE void tstb( void )
{
	CLR_NZVC; SET_NZ8(B);
}

/* $5e ILLEGAL */

/* $5f CLRB inherent -0100 */
M6800_INLINE void clrb( void )
{
	B=0;
	CLR_NZVC; SEZ;
}

/* $60 NEG indexed ?**** */
M6800_INLINE void neg_ix( void )
{
	UINT16 r,t;
	IDXBYTE(t); r=-t;
	CLR_NZVC; SET_FLAGS8(0,t,r);
	WM(EAD,r);
}

/* $61 AIM --**0- */ /* HD63701YO only */
M6800_INLINE void aim_ix( void )
{
	UINT8 t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r &= t;
	CLR_NZV; SET_NZ8(r);
	WM(EAD,r);
}

/* $62 OIM --**0- */ /* HD63701YO only */
M6800_INLINE void oim_ix( void )
{
	UINT8 t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r |= t;
	CLR_NZV; SET_NZ8(r);
	WM(EAD,r);
}

/* $63 COM indexed -**01 */
M6800_INLINE void com_ix( void )
{
	UINT8 t;
	IDXBYTE(t); t = ~t;
	CLR_NZV; SET_NZ8(t); SEC;
	WM(EAD,t);
}

/* $64 LSR indexed -0*-* */
M6800_INLINE void lsr_ix( void )
{
	UINT8 t;
	IDXBYTE(t); CLR_NZC; CC|=(t&0x01);
	t>>=1; SET_Z8(t);
	WM(EAD,t);
}

/* $65 EIM --**0- */ /* HD63701YO only */
M6800_INLINE void eim_ix( void )
{
	UINT8 t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r ^= t;
	CLR_NZV; SET_NZ8(r);
	WM(EAD,r);
}

/* $66 ROR indexed -**-* */
M6800_INLINE void ror_ix( void )
{
	UINT8 t,r;
	IDXBYTE(t); r=(CC&0x01)<<7;
	CLR_NZC; CC|=(t&0x01);
	r |= t>>1; SET_NZ8(r);
	WM(EAD,r);
}

/* $67 ASR indexed ?**-* */
M6800_INLINE void asr_ix( void )
{
	UINT8 t;
	IDXBYTE(t); CLR_NZC; CC|=(t&0x01);
	t>>=1; t|=((t&0x40)<<1);
	SET_NZ8(t);
	WM(EAD,t);
}

/* $68 ASL indexed ?**** */
M6800_INLINE void asl_ix( void )
{
	UINT16 t,r;
	IDXBYTE(t); r=t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $69 ROL indexed -**** */
M6800_INLINE void rol_ix( void )
{
	UINT16 t,r;
	IDXBYTE(t); r = CC&0x01; r |= t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $6a DEC indexed -***- */
M6800_INLINE void dec_ix( void )
{
	UINT8 t;
	IDXBYTE(t); --t;
	CLR_NZV; SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $6b TIM --**0- */ /* HD63701YO only */
M6800_INLINE void tim_ix( void )
{
	UINT8 t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r &= t;
	CLR_NZV; SET_NZ8(r);
}

/* $6c INC indexed -***- */
M6800_INLINE void inc_ix( void )
{
	UINT8 t;
	IDXBYTE(t); ++t;
	CLR_NZV; SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $6d TST indexed -**0- */
M6800_INLINE void tst_ix( void )
{
	UINT8 t;
	IDXBYTE(t); CLR_NZVC; SET_NZ8(t);
}

/* $6e JMP indexed ----- */
M6800_INLINE void jmp_ix( void )
{
	INDEXED; PC=EA; CHANGE_PC();
}

/* $6f CLR indexed -0100 */
M6800_INLINE void clr_ix( void )
{
	INDEXED; WM(EAD,0);
	CLR_NZVC; SEZ;
}

/* $70 NEG extended ?**** */
M6800_INLINE void neg_ex( void )
{
	UINT16 r,t;
	EXTBYTE(t); r=-t;
	CLR_NZVC; SET_FLAGS8(0,t,r);
	WM(EAD,r);
}

/* $71 AIM --**0- */ /* HD63701YO only */
M6800_INLINE void aim_di( void )
{
	UINT8 t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r &= t;
	CLR_NZV; SET_NZ8(r);
	WM(EAD,r);
}

/* $72 OIM --**0- */ /* HD63701YO only */
M6800_INLINE void oim_di( void )
{
	UINT8 t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r |= t;
	CLR_NZV; SET_NZ8(r);
	WM(EAD,r);
}

/* $73 COM extended -**01 */
M6800_INLINE void com_ex( void )
{
	UINT8 t;
	EXTBYTE(t); t = ~t;
	CLR_NZV; SET_NZ8(t); SEC;
	WM(EAD,t);
}

/* $74 LSR extended -0*-* */
M6800_INLINE void lsr_ex( void )
{
	UINT8 t;
	EXTBYTE(t);
	CLR_NZC;
	CC|=(t&0x01);
	t>>=1;
	SET_Z8(t);
	WM(EAD,t);
}

/* $75 EIM --**0- */ /* HD63701YO only */
M6800_INLINE void eim_di( void )
{
	UINT8 t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r ^= t;
	CLR_NZV; SET_NZ8(r);
	WM(EAD,r);
}

/* $76 ROR extended -**-* */
M6800_INLINE void ror_ex( void )
{
	UINT8 t,r;
	EXTBYTE(t); r=(CC&0x01)<<7;
	CLR_NZC; CC|=(t&0x01);
	r |= t>>1; SET_NZ8(r);
	WM(EAD,r);
}

/* $77 ASR extended ?**-* */
M6800_INLINE void asr_ex( void )
{
	UINT8 t;
	EXTBYTE(t); CLR_NZC; CC|=(t&0x01);
	t>>=1; t|=((t&0x40)<<1);
	SET_NZ8(t);
	WM(EAD,t);
}

/* $78 ASL extended ?**** */
M6800_INLINE void asl_ex( void )
{
	UINT16 t,r;
	EXTBYTE(t); r=t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $79 ROL extended -**** */
M6800_INLINE void rol_ex( void )
{
	UINT16 t,r;
	EXTBYTE(t); r = CC&0x01; r |= t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $7a DEC extended -***- */
M6800_INLINE void dec_ex( void )
{
	UINT8 t;
	EXTBYTE(t); --t;
	CLR_NZV; SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $7b TIM --**0- */ /* HD63701YO only */
M6800_INLINE void tim_di( void )
{
	UINT8 t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r &= t;
	CLR_NZV; SET_NZ8(r);
}

/* $7c INC extended -***- */
M6800_INLINE void inc_ex( void )
{
	UINT8 t;
	EXTBYTE(t); ++t;
	CLR_NZV; SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $7d TST extended -**0- */
M6800_INLINE void tst_ex( void )
{
	UINT8 t;
	EXTBYTE(t); CLR_NZVC; SET_NZ8(t);
}

/* $7e JMP extended ----- */
M6800_INLINE void jmp_ex( void )
{
	EXTENDED; PC=EA; CHANGE_PC(); /* TS 971002 */
}

/* $7f CLR extended -0100 */
M6800_INLINE void clr_ex( void )
{
	EXTENDED; WM(EAD,0);
	CLR_NZVC; SEZ;
}

/* $80 SUBA immediate ?**** */
M6800_INLINE void suba_im( void )
{
	UINT16	  t,r;
	IMMBYTE(t); r = A-t;
	CLR_NZVC; SET_FLAGS8(A,t,r);
	A = r;
}

/* $81 CMPA immediate ?**** */
M6800_INLINE void cmpa_im( void )
{
	UINT16	  t,r;
	IMMBYTE(t); r = A-t;
	CLR_NZVC; SET_FLAGS8(A,t,r);
}

/* $82 SBCA immediate ?**** */
M6800_INLINE void sbca_im( void )
{
	UINT16	  t,r;
	IMMBYTE(t); r = A-t-(CC&0x01);
	CLR_NZVC; SET_FLAGS8(A,t,r);
	A = r;
}

/* $83 SUBD immediate -**** */
M6800_INLINE void subd_im( void )
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

/* $84 ANDA immediate -**0- */
M6800_INLINE void anda_im( void )
{
	UINT8 t;
	IMMBYTE(t); A &= t;
	CLR_NZV; SET_NZ8(A);
}

/* $85 BITA immediate -**0- */
M6800_INLINE void bita_im( void )
{
	UINT8 t,r;
	IMMBYTE(t); r = A&t;
	CLR_NZV; SET_NZ8(r);
}

/* $86 LDA immediate -**0- */
M6800_INLINE void lda_im( void )
{
	IMMBYTE(A);
	CLR_NZV; SET_NZ8(A);
}

/* is this a legal instruction? */
/* $87 STA immediate -**0- */
M6800_INLINE void sta_im( void )
{
	CLR_NZV; SET_NZ8(A);
	IMM8; WM(EAD,A);
}

/* $88 EORA immediate -**0- */
M6800_INLINE void eora_im( void )
{
	UINT8 t;
	IMMBYTE(t); A ^= t;
	CLR_NZV; SET_NZ8(A);
}

/* $89 ADCA immediate ***** */
M6800_INLINE void adca_im( void )
{
	UINT16 t,r;
	IMMBYTE(t); r = A+t+(CC&0x01);
	CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
	A = r;
}

/* $8a ORA immediate -**0- */
M6800_INLINE void ora_im( void )
{
	UINT8 t;
	IMMBYTE(t); A |= t;
	CLR_NZV; SET_NZ8(A);
}

/* $8b ADDA immediate ***** */
M6800_INLINE void adda_im( void )
{
	UINT16 t,r;
	IMMBYTE(t); r = A+t;
	CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
	A = r;
}

/* $8c CMPX immediate -***- */
M6800_INLINE void cmpx_im( void )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZV;
	SET_NZ16(r); SET_V16(d,b.d,r);
}

/* $8c CPX immediate -**** (6803) */
M6800_INLINE void cpx_im( void )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC; SET_FLAGS16(d,b.d,r);
}


/* $8d BSR ----- */
M6800_INLINE void bsr( void )
{
	UINT8 t;
	IMMBYTE(t);
	PUSHWORD(pPC);
	PC += SIGNED(t);
	CHANGE_PC();	 /* TS 971002 */
}

/* $8e LDS immediate -**0- */
M6800_INLINE void lds_im( void )
{
	IMMWORD(m6808.s);
	CLR_NZV;
	SET_NZ16(S);
}

/* $8f STS immediate -**0- */
M6800_INLINE void sts_im( void )
{
	CLR_NZV;
	SET_NZ16(S);
	IMM16;
	WM16(EAD,&m6808.s);
}

/* $90 SUBA direct ?**** */
M6800_INLINE void suba_di( void )
{
	UINT16	  t,r;
	DIRBYTE(t); r = A-t;
	CLR_NZVC; SET_FLAGS8(A,t,r);
	A = r;
}

/* $91 CMPA direct ?**** */
M6800_INLINE void cmpa_di( void )
{
	UINT16	  t,r;
	DIRBYTE(t); r = A-t;
	CLR_NZVC; SET_FLAGS8(A,t,r);
}

/* $92 SBCA direct ?**** */
M6800_INLINE void sbca_di( void )
{
	UINT16	  t,r;
	DIRBYTE(t); r = A-t-(CC&0x01);
	CLR_NZVC; SET_FLAGS8(A,t,r);
	A = r;
}

/* $93 SUBD direct -**** */
M6800_INLINE void subd_di( void )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D=r;
}

/* $94 ANDA direct -**0- */
M6800_INLINE void anda_di( void )
{
	UINT8 t;
	DIRBYTE(t); A &= t;
	CLR_NZV; SET_NZ8(A);
}

/* $95 BITA direct -**0- */
M6800_INLINE void bita_di( void )
{
	UINT8 t,r;
	DIRBYTE(t); r = A&t;
	CLR_NZV; SET_NZ8(r);
}

/* $96 LDA direct -**0- */
M6800_INLINE void lda_di( void )
{
	DIRBYTE(A);
	CLR_NZV; SET_NZ8(A);
}

/* $97 STA direct -**0- */
M6800_INLINE void sta_di( void )
{
	CLR_NZV; SET_NZ8(A);
	DIRECT; WM(EAD,A);
}

/* $98 EORA direct -**0- */
M6800_INLINE void eora_di( void )
{
	UINT8 t;
	DIRBYTE(t); A ^= t;
	CLR_NZV; SET_NZ8(A);
}

/* $99 ADCA direct ***** */
M6800_INLINE void adca_di( void )
{
	UINT16 t,r;
	DIRBYTE(t); r = A+t+(CC&0x01);
	CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
	A = r;
}

/* $9a ORA direct -**0- */
M6800_INLINE void ora_di( void )
{
	UINT8 t;
	DIRBYTE(t); A |= t;
	CLR_NZV; SET_NZ8(A);
}

/* $9b ADDA direct ***** */
M6800_INLINE void adda_di( void )
{
	UINT16 t,r;
	DIRBYTE(t); r = A+t;
	CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
	A = r;
}

/* $9c CMPX direct -***- */
M6800_INLINE void cmpx_di( void )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZV;
	SET_NZ16(r); SET_V16(d,b.d,r);
}

/* $9c CPX direct -**** (6803) */
M6800_INLINE void cpx_di( void )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC; SET_FLAGS16(d,b.d,r);
}

/* $9d JSR direct ----- */
M6800_INLINE void jsr_di( void )
{
	DIRECT;
	PUSHWORD(pPC);
    PC = EA;
	CHANGE_PC();
}

/* $9e LDS direct -**0- */
M6800_INLINE void lds_di( void )
{
	DIRWORD(m6808.s);
	CLR_NZV;
	SET_NZ16(S);
}

/* $9f STS direct -**0- */
M6800_INLINE void sts_di( void )
{
	CLR_NZV;
	SET_NZ16(S);
	DIRECT;
	WM16(EAD,&m6808.s);
}

/* $a0 SUBA indexed ?**** */
M6800_INLINE void suba_ix( void )
{
	UINT16	  t,r;
	IDXBYTE(t); r = A-t;
	CLR_NZVC; SET_FLAGS8(A,t,r);
	A = r;
}

/* $a1 CMPA indexed ?**** */
M6800_INLINE void cmpa_ix( void )
{
	UINT16	  t,r;
	IDXBYTE(t); r = A-t;
	CLR_NZVC; SET_FLAGS8(A,t,r);
}

/* $a2 SBCA indexed ?**** */
M6800_INLINE void sbca_ix( void )
{
	UINT16	  t,r;
	IDXBYTE(t); r = A-t-(CC&0x01);
	CLR_NZVC; SET_FLAGS8(A,t,r);
	A = r;
}

/* $a3 SUBD indexed -**** */
M6800_INLINE void subd_ix( void )
{
	UINT32 r,d;
	PAIR b;
	IDXWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $a4 ANDA indexed -**0- */
M6800_INLINE void anda_ix( void )
{
	UINT8 t;
	IDXBYTE(t); A &= t;
	CLR_NZV; SET_NZ8(A);
}

/* $a5 BITA indexed -**0- */
M6800_INLINE void bita_ix( void )
{
	UINT8 t,r;
	IDXBYTE(t); r = A&t;
	CLR_NZV; SET_NZ8(r);
}

/* $a6 LDA indexed -**0- */
M6800_INLINE void lda_ix( void )
{
	IDXBYTE(A);
	CLR_NZV; SET_NZ8(A);
}

/* $a7 STA indexed -**0- */
M6800_INLINE void sta_ix( void )
{
	CLR_NZV; SET_NZ8(A);
	INDEXED; WM(EAD,A);
}

/* $a8 EORA indexed -**0- */
M6800_INLINE void eora_ix( void )
{
	UINT8 t;
	IDXBYTE(t); A ^= t;
	CLR_NZV; SET_NZ8(A);
}

/* $a9 ADCA indexed ***** */
M6800_INLINE void adca_ix( void )
{
	UINT16 t,r;
	IDXBYTE(t); r = A+t+(CC&0x01);
	CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
	A = r;
}

/* $aa ORA indexed -**0- */
M6800_INLINE void ora_ix( void )
{
	UINT8 t;
	IDXBYTE(t); A |= t;
	CLR_NZV; SET_NZ8(A);
}

/* $ab ADDA indexed ***** */
M6800_INLINE void adda_ix( void )
{
	UINT16 t,r;
	IDXBYTE(t); r = A+t;
	CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
	A = r;
}

/* $ac CMPX indexed -***- */
M6800_INLINE void cmpx_ix( void )
{
	UINT32 r,d;
	PAIR b;
	IDXWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZV;
	SET_NZ16(r); SET_V16(d,b.d,r);
}

/* $ac CPX indexed -**** (6803)*/
M6800_INLINE void cpx_ix( void )
{
	UINT32 r,d;
	PAIR b;
	IDXWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC; SET_FLAGS16(d,b.d,r);
}

/* $ad JSR indexed ----- */
M6800_INLINE void jsr_ix( void )
{
	INDEXED;
	PUSHWORD(pPC);
    PC = EA;
	CHANGE_PC();
}

/* $ae LDS indexed -**0- */
M6800_INLINE void lds_ix( void )
{
	IDXWORD(m6808.s);
	CLR_NZV;
	SET_NZ16(S);
}

/* $af STS indexed -**0- */
M6800_INLINE void sts_ix( void )
{
	CLR_NZV;
	SET_NZ16(S);
	INDEXED;
	WM16(EAD,&m6808.s);
}

/* $b0 SUBA extended ?**** */
M6800_INLINE void suba_ex( void )
{
	UINT16	  t,r;
	EXTBYTE(t); r = A-t;
	CLR_NZVC; SET_FLAGS8(A,t,r);
	A = r;
}

/* $b1 CMPA extended ?**** */
M6800_INLINE void cmpa_ex( void )
{
	UINT16	  t,r;
	EXTBYTE(t); r = A-t;
	CLR_NZVC; SET_FLAGS8(A,t,r);
}

/* $b2 SBCA extended ?**** */
M6800_INLINE void sbca_ex( void )
{
	UINT16	  t,r;
	EXTBYTE(t); r = A-t-(CC&0x01);
	CLR_NZVC; SET_FLAGS8(A,t,r);
	A = r;
}

/* $b3 SUBD extended -**** */
M6800_INLINE void subd_ex( void )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D=r;
}

/* $b4 ANDA extended -**0- */
M6800_INLINE void anda_ex( void )
{
	UINT8 t;
	EXTBYTE(t); A &= t;
	CLR_NZV; SET_NZ8(A);
}

/* $b5 BITA extended -**0- */
M6800_INLINE void bita_ex( void )
{
	UINT8 t,r;
	EXTBYTE(t); r = A&t;
	CLR_NZV; SET_NZ8(r);
}

/* $b6 LDA extended -**0- */
M6800_INLINE void lda_ex( void )
{
	EXTBYTE(A);
	CLR_NZV; SET_NZ8(A);
}

/* $b7 STA extended -**0- */
M6800_INLINE void sta_ex( void )
{
	CLR_NZV; SET_NZ8(A);
	EXTENDED; WM(EAD,A);
}

/* $b8 EORA extended -**0- */
M6800_INLINE void eora_ex( void )
{
	UINT8 t;
	EXTBYTE(t); A ^= t;
	CLR_NZV; SET_NZ8(A);
}

/* $b9 ADCA extended ***** */
M6800_INLINE void adca_ex( void )
{
	UINT16 t,r;
	EXTBYTE(t); r = A+t+(CC&0x01);
	CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
	A = r;
}

/* $ba ORA extended -**0- */
M6800_INLINE void ora_ex( void )
{
	UINT8 t;
	EXTBYTE(t); A |= t;
	CLR_NZV; SET_NZ8(A);
}

/* $bb ADDA extended ***** */
M6800_INLINE void adda_ex( void )
{
	UINT16 t,r;
	EXTBYTE(t); r = A+t;
	CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
	A = r;
}

/* $bc CMPX extended -***- */
M6800_INLINE void cmpx_ex( void )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZV;
	SET_NZ16(r); SET_V16(d,b.d,r);
}

/* $bc CPX extended -**** (6803) */
M6800_INLINE void cpx_ex( void )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC; SET_FLAGS16(d,b.d,r);
}

/* $bd JSR extended ----- */
M6800_INLINE void jsr_ex( void )
{
	EXTENDED;
	PUSHWORD(pPC);
    PC = EA;
	CHANGE_PC();
}

/* $be LDS extended -**0- */
M6800_INLINE void lds_ex( void )
{
	EXTWORD(m6808.s);
	CLR_NZV;
	SET_NZ16(S);
}

/* $bf STS extended -**0- */
M6800_INLINE void sts_ex( void )
{
	CLR_NZV;
	SET_NZ16(S);
	EXTENDED;
	WM16(EAD,&m6808.s);
}

/* $c0 SUBB immediate ?**** */
M6800_INLINE void subb_im( void )
{
	UINT16	  t,r;
	IMMBYTE(t); r = B-t;
	CLR_NZVC; SET_FLAGS8(B,t,r);
	B = r;
}

/* $c1 CMPB immediate ?**** */
M6800_INLINE void cmpb_im( void )
{
	UINT16	  t,r;
	IMMBYTE(t); r = B-t;
	CLR_NZVC; SET_FLAGS8(B,t,r);
}

/* $c2 SBCB immediate ?**** */
M6800_INLINE void sbcb_im( void )
{
	UINT16	  t,r;
	IMMBYTE(t); r = B-t-(CC&0x01);
	CLR_NZVC; SET_FLAGS8(B,t,r);
	B = r;
}

/* $c3 ADDD immediate -**** */
M6800_INLINE void addd_im( void )
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
M6800_INLINE void andb_im( void )
{
	UINT8 t;
	IMMBYTE(t); B &= t;
	CLR_NZV; SET_NZ8(B);
}

/* $c5 BITB immediate -**0- */
M6800_INLINE void bitb_im( void )
{
	UINT8 t,r;
	IMMBYTE(t); r = B&t;
	CLR_NZV; SET_NZ8(r);
}

/* $c6 LDB immediate -**0- */
M6800_INLINE void ldb_im( void )
{
	IMMBYTE(B);
	CLR_NZV; SET_NZ8(B);
}

/* is this a legal instruction? */
/* $c7 STB immediate -**0- */
M6800_INLINE void stb_im( void )
{
	CLR_NZV; SET_NZ8(B);
	IMM8; WM(EAD,B);
}

/* $c8 EORB immediate -**0- */
M6800_INLINE void eorb_im( void )
{
	UINT8 t;
	IMMBYTE(t); B ^= t;
	CLR_NZV; SET_NZ8(B);
}

/* $c9 ADCB immediate ***** */
M6800_INLINE void adcb_im( void )
{
	UINT16 t,r;
	IMMBYTE(t); r = B+t+(CC&0x01);
	CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
	B = r;
}

/* $ca ORB immediate -**0- */
M6800_INLINE void orb_im( void )
{
	UINT8 t;
	IMMBYTE(t); B |= t;
	CLR_NZV; SET_NZ8(B);
}

/* $cb ADDB immediate ***** */
M6800_INLINE void addb_im( void )
{
	UINT16 t,r;
	IMMBYTE(t); r = B+t;
	CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
	B = r;
}

/* $CC LDD immediate -**0- */
M6800_INLINE void ldd_im( void )
{
	IMMWORD(m6808.d);
	CLR_NZV;
	SET_NZ16(D);
}

/* is this a legal instruction? */
/* $cd STD immediate -**0- */
M6800_INLINE void std_im( void )
{
	IMM16;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD,&m6808.d);
}

/* $ce LDX immediate -**0- */
M6800_INLINE void ldx_im( void )
{
	IMMWORD(m6808.x);
	CLR_NZV;
	SET_NZ16(X);
}

/* $cf STX immediate -**0- */
M6800_INLINE void stx_im( void )
{
	CLR_NZV;
	SET_NZ16(X);
	IMM16;
	WM16(EAD,&m6808.x);
}

/* $d0 SUBB direct ?**** */
M6800_INLINE void subb_di( void )
{
	UINT16	  t,r;
	DIRBYTE(t); r = B-t;
	CLR_NZVC; SET_FLAGS8(B,t,r);
	B = r;
}

/* $d1 CMPB direct ?**** */
M6800_INLINE void cmpb_di( void )
{
	UINT16	  t,r;
	DIRBYTE(t); r = B-t;
	CLR_NZVC; SET_FLAGS8(B,t,r);
}

/* $d2 SBCB direct ?**** */
M6800_INLINE void sbcb_di( void )
{
	UINT16	  t,r;
	DIRBYTE(t); r = B-t-(CC&0x01);
	CLR_NZVC; SET_FLAGS8(B,t,r);
	B = r;
}

/* $d3 ADDD direct -**** */
M6800_INLINE void addd_di( void )
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
M6800_INLINE void andb_di( void )
{
	UINT8 t;
	DIRBYTE(t); B &= t;
	CLR_NZV; SET_NZ8(B);
}

/* $d5 BITB direct -**0- */
M6800_INLINE void bitb_di( void )
{
	UINT8 t,r;
	DIRBYTE(t); r = B&t;
	CLR_NZV; SET_NZ8(r);
}

/* $d6 LDB direct -**0- */
M6800_INLINE void ldb_di( void )
{
	DIRBYTE(B);
	CLR_NZV; SET_NZ8(B);
}

/* $d7 STB direct -**0- */
M6800_INLINE void stb_di( void )
{
	CLR_NZV; SET_NZ8(B);
	DIRECT; WM(EAD,B);
}

/* $d8 EORB direct -**0- */
M6800_INLINE void eorb_di( void )
{
	UINT8 t;
	DIRBYTE(t); B ^= t;
	CLR_NZV; SET_NZ8(B);
}

/* $d9 ADCB direct ***** */
M6800_INLINE void adcb_di( void )
{
	UINT16 t,r;
	DIRBYTE(t); r = B+t+(CC&0x01);
	CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
	B = r;
}

/* $da ORB direct -**0- */
M6800_INLINE void orb_di( void )
{
	UINT8 t;
	DIRBYTE(t); B |= t;
	CLR_NZV; SET_NZ8(B);
}

/* $db ADDB direct ***** */
M6800_INLINE void addb_di( void )
{
	UINT16 t,r;
	DIRBYTE(t); r = B+t;
	CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
	B = r;
}

/* $dc LDD direct -**0- */
M6800_INLINE void ldd_di( void )
{
	DIRWORD(m6808.d);
	CLR_NZV;
	SET_NZ16(D);
}

/* $dd STD direct -**0- */
M6800_INLINE void std_di( void )
{
	DIRECT;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD,&m6808.d);
}

/* $de LDX direct -**0- */
M6800_INLINE void ldx_di( void )
{
	DIRWORD(m6808.x);
	CLR_NZV;
	SET_NZ16(X);
}

/* $dF STX direct -**0- */
M6800_INLINE void stx_di( void )
{
	CLR_NZV;
	SET_NZ16(X);
	DIRECT;
	WM16(EAD,&m6808.x);
}

/* $e0 SUBB indexed ?**** */
M6800_INLINE void subb_ix( void )
{
	UINT16	  t,r;
	IDXBYTE(t); r = B-t;
	CLR_NZVC; SET_FLAGS8(B,t,r);
	B = r;
}

/* $e1 CMPB indexed ?**** */
M6800_INLINE void cmpb_ix( void )
{
	UINT16	  t,r;
	IDXBYTE(t); r = B-t;
	CLR_NZVC; SET_FLAGS8(B,t,r);
}

/* $e2 SBCB indexed ?**** */
M6800_INLINE void sbcb_ix( void )
{
	UINT16	  t,r;
	IDXBYTE(t); r = B-t-(CC&0x01);
	CLR_NZVC; SET_FLAGS8(B,t,r);
	B = r;
}

/* $e3 ADDD indexed -**** */
M6800_INLINE void addd_ix( void )
{
	UINT32 r,d;
	PAIR b;
	IDXWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $e4 ANDB indexed -**0- */
M6800_INLINE void andb_ix( void )
{
	UINT8 t;
	IDXBYTE(t); B &= t;
	CLR_NZV; SET_NZ8(B);
}

/* $e5 BITB indexed -**0- */
M6800_INLINE void bitb_ix( void )
{
	UINT8 t,r;
	IDXBYTE(t); r = B&t;
	CLR_NZV; SET_NZ8(r);
}

/* $e6 LDB indexed -**0- */
M6800_INLINE void ldb_ix( void )
{
	IDXBYTE(B);
	CLR_NZV; SET_NZ8(B);
}

/* $e7 STB indexed -**0- */
M6800_INLINE void stb_ix( void )
{
	CLR_NZV; SET_NZ8(B);
	INDEXED; WM(EAD,B);
}

/* $e8 EORB indexed -**0- */
M6800_INLINE void eorb_ix( void )
{
	UINT8 t;
	IDXBYTE(t); B ^= t;
	CLR_NZV; SET_NZ8(B);
}

/* $e9 ADCB indexed ***** */
M6800_INLINE void adcb_ix( void )
{
	UINT16 t,r;
	IDXBYTE(t); r = B+t+(CC&0x01);
	CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
	B = r;
}

/* $ea ORB indexed -**0- */
M6800_INLINE void orb_ix( void )
{
	UINT8 t;
	IDXBYTE(t); B |= t;
	CLR_NZV; SET_NZ8(B);
}

/* $eb ADDB indexed ***** */
M6800_INLINE void addb_ix( void )
{
	UINT16 t,r;
	IDXBYTE(t); r = B+t;
	CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
	B = r;
}

/* $ec LDD indexed -**0- */
M6800_INLINE void ldd_ix( void )
{
	IDXWORD(m6808.d);
	CLR_NZV;
	SET_NZ16(D);
}

#if 0
/* $ec ADCX immediate -****    NSC8105 only.  Flags are a guess - copied from addb_im() */
M6800_INLINE void adcx_im( void )
{
	UINT16 t,r;
	IMMBYTE(t); r = X+t+(CC&0x01);
	CLR_HNZVC; SET_FLAGS8(X,t,r); SET_H(X,t,r);
	X = r;
}
#endif

/* $ed STD indexed -**0- */
M6800_INLINE void std_ix( void )
{
	INDEXED;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD,&m6808.d);
}

/* $ee LDX indexed -**0- */
M6800_INLINE void ldx_ix( void )
{
	IDXWORD(m6808.x);
	CLR_NZV;
	SET_NZ16(X);
}

/* $ef STX indexed -**0- */
M6800_INLINE void stx_ix( void )
{
	CLR_NZV;
	SET_NZ16(X);
	INDEXED;
	WM16(EAD,&m6808.x);
}

/* $f0 SUBB extended ?**** */
M6800_INLINE void subb_ex( void )
{
	UINT16	  t,r;
	EXTBYTE(t); r = B-t;
	CLR_NZVC; SET_FLAGS8(B,t,r);
	B = r;
}

/* $f1 CMPB extended ?**** */
M6800_INLINE void cmpb_ex( void )
{
	UINT16	  t,r;
	EXTBYTE(t); r = B-t;
	CLR_NZVC; SET_FLAGS8(B,t,r);
}

/* $f2 SBCB extended ?**** */
M6800_INLINE void sbcb_ex( void )
{
	UINT16	  t,r;
	EXTBYTE(t); r = B-t-(CC&0x01);
	CLR_NZVC; SET_FLAGS8(B,t,r);
	B = r;
}

/* $f3 ADDD extended -**** */
M6800_INLINE void addd_ex( void )
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
M6800_INLINE void andb_ex( void )
{
	UINT8 t;
	EXTBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $f5 BITB extended -**0- */
M6800_INLINE void bitb_ex( void )
{
	UINT8 t,r;
	EXTBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $f6 LDB extended -**0- */
M6800_INLINE void ldb_ex( void )
{
	EXTBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $f7 STB extended -**0- */
M6800_INLINE void stb_ex( void )
{
	CLR_NZV; SET_NZ8(B);
	EXTENDED; WM(EAD,B);
}

/* $f8 EORB extended -**0- */
M6800_INLINE void eorb_ex( void )
{
	UINT8 t;
	EXTBYTE(t); B ^= t;
	CLR_NZV; SET_NZ8(B);
}

/* $f9 ADCB extended ***** */
M6800_INLINE void adcb_ex( void )
{
	UINT16 t,r;
	EXTBYTE(t); r = B+t+(CC&0x01);
	CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
	B = r;
}

/* $fa ORB extended -**0- */
M6800_INLINE void orb_ex( void )
{
	UINT8 t;
	EXTBYTE(t); B |= t;
	CLR_NZV; SET_NZ8(B);
}

/* $fb ADDB extended ***** */
M6800_INLINE void addb_ex( void )
{
	UINT16 t,r;
	EXTBYTE(t); r = B+t;
	CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
	B = r;
}

/* $fc LDD extended -**0- */
M6800_INLINE void ldd_ex( void )
{
	EXTWORD(m6808.d);
	CLR_NZV;
	SET_NZ16(D);
}

/* $fc ADDX extended -****    NSC8105 only.  Flags are a guess */
M6800_INLINE void addx_ex( void )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = X;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	X = r;
}

/* $fd STD extended -**0- */
M6800_INLINE void std_ex( void )
{
	EXTENDED;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD,&m6808.d);
}

/* $fe LDX extended -**0- */
M6800_INLINE void ldx_ex( void )
{
	EXTWORD(m6808.x);
	CLR_NZV;
	SET_NZ16(X);
}

/* $ff STX extended -**0- */
M6800_INLINE void stx_ex( void )
{
	CLR_NZV;
	SET_NZ16(X);
	EXTENDED;
	WM16(EAD,&m6808.x);
}
