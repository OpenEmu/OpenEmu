#define OP(num,func_name) static void func_name(nec_state_t *nec_state)

OP( 0x00, i_add_br8  ) { DEF_br8;	ADDB;	PutbackRMByte(ModRM,dst);	CLKM(2,2,2,16,16,7);		}
OP( 0x01, i_add_wr16 ) { DEF_wr16;	ADDW;	PutbackRMWord(ModRM,dst);	CLKR(24,24,11,24,16,7,2,EA);}
OP( 0x02, i_add_r8b  ) { DEF_r8b;	ADDB;	RegByte(ModRM)=dst;			CLKM(2,2,2,11,11,6);		}
OP( 0x03, i_add_r16w ) { DEF_r16w;	ADDW;	RegWord(ModRM)=dst;			CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x04, i_add_ald8 ) { DEF_ald8;	ADDB;	Breg(AL)=dst;			CLKS(4,4,2);				}
OP( 0x05, i_add_axd16) { DEF_axd16;	ADDW;	Wreg(AW)=dst;			CLKS(4,4,2);				}
OP( 0x06, i_push_es  ) { PUSH(Sreg(DS1));	CLKS(12,8,3);	}
OP( 0x07, i_pop_es   ) { POP(Sreg(DS1));	CLKS(12,8,5);	}

OP( 0x08, i_or_br8   ) { DEF_br8;	ORB;	PutbackRMByte(ModRM,dst);	CLKM(2,2,2,16,16,7);		}
OP( 0x09, i_or_wr16  ) { DEF_wr16;	ORW;	PutbackRMWord(ModRM,dst);	CLKR(24,24,11,24,16,7,2,EA);}
OP( 0x0a, i_or_r8b   ) { DEF_r8b;	ORB;	RegByte(ModRM)=dst;			CLKM(2,2,2,11,11,6);		}
OP( 0x0b, i_or_r16w  ) { DEF_r16w;	ORW;	RegWord(ModRM)=dst;			CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x0c, i_or_ald8  ) { DEF_ald8;	ORB;	Breg(AL)=dst;			CLKS(4,4,2);				}
OP( 0x0d, i_or_axd16 ) { DEF_axd16;	ORW;	Wreg(AW)=dst;			CLKS(4,4,2);				}
OP( 0x0e, i_push_cs  ) { PUSH(Sreg(PS));	CLKS(12,8,3);	}
#if defined NEC_CPU_CPP
OP( 0x0f, i_pre_nec  ) { UINT32 ModRM, tmp, tmp2;
	switch (FETCH()) {
		case 0x10 : BITOP_BYTE;	CLKS(3,3,4); tmp2 = Breg(CL) & 0x7;	nec_state->ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;	nec_state->CarryVal=nec_state->OverVal=0; break; /* Test */
		case 0x11 : BITOP_WORD;	CLKS(3,3,4); tmp2 = Breg(CL) & 0xf;	nec_state->ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;	nec_state->CarryVal=nec_state->OverVal=0; break; /* Test */
		case 0x12 : BITOP_BYTE;	CLKS(5,5,4); tmp2 = Breg(CL) & 0x7;	tmp &= ~(1<<tmp2);	PutbackRMByte(ModRM,tmp);	break; /* Clr */
		case 0x13 : BITOP_WORD;	CLKS(5,5,4); tmp2 = Breg(CL) & 0xf;	tmp &= ~(1<<tmp2);	PutbackRMWord(ModRM,tmp);	break; /* Clr */
		case 0x14 : BITOP_BYTE;	CLKS(4,4,4); tmp2 = Breg(CL) & 0x7;	tmp |= (1<<tmp2);	PutbackRMByte(ModRM,tmp);	break; /* Set */
		case 0x15 : BITOP_WORD;	CLKS(4,4,4); tmp2 = Breg(CL) & 0xf;	tmp |= (1<<tmp2);	PutbackRMWord(ModRM,tmp);	break; /* Set */
		case 0x16 : BITOP_BYTE;	CLKS(4,4,4); tmp2 = Breg(CL) & 0x7;	BIT_NOT;			PutbackRMByte(ModRM,tmp);	break; /* Not */
		case 0x17 : BITOP_WORD;	CLKS(4,4,4); tmp2 = Breg(CL) & 0xf;	BIT_NOT;			PutbackRMWord(ModRM,tmp);	break; /* Not */

		case 0x18 : BITOP_BYTE;	CLKS(4,4,4); tmp2 = (FETCH()) & 0x7;	nec_state->ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;	nec_state->CarryVal=nec_state->OverVal=0; break; /* Test */
		case 0x19 : BITOP_WORD;	CLKS(4,4,4); tmp2 = (FETCH()) & 0xf;	nec_state->ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;	nec_state->CarryVal=nec_state->OverVal=0; break; /* Test */
		case 0x1a : BITOP_BYTE;	CLKS(6,6,4); tmp2 = (FETCH()) & 0x7;	tmp &= ~(1<<tmp2);		PutbackRMByte(ModRM,tmp);	break; /* Clr */
		case 0x1b : BITOP_WORD;	CLKS(6,6,4); tmp2 = (FETCH()) & 0xf;	tmp &= ~(1<<tmp2);		PutbackRMWord(ModRM,tmp);	break; /* Clr */
		case 0x1c : BITOP_BYTE;	CLKS(5,5,4); tmp2 = (FETCH()) & 0x7;	tmp |= (1<<tmp2);		PutbackRMByte(ModRM,tmp);	break; /* Set */
		case 0x1d : BITOP_WORD;	CLKS(5,5,4); tmp2 = (FETCH()) & 0xf;	tmp |= (1<<tmp2);		PutbackRMWord(ModRM,tmp);	break; /* Set */
		case 0x1e : BITOP_BYTE;	CLKS(5,5,4); tmp2 = (FETCH()) & 0x7;	BIT_NOT;				PutbackRMByte(ModRM,tmp);	break; /* Not */
		case 0x1f : BITOP_WORD;	CLKS(5,5,4); tmp2 = (FETCH()) & 0xf;	BIT_NOT;				PutbackRMWord(ModRM,tmp);	break; /* Not */

		case 0x20 :	ADD4S; CLKS(7,7,2); break;
		case 0x22 :	SUB4S; CLKS(7,7,2); break;
		case 0x26 :	CMP4S; CLKS(7,7,2); break;
		case 0x28 : ModRM = FETCH(); tmp = GetRMByte(ModRM); tmp <<= 4; tmp |= Breg(AL) & 0xf; Breg(AL) = (Breg(AL) & 0xf0) | ((tmp>>8)&0xf); tmp &= 0xff; PutbackRMByte(ModRM,tmp); CLKM(13,13,9,28,28,15); break;
		case 0x2a : ModRM = FETCH(); tmp = GetRMByte(ModRM); tmp2 = (Breg(AL) & 0xf)<<4; Breg(AL) = (Breg(AL) & 0xf0) | (tmp&0xf); tmp = tmp2 | (tmp>>4);	PutbackRMByte(ModRM,tmp); CLKM(17,17,13,32,32,19); break;
		case 0x31 : ModRM = FETCH(); ModRM=0; break;
		case 0x33 : ModRM = FETCH(); ModRM=0; break;
		case 0xe0 : ModRM = FETCH(); ModRM=0; break;
		case 0xf0 : ModRM = FETCH(); ModRM=0; break;
		case 0xff : ModRM = FETCH(); ModRM=0; break;
//		default:    logerror("%06x: Unknown V20 instruction\n",PC(nec_state)); break;
	}
}
#endif

OP( 0x10, i_adc_br8  ) { DEF_br8;	src+=CF;	ADDB;	PutbackRMByte(ModRM,dst);	CLKM(2,2,2,16,16,7);		}
OP( 0x11, i_adc_wr16 ) { DEF_wr16;	src+=CF;	ADDW;	PutbackRMWord(ModRM,dst);	CLKR(24,24,11,24,16,7,2,EA);}
OP( 0x12, i_adc_r8b  ) { DEF_r8b;	src+=CF;	ADDB;	RegByte(ModRM)=dst;			CLKM(2,2,2,11,11,6);		}
OP( 0x13, i_adc_r16w ) { DEF_r16w;	src+=CF;	ADDW;	RegWord(ModRM)=dst;			CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x14, i_adc_ald8 ) { DEF_ald8;	src+=CF;	ADDB;	Breg(AL)=dst;			CLKS(4,4,2);				}
OP( 0x15, i_adc_axd16) { DEF_axd16;	src+=CF;	ADDW;	Wreg(AW)=dst;			CLKS(4,4,2);				}
OP( 0x16, i_push_ss  ) { PUSH(Sreg(SS));		CLKS(12,8,3);	}
OP( 0x17, i_pop_ss   ) { POP(Sreg(SS));		CLKS(12,8,5);	nec_state->no_interrupt=1; }

