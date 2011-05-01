case 0x4001:
case 0x4002:
case 0x4003:
case 0x4004:
case 0x4005:
case 0x4006:
case 0x4007:

// NEGX
case 0x4000:
{
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ |= res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x4011:
case 0x4012:
case 0x4013:
case 0x4014:
case 0x4015:
case 0x4016:
case 0x4017:

// NEGX
case 0x4010:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x4019:
case 0x401A:
case 0x401B:
case 0x401C:
case 0x401D:
case 0x401E:

// NEGX
case 0x4018:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x4021:
case 0x4022:
case 0x4023:
case 0x4024:
case 0x4025:
case 0x4026:

// NEGX
case 0x4020:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x4029:
case 0x402A:
case 0x402B:
case 0x402C:
case 0x402D:
case 0x402E:
case 0x402F:

// NEGX
case 0x4028:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x4031:
case 0x4032:
case 0x4033:
case 0x4034:
case 0x4035:
case 0x4036:
case 0x4037:

// NEGX
case 0x4030:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)

// NEGX
case 0x4038:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)

// NEGX
case 0x4039:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)

// NEGX
case 0x401F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)

// NEGX
case 0x4027:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x4041:
case 0x4042:
case 0x4043:
case 0x4044:
case 0x4045:
case 0x4046:
case 0x4047:

// NEGX
case 0x4040:
{
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x4051:
case 0x4052:
case 0x4053:
case 0x4054:
case 0x4055:
case 0x4056:
case 0x4057:

// NEGX
case 0x4050:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x4059:
case 0x405A:
case 0x405B:
case 0x405C:
case 0x405D:
case 0x405E:

// NEGX
case 0x4058:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x4061:
case 0x4062:
case 0x4063:
case 0x4064:
case 0x4065:
case 0x4066:

// NEGX
case 0x4060:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x4069:
case 0x406A:
case 0x406B:
case 0x406C:
case 0x406D:
case 0x406E:
case 0x406F:

// NEGX
case 0x4068:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x4071:
case 0x4072:
case 0x4073:
case 0x4074:
case 0x4075:
case 0x4076:
case 0x4077:

// NEGX
case 0x4070:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// NEGX
case 0x4078:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// NEGX
case 0x4079:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// NEGX
case 0x405F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// NEGX
case 0x4067:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x4081:
case 0x4082:
case 0x4083:
case 0x4084:
case 0x4085:
case 0x4086:
case 0x4087:

// NEGX
case 0x4080:
{
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0x4091:
case 0x4092:
case 0x4093:
case 0x4094:
case 0x4095:
case 0x4096:
case 0x4097:

// NEGX
case 0x4090:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x4099:
case 0x409A:
case 0x409B:
case 0x409C:
case 0x409D:
case 0x409E:

// NEGX
case 0x4098:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x40A1:
case 0x40A2:
case 0x40A3:
case 0x40A4:
case 0x40A5:
case 0x40A6:

// NEGX
case 0x40A0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x40A9:
case 0x40AA:
case 0x40AB:
case 0x40AC:
case 0x40AD:
case 0x40AE:
case 0x40AF:

// NEGX
case 0x40A8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x40B1:
case 0x40B2:
case 0x40B3:
case 0x40B4:
case 0x40B5:
case 0x40B6:
case 0x40B7:

// NEGX
case 0x40B0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)

// NEGX
case 0x40B8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)

// NEGX
case 0x40B9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)

// NEGX
case 0x409F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)

// NEGX
case 0x40A7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x4201:
case 0x4202:
case 0x4203:
case 0x4204:
case 0x4205:
case 0x4206:
case 0x4207:

// CLR
case 0x4200:
{
	u32 res;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x4211:
case 0x4212:
case 0x4213:
case 0x4214:
case 0x4215:
case 0x4216:
case 0x4217:

// CLR
case 0x4210:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x4219:
case 0x421A:
case 0x421B:
case 0x421C:
case 0x421D:
case 0x421E:

// CLR
case 0x4218:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x4221:
case 0x4222:
case 0x4223:
case 0x4224:
case 0x4225:
case 0x4226:

// CLR
case 0x4220:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x4229:
case 0x422A:
case 0x422B:
case 0x422C:
case 0x422D:
case 0x422E:
case 0x422F:

// CLR
case 0x4228:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x4231:
case 0x4232:
case 0x4233:
case 0x4234:
case 0x4235:
case 0x4236:
case 0x4237:

// CLR
case 0x4230:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)

// CLR
case 0x4238:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)

// CLR
case 0x4239:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)

// CLR
case 0x421F:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)

// CLR
case 0x4227:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x4241:
case 0x4242:
case 0x4243:
case 0x4244:
case 0x4245:
case 0x4246:
case 0x4247:

// CLR
case 0x4240:
{
	u32 res;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x4251:
case 0x4252:
case 0x4253:
case 0x4254:
case 0x4255:
case 0x4256:
case 0x4257:

// CLR
case 0x4250:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x4259:
case 0x425A:
case 0x425B:
case 0x425C:
case 0x425D:
case 0x425E:

// CLR
case 0x4258:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x4261:
case 0x4262:
case 0x4263:
case 0x4264:
case 0x4265:
case 0x4266:

// CLR
case 0x4260:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x4269:
case 0x426A:
case 0x426B:
case 0x426C:
case 0x426D:
case 0x426E:
case 0x426F:

// CLR
case 0x4268:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x4271:
case 0x4272:
case 0x4273:
case 0x4274:
case 0x4275:
case 0x4276:
case 0x4277:

// CLR
case 0x4270:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// CLR
case 0x4278:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// CLR
case 0x4279:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// CLR
case 0x425F:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// CLR
case 0x4267:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x4281:
case 0x4282:
case 0x4283:
case 0x4284:
case 0x4285:
case 0x4286:
case 0x4287:

// CLR
case 0x4280:
{
	u32 res;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0x4291:
case 0x4292:
case 0x4293:
case 0x4294:
case 0x4295:
case 0x4296:
case 0x4297:

// CLR
case 0x4290:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x4299:
case 0x429A:
case 0x429B:
case 0x429C:
case 0x429D:
case 0x429E:

// CLR
case 0x4298:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x42A1:
case 0x42A2:
case 0x42A3:
case 0x42A4:
case 0x42A5:
case 0x42A6:

// CLR
case 0x42A0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x42A9:
case 0x42AA:
case 0x42AB:
case 0x42AC:
case 0x42AD:
case 0x42AE:
case 0x42AF:

// CLR
case 0x42A8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x42B1:
case 0x42B2:
case 0x42B3:
case 0x42B4:
case 0x42B5:
case 0x42B6:
case 0x42B7:

// CLR
case 0x42B0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)

// CLR
case 0x42B8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)

// CLR
case 0x42B9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)

// CLR
case 0x429F:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)

// CLR
case 0x42A7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	res = 0;
	CPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x4401:
case 0x4402:
case 0x4403:
case 0x4404:
case 0x4405:
case 0x4406:
case 0x4407:

// NEG
case 0x4400:
{
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	res = -src;
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x4411:
case 0x4412:
case 0x4413:
case 0x4414:
case 0x4415:
case 0x4416:
case 0x4417:

// NEG
case 0x4410:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src;
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x4419:
case 0x441A:
case 0x441B:
case 0x441C:
case 0x441D:
case 0x441E:

// NEG
case 0x4418:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src;
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x4421:
case 0x4422:
case 0x4423:
case 0x4424:
case 0x4425:
case 0x4426:

// NEG
case 0x4420:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src;
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x4429:
case 0x442A:
case 0x442B:
case 0x442C:
case 0x442D:
case 0x442E:
case 0x442F:

// NEG
case 0x4428:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src;
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x4431:
case 0x4432:
case 0x4433:
case 0x4434:
case 0x4435:
case 0x4436:
case 0x4437:

// NEG
case 0x4430:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src;
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)

// NEG
case 0x4438:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src;
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)

// NEG
case 0x4439:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src;
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)

// NEG
case 0x441F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src;
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)

// NEG
case 0x4427:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = -src;
	CPU->flag_V = res & src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x4441:
case 0x4442:
case 0x4443:
case 0x4444:
case 0x4445:
case 0x4446:
case 0x4447:

