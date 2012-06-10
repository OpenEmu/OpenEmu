case 0x0001:
case 0x0002:
case 0x0003:
case 0x0004:
case 0x0005:
case 0x0006:
case 0x0007:

// ORI
case 0x0000:
{
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(0)
case 0x0011:
case 0x0012:
case 0x0013:
case 0x0014:
case 0x0015:
case 0x0016:
case 0x0017:

// ORI
case 0x0010:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_BYat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0019:
case 0x001A:
case 0x001B:
case 0x001C:
case 0x001D:
case 0x001E:

// ORI
case 0x0018:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	timestamp += 4;
	READ_BYat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0021:
case 0x0022:
case 0x0023:
case 0x0024:
case 0x0025:
case 0x0026:

// ORI
case 0x0020:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0029:
case 0x002A:
case 0x002B:
case 0x002C:
case 0x002D:
case 0x002E:
case 0x002F:

// ORI
case 0x0028:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0031:
case 0x0032:
case 0x0033:
case 0x0034:
case 0x0035:
case 0x0036:
case 0x0037:

// ORI
case 0x0030:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_BYat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// ORI
case 0x0038:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// ORI
case 0x0039:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_BYat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// ORI
case 0x001F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// ORI
case 0x0027:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0041:
case 0x0042:
case 0x0043:
case 0x0044:
case 0x0045:
case 0x0046:
case 0x0047:

// ORI
case 0x0040:
{
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(0)
case 0x0051:
case 0x0052:
case 0x0053:
case 0x0054:
case 0x0055:
case 0x0056:
case 0x0057:

// ORI
case 0x0050:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_WOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0059:
case 0x005A:
case 0x005B:
case 0x005C:
case 0x005D:
case 0x005E:

// ORI
case 0x0058:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	timestamp += 4;
	READ_WOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0061:
case 0x0062:
case 0x0063:
case 0x0064:
case 0x0065:
case 0x0066:

// ORI
case 0x0060:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_WOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0069:
case 0x006A:
case 0x006B:
case 0x006C:
case 0x006D:
case 0x006E:
case 0x006F:

// ORI
case 0x0068:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_WOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0071:
case 0x0072:
case 0x0073:
case 0x0074:
case 0x0075:
case 0x0076:
case 0x0077:

// ORI
case 0x0070:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_WOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)

// ORI
case 0x0078:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_WOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)

// ORI
case 0x0079:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_WOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)

// ORI
case 0x005F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	timestamp += 4;
	READ_WOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)

// ORI
case 0x0067:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_WOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0081:
case 0x0082:
case 0x0083:
case 0x0084:
case 0x0085:
case 0x0086:
case 0x0087:

// ORI
case 0x0080:
{
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0x0091:
case 0x0092:
case 0x0093:
case 0x0094:
case 0x0095:
case 0x0096:
case 0x0097:

// ORI
case 0x0090:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 12;
	READ_LOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x0099:
case 0x009A:
case 0x009B:
case 0x009C:
case 0x009D:
case 0x009E:

// ORI
case 0x0098:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	timestamp += 12;
	READ_LOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x00A1:
case 0x00A2:
case 0x00A3:
case 0x00A4:
case 0x00A5:
case 0x00A6:

// ORI
case 0x00A0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 14;
	READ_LOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x00A9:
case 0x00AA:
case 0x00AB:
case 0x00AC:
case 0x00AD:
case 0x00AE:
case 0x00AF:

// ORI
case 0x00A8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 16;
	READ_LOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x00B1:
case 0x00B2:
case 0x00B3:
case 0x00B4:
case 0x00B5:
case 0x00B6:
case 0x00B7:

// ORI
case 0x00B0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 18;
	READ_LOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// ORI
case 0x00B8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 16;
	READ_LOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// ORI
case 0x00B9:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 20;
	READ_LOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// ORI
case 0x009F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	timestamp += 12;
	READ_LOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// ORI
case 0x00A7:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	timestamp += 14;
	READ_LOat_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// ORICCR
case 0x003C:
{
	u32 res;
	res = FETCH_BYTE & C68K_CCR_MASK;
	PC += 2;
	res |= GET_CCR;
	SET_CCR(res)
}
RET(20)

// ORISR
case 0x007C:
{
	u32 res;
	if (CPU->flag_S)
	{
		res = FETCH_WORD & C68K_SR_MASK;
		PC += 2;
		res |= GET_SR;
		SET_SR(res)
	}
	else
	{
		res = CPU->USP;
		CPU->USP = CPU->A[7];
		CPU->A[7] = res;
		res = C68K_PRIVILEGE_VIOLATION_EX;
	timestamp += c68k_exception_cycle_table[res];
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
timestamp += 20;
goto C68k_Exec_End;
case 0x0201:
case 0x0202:
case 0x0203:
case 0x0204:
case 0x0205:
case 0x0206:
case 0x0207:

// ANDI
case 0x0200:
{
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(0)
case 0x0211:
case 0x0212:
case 0x0213:
case 0x0214:
case 0x0215:
case 0x0216:
case 0x0217:

// ANDI
case 0x0210:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_BYat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0219:
case 0x021A:
case 0x021B:
case 0x021C:
case 0x021D:
case 0x021E:

// ANDI
case 0x0218:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	timestamp += 4;
	READ_BYat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0221:
case 0x0222:
case 0x0223:
case 0x0224:
case 0x0225:
case 0x0226:

// ANDI
case 0x0220:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0229:
case 0x022A:
case 0x022B:
case 0x022C:
case 0x022D:
case 0x022E:
case 0x022F:

// ANDI
case 0x0228:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0231:
case 0x0232:
case 0x0233:
case 0x0234:
case 0x0235:
case 0x0236:
case 0x0237:

// ANDI
case 0x0230:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_BYat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// ANDI
case 0x0238:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// ANDI
case 0x0239:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_BYat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// ANDI
case 0x021F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// ANDI
case 0x0227:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0241:
case 0x0242:
case 0x0243:
case 0x0244:
case 0x0245:
case 0x0246:
case 0x0247:

// ANDI
case 0x0240:
{
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(0)
case 0x0251:
case 0x0252:
case 0x0253:
case 0x0254:
case 0x0255:
case 0x0256:
case 0x0257:

// ANDI
case 0x0250:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_WOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0259:
case 0x025A:
case 0x025B:
case 0x025C:
case 0x025D:
case 0x025E:

// ANDI
case 0x0258:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	timestamp += 4;
	READ_WOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0261:
case 0x0262:
case 0x0263:
case 0x0264:
case 0x0265:
case 0x0266:

// ANDI
case 0x0260:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_WOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0269:
case 0x026A:
case 0x026B:
case 0x026C:
case 0x026D:
case 0x026E:
case 0x026F:

// ANDI
case 0x0268:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_WOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0271:
case 0x0272:
case 0x0273:
case 0x0274:
case 0x0275:
case 0x0276:
case 0x0277:

// ANDI
case 0x0270:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_WOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)

// ANDI
case 0x0278:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_WOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)

// ANDI
case 0x0279:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_WOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)

// ANDI
case 0x025F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	timestamp += 4;
	READ_WOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)

// ANDI
case 0x0267:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_WOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0281:
case 0x0282:
case 0x0283:
case 0x0284:
case 0x0285:
case 0x0286:
case 0x0287:

// ANDI
case 0x0280:
{
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0x0291:
case 0x0292:
case 0x0293:
case 0x0294:
case 0x0295:
case 0x0296:
case 0x0297:

// ANDI
case 0x0290:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 12;
	READ_LOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x0299:
case 0x029A:
case 0x029B:
case 0x029C:
case 0x029D:
case 0x029E:

// ANDI
case 0x0298:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	timestamp += 12;
	READ_LOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x02A1:
case 0x02A2:
case 0x02A3:
case 0x02A4:
case 0x02A5:
case 0x02A6:

// ANDI
case 0x02A0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 14;
	READ_LOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x02A9:
case 0x02AA:
case 0x02AB:
case 0x02AC:
case 0x02AD:
case 0x02AE:
case 0x02AF:

// ANDI
case 0x02A8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 16;
	READ_LOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x02B1:
case 0x02B2:
case 0x02B3:
case 0x02B4:
case 0x02B5:
case 0x02B6:
case 0x02B7:

// ANDI
case 0x02B0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 18;
	READ_LOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// ANDI
case 0x02B8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 16;
	READ_LOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// ANDI
case 0x02B9:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 20;
	READ_LOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// ANDI
case 0x029F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	timestamp += 12;
	READ_LOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// ANDI
case 0x02A7:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	timestamp += 14;
	READ_LOat_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// ANDICCR
case 0x023C:
{
	u32 res;
	res = FETCH_BYTE & C68K_CCR_MASK;
	PC += 2;
	res &= GET_CCR;
	SET_CCR(res)
}
RET(20)

// ANDISR
case 0x027C:
{
	u32 res;
	if (CPU->flag_S)
	{
		res = FETCH_WORD & C68K_SR_MASK;
		PC += 2;
		res &= GET_SR;
		SET_SR(res)
		if (!CPU->flag_S)
		{
			res = CPU->A[7];
			CPU->A[7] = CPU->USP;
			CPU->USP = res;
		}
	}
	else
	{
		res = CPU->USP;
		CPU->USP = CPU->A[7];
		CPU->A[7] = res;
		res = C68K_PRIVILEGE_VIOLATION_EX;
	timestamp += c68k_exception_cycle_table[res];
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
timestamp += 20;
goto C68k_Exec_End;
case 0x0A01:
case 0x0A02:
case 0x0A03:
case 0x0A04:
case 0x0A05:
case 0x0A06:
case 0x0A07:

// EORI
case 0x0A00:
{
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(0)
case 0x0A11:
case 0x0A12:
case 0x0A13:
case 0x0A14:
case 0x0A15:
case 0x0A16:
case 0x0A17:

// EORI
case 0x0A10:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_BYat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0A19:
case 0x0A1A:
case 0x0A1B:
case 0x0A1C:
case 0x0A1D:
case 0x0A1E:

// EORI
case 0x0A18:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	timestamp += 4;
	READ_BYat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0A21:
case 0x0A22:
case 0x0A23:
case 0x0A24:
case 0x0A25:
case 0x0A26:

// EORI
case 0x0A20:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0A29:
case 0x0A2A:
case 0x0A2B:
case 0x0A2C:
case 0x0A2D:
case 0x0A2E:
case 0x0A2F:

// EORI
case 0x0A28:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0A31:
case 0x0A32:
case 0x0A33:
case 0x0A34:
case 0x0A35:
case 0x0A36:
case 0x0A37:

// EORI
case 0x0A30:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_BYat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// EORI
case 0x0A38:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// EORI
case 0x0A39:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_BYat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// EORI
case 0x0A1F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// EORI
case 0x0A27:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0A41:
case 0x0A42:
case 0x0A43:
case 0x0A44:
case 0x0A45:
case 0x0A46:
case 0x0A47:

// EORI
case 0x0A40:
{
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(0)
case 0x0A51:
case 0x0A52:
case 0x0A53:
case 0x0A54:
case 0x0A55:
case 0x0A56:
case 0x0A57:

// EORI
case 0x0A50:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_WOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0A59:
case 0x0A5A:
case 0x0A5B:
case 0x0A5C:
case 0x0A5D:
case 0x0A5E:

// EORI
case 0x0A58:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	timestamp += 4;
	READ_WOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0A61:
case 0x0A62:
case 0x0A63:
case 0x0A64:
case 0x0A65:
case 0x0A66:

// EORI
case 0x0A60:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_WOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0A69:
case 0x0A6A:
case 0x0A6B:
case 0x0A6C:
case 0x0A6D:
case 0x0A6E:
case 0x0A6F:

// EORI
case 0x0A68:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_WOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0A71:
case 0x0A72:
case 0x0A73:
case 0x0A74:
case 0x0A75:
case 0x0A76:
case 0x0A77:

// EORI
case 0x0A70:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_WOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)

// EORI
case 0x0A78:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_WOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)

// EORI
case 0x0A79:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_WOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)

// EORI
case 0x0A5F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	timestamp += 4;
	READ_WOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)

// EORI
case 0x0A67:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_WOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0A81:
case 0x0A82:
case 0x0A83:
case 0x0A84:
case 0x0A85:
case 0x0A86:
case 0x0A87:

// EORI
case 0x0A80:
{
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0x0A91:
case 0x0A92:
case 0x0A93:
case 0x0A94:
case 0x0A95:
case 0x0A96:
case 0x0A97:

// EORI
case 0x0A90:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 12;
	READ_LOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x0A99:
case 0x0A9A:
case 0x0A9B:
case 0x0A9C:
case 0x0A9D:
case 0x0A9E:

// EORI
case 0x0A98:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	timestamp += 12;
	READ_LOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x0AA1:
case 0x0AA2:
case 0x0AA3:
case 0x0AA4:
case 0x0AA5:
case 0x0AA6:

// EORI
case 0x0AA0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 14;
	READ_LOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x0AA9:
case 0x0AAA:
case 0x0AAB:
case 0x0AAC:
case 0x0AAD:
case 0x0AAE:
case 0x0AAF:

// EORI
case 0x0AA8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 16;
	READ_LOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x0AB1:
case 0x0AB2:
case 0x0AB3:
case 0x0AB4:
case 0x0AB5:
case 0x0AB6:
case 0x0AB7:

// EORI
case 0x0AB0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 18;
	READ_LOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// EORI
case 0x0AB8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 16;
	READ_LOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// EORI
case 0x0AB9:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 20;
	READ_LOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// EORI
case 0x0A9F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	timestamp += 12;
	READ_LOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// EORI
case 0x0AA7:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	timestamp += 14;
	READ_LOat_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// EORICCR
case 0x0A3C:
{
	u32 res;
	res = FETCH_BYTE & C68K_CCR_MASK;
	PC += 2;
	res ^= GET_CCR;
	SET_CCR(res)
}
RET(20)

// EORISR
case 0x0A7C:
{
	u32 res;
	if (CPU->flag_S)
	{
		res = FETCH_WORD & C68K_SR_MASK;
		PC += 2;
		res ^= GET_SR;
		SET_SR(res)
		if (!CPU->flag_S)
		{
			res = CPU->A[7];
			CPU->A[7] = CPU->USP;
			CPU->USP = res;
		}
	}
	else
	{
		res = CPU->USP;
		CPU->USP = CPU->A[7];
		CPU->A[7] = res;
		res = C68K_PRIVILEGE_VIOLATION_EX;
	timestamp += c68k_exception_cycle_table[res];
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
timestamp += 20;
goto C68k_Exec_End;
case 0x0401:
case 0x0402:
case 0x0403:
case 0x0404:
case 0x0405:
case 0x0406:
case 0x0407:

// SUBI
case 0x0400:
{
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	dst = (u8)CPU->D[(Opcode >> 0) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(0)
case 0x0411:
case 0x0412:
case 0x0413:
case 0x0414:
case 0x0415:
case 0x0416:
case 0x0417:

// SUBI
case 0x0410:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0419:
case 0x041A:
case 0x041B:
case 0x041C:
case 0x041D:
case 0x041E:

// SUBI
case 0x0418:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	timestamp += 4;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0421:
case 0x0422:
case 0x0423:
case 0x0424:
case 0x0425:
case 0x0426:

// SUBI
case 0x0420:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0429:
case 0x042A:
case 0x042B:
case 0x042C:
case 0x042D:
case 0x042E:
case 0x042F:

// SUBI
case 0x0428:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0431:
case 0x0432:
case 0x0433:
case 0x0434:
case 0x0435:
case 0x0436:
case 0x0437:

// SUBI
case 0x0430:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// SUBI
case 0x0438:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// SUBI
case 0x0439:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// SUBI
case 0x041F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	timestamp += 4;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// SUBI
case 0x0427:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0441:
case 0x0442:
case 0x0443:
case 0x0444:
case 0x0445:
case 0x0446:
case 0x0447:

// SUBI
case 0x0440:
{
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	dst = (u16)CPU->D[(Opcode >> 0) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(0)
case 0x0451:
case 0x0452:
case 0x0453:
case 0x0454:
case 0x0455:
case 0x0456:
case 0x0457:

// SUBI
case 0x0450:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0459:
case 0x045A:
case 0x045B:
case 0x045C:
case 0x045D:
case 0x045E:

// SUBI
case 0x0458:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	timestamp += 4;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0461:
case 0x0462:
case 0x0463:
case 0x0464:
case 0x0465:
case 0x0466:

// SUBI
case 0x0460:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0469:
case 0x046A:
case 0x046B:
case 0x046C:
case 0x046D:
case 0x046E:
case 0x046F:

// SUBI
case 0x0468:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0471:
case 0x0472:
case 0x0473:
case 0x0474:
case 0x0475:
case 0x0476:
case 0x0477:

// SUBI
case 0x0470:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)

// SUBI
case 0x0478:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)

// SUBI
case 0x0479:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)

// SUBI
case 0x045F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	timestamp += 4;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)

// SUBI
case 0x0467:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0481:
case 0x0482:
case 0x0483:
case 0x0484:
case 0x0485:
case 0x0486:
case 0x0487:

// SUBI
case 0x0480:
{
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	dst = (u32)CPU->D[(Opcode >> 0) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0x0491:
case 0x0492:
case 0x0493:
case 0x0494:
case 0x0495:
case 0x0496:
case 0x0497:

// SUBI
case 0x0490:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 12;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x0499:
case 0x049A:
case 0x049B:
case 0x049C:
case 0x049D:
case 0x049E:

// SUBI
case 0x0498:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	timestamp += 12;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x04A1:
case 0x04A2:
case 0x04A3:
case 0x04A4:
case 0x04A5:
case 0x04A6:

// SUBI
case 0x04A0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 14;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x04A9:
case 0x04AA:
case 0x04AB:
case 0x04AC:
case 0x04AD:
case 0x04AE:
case 0x04AF:

// SUBI
case 0x04A8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 16;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x04B1:
case 0x04B2:
case 0x04B3:
case 0x04B4:
case 0x04B5:
case 0x04B6:
case 0x04B7:

// SUBI
case 0x04B0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 18;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// SUBI
case 0x04B8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 16;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// SUBI
case 0x04B9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 20;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// SUBI
case 0x049F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	timestamp += 12;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// SUBI
case 0x04A7:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	timestamp += 14;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x0601:
case 0x0602:
case 0x0603:
case 0x0604:
case 0x0605:
case 0x0606:
case 0x0607:

// ADDI
case 0x0600:
{
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	dst = (u8)CPU->D[(Opcode >> 0) & 7];
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(0)
case 0x0611:
case 0x0612:
case 0x0613:
case 0x0614:
case 0x0615:
case 0x0616:
case 0x0617:

// ADDI
case 0x0610:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_BYat_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0619:
case 0x061A:
case 0x061B:
case 0x061C:
case 0x061D:
case 0x061E:

// ADDI
case 0x0618:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	timestamp += 4;
	READ_BYat_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0621:
case 0x0622:
case 0x0623:
case 0x0624:
case 0x0625:
case 0x0626:

// ADDI
case 0x0620:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0629:
case 0x062A:
case 0x062B:
case 0x062C:
case 0x062D:
case 0x062E:
case 0x062F:

// ADDI
case 0x0628:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0631:
case 0x0632:
case 0x0633:
case 0x0634:
case 0x0635:
case 0x0636:
case 0x0637:

// ADDI
case 0x0630:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_BYat_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// ADDI
case 0x0638:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// ADDI
case 0x0639:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_BYat_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// ADDI
case 0x061F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	timestamp += 4;
	READ_BYat_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)

// ADDI
case 0x0627:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
}
RET(0)
case 0x0641:
case 0x0642:
case 0x0643:
case 0x0644:
case 0x0645:
case 0x0646:
case 0x0647:

// ADDI
case 0x0640:
{
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	dst = (u16)CPU->D[(Opcode >> 0) & 7];
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(0)
case 0x0651:
case 0x0652:
case 0x0653:
case 0x0654:
case 0x0655:
case 0x0656:
case 0x0657:

// ADDI
case 0x0650:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_WOat_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0659:
case 0x065A:
case 0x065B:
case 0x065C:
case 0x065D:
case 0x065E:

// ADDI
case 0x0658:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	timestamp += 4;
	READ_WOat_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0661:
case 0x0662:
case 0x0663:
case 0x0664:
case 0x0665:
case 0x0666:

// ADDI
case 0x0660:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_WOat_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0669:
case 0x066A:
case 0x066B:
case 0x066C:
case 0x066D:
case 0x066E:
case 0x066F:

// ADDI
case 0x0668:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_WOat_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0671:
case 0x0672:
case 0x0673:
case 0x0674:
case 0x0675:
case 0x0676:
case 0x0677:

// ADDI
case 0x0670:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_WOat_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)

// ADDI
case 0x0678:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_WOat_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)

// ADDI
case 0x0679:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_WOat_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)

// ADDI
case 0x065F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	timestamp += 4;
	READ_WOat_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)

// ADDI
case 0x0667:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_WOat_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
}
RET(0)
case 0x0681:
case 0x0682:
case 0x0683:
case 0x0684:
case 0x0685:
case 0x0686:
case 0x0687:

// ADDI
case 0x0680:
{
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	dst = (u32)CPU->D[(Opcode >> 0) & 7];
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0x0691:
case 0x0692:
case 0x0693:
case 0x0694:
case 0x0695:
case 0x0696:
case 0x0697:

// ADDI
case 0x0690:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 12;
	READ_LOat_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x0699:
case 0x069A:
case 0x069B:
case 0x069C:
case 0x069D:
case 0x069E:

// ADDI
case 0x0698:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	timestamp += 12;
	READ_LOat_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x06A1:
case 0x06A2:
case 0x06A3:
case 0x06A4:
case 0x06A5:
case 0x06A6:

// ADDI
case 0x06A0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 14;
	READ_LOat_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x06A9:
case 0x06AA:
case 0x06AB:
case 0x06AC:
case 0x06AD:
case 0x06AE:
case 0x06AF:

// ADDI
case 0x06A8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 16;
	READ_LOat_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x06B1:
case 0x06B2:
case 0x06B3:
case 0x06B4:
case 0x06B5:
case 0x06B6:
case 0x06B7:

// ADDI
case 0x06B0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 18;
	READ_LOat_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// ADDI
case 0x06B8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 16;
	READ_LOat_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// ADDI
case 0x06B9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 20;
	READ_LOat_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// ADDI
case 0x069F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	timestamp += 12;
	READ_LOat_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)

// ADDI
case 0x06A7:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	timestamp += 14;
	READ_LOat_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
}
RET(0)
case 0x0C01:
case 0x0C02:
case 0x0C03:
case 0x0C04:
case 0x0C05:
case 0x0C06:
case 0x0C07:

// CMPI
case 0x0C00:
{
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	dst = (u8)CPU->D[(Opcode >> 0) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
}
RET(0)
case 0x0C11:
case 0x0C12:
case 0x0C13:
case 0x0C14:
case 0x0C15:
case 0x0C16:
case 0x0C17:

// CMPI
case 0x0C10:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
}
RET(0)
case 0x0C19:
case 0x0C1A:
case 0x0C1B:
case 0x0C1C:
case 0x0C1D:
case 0x0C1E:

// CMPI
case 0x0C18:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
}
RET(0)
case 0x0C21:
case 0x0C22:
case 0x0C23:
case 0x0C24:
case 0x0C25:
case 0x0C26:

// CMPI
case 0x0C20:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 2;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
}
RET(0)
case 0x0C29:
case 0x0C2A:
case 0x0C2B:
case 0x0C2C:
case 0x0C2D:
case 0x0C2E:
case 0x0C2F:

// CMPI
case 0x0C28:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
}
RET(0)
case 0x0C31:
case 0x0C32:
case 0x0C33:
case 0x0C34:
case 0x0C35:
case 0x0C36:
case 0x0C37:

// CMPI
case 0x0C30:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 6;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
}
RET(0)

// CMPI
case 0x0C38:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
}
RET(0)

// CMPI
case 0x0C39:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 8;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
}
RET(0)

// CMPI
case 0x0C1F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
}
RET(0)

// CMPI
case 0x0C27:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_BYTE;
	PC += 2;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 2;
	READ_BYat_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
}
RET(0)
case 0x0C41:
case 0x0C42:
case 0x0C43:
case 0x0C44:
case 0x0C45:
case 0x0C46:
case 0x0C47:

// CMPI
case 0x0C40:
{
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	dst = (u16)CPU->D[(Opcode >> 0) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
}
RET(0)
case 0x0C51:
case 0x0C52:
case 0x0C53:
case 0x0C54:
case 0x0C55:
case 0x0C56:
case 0x0C57:

// CMPI
case 0x0C50:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
}
RET(0)
case 0x0C59:
case 0x0C5A:
case 0x0C5B:
case 0x0C5C:
case 0x0C5D:
case 0x0C5E:

// CMPI
case 0x0C58:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
}
RET(0)
case 0x0C61:
case 0x0C62:
case 0x0C63:
case 0x0C64:
case 0x0C65:
case 0x0C66:

// CMPI
case 0x0C60:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 2;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
}
RET(0)
case 0x0C69:
case 0x0C6A:
case 0x0C6B:
case 0x0C6C:
case 0x0C6D:
case 0x0C6E:
case 0x0C6F:

// CMPI
case 0x0C68:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
}
RET(0)
case 0x0C71:
case 0x0C72:
case 0x0C73:
case 0x0C74:
case 0x0C75:
case 0x0C76:
case 0x0C77:

// CMPI
case 0x0C70:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 6;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
}
RET(0)

// CMPI
case 0x0C78:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
}
RET(0)

// CMPI
case 0x0C79:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 8;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
}
RET(0)

// CMPI
case 0x0C5F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
}
RET(0)

// CMPI
case 0x0C67:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 2;
	READ_WOat_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
}
RET(0)
case 0x0C81:
case 0x0C82:
case 0x0C83:
case 0x0C84:
case 0x0C85:
case 0x0C86:
case 0x0C87:

// CMPI
case 0x0C80:
{
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	dst = (u32)CPU->D[(Opcode >> 0) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(6)
case 0x0C91:
case 0x0C92:
case 0x0C93:
case 0x0C94:
case 0x0C95:
case 0x0C96:
case 0x0C97:

// CMPI
case 0x0C90:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(0)
case 0x0C99:
case 0x0C9A:
case 0x0C9B:
case 0x0C9C:
case 0x0C9D:
case 0x0C9E:

// CMPI
case 0x0C98:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	timestamp += 4;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(0)
case 0x0CA1:
case 0x0CA2:
case 0x0CA3:
case 0x0CA4:
case 0x0CA5:
case 0x0CA6:

// CMPI
case 0x0CA0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(0)
case 0x0CA9:
case 0x0CAA:
case 0x0CAB:
case 0x0CAC:
case 0x0CAD:
case 0x0CAE:
case 0x0CAF:

// CMPI
case 0x0CA8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(0)
case 0x0CB1:
case 0x0CB2:
case 0x0CB3:
case 0x0CB4:
case 0x0CB5:
case 0x0CB6:
case 0x0CB7:

// CMPI
case 0x0CB0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(0)

// CMPI
case 0x0CB8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(0)

// CMPI
case 0x0CB9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(0)

// CMPI
case 0x0C9F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	timestamp += 4;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(0)

// CMPI
case 0x0CA7:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	timestamp += 8;
	src = FETCH_LONG;
	PC += 4;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_LOat_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(0)
case 0x0801:
case 0x0802:
case 0x0803:
case 0x0804:
case 0x0805:
case 0x0806:
case 0x0807:

// BTSTn
case 0x0800:
{
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 31);
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_notZ = res & src;
}
RET(6)
case 0x0811:
case 0x0812:
case 0x0813:
case 0x0814:
case 0x0815:
case 0x0816:
case 0x0817:

// BTSTn
case 0x0810:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x0819:
case 0x081A:
case 0x081B:
case 0x081C:
case 0x081D:
case 0x081E:

// BTSTn
case 0x0818:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x0821:
case 0x0822:
case 0x0823:
case 0x0824:
case 0x0825:
case 0x0826:

// BTSTn
case 0x0820:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x0829:
case 0x082A:
case 0x082B:
case 0x082C:
case 0x082D:
case 0x082E:
case 0x082F:

// BTSTn
case 0x0828:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x0831:
case 0x0832:
case 0x0833:
case 0x0834:
case 0x0835:
case 0x0836:
case 0x0837:

// BTSTn
case 0x0830:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)