OP( 0x18, i_sbb_br8  ) { DEF_br8;	src+=CF;	SUBB;	PutbackRMByte(ModRM,dst);	CLKM(2,2,2,16,16,7);		}
OP( 0x19, i_sbb_wr16 ) { DEF_wr16;	src+=CF;	SUBW;	PutbackRMWord(ModRM,dst);	CLKR(24,24,11,24,16,7,2,EA);}
OP( 0x1a, i_sbb_r8b  ) { DEF_r8b;	src+=CF;	SUBB;	RegByte(ModRM)=dst;			CLKM(2,2,2,11,11,6);		}
OP( 0x1b, i_sbb_r16w ) { DEF_r16w;	src+=CF;	SUBW;	RegWord(ModRM)=dst;			CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x1c, i_sbb_ald8 ) { DEF_ald8;	src+=CF;	SUBB;	Breg(AL)=dst;			CLKS(4,4,2);				}
OP( 0x1d, i_sbb_axd16) { DEF_axd16;	src+=CF;	SUBW;	Wreg(AW)=dst;			CLKS(4,4,2);	}
OP( 0x1e, i_push_ds  ) { PUSH(Sreg(DS0));		CLKS(12,8,3);	}
OP( 0x1f, i_pop_ds   ) { POP(Sreg(DS0));		CLKS(12,8,5);	}

OP( 0x20, i_and_br8  ) { DEF_br8;	ANDB;	PutbackRMByte(ModRM,dst);	CLKM(2,2,2,16,16,7);		}
OP( 0x21, i_and_wr16 ) { DEF_wr16;	ANDW;	PutbackRMWord(ModRM,dst);	CLKR(24,24,11,24,16,7,2,EA);}
OP( 0x22, i_and_r8b  ) { DEF_r8b;	ANDB;	RegByte(ModRM)=dst;			CLKM(2,2,2,11,11,6);		}
OP( 0x23, i_and_r16w ) { DEF_r16w;	ANDW;	RegWord(ModRM)=dst;			CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x24, i_and_ald8 ) { DEF_ald8;	ANDB;	Breg(AL)=dst;			CLKS(4,4,2);				}
OP( 0x25, i_and_axd16) { DEF_axd16;	ANDW;	Wreg(AW)=dst;			CLKS(4,4,2);	}
OP( 0x26, i_es       ) { nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(DS1)<<4;	CLK(2);		nec_instruction[fetchop(nec_state)](nec_state);	nec_state->seg_prefix=FALSE; }
OP( 0x27, i_daa      ) { ADJ4(6,0x60);									CLKS(3,3,2);	}

OP( 0x28, i_sub_br8  ) { DEF_br8;	SUBB;	PutbackRMByte(ModRM,dst);	CLKM(2,2,2,16,16,7);		}
OP( 0x29, i_sub_wr16 ) { DEF_wr16;	SUBW;	PutbackRMWord(ModRM,dst);	CLKR(24,24,11,24,16,7,2,EA);}
OP( 0x2a, i_sub_r8b  ) { DEF_r8b;	SUBB;	RegByte(ModRM)=dst;			CLKM(2,2,2,11,11,6);		}
OP( 0x2b, i_sub_r16w ) { DEF_r16w;	SUBW;	RegWord(ModRM)=dst;			CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x2c, i_sub_ald8 ) { DEF_ald8;	SUBB;	Breg(AL)=dst;			CLKS(4,4,2);				}
OP( 0x2d, i_sub_axd16) { DEF_axd16;	SUBW;	Wreg(AW)=dst;			CLKS(4,4,2);	}
OP( 0x2e, i_cs       ) { nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(PS)<<4;	CLK(2);		nec_instruction[fetchop(nec_state)](nec_state);	nec_state->seg_prefix=FALSE; }
OP( 0x2f, i_das      ) { ADJ4(-6,-0x60);								CLKS(3,3,2);	}

OP( 0x30, i_xor_br8  ) { DEF_br8;	XORB;	PutbackRMByte(ModRM,dst);	CLKM(2,2,2,16,16,7);		}
OP( 0x31, i_xor_wr16 ) { DEF_wr16;	XORW;	PutbackRMWord(ModRM,dst);	CLKR(24,24,11,24,16,7,2,EA);}
OP( 0x32, i_xor_r8b  ) { DEF_r8b;	XORB;	RegByte(ModRM)=dst;			CLKM(2,2,2,11,11,6);		}
OP( 0x33, i_xor_r16w ) { DEF_r16w;	XORW;	RegWord(ModRM)=dst;			CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x34, i_xor_ald8 ) { DEF_ald8;	XORB;	Breg(AL)=dst;			CLKS(4,4,2);				}
OP( 0x35, i_xor_axd16) { DEF_axd16;	XORW;	Wreg(AW)=dst;			CLKS(4,4,2);	}
OP( 0x36, i_ss       ) { nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(SS)<<4;	CLK(2);		nec_instruction[fetchop(nec_state)](nec_state);	nec_state->seg_prefix=FALSE; }
OP( 0x37, i_aaa      ) { ADJB(6, (Breg(AL) > 0xf9) ? 2 : 1);		CLKS(7,7,4);	}

OP( 0x38, i_cmp_br8  ) { DEF_br8;	SUBB;					CLKM(2,2,2,11,11,6); }
OP( 0x39, i_cmp_wr16 ) { DEF_wr16;	SUBW;					CLKR(15,15,8,15,11,6,2,EA);}
OP( 0x3a, i_cmp_r8b  ) { DEF_r8b;	SUBB;					CLKM(2,2,2,11,11,6); }
OP( 0x3b, i_cmp_r16w ) { DEF_r16w;	SUBW;					CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x3c, i_cmp_ald8 ) { DEF_ald8;	SUBB;					CLKS(4,4,2); }
OP( 0x3d, i_cmp_axd16) { DEF_axd16;	SUBW;					CLKS(4,4,2);	}
OP( 0x3e, i_ds       ) { nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(DS0)<<4;	CLK(2);		nec_instruction[fetchop(nec_state)](nec_state);	nec_state->seg_prefix=FALSE; }
OP( 0x3f, i_aas      ) { ADJB(-6, (Breg(AL) < 6) ? -2 : -1);		CLKS(7,7,4);	}

OP( 0x40, i_inc_ax  ) { IncWordReg(AW);						CLK(2);	}
OP( 0x41, i_inc_cx  ) { IncWordReg(CW);						CLK(2);	}
OP( 0x42, i_inc_dx  ) { IncWordReg(DW);						CLK(2);	}
OP( 0x43, i_inc_bx  ) { IncWordReg(BW);						CLK(2);	}
OP( 0x44, i_inc_sp  ) { IncWordReg(SP);						CLK(2);	}
OP( 0x45, i_inc_bp  ) { IncWordReg(BP);						CLK(2);	}
OP( 0x46, i_inc_si  ) { IncWordReg(IX);						CLK(2);	}
OP( 0x47, i_inc_di  ) { IncWordReg(IY);						CLK(2);	}

OP( 0x48, i_dec_ax  ) { DecWordReg(AW);						CLK(2);	}
OP( 0x49, i_dec_cx  ) { DecWordReg(CW);						CLK(2);	}
OP( 0x4a, i_dec_dx  ) { DecWordReg(DW);						CLK(2);	}
OP( 0x4b, i_dec_bx  ) { DecWordReg(BW);						CLK(2);	}
OP( 0x4c, i_dec_sp  ) { DecWordReg(SP);						CLK(2);	}
OP( 0x4d, i_dec_bp  ) { DecWordReg(BP);						CLK(2);	}
OP( 0x4e, i_dec_si  ) { DecWordReg(IX);						CLK(2);	}
OP( 0x4f, i_dec_di  ) { DecWordReg(IY);						CLK(2);	}

OP( 0x50, i_push_ax ) { PUSH(Wreg(AW));					CLKS(12,8,3); }
OP( 0x51, i_push_cx ) { PUSH(Wreg(CW));					CLKS(12,8,3); }
OP( 0x52, i_push_dx ) { PUSH(Wreg(DW));					CLKS(12,8,3); }
OP( 0x53, i_push_bx ) { PUSH(Wreg(BW));					CLKS(12,8,3); }
OP( 0x54, i_push_sp ) { PUSH(Wreg(SP));					CLKS(12,8,3); }
OP( 0x55, i_push_bp ) { PUSH(Wreg(BP));					CLKS(12,8,3); }
OP( 0x56, i_push_si ) { PUSH(Wreg(IX));					CLKS(12,8,3); }
OP( 0x57, i_push_di ) { PUSH(Wreg(IY));					CLKS(12,8,3); }

