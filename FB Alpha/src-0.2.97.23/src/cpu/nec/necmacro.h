/* parameter x = result, y = source 1, z = source 2 */

#define SetTF(x)		(nec_state->TF = (x))
#define SetIF(x)		(nec_state->IF = (x))
#define SetDF(x)		(nec_state->DF = (x))
#define SetMD(x)		(nec_state->MF = (x))	/* OB [19.07.99] Mode Flag V30 */

#define SetCFB(x)		(nec_state->CarryVal = (x) & 0x100)
#define SetCFW(x)		(nec_state->CarryVal = (x) & 0x10000)
#define SetAF(x,y,z)	(nec_state->AuxVal = ((x) ^ ((y) ^ (z))) & 0x10)
#define SetSF(x)		(nec_state->SignVal = (x))
#define SetZF(x)		(nec_state->ZeroVal = (x))
#define SetPF(x)		(nec_state->ParityVal = (x))

#define SetSZPF_Byte(x) (nec_state->SignVal=nec_state->ZeroVal=nec_state->ParityVal=(INT8)(x))
#define SetSZPF_Word(x) (nec_state->SignVal=nec_state->ZeroVal=nec_state->ParityVal=(INT16)(x))

#define SetOFW_Add(x,y,z)	(nec_state->OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x8000)
#define SetOFB_Add(x,y,z)	(nec_state->OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x80)
#define SetOFW_Sub(x,y,z)	(nec_state->OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x8000)
#define SetOFB_Sub(x,y,z)	(nec_state->OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x80)

#define ADDB { UINT32 res=dst+src; SetCFB(res); SetOFB_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define ADDW { UINT32 res=dst+src; SetCFW(res); SetOFW_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define SUBB { UINT32 res=dst-src; SetCFB(res); SetOFB_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define SUBW { UINT32 res=dst-src; SetCFW(res); SetOFW_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define ORB dst|=src; nec_state->CarryVal=nec_state->OverVal=nec_state->AuxVal=0; SetSZPF_Byte(dst)
#define ORW dst|=src; nec_state->CarryVal=nec_state->OverVal=nec_state->AuxVal=0; SetSZPF_Word(dst)

#define ANDB dst&=src; nec_state->CarryVal=nec_state->OverVal=nec_state->AuxVal=0; SetSZPF_Byte(dst)
#define ANDW dst&=src; nec_state->CarryVal=nec_state->OverVal=nec_state->AuxVal=0; SetSZPF_Word(dst)

#define XORB dst^=src; nec_state->CarryVal=nec_state->OverVal=nec_state->AuxVal=0; SetSZPF_Byte(dst)
#define XORW dst^=src; nec_state->CarryVal=nec_state->OverVal=nec_state->AuxVal=0; SetSZPF_Word(dst)

#define IncWordReg(Reg) 					\
	unsigned tmp = (unsigned)Wreg(Reg); \
	unsigned tmp1 = tmp+1;					\
	nec_state->OverVal = (tmp == 0x7fff);			\
	SetAF(tmp1,tmp,1);						\
	SetSZPF_Word(tmp1); 					\
	Wreg(Reg)=tmp1

#define DecWordReg(Reg) 					\
	unsigned tmp = (unsigned)Wreg(Reg); \
	unsigned tmp1 = tmp-1;					\
	nec_state->OverVal = (tmp == 0x8000);			\
	SetAF(tmp1,tmp,1);						\
	SetSZPF_Word(tmp1); 					\
	Wreg(Reg)=tmp1

#define JMP(flag)							\
	int tmp;								\
	EMPTY_PREFETCH();						\
	tmp = (int)((INT8)FETCH());				\
	if (flag)								\
	{										\
		static const UINT8 table[3]={3,10,10};	\
		nec_state->ip = (WORD)(nec_state->ip+tmp);			\
		nec_state->icount-=table[nec_state->chip_type/8];	\
		CHANGE_PC;							\
		return;								\
	}

#define ADJ4(param1,param2)					\
	if (AF || ((Breg(AL) & 0xf) > 9))	\
	{										\
		UINT16 tmp;							\
		tmp = Breg(AL) + param1;		\
		Breg(AL) = tmp;					\
		nec_state->AuxVal = 1;						\
		nec_state->CarryVal |= tmp & 0x100;			\
	}										\
	if (CF || (Breg(AL)>0x9f))			\
	{										\
		Breg(AL) += param2;				\
		nec_state->CarryVal = 1;						\
	}										\
	SetSZPF_Byte(Breg(AL))