// BTSTn
case 0x0838:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)

// BTSTn
case 0x0839:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)

// BTSTn
case 0x083A:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)

// BTSTn
case 0x083B:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = PC;
	DECODE_EXT_WORD
	timestamp += 10;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)

// BTSTn
case 0x081F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[7];
	CPU->A[7] += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)

// BTSTn
case 0x0827:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x0841:
case 0x0842:
case 0x0843:
case 0x0844:
case 0x0845:
case 0x0846:
case 0x0847:

// BCHGn
case 0x0840:
{
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 31);
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_notZ = res & src;
	res ^= src;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0x0851:
case 0x0852:
case 0x0853:
case 0x0854:
case 0x0855:
case 0x0856:
case 0x0857:

// BCHGn
case 0x0850:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0859:
case 0x085A:
case 0x085B:
case 0x085C:
case 0x085D:
case 0x085E:

// BCHGn
case 0x0858:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0861:
case 0x0862:
case 0x0863:
case 0x0864:
case 0x0865:
case 0x0866:

// BCHGn
case 0x0860:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0869:
case 0x086A:
case 0x086B:
case 0x086C:
case 0x086D:
case 0x086E:
case 0x086F:

// BCHGn
case 0x0868:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0871:
case 0x0872:
case 0x0873:
case 0x0874:
case 0x0875:
case 0x0876:
case 0x0877:

// BCHGn
case 0x0870:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)

// BCHGn
case 0x0878:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)

// BCHGn
case 0x0879:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)

// BCHGn
case 0x085F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[7];
	CPU->A[7] += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)

// BCHGn
case 0x0867:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0881:
case 0x0882:
case 0x0883:
case 0x0884:
case 0x0885:
case 0x0886:
case 0x0887:

// BCLRn
case 0x0880:
{
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 31);
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_notZ = res & src;
	res &= ~src;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(10)
case 0x0891:
case 0x0892:
case 0x0893:
case 0x0894:
case 0x0895:
case 0x0896:
case 0x0897:

// BCLRn
case 0x0890:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0899:
case 0x089A:
case 0x089B:
case 0x089C:
case 0x089D:
case 0x089E:

// BCLRn
case 0x0898:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x08A1:
case 0x08A2:
case 0x08A3:
case 0x08A4:
case 0x08A5:
case 0x08A6:

// BCLRn
case 0x08A0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x08A9:
case 0x08AA:
case 0x08AB:
case 0x08AC:
case 0x08AD:
case 0x08AE:
case 0x08AF:

// BCLRn
case 0x08A8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x08B1:
case 0x08B2:
case 0x08B3:
case 0x08B4:
case 0x08B5:
case 0x08B6:
case 0x08B7:

// BCLRn
case 0x08B0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)

// BCLRn
case 0x08B8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)

// BCLRn
case 0x08B9:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)

// BCLRn
case 0x089F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[7];
	CPU->A[7] += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)

// BCLRn
case 0x08A7:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x08C1:
case 0x08C2:
case 0x08C3:
case 0x08C4:
case 0x08C5:
case 0x08C6:
case 0x08C7:

// BSETn
case 0x08C0:
{
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 31);
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_notZ = res & src;
	res |= src;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0x08D1:
case 0x08D2:
case 0x08D3:
case 0x08D4:
case 0x08D5:
case 0x08D6:
case 0x08D7:

// BSETn
case 0x08D0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x08D9:
case 0x08DA:
case 0x08DB:
case 0x08DC:
case 0x08DD:
case 0x08DE:

// BSETn
case 0x08D8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x08E1:
case 0x08E2:
case 0x08E3:
case 0x08E4:
case 0x08E5:
case 0x08E6:

// BSETn
case 0x08E0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x08E9:
case 0x08EA:
case 0x08EB:
case 0x08EC:
case 0x08ED:
case 0x08EE:
case 0x08EF:

// BSETn
case 0x08E8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x08F1:
case 0x08F2:
case 0x08F3:
case 0x08F4:
case 0x08F5:
case 0x08F6:
case 0x08F7:

// BSETn
case 0x08F0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 10;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)

// BSETn
case 0x08F8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 8;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)

// BSETn
case 0x08F9:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 12;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)

// BSETn
case 0x08DF:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[7];
	CPU->A[7] += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)

// BSETn
case 0x08E7:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = FETCH_BYTE;
	PC += 2;
	src = 1 << (src & 7);
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 6;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0300:
case 0x0500:
case 0x0700:
case 0x0900:
case 0x0B00:
case 0x0D00:
case 0x0F00:
case 0x0101:
case 0x0301:
case 0x0501:
case 0x0701:
case 0x0901:
case 0x0B01:
case 0x0D01:
case 0x0F01:
case 0x0102:
case 0x0302:
case 0x0502:
case 0x0702:
case 0x0902:
case 0x0B02:
case 0x0D02:
case 0x0F02:
case 0x0103:
case 0x0303:
case 0x0503:
case 0x0703:
case 0x0903:
case 0x0B03:
case 0x0D03:
case 0x0F03:
case 0x0104:
case 0x0304:
case 0x0504:
case 0x0704:
case 0x0904:
case 0x0B04:
case 0x0D04:
case 0x0F04:
case 0x0105:
case 0x0305:
case 0x0505:
case 0x0705:
case 0x0905:
case 0x0B05:
case 0x0D05:
case 0x0F05:
case 0x0106:
case 0x0306:
case 0x0506:
case 0x0706:
case 0x0906:
case 0x0B06:
case 0x0D06:
case 0x0F06:
case 0x0107:
case 0x0307:
case 0x0507:
case 0x0707:
case 0x0907:
case 0x0B07:
case 0x0D07:
case 0x0F07:

// BTST
case 0x0100:
{
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 31);
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_notZ = res & src;
}
RET(2)
case 0x0310:
case 0x0510:
case 0x0710:
case 0x0910:
case 0x0B10:
case 0x0D10:
case 0x0F10:
case 0x0111:
case 0x0311:
case 0x0511:
case 0x0711:
case 0x0911:
case 0x0B11:
case 0x0D11:
case 0x0F11:
case 0x0112:
case 0x0312:
case 0x0512:
case 0x0712:
case 0x0912:
case 0x0B12:
case 0x0D12:
case 0x0F12:
case 0x0113:
case 0x0313:
case 0x0513:
case 0x0713:
case 0x0913:
case 0x0B13:
case 0x0D13:
case 0x0F13:
case 0x0114:
case 0x0314:
case 0x0514:
case 0x0714:
case 0x0914:
case 0x0B14:
case 0x0D14:
case 0x0F14:
case 0x0115:
case 0x0315:
case 0x0515:
case 0x0715:
case 0x0915:
case 0x0B15:
case 0x0D15:
case 0x0F15:
case 0x0116:
case 0x0316:
case 0x0516:
case 0x0716:
case 0x0916:
case 0x0B16:
case 0x0D16:
case 0x0F16:
case 0x0117:
case 0x0317:
case 0x0517:
case 0x0717:
case 0x0917:
case 0x0B17:
case 0x0D17:
case 0x0F17:

// BTST
case 0x0110:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x0318:
case 0x0518:
case 0x0718:
case 0x0918:
case 0x0B18:
case 0x0D18:
case 0x0F18:
case 0x0119:
case 0x0319:
case 0x0519:
case 0x0719:
case 0x0919:
case 0x0B19:
case 0x0D19:
case 0x0F19:
case 0x011A:
case 0x031A:
case 0x051A:
case 0x071A:
case 0x091A:
case 0x0B1A:
case 0x0D1A:
case 0x0F1A:
case 0x011B:
case 0x031B:
case 0x051B:
case 0x071B:
case 0x091B:
case 0x0B1B:
case 0x0D1B:
case 0x0F1B:
case 0x011C:
case 0x031C:
case 0x051C:
case 0x071C:
case 0x091C:
case 0x0B1C:
case 0x0D1C:
case 0x0F1C:
case 0x011D:
case 0x031D:
case 0x051D:
case 0x071D:
case 0x091D:
case 0x0B1D:
case 0x0D1D:
case 0x0F1D:
case 0x011E:
case 0x031E:
case 0x051E:
case 0x071E:
case 0x091E:
case 0x0B1E:
case 0x0D1E:
case 0x0F1E:

// BTST
case 0x0118:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x0320:
case 0x0520:
case 0x0720:
case 0x0920:
case 0x0B20:
case 0x0D20:
case 0x0F20:
case 0x0121:
case 0x0321:
case 0x0521:
case 0x0721:
case 0x0921:
case 0x0B21:
case 0x0D21:
case 0x0F21:
case 0x0122:
case 0x0322:
case 0x0522:
case 0x0722:
case 0x0922:
case 0x0B22:
case 0x0D22:
case 0x0F22:
case 0x0123:
case 0x0323:
case 0x0523:
case 0x0723:
case 0x0923:
case 0x0B23:
case 0x0D23:
case 0x0F23:
case 0x0124:
case 0x0324:
case 0x0524:
case 0x0724:
case 0x0924:
case 0x0B24:
case 0x0D24:
case 0x0F24:
case 0x0125:
case 0x0325:
case 0x0525:
case 0x0725:
case 0x0925:
case 0x0B25:
case 0x0D25:
case 0x0F25:
case 0x0126:
case 0x0326:
case 0x0526:
case 0x0726:
case 0x0926:
case 0x0B26:
case 0x0D26:
case 0x0F26:

// BTST
case 0x0120:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 2;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x0328:
case 0x0528:
case 0x0728:
case 0x0928:
case 0x0B28:
case 0x0D28:
case 0x0F28:
case 0x0129:
case 0x0329:
case 0x0529:
case 0x0729:
case 0x0929:
case 0x0B29:
case 0x0D29:
case 0x0F29:
case 0x012A:
case 0x032A:
case 0x052A:
case 0x072A:
case 0x092A:
case 0x0B2A:
case 0x0D2A:
case 0x0F2A:
case 0x012B:
case 0x032B:
case 0x052B:
case 0x072B:
case 0x092B:
case 0x0B2B:
case 0x0D2B:
case 0x0F2B:
case 0x012C:
case 0x032C:
case 0x052C:
case 0x072C:
case 0x092C:
case 0x0B2C:
case 0x0D2C:
case 0x0F2C:
case 0x012D:
case 0x032D:
case 0x052D:
case 0x072D:
case 0x092D:
case 0x0B2D:
case 0x0D2D:
case 0x0F2D:
case 0x012E:
case 0x032E:
case 0x052E:
case 0x072E:
case 0x092E:
case 0x0B2E:
case 0x0D2E:
case 0x0F2E:
case 0x012F:
case 0x032F:
case 0x052F:
case 0x072F:
case 0x092F:
case 0x0B2F:
case 0x0D2F:
case 0x0F2F:

// BTST
case 0x0128:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x0330:
case 0x0530:
case 0x0730:
case 0x0930:
case 0x0B30:
case 0x0D30:
case 0x0F30:
case 0x0131:
case 0x0331:
case 0x0531:
case 0x0731:
case 0x0931:
case 0x0B31:
case 0x0D31:
case 0x0F31:
case 0x0132:
case 0x0332:
case 0x0532:
case 0x0732:
case 0x0932:
case 0x0B32:
case 0x0D32:
case 0x0F32:
case 0x0133:
case 0x0333:
case 0x0533:
case 0x0733:
case 0x0933:
case 0x0B33:
case 0x0D33:
case 0x0F33:
case 0x0134:
case 0x0334:
case 0x0534:
case 0x0734:
case 0x0934:
case 0x0B34:
case 0x0D34:
case 0x0F34:
case 0x0135:
case 0x0335:
case 0x0535:
case 0x0735:
case 0x0935:
case 0x0B35:
case 0x0D35:
case 0x0F35:
case 0x0136:
case 0x0336:
case 0x0536:
case 0x0736:
case 0x0936:
case 0x0B36:
case 0x0D36:
case 0x0F36:
case 0x0137:
case 0x0337:
case 0x0537:
case 0x0737:
case 0x0937:
case 0x0B37:
case 0x0D37:
case 0x0F37:

// BTST
case 0x0130:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 6;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x0338:
case 0x0538:
case 0x0738:
case 0x0938:
case 0x0B38:
case 0x0D38:
case 0x0F38:

// BTST
case 0x0138:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x0339:
case 0x0539:
case 0x0739:
case 0x0939:
case 0x0B39:
case 0x0D39:
case 0x0F39:

// BTST
case 0x0139:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 8;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x033A:
case 0x053A:
case 0x073A:
case 0x093A:
case 0x0B3A:
case 0x0D3A:
case 0x0F3A:

// BTST
case 0x013A:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x033B:
case 0x053B:
case 0x073B:
case 0x093B:
case 0x0B3B:
case 0x0D3B:
case 0x0F3B:

// BTST
case 0x013B:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = PC;
	DECODE_EXT_WORD
	timestamp += 6;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x033C:
case 0x053C:
case 0x073C:
case 0x093C:
case 0x0B3C:
case 0x0D3C:
case 0x0F3C:

// BTST
case 0x013C:
{
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	res = FETCH_BYTE;
	PC += 2;
	CPU->flag_notZ = res & src;
}
RET(4)
case 0x031F:
case 0x051F:
case 0x071F:
case 0x091F:
case 0x0B1F:
case 0x0D1F:
case 0x0F1F:

// BTST
case 0x011F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[7];
	CPU->A[7] += 2;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x0327:
case 0x0527:
case 0x0727:
case 0x0927:
case 0x0B27:
case 0x0D27:
case 0x0F27:

// BTST
case 0x0127:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 2;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
}
RET(0)
case 0x0340:
case 0x0540:
case 0x0740:
case 0x0940:
case 0x0B40:
case 0x0D40:
case 0x0F40:
case 0x0141:
case 0x0341:
case 0x0541:
case 0x0741:
case 0x0941:
case 0x0B41:
case 0x0D41:
case 0x0F41:
case 0x0142:
case 0x0342:
case 0x0542:
case 0x0742:
case 0x0942:
case 0x0B42:
case 0x0D42:
case 0x0F42:
case 0x0143:
case 0x0343:
case 0x0543:
case 0x0743:
case 0x0943:
case 0x0B43:
case 0x0D43:
case 0x0F43:
case 0x0144:
case 0x0344:
case 0x0544:
case 0x0744:
case 0x0944:
case 0x0B44:
case 0x0D44:
case 0x0F44:
case 0x0145:
case 0x0345:
case 0x0545:
case 0x0745:
case 0x0945:
case 0x0B45:
case 0x0D45:
case 0x0F45:
case 0x0146:
case 0x0346:
case 0x0546:
case 0x0746:
case 0x0946:
case 0x0B46:
case 0x0D46:
case 0x0F46:
case 0x0147:
case 0x0347:
case 0x0547:
case 0x0747:
case 0x0947:
case 0x0B47:
case 0x0D47:
case 0x0F47:

// BCHG
case 0x0140:
{
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 31);
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_notZ = res & src;
	res ^= src;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x0350:
case 0x0550:
case 0x0750:
case 0x0950:
case 0x0B50:
case 0x0D50:
case 0x0F50:
case 0x0151:
case 0x0351:
case 0x0551:
case 0x0751:
case 0x0951:
case 0x0B51:
case 0x0D51:
case 0x0F51:
case 0x0152:
case 0x0352:
case 0x0552:
case 0x0752:
case 0x0952:
case 0x0B52:
case 0x0D52:
case 0x0F52:
case 0x0153:
case 0x0353:
case 0x0553:
case 0x0753:
case 0x0953:
case 0x0B53:
case 0x0D53:
case 0x0F53:
case 0x0154:
case 0x0354:
case 0x0554:
case 0x0754:
case 0x0954:
case 0x0B54:
case 0x0D54:
case 0x0F54:
case 0x0155:
case 0x0355:
case 0x0555:
case 0x0755:
case 0x0955:
case 0x0B55:
case 0x0D55:
case 0x0F55:
case 0x0156:
case 0x0356:
case 0x0556:
case 0x0756:
case 0x0956:
case 0x0B56:
case 0x0D56:
case 0x0F56:
case 0x0157:
case 0x0357:
case 0x0557:
case 0x0757:
case 0x0957:
case 0x0B57:
case 0x0D57:
case 0x0F57:

// BCHG
case 0x0150:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0358:
case 0x0558:
case 0x0758:
case 0x0958:
case 0x0B58:
case 0x0D58:
case 0x0F58:
case 0x0159:
case 0x0359:
case 0x0559:
case 0x0759:
case 0x0959:
case 0x0B59:
case 0x0D59:
case 0x0F59:
case 0x015A:
case 0x035A:
case 0x055A:
case 0x075A:
case 0x095A:
case 0x0B5A:
case 0x0D5A:
case 0x0F5A:
case 0x015B:
case 0x035B:
case 0x055B:
case 0x075B:
case 0x095B:
case 0x0B5B:
case 0x0D5B:
case 0x0F5B:
case 0x015C:
case 0x035C:
case 0x055C:
case 0x075C:
case 0x095C:
case 0x0B5C:
case 0x0D5C:
case 0x0F5C:
case 0x015D:
case 0x035D:
case 0x055D:
case 0x075D:
case 0x095D:
case 0x0B5D:
case 0x0D5D:
case 0x0F5D:
case 0x015E:
case 0x035E:
case 0x055E:
case 0x075E:
case 0x095E:
case 0x0B5E:
case 0x0D5E:
case 0x0F5E:

// BCHG
case 0x0158:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0360:
case 0x0560:
case 0x0760:
case 0x0960:
case 0x0B60:
case 0x0D60:
case 0x0F60:
case 0x0161:
case 0x0361:
case 0x0561:
case 0x0761:
case 0x0961:
case 0x0B61:
case 0x0D61:
case 0x0F61:
case 0x0162:
case 0x0362:
case 0x0562:
case 0x0762:
case 0x0962:
case 0x0B62:
case 0x0D62:
case 0x0F62:
case 0x0163:
case 0x0363:
case 0x0563:
case 0x0763:
case 0x0963:
case 0x0B63:
case 0x0D63:
case 0x0F63:
case 0x0164:
case 0x0364:
case 0x0564:
case 0x0764:
case 0x0964:
case 0x0B64:
case 0x0D64:
case 0x0F64:
case 0x0165:
case 0x0365:
case 0x0565:
case 0x0765:
case 0x0965:
case 0x0B65:
case 0x0D65:
case 0x0F65:
case 0x0166:
case 0x0366:
case 0x0566:
case 0x0766:
case 0x0966:
case 0x0B66:
case 0x0D66:
case 0x0F66:

// BCHG
case 0x0160:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 2;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0368:
case 0x0568:
case 0x0768:
case 0x0968:
case 0x0B68:
case 0x0D68:
case 0x0F68:
case 0x0169:
case 0x0369:
case 0x0569:
case 0x0769:
case 0x0969:
case 0x0B69:
case 0x0D69:
case 0x0F69:
case 0x016A:
case 0x036A:
case 0x056A:
case 0x076A:
case 0x096A:
case 0x0B6A:
case 0x0D6A:
case 0x0F6A:
case 0x016B:
case 0x036B:
case 0x056B:
case 0x076B:
case 0x096B:
case 0x0B6B:
case 0x0D6B:
case 0x0F6B:
case 0x016C:
case 0x036C:
case 0x056C:
case 0x076C:
case 0x096C:
case 0x0B6C:
case 0x0D6C:
case 0x0F6C:
case 0x016D:
case 0x036D:
case 0x056D:
case 0x076D:
case 0x096D:
case 0x0B6D:
case 0x0D6D:
case 0x0F6D:
case 0x016E:
case 0x036E:
case 0x056E:
case 0x076E:
case 0x096E:
case 0x0B6E:
case 0x0D6E:
case 0x0F6E:
case 0x016F:
case 0x036F:
case 0x056F:
case 0x076F:
case 0x096F:
case 0x0B6F:
case 0x0D6F:
case 0x0F6F:

// BCHG
case 0x0168:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0370:
case 0x0570:
case 0x0770:
case 0x0970:
case 0x0B70:
case 0x0D70:
case 0x0F70:
case 0x0171:
case 0x0371:
case 0x0571:
case 0x0771:
case 0x0971:
case 0x0B71:
case 0x0D71:
case 0x0F71:
case 0x0172:
case 0x0372:
case 0x0572:
case 0x0772:
case 0x0972:
case 0x0B72:
case 0x0D72:
case 0x0F72:
case 0x0173:
case 0x0373:
case 0x0573:
case 0x0773:
case 0x0973:
case 0x0B73:
case 0x0D73:
case 0x0F73:
case 0x0174:
case 0x0374:
case 0x0574:
case 0x0774:
case 0x0974:
case 0x0B74:
case 0x0D74:
case 0x0F74:
case 0x0175:
case 0x0375:
case 0x0575:
case 0x0775:
case 0x0975:
case 0x0B75:
case 0x0D75:
case 0x0F75:
case 0x0176:
case 0x0376:
case 0x0576:
case 0x0776:
case 0x0976:
case 0x0B76:
case 0x0D76:
case 0x0F76:
case 0x0177:
case 0x0377:
case 0x0577:
case 0x0777:
case 0x0977:
case 0x0B77:
case 0x0D77:
case 0x0F77:

// BCHG
case 0x0170:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 6;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0378:
case 0x0578:
case 0x0778:
case 0x0978:
case 0x0B78:
case 0x0D78:
case 0x0F78:

// BCHG
case 0x0178:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0379:
case 0x0579:
case 0x0779:
case 0x0979:
case 0x0B79:
case 0x0D79:
case 0x0F79:

// BCHG
case 0x0179:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 8;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x035F:
case 0x055F:
case 0x075F:
case 0x095F:
case 0x0B5F:
case 0x0D5F:
case 0x0F5F:

// BCHG
case 0x015F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[7];
	CPU->A[7] += 2;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0367:
case 0x0567:
case 0x0767:
case 0x0967:
case 0x0B67:
case 0x0D67:
case 0x0F67:

// BCHG
case 0x0167:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 2;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res ^= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0380:
case 0x0580:
case 0x0780:
case 0x0980:
case 0x0B80:
case 0x0D80:
case 0x0F80:
case 0x0181:
case 0x0381:
case 0x0581:
case 0x0781:
case 0x0981:
case 0x0B81:
case 0x0D81:
case 0x0F81:
case 0x0182:
case 0x0382:
case 0x0582:
case 0x0782:
case 0x0982:
case 0x0B82:
case 0x0D82:
case 0x0F82:
case 0x0183:
case 0x0383:
case 0x0583:
case 0x0783:
case 0x0983:
case 0x0B83:
case 0x0D83:
case 0x0F83:
case 0x0184:
case 0x0384:
case 0x0584:
case 0x0784:
case 0x0984:
case 0x0B84:
case 0x0D84:
case 0x0F84:
case 0x0185:
case 0x0385:
case 0x0585:
case 0x0785:
case 0x0985:
case 0x0B85:
case 0x0D85:
case 0x0F85:
case 0x0186:
case 0x0386:
case 0x0586:
case 0x0786:
case 0x0986:
case 0x0B86:
case 0x0D86:
case 0x0F86:
case 0x0187:
case 0x0387:
case 0x0587:
case 0x0787:
case 0x0987:
case 0x0B87:
case 0x0D87:
case 0x0F87:

// BCLR
case 0x0180:
{
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 31);
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_notZ = res & src;
	res &= ~src;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0x0390:
case 0x0590:
case 0x0790:
case 0x0990:
case 0x0B90:
case 0x0D90:
case 0x0F90:
case 0x0191:
case 0x0391:
case 0x0591:
case 0x0791:
case 0x0991:
case 0x0B91:
case 0x0D91:
case 0x0F91:
case 0x0192:
case 0x0392:
case 0x0592:
case 0x0792:
case 0x0992:
case 0x0B92:
case 0x0D92:
case 0x0F92:
case 0x0193:
case 0x0393:
case 0x0593:
case 0x0793:
case 0x0993:
case 0x0B93:
case 0x0D93:
case 0x0F93:
case 0x0194:
case 0x0394:
case 0x0594:
case 0x0794:
case 0x0994:
case 0x0B94:
case 0x0D94:
case 0x0F94:
case 0x0195:
case 0x0395:
case 0x0595:
case 0x0795:
case 0x0995:
case 0x0B95:
case 0x0D95:
case 0x0F95:
case 0x0196:
case 0x0396:
case 0x0596:
case 0x0796:
case 0x0996:
case 0x0B96:
case 0x0D96:
case 0x0F96:
case 0x0197:
case 0x0397:
case 0x0597:
case 0x0797:
case 0x0997:
case 0x0B97:
case 0x0D97:
case 0x0F97:

// BCLR
case 0x0190:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0398:
case 0x0598:
case 0x0798:
case 0x0998:
case 0x0B98:
case 0x0D98:
case 0x0F98:
case 0x0199:
case 0x0399:
case 0x0599:
case 0x0799:
case 0x0999:
case 0x0B99:
case 0x0D99:
case 0x0F99:
case 0x019A:
case 0x039A:
case 0x059A:
case 0x079A:
case 0x099A:
case 0x0B9A:
case 0x0D9A:
case 0x0F9A:
case 0x019B:
case 0x039B:
case 0x059B:
case 0x079B:
case 0x099B:
case 0x0B9B:
case 0x0D9B:
case 0x0F9B:
case 0x019C:
case 0x039C:
case 0x059C:
case 0x079C:
case 0x099C:
case 0x0B9C:
case 0x0D9C:
case 0x0F9C:
case 0x019D:
case 0x039D:
case 0x059D:
case 0x079D:
case 0x099D:
case 0x0B9D:
case 0x0D9D:
case 0x0F9D:
case 0x019E:
case 0x039E:
case 0x059E:
case 0x079E:
case 0x099E:
case 0x0B9E:
case 0x0D9E:
case 0x0F9E:

// BCLR
case 0x0198:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03A0:
case 0x05A0:
case 0x07A0:
case 0x09A0:
case 0x0BA0:
case 0x0DA0:
case 0x0FA0:
case 0x01A1:
case 0x03A1:
case 0x05A1:
case 0x07A1:
case 0x09A1:
case 0x0BA1:
case 0x0DA1:
case 0x0FA1:
case 0x01A2:
case 0x03A2:
case 0x05A2:
case 0x07A2:
case 0x09A2:
case 0x0BA2:
case 0x0DA2:
case 0x0FA2:
case 0x01A3:
case 0x03A3:
case 0x05A3:
case 0x07A3:
case 0x09A3:
case 0x0BA3:
case 0x0DA3:
case 0x0FA3:
case 0x01A4:
case 0x03A4:
case 0x05A4:
case 0x07A4:
case 0x09A4:
case 0x0BA4:
case 0x0DA4:
case 0x0FA4:
case 0x01A5:
case 0x03A5:
case 0x05A5:
case 0x07A5:
case 0x09A5:
case 0x0BA5:
case 0x0DA5:
case 0x0FA5:
case 0x01A6:
case 0x03A6:
case 0x05A6:
case 0x07A6:
case 0x09A6:
case 0x0BA6:
case 0x0DA6:
case 0x0FA6:

// BCLR
case 0x01A0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 2;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03A8:
case 0x05A8:
case 0x07A8:
case 0x09A8:
case 0x0BA8:
case 0x0DA8:
case 0x0FA8:
case 0x01A9:
case 0x03A9:
case 0x05A9:
case 0x07A9:
case 0x09A9:
case 0x0BA9:
case 0x0DA9:
case 0x0FA9:
case 0x01AA:
case 0x03AA:
case 0x05AA:
case 0x07AA:
case 0x09AA:
case 0x0BAA:
case 0x0DAA:
case 0x0FAA:
case 0x01AB:
case 0x03AB:
case 0x05AB:
case 0x07AB:
case 0x09AB:
case 0x0BAB:
case 0x0DAB:
case 0x0FAB:
case 0x01AC:
case 0x03AC:
case 0x05AC:
case 0x07AC:
case 0x09AC:
case 0x0BAC:
case 0x0DAC:
case 0x0FAC:
case 0x01AD:
case 0x03AD:
case 0x05AD:
case 0x07AD:
case 0x09AD:
case 0x0BAD:
case 0x0DAD:
case 0x0FAD:
case 0x01AE:
case 0x03AE:
case 0x05AE:
case 0x07AE:
case 0x09AE:
case 0x0BAE:
case 0x0DAE:
case 0x0FAE:
case 0x01AF:
case 0x03AF:
case 0x05AF:
case 0x07AF:
case 0x09AF:
case 0x0BAF:
case 0x0DAF:
case 0x0FAF:

// BCLR
case 0x01A8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03B0:
case 0x05B0:
case 0x07B0:
case 0x09B0:
case 0x0BB0:
case 0x0DB0:
case 0x0FB0:
case 0x01B1:
case 0x03B1:
case 0x05B1:
case 0x07B1:
case 0x09B1:
case 0x0BB1:
case 0x0DB1:
case 0x0FB1:
case 0x01B2:
case 0x03B2:
case 0x05B2:
case 0x07B2:
case 0x09B2:
case 0x0BB2:
case 0x0DB2:
case 0x0FB2:
case 0x01B3:
case 0x03B3:
case 0x05B3:
case 0x07B3:
case 0x09B3:
case 0x0BB3:
case 0x0DB3:
case 0x0FB3:
case 0x01B4:
case 0x03B4:
case 0x05B4:
case 0x07B4:
case 0x09B4:
case 0x0BB4:
case 0x0DB4:
case 0x0FB4:
case 0x01B5:
case 0x03B5:
case 0x05B5:
case 0x07B5:
case 0x09B5:
case 0x0BB5:
case 0x0DB5:
case 0x0FB5:
case 0x01B6:
case 0x03B6:
case 0x05B6:
case 0x07B6:
case 0x09B6:
case 0x0BB6:
case 0x0DB6:
case 0x0FB6:
case 0x01B7:
case 0x03B7:
case 0x05B7:
case 0x07B7:
case 0x09B7:
case 0x0BB7:
case 0x0DB7:
case 0x0FB7:

// BCLR
case 0x01B0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 6;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03B8:
case 0x05B8:
case 0x07B8:
case 0x09B8:
case 0x0BB8:
case 0x0DB8:
case 0x0FB8:

// BCLR
case 0x01B8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03B9:
case 0x05B9:
case 0x07B9:
case 0x09B9:
case 0x0BB9:
case 0x0DB9:
case 0x0FB9:

// BCLR
case 0x01B9:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 8;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x039F:
case 0x059F:
case 0x079F:
case 0x099F:
case 0x0B9F:
case 0x0D9F:
case 0x0F9F:

// BCLR
case 0x019F:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[7];
	CPU->A[7] += 2;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03A7:
case 0x05A7:
case 0x07A7:
case 0x09A7:
case 0x0BA7:
case 0x0DA7:
case 0x0FA7:

// BCLR
case 0x01A7:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 2;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res &= ~src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03C0:
case 0x05C0:
case 0x07C0:
case 0x09C0:
case 0x0BC0:
case 0x0DC0:
case 0x0FC0:
case 0x01C1:
case 0x03C1:
case 0x05C1:
case 0x07C1:
case 0x09C1:
case 0x0BC1:
case 0x0DC1:
case 0x0FC1:
case 0x01C2:
case 0x03C2:
case 0x05C2:
case 0x07C2:
case 0x09C2:
case 0x0BC2:
case 0x0DC2:
case 0x0FC2:
case 0x01C3:
case 0x03C3:
case 0x05C3:
case 0x07C3:
case 0x09C3:
case 0x0BC3:
case 0x0DC3:
case 0x0FC3:
case 0x01C4:
case 0x03C4:
case 0x05C4:
case 0x07C4:
case 0x09C4:
case 0x0BC4:
case 0x0DC4:
case 0x0FC4:
case 0x01C5:
case 0x03C5:
case 0x05C5:
case 0x07C5:
case 0x09C5:
case 0x0BC5:
case 0x0DC5:
case 0x0FC5:
case 0x01C6:
case 0x03C6:
case 0x05C6:
case 0x07C6:
case 0x09C6:
case 0x0BC6:
case 0x0DC6:
case 0x0FC6:
case 0x01C7:
case 0x03C7:
case 0x05C7:
case 0x07C7:
case 0x09C7:
case 0x0BC7:
case 0x0DC7:
case 0x0FC7:

// BSET
case 0x01C0:
{
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 31);
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_notZ = res & src;
	res |= src;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x03D0:
case 0x05D0:
case 0x07D0:
case 0x09D0:
case 0x0BD0:
case 0x0DD0:
case 0x0FD0:
case 0x01D1:
case 0x03D1:
case 0x05D1:
case 0x07D1:
case 0x09D1:
case 0x0BD1:
case 0x0DD1:
case 0x0FD1:
case 0x01D2:
case 0x03D2:
case 0x05D2:
case 0x07D2:
case 0x09D2:
case 0x0BD2:
case 0x0DD2:
case 0x0FD2:
case 0x01D3:
case 0x03D3:
case 0x05D3:
case 0x07D3:
case 0x09D3:
case 0x0BD3:
case 0x0DD3:
case 0x0FD3:
case 0x01D4:
case 0x03D4:
case 0x05D4:
case 0x07D4:
case 0x09D4:
case 0x0BD4:
case 0x0DD4:
case 0x0FD4:
case 0x01D5:
case 0x03D5:
case 0x05D5:
case 0x07D5:
case 0x09D5:
case 0x0BD5:
case 0x0DD5:
case 0x0FD5:
case 0x01D6:
case 0x03D6:
case 0x05D6:
case 0x07D6:
case 0x09D6:
case 0x0BD6:
case 0x0DD6:
case 0x0FD6:
case 0x01D7:
case 0x03D7:
case 0x05D7:
case 0x07D7:
case 0x09D7:
case 0x0BD7:
case 0x0DD7:
case 0x0FD7:

// BSET
case 0x01D0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03D8:
case 0x05D8:
case 0x07D8:
case 0x09D8:
case 0x0BD8:
case 0x0DD8:
case 0x0FD8:
case 0x01D9:
case 0x03D9:
case 0x05D9:
case 0x07D9:
case 0x09D9:
case 0x0BD9:
case 0x0DD9:
case 0x0FD9:
case 0x01DA:
case 0x03DA:
case 0x05DA:
case 0x07DA:
case 0x09DA:
case 0x0BDA:
case 0x0DDA:
case 0x0FDA:
case 0x01DB:
case 0x03DB:
case 0x05DB:
case 0x07DB:
case 0x09DB:
case 0x0BDB:
case 0x0DDB:
case 0x0FDB:
case 0x01DC:
case 0x03DC:
case 0x05DC:
case 0x07DC:
case 0x09DC:
case 0x0BDC:
case 0x0DDC:
case 0x0FDC:
case 0x01DD:
case 0x03DD:
case 0x05DD:
case 0x07DD:
case 0x09DD:
case 0x0BDD:
case 0x0DDD:
case 0x0FDD:
case 0x01DE:
case 0x03DE:
case 0x05DE:
case 0x07DE:
case 0x09DE:
case 0x0BDE:
case 0x0DDE:
case 0x0FDE:

// BSET
case 0x01D8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03E0:
case 0x05E0:
case 0x07E0:
case 0x09E0:
case 0x0BE0:
case 0x0DE0:
case 0x0FE0:
case 0x01E1:
case 0x03E1:
case 0x05E1:
case 0x07E1:
case 0x09E1:
case 0x0BE1:
case 0x0DE1:
case 0x0FE1:
case 0x01E2:
case 0x03E2:
case 0x05E2:
case 0x07E2:
case 0x09E2:
case 0x0BE2:
case 0x0DE2:
case 0x0FE2:
case 0x01E3:
case 0x03E3:
case 0x05E3:
case 0x07E3:
case 0x09E3:
case 0x0BE3:
case 0x0DE3:
case 0x0FE3:
case 0x01E4:
case 0x03E4:
case 0x05E4:
case 0x07E4:
case 0x09E4:
case 0x0BE4:
case 0x0DE4:
case 0x0FE4:
case 0x01E5:
case 0x03E5:
case 0x05E5:
case 0x07E5:
case 0x09E5:
case 0x0BE5:
case 0x0DE5:
case 0x0FE5:
case 0x01E6:
case 0x03E6:
case 0x05E6:
case 0x07E6:
case 0x09E6:
case 0x0BE6:
case 0x0DE6:
case 0x0FE6:

// BSET
case 0x01E0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	timestamp += 2;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03E8:
case 0x05E8:
case 0x07E8:
case 0x09E8:
case 0x0BE8:
case 0x0DE8:
case 0x0FE8:
case 0x01E9:
case 0x03E9:
case 0x05E9:
case 0x07E9:
case 0x09E9:
case 0x0BE9:
case 0x0DE9:
case 0x0FE9:
case 0x01EA:
case 0x03EA:
case 0x05EA:
case 0x07EA:
case 0x09EA:
case 0x0BEA:
case 0x0DEA:
case 0x0FEA:
case 0x01EB:
case 0x03EB:
case 0x05EB:
case 0x07EB:
case 0x09EB:
case 0x0BEB:
case 0x0DEB:
case 0x0FEB:
case 0x01EC:
case 0x03EC:
case 0x05EC:
case 0x07EC:
case 0x09EC:
case 0x0BEC:
case 0x0DEC:
case 0x0FEC:
case 0x01ED:
case 0x03ED:
case 0x05ED:
case 0x07ED:
case 0x09ED:
case 0x0BED:
case 0x0DED:
case 0x0FED:
case 0x01EE:
case 0x03EE:
case 0x05EE:
case 0x07EE:
case 0x09EE:
case 0x0BEE:
case 0x0DEE:
case 0x0FEE:
case 0x01EF:
case 0x03EF:
case 0x05EF:
case 0x07EF:
case 0x09EF:
case 0x0BEF:
case 0x0DEF:
case 0x0FEF:

// BSET
case 0x01E8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03F0:
case 0x05F0:
case 0x07F0:
case 0x09F0:
case 0x0BF0:
case 0x0DF0:
case 0x0FF0:
case 0x01F1:
case 0x03F1:
case 0x05F1:
case 0x07F1:
case 0x09F1:
case 0x0BF1:
case 0x0DF1:
case 0x0FF1:
case 0x01F2:
case 0x03F2:
case 0x05F2:
case 0x07F2:
case 0x09F2:
case 0x0BF2:
case 0x0DF2:
case 0x0FF2:
case 0x01F3:
case 0x03F3:
case 0x05F3:
case 0x07F3:
case 0x09F3:
case 0x0BF3:
case 0x0DF3:
case 0x0FF3:
case 0x01F4:
case 0x03F4:
case 0x05F4:
case 0x07F4:
case 0x09F4:
case 0x0BF4:
case 0x0DF4:
case 0x0FF4:
case 0x01F5:
case 0x03F5:
case 0x05F5:
case 0x07F5:
case 0x09F5:
case 0x0BF5:
case 0x0DF5:
case 0x0FF5:
case 0x01F6:
case 0x03F6:
case 0x05F6:
case 0x07F6:
case 0x09F6:
case 0x0BF6:
case 0x0DF6:
case 0x0FF6:
case 0x01F7:
case 0x03F7:
case 0x05F7:
case 0x07F7:
case 0x09F7:
case 0x0BF7:
case 0x0DF7:
case 0x0FF7:

// BSET
case 0x01F0:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	timestamp += 6;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03F8:
case 0x05F8:
case 0x07F8:
case 0x09F8:
case 0x0BF8:
case 0x0DF8:
case 0x0FF8:

// BSET
case 0x01F8:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03F9:
case 0x05F9:
case 0x07F9:
case 0x09F9:
case 0x0BF9:
case 0x0DF9:
case 0x0FF9:

// BSET
case 0x01F9:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = (s32)FETCH_LONG;
	PC += 4;
	timestamp += 8;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03DF:
case 0x05DF:
case 0x07DF:
case 0x09DF:
case 0x0BDF:
case 0x0DDF:
case 0x0FDF:

// BSET
case 0x01DF:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[7];
	CPU->A[7] += 2;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x03E7:
case 0x05E7:
case 0x07E7:
case 0x09E7:
case 0x0BE7:
case 0x0DE7:
case 0x0FE7:

// BSET
case 0x01E7:
{
	u32 adr;
	u32 res;
	u32 src;
	timestamp += 4;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	src = 1 << (src & 7);
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	timestamp += 2;
	READ_BYat_F(adr, res)
	CPU->flag_notZ = res & src;
	res |= src;
	WRITE_BYTE_F(adr, res)
}
RET(4)
case 0x0308:
case 0x0508:
case 0x0708:
case 0x0908:
case 0x0B08:
case 0x0D08:
case 0x0F08:
case 0x0109:
case 0x0309:
case 0x0509:
case 0x0709:
case 0x0909:
case 0x0B09:
case 0x0D09:
case 0x0F09:
case 0x010A:
case 0x030A:
case 0x050A:
case 0x070A:
case 0x090A:
case 0x0B0A:
case 0x0D0A:
case 0x0F0A:
case 0x010B:
case 0x030B:
case 0x050B:
case 0x070B:
case 0x090B:
case 0x0B0B:
case 0x0D0B:
case 0x0F0B:
case 0x010C:
case 0x030C:
case 0x050C:
case 0x070C:
case 0x090C:
case 0x0B0C:
case 0x0D0C:
case 0x0F0C:
case 0x010D:
case 0x030D:
case 0x050D:
case 0x070D:
case 0x090D:
case 0x0B0D:
case 0x0D0D:
case 0x0F0D:
case 0x010E:
case 0x030E:
case 0x050E:
case 0x070E:
case 0x090E:
case 0x0B0E:
case 0x0D0E:
case 0x0F0E:
case 0x010F:
case 0x030F:
case 0x050F:
case 0x070F:
case 0x090F:
case 0x0B0F:
case 0x0D0F:
case 0x0F0F:

// MOVEPWaD
case 0x0108:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_BYat_F(adr + 0, res)
	READ_BYTE_F(adr + 2, src)
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = (res << 8) | src;
}
RET(16)
case 0x0348:
case 0x0548:
case 0x0748:
case 0x0948:
case 0x0B48:
case 0x0D48:
case 0x0F48:
case 0x0149:
case 0x0349:
case 0x0549:
case 0x0749:
case 0x0949:
case 0x0B49:
case 0x0D49:
case 0x0F49:
case 0x014A:
case 0x034A:
case 0x054A:
case 0x074A:
case 0x094A:
case 0x0B4A:
case 0x0D4A:
case 0x0F4A:
case 0x014B:
case 0x034B:
case 0x054B:
case 0x074B:
case 0x094B:
case 0x0B4B:
case 0x0D4B:
case 0x0F4B:
case 0x014C:
case 0x034C:
case 0x054C:
case 0x074C:
case 0x094C:
case 0x0B4C:
case 0x0D4C:
case 0x0F4C:
case 0x014D:
case 0x034D:
case 0x054D:
case 0x074D:
case 0x094D:
case 0x0B4D:
case 0x0D4D:
case 0x0F4D:
case 0x014E:
case 0x034E:
case 0x054E:
case 0x074E:
case 0x094E:
case 0x0B4E:
case 0x0D4E:
case 0x0F4E:
case 0x014F:
case 0x034F:
case 0x054F:
case 0x074F:
case 0x094F:
case 0x0B4F:
case 0x0D4F:
case 0x0F4F:

// MOVEPLaD
case 0x0148:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	READ_BYat_F(adr, res)
	res <<= 24;
	adr += 2;
	READ_BYTE_F(adr, src)
	res |= src << 16;
	adr += 2;
	READ_BYTE_F(adr, src)
	res |= src << 8;
	adr += 2;
	READ_BYTE_F(adr, src)
	CPU->D[(Opcode >> 9) & 7] = res | src;
}
RET(24)
case 0x0388:
case 0x0588:
case 0x0788:
case 0x0988:
case 0x0B88:
case 0x0D88:
case 0x0F88:
case 0x0189:
case 0x0389:
case 0x0589:
case 0x0789:
case 0x0989:
case 0x0B89:
case 0x0D89:
case 0x0F89:
case 0x018A:
case 0x038A:
case 0x058A:
case 0x078A:
case 0x098A:
case 0x0B8A:
case 0x0D8A:
case 0x0F8A:
case 0x018B:
case 0x038B:
case 0x058B:
case 0x078B:
case 0x098B:
case 0x0B8B:
case 0x0D8B:
case 0x0F8B:
case 0x018C:
case 0x038C:
case 0x058C:
case 0x078C:
case 0x098C:
case 0x0B8C:
case 0x0D8C:
case 0x0F8C:
case 0x018D:
case 0x038D:
case 0x058D:
case 0x078D:
case 0x098D:
case 0x0B8D:
case 0x0D8D:
case 0x0F8D:
case 0x018E:
case 0x038E:
case 0x058E:
case 0x078E:
case 0x098E:
case 0x0B8E:
case 0x0D8E:
case 0x0F8E:
case 0x018F:
case 0x038F:
case 0x058F:
case 0x078F:
case 0x098F:
case 0x0B8F:
case 0x0D8F:
case 0x0F8F:

// MOVEPWDa
case 0x0188:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	WRITE_BYat_F(adr + 0, res >> 8)
	WRITE_BYTE_F(adr + 2, res >> 0)
}
RET(16)
case 0x03C8:
case 0x05C8:
case 0x07C8:
case 0x09C8:
case 0x0BC8:
case 0x0DC8:
case 0x0FC8:
case 0x01C9:
case 0x03C9:
case 0x05C9:
case 0x07C9:
case 0x09C9:
case 0x0BC9:
case 0x0DC9:
case 0x0FC9:
case 0x01CA:
case 0x03CA:
case 0x05CA:
case 0x07CA:
case 0x09CA:
case 0x0BCA:
case 0x0DCA:
case 0x0FCA:
case 0x01CB:
case 0x03CB:
case 0x05CB:
case 0x07CB:
case 0x09CB:
case 0x0BCB:
case 0x0DCB:
case 0x0FCB:
case 0x01CC:
case 0x03CC:
case 0x05CC:
case 0x07CC:
case 0x09CC:
case 0x0BCC:
case 0x0DCC:
case 0x0FCC:
case 0x01CD:
case 0x03CD:
case 0x05CD:
case 0x07CD:
case 0x09CD:
case 0x0BCD:
case 0x0DCD:
case 0x0FCD:
case 0x01CE:
case 0x03CE:
case 0x05CE:
case 0x07CE:
case 0x09CE:
case 0x0BCE:
case 0x0DCE:
case 0x0FCE:
case 0x01CF:
case 0x03CF:
case 0x05CF:
case 0x07CF:
case 0x09CF:
case 0x0BCF:
case 0x0DCF:
case 0x0FCF:

// MOVEPLDa
case 0x01C8:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	timestamp += 4;
	WRITE_BYat_F(adr, res >> 24)
	adr += 2;
	WRITE_BYTE_F(adr, res >> 16)
	adr += 2;
	WRITE_BYTE_F(adr, res >> 8)
	adr += 2;
	WRITE_BYTE_F(adr, res >> 0)
}
RET(24)