OP( 0x58, i_pop_ax  ) { POP(Wreg(AW));					CLKS(12,8,5); }
OP( 0x59, i_pop_cx  ) { POP(Wreg(CW));					CLKS(12,8,5); }
OP( 0x5a, i_pop_dx  ) { POP(Wreg(DW));					CLKS(12,8,5); }
OP( 0x5b, i_pop_bx  ) { POP(Wreg(BW));					CLKS(12,8,5); }
OP( 0x5c, i_pop_sp  ) { POP(Wreg(SP));					CLKS(12,8,5); }
OP( 0x5d, i_pop_bp  ) { POP(Wreg(BP));					CLKS(12,8,5); }
OP( 0x5e, i_pop_si  ) { POP(Wreg(IX));					CLKS(12,8,5); }
OP( 0x5f, i_pop_di  ) { POP(Wreg(IY));					CLKS(12,8,5); }

OP( 0x60, i_pusha  ) {
	unsigned tmp=Wreg(SP);
	PUSH(Wreg(AW));
	PUSH(Wreg(CW));
	PUSH(Wreg(DW));
	PUSH(Wreg(BW));
	PUSH(tmp);
	PUSH(Wreg(BP));
	PUSH(Wreg(IX));
	PUSH(Wreg(IY));
	CLKS(67,35,20);
}
static unsigned nec_popa_tmp;
OP( 0x61, i_popa  ) {
	POP(Wreg(IY));
	POP(Wreg(IX));
	POP(Wreg(BP));
	POP(nec_popa_tmp);
	POP(Wreg(BW));
	POP(Wreg(DW));
	POP(Wreg(CW));
	POP(Wreg(AW));
	CLKS(75,43,22);
}
OP( 0x62, i_chkind  ) {
	UINT32 low,high,tmp;
	GetModRM;
	low = GetRMWord(ModRM);
	high= GetnextRMWord;
	tmp= RegWord(ModRM);
	if (tmp<low || tmp>high) {
		nec_interrupt(nec_state, NEC_CHKIND_VECTOR, BRK);
	}
	nec_state->icount-=20;
//	logerror("%06x: bound %04x high %04x low %04x tmp\n",PC(nec_state),high,low,tmp);
}
OP( 0x64, i_repnc  ) {	UINT32 next = fetchop(nec_state);	UINT16 c = Wreg(CW);
	switch(next) { /* Segments */
		case 0x26:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(DS1)<<4;	next = fetchop(nec_state);	CLK(2); break;
		case 0x2e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(PS)<<4;	next = fetchop(nec_state);	CLK(2); break;
		case 0x36:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(SS)<<4;	next = fetchop(nec_state);	CLK(2); break;
		case 0x3e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(DS0)<<4;	next = fetchop(nec_state);	CLK(2); break;
	}

	switch(next) {
		case 0x6c:	CLK(2); if (c) do { i_insb(nec_state);  c--; } while (c>0 && !CF); Wreg(CW)=c; break;
		case 0x6d:  CLK(2); if (c) do { i_insw(nec_state);  c--; } while (c>0 && !CF); Wreg(CW)=c; break;
		case 0x6e:	CLK(2); if (c) do { i_outsb(nec_state); c--; } while (c>0 && !CF); Wreg(CW)=c; break;
		case 0x6f:  CLK(2); if (c) do { i_outsw(nec_state); c--; } while (c>0 && !CF); Wreg(CW)=c; break;
		case 0xa4:	CLK(2); if (c) do { i_movsb(nec_state); c--; } while (c>0 && !CF); Wreg(CW)=c; break;
		case 0xa5:  CLK(2); if (c) do { i_movsw(nec_state); c--; } while (c>0 && !CF); Wreg(CW)=c; break;
		case 0xa6:	CLK(2); if (c) do { i_cmpsb(nec_state); c--; } while (c>0 && !CF); Wreg(CW)=c; break;
		case 0xa7:	CLK(2); if (c) do { i_cmpsw(nec_state); c--; } while (c>0 && !CF); Wreg(CW)=c; break;
		case 0xaa:	CLK(2); if (c) do { i_stosb(nec_state); c--; } while (c>0 && !CF); Wreg(CW)=c; break;
		case 0xab:  CLK(2); if (c) do { i_stosw(nec_state); c--; } while (c>0 && !CF); Wreg(CW)=c; break;
		case 0xac:	CLK(2); if (c) do { i_lodsb(nec_state); c--; } while (c>0 && !CF); Wreg(CW)=c; break;
		case 0xad:  CLK(2); if (c) do { i_lodsw(nec_state); c--; } while (c>0 && !CF); Wreg(CW)=c; break;
		case 0xae:	CLK(2); if (c) do { i_scasb(nec_state); c--; } while (c>0 && !CF); Wreg(CW)=c; break;
		case 0xaf:	CLK(2); if (c) do { i_scasw(nec_state); c--; } while (c>0 && !CF); Wreg(CW)=c; break;
		default:	nec_instruction[next](nec_state);
	}
	nec_state->seg_prefix=FALSE;
}

OP( 0x65, i_repc  ) {	UINT32 next = fetchop(nec_state);	UINT16 c = Wreg(CW);
	switch(next) { /* Segments */
		case 0x26:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(DS1)<<4;	next = fetchop(nec_state);	CLK(2); break;
		case 0x2e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(PS)<<4;	next = fetchop(nec_state);	CLK(2); break;
		case 0x36:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(SS)<<4;	next = fetchop(nec_state);	CLK(2); break;
		case 0x3e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(DS0)<<4;	next = fetchop(nec_state);	CLK(2); break;
	}

	switch(next) {
		case 0x6c:	CLK(2); if (c) do { i_insb(nec_state);  c--; } while (c>0 && CF);	Wreg(CW)=c;	break;
		case 0x6d:  CLK(2); if (c) do { i_insw(nec_state);  c--; } while (c>0 && CF);	Wreg(CW)=c;	break;
		case 0x6e:	CLK(2); if (c) do { i_outsb(nec_state); c--; } while (c>0 && CF);	Wreg(CW)=c;	break;
		case 0x6f:  CLK(2); if (c) do { i_outsw(nec_state); c--; } while (c>0 && CF);	Wreg(CW)=c;	break;
		case 0xa4:	CLK(2); if (c) do { i_movsb(nec_state); c--; } while (c>0 && CF);	Wreg(CW)=c;	break;
		case 0xa5:  CLK(2); if (c) do { i_movsw(nec_state); c--; } while (c>0 && CF);	Wreg(CW)=c;	break;
		case 0xa6:	CLK(2); if (c) do { i_cmpsb(nec_state); c--; } while (c>0 && CF);	Wreg(CW)=c; break;
		case 0xa7:	CLK(2); if (c) do { i_cmpsw(nec_state); c--; } while (c>0 && CF);	Wreg(CW)=c; break;
		case 0xaa:	CLK(2); if (c) do { i_stosb(nec_state); c--; } while (c>0 && CF);	Wreg(CW)=c;	break;
		case 0xab:  CLK(2); if (c) do { i_stosw(nec_state); c--; } while (c>0 && CF);	Wreg(CW)=c;	break;
		case 0xac:	CLK(2); if (c) do { i_lodsb(nec_state); c--; } while (c>0 && CF);	Wreg(CW)=c;	break;
		case 0xad:  CLK(2); if (c) do { i_lodsw(nec_state); c--; } while (c>0 && CF);	Wreg(CW)=c;	break;
		case 0xae:	CLK(2); if (c) do { i_scasb(nec_state); c--; } while (c>0 && CF);	Wreg(CW)=c; break;
		case 0xaf:	CLK(2); if (c) do { i_scasw(nec_state); c--; } while (c>0 && CF);	Wreg(CW)=c; break;
		default:	nec_instruction[next](nec_state);
	}
	nec_state->seg_prefix=FALSE;
}