// NEG
case 0x4440:
{
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	res = -src;
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x4451:
case 0x4452:
case 0x4453:
case 0x4454:
case 0x4455:
case 0x4456:
case 0x4457:

// NEG
case 0x4450:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src;
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x4459:
case 0x445A:
case 0x445B:
case 0x445C:
case 0x445D:
case 0x445E:

// NEG
case 0x4458:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src;
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x4461:
case 0x4462:
case 0x4463:
case 0x4464:
case 0x4465:
case 0x4466:

// NEG
case 0x4460:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src;
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x4469:
case 0x446A:
case 0x446B:
case 0x446C:
case 0x446D:
case 0x446E:
case 0x446F:

// NEG
case 0x4468:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src;
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x4471:
case 0x4472:
case 0x4473:
case 0x4474:
case 0x4475:
case 0x4476:
case 0x4477:

// NEG
case 0x4470:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src;
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// NEG
case 0x4478:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src;
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// NEG
case 0x4479:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src;
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// NEG
case 0x445F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src;
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// NEG
case 0x4467:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	res = -src;
	CPU->flag_V = (res & src) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x4481:
case 0x4482:
case 0x4483:
case 0x4484:
case 0x4485:
case 0x4486:
case 0x4487:

// NEG
case 0x4480:
{
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	res = -src;
	CPU->flag_notZ = res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0x4491:
case 0x4492:
case 0x4493:
case 0x4494:
case 0x4495:
case 0x4496:
case 0x4497:

// NEG
case 0x4490:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src;
	CPU->flag_notZ = res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x4499:
case 0x449A:
case 0x449B:
case 0x449C:
case 0x449D:
case 0x449E:

// NEG
case 0x4498:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src;
	CPU->flag_notZ = res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x44A1:
case 0x44A2:
case 0x44A3:
case 0x44A4:
case 0x44A5:
case 0x44A6:

// NEG
case 0x44A0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src;
	CPU->flag_notZ = res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x44A9:
case 0x44AA:
case 0x44AB:
case 0x44AC:
case 0x44AD:
case 0x44AE:
case 0x44AF:

// NEG
case 0x44A8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src;
	CPU->flag_notZ = res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x44B1:
case 0x44B2:
case 0x44B3:
case 0x44B4:
case 0x44B5:
case 0x44B6:
case 0x44B7:

// NEG
case 0x44B0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src;
	CPU->flag_notZ = res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)

// NEG
case 0x44B8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src;
	CPU->flag_notZ = res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)

// NEG
case 0x44B9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src;
	CPU->flag_notZ = res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)

// NEG
case 0x449F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src;
	CPU->flag_notZ = res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)

// NEG
case 0x44A7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	res = -src;
	CPU->flag_notZ = res;
	CPU->flag_V = (res & src) >> 24;
	CPU->flag_X = CPU->flag_C = (src | res) >> 23;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x4601:
case 0x4602:
case 0x4603:
case 0x4604:
case 0x4605:
case 0x4606:
case 0x4607:

// NOT
case 0x4600:
{
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_N = res;
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x4611:
case 0x4612:
case 0x4613:
case 0x4614:
case 0x4615:
case 0x4616:
case 0x4617:

// NOT
case 0x4610:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_N = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x4619:
case 0x461A:
case 0x461B:
case 0x461C:
case 0x461D:
case 0x461E:

// NOT
case 0x4618:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_N = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x4621:
case 0x4622:
case 0x4623:
case 0x4624:
case 0x4625:
case 0x4626:

// NOT
case 0x4620:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_N = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x4629:
case 0x462A:
case 0x462B:
case 0x462C:
case 0x462D:
case 0x462E:
case 0x462F:

// NOT
case 0x4628:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_N = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x4631:
case 0x4632:
case 0x4633:
case 0x4634:
case 0x4635:
case 0x4636:
case 0x4637:

// NOT
case 0x4630:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_N = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)

// NOT
case 0x4638:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_N = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)

// NOT
case 0x4639:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_N = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)

// NOT
case 0x461F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_N = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)

// NOT
case 0x4627:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_N = res;
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x4641:
case 0x4642:
case 0x4643:
case 0x4644:
case 0x4645:
case 0x4646:
case 0x4647:

// NOT
case 0x4640:
{
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res & 0xFFFF;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x4651:
case 0x4652:
case 0x4653:
case 0x4654:
case 0x4655:
case 0x4656:
case 0x4657:

// NOT
case 0x4650:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res & 0xFFFF;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x4659:
case 0x465A:
case 0x465B:
case 0x465C:
case 0x465D:
case 0x465E:

// NOT
case 0x4658:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res & 0xFFFF;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x4661:
case 0x4662:
case 0x4663:
case 0x4664:
case 0x4665:
case 0x4666:

// NOT
case 0x4660:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res & 0xFFFF;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x4669:
case 0x466A:
case 0x466B:
case 0x466C:
case 0x466D:
case 0x466E:
case 0x466F:

// NOT
case 0x4668:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res & 0xFFFF;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x4671:
case 0x4672:
case 0x4673:
case 0x4674:
case 0x4675:
case 0x4676:
case 0x4677:

// NOT
case 0x4670:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res & 0xFFFF;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// NOT
case 0x4678:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res & 0xFFFF;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// NOT
case 0x4679:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res & 0xFFFF;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// NOT
case 0x465F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res & 0xFFFF;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// NOT
case 0x4667:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res & 0xFFFF;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x4681:
case 0x4682:
case 0x4683:
case 0x4684:
case 0x4685:
case 0x4686:
case 0x4687:

// NOT
case 0x4680:
{
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0x4691:
case 0x4692:
case 0x4693:
case 0x4694:
case 0x4695:
case 0x4696:
case 0x4697:

// NOT
case 0x4690:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_LONG_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x4699:
case 0x469A:
case 0x469B:
case 0x469C:
case 0x469D:
case 0x469E:

// NOT
case 0x4698:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x46A1:
case 0x46A2:
case 0x46A3:
case 0x46A4:
case 0x46A5:
case 0x46A6:

// NOT
case 0x46A0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x46A9:
case 0x46AA:
case 0x46AB:
case 0x46AC:
case 0x46AD:
case 0x46AE:
case 0x46AF:

// NOT
case 0x46A8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x46B1:
case 0x46B2:
case 0x46B3:
case 0x46B4:
case 0x46B5:
case 0x46B6:
case 0x46B7:

// NOT
case 0x46B0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)

// NOT
case 0x46B8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)

// NOT
case 0x46B9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)

// NOT
case 0x469F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)

// NOT
case 0x46A7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	res = ~src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x40C1:
case 0x40C2:
case 0x40C3:
case 0x40C4:
case 0x40C5:
case 0x40C6:
case 0x40C7:

// MOVESRa
case 0x40C0:
{
	u32 res;
	res = GET_SR;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0x40D1:
case 0x40D2:
case 0x40D3:
case 0x40D4:
case 0x40D5:
case 0x40D6:
case 0x40D7:

// MOVESRa
case 0x40D0:
{
	u32 adr;
	u32 res;
	res = GET_SR;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x40D9:
case 0x40DA:
case 0x40DB:
case 0x40DC:
case 0x40DD:
case 0x40DE:

// MOVESRa
case 0x40D8:
{
	u32 adr;
	u32 res;
	res = GET_SR;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x40E1:
case 0x40E2:
case 0x40E3:
case 0x40E4:
case 0x40E5:
case 0x40E6:

// MOVESRa
case 0x40E0:
{
	u32 adr;
	u32 res;
	res = GET_SR;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x40E9:
case 0x40EA:
case 0x40EB:
case 0x40EC:
case 0x40ED:
case 0x40EE:
case 0x40EF:

// MOVESRa
case 0x40E8:
{
	u32 adr;
	u32 res;
	res = GET_SR;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x40F1:
case 0x40F2:
case 0x40F3:
case 0x40F4:
case 0x40F5:
case 0x40F6:
case 0x40F7:

// MOVESRa
case 0x40F0:
{
	u32 adr;
	u32 res;
	res = GET_SR;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// MOVESRa
case 0x40F8:
{
	u32 adr;
	u32 res;
	res = GET_SR;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// MOVESRa
case 0x40F9:
{
	u32 adr;
	u32 res;
	res = GET_SR;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// MOVESRa
case 0x40DF:
{
	u32 adr;
	u32 res;
	res = GET_SR;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// MOVESRa
case 0x40E7:
{
	u32 adr;
	u32 res;
	res = GET_SR;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x44C1:
case 0x44C2:
case 0x44C3:
case 0x44C4:
case 0x44C5:
case 0x44C6:
case 0x44C7:

// MOVEaCCR
case 0x44C0:
{
	u32 res;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	SET_CCR(res)
}
RET(12)
case 0x44D1:
case 0x44D2:
case 0x44D3:
case 0x44D4:
case 0x44D5:
case 0x44D6:
case 0x44D7:

// MOVEaCCR
case 0x44D0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, res)
	SET_CCR(res)
	POST_IO
}
RET(16)
case 0x44D9:
case 0x44DA:
case 0x44DB:
case 0x44DC:
case 0x44DD:
case 0x44DE:

// MOVEaCCR
case 0x44D8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	SET_CCR(res)
	POST_IO
}
RET(16)
case 0x44E1:
case 0x44E2:
case 0x44E3:
case 0x44E4:
case 0x44E5:
case 0x44E6:

// MOVEaCCR
case 0x44E0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, res)
	SET_CCR(res)
	POST_IO
}
RET(18)
case 0x44E9:
case 0x44EA:
case 0x44EB:
case 0x44EC:
case 0x44ED:
case 0x44EE:
case 0x44EF:

// MOVEaCCR
case 0x44E8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	SET_CCR(res)
	POST_IO
}
RET(20)
case 0x44F1:
case 0x44F2:
case 0x44F3:
case 0x44F4:
case 0x44F5:
case 0x44F6:
case 0x44F7:

// MOVEaCCR
case 0x44F0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	SET_CCR(res)
	POST_IO
}
RET(22)

// MOVEaCCR
case 0x44F8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	SET_CCR(res)
	POST_IO
}
RET(20)

// MOVEaCCR
case 0x44F9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, res)
	SET_CCR(res)
	POST_IO
}
RET(24)