#define ADJB(param1,param2)					\
	if (AF || ((Breg(AL) & 0xf) > 9))	\
	{										\
		Breg(AL) += param1;				\
		Breg(AH) += param2;				\
		nec_state->AuxVal = 1;						\
		nec_state->CarryVal = 1;						\
	}										\
	else									\
	{										\
		nec_state->AuxVal = 0;						\
		nec_state->CarryVal = 0;						\
	}										\
	Breg(AL) &= 0x0F

#define BITOP_BYTE							\
	ModRM = FETCH();							\
	if (ModRM >= 0xc0) {					\
		tmp=Breg(Mod_RM.RM.b[ModRM]);	\
	}										\
	else {									\
		(*GetEA[ModRM])(nec_state);					\
		tmp=read_mem_byte(EA);					\
	}

#define BITOP_WORD							\
	ModRM = FETCH();							\
	if (ModRM >= 0xc0) {					\
		tmp=Wreg(Mod_RM.RM.w[ModRM]);	\
	}										\
	else {									\
		(*GetEA[ModRM])(nec_state);					\
		tmp=read_mem_word(EA);					\
	}

#define BIT_NOT								\
	if (tmp & (1<<tmp2))					\
		tmp &= ~(1<<tmp2);					\
	else									\
		tmp |= (1<<tmp2)

#define XchgAWReg(Reg)						\
	WORD tmp;								\
	tmp = Wreg(Reg);					\
	Wreg(Reg) = Wreg(AW);			\
	Wreg(AW) = tmp

#define ROL_BYTE nec_state->CarryVal = dst & 0x80; dst = (dst << 1)+CF
#define ROL_WORD nec_state->CarryVal = dst & 0x8000; dst = (dst << 1)+CF
#define ROR_BYTE nec_state->CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<7)
#define ROR_WORD nec_state->CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<15)
#define ROLC_BYTE dst = (dst << 1) + CF; SetCFB(dst)
#define ROLC_WORD dst = (dst << 1) + CF; SetCFW(dst)
#define RORC_BYTE dst = (CF<<8)+dst; nec_state->CarryVal = dst & 0x01; dst >>= 1
#define RORC_WORD dst = (CF<<16)+dst; nec_state->CarryVal = dst & 0x01; dst >>= 1
#define SHL_BYTE(c) nec_state->icount-=c; dst <<= c;	SetCFB(dst); SetSZPF_Byte(dst);	PutbackRMByte(ModRM,(BYTE)dst)
#define SHL_WORD(c) nec_state->icount-=c; dst <<= c;	SetCFW(dst); SetSZPF_Word(dst);	PutbackRMWord(ModRM,(WORD)dst)
#define SHR_BYTE(c) nec_state->icount-=c; dst >>= c-1; nec_state->CarryVal = dst & 0x1; dst >>= 1; SetSZPF_Byte(dst); PutbackRMByte(ModRM,(BYTE)dst)
#define SHR_WORD(c) nec_state->icount-=c; dst >>= c-1; nec_state->CarryVal = dst & 0x1; dst >>= 1; SetSZPF_Word(dst); PutbackRMWord(ModRM,(WORD)dst)
#define SHRA_BYTE(c) nec_state->icount-=c; dst = ((INT8)dst) >> (c-1);	nec_state->CarryVal = dst & 0x1;	dst = ((INT8)((BYTE)dst)) >> 1; SetSZPF_Byte(dst); PutbackRMByte(ModRM,(BYTE)dst)
#define SHRA_WORD(c) nec_state->icount-=c; dst = ((INT16)dst) >> (c-1);	nec_state->CarryVal = dst & 0x1;	dst = ((INT16)((WORD)dst)) >> 1; SetSZPF_Word(dst); PutbackRMWord(ModRM,(WORD)dst)

#define DIVUB												\
	uresult = Wreg(AW);									\
	uresult2 = uresult % tmp;								\
	if ((uresult /= tmp) > 0xff) {							\
		nec_interrupt(nec_state, NEC_DIVIDE_VECTOR, BRK); break;							\
	} else {												\
		Breg(AL) = uresult;								\
		Breg(AH) = uresult2;							\
	}