OP( 0x68, i_push_d16 ) { UINT32 tmp;	tmp = FETCHWORD(); PUSH(tmp);	CLKW(12,12,5,12,8,5,Wreg(SP));	}
OP( 0x69, i_imul_d16 ) { UINT32 tmp;	DEF_r16w;	tmp = FETCHWORD(); dst = (INT32)((INT16)src)*(INT32)((INT16)tmp); nec_state->CarryVal = nec_state->OverVal = (((INT32)dst) >> 15 != 0) && (((INT32)dst) >> 15 != -1);     RegWord(ModRM)=(WORD)dst;     nec_state->icount-=(ModRM >=0xc0 )?38:47;}
OP( 0x6a, i_push_d8  ) { UINT32 tmp = (WORD)((INT16)((INT8)FETCH()));	PUSH(tmp);	CLKW(11,11,5,11,7,3,Wreg(SP));	}
OP( 0x6b, i_imul_d8  ) { UINT32 src2; DEF_r16w; src2= (WORD)((INT16)((INT8)FETCH())); dst = (INT32)((INT16)src)*(INT32)((INT16)src2); nec_state->CarryVal = nec_state->OverVal = (((INT32)dst) >> 15 != 0) && (((INT32)dst) >> 15 != -1); RegWord(ModRM)=(WORD)dst; nec_state->icount-=(ModRM >=0xc0 )?31:39; }
OP( 0x6c, i_insb     ) { PutMemB(DS1,Wreg(IY),read_port_byte(Wreg(DW))); Wreg(IY)+= -2 * nec_state->DF + 1; CLK(8); }
OP( 0x6d, i_insw     ) { PutMemW(DS1,Wreg(IY),read_port_word(Wreg(DW))); Wreg(IY)+= -4 * nec_state->DF + 2; CLKS(18,10,8); }
OP( 0x6e, i_outsb    ) { write_port_byte(Wreg(DW),GetMemB(DS0,Wreg(IX))); Wreg(IX)+= -2 * nec_state->DF + 1; CLK(8); }
OP( 0x6f, i_outsw    ) { write_port_word(Wreg(DW),GetMemW(DS0,Wreg(IX))); Wreg(IX)+= -4 * nec_state->DF + 2; CLKS(18,10,8); }

OP( 0x70, i_jo      ) { JMP( OF);				CLKS(4,4,3); }
OP( 0x71, i_jno     ) { JMP(!OF);				CLKS(4,4,3); }
OP( 0x72, i_jc      ) { JMP( CF);				CLKS(4,4,3); }
OP( 0x73, i_jnc     ) { JMP(!CF);				CLKS(4,4,3); }
OP( 0x74, i_jz      ) { JMP( ZF);				CLKS(4,4,3); }
OP( 0x75, i_jnz     ) { JMP(!ZF);				CLKS(4,4,3); }
OP( 0x76, i_jce     ) { JMP(CF || ZF);			CLKS(4,4,3); }
OP( 0x77, i_jnce    ) { JMP(!(CF || ZF));		CLKS(4,4,3); }
OP( 0x78, i_js      ) { JMP( SF);				CLKS(4,4,3); }
OP( 0x79, i_jns     ) { JMP(!SF);				CLKS(4,4,3); }
OP( 0x7a, i_jp      ) { JMP( PF);				CLKS(4,4,3); }
OP( 0x7b, i_jnp     ) { JMP(!PF);				CLKS(4,4,3); }
OP( 0x7c, i_jl      ) { JMP((SF!=OF)&&(!ZF));	CLKS(4,4,3); }
OP( 0x7d, i_jnl     ) { JMP((ZF)||(SF==OF));	CLKS(4,4,3); }
OP( 0x7e, i_jle     ) { JMP((ZF)||(SF!=OF));	CLKS(4,4,3); }
OP( 0x7f, i_jnle    ) { JMP((SF==OF)&&(!ZF));	CLKS(4,4,3); }

OP( 0x80, i_80pre   ) { UINT32 dst, src; GetModRM; dst = GetRMByte(ModRM); src = FETCH();
	if (ModRM >=0xc0 ) CLKS(4,4,2) else if ((ModRM & 0x38)==0x38) CLKS(13,13,6) else CLKS(18,18,7)
	switch (ModRM & 0x38) {
		case 0x00: ADDB;			PutbackRMByte(ModRM,dst);	break;
		case 0x08: ORB;				PutbackRMByte(ModRM,dst);	break;
		case 0x10: src+=CF;	ADDB;	PutbackRMByte(ModRM,dst);	break;
		case 0x18: src+=CF;	SUBB;	PutbackRMByte(ModRM,dst);	break;
		case 0x20: ANDB;			PutbackRMByte(ModRM,dst);	break;
		case 0x28: SUBB;			PutbackRMByte(ModRM,dst);	break;
		case 0x30: XORB;			PutbackRMByte(ModRM,dst);	break;
		case 0x38: SUBB;			break;	/* CMP */
	}
}

OP( 0x81, i_81pre   ) { UINT32 dst, src; GetModRM; dst = GetRMWord(ModRM); src = FETCH(); src+= (FETCH() << 8);
	if (ModRM >=0xc0 ) CLKS(4,4,2) else if ((ModRM & 0x38)==0x38) CLKW(17,17,8,17,13,6,EA) else CLKW(26,26,11,26,18,7,EA)
	switch (ModRM & 0x38) {
		case 0x00: ADDW;			PutbackRMWord(ModRM,dst);	break;
		case 0x08: ORW;				PutbackRMWord(ModRM,dst);	break;
		case 0x10: src+=CF;	ADDW;	PutbackRMWord(ModRM,dst);	break;
		case 0x18: src+=CF;	SUBW;	PutbackRMWord(ModRM,dst);	break;
		case 0x20: ANDW;			PutbackRMWord(ModRM,dst);	break;
		case 0x28: SUBW;			PutbackRMWord(ModRM,dst);	break;
		case 0x30: XORW;			PutbackRMWord(ModRM,dst);	break;
		case 0x38: SUBW;			break;	/* CMP */
	}
}

OP( 0x82, i_82pre   ) { UINT32 dst, src; GetModRM; dst = GetRMByte(ModRM); src = (BYTE)((INT8)FETCH());
	if (ModRM >=0xc0 ) CLKS(4,4,2) else if ((ModRM & 0x38)==0x38) CLKS(13,13,6) else CLKS(18,18,7)
	switch (ModRM & 0x38) {
		case 0x00: ADDB;			PutbackRMByte(ModRM,dst);	break;
		case 0x08: ORB;				PutbackRMByte(ModRM,dst);	break;
		case 0x10: src+=CF;	ADDB;	PutbackRMByte(ModRM,dst);	break;
		case 0x18: src+=CF;	SUBB;	PutbackRMByte(ModRM,dst);	break;
		case 0x20: ANDB;			PutbackRMByte(ModRM,dst);	break;
		case 0x28: SUBB;			PutbackRMByte(ModRM,dst);	break;
		case 0x30: XORB;			PutbackRMByte(ModRM,dst);	break;
		case 0x38: SUBB;			break;	/* CMP */
	}
}

OP( 0x83, i_83pre   ) { UINT32 dst, src; GetModRM; dst = GetRMWord(ModRM); src = (WORD)((INT16)((INT8)FETCH()));
	if (ModRM >=0xc0 ) CLKS(4,4,2) else if ((ModRM & 0x38)==0x38) CLKW(17,17,8,17,13,6,EA) else CLKW(26,26,11,26,18,7,EA)
	switch (ModRM & 0x38) {
		case 0x00: ADDW;			PutbackRMWord(ModRM,dst);	break;
		case 0x08: ORW;				PutbackRMWord(ModRM,dst);	break;
		case 0x10: src+=CF;	ADDW;	PutbackRMWord(ModRM,dst);	break;
		case 0x18: src+=CF;	SUBW;	PutbackRMWord(ModRM,dst);	break;
		case 0x20: ANDW;			PutbackRMWord(ModRM,dst);	break;
		case 0x28: SUBW;			PutbackRMWord(ModRM,dst);	break;
		case 0x30: XORW;			PutbackRMWord(ModRM,dst);	break;
		case 0x38: SUBW;			break;	/* CMP */
	}
}

OP( 0x84, i_test_br8  ) { DEF_br8;	ANDB;	CLKM(2,2,2,10,10,6);		}
OP( 0x85, i_test_wr16 ) { DEF_wr16;	ANDW;	CLKR(14,14,8,14,10,6,2,EA);	}
OP( 0x86, i_xchg_br8  ) { DEF_br8;	RegByte(ModRM)=dst; PutbackRMByte(ModRM,src); CLKM(3,3,3,16,18,8); }
OP( 0x87, i_xchg_wr16 ) { DEF_wr16;	RegWord(ModRM)=dst; PutbackRMWord(ModRM,src); CLKR(24,24,12,24,16,8,3,EA); }