// MOVEaCCR
case 0x44FA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	SET_CCR(res)
	POST_IO
}
RET(20)

// MOVEaCCR
case 0x44FB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	SET_CCR(res)
	POST_IO
}
RET(22)

// MOVEaCCR
case 0x44FC:
{
	u32 res;
	res = FETCH_WORD;
	PC += 2;
	SET_CCR(res)
}
RET(16)

// MOVEaCCR
case 0x44DF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	SET_CCR(res)
	POST_IO
}
RET(16)

// MOVEaCCR
case 0x44E7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, res)
	SET_CCR(res)
	POST_IO
}
RET(18)
case 0x46C1:
case 0x46C2:
case 0x46C3:
case 0x46C4:
case 0x46C5:
case 0x46C6:
case 0x46C7:

// MOVEaSR
case 0x46C0:
{
	u32 res;
	if (CPU->flag_S)
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
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
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
POST_IO
CCnt -= 12;
goto C68k_Exec_End;
case 0x46D1:
case 0x46D2:
case 0x46D3:
case 0x46D4:
case 0x46D5:
case 0x46D6:
case 0x46D7:

// MOVEaSR
case 0x46D0:
{
	u32 adr;
	u32 res;
	if (CPU->flag_S)
	{
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, res)
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
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
POST_IO
CCnt -= 16;
goto C68k_Exec_End;
case 0x46D9:
case 0x46DA:
case 0x46DB:
case 0x46DC:
case 0x46DD:
case 0x46DE:

// MOVEaSR
case 0x46D8:
{
	u32 adr;
	u32 res;
	if (CPU->flag_S)
	{
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, res)
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
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
POST_IO
CCnt -= 16;
goto C68k_Exec_End;
case 0x46E1:
case 0x46E2:
case 0x46E3:
case 0x46E4:
case 0x46E5:
case 0x46E6:

// MOVEaSR
case 0x46E0:
{
	u32 adr;
	u32 res;
	if (CPU->flag_S)
	{
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, res)
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
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
POST_IO
CCnt -= 18;
goto C68k_Exec_End;
case 0x46E9:
case 0x46EA:
case 0x46EB:
case 0x46EC:
case 0x46ED:
case 0x46EE:
case 0x46EF:

// MOVEaSR
case 0x46E8:
{
	u32 adr;
	u32 res;
	if (CPU->flag_S)
	{
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
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
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
POST_IO
CCnt -= 20;
goto C68k_Exec_End;
case 0x46F1:
case 0x46F2:
case 0x46F3:
case 0x46F4:
case 0x46F5:
case 0x46F6:
case 0x46F7:

// MOVEaSR
case 0x46F0:
{
	u32 adr;
	u32 res;
	if (CPU->flag_S)
	{
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
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
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
POST_IO
CCnt -= 22;
goto C68k_Exec_End;

// MOVEaSR
case 0x46F8:
{
	u32 adr;
	u32 res;
	if (CPU->flag_S)
	{
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
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
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
POST_IO
CCnt -= 20;
goto C68k_Exec_End;

// MOVEaSR
case 0x46F9:
{
	u32 adr;
	u32 res;
	if (CPU->flag_S)
	{
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, res)
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
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
POST_IO
CCnt -= 24;
goto C68k_Exec_End;

// MOVEaSR
case 0x46FA:
{
	u32 adr;
	u32 res;
	if (CPU->flag_S)
	{
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
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
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
POST_IO
CCnt -= 20;
goto C68k_Exec_End;

// MOVEaSR
case 0x46FB:
{
	u32 adr;
	u32 res;
	if (CPU->flag_S)
	{
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
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
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
POST_IO
CCnt -= 22;
goto C68k_Exec_End;

// MOVEaSR
case 0x46FC:
{
	u32 res;
	if (CPU->flag_S)
	{
	res = FETCH_WORD;
	PC += 2;
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
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
POST_IO
CCnt -= 16;
goto C68k_Exec_End;

// MOVEaSR
case 0x46DF:
{
	u32 adr;
	u32 res;
	if (CPU->flag_S)
	{
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, res)
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
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
POST_IO
CCnt -= 16;
goto C68k_Exec_End;

// MOVEaSR
case 0x46E7:
{
	u32 adr;
	u32 res;
	if (CPU->flag_S)
	{
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, res)
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
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
}
POST_IO
CCnt -= 18;
goto C68k_Exec_End;
case 0x4801:
case 0x4802:
case 0x4803:
case 0x4804:
case 0x4805:
case 0x4806:
case 0x4807:

// NBCD
case 0x4800:
{
	u32 res;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	res = 0x9a - res - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);

	if (res != 0x9a)
	{
		if ((res & 0x0f) == 0xa) res = (res & 0xf0) + 0x10;
		res &= 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
		CPU->flag_notZ |= res;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_N = res;
}
RET(6)
case 0x4811:
case 0x4812:
case 0x4813:
case 0x4814:
case 0x4815:
case 0x4816:
case 0x4817:

// NBCD
case 0x4810:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, res)
	res = 0x9a - res - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);

	if (res != 0x9a)
	{
		if ((res & 0x0f) == 0xa) res = (res & 0xf0) + 0x10;
		res &= 0xFF;
	WRITE_BYTE_F(adr, res)
		CPU->flag_notZ |= res;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_N = res;
	POST_IO
}
RET(12)
case 0x4819:
case 0x481A:
case 0x481B:
case 0x481C:
case 0x481D:
case 0x481E:

// NBCD
case 0x4818:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, res)
	res = 0x9a - res - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);

	if (res != 0x9a)
	{
		if ((res & 0x0f) == 0xa) res = (res & 0xf0) + 0x10;
		res &= 0xFF;
	WRITE_BYTE_F(adr, res)
		CPU->flag_notZ |= res;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_N = res;
	POST_IO
}
RET(12)
case 0x4821:
case 0x4822:
case 0x4823:
case 0x4824:
case 0x4825:
case 0x4826:

// NBCD
case 0x4820:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, res)
	res = 0x9a - res - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);

	if (res != 0x9a)
	{
		if ((res & 0x0f) == 0xa) res = (res & 0xf0) + 0x10;
		res &= 0xFF;
	WRITE_BYTE_F(adr, res)
		CPU->flag_notZ |= res;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_N = res;
	POST_IO
}
RET(14)
case 0x4829:
case 0x482A:
case 0x482B:
case 0x482C:
case 0x482D:
case 0x482E:
case 0x482F:

// NBCD
case 0x4828:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	res = 0x9a - res - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);

	if (res != 0x9a)
	{
		if ((res & 0x0f) == 0xa) res = (res & 0xf0) + 0x10;
		res &= 0xFF;
	WRITE_BYTE_F(adr, res)
		CPU->flag_notZ |= res;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_N = res;
	POST_IO
}
RET(16)
case 0x4831:
case 0x4832:
case 0x4833:
case 0x4834:
case 0x4835:
case 0x4836:
case 0x4837:

// NBCD
case 0x4830:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	res = 0x9a - res - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);

	if (res != 0x9a)
	{
		if ((res & 0x0f) == 0xa) res = (res & 0xf0) + 0x10;
		res &= 0xFF;
	WRITE_BYTE_F(adr, res)
		CPU->flag_notZ |= res;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_N = res;
	POST_IO
}
RET(18)

// NBCD
case 0x4838:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	res = 0x9a - res - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);

	if (res != 0x9a)
	{
		if ((res & 0x0f) == 0xa) res = (res & 0xf0) + 0x10;
		res &= 0xFF;
	WRITE_BYTE_F(adr, res)
		CPU->flag_notZ |= res;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_N = res;
	POST_IO
}
RET(16)

// NBCD
case 0x4839:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_BYTE_F(adr, res)
	res = 0x9a - res - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);

	if (res != 0x9a)
	{
		if ((res & 0x0f) == 0xa) res = (res & 0xf0) + 0x10;
		res &= 0xFF;
	WRITE_BYTE_F(adr, res)
		CPU->flag_notZ |= res;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_N = res;
	POST_IO
}
RET(20)

// NBCD
case 0x481F:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	res = 0x9a - res - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);

	if (res != 0x9a)
	{
		if ((res & 0x0f) == 0xa) res = (res & 0xf0) + 0x10;
		res &= 0xFF;
	WRITE_BYTE_F(adr, res)
		CPU->flag_notZ |= res;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_N = res;
	POST_IO
}
RET(12)

// NBCD
case 0x4827:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, res)
	res = 0x9a - res - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);

	if (res != 0x9a)
	{
		if ((res & 0x0f) == 0xa) res = (res & 0xf0) + 0x10;
		res &= 0xFF;
	WRITE_BYTE_F(adr, res)
		CPU->flag_notZ |= res;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_N = res;
	POST_IO
}
RET(14)
case 0x4851:
case 0x4852:
case 0x4853:
case 0x4854:
case 0x4855:
case 0x4856:
case 0x4857:

// PEA
case 0x4850:
{
	u32 adr;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	PUSH_32_F(adr)
	POST_IO
}
RET(12)
case 0x4869:
case 0x486A:
case 0x486B:
case 0x486C:
case 0x486D:
case 0x486E:
case 0x486F:

// PEA
case 0x4868:
{
	u32 adr;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	PUSH_32_F(adr)
	POST_IO
}
RET(16)
case 0x4871:
case 0x4872:
case 0x4873:
case 0x4874:
case 0x4875:
case 0x4876:
case 0x4877:

// PEA
case 0x4870:
{
	u32 adr;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	PUSH_32_F(adr)
	POST_IO
}
RET(20)

// PEA
case 0x4878:
{
	u32 adr;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	PUSH_32_F(adr)
	POST_IO
}
RET(16)

// PEA
case 0x4879:
{
	u32 adr;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	PUSH_32_F(adr)
	POST_IO
}
RET(20)

// PEA
case 0x487A:
{
	u32 adr;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	PUSH_32_F(adr)
	POST_IO
}
RET(16)

// PEA
case 0x487B:
{
	u32 adr;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	PUSH_32_F(adr)
	POST_IO
}
RET(20)
case 0x4841:
case 0x4842:
case 0x4843:
case 0x4844:
case 0x4845:
case 0x4846:
case 0x4847:

// SWAP
case 0x4840:
{
	u32 res;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	res = (res >> 16) | (res << 16);
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x4891:
case 0x4892:
case 0x4893:
case 0x4894:
case 0x4895:
case 0x4896:
case 0x4897:

// MOVEMRa
case 0x4890:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			WRITE_WORD_F(adr, CPU->DA[src])
			adr += 2;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(12)
case 0x48A1:
case 0x48A2:
case 0x48A3:
case 0x48A4:
case 0x48A5:
case 0x48A6:

// MOVEMRa
case 0x48A0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	src = 15;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			adr -= 2;
			WRITE_WORD_F(adr, CPU->DA[src])
		}
		src --;
	} while (res >>= 1);
	CPU->A[(Opcode >> 0) & 7] = adr;
	POST_IO
	CCnt -= (dst - adr) * 2;
}
RET(8)
case 0x48A9:
case 0x48AA:
case 0x48AB:
case 0x48AC:
case 0x48AD:
case 0x48AE:
case 0x48AF:

// MOVEMRa
case 0x48A8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			WRITE_WORD_F(adr, CPU->DA[src])
			adr += 2;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(20)
case 0x48B1:
case 0x48B2:
case 0x48B3:
case 0x48B4:
case 0x48B5:
case 0x48B6:
case 0x48B7:

// MOVEMRa
case 0x48B0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			WRITE_WORD_F(adr, CPU->DA[src])
			adr += 2;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(24)

// MOVEMRa
case 0x48B8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			WRITE_WORD_F(adr, CPU->DA[src])
			adr += 2;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(20)

// MOVEMRa
case 0x48B9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			WRITE_WORD_F(adr, CPU->DA[src])
			adr += 2;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(28)

// MOVEMRa
case 0x48A7:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7];
	src = 15;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			adr -= 2;
			WRITE_WORD_F(adr, CPU->DA[src])
		}
		src --;
	} while (res >>= 1);
	CPU->A[7] = adr;
	POST_IO
	CCnt -= (dst - adr) * 2;
}
RET(8)
case 0x48D1:
case 0x48D2:
case 0x48D3:
case 0x48D4:
case 0x48D5:
case 0x48D6:
case 0x48D7:

// MOVEMRa
case 0x48D0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			WRITE_LONG_F(adr, CPU->DA[src])
			adr += 4;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(16)
case 0x48E1:
case 0x48E2:
case 0x48E3:
case 0x48E4:
case 0x48E5:
case 0x48E6:

// MOVEMRa
case 0x48E0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	src = 15;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			adr -= 4;
			WRITE_LONG_DEC_F(adr, CPU->DA[src])
		}
		src --;
	} while (res >>= 1);
	CPU->A[(Opcode >> 0) & 7] = adr;
	POST_IO
	CCnt -= (dst - adr) * 2;
}
RET(8)
case 0x48E9:
case 0x48EA:
case 0x48EB:
case 0x48EC:
case 0x48ED:
case 0x48EE:
case 0x48EF:

// MOVEMRa
case 0x48E8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			WRITE_LONG_F(adr, CPU->DA[src])
			adr += 4;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(24)
case 0x48F1:
case 0x48F2:
case 0x48F3:
case 0x48F4:
case 0x48F5:
case 0x48F6:
case 0x48F7:

// MOVEMRa
case 0x48F0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			WRITE_LONG_F(adr, CPU->DA[src])
			adr += 4;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(28)

// MOVEMRa
case 0x48F8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			WRITE_LONG_F(adr, CPU->DA[src])
			adr += 4;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(24)

// MOVEMRa
case 0x48F9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			WRITE_LONG_F(adr, CPU->DA[src])
			adr += 4;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(32)

// MOVEMRa
case 0x48E7:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7];
	src = 15;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			adr -= 4;
			WRITE_LONG_DEC_F(adr, CPU->DA[src])
		}
		src --;
	} while (res >>= 1);
	CPU->A[7] = adr;
	POST_IO
	CCnt -= (dst - adr) * 2;
}
RET(8)
case 0x4881:
case 0x4882:
case 0x4883:
case 0x4884:
case 0x4885:
case 0x4886:
case 0x4887:

// EXT
case 0x4880:
{
	u32 res;
	res = (s32)(s8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x48C1:
case 0x48C2:
case 0x48C3:
case 0x48C4:
case 0x48C5:
case 0x48C6:
case 0x48C7:

// EXT
case 0x48C0:
{
	u32 res;
	res = (s32)(s16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x4A01:
case 0x4A02:
case 0x4A03:
case 0x4A04:
case 0x4A05:
case 0x4A06:
case 0x4A07:

// TST
case 0x4A00:
{
	u32 res;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
}
RET(4)
case 0x4A11:
case 0x4A12:
case 0x4A13:
case 0x4A14:
case 0x4A15:
case 0x4A16:
case 0x4A17:

// TST
case 0x4A10:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	POST_IO
}
RET(8)
case 0x4A19:
case 0x4A1A:
case 0x4A1B:
case 0x4A1C:
case 0x4A1D:
case 0x4A1E:

// TST
case 0x4A18:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	POST_IO
}
RET(8)
case 0x4A21:
case 0x4A22:
case 0x4A23:
case 0x4A24:
case 0x4A25:
case 0x4A26:

// TST
case 0x4A20:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	POST_IO
}
RET(10)
case 0x4A29:
case 0x4A2A:
case 0x4A2B:
case 0x4A2C:
case 0x4A2D:
case 0x4A2E:
case 0x4A2F:

// TST
case 0x4A28:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	POST_IO
}
RET(12)
case 0x4A31:
case 0x4A32:
case 0x4A33:
case 0x4A34:
case 0x4A35:
case 0x4A36:
case 0x4A37:

// TST
case 0x4A30:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	POST_IO
}
RET(14)

// TST
case 0x4A38:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	POST_IO
}
RET(12)

// TST
case 0x4A39:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	POST_IO
}
RET(16)

// TST
case 0x4A1F:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	POST_IO
}
RET(8)

// TST
case 0x4A27:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	POST_IO
}
RET(10)
case 0x4A41:
case 0x4A42:
case 0x4A43:
case 0x4A44:
case 0x4A45:
case 0x4A46:
case 0x4A47:

// TST
case 0x4A40:
{
	u32 res;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
}
RET(4)
case 0x4A51:
case 0x4A52:
case 0x4A53:
case 0x4A54:
case 0x4A55:
case 0x4A56:
case 0x4A57:

// TST
case 0x4A50:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	POST_IO
}
RET(8)
case 0x4A59:
case 0x4A5A:
case 0x4A5B:
case 0x4A5C:
case 0x4A5D:
case 0x4A5E:

// TST
case 0x4A58:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	POST_IO
}
RET(8)
case 0x4A61:
case 0x4A62:
case 0x4A63:
case 0x4A64:
case 0x4A65:
case 0x4A66:

// TST
case 0x4A60:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	POST_IO
}
RET(10)
case 0x4A69:
case 0x4A6A:
case 0x4A6B:
case 0x4A6C:
case 0x4A6D:
case 0x4A6E:
case 0x4A6F:

// TST
case 0x4A68:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	POST_IO
}
RET(12)
case 0x4A71:
case 0x4A72:
case 0x4A73:
case 0x4A74:
case 0x4A75:
case 0x4A76:
case 0x4A77:

// TST
case 0x4A70:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	POST_IO
}
RET(14)

// TST
case 0x4A78:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	POST_IO
}
RET(12)

// TST
case 0x4A79:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	POST_IO
}
RET(16)

// TST
case 0x4A5F:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	POST_IO
}
RET(8)

// TST
case 0x4A67:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	POST_IO
}
RET(10)
case 0x4A81:
case 0x4A82:
case 0x4A83:
case 0x4A84:
case 0x4A85:
case 0x4A86:
case 0x4A87:

// TST
case 0x4A80:
{
	u32 res;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
}
RET(4)
case 0x4A91:
case 0x4A92:
case 0x4A93:
case 0x4A94:
case 0x4A95:
case 0x4A96:
case 0x4A97:

// TST
case 0x4A90:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(12)
case 0x4A99:
case 0x4A9A:
case 0x4A9B:
case 0x4A9C:
case 0x4A9D:
case 0x4A9E:

// TST
case 0x4A98:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(12)
case 0x4AA1:
case 0x4AA2:
case 0x4AA3:
case 0x4AA4:
case 0x4AA5:
case 0x4AA6:

// TST
case 0x4AA0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(14)
case 0x4AA9:
case 0x4AAA:
case 0x4AAB:
case 0x4AAC:
case 0x4AAD:
case 0x4AAE:
case 0x4AAF:

// TST
case 0x4AA8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(16)
case 0x4AB1:
case 0x4AB2:
case 0x4AB3:
case 0x4AB4:
case 0x4AB5:
case 0x4AB6:
case 0x4AB7:

// TST
case 0x4AB0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(18)

// TST
case 0x4AB8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(16)

// TST
case 0x4AB9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(20)

// TST
case 0x4A9F:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(12)

// TST
case 0x4AA7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(14)
case 0x4AC1:
case 0x4AC2:
case 0x4AC3:
case 0x4AC4:
case 0x4AC5:
case 0x4AC6:
case 0x4AC7:

// TAS
case 0x4AC0:
{
	u32 res;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	res |= 0x80;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x4AD1:
case 0x4AD2:
case 0x4AD3:
case 0x4AD4:
case 0x4AD5:
case 0x4AD6:
case 0x4AD7:

// TAS
case 0x4AD0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
if(!CPU->TAS_Hack)
{
	res |= 0x80;
	WRITE_BYTE_F(adr, res)
}
	POST_IO
}
RET(8)
case 0x4AD9:
case 0x4ADA:
case 0x4ADB:
case 0x4ADC:
case 0x4ADD:
case 0x4ADE:

// TAS
case 0x4AD8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
if(!CPU->TAS_Hack)
{
	res |= 0x80;
	WRITE_BYTE_F(adr, res)
}
	POST_IO
}
RET(8)
case 0x4AE1:
case 0x4AE2:
case 0x4AE3:
case 0x4AE4:
case 0x4AE5:
case 0x4AE6:

// TAS
case 0x4AE0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
if(!CPU->TAS_Hack)
{
	res |= 0x80;
	WRITE_BYTE_F(adr, res)
}
	POST_IO
}
RET(10)
case 0x4AE9:
case 0x4AEA:
case 0x4AEB:
case 0x4AEC:
case 0x4AED:
case 0x4AEE:
case 0x4AEF:

// TAS
case 0x4AE8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
if(!CPU->TAS_Hack)
{
	res |= 0x80;
	WRITE_BYTE_F(adr, res)
}
	POST_IO
}
RET(12)
case 0x4AF1:
case 0x4AF2:
case 0x4AF3:
case 0x4AF4:
case 0x4AF5:
case 0x4AF6:
case 0x4AF7:

// TAS
case 0x4AF0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
if(!CPU->TAS_Hack)
{
	res |= 0x80;
	WRITE_BYTE_F(adr, res)
}
	POST_IO
}
RET(14)

// TAS
case 0x4AF8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
if(!CPU->TAS_Hack)
{
	res |= 0x80;
	WRITE_BYTE_F(adr, res)
}
	POST_IO
}
RET(12)

// TAS
case 0x4AF9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
if(!CPU->TAS_Hack)
{
	res |= 0x80;
	WRITE_BYTE_F(adr, res)
}
	POST_IO
}
RET(16)

// TAS
case 0x4ADF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
if(!CPU->TAS_Hack)
{
	res |= 0x80;
	WRITE_BYTE_F(adr, res)
}
	POST_IO
}
RET(8)

// TAS
case 0x4AE7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
if(!CPU->TAS_Hack)
{
	res |= 0x80;
	WRITE_BYTE_F(adr, res)
}
	POST_IO
}
RET(10)
default:

// ILLEGAL
case 0x4AFC:
{
	u32 res;
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ILLEGAL_INSTRUCTION_EX;
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
}
RET(4)
case 0x4C91:
case 0x4C92:
case 0x4C93:
case 0x4C94:
case 0x4C95:
case 0x4C96:
case 0x4C97:

// MOVEMaR
case 0x4C90:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READSX_WORD_F(adr, CPU->DA[src])
			adr += 2;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(16)
case 0x4C99:
case 0x4C9A:
case 0x4C9B:
case 0x4C9C:
case 0x4C9D:
case 0x4C9E:

// MOVEMaR
case 0x4C98:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READSX_WORD_F(adr, CPU->DA[src])
			adr += 2;
		}
		src ++;
	} while (res >>= 1);
	CPU->A[(Opcode >> 0) & 7] = adr;
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(12)
case 0x4CA9:
case 0x4CAA:
case 0x4CAB:
case 0x4CAC:
case 0x4CAD:
case 0x4CAE:
case 0x4CAF:

// MOVEMaR
case 0x4CA8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READSX_WORD_F(adr, CPU->DA[src])
			adr += 2;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(24)
case 0x4CB1:
case 0x4CB2:
case 0x4CB3:
case 0x4CB4:
case 0x4CB5:
case 0x4CB6:
case 0x4CB7:

// MOVEMaR
case 0x4CB0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READSX_WORD_F(adr, CPU->DA[src])
			adr += 2;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(28)

// MOVEMaR
case 0x4CB8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READSX_WORD_F(adr, CPU->DA[src])
			adr += 2;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(24)

// MOVEMaR
case 0x4CB9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READSX_WORD_F(adr, CPU->DA[src])
			adr += 2;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(32)

// MOVEMaR
case 0x4CBA:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READSX_WORD_F(adr, CPU->DA[src])
			adr += 2;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(24)

// MOVEMaR
case 0x4CBB:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = PC;
	DECODE_EXT_WORD
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READSX_WORD_F(adr, CPU->DA[src])
			adr += 2;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(28)

// MOVEMaR
case 0x4C9F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7];
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READSX_WORD_F(adr, CPU->DA[src])
			adr += 2;
		}
		src ++;
	} while (res >>= 1);
	CPU->A[7] = adr;
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(12)
case 0x4CD1:
case 0x4CD2:
case 0x4CD3:
case 0x4CD4:
case 0x4CD5:
case 0x4CD6:
case 0x4CD7:

// MOVEMaR
case 0x4CD0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READ_LONG_F(adr, CPU->DA[src])
			adr += 4;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(20)
case 0x4CD9:
case 0x4CDA:
case 0x4CDB:
case 0x4CDC:
case 0x4CDD:
case 0x4CDE:

// MOVEMaR
case 0x4CD8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READ_LONG_F(adr, CPU->DA[src])
			adr += 4;
		}
		src ++;
	} while (res >>= 1);
	CPU->A[(Opcode >> 0) & 7] = adr;
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(12)
case 0x4CE9:
case 0x4CEA:
case 0x4CEB:
case 0x4CEC:
case 0x4CED:
case 0x4CEE:
case 0x4CEF:

// MOVEMaR
case 0x4CE8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READ_LONG_F(adr, CPU->DA[src])
			adr += 4;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(28)
case 0x4CF1:
case 0x4CF2:
case 0x4CF3:
case 0x4CF4:
case 0x4CF5:
case 0x4CF6:
case 0x4CF7:

// MOVEMaR
case 0x4CF0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READ_LONG_F(adr, CPU->DA[src])
			adr += 4;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(32)

// MOVEMaR
case 0x4CF8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READ_LONG_F(adr, CPU->DA[src])
			adr += 4;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(28)

// MOVEMaR
case 0x4CF9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = (s32)FETCH_LONG;
	PC += 4;
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READ_LONG_F(adr, CPU->DA[src])
			adr += 4;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(36)

// MOVEMaR
case 0x4CFA:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READ_LONG_F(adr, CPU->DA[src])
			adr += 4;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(28)

