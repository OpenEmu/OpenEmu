#define cpu_readop cpu_readmem20	
#define cpu_readop_arg cpu_readmem20	

typedef enum { DS1, PS, SS, DS0 } SREGS;
typedef enum { AW, CW, DW, BW, SP, BP, IX, IY } WREGS;

#define NEC_NMI_INT_VECTOR 2

#ifdef LSB_FIRST
typedef enum { AL,AH,CL,CH,DL,DH,BL,BH,SPL,SPH,BPL,BPH,IXL,IXH,IYL,IYH } BREGS;
#else
typedef enum { AH,AL,CH,CL,DH,DL,BH,BL,SPH,SPL,BPH,BPL,IXH,IXL,IYH,IYL } BREGS;
#endif

/* parameter x = result, y = source 1, z = source 2 */

#define SetTF(x)		(I.TF = (x))
#define SetIF(x)		(I.IF = (x))
#define SetDF(x)		(I.DF = (x))

#define SetCFB(x)		(I.CarryVal = (x) & 0x100)
#define SetCFW(x)		(I.CarryVal = (x) & 0x10000)

#define SetAF(x,y,z)	(I.AuxVal = ((x) ^ ((y) ^ (z))) & 0x10)




#define SetSF(x)		(I.SignVal = (x))
#define SetZF(x)		(I.ZeroVal = (x))
#define SetPF(x)		(I.ParityVal = (x))

#define SetSZPF_Byte(x) (I.SignVal=I.ZeroVal=I.ParityVal=(int8)(x))
#define SetSZPF_Word(x) (I.SignVal=I.ZeroVal=I.ParityVal=(int16)(x))

#define SetOFW_Add(x,y,z)	(I.OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x8000)
#define SetOFB_Add(x,y,z)	(I.OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x80)
#define SetOFW_Sub(x,y,z)	(I.OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x8000)
#define SetOFB_Sub(x,y,z)	(I.OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x80)

#define ADDB { uint32 res=dst+src; SetCFB(res); SetOFB_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(uint8)res; }
#define ADDW { uint32 res=dst+src; SetCFW(res); SetOFW_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(uint16)res; }

#define SUBB { uint32 res=dst-src; SetCFB(res); SetOFB_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(uint8)res; }
#define SUBW { uint32 res=dst-src; SetCFW(res); SetOFW_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(uint16)res; }

#define ORB dst|=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Byte(dst)
#define ORW dst|=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Word(dst)

#define ANDB dst&=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Byte(dst)
#define ANDW dst&=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Word(dst)

#define XORB dst^=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Byte(dst)
#define XORW dst^=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Word(dst)

#define CF		(I.CarryVal!=0)
#define SF		(I.SignVal<0)
#define ZF		(I.ZeroVal==0)
#define PF		parity_table[(uint8)I.ParityVal]
#define AF		(I.AuxVal!=0)
#define FLAG_O		(I.OverVal!=0)

/************************************************************************/

#define SegBase(Seg) (I.sregs[Seg] << 4)

#define DefaultBase(Seg) ((seg_prefix && (Seg==DS0 || Seg==SS)) ? prefix_base : I.sregs[Seg] << 4)

#define GetMemB(Seg,Off) ((uint8)cpu_readmem20((DefaultBase(Seg)+(Off))))
#define GetMemW(Seg,Off) ((uint16) cpu_readmem20((DefaultBase(Seg)+(Off))) + (cpu_readmem20((DefaultBase(Seg)+((Off)+1)))<<8) )

#define PutMemB(Seg,Off,x) { cpu_writemem20((DefaultBase(Seg)+(Off)),(x)); }
#define PutMemW(Seg,Off,x) { PutMemB(Seg,Off,(x)&0xff); PutMemB(Seg,(Off)+1,(uint8)((x)>>8)); }

/* Todo:  Remove these later - plus readword could overflow */
#define ReadByte(ea) ((uint8)cpu_readmem20((ea)))
#define ReadWord(ea) (cpu_readmem20((ea))+(cpu_readmem20(((ea)+1))<<8))
#define WriteByte(ea,val) { cpu_writemem20((ea),val); }
#define WriteWord(ea,val) { cpu_writemem20((ea),(uint8)(val)); cpu_writemem20(((ea)+1),(val)>>8); }