OP( 0x88, i_mov_br8   ) { UINT8  src; GetModRM; src = RegByte(ModRM);	PutRMByte(ModRM,src);	CLKM(2,2,2,9,9,3);			}
OP( 0x89, i_mov_wr16  ) { UINT16 src; GetModRM; src = RegWord(ModRM);	PutRMWord(ModRM,src);	CLKR(13,13,5,13,9,3,2,EA);	}
OP( 0x8a, i_mov_r8b   ) { UINT8  src; GetModRM; src = GetRMByte(ModRM);	RegByte(ModRM)=src;		CLKM(2,2,2,11,11,5);		}
OP( 0x8b, i_mov_r16w  ) { UINT16 src; GetModRM; src = GetRMWord(ModRM);	RegWord(ModRM)=src; 	CLKR(15,15,7,15,11,5,2,EA); 	}
OP( 0x8c, i_mov_wsreg ) { GetModRM;
	switch (ModRM & 0x38) {
		case 0x00: PutRMWord(ModRM,Sreg(DS1)); CLKR(14,14,5,14,10,3,2,EA); break;
		case 0x08: PutRMWord(ModRM,Sreg(PS)); CLKR(14,14,5,14,10,3,2,EA); break;
		case 0x10: PutRMWord(ModRM,Sreg(SS)); CLKR(14,14,5,14,10,3,2,EA); break;
		case 0x18: PutRMWord(ModRM,Sreg(DS0)); CLKR(14,14,5,14,10,3,2,EA); break;
//		default:   logerror("%06x: MOV Sreg - Invalid register\n",PC(nec_state));
	}
}
OP( 0x8d, i_lea       ) { UINT16 ModRM = FETCH(); (void)(*GetEA[ModRM])(nec_state); RegWord(ModRM)=EO;	CLKS(4,4,2); }
OP( 0x8e, i_mov_sregw ) { UINT16 src; GetModRM; src = GetRMWord(ModRM); CLKR(15,15,7,15,11,5,2,EA);
	switch (ModRM & 0x38) {
		case 0x00: Sreg(DS1) = src; break; /* mov es,ew */
		case 0x08: Sreg(PS) = src; break; /* mov cs,ew */
		case 0x10: Sreg(SS) = src; break; /* mov ss,ew */
		case 0x18: Sreg(DS0) = src; break; /* mov ds,ew */
//		default:   logerror("%06x: MOV Sreg - Invalid register\n",PC(nec_state));
	}
	nec_state->no_interrupt=1;
}
OP( 0x8f, i_popw ) { UINT16 tmp; GetModRM; POP(tmp); PutRMWord(ModRM,tmp); nec_state->icount-=21; }
OP( 0x90, i_nop  ) { CLK(3); /* { if (nec_state->MF == 0) printf("90 -> %06x: \n",PC(nec_state)); }  */ }
OP( 0x91, i_xchg_axcx ) { XchgAWReg(CW); CLK(3); }
OP( 0x92, i_xchg_axdx ) { XchgAWReg(DW); CLK(3); }
OP( 0x93, i_xchg_axbx ) { XchgAWReg(BW); CLK(3); }
OP( 0x94, i_xchg_axsp ) { XchgAWReg(SP); CLK(3); }
OP( 0x95, i_xchg_axbp ) { XchgAWReg(BP); CLK(3); }
OP( 0x96, i_xchg_axsi ) { XchgAWReg(IX); CLK(3); }
OP( 0x97, i_xchg_axdi ) { XchgAWReg(IY); CLK(3); }

OP( 0x98, i_cbw       ) { Breg(AH) = (Breg(AL) & 0x80) ? 0xff : 0;		CLK(2);	}
OP( 0x99, i_cwd       ) { Wreg(DW) = (Breg(AH) & 0x80) ? 0xffff : 0;	CLK(4);	}
OP( 0x9a, i_call_far  ) { UINT32 tmp, tmp2;	tmp = FETCHWORD(); tmp2 = FETCHWORD(); PUSH(Sreg(PS)); PUSH(nec_state->ip); nec_state->ip = (WORD)tmp; Sreg(PS) = (WORD)tmp2; CHANGE_PC; CLKW(29,29,13,29,21,9,Wreg(SP)); }
OP( 0x9b, i_wait      ) { if (!nec_state->poll_state) nec_state->ip--; CLK(5); }
OP( 0x9c, i_pushf     ) { UINT16 tmp = CompressFlags(); PUSH( tmp ); CLKS(12,8,3); }
OP( 0x9d, i_popf      ) { UINT32 tmp; POP(tmp); ExpandFlags(tmp); CLKS(12,8,5); if (nec_state->TF) nec_trap(nec_state); }
OP( 0x9e, i_sahf      ) { UINT32 tmp = (CompressFlags() & 0xff00) | (Breg(AH) & 0xd5); ExpandFlags(tmp); CLKS(3,3,2); }
OP( 0x9f, i_lahf      ) { Breg(AH) = CompressFlags() & 0xff; CLKS(3,3,2); }

OP( 0xa0, i_mov_aldisp ) { UINT32 addr; addr = FETCHWORD(); Breg(AL) = GetMemB(DS0, addr); CLKS(10,10,5); }
OP( 0xa1, i_mov_axdisp ) { UINT32 addr; addr = FETCHWORD(); Wreg(AW) = GetMemW(DS0, addr); CLKW(14,14,7,14,10,5,addr); }
OP( 0xa2, i_mov_dispal ) { UINT32 addr; addr = FETCHWORD(); PutMemB(DS0, addr, Breg(AL));  CLKS(9,9,3); }
OP( 0xa3, i_mov_dispax ) { UINT32 addr; addr = FETCHWORD(); PutMemW(DS0, addr, Wreg(AW));  CLKW(13,13,5,13,9,3,addr); }
OP( 0xa4, i_movsb      ) { UINT32 tmp = GetMemB(DS0,Wreg(IX)); PutMemB(DS1,Wreg(IY), tmp); Wreg(IY) += -2 * nec_state->DF + 1; Wreg(IX) += -2 * nec_state->DF + 1; CLKS(8,8,6); }
OP( 0xa5, i_movsw      ) { UINT32 tmp = GetMemW(DS0,Wreg(IX)); PutMemW(DS1,Wreg(IY), tmp); Wreg(IY) += -4 * nec_state->DF + 2; Wreg(IX) += -4 * nec_state->DF + 2; CLKS(16,16,10); }
OP( 0xa6, i_cmpsb      ) { UINT32 src = GetMemB(DS1, Wreg(IY)); UINT32 dst = GetMemB(DS0, Wreg(IX)); SUBB; Wreg(IY) += -2 * nec_state->DF + 1; Wreg(IX) += -2 * nec_state->DF + 1; CLKS(14,14,14); }
OP( 0xa7, i_cmpsw      ) { UINT32 src = GetMemW(DS1, Wreg(IY)); UINT32 dst = GetMemW(DS0, Wreg(IX)); SUBW; Wreg(IY) += -4 * nec_state->DF + 2; Wreg(IX) += -4 * nec_state->DF + 2; CLKS(14,14,14); }

OP( 0xa8, i_test_ald8  ) { DEF_ald8;  ANDB; CLKS(4,4,2); }
OP( 0xa9, i_test_axd16 ) { DEF_axd16; ANDW; CLKS(4,4,2); }
OP( 0xaa, i_stosb      ) { PutMemB(DS1,Wreg(IY),Breg(AL));	Wreg(IY) += -2 * nec_state->DF + 1; CLKS(4,4,3);  }
OP( 0xab, i_stosw      ) { PutMemW(DS1,Wreg(IY),Wreg(AW));	Wreg(IY) += -4 * nec_state->DF + 2; CLKW(8,8,5,8,4,3,Wreg(IY)); }
OP( 0xac, i_lodsb      ) { Breg(AL) = GetMemB(DS0,Wreg(IX)); Wreg(IX) += -2 * nec_state->DF + 1; CLKS(4,4,3);  }
OP( 0xad, i_lodsw      ) { Wreg(AW) = GetMemW(DS0,Wreg(IX)); Wreg(IX) += -4 * nec_state->DF + 2; CLKW(8,8,5,8,4,3,Wreg(IX)); }
OP( 0xae, i_scasb      ) { UINT32 src = GetMemB(DS1, Wreg(IY));	UINT32 dst = Breg(AL); SUBB; Wreg(IY) += -2 * nec_state->DF + 1; CLKS(4,4,3);  }
OP( 0xaf, i_scasw      ) { UINT32 src = GetMemW(DS1, Wreg(IY));	UINT32 dst = Wreg(AW); SUBW; Wreg(IY) += -4 * nec_state->DF + 2; CLKW(8,8,5,8,4,3,Wreg(IY)); }

OP( 0xb0, i_mov_ald8  ) { Breg(AL) = FETCH();	CLKS(4,4,2); }
OP( 0xb1, i_mov_cld8  ) { Breg(CL) = FETCH(); CLKS(4,4,2); }
OP( 0xb2, i_mov_dld8  ) { Breg(DL) = FETCH(); CLKS(4,4,2); }
OP( 0xb3, i_mov_bld8  ) { Breg(BL) = FETCH(); CLKS(4,4,2); }
OP( 0xb4, i_mov_ahd8  ) { Breg(AH) = FETCH(); CLKS(4,4,2); }
OP( 0xb5, i_mov_chd8  ) { Breg(CH) = FETCH(); CLKS(4,4,2); }
OP( 0xb6, i_mov_dhd8  ) { Breg(DH) = FETCH(); CLKS(4,4,2); }
OP( 0xb7, i_mov_bhd8  ) { Breg(BH) = FETCH();	CLKS(4,4,2); }