// MOVEMaR
case 0x4CFB:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = PC;
	DECODE_EXT_WORD
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READ_LONG_F(adr, CPU->DA[src])
			adr += 4;
		}
		src ++;
	} while (res >>= 1);
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(32)

// MOVEMaR
case 0x4CDF:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	res = FETCH_WORD;
	PC += 2;
	adr = CPU->A[7];
	src = 0;
	dst = adr;
	PRE_IO
	do
	{
		if (res & 1)
		{
			READ_LONG_F(adr, CPU->DA[src])
			adr += 4;
		}
		src ++;
	} while (res >>= 1);
	CPU->A[7] = adr;
	POST_IO
	CCnt -= (adr - dst) * 2;
}
RET(12)
case 0x4E41:
case 0x4E42:
case 0x4E43:
case 0x4E44:
case 0x4E45:
case 0x4E46:
case 0x4E47:
case 0x4E48:
case 0x4E49:
case 0x4E4A:
case 0x4E4B:
case 0x4E4C:
case 0x4E4D:
case 0x4E4E:
case 0x4E4F:

// TRAP
case 0x4E40:
{
	u32 res;
	if (!CPU->flag_S)
	{
		res = CPU->USP;
		CPU->USP = CPU->A[7];
		CPU->A[7] = res;
	}
	res = C68K_TRAP_BASE_EX + (Opcode & 0xF);
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
	PUSH_32_F(PC)
	PUSH_16_F(GET_SR)
	CPU->flag_S = C68K_SR_S;
	READ_LONG_F(res * 4, PC)
	SET_PC(PC)
	POST_IO
}
RET(4)
case 0x4E51:
case 0x4E52:
case 0x4E53:
case 0x4E54:
case 0x4E55:
case 0x4E56:

// LINK
case 0x4E50:
{
	u32 res;
	res = (u32)CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	PUSH_32_F(res)
	res = CPU->A[7];
	CPU->A[(Opcode >> 0) & 7] = res;
	CPU->A[7] += (s32)(s16)FETCH_WORD;
	PC += 2;
	POST_IO
}
RET(16)

// LINKA7
case 0x4E57:
{
	CPU->A[7] -= 4;
	PRE_IO
	WRITE_LONG_DEC_F(CPU->A[7], CPU->A[7])
	CPU->A[7] += (s32)(s16)FETCH_WORD;
	PC += 2;
	POST_IO
}
RET(16)
case 0x4E59:
case 0x4E5A:
case 0x4E5B:
case 0x4E5C:
case 0x4E5D:
case 0x4E5E:

// ULNK
case 0x4E58:
{
	u32 res;
	u32 src;
	src = (u32)CPU->A[(Opcode >> 0) & 7];
	CPU->A[7] = src + 4;
	PRE_IO
	READ_LONG_F(src, res)
	CPU->A[(Opcode >> 0) & 7] = res;
	POST_IO
}
RET(12)

// ULNKA7
case 0x4E5F:
{
	PRE_IO
	READ_LONG_F(CPU->A[7], CPU->A[7])
	POST_IO
}
RET(12)
case 0x4E61:
case 0x4E62:
case 0x4E63:
case 0x4E64:
case 0x4E65:
case 0x4E66:
case 0x4E67:

// MOVEAUSP
case 0x4E60:
{
	u32 res;
	if (!CPU->flag_S)
	{
		res = CPU->USP;
		CPU->USP = CPU->A[7];
		CPU->A[7] = res;
		res = C68K_PRIVILEGE_VIOLATION_EX;
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(4)
	}
	res = (u32)CPU->A[(Opcode >> 0) & 7];
	CPU->USP = res;
}
RET(4)
case 0x4E69:
case 0x4E6A:
case 0x4E6B:
case 0x4E6C:
case 0x4E6D:
case 0x4E6E:
case 0x4E6F:

// MOVEUSPA
case 0x4E68:
{
	u32 res;
	if (!CPU->flag_S)
	{
		res = CPU->USP;
		CPU->USP = CPU->A[7];
		CPU->A[7] = res;
		res = C68K_PRIVILEGE_VIOLATION_EX;
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(4)
	}
	res = CPU->USP;
	CPU->A[(Opcode >> 0) & 7] = res;
}
RET(4)

// RESET
case 0x4E70:
{
	u32 res;
	if (!CPU->flag_S)
	{
		res = CPU->USP;
		CPU->USP = CPU->A[7];
		CPU->A[7] = res;
		res = C68K_PRIVILEGE_VIOLATION_EX;
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(4)
	}
	PRE_IO
	CPU->Reset_CallBack();
	POST_IO
}
RET(132)

// NOP
case 0x4E71:
{
}
RET(4)

// STOP
case 0x4E72:
{
	u32 res;
	if (!CPU->flag_S)
	{
		PC += 2;
		res = CPU->USP;
		CPU->USP = CPU->A[7];
		CPU->A[7] = res;
		res = C68K_PRIVILEGE_VIOLATION_EX;
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(4)
	}
	res = FETCH_WORD & C68K_SR_MASK;
	PC += 2;
	SET_SR(res)
	if (!CPU->flag_S)
	{
		res = CPU->A[7];
		CPU->A[7] = CPU->USP;
		CPU->USP = res;
	}
	CPU->Status |= C68K_HALTED;
	CCnt = 0;
}
CCnt -= 4;
goto C68k_Exec_End;

// RTE
case 0x4E73:
{
	u32 res;
	if (!CPU->flag_S)
	{
		res = CPU->USP;
		CPU->USP = CPU->A[7];
		CPU->A[7] = res;
		res = C68K_PRIVILEGE_VIOLATION_EX;
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(4)
	}
	PRE_IO
	POP_16_F(res)
	SET_SR(res)
	POP_32_F(res)
	SET_PC(res)
	if (!CPU->flag_S)
	{
		res = CPU->A[7];
		CPU->A[7] = CPU->USP;
		CPU->USP = res;
	}
}
POST_IO
CCnt -= 20;
goto C68k_Exec_End;

// RTS
case 0x4E75:
{
	u32 res;
	PRE_IO
	POP_32_F(res)
	SET_PC(res)
	POST_IO
}
RET(16)

// TRAPV
case 0x4E76:
{
	u32 res;
	if (CPU->flag_V & 0x80)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_TRAPV_EX;
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
	POST_IO
}
RET(4)

// RTR
case 0x4E77:
{
	u32 res;
	PRE_IO
	POP_16_F(res)
	SET_CCR(res)
	POP_32_F(res)
	SET_PC(res)
	POST_IO
}
RET(20)
case 0x4E91:
case 0x4E92:
case 0x4E93:
case 0x4E94:
case 0x4E95:
case 0x4E96:
case 0x4E97:

// JSR
case 0x4E90:
{
	u32 adr;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	PUSH_32_F(PC)
	SET_PC(adr)
	POST_IO
}
RET(16)
case 0x4EA9:
case 0x4EAA:
case 0x4EAB:
case 0x4EAC:
case 0x4EAD:
case 0x4EAE:
case 0x4EAF:

// JSR
case 0x4EA8:
{
	u32 adr;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	PUSH_32_F(PC)
	SET_PC(adr)
	POST_IO
}
RET(18)
case 0x4EB1:
case 0x4EB2:
case 0x4EB3:
case 0x4EB4:
case 0x4EB5:
case 0x4EB6:
case 0x4EB7:

// JSR
case 0x4EB0:
{
	u32 adr;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	PUSH_32_F(PC)
	SET_PC(adr)
	POST_IO
}
RET(22)

// JSR
case 0x4EB8:
{
	u32 adr;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	PUSH_32_F(PC)
	SET_PC(adr)
	POST_IO
}
RET(18)

// JSR
case 0x4EB9:
{
	u32 adr;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	PUSH_32_F(PC)
	SET_PC(adr)
	POST_IO
}
RET(20)

// JSR
case 0x4EBA:
{
	u32 adr;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	PUSH_32_F(PC)
	SET_PC(adr)
	POST_IO
}
RET(18)

// JSR
case 0x4EBB:
{
	u32 adr;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	PUSH_32_F(PC)
	SET_PC(adr)
	POST_IO
}
RET(22)
case 0x4ED1:
case 0x4ED2:
case 0x4ED3:
case 0x4ED4:
case 0x4ED5:
case 0x4ED6:
case 0x4ED7:

// JMP
case 0x4ED0:
{
	u32 adr;
	adr = CPU->A[(Opcode >> 0) & 7];
	SET_PC(adr)
}
RET(8)
case 0x4EE9:
case 0x4EEA:
case 0x4EEB:
case 0x4EEC:
case 0x4EED:
case 0x4EEE:
case 0x4EEF:

// JMP
case 0x4EE8:
{
	u32 adr;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	SET_PC(adr)
}
RET(10)
case 0x4EF1:
case 0x4EF2:
case 0x4EF3:
case 0x4EF4:
case 0x4EF5:
case 0x4EF6:
case 0x4EF7:

// JMP
case 0x4EF0:
{
	u32 adr;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	SET_PC(adr)
}
RET(14)

// JMP
case 0x4EF8:
{
	u32 adr;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	SET_PC(adr)
}
RET(10)

// JMP
case 0x4EF9:
{
	u32 adr;
	adr = (s32)FETCH_LONG;
	PC += 4;
	SET_PC(adr)
}
RET(12)

// JMP
case 0x4EFA:
{
	u32 adr;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	SET_PC(adr)
}
RET(10)

// JMP
case 0x4EFB:
{
	u32 adr;
	adr = PC;
	DECODE_EXT_WORD
	SET_PC(adr)
}
RET(14)
case 0x4380:
case 0x4580:
case 0x4780:
case 0x4980:
case 0x4B80:
case 0x4D80:
case 0x4F80:
case 0x4181:
case 0x4381:
case 0x4581:
case 0x4781:
case 0x4981:
case 0x4B81:
case 0x4D81:
case 0x4F81:
case 0x4182:
case 0x4382:
case 0x4582:
case 0x4782:
case 0x4982:
case 0x4B82:
case 0x4D82:
case 0x4F82:
case 0x4183:
case 0x4383:
case 0x4583:
case 0x4783:
case 0x4983:
case 0x4B83:
case 0x4D83:
case 0x4F83:
case 0x4184:
case 0x4384:
case 0x4584:
case 0x4784:
case 0x4984:
case 0x4B84:
case 0x4D84:
case 0x4F84:
case 0x4185:
case 0x4385:
case 0x4585:
case 0x4785:
case 0x4985:
case 0x4B85:
case 0x4D85:
case 0x4F85:
case 0x4186:
case 0x4386:
case 0x4586:
case 0x4786:
case 0x4986:
case 0x4B86:
case 0x4D86:
case 0x4F86:
case 0x4187:
case 0x4387:
case 0x4587:
case 0x4787:
case 0x4987:
case 0x4B87:
case 0x4D87:
case 0x4F87:

// CHK
case 0x4180:
{
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	if (((s32)res < 0) || (res > src))
	{
		CPU->flag_N = res >> 8;
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_CHK_EX;
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
	POST_IO
}
RET(10)
case 0x4390:
case 0x4590:
case 0x4790:
case 0x4990:
case 0x4B90:
case 0x4D90:
case 0x4F90:
case 0x4191:
case 0x4391:
case 0x4591:
case 0x4791:
case 0x4991:
case 0x4B91:
case 0x4D91:
case 0x4F91:
case 0x4192:
case 0x4392:
case 0x4592:
case 0x4792:
case 0x4992:
case 0x4B92:
case 0x4D92:
case 0x4F92:
case 0x4193:
case 0x4393:
case 0x4593:
case 0x4793:
case 0x4993:
case 0x4B93:
case 0x4D93:
case 0x4F93:
case 0x4194:
case 0x4394:
case 0x4594:
case 0x4794:
case 0x4994:
case 0x4B94:
case 0x4D94:
case 0x4F94:
case 0x4195:
case 0x4395:
case 0x4595:
case 0x4795:
case 0x4995:
case 0x4B95:
case 0x4D95:
case 0x4F95:
case 0x4196:
case 0x4396:
case 0x4596:
case 0x4796:
case 0x4996:
case 0x4B96:
case 0x4D96:
case 0x4F96:
case 0x4197:
case 0x4397:
case 0x4597:
case 0x4797:
case 0x4997:
case 0x4B97:
case 0x4D97:
case 0x4F97:

// CHK
case 0x4190:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	if (((s32)res < 0) || (res > src))
	{
		CPU->flag_N = res >> 8;
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_CHK_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
	POST_IO
}
RET(14)
case 0x4398:
case 0x4598:
case 0x4798:
case 0x4998:
case 0x4B98:
case 0x4D98:
case 0x4F98:
case 0x4199:
case 0x4399:
case 0x4599:
case 0x4799:
case 0x4999:
case 0x4B99:
case 0x4D99:
case 0x4F99:
case 0x419A:
case 0x439A:
case 0x459A:
case 0x479A:
case 0x499A:
case 0x4B9A:
case 0x4D9A:
case 0x4F9A:
case 0x419B:
case 0x439B:
case 0x459B:
case 0x479B:
case 0x499B:
case 0x4B9B:
case 0x4D9B:
case 0x4F9B:
case 0x419C:
case 0x439C:
case 0x459C:
case 0x479C:
case 0x499C:
case 0x4B9C:
case 0x4D9C:
case 0x4F9C:
case 0x419D:
case 0x439D:
case 0x459D:
case 0x479D:
case 0x499D:
case 0x4B9D:
case 0x4D9D:
case 0x4F9D:
case 0x419E:
case 0x439E:
case 0x459E:
case 0x479E:
case 0x499E:
case 0x4B9E:
case 0x4D9E:
case 0x4F9E:

// CHK
case 0x4198:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	if (((s32)res < 0) || (res > src))
	{
		CPU->flag_N = res >> 8;
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_CHK_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
	POST_IO
}
RET(14)
case 0x43A0:
case 0x45A0:
case 0x47A0:
case 0x49A0:
case 0x4BA0:
case 0x4DA0:
case 0x4FA0:
case 0x41A1:
case 0x43A1:
case 0x45A1:
case 0x47A1:
case 0x49A1:
case 0x4BA1:
case 0x4DA1:
case 0x4FA1:
case 0x41A2:
case 0x43A2:
case 0x45A2:
case 0x47A2:
case 0x49A2:
case 0x4BA2:
case 0x4DA2:
case 0x4FA2:
case 0x41A3:
case 0x43A3:
case 0x45A3:
case 0x47A3:
case 0x49A3:
case 0x4BA3:
case 0x4DA3:
case 0x4FA3:
case 0x41A4:
case 0x43A4:
case 0x45A4:
case 0x47A4:
case 0x49A4:
case 0x4BA4:
case 0x4DA4:
case 0x4FA4:
case 0x41A5:
case 0x43A5:
case 0x45A5:
case 0x47A5:
case 0x49A5:
case 0x4BA5:
case 0x4DA5:
case 0x4FA5:
case 0x41A6:
case 0x43A6:
case 0x45A6:
case 0x47A6:
case 0x49A6:
case 0x4BA6:
case 0x4DA6:
case 0x4FA6:

// CHK
case 0x41A0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	if (((s32)res < 0) || (res > src))
	{
		CPU->flag_N = res >> 8;
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_CHK_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
	POST_IO
}
RET(16)
case 0x43A8:
case 0x45A8:
case 0x47A8:
case 0x49A8:
case 0x4BA8:
case 0x4DA8:
case 0x4FA8:
case 0x41A9:
case 0x43A9:
case 0x45A9:
case 0x47A9:
case 0x49A9:
case 0x4BA9:
case 0x4DA9:
case 0x4FA9:
case 0x41AA:
case 0x43AA:
case 0x45AA:
case 0x47AA:
case 0x49AA:
case 0x4BAA:
case 0x4DAA:
case 0x4FAA:
case 0x41AB:
case 0x43AB:
case 0x45AB:
case 0x47AB:
case 0x49AB:
case 0x4BAB:
case 0x4DAB:
case 0x4FAB:
case 0x41AC:
case 0x43AC:
case 0x45AC:
case 0x47AC:
case 0x49AC:
case 0x4BAC:
case 0x4DAC:
case 0x4FAC:
case 0x41AD:
case 0x43AD:
case 0x45AD:
case 0x47AD:
case 0x49AD:
case 0x4BAD:
case 0x4DAD:
case 0x4FAD:
case 0x41AE:
case 0x43AE:
case 0x45AE:
case 0x47AE:
case 0x49AE:
case 0x4BAE:
case 0x4DAE:
case 0x4FAE:
case 0x41AF:
case 0x43AF:
case 0x45AF:
case 0x47AF:
case 0x49AF:
case 0x4BAF:
case 0x4DAF:
case 0x4FAF:

// CHK
case 0x41A8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	if (((s32)res < 0) || (res > src))
	{
		CPU->flag_N = res >> 8;
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_CHK_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
	POST_IO
}
RET(18)
case 0x43B0:
case 0x45B0:
case 0x47B0:
case 0x49B0:
case 0x4BB0:
case 0x4DB0:
case 0x4FB0:
case 0x41B1:
case 0x43B1:
case 0x45B1:
case 0x47B1:
case 0x49B1:
case 0x4BB1:
case 0x4DB1:
case 0x4FB1:
case 0x41B2:
case 0x43B2:
case 0x45B2:
case 0x47B2:
case 0x49B2:
case 0x4BB2:
case 0x4DB2:
case 0x4FB2:
case 0x41B3:
case 0x43B3:
case 0x45B3:
case 0x47B3:
case 0x49B3:
case 0x4BB3:
case 0x4DB3:
case 0x4FB3:
case 0x41B4:
case 0x43B4:
case 0x45B4:
case 0x47B4:
case 0x49B4:
case 0x4BB4:
case 0x4DB4:
case 0x4FB4:
case 0x41B5:
case 0x43B5:
case 0x45B5:
case 0x47B5:
case 0x49B5:
case 0x4BB5:
case 0x4DB5:
case 0x4FB5:
case 0x41B6:
case 0x43B6:
case 0x45B6:
case 0x47B6:
case 0x49B6:
case 0x4BB6:
case 0x4DB6:
case 0x4FB6:
case 0x41B7:
case 0x43B7:
case 0x45B7:
case 0x47B7:
case 0x49B7:
case 0x4BB7:
case 0x4DB7:
case 0x4FB7:

// CHK
case 0x41B0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	if (((s32)res < 0) || (res > src))
	{
		CPU->flag_N = res >> 8;
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_CHK_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
	POST_IO
}
RET(20)
case 0x43B8:
case 0x45B8:
case 0x47B8:
case 0x49B8:
case 0x4BB8:
case 0x4DB8:
case 0x4FB8:

// CHK
case 0x41B8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	if (((s32)res < 0) || (res > src))
	{
		CPU->flag_N = res >> 8;
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_CHK_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
	POST_IO
}
RET(18)
case 0x43B9:
case 0x45B9:
case 0x47B9:
case 0x49B9:
case 0x4BB9:
case 0x4DB9:
case 0x4FB9:

// CHK
case 0x41B9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	if (((s32)res < 0) || (res > src))
	{
		CPU->flag_N = res >> 8;
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_CHK_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
	POST_IO
}
RET(22)
case 0x43BA:
case 0x45BA:
case 0x47BA:
case 0x49BA:
case 0x4BBA:
case 0x4DBA:
case 0x4FBA:

// CHK
case 0x41BA:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	if (((s32)res < 0) || (res > src))
	{
		CPU->flag_N = res >> 8;
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_CHK_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
	POST_IO
}
RET(18)
case 0x43BB:
case 0x45BB:
case 0x47BB:
case 0x49BB:
case 0x4BBB:
case 0x4DBB:
case 0x4FBB:

// CHK
case 0x41BB:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	if (((s32)res < 0) || (res > src))
	{
		CPU->flag_N = res >> 8;
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_CHK_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
	POST_IO
}
RET(20)
case 0x43BC:
case 0x45BC:
case 0x47BC:
case 0x49BC:
case 0x4BBC:
case 0x4DBC:
case 0x4FBC:

// CHK
case 0x41BC:
{
	u32 res;
	u32 src;
	src = FETCH_WORD;
	PC += 2;
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	if (((s32)res < 0) || (res > src))
	{
		CPU->flag_N = res >> 8;
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_CHK_EX;
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
	POST_IO
}
RET(14)
case 0x439F:
case 0x459F:
case 0x479F:
case 0x499F:
case 0x4B9F:
case 0x4D9F:
case 0x4F9F:

// CHK
case 0x419F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	if (((s32)res < 0) || (res > src))
	{
		CPU->flag_N = res >> 8;
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_CHK_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
	POST_IO
}
RET(14)
case 0x43A7:
case 0x45A7:
case 0x47A7:
case 0x49A7:
case 0x4BA7:
case 0x4DA7:
case 0x4FA7:

// CHK
case 0x41A7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	if (((s32)res < 0) || (res > src))
	{
		CPU->flag_N = res >> 8;
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_CHK_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	}
	POST_IO
}
RET(16)
case 0x43D0:
case 0x45D0:
case 0x47D0:
case 0x49D0:
case 0x4BD0:
case 0x4DD0:
case 0x4FD0:
case 0x41D1:
case 0x43D1:
case 0x45D1:
case 0x47D1:
case 0x49D1:
case 0x4BD1:
case 0x4DD1:
case 0x4FD1:
case 0x41D2:
case 0x43D2:
case 0x45D2:
case 0x47D2:
case 0x49D2:
case 0x4BD2:
case 0x4DD2:
case 0x4FD2:
case 0x41D3:
case 0x43D3:
case 0x45D3:
case 0x47D3:
case 0x49D3:
case 0x4BD3:
case 0x4DD3:
case 0x4FD3:
case 0x41D4:
case 0x43D4:
case 0x45D4:
case 0x47D4:
case 0x49D4:
case 0x4BD4:
case 0x4DD4:
case 0x4FD4:
case 0x41D5:
case 0x43D5:
case 0x45D5:
case 0x47D5:
case 0x49D5:
case 0x4BD5:
case 0x4DD5:
case 0x4FD5:
case 0x41D6:
case 0x43D6:
case 0x45D6:
case 0x47D6:
case 0x49D6:
case 0x4BD6:
case 0x4DD6:
case 0x4FD6:
case 0x41D7:
case 0x43D7:
case 0x45D7:
case 0x47D7:
case 0x49D7:
case 0x4BD7:
case 0x4DD7:
case 0x4FD7:

// LEA
case 0x41D0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	res = adr;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(4)
case 0x43E8:
case 0x45E8:
case 0x47E8:
case 0x49E8:
case 0x4BE8:
case 0x4DE8:
case 0x4FE8:
case 0x41E9:
case 0x43E9:
case 0x45E9:
case 0x47E9:
case 0x49E9:
case 0x4BE9:
case 0x4DE9:
case 0x4FE9:
case 0x41EA:
case 0x43EA:
case 0x45EA:
case 0x47EA:
case 0x49EA:
case 0x4BEA:
case 0x4DEA:
case 0x4FEA:
case 0x41EB:
case 0x43EB:
case 0x45EB:
case 0x47EB:
case 0x49EB:
case 0x4BEB:
case 0x4DEB:
case 0x4FEB:
case 0x41EC:
case 0x43EC:
case 0x45EC:
case 0x47EC:
case 0x49EC:
case 0x4BEC:
case 0x4DEC:
case 0x4FEC:
case 0x41ED:
case 0x43ED:
case 0x45ED:
case 0x47ED:
case 0x49ED:
case 0x4BED:
case 0x4DED:
case 0x4FED:
case 0x41EE:
case 0x43EE:
case 0x45EE:
case 0x47EE:
case 0x49EE:
case 0x4BEE:
case 0x4DEE:
case 0x4FEE:
case 0x41EF:
case 0x43EF:
case 0x45EF:
case 0x47EF:
case 0x49EF:
case 0x4BEF:
case 0x4DEF:
case 0x4FEF:

// LEA
case 0x41E8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	res = adr;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(8)
case 0x43F0:
case 0x45F0:
case 0x47F0:
case 0x49F0:
case 0x4BF0:
case 0x4DF0:
case 0x4FF0:
case 0x41F1:
case 0x43F1:
case 0x45F1:
case 0x47F1:
case 0x49F1:
case 0x4BF1:
case 0x4DF1:
case 0x4FF1:
case 0x41F2:
case 0x43F2:
case 0x45F2:
case 0x47F2:
case 0x49F2:
case 0x4BF2:
case 0x4DF2:
case 0x4FF2:
case 0x41F3:
case 0x43F3:
case 0x45F3:
case 0x47F3:
case 0x49F3:
case 0x4BF3:
case 0x4DF3:
case 0x4FF3:
case 0x41F4:
case 0x43F4:
case 0x45F4:
case 0x47F4:
case 0x49F4:
case 0x4BF4:
case 0x4DF4:
case 0x4FF4:
case 0x41F5:
case 0x43F5:
case 0x45F5:
case 0x47F5:
case 0x49F5:
case 0x4BF5:
case 0x4DF5:
case 0x4FF5:
case 0x41F6:
case 0x43F6:
case 0x45F6:
case 0x47F6:
case 0x49F6:
case 0x4BF6:
case 0x4DF6:
case 0x4FF6:
case 0x41F7:
case 0x43F7:
case 0x45F7:
case 0x47F7:
case 0x49F7:
case 0x4BF7:
case 0x4DF7:
case 0x4FF7:

// LEA
case 0x41F0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	res = adr;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(12)
case 0x43F8:
case 0x45F8:
case 0x47F8:
case 0x49F8:
case 0x4BF8:
case 0x4DF8:
case 0x4FF8:

// LEA
case 0x41F8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	res = adr;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(8)
case 0x43F9:
case 0x45F9:
case 0x47F9:
case 0x49F9:
case 0x4BF9:
case 0x4DF9:
case 0x4FF9:

// LEA
case 0x41F9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	res = adr;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(12)
case 0x43FA:
case 0x45FA:
case 0x47FA:
case 0x49FA:
case 0x4BFA:
case 0x4DFA:
case 0x4FFA:

// LEA
case 0x41FA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	res = adr;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(8)
case 0x43FB:
case 0x45FB:
case 0x47FB:
case 0x49FB:
case 0x4BFB:
case 0x4DFB:
case 0x4FFB:

// LEA
case 0x41FB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	res = adr;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(12)