#define DIVB												\
	result = (INT16)Wreg(AW);							\
	result2 = result % (INT16)((INT8)tmp);					\
	if ((result /= (INT16)((INT8)tmp)) > 0xff) {			\
		nec_interrupt(nec_state, NEC_DIVIDE_VECTOR, BRK); break;							\
	} else {												\
		Breg(AL) = result;								\
		Breg(AH) = result2;								\
	}

#define DIVUW												\
	uresult = (((UINT32)Wreg(DW)) << 16) | Wreg(AW);\
	uresult2 = uresult % tmp;								\
	if ((uresult /= tmp) > 0xffff) {						\
		nec_interrupt(nec_state, NEC_DIVIDE_VECTOR, BRK); break;							\
	} else {												\
		Wreg(AW)=uresult;								\
		Wreg(DW)=uresult2;								\
	}

#define DIVW												\
	result = ((UINT32)Wreg(DW) << 16) + Wreg(AW);	\
	result2 = result % (INT32)((INT16)tmp);					\
	if ((result /= (INT32)((INT16)tmp)) > 0xffff) {			\
		nec_interrupt(nec_state, NEC_DIVIDE_VECTOR, BRK); break;							\
	} else {												\
		Wreg(AW)=result;								\
		Wreg(DW)=result2;								\
	}

#define ADD4S {												\
	int i,v1,v2,result;										\
	int count = (Breg(CL)+1)/2;							\
	unsigned di = Wreg(IY);								\
	unsigned si = Wreg(IX);								\
	static const UINT8 table[3]={18,19,19};					\
	nec_state->ZeroVal = nec_state->CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		nec_state->icount-=table[nec_state->chip_type/8];					\
		tmp = GetMemB(DS0, si);								\
		tmp2 = GetMemB(DS1, di);							\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		result = v1+v2+nec_state->CarryVal;							\
		nec_state->CarryVal = result > 99 ? 1 : 0;					\
		result = result % 100;								\
		v1 = ((result/10)<<4) | (result % 10);				\
		PutMemB(DS1, di,v1);								\
		if (v1) nec_state->ZeroVal = 1;								\
		si++;												\
		di++;												\
	}														\
}

#define SUB4S {												\
	int count = (Breg(CL)+1)/2;							\
	int i;												\
	unsigned int v1,v2,result;								\
	unsigned di = Wreg(IY);								\
	unsigned si = Wreg(IX);								\
	static const UINT8 table[3]={18,19,19};					\
	nec_state->ZeroVal = nec_state->CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		nec_state->icount-=table[nec_state->chip_type/8];					\
		tmp = GetMemB(DS1, di);								\
		tmp2 = GetMemB(DS0, si);							\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		if (v1 < (v2+nec_state->CarryVal)) {							\
			v1+=100;										\
			result = v1-(v2+nec_state->CarryVal);					\
			nec_state->CarryVal = 1;									\
		} else {											\
			result = v1-(v2+nec_state->CarryVal);					\
			nec_state->CarryVal = 0;									\
		}													\
		v1 = ((result/10)<<4) | (result % 10);				\
		PutMemB(DS1, di,v1);								\
		if (v1) nec_state->ZeroVal = 1;								\
		si++;												\
		di++;												\
	}													\
}

#define CMP4S {												\
	int count = (Breg(CL)+1)/2;							\
	int i;										\
	unsigned int v1,v2,result;								\
	unsigned di = Wreg(IY);								\
	unsigned si = Wreg(IX);								\
	static const UINT8 table[3]={14,19,19};					\
	nec_state->ZeroVal = nec_state->CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		nec_state->icount-=table[nec_state->chip_type/8];					\
		tmp = GetMemB(DS1, di);								\
		tmp2 = GetMemB(DS0, si);							\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		if (v1 < (v2+nec_state->CarryVal)) {							\
			v1+=100;										\
			result = v1-(v2+nec_state->CarryVal);					\
			nec_state->CarryVal = 1;									\
		} else {											\
			result = v1-(v2+nec_state->CarryVal);					\
			nec_state->CarryVal = 0;									\
		}													\
		v1 = ((result/10)<<4) | (result % 10);				\
		if (v1) nec_state->ZeroVal = 1;								\
		si++;												\
		di++;												\
	}														\
}