OP( 0xb8, i_mov_axd16 ) { Breg(AL) = FETCH();	 Breg(AH) = FETCH();	CLKS(4,4,2); }
OP( 0xb9, i_mov_cxd16 ) { Breg(CL) = FETCH();	 Breg(CH) = FETCH();	CLKS(4,4,2); }
OP( 0xba, i_mov_dxd16 ) { Breg(DL) = FETCH();	 Breg(DH) = FETCH();	CLKS(4,4,2); }
OP( 0xbb, i_mov_bxd16 ) { Breg(BL) = FETCH();	 Breg(BH) = FETCH();	CLKS(4,4,2); }
OP( 0xbc, i_mov_spd16 ) { Wreg(SP) = FETCHWORD();	CLKS(4,4,2); }
OP( 0xbd, i_mov_bpd16 ) { Wreg(BP) = FETCHWORD();	CLKS(4,4,2); }
OP( 0xbe, i_mov_sid16 ) { Wreg(IX) = FETCHWORD();	CLKS(4,4,2); }
OP( 0xbf, i_mov_did16 ) { Wreg(IY) = FETCHWORD();	CLKS(4,4,2); }

OP( 0xc0, i_rotshft_bd8 ) {
	UINT32 src, dst; UINT8 c;
	GetModRM; src = (unsigned)GetRMByte(ModRM); dst=src;
	c=FETCH();
	CLKM(7,7,2,19,19,6);
	if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_BYTE;  c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x08: do { ROR_BYTE;  c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x10: do { ROLC_BYTE; c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x18: do { RORC_BYTE; c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x20: SHL_BYTE(c); break;
		case 0x28: SHR_BYTE(c); break;
//		case 0x30: logerror("%06x: Undefined opcode 0xc0 0x30 (SHLA)\n",PC(nec_state)); break;
		case 0x38: SHRA_BYTE(c); break;
	}
}

OP( 0xc1, i_rotshft_wd8 ) {
	UINT32 src, dst;  UINT8 c;
	GetModRM; src = (unsigned)GetRMWord(ModRM); dst=src;
	c=FETCH();
	CLKM(7,7,2,27,19,6);
	if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_WORD;  c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x08: do { ROR_WORD;  c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x10: do { ROLC_WORD; c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x18: do { RORC_WORD; c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x20: SHL_WORD(c); break;
		case 0x28: SHR_WORD(c); break;
//		case 0x30: logerror("%06x: Undefined opcode 0xc1 0x30 (SHLA)\n",PC(nec_state)); break;
		case 0x38: SHRA_WORD(c); break;
	}
}

OP( 0xc2, i_ret_d16  ) { UINT32 count = FETCH(); count += FETCH() << 8; POP(nec_state->ip); Wreg(SP)+=count; CHANGE_PC; CLKS(24,24,10); }
OP( 0xc3, i_ret      ) { POP(nec_state->ip); CHANGE_PC; CLKS(19,19,10); }
OP( 0xc4, i_les_dw   ) { GetModRM; WORD tmp = GetRMWord(ModRM); RegWord(ModRM)=tmp; Sreg(DS1) = GetnextRMWord; CLKW(26,26,14,26,18,10,EA); }
OP( 0xc5, i_lds_dw   ) { GetModRM; WORD tmp = GetRMWord(ModRM); RegWord(ModRM)=tmp; Sreg(DS0) = GetnextRMWord; CLKW(26,26,14,26,18,10,EA); }
OP( 0xc6, i_mov_bd8  ) { GetModRM; PutImmRMByte(ModRM); nec_state->icount-=(ModRM >=0xc0 )?4:11; }
OP( 0xc7, i_mov_wd16 ) { GetModRM; PutImmRMWord(ModRM); nec_state->icount-=(ModRM >=0xc0 )?4:15; }

OP( 0xc8, i_enter ) {
	UINT32 nb = FETCH();
	UINT32 i,level;

	nec_state->icount-=23;
	nb += FETCH() << 8;
	level = FETCH();
	PUSH(Wreg(BP));
	Wreg(BP)=Wreg(SP);
	Wreg(SP) -= nb;
	for (i=1;i<level;i++) {
		PUSH(GetMemW(SS,Wreg(BP)-i*2));
		nec_state->icount-=16;
	}
	if (level) PUSH(Wreg(BP));
}
OP( 0xc9, i_leave ) {
	Wreg(SP)=Wreg(BP);
	POP(Wreg(BP));
	nec_state->icount-=8;
}
OP( 0xca, i_retf_d16  ) { UINT32 count = FETCH(); count += FETCH() << 8; POP(nec_state->ip); POP(Sreg(PS)); Wreg(SP)+=count; CHANGE_PC; CLKS(32,32,16); }
OP( 0xcb, i_retf      ) { POP(nec_state->ip); POP(Sreg(PS)); CHANGE_PC; CLKS(29,29,16); }
OP( 0xcc, i_int3      ) { nec_interrupt(nec_state, 3, BRK); CLKS(50,50,24); }
OP( 0xcd, i_int       ) { nec_interrupt(nec_state, FETCH(), BRK); CLKS(50,50,24); }
OP( 0xce, i_into      ) { if (OF) { nec_interrupt(nec_state, NEC_BRKV_VECTOR, BRK); CLKS(52,52,26); } else CLK(3); }
OP( 0xcf, i_iret      ) { POP(nec_state->ip); POP(Sreg(PS)); i_popf(nec_state); CHANGE_PC; CLKS(39,39,19); }

OP( 0xd0, i_rotshft_b ) {
	UINT32 src, dst; GetModRM; src = (UINT32)GetRMByte(ModRM); dst=src;
	CLKM(6,6,2,16,16,7);
	switch (ModRM & 0x38) {
		case 0x00: ROL_BYTE;  PutbackRMByte(ModRM,(BYTE)dst); nec_state->OverVal = (src^dst)&0x80; break;
		case 0x08: ROR_BYTE;  PutbackRMByte(ModRM,(BYTE)dst); nec_state->OverVal = (src^dst)&0x80; break;
		case 0x10: ROLC_BYTE; PutbackRMByte(ModRM,(BYTE)dst); nec_state->OverVal = (src^dst)&0x80; break;
		case 0x18: RORC_BYTE; PutbackRMByte(ModRM,(BYTE)dst); nec_state->OverVal = (src^dst)&0x80; break;
		case 0x20: SHL_BYTE(1); nec_state->OverVal = (src^dst)&0x80; break;
		case 0x28: SHR_BYTE(1); nec_state->OverVal = (src^dst)&0x80; break;
//		case 0x30: logerror("%06x: Undefined opcode 0xd0 0x30 (SHLA)\n",PC(nec_state)); break;
		case 0x38: SHRA_BYTE(1); nec_state->OverVal = 0; break;
	}
}

OP( 0xd1, i_rotshft_w ) {
	UINT32 src, dst; GetModRM; src = (UINT32)GetRMWord(ModRM); dst=src;
	CLKM(6,6,2,24,16,7);
	switch (ModRM & 0x38) {
		case 0x00: ROL_WORD;  PutbackRMWord(ModRM,(WORD)dst); nec_state->OverVal = (src^dst)&0x8000; break;
		case 0x08: ROR_WORD;  PutbackRMWord(ModRM,(WORD)dst); nec_state->OverVal = (src^dst)&0x8000; break;
		case 0x10: ROLC_WORD; PutbackRMWord(ModRM,(WORD)dst); nec_state->OverVal = (src^dst)&0x8000; break;
		case 0x18: RORC_WORD; PutbackRMWord(ModRM,(WORD)dst); nec_state->OverVal = (src^dst)&0x8000; break;
		case 0x20: SHL_WORD(1); nec_state->OverVal = (src^dst)&0x8000;  break;
		case 0x28: SHR_WORD(1); nec_state->OverVal = (src^dst)&0x8000;  break;
//		case 0x30: logerror("%06x: Undefined opcode 0xd1 0x30 (SHLA)\n",PC(nec_state)); break;
		case 0x38: SHRA_WORD(1); nec_state->OverVal = 0; break;
	}
}

OP( 0xd2, i_rotshft_bcl ) {
	UINT32 src, dst; UINT8 c; GetModRM; src = (UINT32)GetRMByte(ModRM); dst=src;
	c=Breg(CL);
	CLKM(7,7,2,19,19,6);
	if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_BYTE;  c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x08: do { ROR_BYTE;  c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x10: do { ROLC_BYTE; c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x18: do { RORC_BYTE; c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x20: SHL_BYTE(c); break;
		case 0x28: SHR_BYTE(c); break;
//		case 0x30: logerror("%06x: Undefined opcode 0xd2 0x30 (SHLA)\n",PC(nec_state)); break;
		case 0x38: SHRA_BYTE(c); break;
	}
}

OP( 0xd3, i_rotshft_wcl ) {
	UINT32 src, dst; UINT8 c; GetModRM; src = (UINT32)GetRMWord(ModRM); dst=src;
	c=Breg(CL);
	CLKM(7,7,2,27,19,6);
	if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_WORD;  c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x08: do { ROR_WORD;  c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x10: do { ROLC_WORD; c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x18: do { RORC_WORD; c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x20: SHL_WORD(c); break;
		case 0x28: SHR_WORD(c); break;
//		case 0x30: logerror("%06x: Undefined opcode 0xd3 0x30 (SHLA)\n",PC(nec_state)); break;
		case 0x38: SHRA_WORD(c); break;
	}
}

OP( 0xd4, i_aam    ) { FETCH(); Breg(AH) = Breg(AL) / 10; Breg(AL) %= 10; SetSZPF_Word(Wreg(AW)); CLKS(15,15,12); }
OP( 0xd5, i_aad    ) { FETCH(); Breg(AL) = Breg(AH) * 10 + Breg(AL); Breg(AH) = 0; SetSZPF_Byte(Breg(AL)); CLKS(7,7,8); }
OP( 0xd6, i_setalc ) { Breg(AL) = (CF)?0xff:0x00; nec_state->icount-=3; }
OP( 0xd7, i_trans  ) { UINT32 dest = (Wreg(BW)+Breg(AL))&0xffff; Breg(AL) = GetMemB(DS0, dest); CLKS(9,9,5); }
OP( 0xd8, i_fpo    ) { GetModRM; ModRM = ModRM; nec_state->icount-=2;	}

OP( 0xe0, i_loopne ) { INT8 disp = (INT8)FETCH(); Wreg(CW)--; if (!ZF && Wreg(CW)) { nec_state->ip = (WORD)(nec_state->ip+disp); /*CHANGE_PC;*/ CLKS(14,14,6); } else CLKS(5,5,3); }
OP( 0xe1, i_loope  ) { INT8 disp = (INT8)FETCH(); Wreg(CW)--; if ( ZF && Wreg(CW)) { nec_state->ip = (WORD)(nec_state->ip+disp); /*CHANGE_PC;*/ CLKS(14,14,6); } else CLKS(5,5,3); }
OP( 0xe2, i_loop   ) { INT8 disp = (INT8)FETCH(); Wreg(CW)--; if (Wreg(CW)) { nec_state->ip = (WORD)(nec_state->ip+disp); /*CHANGE_PC;*/ CLKS(13,13,6); } else CLKS(5,5,3); }
OP( 0xe3, i_jcxz   ) { INT8 disp = (INT8)FETCH(); if (Wreg(CW) == 0) { nec_state->ip = (WORD)(nec_state->ip+disp); /*CHANGE_PC;*/ CLKS(13,13,6); } else CLKS(5,5,3); }
OP( 0xe4, i_inal   ) { UINT8 port = FETCH(); Breg(AL) = read_port_byte(port); CLKS(9,9,5);	}
OP( 0xe5, i_inax   ) { UINT8 port = FETCH(); Wreg(AW) = read_port_word(port); CLKW(13,13,7,13,9,5,port); }
OP( 0xe6, i_outal  ) { UINT8 port = FETCH(); write_port_byte(port, Breg(AL)); CLKS(8,8,3);	}
OP( 0xe7, i_outax  ) { UINT8 port = FETCH(); write_port_word(port, Wreg(AW)); CLKW(12,12,5,12,8,3,port);	}

OP( 0xe8, i_call_d16 ) { UINT32 tmp; tmp = FETCHWORD(); PUSH(nec_state->ip); nec_state->ip = (WORD)(nec_state->ip+(INT16)tmp); CHANGE_PC; nec_state->icount-=24; }
OP( 0xe9, i_jmp_d16  ) { UINT32 tmp; tmp = FETCHWORD(); nec_state->ip = (WORD)(nec_state->ip+(INT16)tmp); CHANGE_PC; nec_state->icount-=15; }
OP( 0xea, i_jmp_far  ) { UINT32 tmp,tmp1; tmp = FETCHWORD(); tmp1 = FETCHWORD(); Sreg(PS) = (WORD)tmp1; 	nec_state->ip = (WORD)tmp; CHANGE_PC; nec_state->icount-=27;  }
OP( 0xeb, i_jmp_d8   ) { int tmp = (int)((INT8)FETCH()); nec_state->icount-=12; nec_state->ip = (WORD)(nec_state->ip+tmp); }
OP( 0xec, i_inaldx   ) { Breg(AL) = read_port_byte(Wreg(DW)); CLKS(8,8,5);}
OP( 0xed, i_inaxdx   ) { Wreg(AW) = read_port_word(Wreg(DW)); CLKW(12,12,7,12,8,5,Wreg(DW)); }
OP( 0xee, i_outdxal  ) { write_port_byte(Wreg(DW), Breg(AL)); CLKS(8,8,3);	}
OP( 0xef, i_outdxax  ) { write_port_word(Wreg(DW), Wreg(AW)); CLKW(12,12,5,12,8,3,Wreg(DW)); }

OP( 0xf0, i_lock     ) { nec_state->no_interrupt=1; CLK(2); }
OP( 0xf2, i_repne    ) { UINT32 next = fetchop(nec_state); UINT16 c = Wreg(CW);
	switch(next) { /* Segments */
		case 0x26:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(DS1)<<4;	next = fetchop(nec_state);	CLK(2); break;
		case 0x2e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(PS)<<4;		next = fetchop(nec_state);	CLK(2); break;
		case 0x36:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(SS)<<4;		next = fetchop(nec_state);	CLK(2); break;
		case 0x3e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(DS0)<<4;	next = fetchop(nec_state);	CLK(2); break;
	}

	switch(next) {
		case 0x6c:	CLK(2); if (c) do { i_insb(nec_state);  c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0x6d:  CLK(2); if (c) do { i_insw(nec_state);  c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0x6e:	CLK(2); if (c) do { i_outsb(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0x6f:  CLK(2); if (c) do { i_outsw(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0xa4:	CLK(2); if (c) do { i_movsb(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0xa5:  CLK(2); if (c) do { i_movsw(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0xa6:	CLK(2); if (c) do { i_cmpsb(nec_state); c--; } while (c>0 && ZF==0);	Wreg(CW)=c; break;
		case 0xa7:	CLK(2); if (c) do { i_cmpsw(nec_state); c--; } while (c>0 && ZF==0);	Wreg(CW)=c; break;
		case 0xaa:	CLK(2); if (c) do { i_stosb(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0xab:  CLK(2); if (c) do { i_stosw(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0xac:	CLK(2); if (c) do { i_lodsb(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0xad:  CLK(2); if (c) do { i_lodsw(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0xae:	CLK(2); if (c) do { i_scasb(nec_state); c--; } while (c>0 && ZF==0);	Wreg(CW)=c; break;
		case 0xaf:	CLK(2); if (c) do { i_scasw(nec_state); c--; } while (c>0 && ZF==0);	Wreg(CW)=c; break;
		default:	nec_instruction[next](nec_state);
	}
	nec_state->seg_prefix=FALSE;
}
OP( 0xf3, i_repe     ) { UINT32 next = fetchop(nec_state); UINT16 c = Wreg(CW);
	switch(next) { /* Segments */
		case 0x26:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(DS1)<<4;	next = fetchop(nec_state);	CLK(2); break;
		case 0x2e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(PS)<<4;	next = fetchop(nec_state);	CLK(2); break;
		case 0x36:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(SS)<<4;	next = fetchop(nec_state);	CLK(2); break;
		case 0x3e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=Sreg(DS0)<<4;	next = fetchop(nec_state);	CLK(2); break;
	}

	switch(next) {
		case 0x6c:	CLK(2); if (c) do { i_insb(nec_state);  c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0x6d:  CLK(2); if (c) do { i_insw(nec_state);  c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0x6e:	CLK(2); if (c) do { i_outsb(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0x6f:  CLK(2); if (c) do { i_outsw(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0xa4:	CLK(2); if (c) do { i_movsb(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0xa5:  CLK(2); if (c) do { i_movsw(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0xa6:	CLK(2); if (c) do { i_cmpsb(nec_state); c--; } while (c>0 && ZF==1);	Wreg(CW)=c; break;
		case 0xa7:	CLK(2); if (c) do { i_cmpsw(nec_state); c--; } while (c>0 && ZF==1);	Wreg(CW)=c; break;
		case 0xaa:	CLK(2); if (c) do { i_stosb(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0xab:  CLK(2); if (c) do { i_stosw(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0xac:	CLK(2); if (c) do { i_lodsb(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0xad:  CLK(2); if (c) do { i_lodsw(nec_state); c--; } while (c>0);	Wreg(CW)=c;	break;
		case 0xae:	CLK(2); if (c) do { i_scasb(nec_state); c--; } while (c>0 && ZF==1);	Wreg(CW)=c; break;
		case 0xaf:	CLK(2); if (c) do { i_scasw(nec_state); c--; } while (c>0 && ZF==1);	Wreg(CW)=c; break;
		default:	nec_instruction[next](nec_state);
	}
	nec_state->seg_prefix=FALSE;
}
OP( 0xf4, i_hlt ) { nec_state->halted=1; nec_state->icount=0; }
OP( 0xf5, i_cmc ) { nec_state->CarryVal = !CF; CLK(2); }
OP( 0xf6, i_f6pre ) { UINT32 tmp; UINT32 uresult,uresult2; INT32 result,result2;
	GetModRM; tmp = GetRMByte(ModRM);
	switch (ModRM & 0x38) {
		case 0x00: tmp &= FETCH(); nec_state->CarryVal = nec_state->OverVal = 0; SetSZPF_Byte(tmp); nec_state->icount-=(ModRM >=0xc0 )?4:11; break; /* TEST */
//		case 0x08: logerror("%06x: Undefined opcode 0xf6 0x08\n",PC(nec_state)); break;
		case 0x10: PutbackRMByte(ModRM,~tmp); nec_state->icount-=(ModRM >=0xc0 )?2:16; break; /* NOT */
		case 0x18: nec_state->CarryVal=(tmp!=0); tmp=(~tmp)+1; SetSZPF_Byte(tmp); PutbackRMByte(ModRM,tmp&0xff); nec_state->icount-=(ModRM >=0xc0 )?2:16; break; /* NEG */
		case 0x20: uresult = Breg(AL)*tmp; Wreg(AW)=(WORD)uresult; nec_state->CarryVal=nec_state->OverVal=(Breg(AH)!=0); nec_state->icount-=(ModRM >=0xc0 )?30:36; break; /* MULU */
		case 0x28: result = (INT16)((INT8)Breg(AL))*(INT16)((INT8)tmp); Wreg(AW)=(WORD)result; nec_state->CarryVal=nec_state->OverVal=(Breg(AH)!=0); nec_state->icount-=(ModRM >=0xc0 )?30:36; break; /* MUL */
		case 0x30: if (tmp) { DIVUB; } else nec_interrupt(nec_state, NEC_DIVIDE_VECTOR, BRK); nec_state->icount-=(ModRM >=0xc0 )?43:53; break;
		case 0x38: if (tmp) { DIVB;  } else nec_interrupt(nec_state, NEC_DIVIDE_VECTOR, BRK); nec_state->icount-=(ModRM >=0xc0 )?43:53; break;
	}
}

OP( 0xf7, i_f7pre   ) { UINT32 tmp,tmp2; UINT32 uresult,uresult2; INT32 result,result2;
	GetModRM; tmp = GetRMWord(ModRM);
	switch (ModRM & 0x38) {
		case 0x00: tmp2 = FETCHWORD(); tmp &= tmp2; nec_state->CarryVal = nec_state->OverVal = 0; SetSZPF_Word(tmp); nec_state->icount-=(ModRM >=0xc0 )?4:11; break; /* TEST */
//		case 0x08: logerror("%06x: Undefined opcode 0xf7 0x08\n",PC(nec_state)); break;
		case 0x10: PutbackRMWord(ModRM,~tmp); nec_state->icount-=(ModRM >=0xc0 )?2:16; break; /* NOT */
		case 0x18: nec_state->CarryVal=(tmp!=0); tmp=(~tmp)+1; SetSZPF_Word(tmp); PutbackRMWord(ModRM,tmp&0xffff); nec_state->icount-=(ModRM >=0xc0 )?2:16; break; /* NEG */
		case 0x20: uresult = Wreg(AW)*tmp; Wreg(AW)=uresult&0xffff; Wreg(DW)=((UINT32)uresult)>>16; nec_state->CarryVal=nec_state->OverVal=(Wreg(DW)!=0); nec_state->icount-=(ModRM >=0xc0 )?30:36; break; /* MULU */
		case 0x28: result = (INT32)((INT16)Wreg(AW))*(INT32)((INT16)tmp); Wreg(AW)=result&0xffff; Wreg(DW)=result>>16; nec_state->CarryVal=nec_state->OverVal=(Wreg(DW)!=0); nec_state->icount-=(ModRM >=0xc0 )?30:36; break; /* MUL */
		case 0x30: if (tmp) { DIVUW; } else nec_interrupt(nec_state, NEC_DIVIDE_VECTOR, BRK); nec_state->icount-=(ModRM >=0xc0 )?43:53; break;
		case 0x38: if (tmp) { DIVW;  } else nec_interrupt(nec_state, NEC_DIVIDE_VECTOR, BRK); nec_state->icount-=(ModRM >=0xc0 )?43:53; break;
	}
}

OP( 0xf8, i_clc   ) { nec_state->CarryVal = 0;	CLK(2);	}
OP( 0xf9, i_stc   ) { nec_state->CarryVal = 1;	CLK(2);	}
OP( 0xfa, i_di    ) { SetIF(0);			CLK(2);	}
OP( 0xfb, i_ei    ) { SetIF(1);			CLK(2);	}
OP( 0xfc, i_cld   ) { SetDF(0);			CLK(2);	}
OP( 0xfd, i_std   ) { SetDF(1);			CLK(2);	}
OP( 0xfe, i_fepre ) { UINT32 tmp, tmp1; GetModRM; tmp=GetRMByte(ModRM);
	switch(ModRM & 0x38) {
		case 0x00: tmp1 = tmp+1; nec_state->OverVal = (tmp==0x7f); SetAF(tmp1,tmp,1); SetSZPF_Byte(tmp1); PutbackRMByte(ModRM,(BYTE)tmp1); CLKM(2,2,2,16,16,7); break; /* INC */
		case 0x08: tmp1 = tmp-1; nec_state->OverVal = (tmp==0x80); SetAF(tmp1,tmp,1); SetSZPF_Byte(tmp1); PutbackRMByte(ModRM,(BYTE)tmp1); CLKM(2,2,2,16,16,7); break; /* DEC */
//		default:   logerror("%06x: FE Pre with unimplemented mod\n",PC(nec_state));
	}
}
OP( 0xff, i_ffpre ) { UINT32 tmp, tmp1; GetModRM; tmp=GetRMWord(ModRM);
	switch(ModRM & 0x38) {
		case 0x00: tmp1 = tmp+1; nec_state->OverVal = (tmp==0x7fff); SetAF(tmp1,tmp,1); SetSZPF_Word(tmp1); PutbackRMWord(ModRM,(WORD)tmp1); CLKM(2,2,2,24,16,7); break; /* INC */
		case 0x08: tmp1 = tmp-1; nec_state->OverVal = (tmp==0x8000); SetAF(tmp1,tmp,1); SetSZPF_Word(tmp1); PutbackRMWord(ModRM,(WORD)tmp1); CLKM(2,2,2,24,16,7); break; /* DEC */
		case 0x10: PUSH(nec_state->ip);	nec_state->ip = (WORD)tmp; CHANGE_PC; nec_state->icount-=(ModRM >=0xc0 )?16:20; break; /* CALL */
		case 0x18: tmp1 = Sreg(PS); Sreg(PS) = GetnextRMWord; PUSH(tmp1); PUSH(nec_state->ip); nec_state->ip = tmp; CHANGE_PC; nec_state->icount-=(ModRM >=0xc0 )?16:26; break; /* CALL FAR */
		case 0x20: nec_state->ip = tmp; CHANGE_PC; nec_state->icount-=13; break; /* JMP */
		case 0x28: nec_state->ip = tmp; Sreg(PS) = GetnextRMWord; CHANGE_PC; nec_state->icount-=15; break; /* JMP FAR */
		case 0x30: PUSH(tmp); nec_state->icount-=4; break;
//		default:   logerror("%06x: FF Pre with unimplemented mod\n",PC(nec_state));
	}
}

static void i_invalid(nec_state_t *nec_state)
{
	nec_state->icount-=10;
//	logerror("%06x: Invalid Opcode\n",PC(nec_state));
}