#define read_port(port) cpu_readport(port)
#define write_port(port,val) cpu_writeport(port,val)

#define FETCH (cpu_readop_arg((I.sregs[PS]<<4)+I.pc++))
#define FETCHOP (cpu_readop((I.sregs[PS]<<4)+I.pc++))
#define FETCHuint16(var) { var=cpu_readop_arg((((I.sregs[PS]<<4)+I.pc)))+(cpu_readop_arg((((I.sregs[PS]<<4)+I.pc+1)))<<8); I.pc+=2; }
#define PUSH(val) { I.regs.w[SP]-=2; WriteWord((((I.sregs[SS]<<4)+I.regs.w[SP])),val); }
#define POP(var) { var = ReadWord((((I.sregs[SS]<<4)+I.regs.w[SP]))); I.regs.w[SP]+=2; }
#define PEEK(addr) ((uint8)cpu_readop_arg(addr))
#define PEEKOP(addr) ((uint8)cpu_readop(addr))

#define GetModRM uint32 ModRM=cpu_readop_arg((I.sregs[PS]<<4)+I.pc++)

/* Cycle count macros:
	CLK  - cycle count is the same on all processors
	CLKM - cycle count for reg/mem instructions


	Prefetch & buswait time is not emulated.
	Extra cycles for PUSH'ing or POP'ing registers to odd addresses is not emulated.
*/

#define _REAL_CLK(cycles) { v30mz_ICount -= cycles; v30mz_timestamp += cycles; }
#define CLK _REAL_CLK
//#define CLK(cycles) { _REAL_CLK(cycles); if(ws_CheckDMA(cycles)) _REAL_CLK(1); }

#define CLKM(mcount, ccount) { if(ModRM >=0xc0 ) { CLK(ccount);} else {CLK(mcount);} }


#define CompressFlags() (uint16)(CF | (PF << 2) | (AF << 4) | (ZF << 6) \
				| (SF << 7) | (I.TF << 8) | (I.IF << 9) \
				| (I.DF << 10) | (FLAG_O << 11) | (0xF002))


#define ExpandFlags(f) \
{ \
	I.CarryVal = (f) & 1; \
	I.ParityVal = !((f) & 4); \
	I.AuxVal = (f) & 16; \
	I.ZeroVal = !((f) & 64); \
	I.SignVal = (f) & 128 ? -1 : 0; \
	I.TF = ((f) & 256) == 256; \
	I.IF = ((f) & 512) == 512; \
	I.DF = ((f) & 1024) == 1024; \
	I.OverVal = (f) & 2048; \
}



#define IncWordReg(Reg) 					\
	unsigned tmp = (unsigned)I.regs.w[Reg]; \
	unsigned tmp1 = tmp+1;					\
	I.OverVal = (tmp == 0x7fff); 			\
	SetAF(tmp1,tmp,1);						\
	SetSZPF_Word(tmp1); 					\
	I.regs.w[Reg]=tmp1



#define DecWordReg(Reg) 					\
	unsigned tmp = (unsigned)I.regs.w[Reg]; \
    unsigned tmp1 = tmp-1; 					\
	I.OverVal = (tmp == 0x8000); 			\
    SetAF(tmp1,tmp,1); 						\
    SetSZPF_Word(tmp1); 					\
	I.regs.w[Reg]=tmp1

#define JMP(flag)							\
	int tmp = (int)((int8)FETCH);			\
	if (flag)								\
	{										\
		I.pc = (uint16)(I.pc+tmp);			\
		CLK(3);	\
		ADDBRANCHTRACE(I.sregs[PS], I.pc);		\
		return;								\
	}

#define ADJ4(param1,param2)					\
	if (AF || ((I.regs.b[AL] & 0xf) > 9))	\
	{										\
		uint16 tmp;							\
		tmp = I.regs.b[AL] + param1;	\
		I.regs.b[AL] = tmp;	\
		I.AuxVal = 1;						\
		I.CarryVal |= tmp & 0x100; /*if(tmp&0x100){puts("Meow"); }*//* Correct? */	\
	}										\
	if (CF || (I.regs.b[AL] > 0x9f))		\
	{										\
		I.regs.b[AL] += param2;				\
		I.CarryVal = 1;						\
	}										\
	SetSZPF_Byte(I.regs.b[AL])

