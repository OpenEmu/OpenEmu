static struct {
	struct {
		WREGS w[256];
		BREGS b[256];
	} reg;
	struct {
		WREGS w[256];
		BREGS b[256];
	} RM;
} Mod_RM;

#define RegWord(ModRM) I.regs.w[Mod_RM.reg.w[ModRM]]
#define RegByte(ModRM) I.regs.b[Mod_RM.reg.b[ModRM]]

#define GetRMWord(ModRM) \
	((ModRM) >= 0xc0 ? I.regs.w[Mod_RM.RM.w[ModRM]] : ( (*GetEA[ModRM])(), ReadWord( EA ) ))

#define PutbackRMWord(ModRM,val) 			     \
{ 							     \
	if (ModRM >= 0xc0) I.regs.w[Mod_RM.RM.w[ModRM]]=val; \
    else WriteWord(EA,val);  \
}

#define GetnextRMWord ReadWord((EA&0xf0000)|((EA+2)&0xffff))

#define PutRMWord(ModRM,val)				\
{							\
	if (ModRM >= 0xc0)				\
		I.regs.w[Mod_RM.RM.w[ModRM]]=val;	\
	else {						\
		(*GetEA[ModRM])();			\
		WriteWord( EA ,val);			\
	}						\
}

#define PutImmRMWord(ModRM) 				\
{							\
	uint16 val;					\
	if (ModRM >= 0xc0)				\
		FETCHuint16(I.regs.w[Mod_RM.RM.w[ModRM]]) \
	else {						\
		(*GetEA[ModRM])();			\
		FETCHuint16(val)				\
		WriteWord( EA , val);			\
	}						\
}
	
#define GetRMByte(ModRM) \
	((ModRM) >= 0xc0 ? I.regs.b[Mod_RM.RM.b[ModRM]] : ReadByte( (*GetEA[ModRM])() ))
	
#define PutRMByte(ModRM,val)				\
{							\
	if (ModRM >= 0xc0)				\
		I.regs.b[Mod_RM.RM.b[ModRM]]=val;	\
	else						\
		WriteByte( (*GetEA[ModRM])() ,val); 	\
}

#define PutImmRMByte(ModRM) 				\
{							\
	if (ModRM >= 0xc0)				\
		I.regs.b[Mod_RM.RM.b[ModRM]]=FETCH; 	\
	else {						\
		(*GetEA[ModRM])();			\
		WriteByte( EA , FETCH );		\
	}						\
}
	
#define PutbackRMByte(ModRM,val)			\
{							\
	if (ModRM >= 0xc0)				\
		I.regs.b[Mod_RM.RM.b[ModRM]]=val;	\
	else						\
		WriteByte(EA,val);			\
}

#define DEF_br8							\
	uint32 ModRM = FETCH,src,dst;		\
	src = RegByte(ModRM);				\
    dst = GetRMByte(ModRM)
    
#define DEF_wr16						\
	uint32 ModRM = FETCH,src,dst;		\
	src = RegWord(ModRM);				\
    dst = GetRMWord(ModRM)

#define DEF_r8b							\
	uint32 ModRM = FETCH,src,dst;		\
	dst = RegByte(ModRM);				\
    src = GetRMByte(ModRM)

#define DEF_r16w						\
	uint32 ModRM = FETCH,src,dst;		\
	dst = RegWord(ModRM);				\
    src = GetRMWord(ModRM)

#define DEF_ald8						\
	uint32 src = FETCH;					\
	uint32 dst = I.regs.b[AL]

#define DEF_axd16						\
	uint32 src = FETCH; 				\
	uint32 dst = I.regs.w[AW];			\
    src += (FETCH << 8)
