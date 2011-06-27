case 0x50C1:
case 0x50C2:
case 0x50C3:
case 0x50C4:
case 0x50C5:
case 0x50C6:
case 0x50C7:

// STCC
case 0x50C0:
{
	u32 res;
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
}
case 0x51C1:
case 0x51C2:
case 0x51C3:
case 0x51C4:
case 0x51C5:
case 0x51C6:
case 0x51C7:

// STCC
case 0x51C0:
{
	u32 res;
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x52C1:
case 0x52C2:
case 0x52C3:
case 0x52C4:
case 0x52C5:
case 0x52C6:
case 0x52C7:

// STCC
case 0x52C0:
{
	u32 res;
	if (CPU->flag_notZ && (!(CPU->flag_C & 0x100)))
	{
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x53C1:
case 0x53C2:
case 0x53C3:
case 0x53C4:
case 0x53C5:
case 0x53C6:
case 0x53C7:

// STCC
case 0x53C0:
{
	u32 res;
	if ((!CPU->flag_notZ) || (CPU->flag_C & 0x100))
	{
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x54C1:
case 0x54C2:
case 0x54C3:
case 0x54C4:
case 0x54C5:
case 0x54C6:
case 0x54C7:

// STCC
case 0x54C0:
{
	u32 res;
	if (!(CPU->flag_C & 0x100))
	{
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x55C1:
case 0x55C2:
case 0x55C3:
case 0x55C4:
case 0x55C5:
case 0x55C6:
case 0x55C7:

// STCC
case 0x55C0:
{
	u32 res;
	if (CPU->flag_C & 0x100)
	{
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x56C1:
case 0x56C2:
case 0x56C3:
case 0x56C4:
case 0x56C5:
case 0x56C6:
case 0x56C7:

// STCC
case 0x56C0:
{
	u32 res;
	if (CPU->flag_notZ)
	{
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x57C1:
case 0x57C2:
case 0x57C3:
case 0x57C4:
case 0x57C5:
case 0x57C6:
case 0x57C7:

// STCC
case 0x57C0:
{
	u32 res;
	if (!CPU->flag_notZ)
	{
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x58C1:
case 0x58C2:
case 0x58C3:
case 0x58C4:
case 0x58C5:
case 0x58C6:
case 0x58C7:

// STCC
case 0x58C0:
{
	u32 res;
	if (!(CPU->flag_V & 0x80))
	{
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x59C1:
case 0x59C2:
case 0x59C3:
case 0x59C4:
case 0x59C5:
case 0x59C6:
case 0x59C7:

// STCC
case 0x59C0:
{
	u32 res;
	if (CPU->flag_V & 0x80)
	{
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x5AC1:
case 0x5AC2:
case 0x5AC3:
case 0x5AC4:
case 0x5AC5:
case 0x5AC6:
case 0x5AC7:

// STCC
case 0x5AC0:
{
	u32 res;
	if (!(CPU->flag_N & 0x80))
	{
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x5BC1:
case 0x5BC2:
case 0x5BC3:
case 0x5BC4:
case 0x5BC5:
case 0x5BC6:
case 0x5BC7:

// STCC
case 0x5BC0:
{
	u32 res;
	if (CPU->flag_N & 0x80)
	{
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x5CC1:
case 0x5CC2:
case 0x5CC3:
case 0x5CC4:
case 0x5CC5:
case 0x5CC6:
case 0x5CC7:

// STCC
case 0x5CC0:
{
	u32 res;
	if (!((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x5DC1:
case 0x5DC2:
case 0x5DC3:
case 0x5DC4:
case 0x5DC5:
case 0x5DC6:
case 0x5DC7:

// STCC
case 0x5DC0:
{
	u32 res;
	if ((CPU->flag_N ^ CPU->flag_V) & 0x80)
	{
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x5EC1:
case 0x5EC2:
case 0x5EC3:
case 0x5EC4:
case 0x5EC5:
case 0x5EC6:
case 0x5EC7:

// STCC
case 0x5EC0:
{
	u32 res;
	if (CPU->flag_notZ && (!((CPU->flag_N ^ CPU->flag_V) & 0x80)))
	{
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x5FC1:
case 0x5FC2:
case 0x5FC3:
case 0x5FC4:
case 0x5FC5:
case 0x5FC6:
case 0x5FC7:

// STCC
case 0x5FC0:
{
	u32 res;
	if ((!CPU->flag_notZ) || ((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(4)
}
case 0x50D1:
case 0x50D2:
case 0x50D3:
case 0x50D4:
case 0x50D5:
case 0x50D6:
case 0x50D7:

// STCC
case 0x50D0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x51D1:
case 0x51D2:
case 0x51D3:
case 0x51D4:
case 0x51D5:
case 0x51D6:
case 0x51D7:

// STCC
case 0x51D0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x52D1:
case 0x52D2:
case 0x52D3:
case 0x52D4:
case 0x52D5:
case 0x52D6:
case 0x52D7:

// STCC
case 0x52D0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	if (CPU->flag_notZ && (!(CPU->flag_C & 0x100)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x53D1:
case 0x53D2:
case 0x53D3:
case 0x53D4:
case 0x53D5:
case 0x53D6:
case 0x53D7:

// STCC
case 0x53D0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	if ((!CPU->flag_notZ) || (CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x54D1:
case 0x54D2:
case 0x54D3:
case 0x54D4:
case 0x54D5:
case 0x54D6:
case 0x54D7:

// STCC
case 0x54D0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	if (!(CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x55D1:
case 0x55D2:
case 0x55D3:
case 0x55D4:
case 0x55D5:
case 0x55D6:
case 0x55D7:

// STCC
case 0x55D0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	if (CPU->flag_C & 0x100)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x56D1:
case 0x56D2:
case 0x56D3:
case 0x56D4:
case 0x56D5:
case 0x56D6:
case 0x56D7:

// STCC
case 0x56D0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	if (CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x57D1:
case 0x57D2:
case 0x57D3:
case 0x57D4:
case 0x57D5:
case 0x57D6:
case 0x57D7:

// STCC
case 0x57D0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	if (!CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x58D1:
case 0x58D2:
case 0x58D3:
case 0x58D4:
case 0x58D5:
case 0x58D6:
case 0x58D7:

// STCC
case 0x58D0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	if (!(CPU->flag_V & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x59D1:
case 0x59D2:
case 0x59D3:
case 0x59D4:
case 0x59D5:
case 0x59D6:
case 0x59D7:

// STCC
case 0x59D0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	if (CPU->flag_V & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x5AD1:
case 0x5AD2:
case 0x5AD3:
case 0x5AD4:
case 0x5AD5:
case 0x5AD6:
case 0x5AD7:

// STCC
case 0x5AD0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	if (!(CPU->flag_N & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x5BD1:
case 0x5BD2:
case 0x5BD3:
case 0x5BD4:
case 0x5BD5:
case 0x5BD6:
case 0x5BD7:

// STCC
case 0x5BD0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	if (CPU->flag_N & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x5CD1:
case 0x5CD2:
case 0x5CD3:
case 0x5CD4:
case 0x5CD5:
case 0x5CD6:
case 0x5CD7:

// STCC
case 0x5CD0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	if (!((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x5DD1:
case 0x5DD2:
case 0x5DD3:
case 0x5DD4:
case 0x5DD5:
case 0x5DD6:
case 0x5DD7:

// STCC
case 0x5DD0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	if ((CPU->flag_N ^ CPU->flag_V) & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x5ED1:
case 0x5ED2:
case 0x5ED3:
case 0x5ED4:
case 0x5ED5:
case 0x5ED6:
case 0x5ED7:

// STCC
case 0x5ED0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	if (CPU->flag_notZ && (!((CPU->flag_N ^ CPU->flag_V) & 0x80)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x5FD1:
case 0x5FD2:
case 0x5FD3:
case 0x5FD4:
case 0x5FD5:
case 0x5FD6:
case 0x5FD7:

// STCC
case 0x5FD0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	if ((!CPU->flag_notZ) || ((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x50D9:
case 0x50DA:
case 0x50DB:
case 0x50DC:
case 0x50DD:
case 0x50DE:

// STCC
case 0x50D8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x51D9:
case 0x51DA:
case 0x51DB:
case 0x51DC:
case 0x51DD:
case 0x51DE:

// STCC
case 0x51D8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x52D9:
case 0x52DA:
case 0x52DB:
case 0x52DC:
case 0x52DD:
case 0x52DE:

// STCC
case 0x52D8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	if (CPU->flag_notZ && (!(CPU->flag_C & 0x100)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x53D9:
case 0x53DA:
case 0x53DB:
case 0x53DC:
case 0x53DD:
case 0x53DE:

// STCC
case 0x53D8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	if ((!CPU->flag_notZ) || (CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x54D9:
case 0x54DA:
case 0x54DB:
case 0x54DC:
case 0x54DD:
case 0x54DE:

// STCC
case 0x54D8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	if (!(CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x55D9:
case 0x55DA:
case 0x55DB:
case 0x55DC:
case 0x55DD:
case 0x55DE:

// STCC
case 0x55D8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	if (CPU->flag_C & 0x100)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x56D9:
case 0x56DA:
case 0x56DB:
case 0x56DC:
case 0x56DD:
case 0x56DE:

// STCC
case 0x56D8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	if (CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x57D9:
case 0x57DA:
case 0x57DB:
case 0x57DC:
case 0x57DD:
case 0x57DE:

// STCC
case 0x57D8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	if (!CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x58D9:
case 0x58DA:
case 0x58DB:
case 0x58DC:
case 0x58DD:
case 0x58DE:

// STCC
case 0x58D8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	if (!(CPU->flag_V & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x59D9:
case 0x59DA:
case 0x59DB:
case 0x59DC:
case 0x59DD:
case 0x59DE:

// STCC
case 0x59D8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	if (CPU->flag_V & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x5AD9:
case 0x5ADA:
case 0x5ADB:
case 0x5ADC:
case 0x5ADD:
case 0x5ADE:

// STCC
case 0x5AD8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	if (!(CPU->flag_N & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x5BD9:
case 0x5BDA:
case 0x5BDB:
case 0x5BDC:
case 0x5BDD:
case 0x5BDE:

// STCC
case 0x5BD8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	if (CPU->flag_N & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x5CD9:
case 0x5CDA:
case 0x5CDB:
case 0x5CDC:
case 0x5CDD:
case 0x5CDE:

// STCC
case 0x5CD8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	if (!((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x5DD9:
case 0x5DDA:
case 0x5DDB:
case 0x5DDC:
case 0x5DDD:
case 0x5DDE:

// STCC
case 0x5DD8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	if ((CPU->flag_N ^ CPU->flag_V) & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x5ED9:
case 0x5EDA:
case 0x5EDB:
case 0x5EDC:
case 0x5EDD:
case 0x5EDE:

// STCC
case 0x5ED8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	if (CPU->flag_notZ && (!((CPU->flag_N ^ CPU->flag_V) & 0x80)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x5FD9:
case 0x5FDA:
case 0x5FDB:
case 0x5FDC:
case 0x5FDD:
case 0x5FDE:

// STCC
case 0x5FD8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	if ((!CPU->flag_notZ) || ((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}
case 0x50E1:
case 0x50E2:
case 0x50E3:
case 0x50E4:
case 0x50E5:
case 0x50E6:

// STCC
case 0x50E0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x51E1:
case 0x51E2:
case 0x51E3:
case 0x51E4:
case 0x51E5:
case 0x51E6:

// STCC
case 0x51E0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x52E1:
case 0x52E2:
case 0x52E3:
case 0x52E4:
case 0x52E5:
case 0x52E6:

// STCC
case 0x52E0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	if (CPU->flag_notZ && (!(CPU->flag_C & 0x100)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x53E1:
case 0x53E2:
case 0x53E3:
case 0x53E4:
case 0x53E5:
case 0x53E6:

// STCC
case 0x53E0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	if ((!CPU->flag_notZ) || (CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x54E1:
case 0x54E2:
case 0x54E3:
case 0x54E4:
case 0x54E5:
case 0x54E6:

// STCC
case 0x54E0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	if (!(CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x55E1:
case 0x55E2:
case 0x55E3:
case 0x55E4:
case 0x55E5:
case 0x55E6:

// STCC
case 0x55E0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	if (CPU->flag_C & 0x100)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x56E1:
case 0x56E2:
case 0x56E3:
case 0x56E4:
case 0x56E5:
case 0x56E6:

// STCC
case 0x56E0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	if (CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x57E1:
case 0x57E2:
case 0x57E3:
case 0x57E4:
case 0x57E5:
case 0x57E6:

// STCC
case 0x57E0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	if (!CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x58E1:
case 0x58E2:
case 0x58E3:
case 0x58E4:
case 0x58E5:
case 0x58E6:

// STCC
case 0x58E0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	if (!(CPU->flag_V & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x59E1:
case 0x59E2:
case 0x59E3:
case 0x59E4:
case 0x59E5:
case 0x59E6:

// STCC
case 0x59E0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	if (CPU->flag_V & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x5AE1:
case 0x5AE2:
case 0x5AE3:
case 0x5AE4:
case 0x5AE5:
case 0x5AE6:

// STCC
case 0x5AE0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	if (!(CPU->flag_N & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x5BE1:
case 0x5BE2:
case 0x5BE3:
case 0x5BE4:
case 0x5BE5:
case 0x5BE6:

// STCC
case 0x5BE0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	if (CPU->flag_N & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x5CE1:
case 0x5CE2:
case 0x5CE3:
case 0x5CE4:
case 0x5CE5:
case 0x5CE6:

// STCC
case 0x5CE0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	if (!((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x5DE1:
case 0x5DE2:
case 0x5DE3:
case 0x5DE4:
case 0x5DE5:
case 0x5DE6:

// STCC
case 0x5DE0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	if ((CPU->flag_N ^ CPU->flag_V) & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x5EE1:
case 0x5EE2:
case 0x5EE3:
case 0x5EE4:
case 0x5EE5:
case 0x5EE6:

// STCC
case 0x5EE0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	if (CPU->flag_notZ && (!((CPU->flag_N ^ CPU->flag_V) & 0x80)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x5FE1:
case 0x5FE2:
case 0x5FE3:
case 0x5FE4:
case 0x5FE5:
case 0x5FE6:

// STCC
case 0x5FE0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	if ((!CPU->flag_notZ) || ((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x50E9:
case 0x50EA:
case 0x50EB:
case 0x50EC:
case 0x50ED:
case 0x50EE:
case 0x50EF:

// STCC
case 0x50E8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x51E9:
case 0x51EA:
case 0x51EB:
case 0x51EC:
case 0x51ED:
case 0x51EE:
case 0x51EF:

// STCC
case 0x51E8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x52E9:
case 0x52EA:
case 0x52EB:
case 0x52EC:
case 0x52ED:
case 0x52EE:
case 0x52EF:

// STCC
case 0x52E8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	if (CPU->flag_notZ && (!(CPU->flag_C & 0x100)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x53E9:
case 0x53EA:
case 0x53EB:
case 0x53EC:
case 0x53ED:
case 0x53EE:
case 0x53EF:

// STCC
case 0x53E8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	if ((!CPU->flag_notZ) || (CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x54E9:
case 0x54EA:
case 0x54EB:
case 0x54EC:
case 0x54ED:
case 0x54EE:
case 0x54EF:

// STCC
case 0x54E8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	if (!(CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x55E9:
case 0x55EA:
case 0x55EB:
case 0x55EC:
case 0x55ED:
case 0x55EE:
case 0x55EF:

// STCC
case 0x55E8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	if (CPU->flag_C & 0x100)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x56E9:
case 0x56EA:
case 0x56EB:
case 0x56EC:
case 0x56ED:
case 0x56EE:
case 0x56EF:

// STCC
case 0x56E8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	if (CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x57E9:
case 0x57EA:
case 0x57EB:
case 0x57EC:
case 0x57ED:
case 0x57EE:
case 0x57EF:

// STCC
case 0x57E8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	if (!CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x58E9:
case 0x58EA:
case 0x58EB:
case 0x58EC:
case 0x58ED:
case 0x58EE:
case 0x58EF:

// STCC
case 0x58E8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	if (!(CPU->flag_V & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x59E9:
case 0x59EA:
case 0x59EB:
case 0x59EC:
case 0x59ED:
case 0x59EE:
case 0x59EF:

// STCC
case 0x59E8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	if (CPU->flag_V & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x5AE9:
case 0x5AEA:
case 0x5AEB:
case 0x5AEC:
case 0x5AED:
case 0x5AEE:
case 0x5AEF:

// STCC
case 0x5AE8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	if (!(CPU->flag_N & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x5BE9:
case 0x5BEA:
case 0x5BEB:
case 0x5BEC:
case 0x5BED:
case 0x5BEE:
case 0x5BEF:

// STCC
case 0x5BE8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	if (CPU->flag_N & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x5CE9:
case 0x5CEA:
case 0x5CEB:
case 0x5CEC:
case 0x5CED:
case 0x5CEE:
case 0x5CEF:

// STCC
case 0x5CE8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	if (!((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x5DE9:
case 0x5DEA:
case 0x5DEB:
case 0x5DEC:
case 0x5DED:
case 0x5DEE:
case 0x5DEF:

// STCC
case 0x5DE8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	if ((CPU->flag_N ^ CPU->flag_V) & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x5EE9:
case 0x5EEA:
case 0x5EEB:
case 0x5EEC:
case 0x5EED:
case 0x5EEE:
case 0x5EEF:

// STCC
case 0x5EE8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	if (CPU->flag_notZ && (!((CPU->flag_N ^ CPU->flag_V) & 0x80)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x5FE9:
case 0x5FEA:
case 0x5FEB:
case 0x5FEC:
case 0x5FED:
case 0x5FEE:
case 0x5FEF:

// STCC
case 0x5FE8:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	if ((!CPU->flag_notZ) || ((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}
case 0x50F1:
case 0x50F2:
case 0x50F3:
case 0x50F4:
case 0x50F5:
case 0x50F6:
case 0x50F7:

// STCC
case 0x50F0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x51F1:
case 0x51F2:
case 0x51F3:
case 0x51F4:
case 0x51F5:
case 0x51F6:
case 0x51F7:

// STCC
case 0x51F0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x52F1:
case 0x52F2:
case 0x52F3:
case 0x52F4:
case 0x52F5:
case 0x52F6:
case 0x52F7:

// STCC
case 0x52F0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	if (CPU->flag_notZ && (!(CPU->flag_C & 0x100)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x53F1:
case 0x53F2:
case 0x53F3:
case 0x53F4:
case 0x53F5:
case 0x53F6:
case 0x53F7:

// STCC
case 0x53F0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	if ((!CPU->flag_notZ) || (CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x54F1:
case 0x54F2:
case 0x54F3:
case 0x54F4:
case 0x54F5:
case 0x54F6:
case 0x54F7:

// STCC
case 0x54F0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	if (!(CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x55F1:
case 0x55F2:
case 0x55F3:
case 0x55F4:
case 0x55F5:
case 0x55F6:
case 0x55F7:

// STCC
case 0x55F0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	if (CPU->flag_C & 0x100)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x56F1:
case 0x56F2:
case 0x56F3:
case 0x56F4:
case 0x56F5:
case 0x56F6:
case 0x56F7:

// STCC
case 0x56F0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	if (CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x57F1:
case 0x57F2:
case 0x57F3:
case 0x57F4:
case 0x57F5:
case 0x57F6:
case 0x57F7:

// STCC
case 0x57F0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	if (!CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x58F1:
case 0x58F2:
case 0x58F3:
case 0x58F4:
case 0x58F5:
case 0x58F6:
case 0x58F7:

// STCC
case 0x58F0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	if (!(CPU->flag_V & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x59F1:
case 0x59F2:
case 0x59F3:
case 0x59F4:
case 0x59F5:
case 0x59F6:
case 0x59F7:

// STCC
case 0x59F0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	if (CPU->flag_V & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x5AF1:
case 0x5AF2:
case 0x5AF3:
case 0x5AF4:
case 0x5AF5:
case 0x5AF6:
case 0x5AF7:

// STCC
case 0x5AF0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	if (!(CPU->flag_N & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x5BF1:
case 0x5BF2:
case 0x5BF3:
case 0x5BF4:
case 0x5BF5:
case 0x5BF6:
case 0x5BF7:

// STCC
case 0x5BF0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	if (CPU->flag_N & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x5CF1:
case 0x5CF2:
case 0x5CF3:
case 0x5CF4:
case 0x5CF5:
case 0x5CF6:
case 0x5CF7:

// STCC
case 0x5CF0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	if (!((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x5DF1:
case 0x5DF2:
case 0x5DF3:
case 0x5DF4:
case 0x5DF5:
case 0x5DF6:
case 0x5DF7:

// STCC
case 0x5DF0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	if ((CPU->flag_N ^ CPU->flag_V) & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x5EF1:
case 0x5EF2:
case 0x5EF3:
case 0x5EF4:
case 0x5EF5:
case 0x5EF6:
case 0x5EF7:

// STCC
case 0x5EF0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	if (CPU->flag_notZ && (!((CPU->flag_N ^ CPU->flag_V) & 0x80)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}
case 0x5FF1:
case 0x5FF2:
case 0x5FF3:
case 0x5FF4:
case 0x5FF5:
case 0x5FF6:
case 0x5FF7:

// STCC
case 0x5FF0:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	if ((!CPU->flag_notZ) || ((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(18)
}

// STCC
case 0x50F8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x51F8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x52F8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	if (CPU->flag_notZ && (!(CPU->flag_C & 0x100)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x53F8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	if ((!CPU->flag_notZ) || (CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x54F8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	if (!(CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x55F8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	if (CPU->flag_C & 0x100)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x56F8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	if (CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x57F8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	if (!CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x58F8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	if (!(CPU->flag_V & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x59F8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	if (CPU->flag_V & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x5AF8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	if (!(CPU->flag_N & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x5BF8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	if (CPU->flag_N & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x5CF8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	if (!((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x5DF8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	if ((CPU->flag_N ^ CPU->flag_V) & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x5EF8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	if (CPU->flag_notZ && (!((CPU->flag_N ^ CPU->flag_V) & 0x80)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x5FF8:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	if ((!CPU->flag_notZ) || ((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(16)
}

// STCC
case 0x50F9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x51F9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x52F9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	if (CPU->flag_notZ && (!(CPU->flag_C & 0x100)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x53F9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	if ((!CPU->flag_notZ) || (CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x54F9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	if (!(CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x55F9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	if (CPU->flag_C & 0x100)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x56F9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	if (CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x57F9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	if (!CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x58F9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	if (!(CPU->flag_V & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x59F9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	if (CPU->flag_V & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x5AF9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	if (!(CPU->flag_N & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x5BF9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	if (CPU->flag_N & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x5CF9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	if (!((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x5DF9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	if ((CPU->flag_N ^ CPU->flag_V) & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x5EF9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	if (CPU->flag_notZ && (!((CPU->flag_N ^ CPU->flag_V) & 0x80)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x5FF9:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	if ((!CPU->flag_notZ) || ((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(20)
}

// STCC
case 0x50DF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x51DF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x52DF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	if (CPU->flag_notZ && (!(CPU->flag_C & 0x100)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x53DF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	if ((!CPU->flag_notZ) || (CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x54DF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	if (!(CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x55DF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	if (CPU->flag_C & 0x100)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x56DF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	if (CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x57DF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	if (!CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x58DF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	if (!(CPU->flag_V & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x59DF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	if (CPU->flag_V & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x5ADF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	if (!(CPU->flag_N & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x5BDF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	if (CPU->flag_N & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x5CDF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	if (!((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x5DDF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	if ((CPU->flag_N ^ CPU->flag_V) & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x5EDF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	if (CPU->flag_notZ && (!((CPU->flag_N ^ CPU->flag_V) & 0x80)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x5FDF:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	if ((!CPU->flag_notZ) || ((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(12)
}

// STCC
case 0x50E7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x51E7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x52E7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	if (CPU->flag_notZ && (!(CPU->flag_C & 0x100)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x53E7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	if ((!CPU->flag_notZ) || (CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x54E7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	if (!(CPU->flag_C & 0x100))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x55E7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	if (CPU->flag_C & 0x100)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x56E7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	if (CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x57E7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	if (!CPU->flag_notZ)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x58E7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	if (!(CPU->flag_V & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x59E7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	if (CPU->flag_V & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x5AE7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	if (!(CPU->flag_N & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x5BE7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	if (CPU->flag_N & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x5CE7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	if (!((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x5DE7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	if ((CPU->flag_N ^ CPU->flag_V) & 0x80)
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x5EE7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	if (CPU->flag_notZ && (!((CPU->flag_N ^ CPU->flag_V) & 0x80)))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}

// STCC
case 0x5FE7:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	if ((!CPU->flag_notZ) || ((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = 0xFF;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
	}
	res = 0;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
	RET(14)
}
case 0x50C9:
case 0x50CA:
case 0x50CB:
case 0x50CC:
case 0x50CD:
case 0x50CE:
case 0x50CF:

// DBCC
case 0x50C8:
{
	u32 res;
	PC += 2;
}
RET(12)
case 0x51C9:
case 0x51CA:
case 0x51CB:
case 0x51CC:
case 0x51CD:
case 0x51CE:
case 0x51CF:

// DBCC
case 0x51C8:
{
	u32 res;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	PC += 2;
}
RET(14)
case 0x52C9:
case 0x52CA:
case 0x52CB:
case 0x52CC:
case 0x52CD:
case 0x52CE:
case 0x52CF:

// DBCC
case 0x52C8:
{
	u32 res;
	if ((!CPU->flag_notZ) || (CPU->flag_C & 0x100))
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	}
	else
	{
		PC += 2;
	RET(12)
	}
	PC += 2;
}
RET(14)
case 0x53C9:
case 0x53CA:
case 0x53CB:
case 0x53CC:
case 0x53CD:
case 0x53CE:
case 0x53CF:

// DBCC
case 0x53C8:
{
	u32 res;
	if (CPU->flag_notZ && (!(CPU->flag_C & 0x100)))
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	}
	else
	{
		PC += 2;
	RET(12)
	}
	PC += 2;
}
RET(14)
case 0x54C9:
case 0x54CA:
case 0x54CB:
case 0x54CC:
case 0x54CD:
case 0x54CE:
case 0x54CF:

// DBCC
case 0x54C8:
{
	u32 res;
	if (CPU->flag_C & 0x100)
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	}
	else
	{
		PC += 2;
	RET(12)
	}
	PC += 2;
}
RET(14)
case 0x55C9:
case 0x55CA:
case 0x55CB:
case 0x55CC:
case 0x55CD:
case 0x55CE:
case 0x55CF:

// DBCC
case 0x55C8:
{
	u32 res;
	if (!(CPU->flag_C & 0x100))
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	}
	else
	{
		PC += 2;
	RET(12)
	}
	PC += 2;
}
RET(14)
case 0x56C9:
case 0x56CA:
case 0x56CB:
case 0x56CC:
case 0x56CD:
case 0x56CE:
case 0x56CF:

// DBCC
case 0x56C8:
{
	u32 res;
	if (!CPU->flag_notZ)
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	}
	else
	{
		PC += 2;
	RET(12)
	}
	PC += 2;
}
RET(14)
case 0x57C9:
case 0x57CA:
case 0x57CB:
case 0x57CC:
case 0x57CD:
case 0x57CE:
case 0x57CF:

// DBCC
case 0x57C8:
{
	u32 res;
	if (CPU->flag_notZ)
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	}
	else
	{
		PC += 2;
	RET(12)
	}
	PC += 2;
}
RET(14)
case 0x58C9:
case 0x58CA:
case 0x58CB:
case 0x58CC:
case 0x58CD:
case 0x58CE:
case 0x58CF:

// DBCC
case 0x58C8:
{
	u32 res;
	if (CPU->flag_V & 0x80)
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	}
	else
	{
		PC += 2;
	RET(12)
	}
	PC += 2;
}
RET(14)
case 0x59C9:
case 0x59CA:
case 0x59CB:
case 0x59CC:
case 0x59CD:
case 0x59CE:
case 0x59CF:

// DBCC
case 0x59C8:
{
	u32 res;
	if (!(CPU->flag_V & 0x80))
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	}
	else
	{
		PC += 2;
	RET(12)
	}
	PC += 2;
}
RET(14)
case 0x5AC9:
case 0x5ACA:
case 0x5ACB:
case 0x5ACC:
case 0x5ACD:
case 0x5ACE:
case 0x5ACF:

// DBCC
case 0x5AC8:
{
	u32 res;
	if (CPU->flag_N & 0x80)
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	}
	else
	{
		PC += 2;
	RET(12)
	}
	PC += 2;
}
RET(14)
case 0x5BC9:
case 0x5BCA:
case 0x5BCB:
case 0x5BCC:
case 0x5BCD:
case 0x5BCE:
case 0x5BCF:

// DBCC
case 0x5BC8:
{
	u32 res;
	if (!(CPU->flag_N & 0x80))
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	}
	else
	{
		PC += 2;
	RET(12)
	}
	PC += 2;
}
RET(14)
case 0x5CC9:
case 0x5CCA:
case 0x5CCB:
case 0x5CCC:
case 0x5CCD:
case 0x5CCE:
case 0x5CCF:

// DBCC
case 0x5CC8:
{
	u32 res;
	if ((CPU->flag_N ^ CPU->flag_V) & 0x80)
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	}
	else
	{
		PC += 2;
	RET(12)
	}
	PC += 2;
}
RET(14)
case 0x5DC9:
case 0x5DCA:
case 0x5DCB:
case 0x5DCC:
case 0x5DCD:
case 0x5DCE:
case 0x5DCF:

// DBCC
case 0x5DC8:
{
	u32 res;
	if (!((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	}
	else
	{
		PC += 2;
	RET(12)
	}
	PC += 2;
}
RET(14)
case 0x5EC9:
case 0x5ECA:
case 0x5ECB:
case 0x5ECC:
case 0x5ECD:
case 0x5ECE:
case 0x5ECF:

// DBCC
case 0x5EC8:
{
	u32 res;
	if ((!CPU->flag_notZ) || ((CPU->flag_N ^ CPU->flag_V) & 0x80))
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	}
	else
	{
		PC += 2;
	RET(12)
	}
	PC += 2;
}
RET(14)
case 0x5FC9:
case 0x5FCA:
case 0x5FCB:
case 0x5FCC:
case 0x5FCD:
case 0x5FCE:
case 0x5FCF:

// DBCC
case 0x5FC8:
{
	u32 res;
	if (CPU->flag_notZ && (!((CPU->flag_N ^ CPU->flag_V) & 0x80)))
	{
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res--;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	if ((s32)res != -1)
	{
		PC += (s32)(s16)FETCH_WORD;
		SET_PC(PC);
	RET(10)
	}
	}
	else
	{
		PC += 2;
	RET(12)
	}
	PC += 2;
}
RET(14)
case 0x5200:
case 0x5400:
case 0x5600:
case 0x5800:
case 0x5A00:
case 0x5C00:
case 0x5E00:
case 0x5001:
case 0x5201:
case 0x5401:
case 0x5601:
case 0x5801:
case 0x5A01:
case 0x5C01:
case 0x5E01:
case 0x5002:
case 0x5202:
case 0x5402:
case 0x5602:
case 0x5802:
case 0x5A02:
case 0x5C02:
case 0x5E02:
case 0x5003:
case 0x5203:
case 0x5403:
case 0x5603:
case 0x5803:
case 0x5A03:
case 0x5C03:
case 0x5E03:
case 0x5004:
case 0x5204:
case 0x5404:
case 0x5604:
case 0x5804:
case 0x5A04:
case 0x5C04:
case 0x5E04:
case 0x5005:
case 0x5205:
case 0x5405:
case 0x5605:
case 0x5805:
case 0x5A05:
case 0x5C05:
case 0x5E05:
case 0x5006:
case 0x5206:
case 0x5406:
case 0x5606:
case 0x5806:
case 0x5A06:
case 0x5C06:
case 0x5E06:
case 0x5007:
case 0x5207:
case 0x5407:
case 0x5607:
case 0x5807:
case 0x5A07:
case 0x5C07:
case 0x5E07:

// ADDQ
case 0x5000:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	dst = (u8)CPU->D[(Opcode >> 0) & 7];
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x5210:
case 0x5410:
case 0x5610:
case 0x5810:
case 0x5A10:
case 0x5C10:
case 0x5E10:
case 0x5011:
case 0x5211:
case 0x5411:
case 0x5611:
case 0x5811:
case 0x5A11:
case 0x5C11:
case 0x5E11:
case 0x5012:
case 0x5212:
case 0x5412:
case 0x5612:
case 0x5812:
case 0x5A12:
case 0x5C12:
case 0x5E12:
case 0x5013:
case 0x5213:
case 0x5413:
case 0x5613:
case 0x5813:
case 0x5A13:
case 0x5C13:
case 0x5E13:
case 0x5014:
case 0x5214:
case 0x5414:
case 0x5614:
case 0x5814:
case 0x5A14:
case 0x5C14:
case 0x5E14:
case 0x5015:
case 0x5215:
case 0x5415:
case 0x5615:
case 0x5815:
case 0x5A15:
case 0x5C15:
case 0x5E15:
case 0x5016:
case 0x5216:
case 0x5416:
case 0x5616:
case 0x5816:
case 0x5A16:
case 0x5C16:
case 0x5E16:
case 0x5017:
case 0x5217:
case 0x5417:
case 0x5617:
case 0x5817:
case 0x5A17:
case 0x5C17:
case 0x5E17:

// ADDQ
case 0x5010:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x5218:
case 0x5418:
case 0x5618:
case 0x5818:
case 0x5A18:
case 0x5C18:
case 0x5E18:
case 0x5019:
case 0x5219:
case 0x5419:
case 0x5619:
case 0x5819:
case 0x5A19:
case 0x5C19:
case 0x5E19:
case 0x501A:
case 0x521A:
case 0x541A:
case 0x561A:
case 0x581A:
case 0x5A1A:
case 0x5C1A:
case 0x5E1A:
case 0x501B:
case 0x521B:
case 0x541B:
case 0x561B:
case 0x581B:
case 0x5A1B:
case 0x5C1B:
case 0x5E1B:
case 0x501C:
case 0x521C:
case 0x541C:
case 0x561C:
case 0x581C:
case 0x5A1C:
case 0x5C1C:
case 0x5E1C:
case 0x501D:
case 0x521D:
case 0x541D:
case 0x561D:
case 0x581D:
case 0x5A1D:
case 0x5C1D:
case 0x5E1D:
case 0x501E:
case 0x521E:
case 0x541E:
case 0x561E:
case 0x581E:
case 0x5A1E:
case 0x5C1E:
case 0x5E1E:

// ADDQ
case 0x5018:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x5220:
case 0x5420:
case 0x5620:
case 0x5820:
case 0x5A20:
case 0x5C20:
case 0x5E20:
case 0x5021:
case 0x5221:
case 0x5421:
case 0x5621:
case 0x5821:
case 0x5A21:
case 0x5C21:
case 0x5E21:
case 0x5022:
case 0x5222:
case 0x5422:
case 0x5622:
case 0x5822:
case 0x5A22:
case 0x5C22:
case 0x5E22:
case 0x5023:
case 0x5223:
case 0x5423:
case 0x5623:
case 0x5823:
case 0x5A23:
case 0x5C23:
case 0x5E23:
case 0x5024:
case 0x5224:
case 0x5424:
case 0x5624:
case 0x5824:
case 0x5A24:
case 0x5C24:
case 0x5E24:
case 0x5025:
case 0x5225:
case 0x5425:
case 0x5625:
case 0x5825:
case 0x5A25:
case 0x5C25:
case 0x5E25:
case 0x5026:
case 0x5226:
case 0x5426:
case 0x5626:
case 0x5826:
case 0x5A26:
case 0x5C26:
case 0x5E26:

// ADDQ
case 0x5020:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x5228:
case 0x5428:
case 0x5628:
case 0x5828:
case 0x5A28:
case 0x5C28:
case 0x5E28:
case 0x5029:
case 0x5229:
case 0x5429:
case 0x5629:
case 0x5829:
case 0x5A29:
case 0x5C29:
case 0x5E29:
case 0x502A:
case 0x522A:
case 0x542A:
case 0x562A:
case 0x582A:
case 0x5A2A:
case 0x5C2A:
case 0x5E2A:
case 0x502B:
case 0x522B:
case 0x542B:
case 0x562B:
case 0x582B:
case 0x5A2B:
case 0x5C2B:
case 0x5E2B:
case 0x502C:
case 0x522C:
case 0x542C:
case 0x562C:
case 0x582C:
case 0x5A2C:
case 0x5C2C:
case 0x5E2C:
case 0x502D:
case 0x522D:
case 0x542D:
case 0x562D:
case 0x582D:
case 0x5A2D:
case 0x5C2D:
case 0x5E2D:
case 0x502E:
case 0x522E:
case 0x542E:
case 0x562E:
case 0x582E:
case 0x5A2E:
case 0x5C2E:
case 0x5E2E:
case 0x502F:
case 0x522F:
case 0x542F:
case 0x562F:
case 0x582F:
case 0x5A2F:
case 0x5C2F:
case 0x5E2F:

// ADDQ
case 0x5028:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x5230:
case 0x5430:
case 0x5630:
case 0x5830:
case 0x5A30:
case 0x5C30:
case 0x5E30:
case 0x5031:
case 0x5231:
case 0x5431:
case 0x5631:
case 0x5831:
case 0x5A31:
case 0x5C31:
case 0x5E31:
case 0x5032:
case 0x5232:
case 0x5432:
case 0x5632:
case 0x5832:
case 0x5A32:
case 0x5C32:
case 0x5E32:
case 0x5033:
case 0x5233:
case 0x5433:
case 0x5633:
case 0x5833:
case 0x5A33:
case 0x5C33:
case 0x5E33:
case 0x5034:
case 0x5234:
case 0x5434:
case 0x5634:
case 0x5834:
case 0x5A34:
case 0x5C34:
case 0x5E34:
case 0x5035:
case 0x5235:
case 0x5435:
case 0x5635:
case 0x5835:
case 0x5A35:
case 0x5C35:
case 0x5E35:
case 0x5036:
case 0x5236:
case 0x5436:
case 0x5636:
case 0x5836:
case 0x5A36:
case 0x5C36:
case 0x5E36:
case 0x5037:
case 0x5237:
case 0x5437:
case 0x5637:
case 0x5837:
case 0x5A37:
case 0x5C37:
case 0x5E37:

// ADDQ
case 0x5030:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x5238:
case 0x5438:
case 0x5638:
case 0x5838:
case 0x5A38:
case 0x5C38:
case 0x5E38:

// ADDQ
case 0x5038:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x5239:
case 0x5439:
case 0x5639:
case 0x5839:
case 0x5A39:
case 0x5C39:
case 0x5E39:

// ADDQ
case 0x5039:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0x521F:
case 0x541F:
case 0x561F:
case 0x581F:
case 0x5A1F:
case 0x5C1F:
case 0x5E1F:

// ADDQ
case 0x501F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x5227:
case 0x5427:
case 0x5627:
case 0x5827:
case 0x5A27:
case 0x5C27:
case 0x5E27:

// ADDQ
case 0x5027:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x5240:
case 0x5440:
case 0x5640:
case 0x5840:
case 0x5A40:
case 0x5C40:
case 0x5E40:
case 0x5041:
case 0x5241:
case 0x5441:
case 0x5641:
case 0x5841:
case 0x5A41:
case 0x5C41:
case 0x5E41:
case 0x5042:
case 0x5242:
case 0x5442:
case 0x5642:
case 0x5842:
case 0x5A42:
case 0x5C42:
case 0x5E42:
case 0x5043:
case 0x5243:
case 0x5443:
case 0x5643:
case 0x5843:
case 0x5A43:
case 0x5C43:
case 0x5E43:
case 0x5044:
case 0x5244:
case 0x5444:
case 0x5644:
case 0x5844:
case 0x5A44:
case 0x5C44:
case 0x5E44:
case 0x5045:
case 0x5245:
case 0x5445:
case 0x5645:
case 0x5845:
case 0x5A45:
case 0x5C45:
case 0x5E45:
case 0x5046:
case 0x5246:
case 0x5446:
case 0x5646:
case 0x5846:
case 0x5A46:
case 0x5C46:
case 0x5E46:
case 0x5047:
case 0x5247:
case 0x5447:
case 0x5647:
case 0x5847:
case 0x5A47:
case 0x5C47:
case 0x5E47:

// ADDQ
case 0x5040:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	dst = (u16)CPU->D[(Opcode >> 0) & 7];
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x5248:
case 0x5448:
case 0x5648:
case 0x5848:
case 0x5A48:
case 0x5C48:
case 0x5E48:
case 0x5049:
case 0x5249:
case 0x5449:
case 0x5649:
case 0x5849:
case 0x5A49:
case 0x5C49:
case 0x5E49:
case 0x504A:
case 0x524A:
case 0x544A:
case 0x564A:
case 0x584A:
case 0x5A4A:
case 0x5C4A:
case 0x5E4A:
case 0x504B:
case 0x524B:
case 0x544B:
case 0x564B:
case 0x584B:
case 0x5A4B:
case 0x5C4B:
case 0x5E4B:
case 0x504C:
case 0x524C:
case 0x544C:
case 0x564C:
case 0x584C:
case 0x5A4C:
case 0x5C4C:
case 0x5E4C:
case 0x504D:
case 0x524D:
case 0x544D:
case 0x564D:
case 0x584D:
case 0x5A4D:
case 0x5C4D:
case 0x5E4D:
case 0x504E:
case 0x524E:
case 0x544E:
case 0x564E:
case 0x584E:
case 0x5A4E:
case 0x5C4E:
case 0x5E4E:
case 0x504F:
case 0x524F:
case 0x544F:
case 0x564F:
case 0x584F:
case 0x5A4F:
case 0x5C4F:
case 0x5E4F:

// ADDQ
case 0x5048:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	dst = (u32)CPU->A[(Opcode >> 0) & 7];
	res = dst + src;
	CPU->A[(Opcode >> 0) & 7] = res;
}
RET(8)
case 0x5250:
case 0x5450:
case 0x5650:
case 0x5850:
case 0x5A50:
case 0x5C50:
case 0x5E50:
case 0x5051:
case 0x5251:
case 0x5451:
case 0x5651:
case 0x5851:
case 0x5A51:
case 0x5C51:
case 0x5E51:
case 0x5052:
case 0x5252:
case 0x5452:
case 0x5652:
case 0x5852:
case 0x5A52:
case 0x5C52:
case 0x5E52:
case 0x5053:
case 0x5253:
case 0x5453:
case 0x5653:
case 0x5853:
case 0x5A53:
case 0x5C53:
case 0x5E53:
case 0x5054:
case 0x5254:
case 0x5454:
case 0x5654:
case 0x5854:
case 0x5A54:
case 0x5C54:
case 0x5E54:
case 0x5055:
case 0x5255:
case 0x5455:
case 0x5655:
case 0x5855:
case 0x5A55:
case 0x5C55:
case 0x5E55:
case 0x5056:
case 0x5256:
case 0x5456:
case 0x5656:
case 0x5856:
case 0x5A56:
case 0x5C56:
case 0x5E56:
case 0x5057:
case 0x5257:
case 0x5457:
case 0x5657:
case 0x5857:
case 0x5A57:
case 0x5C57:
case 0x5E57:

// ADDQ
case 0x5050:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x5258:
case 0x5458:
case 0x5658:
case 0x5858:
case 0x5A58:
case 0x5C58:
case 0x5E58:
case 0x5059:
case 0x5259:
case 0x5459:
case 0x5659:
case 0x5859:
case 0x5A59:
case 0x5C59:
case 0x5E59:
case 0x505A:
case 0x525A:
case 0x545A:
case 0x565A:
case 0x585A:
case 0x5A5A:
case 0x5C5A:
case 0x5E5A:
case 0x505B:
case 0x525B:
case 0x545B:
case 0x565B:
case 0x585B:
case 0x5A5B:
case 0x5C5B:
case 0x5E5B:
case 0x505C:
case 0x525C:
case 0x545C:
case 0x565C:
case 0x585C:
case 0x5A5C:
case 0x5C5C:
case 0x5E5C:
case 0x505D:
case 0x525D:
case 0x545D:
case 0x565D:
case 0x585D:
case 0x5A5D:
case 0x5C5D:
case 0x5E5D:
case 0x505E:
case 0x525E:
case 0x545E:
case 0x565E:
case 0x585E:
case 0x5A5E:
case 0x5C5E:
case 0x5E5E:

// ADDQ
case 0x5058:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x5260:
case 0x5460:
case 0x5660:
case 0x5860:
case 0x5A60:
case 0x5C60:
case 0x5E60:
case 0x5061:
case 0x5261:
case 0x5461:
case 0x5661:
case 0x5861:
case 0x5A61:
case 0x5C61:
case 0x5E61:
case 0x5062:
case 0x5262:
case 0x5462:
case 0x5662:
case 0x5862:
case 0x5A62:
case 0x5C62:
case 0x5E62:
case 0x5063:
case 0x5263:
case 0x5463:
case 0x5663:
case 0x5863:
case 0x5A63:
case 0x5C63:
case 0x5E63:
case 0x5064:
case 0x5264:
case 0x5464:
case 0x5664:
case 0x5864:
case 0x5A64:
case 0x5C64:
case 0x5E64:
case 0x5065:
case 0x5265:
case 0x5465:
case 0x5665:
case 0x5865:
case 0x5A65:
case 0x5C65:
case 0x5E65:
case 0x5066:
case 0x5266:
case 0x5466:
case 0x5666:
case 0x5866:
case 0x5A66:
case 0x5C66:
case 0x5E66:

// ADDQ
case 0x5060:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x5268:
case 0x5468:
case 0x5668:
case 0x5868:
case 0x5A68:
case 0x5C68:
case 0x5E68:
case 0x5069:
case 0x5269:
case 0x5469:
case 0x5669:
case 0x5869:
case 0x5A69:
case 0x5C69:
case 0x5E69:
case 0x506A:
case 0x526A:
case 0x546A:
case 0x566A:
case 0x586A:
case 0x5A6A:
case 0x5C6A:
case 0x5E6A:
case 0x506B:
case 0x526B:
case 0x546B:
case 0x566B:
case 0x586B:
case 0x5A6B:
case 0x5C6B:
case 0x5E6B:
case 0x506C:
case 0x526C:
case 0x546C:
case 0x566C:
case 0x586C:
case 0x5A6C:
case 0x5C6C:
case 0x5E6C:
case 0x506D:
case 0x526D:
case 0x546D:
case 0x566D:
case 0x586D:
case 0x5A6D:
case 0x5C6D:
case 0x5E6D:
case 0x506E:
case 0x526E:
case 0x546E:
case 0x566E:
case 0x586E:
case 0x5A6E:
case 0x5C6E:
case 0x5E6E:
case 0x506F:
case 0x526F:
case 0x546F:
case 0x566F:
case 0x586F:
case 0x5A6F:
case 0x5C6F:
case 0x5E6F:

// ADDQ
case 0x5068:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x5270:
case 0x5470:
case 0x5670:
case 0x5870:
case 0x5A70:
case 0x5C70:
case 0x5E70:
case 0x5071:
case 0x5271:
case 0x5471:
case 0x5671:
case 0x5871:
case 0x5A71:
case 0x5C71:
case 0x5E71:
case 0x5072:
case 0x5272:
case 0x5472:
case 0x5672:
case 0x5872:
case 0x5A72:
case 0x5C72:
case 0x5E72:
case 0x5073:
case 0x5273:
case 0x5473:
case 0x5673:
case 0x5873:
case 0x5A73:
case 0x5C73:
case 0x5E73:
case 0x5074:
case 0x5274:
case 0x5474:
case 0x5674:
case 0x5874:
case 0x5A74:
case 0x5C74:
case 0x5E74:
case 0x5075:
case 0x5275:
case 0x5475:
case 0x5675:
case 0x5875:
case 0x5A75:
case 0x5C75:
case 0x5E75:
case 0x5076:
case 0x5276:
case 0x5476:
case 0x5676:
case 0x5876:
case 0x5A76:
case 0x5C76:
case 0x5E76:
case 0x5077:
case 0x5277:
case 0x5477:
case 0x5677:
case 0x5877:
case 0x5A77:
case 0x5C77:
case 0x5E77:

// ADDQ
case 0x5070:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x5278:
case 0x5478:
case 0x5678:
case 0x5878:
case 0x5A78:
case 0x5C78:
case 0x5E78:

// ADDQ
case 0x5078:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x5279:
case 0x5479:
case 0x5679:
case 0x5879:
case 0x5A79:
case 0x5C79:
case 0x5E79:

// ADDQ
case 0x5079:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0x525F:
case 0x545F:
case 0x565F:
case 0x585F:
case 0x5A5F:
case 0x5C5F:
case 0x5E5F:

// ADDQ
case 0x505F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x5267:
case 0x5467:
case 0x5667:
case 0x5867:
case 0x5A67:
case 0x5C67:
case 0x5E67:

// ADDQ
case 0x5067:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x5280:
case 0x5480:
case 0x5680:
case 0x5880:
case 0x5A80:
case 0x5C80:
case 0x5E80:
case 0x5081:
case 0x5281:
case 0x5481:
case 0x5681:
case 0x5881:
case 0x5A81:
case 0x5C81:
case 0x5E81:
case 0x5082:
case 0x5282:
case 0x5482:
case 0x5682:
case 0x5882:
case 0x5A82:
case 0x5C82:
case 0x5E82:
case 0x5083:
case 0x5283:
case 0x5483:
case 0x5683:
case 0x5883:
case 0x5A83:
case 0x5C83:
case 0x5E83:
case 0x5084:
case 0x5284:
case 0x5484:
case 0x5684:
case 0x5884:
case 0x5A84:
case 0x5C84:
case 0x5E84:
case 0x5085:
case 0x5285:
case 0x5485:
case 0x5685:
case 0x5885:
case 0x5A85:
case 0x5C85:
case 0x5E85:
case 0x5086:
case 0x5286:
case 0x5486:
case 0x5686:
case 0x5886:
case 0x5A86:
case 0x5C86:
case 0x5E86:
case 0x5087:
case 0x5287:
case 0x5487:
case 0x5687:
case 0x5887:
case 0x5A87:
case 0x5C87:
case 0x5E87:

// ADDQ
case 0x5080:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	dst = (u32)CPU->D[(Opcode >> 0) & 7];
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0x5288:
case 0x5488:
case 0x5688:
case 0x5888:
case 0x5A88:
case 0x5C88:
case 0x5E88:
case 0x5089:
case 0x5289:
case 0x5489:
case 0x5689:
case 0x5889:
case 0x5A89:
case 0x5C89:
case 0x5E89:
case 0x508A:
case 0x528A:
case 0x548A:
case 0x568A:
case 0x588A:
case 0x5A8A:
case 0x5C8A:
case 0x5E8A:
case 0x508B:
case 0x528B:
case 0x548B:
case 0x568B:
case 0x588B:
case 0x5A8B:
case 0x5C8B:
case 0x5E8B:
case 0x508C:
case 0x528C:
case 0x548C:
case 0x568C:
case 0x588C:
case 0x5A8C:
case 0x5C8C:
case 0x5E8C:
case 0x508D:
case 0x528D:
case 0x548D:
case 0x568D:
case 0x588D:
case 0x5A8D:
case 0x5C8D:
case 0x5E8D:
case 0x508E:
case 0x528E:
case 0x548E:
case 0x568E:
case 0x588E:
case 0x5A8E:
case 0x5C8E:
case 0x5E8E:
case 0x508F:
case 0x528F:
case 0x548F:
case 0x568F:
case 0x588F:
case 0x5A8F:
case 0x5C8F:
case 0x5E8F:

// ADDQ
case 0x5088:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	dst = (u32)CPU->A[(Opcode >> 0) & 7];
	res = dst + src;
	CPU->A[(Opcode >> 0) & 7] = res;
}
RET(8)
case 0x5290:
case 0x5490:
case 0x5690:
case 0x5890:
case 0x5A90:
case 0x5C90:
case 0x5E90:
case 0x5091:
case 0x5291:
case 0x5491:
case 0x5691:
case 0x5891:
case 0x5A91:
case 0x5C91:
case 0x5E91:
case 0x5092:
case 0x5292:
case 0x5492:
case 0x5692:
case 0x5892:
case 0x5A92:
case 0x5C92:
case 0x5E92:
case 0x5093:
case 0x5293:
case 0x5493:
case 0x5693:
case 0x5893:
case 0x5A93:
case 0x5C93:
case 0x5E93:
case 0x5094:
case 0x5294:
case 0x5494:
case 0x5694:
case 0x5894:
case 0x5A94:
case 0x5C94:
case 0x5E94:
case 0x5095:
case 0x5295:
case 0x5495:
case 0x5695:
case 0x5895:
case 0x5A95:
case 0x5C95:
case 0x5E95:
case 0x5096:
case 0x5296:
case 0x5496:
case 0x5696:
case 0x5896:
case 0x5A96:
case 0x5C96:
case 0x5E96:
case 0x5097:
case 0x5297:
case 0x5497:
case 0x5697:
case 0x5897:
case 0x5A97:
case 0x5C97:
case 0x5E97:

// ADDQ
case 0x5090:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x5298:
case 0x5498:
case 0x5698:
case 0x5898:
case 0x5A98:
case 0x5C98:
case 0x5E98:
case 0x5099:
case 0x5299:
case 0x5499:
case 0x5699:
case 0x5899:
case 0x5A99:
case 0x5C99:
case 0x5E99:
case 0x509A:
case 0x529A:
case 0x549A:
case 0x569A:
case 0x589A:
case 0x5A9A:
case 0x5C9A:
case 0x5E9A:
case 0x509B:
case 0x529B:
case 0x549B:
case 0x569B:
case 0x589B:
case 0x5A9B:
case 0x5C9B:
case 0x5E9B:
case 0x509C:
case 0x529C:
case 0x549C:
case 0x569C:
case 0x589C:
case 0x5A9C:
case 0x5C9C:
case 0x5E9C:
case 0x509D:
case 0x529D:
case 0x549D:
case 0x569D:
case 0x589D:
case 0x5A9D:
case 0x5C9D:
case 0x5E9D:
case 0x509E:
case 0x529E:
case 0x549E:
case 0x569E:
case 0x589E:
case 0x5A9E:
case 0x5C9E:
case 0x5E9E:

// ADDQ
case 0x5098:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x52A0:
case 0x54A0:
case 0x56A0:
case 0x58A0:
case 0x5AA0:
case 0x5CA0:
case 0x5EA0:
case 0x50A1:
case 0x52A1:
case 0x54A1:
case 0x56A1:
case 0x58A1:
case 0x5AA1:
case 0x5CA1:
case 0x5EA1:
case 0x50A2:
case 0x52A2:
case 0x54A2:
case 0x56A2:
case 0x58A2:
case 0x5AA2:
case 0x5CA2:
case 0x5EA2:
case 0x50A3:
case 0x52A3:
case 0x54A3:
case 0x56A3:
case 0x58A3:
case 0x5AA3:
case 0x5CA3:
case 0x5EA3:
case 0x50A4:
case 0x52A4:
case 0x54A4:
case 0x56A4:
case 0x58A4:
case 0x5AA4:
case 0x5CA4:
case 0x5EA4:
case 0x50A5:
case 0x52A5:
case 0x54A5:
case 0x56A5:
case 0x58A5:
case 0x5AA5:
case 0x5CA5:
case 0x5EA5:
case 0x50A6:
case 0x52A6:
case 0x54A6:
case 0x56A6:
case 0x58A6:
case 0x5AA6:
case 0x5CA6:
case 0x5EA6:

// ADDQ
case 0x50A0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x52A8:
case 0x54A8:
case 0x56A8:
case 0x58A8:
case 0x5AA8:
case 0x5CA8:
case 0x5EA8:
case 0x50A9:
case 0x52A9:
case 0x54A9:
case 0x56A9:
case 0x58A9:
case 0x5AA9:
case 0x5CA9:
case 0x5EA9:
case 0x50AA:
case 0x52AA:
case 0x54AA:
case 0x56AA:
case 0x58AA:
case 0x5AAA:
case 0x5CAA:
case 0x5EAA:
case 0x50AB:
case 0x52AB:
case 0x54AB:
case 0x56AB:
case 0x58AB:
case 0x5AAB:
case 0x5CAB:
case 0x5EAB:
case 0x50AC:
case 0x52AC:
case 0x54AC:
case 0x56AC:
case 0x58AC:
case 0x5AAC:
case 0x5CAC:
case 0x5EAC:
case 0x50AD:
case 0x52AD:
case 0x54AD:
case 0x56AD:
case 0x58AD:
case 0x5AAD:
case 0x5CAD:
case 0x5EAD:
case 0x50AE:
case 0x52AE:
case 0x54AE:
case 0x56AE:
case 0x58AE:
case 0x5AAE:
case 0x5CAE:
case 0x5EAE:
case 0x50AF:
case 0x52AF:
case 0x54AF:
case 0x56AF:
case 0x58AF:
case 0x5AAF:
case 0x5CAF:
case 0x5EAF:

// ADDQ
case 0x50A8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x52B0:
case 0x54B0:
case 0x56B0:
case 0x58B0:
case 0x5AB0:
case 0x5CB0:
case 0x5EB0:
case 0x50B1:
case 0x52B1:
case 0x54B1:
case 0x56B1:
case 0x58B1:
case 0x5AB1:
case 0x5CB1:
case 0x5EB1:
case 0x50B2:
case 0x52B2:
case 0x54B2:
case 0x56B2:
case 0x58B2:
case 0x5AB2:
case 0x5CB2:
case 0x5EB2:
case 0x50B3:
case 0x52B3:
case 0x54B3:
case 0x56B3:
case 0x58B3:
case 0x5AB3:
case 0x5CB3:
case 0x5EB3:
case 0x50B4:
case 0x52B4:
case 0x54B4:
case 0x56B4:
case 0x58B4:
case 0x5AB4:
case 0x5CB4:
case 0x5EB4:
case 0x50B5:
case 0x52B5:
case 0x54B5:
case 0x56B5:
case 0x58B5:
case 0x5AB5:
case 0x5CB5:
case 0x5EB5:
case 0x50B6:
case 0x52B6:
case 0x54B6:
case 0x56B6:
case 0x58B6:
case 0x5AB6:
case 0x5CB6:
case 0x5EB6:
case 0x50B7:
case 0x52B7:
case 0x54B7:
case 0x56B7:
case 0x58B7:
case 0x5AB7:
case 0x5CB7:
case 0x5EB7:

// ADDQ
case 0x50B0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0x52B8:
case 0x54B8:
case 0x56B8:
case 0x58B8:
case 0x5AB8:
case 0x5CB8:
case 0x5EB8:

// ADDQ
case 0x50B8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x52B9:
case 0x54B9:
case 0x56B9:
case 0x58B9:
case 0x5AB9:
case 0x5CB9:
case 0x5EB9:

// ADDQ
case 0x50B9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0x529F:
case 0x549F:
case 0x569F:
case 0x589F:
case 0x5A9F:
case 0x5C9F:
case 0x5E9F:

// ADDQ
case 0x509F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x52A7:
case 0x54A7:
case 0x56A7:
case 0x58A7:
case 0x5AA7:
case 0x5CA7:
case 0x5EA7:

// ADDQ
case 0x50A7:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x5300:
case 0x5500:
case 0x5700:
case 0x5900:
case 0x5B00:
case 0x5D00:
case 0x5F00:
case 0x5101:
case 0x5301:
case 0x5501:
case 0x5701:
case 0x5901:
case 0x5B01:
case 0x5D01:
case 0x5F01:
case 0x5102:
case 0x5302:
case 0x5502:
case 0x5702:
case 0x5902:
case 0x5B02:
case 0x5D02:
case 0x5F02:
case 0x5103:
case 0x5303:
case 0x5503:
case 0x5703:
case 0x5903:
case 0x5B03:
case 0x5D03:
case 0x5F03:
case 0x5104:
case 0x5304:
case 0x5504:
case 0x5704:
case 0x5904:
case 0x5B04:
case 0x5D04:
case 0x5F04:
case 0x5105:
case 0x5305:
case 0x5505:
case 0x5705:
case 0x5905:
case 0x5B05:
case 0x5D05:
case 0x5F05:
case 0x5106:
case 0x5306:
case 0x5506:
case 0x5706:
case 0x5906:
case 0x5B06:
case 0x5D06:
case 0x5F06:
case 0x5107:
case 0x5307:
case 0x5507:
case 0x5707:
case 0x5907:
case 0x5B07:
case 0x5D07:
case 0x5F07:

// SUBQ
case 0x5100:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	dst = (u8)CPU->D[(Opcode >> 0) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x5310:
case 0x5510:
case 0x5710:
case 0x5910:
case 0x5B10:
case 0x5D10:
case 0x5F10:
case 0x5111:
case 0x5311:
case 0x5511:
case 0x5711:
case 0x5911:
case 0x5B11:
case 0x5D11:
case 0x5F11:
case 0x5112:
case 0x5312:
case 0x5512:
case 0x5712:
case 0x5912:
case 0x5B12:
case 0x5D12:
case 0x5F12:
case 0x5113:
case 0x5313:
case 0x5513:
case 0x5713:
case 0x5913:
case 0x5B13:
case 0x5D13:
case 0x5F13:
case 0x5114:
case 0x5314:
case 0x5514:
case 0x5714:
case 0x5914:
case 0x5B14:
case 0x5D14:
case 0x5F14:
case 0x5115:
case 0x5315:
case 0x5515:
case 0x5715:
case 0x5915:
case 0x5B15:
case 0x5D15:
case 0x5F15:
case 0x5116:
case 0x5316:
case 0x5516:
case 0x5716:
case 0x5916:
case 0x5B16:
case 0x5D16:
case 0x5F16:
case 0x5117:
case 0x5317:
case 0x5517:
case 0x5717:
case 0x5917:
case 0x5B17:
case 0x5D17:
case 0x5F17:

// SUBQ
case 0x5110:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x5318:
case 0x5518:
case 0x5718:
case 0x5918:
case 0x5B18:
case 0x5D18:
case 0x5F18:
case 0x5119:
case 0x5319:
case 0x5519:
case 0x5719:
case 0x5919:
case 0x5B19:
case 0x5D19:
case 0x5F19:
case 0x511A:
case 0x531A:
case 0x551A:
case 0x571A:
case 0x591A:
case 0x5B1A:
case 0x5D1A:
case 0x5F1A:
case 0x511B:
case 0x531B:
case 0x551B:
case 0x571B:
case 0x591B:
case 0x5B1B:
case 0x5D1B:
case 0x5F1B:
case 0x511C:
case 0x531C:
case 0x551C:
case 0x571C:
case 0x591C:
case 0x5B1C:
case 0x5D1C:
case 0x5F1C:
case 0x511D:
case 0x531D:
case 0x551D:
case 0x571D:
case 0x591D:
case 0x5B1D:
case 0x5D1D:
case 0x5F1D:
case 0x511E:
case 0x531E:
case 0x551E:
case 0x571E:
case 0x591E:
case 0x5B1E:
case 0x5D1E:
case 0x5F1E:

// SUBQ
case 0x5118:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x5320:
case 0x5520:
case 0x5720:
case 0x5920:
case 0x5B20:
case 0x5D20:
case 0x5F20:
case 0x5121:
case 0x5321:
case 0x5521:
case 0x5721:
case 0x5921:
case 0x5B21:
case 0x5D21:
case 0x5F21:
case 0x5122:
case 0x5322:
case 0x5522:
case 0x5722:
case 0x5922:
case 0x5B22:
case 0x5D22:
case 0x5F22:
case 0x5123:
case 0x5323:
case 0x5523:
case 0x5723:
case 0x5923:
case 0x5B23:
case 0x5D23:
case 0x5F23:
case 0x5124:
case 0x5324:
case 0x5524:
case 0x5724:
case 0x5924:
case 0x5B24:
case 0x5D24:
case 0x5F24:
case 0x5125:
case 0x5325:
case 0x5525:
case 0x5725:
case 0x5925:
case 0x5B25:
case 0x5D25:
case 0x5F25:
case 0x5126:
case 0x5326:
case 0x5526:
case 0x5726:
case 0x5926:
case 0x5B26:
case 0x5D26:
case 0x5F26:

// SUBQ
case 0x5120:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x5328:
case 0x5528:
case 0x5728:
case 0x5928:
case 0x5B28:
case 0x5D28:
case 0x5F28:
case 0x5129:
case 0x5329:
case 0x5529:
case 0x5729:
case 0x5929:
case 0x5B29:
case 0x5D29:
case 0x5F29:
case 0x512A:
case 0x532A:
case 0x552A:
case 0x572A:
case 0x592A:
case 0x5B2A:
case 0x5D2A:
case 0x5F2A:
case 0x512B:
case 0x532B:
case 0x552B:
case 0x572B:
case 0x592B:
case 0x5B2B:
case 0x5D2B:
case 0x5F2B:
case 0x512C:
case 0x532C:
case 0x552C:
case 0x572C:
case 0x592C:
case 0x5B2C:
case 0x5D2C:
case 0x5F2C:
case 0x512D:
case 0x532D:
case 0x552D:
case 0x572D:
case 0x592D:
case 0x5B2D:
case 0x5D2D:
case 0x5F2D:
case 0x512E:
case 0x532E:
case 0x552E:
case 0x572E:
case 0x592E:
case 0x5B2E:
case 0x5D2E:
case 0x5F2E:
case 0x512F:
case 0x532F:
case 0x552F:
case 0x572F:
case 0x592F:
case 0x5B2F:
case 0x5D2F:
case 0x5F2F:

// SUBQ
case 0x5128:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x5330:
case 0x5530:
case 0x5730:
case 0x5930:
case 0x5B30:
case 0x5D30:
case 0x5F30:
case 0x5131:
case 0x5331:
case 0x5531:
case 0x5731:
case 0x5931:
case 0x5B31:
case 0x5D31:
case 0x5F31:
case 0x5132:
case 0x5332:
case 0x5532:
case 0x5732:
case 0x5932:
case 0x5B32:
case 0x5D32:
case 0x5F32:
case 0x5133:
case 0x5333:
case 0x5533:
case 0x5733:
case 0x5933:
case 0x5B33:
case 0x5D33:
case 0x5F33:
case 0x5134:
case 0x5334:
case 0x5534:
case 0x5734:
case 0x5934:
case 0x5B34:
case 0x5D34:
case 0x5F34:
case 0x5135:
case 0x5335:
case 0x5535:
case 0x5735:
case 0x5935:
case 0x5B35:
case 0x5D35:
case 0x5F35:
case 0x5136:
case 0x5336:
case 0x5536:
case 0x5736:
case 0x5936:
case 0x5B36:
case 0x5D36:
case 0x5F36:
case 0x5137:
case 0x5337:
case 0x5537:
case 0x5737:
case 0x5937:
case 0x5B37:
case 0x5D37:
case 0x5F37:

// SUBQ
case 0x5130:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x5338:
case 0x5538:
case 0x5738:
case 0x5938:
case 0x5B38:
case 0x5D38:
case 0x5F38:

// SUBQ
case 0x5138:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x5339:
case 0x5539:
case 0x5739:
case 0x5939:
case 0x5B39:
case 0x5D39:
case 0x5F39:

// SUBQ
case 0x5139:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0x531F:
case 0x551F:
case 0x571F:
case 0x591F:
case 0x5B1F:
case 0x5D1F:
case 0x5F1F:

// SUBQ
case 0x511F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x5327:
case 0x5527:
case 0x5727:
case 0x5927:
case 0x5B27:
case 0x5D27:
case 0x5F27:

// SUBQ
case 0x5127:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x5340:
case 0x5540:
case 0x5740:
case 0x5940:
case 0x5B40:
case 0x5D40:
case 0x5F40:
case 0x5141:
case 0x5341:
case 0x5541:
case 0x5741:
case 0x5941:
case 0x5B41:
case 0x5D41:
case 0x5F41:
case 0x5142:
case 0x5342:
case 0x5542:
case 0x5742:
case 0x5942:
case 0x5B42:
case 0x5D42:
case 0x5F42:
case 0x5143:
case 0x5343:
case 0x5543:
case 0x5743:
case 0x5943:
case 0x5B43:
case 0x5D43:
case 0x5F43:
case 0x5144:
case 0x5344:
case 0x5544:
case 0x5744:
case 0x5944:
case 0x5B44:
case 0x5D44:
case 0x5F44:
case 0x5145:
case 0x5345:
case 0x5545:
case 0x5745:
case 0x5945:
case 0x5B45:
case 0x5D45:
case 0x5F45:
case 0x5146:
case 0x5346:
case 0x5546:
case 0x5746:
case 0x5946:
case 0x5B46:
case 0x5D46:
case 0x5F46:
case 0x5147:
case 0x5347:
case 0x5547:
case 0x5747:
case 0x5947:
case 0x5B47:
case 0x5D47:
case 0x5F47:

// SUBQ
case 0x5140:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	dst = (u16)CPU->D[(Opcode >> 0) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0x5348:
case 0x5548:
case 0x5748:
case 0x5948:
case 0x5B48:
case 0x5D48:
case 0x5F48:
case 0x5149:
case 0x5349:
case 0x5549:
case 0x5749:
case 0x5949:
case 0x5B49:
case 0x5D49:
case 0x5F49:
case 0x514A:
case 0x534A:
case 0x554A:
case 0x574A:
case 0x594A:
case 0x5B4A:
case 0x5D4A:
case 0x5F4A:
case 0x514B:
case 0x534B:
case 0x554B:
case 0x574B:
case 0x594B:
case 0x5B4B:
case 0x5D4B:
case 0x5F4B:
case 0x514C:
case 0x534C:
case 0x554C:
case 0x574C:
case 0x594C:
case 0x5B4C:
case 0x5D4C:
case 0x5F4C:
case 0x514D:
case 0x534D:
case 0x554D:
case 0x574D:
case 0x594D:
case 0x5B4D:
case 0x5D4D:
case 0x5F4D:
case 0x514E:
case 0x534E:
case 0x554E:
case 0x574E:
case 0x594E:
case 0x5B4E:
case 0x5D4E:
case 0x5F4E:
case 0x514F:
case 0x534F:
case 0x554F:
case 0x574F:
case 0x594F:
case 0x5B4F:
case 0x5D4F:
case 0x5F4F:

// SUBQ
case 0x5148:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	dst = (u32)CPU->A[(Opcode >> 0) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 0) & 7] = res;
}
RET(8)
case 0x5350:
case 0x5550:
case 0x5750:
case 0x5950:
case 0x5B50:
case 0x5D50:
case 0x5F50:
case 0x5151:
case 0x5351:
case 0x5551:
case 0x5751:
case 0x5951:
case 0x5B51:
case 0x5D51:
case 0x5F51:
case 0x5152:
case 0x5352:
case 0x5552:
case 0x5752:
case 0x5952:
case 0x5B52:
case 0x5D52:
case 0x5F52:
case 0x5153:
case 0x5353:
case 0x5553:
case 0x5753:
case 0x5953:
case 0x5B53:
case 0x5D53:
case 0x5F53:
case 0x5154:
case 0x5354:
case 0x5554:
case 0x5754:
case 0x5954:
case 0x5B54:
case 0x5D54:
case 0x5F54:
case 0x5155:
case 0x5355:
case 0x5555:
case 0x5755:
case 0x5955:
case 0x5B55:
case 0x5D55:
case 0x5F55:
case 0x5156:
case 0x5356:
case 0x5556:
case 0x5756:
case 0x5956:
case 0x5B56:
case 0x5D56:
case 0x5F56:
case 0x5157:
case 0x5357:
case 0x5557:
case 0x5757:
case 0x5957:
case 0x5B57:
case 0x5D57:
case 0x5F57:

// SUBQ
case 0x5150:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x5358:
case 0x5558:
case 0x5758:
case 0x5958:
case 0x5B58:
case 0x5D58:
case 0x5F58:
case 0x5159:
case 0x5359:
case 0x5559:
case 0x5759:
case 0x5959:
case 0x5B59:
case 0x5D59:
case 0x5F59:
case 0x515A:
case 0x535A:
case 0x555A:
case 0x575A:
case 0x595A:
case 0x5B5A:
case 0x5D5A:
case 0x5F5A:
case 0x515B:
case 0x535B:
case 0x555B:
case 0x575B:
case 0x595B:
case 0x5B5B:
case 0x5D5B:
case 0x5F5B:
case 0x515C:
case 0x535C:
case 0x555C:
case 0x575C:
case 0x595C:
case 0x5B5C:
case 0x5D5C:
case 0x5F5C:
case 0x515D:
case 0x535D:
case 0x555D:
case 0x575D:
case 0x595D:
case 0x5B5D:
case 0x5D5D:
case 0x5F5D:
case 0x515E:
case 0x535E:
case 0x555E:
case 0x575E:
case 0x595E:
case 0x5B5E:
case 0x5D5E:
case 0x5F5E:

// SUBQ
case 0x5158:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x5360:
case 0x5560:
case 0x5760:
case 0x5960:
case 0x5B60:
case 0x5D60:
case 0x5F60:
case 0x5161:
case 0x5361:
case 0x5561:
case 0x5761:
case 0x5961:
case 0x5B61:
case 0x5D61:
case 0x5F61:
case 0x5162:
case 0x5362:
case 0x5562:
case 0x5762:
case 0x5962:
case 0x5B62:
case 0x5D62:
case 0x5F62:
case 0x5163:
case 0x5363:
case 0x5563:
case 0x5763:
case 0x5963:
case 0x5B63:
case 0x5D63:
case 0x5F63:
case 0x5164:
case 0x5364:
case 0x5564:
case 0x5764:
case 0x5964:
case 0x5B64:
case 0x5D64:
case 0x5F64:
case 0x5165:
case 0x5365:
case 0x5565:
case 0x5765:
case 0x5965:
case 0x5B65:
case 0x5D65:
case 0x5F65:
case 0x5166:
case 0x5366:
case 0x5566:
case 0x5766:
case 0x5966:
case 0x5B66:
case 0x5D66:
case 0x5F66:

// SUBQ
case 0x5160:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x5368:
case 0x5568:
case 0x5768:
case 0x5968:
case 0x5B68:
case 0x5D68:
case 0x5F68:
case 0x5169:
case 0x5369:
case 0x5569:
case 0x5769:
case 0x5969:
case 0x5B69:
case 0x5D69:
case 0x5F69:
case 0x516A:
case 0x536A:
case 0x556A:
case 0x576A:
case 0x596A:
case 0x5B6A:
case 0x5D6A:
case 0x5F6A:
case 0x516B:
case 0x536B:
case 0x556B:
case 0x576B:
case 0x596B:
case 0x5B6B:
case 0x5D6B:
case 0x5F6B:
case 0x516C:
case 0x536C:
case 0x556C:
case 0x576C:
case 0x596C:
case 0x5B6C:
case 0x5D6C:
case 0x5F6C:
case 0x516D:
case 0x536D:
case 0x556D:
case 0x576D:
case 0x596D:
case 0x5B6D:
case 0x5D6D:
case 0x5F6D:
case 0x516E:
case 0x536E:
case 0x556E:
case 0x576E:
case 0x596E:
case 0x5B6E:
case 0x5D6E:
case 0x5F6E:
case 0x516F:
case 0x536F:
case 0x556F:
case 0x576F:
case 0x596F:
case 0x5B6F:
case 0x5D6F:
case 0x5F6F:

// SUBQ
case 0x5168:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x5370:
case 0x5570:
case 0x5770:
case 0x5970:
case 0x5B70:
case 0x5D70:
case 0x5F70:
case 0x5171:
case 0x5371:
case 0x5571:
case 0x5771:
case 0x5971:
case 0x5B71:
case 0x5D71:
case 0x5F71:
case 0x5172:
case 0x5372:
case 0x5572:
case 0x5772:
case 0x5972:
case 0x5B72:
case 0x5D72:
case 0x5F72:
case 0x5173:
case 0x5373:
case 0x5573:
case 0x5773:
case 0x5973:
case 0x5B73:
case 0x5D73:
case 0x5F73:
case 0x5174:
case 0x5374:
case 0x5574:
case 0x5774:
case 0x5974:
case 0x5B74:
case 0x5D74:
case 0x5F74:
case 0x5175:
case 0x5375:
case 0x5575:
case 0x5775:
case 0x5975:
case 0x5B75:
case 0x5D75:
case 0x5F75:
case 0x5176:
case 0x5376:
case 0x5576:
case 0x5776:
case 0x5976:
case 0x5B76:
case 0x5D76:
case 0x5F76:
case 0x5177:
case 0x5377:
case 0x5577:
case 0x5777:
case 0x5977:
case 0x5B77:
case 0x5D77:
case 0x5F77:

// SUBQ
case 0x5170:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x5378:
case 0x5578:
case 0x5778:
case 0x5978:
case 0x5B78:
case 0x5D78:
case 0x5F78:

// SUBQ
case 0x5178:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x5379:
case 0x5579:
case 0x5779:
case 0x5979:
case 0x5B79:
case 0x5D79:
case 0x5F79:

// SUBQ
case 0x5179:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0x535F:
case 0x555F:
case 0x575F:
case 0x595F:
case 0x5B5F:
case 0x5D5F:
case 0x5F5F:

// SUBQ
case 0x515F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x5367:
case 0x5567:
case 0x5767:
case 0x5967:
case 0x5B67:
case 0x5D67:
case 0x5F67:

// SUBQ
case 0x5167:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x5380:
case 0x5580:
case 0x5780:
case 0x5980:
case 0x5B80:
case 0x5D80:
case 0x5F80:
case 0x5181:
case 0x5381:
case 0x5581:
case 0x5781:
case 0x5981:
case 0x5B81:
case 0x5D81:
case 0x5F81:
case 0x5182:
case 0x5382:
case 0x5582:
case 0x5782:
case 0x5982:
case 0x5B82:
case 0x5D82:
case 0x5F82:
case 0x5183:
case 0x5383:
case 0x5583:
case 0x5783:
case 0x5983:
case 0x5B83:
case 0x5D83:
case 0x5F83:
case 0x5184:
case 0x5384:
case 0x5584:
case 0x5784:
case 0x5984:
case 0x5B84:
case 0x5D84:
case 0x5F84:
case 0x5185:
case 0x5385:
case 0x5585:
case 0x5785:
case 0x5985:
case 0x5B85:
case 0x5D85:
case 0x5F85:
case 0x5186:
case 0x5386:
case 0x5586:
case 0x5786:
case 0x5986:
case 0x5B86:
case 0x5D86:
case 0x5F86:
case 0x5187:
case 0x5387:
case 0x5587:
case 0x5787:
case 0x5987:
case 0x5B87:
case 0x5D87:
case 0x5F87:

// SUBQ
case 0x5180:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	dst = (u32)CPU->D[(Opcode >> 0) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0x5388:
case 0x5588:
case 0x5788:
case 0x5988:
case 0x5B88:
case 0x5D88:
case 0x5F88:
case 0x5189:
case 0x5389:
case 0x5589:
case 0x5789:
case 0x5989:
case 0x5B89:
case 0x5D89:
case 0x5F89:
case 0x518A:
case 0x538A:
case 0x558A:
case 0x578A:
case 0x598A:
case 0x5B8A:
case 0x5D8A:
case 0x5F8A:
case 0x518B:
case 0x538B:
case 0x558B:
case 0x578B:
case 0x598B:
case 0x5B8B:
case 0x5D8B:
case 0x5F8B:
case 0x518C:
case 0x538C:
case 0x558C:
case 0x578C:
case 0x598C:
case 0x5B8C:
case 0x5D8C:
case 0x5F8C:
case 0x518D:
case 0x538D:
case 0x558D:
case 0x578D:
case 0x598D:
case 0x5B8D:
case 0x5D8D:
case 0x5F8D:
case 0x518E:
case 0x538E:
case 0x558E:
case 0x578E:
case 0x598E:
case 0x5B8E:
case 0x5D8E:
case 0x5F8E:
case 0x518F:
case 0x538F:
case 0x558F:
case 0x578F:
case 0x598F:
case 0x5B8F:
case 0x5D8F:
case 0x5F8F:

// SUBQ
case 0x5188:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	dst = (u32)CPU->A[(Opcode >> 0) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 0) & 7] = res;
}
RET(8)
case 0x5390:
case 0x5590:
case 0x5790:
case 0x5990:
case 0x5B90:
case 0x5D90:
case 0x5F90:
case 0x5191:
case 0x5391:
case 0x5591:
case 0x5791:
case 0x5991:
case 0x5B91:
case 0x5D91:
case 0x5F91:
case 0x5192:
case 0x5392:
case 0x5592:
case 0x5792:
case 0x5992:
case 0x5B92:
case 0x5D92:
case 0x5F92:
case 0x5193:
case 0x5393:
case 0x5593:
case 0x5793:
case 0x5993:
case 0x5B93:
case 0x5D93:
case 0x5F93:
case 0x5194:
case 0x5394:
case 0x5594:
case 0x5794:
case 0x5994:
case 0x5B94:
case 0x5D94:
case 0x5F94:
case 0x5195:
case 0x5395:
case 0x5595:
case 0x5795:
case 0x5995:
case 0x5B95:
case 0x5D95:
case 0x5F95:
case 0x5196:
case 0x5396:
case 0x5596:
case 0x5796:
case 0x5996:
case 0x5B96:
case 0x5D96:
case 0x5F96:
case 0x5197:
case 0x5397:
case 0x5597:
case 0x5797:
case 0x5997:
case 0x5B97:
case 0x5D97:
case 0x5F97:

// SUBQ
case 0x5190:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x5398:
case 0x5598:
case 0x5798:
case 0x5998:
case 0x5B98:
case 0x5D98:
case 0x5F98:
case 0x5199:
case 0x5399:
case 0x5599:
case 0x5799:
case 0x5999:
case 0x5B99:
case 0x5D99:
case 0x5F99:
case 0x519A:
case 0x539A:
case 0x559A:
case 0x579A:
case 0x599A:
case 0x5B9A:
case 0x5D9A:
case 0x5F9A:
case 0x519B:
case 0x539B:
case 0x559B:
case 0x579B:
case 0x599B:
case 0x5B9B:
case 0x5D9B:
case 0x5F9B:
case 0x519C:
case 0x539C:
case 0x559C:
case 0x579C:
case 0x599C:
case 0x5B9C:
case 0x5D9C:
case 0x5F9C:
case 0x519D:
case 0x539D:
case 0x559D:
case 0x579D:
case 0x599D:
case 0x5B9D:
case 0x5D9D:
case 0x5F9D:
case 0x519E:
case 0x539E:
case 0x559E:
case 0x579E:
case 0x599E:
case 0x5B9E:
case 0x5D9E:
case 0x5F9E:

// SUBQ
case 0x5198:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x53A0:
case 0x55A0:
case 0x57A0:
case 0x59A0:
case 0x5BA0:
case 0x5DA0:
case 0x5FA0:
case 0x51A1:
case 0x53A1:
case 0x55A1:
case 0x57A1:
case 0x59A1:
case 0x5BA1:
case 0x5DA1:
case 0x5FA1:
case 0x51A2:
case 0x53A2:
case 0x55A2:
case 0x57A2:
case 0x59A2:
case 0x5BA2:
case 0x5DA2:
case 0x5FA2:
case 0x51A3:
case 0x53A3:
case 0x55A3:
case 0x57A3:
case 0x59A3:
case 0x5BA3:
case 0x5DA3:
case 0x5FA3:
case 0x51A4:
case 0x53A4:
case 0x55A4:
case 0x57A4:
case 0x59A4:
case 0x5BA4:
case 0x5DA4:
case 0x5FA4:
case 0x51A5:
case 0x53A5:
case 0x55A5:
case 0x57A5:
case 0x59A5:
case 0x5BA5:
case 0x5DA5:
case 0x5FA5:
case 0x51A6:
case 0x53A6:
case 0x55A6:
case 0x57A6:
case 0x59A6:
case 0x5BA6:
case 0x5DA6:
case 0x5FA6:

// SUBQ
case 0x51A0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x53A8:
case 0x55A8:
case 0x57A8:
case 0x59A8:
case 0x5BA8:
case 0x5DA8:
case 0x5FA8:
case 0x51A9:
case 0x53A9:
case 0x55A9:
case 0x57A9:
case 0x59A9:
case 0x5BA9:
case 0x5DA9:
case 0x5FA9:
case 0x51AA:
case 0x53AA:
case 0x55AA:
case 0x57AA:
case 0x59AA:
case 0x5BAA:
case 0x5DAA:
case 0x5FAA:
case 0x51AB:
case 0x53AB:
case 0x55AB:
case 0x57AB:
case 0x59AB:
case 0x5BAB:
case 0x5DAB:
case 0x5FAB:
case 0x51AC:
case 0x53AC:
case 0x55AC:
case 0x57AC:
case 0x59AC:
case 0x5BAC:
case 0x5DAC:
case 0x5FAC:
case 0x51AD:
case 0x53AD:
case 0x55AD:
case 0x57AD:
case 0x59AD:
case 0x5BAD:
case 0x5DAD:
case 0x5FAD:
case 0x51AE:
case 0x53AE:
case 0x55AE:
case 0x57AE:
case 0x59AE:
case 0x5BAE:
case 0x5DAE:
case 0x5FAE:
case 0x51AF:
case 0x53AF:
case 0x55AF:
case 0x57AF:
case 0x59AF:
case 0x5BAF:
case 0x5DAF:
case 0x5FAF:

// SUBQ
case 0x51A8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x53B0:
case 0x55B0:
case 0x57B0:
case 0x59B0:
case 0x5BB0:
case 0x5DB0:
case 0x5FB0:
case 0x51B1:
case 0x53B1:
case 0x55B1:
case 0x57B1:
case 0x59B1:
case 0x5BB1:
case 0x5DB1:
case 0x5FB1:
case 0x51B2:
case 0x53B2:
case 0x55B2:
case 0x57B2:
case 0x59B2:
case 0x5BB2:
case 0x5DB2:
case 0x5FB2:
case 0x51B3:
case 0x53B3:
case 0x55B3:
case 0x57B3:
case 0x59B3:
case 0x5BB3:
case 0x5DB3:
case 0x5FB3:
case 0x51B4:
case 0x53B4:
case 0x55B4:
case 0x57B4:
case 0x59B4:
case 0x5BB4:
case 0x5DB4:
case 0x5FB4:
case 0x51B5:
case 0x53B5:
case 0x55B5:
case 0x57B5:
case 0x59B5:
case 0x5BB5:
case 0x5DB5:
case 0x5FB5:
case 0x51B6:
case 0x53B6:
case 0x55B6:
case 0x57B6:
case 0x59B6:
case 0x5BB6:
case 0x5DB6:
case 0x5FB6:
case 0x51B7:
case 0x53B7:
case 0x55B7:
case 0x57B7:
case 0x59B7:
case 0x5BB7:
case 0x5DB7:
case 0x5FB7:

// SUBQ
case 0x51B0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0x53B8:
case 0x55B8:
case 0x57B8:
case 0x59B8:
case 0x5BB8:
case 0x5DB8:
case 0x5FB8:

// SUBQ
case 0x51B8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x53B9:
case 0x55B9:
case 0x57B9:
case 0x59B9:
case 0x5BB9:
case 0x5DB9:
case 0x5FB9:

// SUBQ
case 0x51B9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0x539F:
case 0x559F:
case 0x579F:
case 0x599F:
case 0x5B9F:
case 0x5D9F:
case 0x5F9F:

// SUBQ
case 0x519F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x53A7:
case 0x55A7:
case 0x57A7:
case 0x59A7:
case 0x5BA7:
case 0x5DA7:
case 0x5FA7:

// SUBQ
case 0x51A7:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (((Opcode >> 9) - 1) & 7) + 1;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READ_LONG_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