#define ADJB(param1,param2)					\
	if (AF || ((I.regs.b[AL] & 0xf) > 9))	\
    {										\
		I.regs.b[AL] += param1;				\
		I.regs.b[AH] += param2;				\
		I.AuxVal = 1;						\
		I.CarryVal = 1;						\
    }										\
	else									\
	{										\
		I.AuxVal = 0;						\
		I.CarryVal = 0;						\
    }										\
	I.regs.b[AL] &= 0x0F

#define BIT_NOT								\
	if (tmp & (1<<tmp2))					\
		tmp &= ~(1<<tmp2);					\
	else									\
		tmp |= (1<<tmp2)

#define XchgAWReg(Reg) 						\
    uint16 tmp; 								\
	tmp = I.regs.w[Reg]; 					\
	I.regs.w[Reg] = I.regs.w[AW]; 			\
	I.regs.w[AW] = tmp

#define ROL_uint8 I.CarryVal = dst & 0x80; dst = (dst << 1)+CF
#define ROL_uint16 I.CarryVal = dst & 0x8000; dst = (dst << 1)+CF
#define ROR_uint8 I.CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<7)
#define ROR_uint16 I.CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<15)
#define ROLC_uint8 dst = (dst << 1) + CF; SetCFB(dst)
#define ROLC_uint16 dst = (dst << 1) + CF; SetCFW(dst)
#define RORC_uint8 dst = (CF<<8)+dst; I.CarryVal = dst & 0x01; dst >>= 1
#define RORC_uint16 dst = (CF<<16)+dst; I.CarryVal = dst & 0x01; dst >>= 1
#define SHL_uint8(c) dst <<= c;	SetCFB(dst); SetSZPF_Byte(dst);	PutbackRMByte(ModRM,(uint8)dst)
#define SHL_uint16(c) dst <<= c;	SetCFW(dst); SetSZPF_Word(dst);	PutbackRMWord(ModRM,(uint16)dst)
#define SHR_uint8(c) dst >>= c-1; I.CarryVal = dst & 0x1; dst >>= 1; SetSZPF_Byte(dst); PutbackRMByte(ModRM,(uint8)dst)
#define SHR_uint16(c) dst >>= c-1; I.CarryVal = dst & 0x1; dst >>= 1; SetSZPF_Word(dst); PutbackRMWord(ModRM,(uint16)dst)
#define SHRA_uint8(c) dst = ((int8)dst) >> (c-1);	I.CarryVal = dst & 0x1;	dst = ((int8)((uint8)dst)) >> 1; SetSZPF_Byte(dst); PutbackRMByte(ModRM,(uint8)dst)
#define SHRA_uint16(c) dst = ((int16)dst) >> (c-1);	I.CarryVal = dst & 0x1;	dst = ((int16)((uint16)dst)) >> 1; SetSZPF_Word(dst); PutbackRMWord(ModRM,(uint16)dst)

#define DIVUB												\
	uresult = I.regs.w[AW];									\
	uresult2 = uresult % tmp;								\
	if ((uresult /= tmp) > 0xff) {							\
		nec_interrupt(0); break;							\
	} else {												\
		I.regs.b[AL] = uresult;								\
		I.regs.b[AH] = uresult2;							\
	}

#define DIVB												\
	result = (int16)I.regs.w[AW];							\
	result2 = result % (int16)((int8)tmp);					\
	if ((result /= (int16)((int8)tmp)) > 0xff) {			\
		nec_interrupt(0); break;							\
	} else {												\
		I.regs.b[AL] = result;								\
		I.regs.b[AH] = result2;								\
	}

#define DIVUW												\
	uresult = (((uint32)I.regs.w[DW]) << 16) | I.regs.w[AW];\
	uresult2 = uresult % tmp;								\
	if ((uresult /= tmp) > 0xffff) {						\
		nec_interrupt(0); break;							\
	} else {												\
		I.regs.w[AW]=uresult;								\
		I.regs.w[DW]=uresult2;								\
	}

#define DIVW												\
	result = ((uint32)I.regs.w[DW] << 16) + I.regs.w[AW];	\
	result2 = result % (int32)((int16)tmp);					\
	if ((result /= (int32)((int16)tmp)) > 0xffff) {			\
		nec_interrupt(0); break;							\
	} else {												\
		I.regs.w[AW]=result;								\
		I.regs.w[DW]=result2;								\
	}

