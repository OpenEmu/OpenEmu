case 0x9200:
case 0x9400:
case 0x9600:
case 0x9800:
case 0x9A00:
case 0x9C00:
case 0x9E00:
case 0x9001:
case 0x9201:
case 0x9401:
case 0x9601:
case 0x9801:
case 0x9A01:
case 0x9C01:
case 0x9E01:
case 0x9002:
case 0x9202:
case 0x9402:
case 0x9602:
case 0x9802:
case 0x9A02:
case 0x9C02:
case 0x9E02:
case 0x9003:
case 0x9203:
case 0x9403:
case 0x9603:
case 0x9803:
case 0x9A03:
case 0x9C03:
case 0x9E03:
case 0x9004:
case 0x9204:
case 0x9404:
case 0x9604:
case 0x9804:
case 0x9A04:
case 0x9C04:
case 0x9E04:
case 0x9005:
case 0x9205:
case 0x9405:
case 0x9605:
case 0x9805:
case 0x9A05:
case 0x9C05:
case 0x9E05:
case 0x9006:
case 0x9206:
case 0x9406:
case 0x9606:
case 0x9806:
case 0x9A06:
case 0x9C06:
case 0x9E06:
case 0x9007:
case 0x9207:
case 0x9407:
case 0x9607:
case 0x9807:
case 0x9A07:
case 0x9C07:
case 0x9E07:

// SUBaD
case 0x9000:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0x9208:
case 0x9408:
case 0x9608:
case 0x9808:
case 0x9A08:
case 0x9C08:
case 0x9E08:
case 0x9009:
case 0x9209:
case 0x9409:
case 0x9609:
case 0x9809:
case 0x9A09:
case 0x9C09:
case 0x9E09:
case 0x900A:
case 0x920A:
case 0x940A:
case 0x960A:
case 0x980A:
case 0x9A0A:
case 0x9C0A:
case 0x9E0A:
case 0x900B:
case 0x920B:
case 0x940B:
case 0x960B:
case 0x980B:
case 0x9A0B:
case 0x9C0B:
case 0x9E0B:
case 0x900C:
case 0x920C:
case 0x940C:
case 0x960C:
case 0x980C:
case 0x9A0C:
case 0x9C0C:
case 0x9E0C:
case 0x900D:
case 0x920D:
case 0x940D:
case 0x960D:
case 0x980D:
case 0x9A0D:
case 0x9C0D:
case 0x9E0D:
case 0x900E:
case 0x920E:
case 0x940E:
case 0x960E:
case 0x980E:
case 0x9A0E:
case 0x9C0E:
case 0x9E0E:
case 0x900F:
case 0x920F:
case 0x940F:
case 0x960F:
case 0x980F:
case 0x9A0F:
case 0x9C0F:
case 0x9E0F:

// SUBaD
case 0x9008:
{
	u32 res;
	u32 dst;
	u32 src;
	// can't read byte from Ax registers !
	CPU->Status |= C68K_FAULTED;
	CCnt = 0;
	goto C68k_Exec_Really_End;
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0x9210:
case 0x9410:
case 0x9610:
case 0x9810:
case 0x9A10:
case 0x9C10:
case 0x9E10:
case 0x9011:
case 0x9211:
case 0x9411:
case 0x9611:
case 0x9811:
case 0x9A11:
case 0x9C11:
case 0x9E11:
case 0x9012:
case 0x9212:
case 0x9412:
case 0x9612:
case 0x9812:
case 0x9A12:
case 0x9C12:
case 0x9E12:
case 0x9013:
case 0x9213:
case 0x9413:
case 0x9613:
case 0x9813:
case 0x9A13:
case 0x9C13:
case 0x9E13:
case 0x9014:
case 0x9214:
case 0x9414:
case 0x9614:
case 0x9814:
case 0x9A14:
case 0x9C14:
case 0x9E14:
case 0x9015:
case 0x9215:
case 0x9415:
case 0x9615:
case 0x9815:
case 0x9A15:
case 0x9C15:
case 0x9E15:
case 0x9016:
case 0x9216:
case 0x9416:
case 0x9616:
case 0x9816:
case 0x9A16:
case 0x9C16:
case 0x9E16:
case 0x9017:
case 0x9217:
case 0x9417:
case 0x9617:
case 0x9817:
case 0x9A17:
case 0x9C17:
case 0x9E17:

// SUBaD
case 0x9010:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, src)
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x9218:
case 0x9418:
case 0x9618:
case 0x9818:
case 0x9A18:
case 0x9C18:
case 0x9E18:
case 0x9019:
case 0x9219:
case 0x9419:
case 0x9619:
case 0x9819:
case 0x9A19:
case 0x9C19:
case 0x9E19:
case 0x901A:
case 0x921A:
case 0x941A:
case 0x961A:
case 0x981A:
case 0x9A1A:
case 0x9C1A:
case 0x9E1A:
case 0x901B:
case 0x921B:
case 0x941B:
case 0x961B:
case 0x981B:
case 0x9A1B:
case 0x9C1B:
case 0x9E1B:
case 0x901C:
case 0x921C:
case 0x941C:
case 0x961C:
case 0x981C:
case 0x9A1C:
case 0x9C1C:
case 0x9E1C:
case 0x901D:
case 0x921D:
case 0x941D:
case 0x961D:
case 0x981D:
case 0x9A1D:
case 0x9C1D:
case 0x9E1D:
case 0x901E:
case 0x921E:
case 0x941E:
case 0x961E:
case 0x981E:
case 0x9A1E:
case 0x9C1E:
case 0x9E1E:

// SUBaD
case 0x9018:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, src)
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x9220:
case 0x9420:
case 0x9620:
case 0x9820:
case 0x9A20:
case 0x9C20:
case 0x9E20:
case 0x9021:
case 0x9221:
case 0x9421:
case 0x9621:
case 0x9821:
case 0x9A21:
case 0x9C21:
case 0x9E21:
case 0x9022:
case 0x9222:
case 0x9422:
case 0x9622:
case 0x9822:
case 0x9A22:
case 0x9C22:
case 0x9E22:
case 0x9023:
case 0x9223:
case 0x9423:
case 0x9623:
case 0x9823:
case 0x9A23:
case 0x9C23:
case 0x9E23:
case 0x9024:
case 0x9224:
case 0x9424:
case 0x9624:
case 0x9824:
case 0x9A24:
case 0x9C24:
case 0x9E24:
case 0x9025:
case 0x9225:
case 0x9425:
case 0x9625:
case 0x9825:
case 0x9A25:
case 0x9C25:
case 0x9E25:
case 0x9026:
case 0x9226:
case 0x9426:
case 0x9626:
case 0x9826:
case 0x9A26:
case 0x9C26:
case 0x9E26:

// SUBaD
case 0x9020:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0x9228:
case 0x9428:
case 0x9628:
case 0x9828:
case 0x9A28:
case 0x9C28:
case 0x9E28:
case 0x9029:
case 0x9229:
case 0x9429:
case 0x9629:
case 0x9829:
case 0x9A29:
case 0x9C29:
case 0x9E29:
case 0x902A:
case 0x922A:
case 0x942A:
case 0x962A:
case 0x982A:
case 0x9A2A:
case 0x9C2A:
case 0x9E2A:
case 0x902B:
case 0x922B:
case 0x942B:
case 0x962B:
case 0x982B:
case 0x9A2B:
case 0x9C2B:
case 0x9E2B:
case 0x902C:
case 0x922C:
case 0x942C:
case 0x962C:
case 0x982C:
case 0x9A2C:
case 0x9C2C:
case 0x9E2C:
case 0x902D:
case 0x922D:
case 0x942D:
case 0x962D:
case 0x982D:
case 0x9A2D:
case 0x9C2D:
case 0x9E2D:
case 0x902E:
case 0x922E:
case 0x942E:
case 0x962E:
case 0x982E:
case 0x9A2E:
case 0x9C2E:
case 0x9E2E:
case 0x902F:
case 0x922F:
case 0x942F:
case 0x962F:
case 0x982F:
case 0x9A2F:
case 0x9C2F:
case 0x9E2F:

// SUBaD
case 0x9028:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x9230:
case 0x9430:
case 0x9630:
case 0x9830:
case 0x9A30:
case 0x9C30:
case 0x9E30:
case 0x9031:
case 0x9231:
case 0x9431:
case 0x9631:
case 0x9831:
case 0x9A31:
case 0x9C31:
case 0x9E31:
case 0x9032:
case 0x9232:
case 0x9432:
case 0x9632:
case 0x9832:
case 0x9A32:
case 0x9C32:
case 0x9E32:
case 0x9033:
case 0x9233:
case 0x9433:
case 0x9633:
case 0x9833:
case 0x9A33:
case 0x9C33:
case 0x9E33:
case 0x9034:
case 0x9234:
case 0x9434:
case 0x9634:
case 0x9834:
case 0x9A34:
case 0x9C34:
case 0x9E34:
case 0x9035:
case 0x9235:
case 0x9435:
case 0x9635:
case 0x9835:
case 0x9A35:
case 0x9C35:
case 0x9E35:
case 0x9036:
case 0x9236:
case 0x9436:
case 0x9636:
case 0x9836:
case 0x9A36:
case 0x9C36:
case 0x9E36:
case 0x9037:
case 0x9237:
case 0x9437:
case 0x9637:
case 0x9837:
case 0x9A37:
case 0x9C37:
case 0x9E37:

// SUBaD
case 0x9030:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, src)
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x9238:
case 0x9438:
case 0x9638:
case 0x9838:
case 0x9A38:
case 0x9C38:
case 0x9E38:

// SUBaD
case 0x9038:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x9239:
case 0x9439:
case 0x9639:
case 0x9839:
case 0x9A39:
case 0x9C39:
case 0x9E39:

// SUBaD
case 0x9039:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_BYTE_F(adr, src)
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0x923A:
case 0x943A:
case 0x963A:
case 0x983A:
case 0x9A3A:
case 0x9C3A:
case 0x9E3A:

// SUBaD
case 0x903A:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x923B:
case 0x943B:
case 0x963B:
case 0x983B:
case 0x9A3B:
case 0x9C3B:
case 0x9E3B:

// SUBaD
case 0x903B:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, src)
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x923C:
case 0x943C:
case 0x963C:
case 0x983C:
case 0x9A3C:
case 0x9C3C:
case 0x9E3C:

// SUBaD
case 0x903C:
{
	u32 res;
	u32 dst;
	u32 src;
	src = FETCH_BYTE;
	PC += 2;
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0x921F:
case 0x941F:
case 0x961F:
case 0x981F:
case 0x9A1F:
case 0x9C1F:
case 0x9E1F:

// SUBaD
case 0x901F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x9227:
case 0x9427:
case 0x9627:
case 0x9827:
case 0x9A27:
case 0x9C27:
case 0x9E27:

// SUBaD
case 0x9027:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0x9240:
case 0x9440:
case 0x9640:
case 0x9840:
case 0x9A40:
case 0x9C40:
case 0x9E40:
case 0x9041:
case 0x9241:
case 0x9441:
case 0x9641:
case 0x9841:
case 0x9A41:
case 0x9C41:
case 0x9E41:
case 0x9042:
case 0x9242:
case 0x9442:
case 0x9642:
case 0x9842:
case 0x9A42:
case 0x9C42:
case 0x9E42:
case 0x9043:
case 0x9243:
case 0x9443:
case 0x9643:
case 0x9843:
case 0x9A43:
case 0x9C43:
case 0x9E43:
case 0x9044:
case 0x9244:
case 0x9444:
case 0x9644:
case 0x9844:
case 0x9A44:
case 0x9C44:
case 0x9E44:
case 0x9045:
case 0x9245:
case 0x9445:
case 0x9645:
case 0x9845:
case 0x9A45:
case 0x9C45:
case 0x9E45:
case 0x9046:
case 0x9246:
case 0x9446:
case 0x9646:
case 0x9846:
case 0x9A46:
case 0x9C46:
case 0x9E46:
case 0x9047:
case 0x9247:
case 0x9447:
case 0x9647:
case 0x9847:
case 0x9A47:
case 0x9C47:
case 0x9E47:

// SUBaD
case 0x9040:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0x9248:
case 0x9448:
case 0x9648:
case 0x9848:
case 0x9A48:
case 0x9C48:
case 0x9E48:
case 0x9049:
case 0x9249:
case 0x9449:
case 0x9649:
case 0x9849:
case 0x9A49:
case 0x9C49:
case 0x9E49:
case 0x904A:
case 0x924A:
case 0x944A:
case 0x964A:
case 0x984A:
case 0x9A4A:
case 0x9C4A:
case 0x9E4A:
case 0x904B:
case 0x924B:
case 0x944B:
case 0x964B:
case 0x984B:
case 0x9A4B:
case 0x9C4B:
case 0x9E4B:
case 0x904C:
case 0x924C:
case 0x944C:
case 0x964C:
case 0x984C:
case 0x9A4C:
case 0x9C4C:
case 0x9E4C:
case 0x904D:
case 0x924D:
case 0x944D:
case 0x964D:
case 0x984D:
case 0x9A4D:
case 0x9C4D:
case 0x9E4D:
case 0x904E:
case 0x924E:
case 0x944E:
case 0x964E:
case 0x984E:
case 0x9A4E:
case 0x9C4E:
case 0x9E4E:
case 0x904F:
case 0x924F:
case 0x944F:
case 0x964F:
case 0x984F:
case 0x9A4F:
case 0x9C4F:
case 0x9E4F:

// SUBaD
case 0x9048:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->A[(Opcode >> 0) & 7];
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0x9250:
case 0x9450:
case 0x9650:
case 0x9850:
case 0x9A50:
case 0x9C50:
case 0x9E50:
case 0x9051:
case 0x9251:
case 0x9451:
case 0x9651:
case 0x9851:
case 0x9A51:
case 0x9C51:
case 0x9E51:
case 0x9052:
case 0x9252:
case 0x9452:
case 0x9652:
case 0x9852:
case 0x9A52:
case 0x9C52:
case 0x9E52:
case 0x9053:
case 0x9253:
case 0x9453:
case 0x9653:
case 0x9853:
case 0x9A53:
case 0x9C53:
case 0x9E53:
case 0x9054:
case 0x9254:
case 0x9454:
case 0x9654:
case 0x9854:
case 0x9A54:
case 0x9C54:
case 0x9E54:
case 0x9055:
case 0x9255:
case 0x9455:
case 0x9655:
case 0x9855:
case 0x9A55:
case 0x9C55:
case 0x9E55:
case 0x9056:
case 0x9256:
case 0x9456:
case 0x9656:
case 0x9856:
case 0x9A56:
case 0x9C56:
case 0x9E56:
case 0x9057:
case 0x9257:
case 0x9457:
case 0x9657:
case 0x9857:
case 0x9A57:
case 0x9C57:
case 0x9E57:

// SUBaD
case 0x9050:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x9258:
case 0x9458:
case 0x9658:
case 0x9858:
case 0x9A58:
case 0x9C58:
case 0x9E58:
case 0x9059:
case 0x9259:
case 0x9459:
case 0x9659:
case 0x9859:
case 0x9A59:
case 0x9C59:
case 0x9E59:
case 0x905A:
case 0x925A:
case 0x945A:
case 0x965A:
case 0x985A:
case 0x9A5A:
case 0x9C5A:
case 0x9E5A:
case 0x905B:
case 0x925B:
case 0x945B:
case 0x965B:
case 0x985B:
case 0x9A5B:
case 0x9C5B:
case 0x9E5B:
case 0x905C:
case 0x925C:
case 0x945C:
case 0x965C:
case 0x985C:
case 0x9A5C:
case 0x9C5C:
case 0x9E5C:
case 0x905D:
case 0x925D:
case 0x945D:
case 0x965D:
case 0x985D:
case 0x9A5D:
case 0x9C5D:
case 0x9E5D:
case 0x905E:
case 0x925E:
case 0x945E:
case 0x965E:
case 0x985E:
case 0x9A5E:
case 0x9C5E:
case 0x9E5E:

// SUBaD
case 0x9058:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x9260:
case 0x9460:
case 0x9660:
case 0x9860:
case 0x9A60:
case 0x9C60:
case 0x9E60:
case 0x9061:
case 0x9261:
case 0x9461:
case 0x9661:
case 0x9861:
case 0x9A61:
case 0x9C61:
case 0x9E61:
case 0x9062:
case 0x9262:
case 0x9462:
case 0x9662:
case 0x9862:
case 0x9A62:
case 0x9C62:
case 0x9E62:
case 0x9063:
case 0x9263:
case 0x9463:
case 0x9663:
case 0x9863:
case 0x9A63:
case 0x9C63:
case 0x9E63:
case 0x9064:
case 0x9264:
case 0x9464:
case 0x9664:
case 0x9864:
case 0x9A64:
case 0x9C64:
case 0x9E64:
case 0x9065:
case 0x9265:
case 0x9465:
case 0x9665:
case 0x9865:
case 0x9A65:
case 0x9C65:
case 0x9E65:
case 0x9066:
case 0x9266:
case 0x9466:
case 0x9666:
case 0x9866:
case 0x9A66:
case 0x9C66:
case 0x9E66:

// SUBaD
case 0x9060:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0x9268:
case 0x9468:
case 0x9668:
case 0x9868:
case 0x9A68:
case 0x9C68:
case 0x9E68:
case 0x9069:
case 0x9269:
case 0x9469:
case 0x9669:
case 0x9869:
case 0x9A69:
case 0x9C69:
case 0x9E69:
case 0x906A:
case 0x926A:
case 0x946A:
case 0x966A:
case 0x986A:
case 0x9A6A:
case 0x9C6A:
case 0x9E6A:
case 0x906B:
case 0x926B:
case 0x946B:
case 0x966B:
case 0x986B:
case 0x9A6B:
case 0x9C6B:
case 0x9E6B:
case 0x906C:
case 0x926C:
case 0x946C:
case 0x966C:
case 0x986C:
case 0x9A6C:
case 0x9C6C:
case 0x9E6C:
case 0x906D:
case 0x926D:
case 0x946D:
case 0x966D:
case 0x986D:
case 0x9A6D:
case 0x9C6D:
case 0x9E6D:
case 0x906E:
case 0x926E:
case 0x946E:
case 0x966E:
case 0x986E:
case 0x9A6E:
case 0x9C6E:
case 0x9E6E:
case 0x906F:
case 0x926F:
case 0x946F:
case 0x966F:
case 0x986F:
case 0x9A6F:
case 0x9C6F:
case 0x9E6F:

// SUBaD
case 0x9068:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x9270:
case 0x9470:
case 0x9670:
case 0x9870:
case 0x9A70:
case 0x9C70:
case 0x9E70:
case 0x9071:
case 0x9271:
case 0x9471:
case 0x9671:
case 0x9871:
case 0x9A71:
case 0x9C71:
case 0x9E71:
case 0x9072:
case 0x9272:
case 0x9472:
case 0x9672:
case 0x9872:
case 0x9A72:
case 0x9C72:
case 0x9E72:
case 0x9073:
case 0x9273:
case 0x9473:
case 0x9673:
case 0x9873:
case 0x9A73:
case 0x9C73:
case 0x9E73:
case 0x9074:
case 0x9274:
case 0x9474:
case 0x9674:
case 0x9874:
case 0x9A74:
case 0x9C74:
case 0x9E74:
case 0x9075:
case 0x9275:
case 0x9475:
case 0x9675:
case 0x9875:
case 0x9A75:
case 0x9C75:
case 0x9E75:
case 0x9076:
case 0x9276:
case 0x9476:
case 0x9676:
case 0x9876:
case 0x9A76:
case 0x9C76:
case 0x9E76:
case 0x9077:
case 0x9277:
case 0x9477:
case 0x9677:
case 0x9877:
case 0x9A77:
case 0x9C77:
case 0x9E77:

// SUBaD
case 0x9070:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x9278:
case 0x9478:
case 0x9678:
case 0x9878:
case 0x9A78:
case 0x9C78:
case 0x9E78:

// SUBaD
case 0x9078:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x9279:
case 0x9479:
case 0x9679:
case 0x9879:
case 0x9A79:
case 0x9C79:
case 0x9E79:

// SUBaD
case 0x9079:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0x927A:
case 0x947A:
case 0x967A:
case 0x987A:
case 0x9A7A:
case 0x9C7A:
case 0x9E7A:

// SUBaD
case 0x907A:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x927B:
case 0x947B:
case 0x967B:
case 0x987B:
case 0x9A7B:
case 0x9C7B:
case 0x9E7B:

// SUBaD
case 0x907B:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x927C:
case 0x947C:
case 0x967C:
case 0x987C:
case 0x9A7C:
case 0x9C7C:
case 0x9E7C:

// SUBaD
case 0x907C:
{
	u32 res;
	u32 dst;
	u32 src;
	src = FETCH_WORD;
	PC += 2;
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0x925F:
case 0x945F:
case 0x965F:
case 0x985F:
case 0x9A5F:
case 0x9C5F:
case 0x9E5F:

// SUBaD
case 0x905F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x9267:
case 0x9467:
case 0x9667:
case 0x9867:
case 0x9A67:
case 0x9C67:
case 0x9E67:

// SUBaD
case 0x9067:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0x9280:
case 0x9480:
case 0x9680:
case 0x9880:
case 0x9A80:
case 0x9C80:
case 0x9E80:
case 0x9081:
case 0x9281:
case 0x9481:
case 0x9681:
case 0x9881:
case 0x9A81:
case 0x9C81:
case 0x9E81:
case 0x9082:
case 0x9282:
case 0x9482:
case 0x9682:
case 0x9882:
case 0x9A82:
case 0x9C82:
case 0x9E82:
case 0x9083:
case 0x9283:
case 0x9483:
case 0x9683:
case 0x9883:
case 0x9A83:
case 0x9C83:
case 0x9E83:
case 0x9084:
case 0x9284:
case 0x9484:
case 0x9684:
case 0x9884:
case 0x9A84:
case 0x9C84:
case 0x9E84:
case 0x9085:
case 0x9285:
case 0x9485:
case 0x9685:
case 0x9885:
case 0x9A85:
case 0x9C85:
case 0x9E85:
case 0x9086:
case 0x9286:
case 0x9486:
case 0x9686:
case 0x9886:
case 0x9A86:
case 0x9C86:
case 0x9E86:
case 0x9087:
case 0x9287:
case 0x9487:
case 0x9687:
case 0x9887:
case 0x9A87:
case 0x9C87:
case 0x9E87:

// SUBaD
case 0x9080:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0x9288:
case 0x9488:
case 0x9688:
case 0x9888:
case 0x9A88:
case 0x9C88:
case 0x9E88:
case 0x9089:
case 0x9289:
case 0x9489:
case 0x9689:
case 0x9889:
case 0x9A89:
case 0x9C89:
case 0x9E89:
case 0x908A:
case 0x928A:
case 0x948A:
case 0x968A:
case 0x988A:
case 0x9A8A:
case 0x9C8A:
case 0x9E8A:
case 0x908B:
case 0x928B:
case 0x948B:
case 0x968B:
case 0x988B:
case 0x9A8B:
case 0x9C8B:
case 0x9E8B:
case 0x908C:
case 0x928C:
case 0x948C:
case 0x968C:
case 0x988C:
case 0x9A8C:
case 0x9C8C:
case 0x9E8C:
case 0x908D:
case 0x928D:
case 0x948D:
case 0x968D:
case 0x988D:
case 0x9A8D:
case 0x9C8D:
case 0x9E8D:
case 0x908E:
case 0x928E:
case 0x948E:
case 0x968E:
case 0x988E:
case 0x9A8E:
case 0x9C8E:
case 0x9E8E:
case 0x908F:
case 0x928F:
case 0x948F:
case 0x968F:
case 0x988F:
case 0x9A8F:
case 0x9C8F:
case 0x9E8F:

// SUBaD
case 0x9088:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->A[(Opcode >> 0) & 7];
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0x9290:
case 0x9490:
case 0x9690:
case 0x9890:
case 0x9A90:
case 0x9C90:
case 0x9E90:
case 0x9091:
case 0x9291:
case 0x9491:
case 0x9691:
case 0x9891:
case 0x9A91:
case 0x9C91:
case 0x9E91:
case 0x9092:
case 0x9292:
case 0x9492:
case 0x9692:
case 0x9892:
case 0x9A92:
case 0x9C92:
case 0x9E92:
case 0x9093:
case 0x9293:
case 0x9493:
case 0x9693:
case 0x9893:
case 0x9A93:
case 0x9C93:
case 0x9E93:
case 0x9094:
case 0x9294:
case 0x9494:
case 0x9694:
case 0x9894:
case 0x9A94:
case 0x9C94:
case 0x9E94:
case 0x9095:
case 0x9295:
case 0x9495:
case 0x9695:
case 0x9895:
case 0x9A95:
case 0x9C95:
case 0x9E95:
case 0x9096:
case 0x9296:
case 0x9496:
case 0x9696:
case 0x9896:
case 0x9A96:
case 0x9C96:
case 0x9E96:
case 0x9097:
case 0x9297:
case 0x9497:
case 0x9697:
case 0x9897:
case 0x9A97:
case 0x9C97:
case 0x9E97:

// SUBaD
case 0x9090:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_LONG_F(adr, src)
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x9298:
case 0x9498:
case 0x9698:
case 0x9898:
case 0x9A98:
case 0x9C98:
case 0x9E98:
case 0x9099:
case 0x9299:
case 0x9499:
case 0x9699:
case 0x9899:
case 0x9A99:
case 0x9C99:
case 0x9E99:
case 0x909A:
case 0x929A:
case 0x949A:
case 0x969A:
case 0x989A:
case 0x9A9A:
case 0x9C9A:
case 0x9E9A:
case 0x909B:
case 0x929B:
case 0x949B:
case 0x969B:
case 0x989B:
case 0x9A9B:
case 0x9C9B:
case 0x9E9B:
case 0x909C:
case 0x929C:
case 0x949C:
case 0x969C:
case 0x989C:
case 0x9A9C:
case 0x9C9C:
case 0x9E9C:
case 0x909D:
case 0x929D:
case 0x949D:
case 0x969D:
case 0x989D:
case 0x9A9D:
case 0x9C9D:
case 0x9E9D:
case 0x909E:
case 0x929E:
case 0x949E:
case 0x969E:
case 0x989E:
case 0x9A9E:
case 0x9C9E:
case 0x9E9E:

// SUBaD
case 0x9098:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x92A0:
case 0x94A0:
case 0x96A0:
case 0x98A0:
case 0x9AA0:
case 0x9CA0:
case 0x9EA0:
case 0x90A1:
case 0x92A1:
case 0x94A1:
case 0x96A1:
case 0x98A1:
case 0x9AA1:
case 0x9CA1:
case 0x9EA1:
case 0x90A2:
case 0x92A2:
case 0x94A2:
case 0x96A2:
case 0x98A2:
case 0x9AA2:
case 0x9CA2:
case 0x9EA2:
case 0x90A3:
case 0x92A3:
case 0x94A3:
case 0x96A3:
case 0x98A3:
case 0x9AA3:
case 0x9CA3:
case 0x9EA3:
case 0x90A4:
case 0x92A4:
case 0x94A4:
case 0x96A4:
case 0x98A4:
case 0x9AA4:
case 0x9CA4:
case 0x9EA4:
case 0x90A5:
case 0x92A5:
case 0x94A5:
case 0x96A5:
case 0x98A5:
case 0x9AA5:
case 0x9CA5:
case 0x9EA5:
case 0x90A6:
case 0x92A6:
case 0x94A6:
case 0x96A6:
case 0x98A6:
case 0x9AA6:
case 0x9CA6:
case 0x9EA6:

// SUBaD
case 0x90A0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0x92A8:
case 0x94A8:
case 0x96A8:
case 0x98A8:
case 0x9AA8:
case 0x9CA8:
case 0x9EA8:
case 0x90A9:
case 0x92A9:
case 0x94A9:
case 0x96A9:
case 0x98A9:
case 0x9AA9:
case 0x9CA9:
case 0x9EA9:
case 0x90AA:
case 0x92AA:
case 0x94AA:
case 0x96AA:
case 0x98AA:
case 0x9AAA:
case 0x9CAA:
case 0x9EAA:
case 0x90AB:
case 0x92AB:
case 0x94AB:
case 0x96AB:
case 0x98AB:
case 0x9AAB:
case 0x9CAB:
case 0x9EAB:
case 0x90AC:
case 0x92AC:
case 0x94AC:
case 0x96AC:
case 0x98AC:
case 0x9AAC:
case 0x9CAC:
case 0x9EAC:
case 0x90AD:
case 0x92AD:
case 0x94AD:
case 0x96AD:
case 0x98AD:
case 0x9AAD:
case 0x9CAD:
case 0x9EAD:
case 0x90AE:
case 0x92AE:
case 0x94AE:
case 0x96AE:
case 0x98AE:
case 0x9AAE:
case 0x9CAE:
case 0x9EAE:
case 0x90AF:
case 0x92AF:
case 0x94AF:
case 0x96AF:
case 0x98AF:
case 0x9AAF:
case 0x9CAF:
case 0x9EAF:

// SUBaD
case 0x90A8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(18)
case 0x92B0:
case 0x94B0:
case 0x96B0:
case 0x98B0:
case 0x9AB0:
case 0x9CB0:
case 0x9EB0:
case 0x90B1:
case 0x92B1:
case 0x94B1:
case 0x96B1:
case 0x98B1:
case 0x9AB1:
case 0x9CB1:
case 0x9EB1:
case 0x90B2:
case 0x92B2:
case 0x94B2:
case 0x96B2:
case 0x98B2:
case 0x9AB2:
case 0x9CB2:
case 0x9EB2:
case 0x90B3:
case 0x92B3:
case 0x94B3:
case 0x96B3:
case 0x98B3:
case 0x9AB3:
case 0x9CB3:
case 0x9EB3:
case 0x90B4:
case 0x92B4:
case 0x94B4:
case 0x96B4:
case 0x98B4:
case 0x9AB4:
case 0x9CB4:
case 0x9EB4:
case 0x90B5:
case 0x92B5:
case 0x94B5:
case 0x96B5:
case 0x98B5:
case 0x9AB5:
case 0x9CB5:
case 0x9EB5:
case 0x90B6:
case 0x92B6:
case 0x94B6:
case 0x96B6:
case 0x98B6:
case 0x9AB6:
case 0x9CB6:
case 0x9EB6:
case 0x90B7:
case 0x92B7:
case 0x94B7:
case 0x96B7:
case 0x98B7:
case 0x9AB7:
case 0x9CB7:
case 0x9EB7:

// SUBaD
case 0x90B0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, src)
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(20)
case 0x92B8:
case 0x94B8:
case 0x96B8:
case 0x98B8:
case 0x9AB8:
case 0x9CB8:
case 0x9EB8:

// SUBaD
case 0x90B8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(18)
case 0x92B9:
case 0x94B9:
case 0x96B9:
case 0x98B9:
case 0x9AB9:
case 0x9CB9:
case 0x9EB9:

// SUBaD
case 0x90B9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(22)
case 0x92BA:
case 0x94BA:
case 0x96BA:
case 0x98BA:
case 0x9ABA:
case 0x9CBA:
case 0x9EBA:

// SUBaD
case 0x90BA:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(18)
case 0x92BB:
case 0x94BB:
case 0x96BB:
case 0x98BB:
case 0x9ABB:
case 0x9CBB:
case 0x9EBB:

// SUBaD
case 0x90BB:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, src)
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(20)
case 0x92BC:
case 0x94BC:
case 0x96BC:
case 0x98BC:
case 0x9ABC:
case 0x9CBC:
case 0x9EBC:

// SUBaD
case 0x90BC:
{
	u32 res;
	u32 dst;
	u32 src;
	src = FETCH_LONG;
	PC += 4;
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(16)
case 0x929F:
case 0x949F:
case 0x969F:
case 0x989F:
case 0x9A9F:
case 0x9C9F:
case 0x9E9F:

// SUBaD
case 0x909F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x92A7:
case 0x94A7:
case 0x96A7:
case 0x98A7:
case 0x9AA7:
case 0x9CA7:
case 0x9EA7:

// SUBaD
case 0x90A7:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0x9310:
case 0x9510:
case 0x9710:
case 0x9910:
case 0x9B10:
case 0x9D10:
case 0x9F10:
case 0x9111:
case 0x9311:
case 0x9511:
case 0x9711:
case 0x9911:
case 0x9B11:
case 0x9D11:
case 0x9F11:
case 0x9112:
case 0x9312:
case 0x9512:
case 0x9712:
case 0x9912:
case 0x9B12:
case 0x9D12:
case 0x9F12:
case 0x9113:
case 0x9313:
case 0x9513:
case 0x9713:
case 0x9913:
case 0x9B13:
case 0x9D13:
case 0x9F13:
case 0x9114:
case 0x9314:
case 0x9514:
case 0x9714:
case 0x9914:
case 0x9B14:
case 0x9D14:
case 0x9F14:
case 0x9115:
case 0x9315:
case 0x9515:
case 0x9715:
case 0x9915:
case 0x9B15:
case 0x9D15:
case 0x9F15:
case 0x9116:
case 0x9316:
case 0x9516:
case 0x9716:
case 0x9916:
case 0x9B16:
case 0x9D16:
case 0x9F16:
case 0x9117:
case 0x9317:
case 0x9517:
case 0x9717:
case 0x9917:
case 0x9B17:
case 0x9D17:
case 0x9F17:

// SUBDa
case 0x9110:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
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
case 0x9318:
case 0x9518:
case 0x9718:
case 0x9918:
case 0x9B18:
case 0x9D18:
case 0x9F18:
case 0x9119:
case 0x9319:
case 0x9519:
case 0x9719:
case 0x9919:
case 0x9B19:
case 0x9D19:
case 0x9F19:
case 0x911A:
case 0x931A:
case 0x951A:
case 0x971A:
case 0x991A:
case 0x9B1A:
case 0x9D1A:
case 0x9F1A:
case 0x911B:
case 0x931B:
case 0x951B:
case 0x971B:
case 0x991B:
case 0x9B1B:
case 0x9D1B:
case 0x9F1B:
case 0x911C:
case 0x931C:
case 0x951C:
case 0x971C:
case 0x991C:
case 0x9B1C:
case 0x9D1C:
case 0x9F1C:
case 0x911D:
case 0x931D:
case 0x951D:
case 0x971D:
case 0x991D:
case 0x9B1D:
case 0x9D1D:
case 0x9F1D:
case 0x911E:
case 0x931E:
case 0x951E:
case 0x971E:
case 0x991E:
case 0x9B1E:
case 0x9D1E:
case 0x9F1E:

// SUBDa
case 0x9118:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
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
case 0x9320:
case 0x9520:
case 0x9720:
case 0x9920:
case 0x9B20:
case 0x9D20:
case 0x9F20:
case 0x9121:
case 0x9321:
case 0x9521:
case 0x9721:
case 0x9921:
case 0x9B21:
case 0x9D21:
case 0x9F21:
case 0x9122:
case 0x9322:
case 0x9522:
case 0x9722:
case 0x9922:
case 0x9B22:
case 0x9D22:
case 0x9F22:
case 0x9123:
case 0x9323:
case 0x9523:
case 0x9723:
case 0x9923:
case 0x9B23:
case 0x9D23:
case 0x9F23:
case 0x9124:
case 0x9324:
case 0x9524:
case 0x9724:
case 0x9924:
case 0x9B24:
case 0x9D24:
case 0x9F24:
case 0x9125:
case 0x9325:
case 0x9525:
case 0x9725:
case 0x9925:
case 0x9B25:
case 0x9D25:
case 0x9F25:
case 0x9126:
case 0x9326:
case 0x9526:
case 0x9726:
case 0x9926:
case 0x9B26:
case 0x9D26:
case 0x9F26:

// SUBDa
case 0x9120:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
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
case 0x9328:
case 0x9528:
case 0x9728:
case 0x9928:
case 0x9B28:
case 0x9D28:
case 0x9F28:
case 0x9129:
case 0x9329:
case 0x9529:
case 0x9729:
case 0x9929:
case 0x9B29:
case 0x9D29:
case 0x9F29:
case 0x912A:
case 0x932A:
case 0x952A:
case 0x972A:
case 0x992A:
case 0x9B2A:
case 0x9D2A:
case 0x9F2A:
case 0x912B:
case 0x932B:
case 0x952B:
case 0x972B:
case 0x992B:
case 0x9B2B:
case 0x9D2B:
case 0x9F2B:
case 0x912C:
case 0x932C:
case 0x952C:
case 0x972C:
case 0x992C:
case 0x9B2C:
case 0x9D2C:
case 0x9F2C:
case 0x912D:
case 0x932D:
case 0x952D:
case 0x972D:
case 0x992D:
case 0x9B2D:
case 0x9D2D:
case 0x9F2D:
case 0x912E:
case 0x932E:
case 0x952E:
case 0x972E:
case 0x992E:
case 0x9B2E:
case 0x9D2E:
case 0x9F2E:
case 0x912F:
case 0x932F:
case 0x952F:
case 0x972F:
case 0x992F:
case 0x9B2F:
case 0x9D2F:
case 0x9F2F:

// SUBDa
case 0x9128:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
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
case 0x9330:
case 0x9530:
case 0x9730:
case 0x9930:
case 0x9B30:
case 0x9D30:
case 0x9F30:
case 0x9131:
case 0x9331:
case 0x9531:
case 0x9731:
case 0x9931:
case 0x9B31:
case 0x9D31:
case 0x9F31:
case 0x9132:
case 0x9332:
case 0x9532:
case 0x9732:
case 0x9932:
case 0x9B32:
case 0x9D32:
case 0x9F32:
case 0x9133:
case 0x9333:
case 0x9533:
case 0x9733:
case 0x9933:
case 0x9B33:
case 0x9D33:
case 0x9F33:
case 0x9134:
case 0x9334:
case 0x9534:
case 0x9734:
case 0x9934:
case 0x9B34:
case 0x9D34:
case 0x9F34:
case 0x9135:
case 0x9335:
case 0x9535:
case 0x9735:
case 0x9935:
case 0x9B35:
case 0x9D35:
case 0x9F35:
case 0x9136:
case 0x9336:
case 0x9536:
case 0x9736:
case 0x9936:
case 0x9B36:
case 0x9D36:
case 0x9F36:
case 0x9137:
case 0x9337:
case 0x9537:
case 0x9737:
case 0x9937:
case 0x9B37:
case 0x9D37:
case 0x9F37:

// SUBDa
case 0x9130:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
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
case 0x9338:
case 0x9538:
case 0x9738:
case 0x9938:
case 0x9B38:
case 0x9D38:
case 0x9F38:

// SUBDa
case 0x9138:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
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
case 0x9339:
case 0x9539:
case 0x9739:
case 0x9939:
case 0x9B39:
case 0x9D39:
case 0x9F39:

// SUBDa
case 0x9139:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
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
case 0x931F:
case 0x951F:
case 0x971F:
case 0x991F:
case 0x9B1F:
case 0x9D1F:
case 0x9F1F:

// SUBDa
case 0x911F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
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
case 0x9327:
case 0x9527:
case 0x9727:
case 0x9927:
case 0x9B27:
case 0x9D27:
case 0x9F27:

// SUBDa
case 0x9127:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
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
case 0x9350:
case 0x9550:
case 0x9750:
case 0x9950:
case 0x9B50:
case 0x9D50:
case 0x9F50:
case 0x9151:
case 0x9351:
case 0x9551:
case 0x9751:
case 0x9951:
case 0x9B51:
case 0x9D51:
case 0x9F51:
case 0x9152:
case 0x9352:
case 0x9552:
case 0x9752:
case 0x9952:
case 0x9B52:
case 0x9D52:
case 0x9F52:
case 0x9153:
case 0x9353:
case 0x9553:
case 0x9753:
case 0x9953:
case 0x9B53:
case 0x9D53:
case 0x9F53:
case 0x9154:
case 0x9354:
case 0x9554:
case 0x9754:
case 0x9954:
case 0x9B54:
case 0x9D54:
case 0x9F54:
case 0x9155:
case 0x9355:
case 0x9555:
case 0x9755:
case 0x9955:
case 0x9B55:
case 0x9D55:
case 0x9F55:
case 0x9156:
case 0x9356:
case 0x9556:
case 0x9756:
case 0x9956:
case 0x9B56:
case 0x9D56:
case 0x9F56:
case 0x9157:
case 0x9357:
case 0x9557:
case 0x9757:
case 0x9957:
case 0x9B57:
case 0x9D57:
case 0x9F57:

// SUBDa
case 0x9150:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
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
case 0x9358:
case 0x9558:
case 0x9758:
case 0x9958:
case 0x9B58:
case 0x9D58:
case 0x9F58:
case 0x9159:
case 0x9359:
case 0x9559:
case 0x9759:
case 0x9959:
case 0x9B59:
case 0x9D59:
case 0x9F59:
case 0x915A:
case 0x935A:
case 0x955A:
case 0x975A:
case 0x995A:
case 0x9B5A:
case 0x9D5A:
case 0x9F5A:
case 0x915B:
case 0x935B:
case 0x955B:
case 0x975B:
case 0x995B:
case 0x9B5B:
case 0x9D5B:
case 0x9F5B:
case 0x915C:
case 0x935C:
case 0x955C:
case 0x975C:
case 0x995C:
case 0x9B5C:
case 0x9D5C:
case 0x9F5C:
case 0x915D:
case 0x935D:
case 0x955D:
case 0x975D:
case 0x995D:
case 0x9B5D:
case 0x9D5D:
case 0x9F5D:
case 0x915E:
case 0x935E:
case 0x955E:
case 0x975E:
case 0x995E:
case 0x9B5E:
case 0x9D5E:
case 0x9F5E:

// SUBDa
case 0x9158:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
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
case 0x9360:
case 0x9560:
case 0x9760:
case 0x9960:
case 0x9B60:
case 0x9D60:
case 0x9F60:
case 0x9161:
case 0x9361:
case 0x9561:
case 0x9761:
case 0x9961:
case 0x9B61:
case 0x9D61:
case 0x9F61:
case 0x9162:
case 0x9362:
case 0x9562:
case 0x9762:
case 0x9962:
case 0x9B62:
case 0x9D62:
case 0x9F62:
case 0x9163:
case 0x9363:
case 0x9563:
case 0x9763:
case 0x9963:
case 0x9B63:
case 0x9D63:
case 0x9F63:
case 0x9164:
case 0x9364:
case 0x9564:
case 0x9764:
case 0x9964:
case 0x9B64:
case 0x9D64:
case 0x9F64:
case 0x9165:
case 0x9365:
case 0x9565:
case 0x9765:
case 0x9965:
case 0x9B65:
case 0x9D65:
case 0x9F65:
case 0x9166:
case 0x9366:
case 0x9566:
case 0x9766:
case 0x9966:
case 0x9B66:
case 0x9D66:
case 0x9F66:

// SUBDa
case 0x9160:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
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
case 0x9368:
case 0x9568:
case 0x9768:
case 0x9968:
case 0x9B68:
case 0x9D68:
case 0x9F68:
case 0x9169:
case 0x9369:
case 0x9569:
case 0x9769:
case 0x9969:
case 0x9B69:
case 0x9D69:
case 0x9F69:
case 0x916A:
case 0x936A:
case 0x956A:
case 0x976A:
case 0x996A:
case 0x9B6A:
case 0x9D6A:
case 0x9F6A:
case 0x916B:
case 0x936B:
case 0x956B:
case 0x976B:
case 0x996B:
case 0x9B6B:
case 0x9D6B:
case 0x9F6B:
case 0x916C:
case 0x936C:
case 0x956C:
case 0x976C:
case 0x996C:
case 0x9B6C:
case 0x9D6C:
case 0x9F6C:
case 0x916D:
case 0x936D:
case 0x956D:
case 0x976D:
case 0x996D:
case 0x9B6D:
case 0x9D6D:
case 0x9F6D:
case 0x916E:
case 0x936E:
case 0x956E:
case 0x976E:
case 0x996E:
case 0x9B6E:
case 0x9D6E:
case 0x9F6E:
case 0x916F:
case 0x936F:
case 0x956F:
case 0x976F:
case 0x996F:
case 0x9B6F:
case 0x9D6F:
case 0x9F6F:

// SUBDa
case 0x9168:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
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
case 0x9370:
case 0x9570:
case 0x9770:
case 0x9970:
case 0x9B70:
case 0x9D70:
case 0x9F70:
case 0x9171:
case 0x9371:
case 0x9571:
case 0x9771:
case 0x9971:
case 0x9B71:
case 0x9D71:
case 0x9F71:
case 0x9172:
case 0x9372:
case 0x9572:
case 0x9772:
case 0x9972:
case 0x9B72:
case 0x9D72:
case 0x9F72:
case 0x9173:
case 0x9373:
case 0x9573:
case 0x9773:
case 0x9973:
case 0x9B73:
case 0x9D73:
case 0x9F73:
case 0x9174:
case 0x9374:
case 0x9574:
case 0x9774:
case 0x9974:
case 0x9B74:
case 0x9D74:
case 0x9F74:
case 0x9175:
case 0x9375:
case 0x9575:
case 0x9775:
case 0x9975:
case 0x9B75:
case 0x9D75:
case 0x9F75:
case 0x9176:
case 0x9376:
case 0x9576:
case 0x9776:
case 0x9976:
case 0x9B76:
case 0x9D76:
case 0x9F76:
case 0x9177:
case 0x9377:
case 0x9577:
case 0x9777:
case 0x9977:
case 0x9B77:
case 0x9D77:
case 0x9F77:

// SUBDa
case 0x9170:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
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
case 0x9378:
case 0x9578:
case 0x9778:
case 0x9978:
case 0x9B78:
case 0x9D78:
case 0x9F78:

// SUBDa
case 0x9178:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
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
case 0x9379:
case 0x9579:
case 0x9779:
case 0x9979:
case 0x9B79:
case 0x9D79:
case 0x9F79:

// SUBDa
case 0x9179:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
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
case 0x935F:
case 0x955F:
case 0x975F:
case 0x995F:
case 0x9B5F:
case 0x9D5F:
case 0x9F5F:

// SUBDa
case 0x915F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
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
case 0x9367:
case 0x9567:
case 0x9767:
case 0x9967:
case 0x9B67:
case 0x9D67:
case 0x9F67:

// SUBDa
case 0x9167:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
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
case 0x9390:
case 0x9590:
case 0x9790:
case 0x9990:
case 0x9B90:
case 0x9D90:
case 0x9F90:
case 0x9191:
case 0x9391:
case 0x9591:
case 0x9791:
case 0x9991:
case 0x9B91:
case 0x9D91:
case 0x9F91:
case 0x9192:
case 0x9392:
case 0x9592:
case 0x9792:
case 0x9992:
case 0x9B92:
case 0x9D92:
case 0x9F92:
case 0x9193:
case 0x9393:
case 0x9593:
case 0x9793:
case 0x9993:
case 0x9B93:
case 0x9D93:
case 0x9F93:
case 0x9194:
case 0x9394:
case 0x9594:
case 0x9794:
case 0x9994:
case 0x9B94:
case 0x9D94:
case 0x9F94:
case 0x9195:
case 0x9395:
case 0x9595:
case 0x9795:
case 0x9995:
case 0x9B95:
case 0x9D95:
case 0x9F95:
case 0x9196:
case 0x9396:
case 0x9596:
case 0x9796:
case 0x9996:
case 0x9B96:
case 0x9D96:
case 0x9F96:
case 0x9197:
case 0x9397:
case 0x9597:
case 0x9797:
case 0x9997:
case 0x9B97:
case 0x9D97:
case 0x9F97:

// SUBDa
case 0x9190:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
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
case 0x9398:
case 0x9598:
case 0x9798:
case 0x9998:
case 0x9B98:
case 0x9D98:
case 0x9F98:
case 0x9199:
case 0x9399:
case 0x9599:
case 0x9799:
case 0x9999:
case 0x9B99:
case 0x9D99:
case 0x9F99:
case 0x919A:
case 0x939A:
case 0x959A:
case 0x979A:
case 0x999A:
case 0x9B9A:
case 0x9D9A:
case 0x9F9A:
case 0x919B:
case 0x939B:
case 0x959B:
case 0x979B:
case 0x999B:
case 0x9B9B:
case 0x9D9B:
case 0x9F9B:
case 0x919C:
case 0x939C:
case 0x959C:
case 0x979C:
case 0x999C:
case 0x9B9C:
case 0x9D9C:
case 0x9F9C:
case 0x919D:
case 0x939D:
case 0x959D:
case 0x979D:
case 0x999D:
case 0x9B9D:
case 0x9D9D:
case 0x9F9D:
case 0x919E:
case 0x939E:
case 0x959E:
case 0x979E:
case 0x999E:
case 0x9B9E:
case 0x9D9E:
case 0x9F9E:

// SUBDa
case 0x9198:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
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
case 0x93A0:
case 0x95A0:
case 0x97A0:
case 0x99A0:
case 0x9BA0:
case 0x9DA0:
case 0x9FA0:
case 0x91A1:
case 0x93A1:
case 0x95A1:
case 0x97A1:
case 0x99A1:
case 0x9BA1:
case 0x9DA1:
case 0x9FA1:
case 0x91A2:
case 0x93A2:
case 0x95A2:
case 0x97A2:
case 0x99A2:
case 0x9BA2:
case 0x9DA2:
case 0x9FA2:
case 0x91A3:
case 0x93A3:
case 0x95A3:
case 0x97A3:
case 0x99A3:
case 0x9BA3:
case 0x9DA3:
case 0x9FA3:
case 0x91A4:
case 0x93A4:
case 0x95A4:
case 0x97A4:
case 0x99A4:
case 0x9BA4:
case 0x9DA4:
case 0x9FA4:
case 0x91A5:
case 0x93A5:
case 0x95A5:
case 0x97A5:
case 0x99A5:
case 0x9BA5:
case 0x9DA5:
case 0x9FA5:
case 0x91A6:
case 0x93A6:
case 0x95A6:
case 0x97A6:
case 0x99A6:
case 0x9BA6:
case 0x9DA6:
case 0x9FA6:

// SUBDa
case 0x91A0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
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
case 0x93A8:
case 0x95A8:
case 0x97A8:
case 0x99A8:
case 0x9BA8:
case 0x9DA8:
case 0x9FA8:
case 0x91A9:
case 0x93A9:
case 0x95A9:
case 0x97A9:
case 0x99A9:
case 0x9BA9:
case 0x9DA9:
case 0x9FA9:
case 0x91AA:
case 0x93AA:
case 0x95AA:
case 0x97AA:
case 0x99AA:
case 0x9BAA:
case 0x9DAA:
case 0x9FAA:
case 0x91AB:
case 0x93AB:
case 0x95AB:
case 0x97AB:
case 0x99AB:
case 0x9BAB:
case 0x9DAB:
case 0x9FAB:
case 0x91AC:
case 0x93AC:
case 0x95AC:
case 0x97AC:
case 0x99AC:
case 0x9BAC:
case 0x9DAC:
case 0x9FAC:
case 0x91AD:
case 0x93AD:
case 0x95AD:
case 0x97AD:
case 0x99AD:
case 0x9BAD:
case 0x9DAD:
case 0x9FAD:
case 0x91AE:
case 0x93AE:
case 0x95AE:
case 0x97AE:
case 0x99AE:
case 0x9BAE:
case 0x9DAE:
case 0x9FAE:
case 0x91AF:
case 0x93AF:
case 0x95AF:
case 0x97AF:
case 0x99AF:
case 0x9BAF:
case 0x9DAF:
case 0x9FAF:

// SUBDa
case 0x91A8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
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
case 0x93B0:
case 0x95B0:
case 0x97B0:
case 0x99B0:
case 0x9BB0:
case 0x9DB0:
case 0x9FB0:
case 0x91B1:
case 0x93B1:
case 0x95B1:
case 0x97B1:
case 0x99B1:
case 0x9BB1:
case 0x9DB1:
case 0x9FB1:
case 0x91B2:
case 0x93B2:
case 0x95B2:
case 0x97B2:
case 0x99B2:
case 0x9BB2:
case 0x9DB2:
case 0x9FB2:
case 0x91B3:
case 0x93B3:
case 0x95B3:
case 0x97B3:
case 0x99B3:
case 0x9BB3:
case 0x9DB3:
case 0x9FB3:
case 0x91B4:
case 0x93B4:
case 0x95B4:
case 0x97B4:
case 0x99B4:
case 0x9BB4:
case 0x9DB4:
case 0x9FB4:
case 0x91B5:
case 0x93B5:
case 0x95B5:
case 0x97B5:
case 0x99B5:
case 0x9BB5:
case 0x9DB5:
case 0x9FB5:
case 0x91B6:
case 0x93B6:
case 0x95B6:
case 0x97B6:
case 0x99B6:
case 0x9BB6:
case 0x9DB6:
case 0x9FB6:
case 0x91B7:
case 0x93B7:
case 0x95B7:
case 0x97B7:
case 0x99B7:
case 0x9BB7:
case 0x9DB7:
case 0x9FB7:

// SUBDa
case 0x91B0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
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
case 0x93B8:
case 0x95B8:
case 0x97B8:
case 0x99B8:
case 0x9BB8:
case 0x9DB8:
case 0x9FB8:

// SUBDa
case 0x91B8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
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
case 0x93B9:
case 0x95B9:
case 0x97B9:
case 0x99B9:
case 0x9BB9:
case 0x9DB9:
case 0x9FB9:

// SUBDa
case 0x91B9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
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
case 0x939F:
case 0x959F:
case 0x979F:
case 0x999F:
case 0x9B9F:
case 0x9D9F:
case 0x9F9F:

// SUBDa
case 0x919F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
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
case 0x93A7:
case 0x95A7:
case 0x97A7:
case 0x99A7:
case 0x9BA7:
case 0x9DA7:
case 0x9FA7:

// SUBDa
case 0x91A7:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
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
case 0x9300:
case 0x9500:
case 0x9700:
case 0x9900:
case 0x9B00:
case 0x9D00:
case 0x9F00:
case 0x9101:
case 0x9301:
case 0x9501:
case 0x9701:
case 0x9901:
case 0x9B01:
case 0x9D01:
case 0x9F01:
case 0x9102:
case 0x9302:
case 0x9502:
case 0x9702:
case 0x9902:
case 0x9B02:
case 0x9D02:
case 0x9F02:
case 0x9103:
case 0x9303:
case 0x9503:
case 0x9703:
case 0x9903:
case 0x9B03:
case 0x9D03:
case 0x9F03:
case 0x9104:
case 0x9304:
case 0x9504:
case 0x9704:
case 0x9904:
case 0x9B04:
case 0x9D04:
case 0x9F04:
case 0x9105:
case 0x9305:
case 0x9505:
case 0x9705:
case 0x9905:
case 0x9B05:
case 0x9D05:
case 0x9F05:
case 0x9106:
case 0x9306:
case 0x9506:
case 0x9706:
case 0x9906:
case 0x9B06:
case 0x9D06:
case 0x9F06:
case 0x9107:
case 0x9307:
case 0x9507:
case 0x9707:
case 0x9907:
case 0x9B07:
case 0x9D07:
case 0x9F07:

// SUBX
case 0x9100:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ |= res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0x9340:
case 0x9540:
case 0x9740:
case 0x9940:
case 0x9B40:
case 0x9D40:
case 0x9F40:
case 0x9141:
case 0x9341:
case 0x9541:
case 0x9741:
case 0x9941:
case 0x9B41:
case 0x9D41:
case 0x9F41:
case 0x9142:
case 0x9342:
case 0x9542:
case 0x9742:
case 0x9942:
case 0x9B42:
case 0x9D42:
case 0x9F42:
case 0x9143:
case 0x9343:
case 0x9543:
case 0x9743:
case 0x9943:
case 0x9B43:
case 0x9D43:
case 0x9F43:
case 0x9144:
case 0x9344:
case 0x9544:
case 0x9744:
case 0x9944:
case 0x9B44:
case 0x9D44:
case 0x9F44:
case 0x9145:
case 0x9345:
case 0x9545:
case 0x9745:
case 0x9945:
case 0x9B45:
case 0x9D45:
case 0x9F45:
case 0x9146:
case 0x9346:
case 0x9546:
case 0x9746:
case 0x9946:
case 0x9B46:
case 0x9D46:
case 0x9F46:
case 0x9147:
case 0x9347:
case 0x9547:
case 0x9747:
case 0x9947:
case 0x9B47:
case 0x9D47:
case 0x9F47:

// SUBX
case 0x9140:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0x9380:
case 0x9580:
case 0x9780:
case 0x9980:
case 0x9B80:
case 0x9D80:
case 0x9F80:
case 0x9181:
case 0x9381:
case 0x9581:
case 0x9781:
case 0x9981:
case 0x9B81:
case 0x9D81:
case 0x9F81:
case 0x9182:
case 0x9382:
case 0x9582:
case 0x9782:
case 0x9982:
case 0x9B82:
case 0x9D82:
case 0x9F82:
case 0x9183:
case 0x9383:
case 0x9583:
case 0x9783:
case 0x9983:
case 0x9B83:
case 0x9D83:
case 0x9F83:
case 0x9184:
case 0x9384:
case 0x9584:
case 0x9784:
case 0x9984:
case 0x9B84:
case 0x9D84:
case 0x9F84:
case 0x9185:
case 0x9385:
case 0x9585:
case 0x9785:
case 0x9985:
case 0x9B85:
case 0x9D85:
case 0x9F85:
case 0x9186:
case 0x9386:
case 0x9586:
case 0x9786:
case 0x9986:
case 0x9B86:
case 0x9D86:
case 0x9F86:
case 0x9187:
case 0x9387:
case 0x9587:
case 0x9787:
case 0x9987:
case 0x9B87:
case 0x9D87:
case 0x9F87:

// SUBX
case 0x9180:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0x9308:
case 0x9508:
case 0x9708:
case 0x9908:
case 0x9B08:
case 0x9D08:
case 0x9109:
case 0x9309:
case 0x9509:
case 0x9709:
case 0x9909:
case 0x9B09:
case 0x9D09:
case 0x910A:
case 0x930A:
case 0x950A:
case 0x970A:
case 0x990A:
case 0x9B0A:
case 0x9D0A:
case 0x910B:
case 0x930B:
case 0x950B:
case 0x970B:
case 0x990B:
case 0x9B0B:
case 0x9D0B:
case 0x910C:
case 0x930C:
case 0x950C:
case 0x970C:
case 0x990C:
case 0x9B0C:
case 0x9D0C:
case 0x910D:
case 0x930D:
case 0x950D:
case 0x970D:
case 0x990D:
case 0x9B0D:
case 0x9D0D:
case 0x910E:
case 0x930E:
case 0x950E:
case 0x970E:
case 0x990E:
case 0x9B0E:
case 0x9D0E:

// SUBXM
case 0x9108:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	READ_BYTE_F(adr, dst)
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x9348:
case 0x9548:
case 0x9748:
case 0x9948:
case 0x9B48:
case 0x9D48:
case 0x9149:
case 0x9349:
case 0x9549:
case 0x9749:
case 0x9949:
case 0x9B49:
case 0x9D49:
case 0x914A:
case 0x934A:
case 0x954A:
case 0x974A:
case 0x994A:
case 0x9B4A:
case 0x9D4A:
case 0x914B:
case 0x934B:
case 0x954B:
case 0x974B:
case 0x994B:
case 0x9B4B:
case 0x9D4B:
case 0x914C:
case 0x934C:
case 0x954C:
case 0x974C:
case 0x994C:
case 0x9B4C:
case 0x9D4C:
case 0x914D:
case 0x934D:
case 0x954D:
case 0x974D:
case 0x994D:
case 0x9B4D:
case 0x9D4D:
case 0x914E:
case 0x934E:
case 0x954E:
case 0x974E:
case 0x994E:
case 0x9B4E:
case 0x9D4E:

// SUBXM
case 0x9148:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	READ_WORD_F(adr, dst)
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x9388:
case 0x9588:
case 0x9788:
case 0x9988:
case 0x9B88:
case 0x9D88:
case 0x9189:
case 0x9389:
case 0x9589:
case 0x9789:
case 0x9989:
case 0x9B89:
case 0x9D89:
case 0x918A:
case 0x938A:
case 0x958A:
case 0x978A:
case 0x998A:
case 0x9B8A:
case 0x9D8A:
case 0x918B:
case 0x938B:
case 0x958B:
case 0x978B:
case 0x998B:
case 0x9B8B:
case 0x9D8B:
case 0x918C:
case 0x938C:
case 0x958C:
case 0x978C:
case 0x998C:
case 0x9B8C:
case 0x9D8C:
case 0x918D:
case 0x938D:
case 0x958D:
case 0x978D:
case 0x998D:
case 0x9B8D:
case 0x9D8D:
case 0x918E:
case 0x938E:
case 0x958E:
case 0x978E:
case 0x998E:
case 0x9B8E:
case 0x9D8E:

// SUBXM
case 0x9188:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	READ_LONG_F(adr, dst)
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)
case 0x930F:
case 0x950F:
case 0x970F:
case 0x990F:
case 0x9B0F:
case 0x9D0F:

// SUBX7M
case 0x910F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	READ_BYTE_F(adr, dst)
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x934F:
case 0x954F:
case 0x974F:
case 0x994F:
case 0x9B4F:
case 0x9D4F:

// SUBX7M
case 0x914F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	READ_WORD_F(adr, dst)
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x938F:
case 0x958F:
case 0x978F:
case 0x998F:
case 0x9B8F:
case 0x9D8F:

// SUBX7M
case 0x918F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	READ_LONG_F(adr, dst)
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)
case 0x9F09:
case 0x9F0A:
case 0x9F0B:
case 0x9F0C:
case 0x9F0D:
case 0x9F0E:

// SUBXM7
case 0x9F08:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	READ_BYTE_F(adr, dst)
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x9F49:
case 0x9F4A:
case 0x9F4B:
case 0x9F4C:
case 0x9F4D:
case 0x9F4E:

// SUBXM7
case 0x9F48:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	READ_WORD_F(adr, dst)
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x9F89:
case 0x9F8A:
case 0x9F8B:
case 0x9F8C:
case 0x9F8D:
case 0x9F8E:

// SUBXM7
case 0x9F88:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	READ_LONG_F(adr, dst)
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)

// SUBX7M7
case 0x9F0F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	READ_BYTE_F(adr, dst)
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)

// SUBX7M7
case 0x9F4F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	READ_WORD_F(adr, dst)
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// SUBX7M7
case 0x9F8F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	READ_LONG_F(adr, dst)
	res = dst - src - ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)
case 0x92C0:
case 0x94C0:
case 0x96C0:
case 0x98C0:
case 0x9AC0:
case 0x9CC0:
case 0x9EC0:
case 0x90C1:
case 0x92C1:
case 0x94C1:
case 0x96C1:
case 0x98C1:
case 0x9AC1:
case 0x9CC1:
case 0x9EC1:
case 0x90C2:
case 0x92C2:
case 0x94C2:
case 0x96C2:
case 0x98C2:
case 0x9AC2:
case 0x9CC2:
case 0x9EC2:
case 0x90C3:
case 0x92C3:
case 0x94C3:
case 0x96C3:
case 0x98C3:
case 0x9AC3:
case 0x9CC3:
case 0x9EC3:
case 0x90C4:
case 0x92C4:
case 0x94C4:
case 0x96C4:
case 0x98C4:
case 0x9AC4:
case 0x9CC4:
case 0x9EC4:
case 0x90C5:
case 0x92C5:
case 0x94C5:
case 0x96C5:
case 0x98C5:
case 0x9AC5:
case 0x9CC5:
case 0x9EC5:
case 0x90C6:
case 0x92C6:
case 0x94C6:
case 0x96C6:
case 0x98C6:
case 0x9AC6:
case 0x9CC6:
case 0x9EC6:
case 0x90C7:
case 0x92C7:
case 0x94C7:
case 0x96C7:
case 0x98C7:
case 0x9AC7:
case 0x9CC7:
case 0x9EC7:

// SUBA
case 0x90C0:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s16)CPU->D[(Opcode >> 0) & 7];
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(8)
case 0x92C8:
case 0x94C8:
case 0x96C8:
case 0x98C8:
case 0x9AC8:
case 0x9CC8:
case 0x9EC8:
case 0x90C9:
case 0x92C9:
case 0x94C9:
case 0x96C9:
case 0x98C9:
case 0x9AC9:
case 0x9CC9:
case 0x9EC9:
case 0x90CA:
case 0x92CA:
case 0x94CA:
case 0x96CA:
case 0x98CA:
case 0x9ACA:
case 0x9CCA:
case 0x9ECA:
case 0x90CB:
case 0x92CB:
case 0x94CB:
case 0x96CB:
case 0x98CB:
case 0x9ACB:
case 0x9CCB:
case 0x9ECB:
case 0x90CC:
case 0x92CC:
case 0x94CC:
case 0x96CC:
case 0x98CC:
case 0x9ACC:
case 0x9CCC:
case 0x9ECC:
case 0x90CD:
case 0x92CD:
case 0x94CD:
case 0x96CD:
case 0x98CD:
case 0x9ACD:
case 0x9CCD:
case 0x9ECD:
case 0x90CE:
case 0x92CE:
case 0x94CE:
case 0x96CE:
case 0x98CE:
case 0x9ACE:
case 0x9CCE:
case 0x9ECE:
case 0x90CF:
case 0x92CF:
case 0x94CF:
case 0x96CF:
case 0x98CF:
case 0x9ACF:
case 0x9CCF:
case 0x9ECF:

// SUBA
case 0x90C8:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s16)CPU->A[(Opcode >> 0) & 7];
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(8)
case 0x92D0:
case 0x94D0:
case 0x96D0:
case 0x98D0:
case 0x9AD0:
case 0x9CD0:
case 0x9ED0:
case 0x90D1:
case 0x92D1:
case 0x94D1:
case 0x96D1:
case 0x98D1:
case 0x9AD1:
case 0x9CD1:
case 0x9ED1:
case 0x90D2:
case 0x92D2:
case 0x94D2:
case 0x96D2:
case 0x98D2:
case 0x9AD2:
case 0x9CD2:
case 0x9ED2:
case 0x90D3:
case 0x92D3:
case 0x94D3:
case 0x96D3:
case 0x98D3:
case 0x9AD3:
case 0x9CD3:
case 0x9ED3:
case 0x90D4:
case 0x92D4:
case 0x94D4:
case 0x96D4:
case 0x98D4:
case 0x9AD4:
case 0x9CD4:
case 0x9ED4:
case 0x90D5:
case 0x92D5:
case 0x94D5:
case 0x96D5:
case 0x98D5:
case 0x9AD5:
case 0x9CD5:
case 0x9ED5:
case 0x90D6:
case 0x92D6:
case 0x94D6:
case 0x96D6:
case 0x98D6:
case 0x9AD6:
case 0x9CD6:
case 0x9ED6:
case 0x90D7:
case 0x92D7:
case 0x94D7:
case 0x96D7:
case 0x98D7:
case 0x9AD7:
case 0x9CD7:
case 0x9ED7:

// SUBA
case 0x90D0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READSX_WORD_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(10)
case 0x92D8:
case 0x94D8:
case 0x96D8:
case 0x98D8:
case 0x9AD8:
case 0x9CD8:
case 0x9ED8:
case 0x90D9:
case 0x92D9:
case 0x94D9:
case 0x96D9:
case 0x98D9:
case 0x9AD9:
case 0x9CD9:
case 0x9ED9:
case 0x90DA:
case 0x92DA:
case 0x94DA:
case 0x96DA:
case 0x98DA:
case 0x9ADA:
case 0x9CDA:
case 0x9EDA:
case 0x90DB:
case 0x92DB:
case 0x94DB:
case 0x96DB:
case 0x98DB:
case 0x9ADB:
case 0x9CDB:
case 0x9EDB:
case 0x90DC:
case 0x92DC:
case 0x94DC:
case 0x96DC:
case 0x98DC:
case 0x9ADC:
case 0x9CDC:
case 0x9EDC:
case 0x90DD:
case 0x92DD:
case 0x94DD:
case 0x96DD:
case 0x98DD:
case 0x9ADD:
case 0x9CDD:
case 0x9EDD:
case 0x90DE:
case 0x92DE:
case 0x94DE:
case 0x96DE:
case 0x98DE:
case 0x9ADE:
case 0x9CDE:
case 0x9EDE:

// SUBA
case 0x90D8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(10)
case 0x92E0:
case 0x94E0:
case 0x96E0:
case 0x98E0:
case 0x9AE0:
case 0x9CE0:
case 0x9EE0:
case 0x90E1:
case 0x92E1:
case 0x94E1:
case 0x96E1:
case 0x98E1:
case 0x9AE1:
case 0x9CE1:
case 0x9EE1:
case 0x90E2:
case 0x92E2:
case 0x94E2:
case 0x96E2:
case 0x98E2:
case 0x9AE2:
case 0x9CE2:
case 0x9EE2:
case 0x90E3:
case 0x92E3:
case 0x94E3:
case 0x96E3:
case 0x98E3:
case 0x9AE3:
case 0x9CE3:
case 0x9EE3:
case 0x90E4:
case 0x92E4:
case 0x94E4:
case 0x96E4:
case 0x98E4:
case 0x9AE4:
case 0x9CE4:
case 0x9EE4:
case 0x90E5:
case 0x92E5:
case 0x94E5:
case 0x96E5:
case 0x98E5:
case 0x9AE5:
case 0x9CE5:
case 0x9EE5:
case 0x90E6:
case 0x92E6:
case 0x94E6:
case 0x96E6:
case 0x98E6:
case 0x9AE6:
case 0x9CE6:
case 0x9EE6:

// SUBA
case 0x90E0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READSX_WORD_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(12)
case 0x92E8:
case 0x94E8:
case 0x96E8:
case 0x98E8:
case 0x9AE8:
case 0x9CE8:
case 0x9EE8:
case 0x90E9:
case 0x92E9:
case 0x94E9:
case 0x96E9:
case 0x98E9:
case 0x9AE9:
case 0x9CE9:
case 0x9EE9:
case 0x90EA:
case 0x92EA:
case 0x94EA:
case 0x96EA:
case 0x98EA:
case 0x9AEA:
case 0x9CEA:
case 0x9EEA:
case 0x90EB:
case 0x92EB:
case 0x94EB:
case 0x96EB:
case 0x98EB:
case 0x9AEB:
case 0x9CEB:
case 0x9EEB:
case 0x90EC:
case 0x92EC:
case 0x94EC:
case 0x96EC:
case 0x98EC:
case 0x9AEC:
case 0x9CEC:
case 0x9EEC:
case 0x90ED:
case 0x92ED:
case 0x94ED:
case 0x96ED:
case 0x98ED:
case 0x9AED:
case 0x9CED:
case 0x9EED:
case 0x90EE:
case 0x92EE:
case 0x94EE:
case 0x96EE:
case 0x98EE:
case 0x9AEE:
case 0x9CEE:
case 0x9EEE:
case 0x90EF:
case 0x92EF:
case 0x94EF:
case 0x96EF:
case 0x98EF:
case 0x9AEF:
case 0x9CEF:
case 0x9EEF:

// SUBA
case 0x90E8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0x92F0:
case 0x94F0:
case 0x96F0:
case 0x98F0:
case 0x9AF0:
case 0x9CF0:
case 0x9EF0:
case 0x90F1:
case 0x92F1:
case 0x94F1:
case 0x96F1:
case 0x98F1:
case 0x9AF1:
case 0x9CF1:
case 0x9EF1:
case 0x90F2:
case 0x92F2:
case 0x94F2:
case 0x96F2:
case 0x98F2:
case 0x9AF2:
case 0x9CF2:
case 0x9EF2:
case 0x90F3:
case 0x92F3:
case 0x94F3:
case 0x96F3:
case 0x98F3:
case 0x9AF3:
case 0x9CF3:
case 0x9EF3:
case 0x90F4:
case 0x92F4:
case 0x94F4:
case 0x96F4:
case 0x98F4:
case 0x9AF4:
case 0x9CF4:
case 0x9EF4:
case 0x90F5:
case 0x92F5:
case 0x94F5:
case 0x96F5:
case 0x98F5:
case 0x9AF5:
case 0x9CF5:
case 0x9EF5:
case 0x90F6:
case 0x92F6:
case 0x94F6:
case 0x96F6:
case 0x98F6:
case 0x9AF6:
case 0x9CF6:
case 0x9EF6:
case 0x90F7:
case 0x92F7:
case 0x94F7:
case 0x96F7:
case 0x98F7:
case 0x9AF7:
case 0x9CF7:
case 0x9EF7:

// SUBA
case 0x90F0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READSX_WORD_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(16)
case 0x92F8:
case 0x94F8:
case 0x96F8:
case 0x98F8:
case 0x9AF8:
case 0x9CF8:
case 0x9EF8:

// SUBA
case 0x90F8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0x92F9:
case 0x94F9:
case 0x96F9:
case 0x98F9:
case 0x9AF9:
case 0x9CF9:
case 0x9EF9:

// SUBA
case 0x90F9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READSX_WORD_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(18)
case 0x92FA:
case 0x94FA:
case 0x96FA:
case 0x98FA:
case 0x9AFA:
case 0x9CFA:
case 0x9EFA:

// SUBA
case 0x90FA:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0x92FB:
case 0x94FB:
case 0x96FB:
case 0x98FB:
case 0x9AFB:
case 0x9CFB:
case 0x9EFB:

// SUBA
case 0x90FB:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READSX_WORD_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(16)
case 0x92FC:
case 0x94FC:
case 0x96FC:
case 0x98FC:
case 0x9AFC:
case 0x9CFC:
case 0x9EFC:

// SUBA
case 0x90FC:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s16)FETCH_WORD;
	PC += 2;
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(12)
case 0x92DF:
case 0x94DF:
case 0x96DF:
case 0x98DF:
case 0x9ADF:
case 0x9CDF:
case 0x9EDF:

// SUBA
case 0x90DF:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(10)
case 0x92E7:
case 0x94E7:
case 0x96E7:
case 0x98E7:
case 0x9AE7:
case 0x9CE7:
case 0x9EE7:

// SUBA
case 0x90E7:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READSX_WORD_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(12)
case 0x93C0:
case 0x95C0:
case 0x97C0:
case 0x99C0:
case 0x9BC0:
case 0x9DC0:
case 0x9FC0:
case 0x91C1:
case 0x93C1:
case 0x95C1:
case 0x97C1:
case 0x99C1:
case 0x9BC1:
case 0x9DC1:
case 0x9FC1:
case 0x91C2:
case 0x93C2:
case 0x95C2:
case 0x97C2:
case 0x99C2:
case 0x9BC2:
case 0x9DC2:
case 0x9FC2:
case 0x91C3:
case 0x93C3:
case 0x95C3:
case 0x97C3:
case 0x99C3:
case 0x9BC3:
case 0x9DC3:
case 0x9FC3:
case 0x91C4:
case 0x93C4:
case 0x95C4:
case 0x97C4:
case 0x99C4:
case 0x9BC4:
case 0x9DC4:
case 0x9FC4:
case 0x91C5:
case 0x93C5:
case 0x95C5:
case 0x97C5:
case 0x99C5:
case 0x9BC5:
case 0x9DC5:
case 0x9FC5:
case 0x91C6:
case 0x93C6:
case 0x95C6:
case 0x97C6:
case 0x99C6:
case 0x9BC6:
case 0x9DC6:
case 0x9FC6:
case 0x91C7:
case 0x93C7:
case 0x95C7:
case 0x97C7:
case 0x99C7:
case 0x9BC7:
case 0x9DC7:
case 0x9FC7:

// SUBA
case 0x91C0:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s32)CPU->D[(Opcode >> 0) & 7];
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(6)
case 0x93C8:
case 0x95C8:
case 0x97C8:
case 0x99C8:
case 0x9BC8:
case 0x9DC8:
case 0x9FC8:
case 0x91C9:
case 0x93C9:
case 0x95C9:
case 0x97C9:
case 0x99C9:
case 0x9BC9:
case 0x9DC9:
case 0x9FC9:
case 0x91CA:
case 0x93CA:
case 0x95CA:
case 0x97CA:
case 0x99CA:
case 0x9BCA:
case 0x9DCA:
case 0x9FCA:
case 0x91CB:
case 0x93CB:
case 0x95CB:
case 0x97CB:
case 0x99CB:
case 0x9BCB:
case 0x9DCB:
case 0x9FCB:
case 0x91CC:
case 0x93CC:
case 0x95CC:
case 0x97CC:
case 0x99CC:
case 0x9BCC:
case 0x9DCC:
case 0x9FCC:
case 0x91CD:
case 0x93CD:
case 0x95CD:
case 0x97CD:
case 0x99CD:
case 0x9BCD:
case 0x9DCD:
case 0x9FCD:
case 0x91CE:
case 0x93CE:
case 0x95CE:
case 0x97CE:
case 0x99CE:
case 0x9BCE:
case 0x9DCE:
case 0x9FCE:
case 0x91CF:
case 0x93CF:
case 0x95CF:
case 0x97CF:
case 0x99CF:
case 0x9BCF:
case 0x9DCF:
case 0x9FCF:

// SUBA
case 0x91C8:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s32)CPU->A[(Opcode >> 0) & 7];
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(6)
case 0x93D0:
case 0x95D0:
case 0x97D0:
case 0x99D0:
case 0x9BD0:
case 0x9DD0:
case 0x9FD0:
case 0x91D1:
case 0x93D1:
case 0x95D1:
case 0x97D1:
case 0x99D1:
case 0x9BD1:
case 0x9DD1:
case 0x9FD1:
case 0x91D2:
case 0x93D2:
case 0x95D2:
case 0x97D2:
case 0x99D2:
case 0x9BD2:
case 0x9DD2:
case 0x9FD2:
case 0x91D3:
case 0x93D3:
case 0x95D3:
case 0x97D3:
case 0x99D3:
case 0x9BD3:
case 0x9DD3:
case 0x9FD3:
case 0x91D4:
case 0x93D4:
case 0x95D4:
case 0x97D4:
case 0x99D4:
case 0x9BD4:
case 0x9DD4:
case 0x9FD4:
case 0x91D5:
case 0x93D5:
case 0x95D5:
case 0x97D5:
case 0x99D5:
case 0x9BD5:
case 0x9DD5:
case 0x9FD5:
case 0x91D6:
case 0x93D6:
case 0x95D6:
case 0x97D6:
case 0x99D6:
case 0x9BD6:
case 0x9DD6:
case 0x9FD6:
case 0x91D7:
case 0x93D7:
case 0x95D7:
case 0x97D7:
case 0x99D7:
case 0x9BD7:
case 0x9DD7:
case 0x9FD7:

// SUBA
case 0x91D0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READSX_LONG_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0x93D8:
case 0x95D8:
case 0x97D8:
case 0x99D8:
case 0x9BD8:
case 0x9DD8:
case 0x9FD8:
case 0x91D9:
case 0x93D9:
case 0x95D9:
case 0x97D9:
case 0x99D9:
case 0x9BD9:
case 0x9DD9:
case 0x9FD9:
case 0x91DA:
case 0x93DA:
case 0x95DA:
case 0x97DA:
case 0x99DA:
case 0x9BDA:
case 0x9DDA:
case 0x9FDA:
case 0x91DB:
case 0x93DB:
case 0x95DB:
case 0x97DB:
case 0x99DB:
case 0x9BDB:
case 0x9DDB:
case 0x9FDB:
case 0x91DC:
case 0x93DC:
case 0x95DC:
case 0x97DC:
case 0x99DC:
case 0x9BDC:
case 0x9DDC:
case 0x9FDC:
case 0x91DD:
case 0x93DD:
case 0x95DD:
case 0x97DD:
case 0x99DD:
case 0x9BDD:
case 0x9DDD:
case 0x9FDD:
case 0x91DE:
case 0x93DE:
case 0x95DE:
case 0x97DE:
case 0x99DE:
case 0x9BDE:
case 0x9DDE:
case 0x9FDE:

// SUBA
case 0x91D8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READSX_LONG_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0x93E0:
case 0x95E0:
case 0x97E0:
case 0x99E0:
case 0x9BE0:
case 0x9DE0:
case 0x9FE0:
case 0x91E1:
case 0x93E1:
case 0x95E1:
case 0x97E1:
case 0x99E1:
case 0x9BE1:
case 0x9DE1:
case 0x9FE1:
case 0x91E2:
case 0x93E2:
case 0x95E2:
case 0x97E2:
case 0x99E2:
case 0x9BE2:
case 0x9DE2:
case 0x9FE2:
case 0x91E3:
case 0x93E3:
case 0x95E3:
case 0x97E3:
case 0x99E3:
case 0x9BE3:
case 0x9DE3:
case 0x9FE3:
case 0x91E4:
case 0x93E4:
case 0x95E4:
case 0x97E4:
case 0x99E4:
case 0x9BE4:
case 0x9DE4:
case 0x9FE4:
case 0x91E5:
case 0x93E5:
case 0x95E5:
case 0x97E5:
case 0x99E5:
case 0x9BE5:
case 0x9DE5:
case 0x9FE5:
case 0x91E6:
case 0x93E6:
case 0x95E6:
case 0x97E6:
case 0x99E6:
case 0x9BE6:
case 0x9DE6:
case 0x9FE6:

// SUBA
case 0x91E0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READSX_LONG_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(16)
case 0x93E8:
case 0x95E8:
case 0x97E8:
case 0x99E8:
case 0x9BE8:
case 0x9DE8:
case 0x9FE8:
case 0x91E9:
case 0x93E9:
case 0x95E9:
case 0x97E9:
case 0x99E9:
case 0x9BE9:
case 0x9DE9:
case 0x9FE9:
case 0x91EA:
case 0x93EA:
case 0x95EA:
case 0x97EA:
case 0x99EA:
case 0x9BEA:
case 0x9DEA:
case 0x9FEA:
case 0x91EB:
case 0x93EB:
case 0x95EB:
case 0x97EB:
case 0x99EB:
case 0x9BEB:
case 0x9DEB:
case 0x9FEB:
case 0x91EC:
case 0x93EC:
case 0x95EC:
case 0x97EC:
case 0x99EC:
case 0x9BEC:
case 0x9DEC:
case 0x9FEC:
case 0x91ED:
case 0x93ED:
case 0x95ED:
case 0x97ED:
case 0x99ED:
case 0x9BED:
case 0x9DED:
case 0x9FED:
case 0x91EE:
case 0x93EE:
case 0x95EE:
case 0x97EE:
case 0x99EE:
case 0x9BEE:
case 0x9DEE:
case 0x9FEE:
case 0x91EF:
case 0x93EF:
case 0x95EF:
case 0x97EF:
case 0x99EF:
case 0x9BEF:
case 0x9DEF:
case 0x9FEF:

// SUBA
case 0x91E8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_LONG_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(18)
case 0x93F0:
case 0x95F0:
case 0x97F0:
case 0x99F0:
case 0x9BF0:
case 0x9DF0:
case 0x9FF0:
case 0x91F1:
case 0x93F1:
case 0x95F1:
case 0x97F1:
case 0x99F1:
case 0x9BF1:
case 0x9DF1:
case 0x9FF1:
case 0x91F2:
case 0x93F2:
case 0x95F2:
case 0x97F2:
case 0x99F2:
case 0x9BF2:
case 0x9DF2:
case 0x9FF2:
case 0x91F3:
case 0x93F3:
case 0x95F3:
case 0x97F3:
case 0x99F3:
case 0x9BF3:
case 0x9DF3:
case 0x9FF3:
case 0x91F4:
case 0x93F4:
case 0x95F4:
case 0x97F4:
case 0x99F4:
case 0x9BF4:
case 0x9DF4:
case 0x9FF4:
case 0x91F5:
case 0x93F5:
case 0x95F5:
case 0x97F5:
case 0x99F5:
case 0x9BF5:
case 0x9DF5:
case 0x9FF5:
case 0x91F6:
case 0x93F6:
case 0x95F6:
case 0x97F6:
case 0x99F6:
case 0x9BF6:
case 0x9DF6:
case 0x9FF6:
case 0x91F7:
case 0x93F7:
case 0x95F7:
case 0x97F7:
case 0x99F7:
case 0x9BF7:
case 0x9DF7:
case 0x9FF7:

// SUBA
case 0x91F0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READSX_LONG_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(20)
case 0x93F8:
case 0x95F8:
case 0x97F8:
case 0x99F8:
case 0x9BF8:
case 0x9DF8:
case 0x9FF8:

// SUBA
case 0x91F8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_LONG_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(18)
case 0x93F9:
case 0x95F9:
case 0x97F9:
case 0x99F9:
case 0x9BF9:
case 0x9DF9:
case 0x9FF9:

// SUBA
case 0x91F9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READSX_LONG_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(22)
case 0x93FA:
case 0x95FA:
case 0x97FA:
case 0x99FA:
case 0x9BFA:
case 0x9DFA:
case 0x9FFA:

// SUBA
case 0x91FA:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_LONG_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(18)
case 0x93FB:
case 0x95FB:
case 0x97FB:
case 0x99FB:
case 0x9BFB:
case 0x9DFB:
case 0x9FFB:

// SUBA
case 0x91FB:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READSX_LONG_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(20)
case 0x93FC:
case 0x95FC:
case 0x97FC:
case 0x99FC:
case 0x9BFC:
case 0x9DFC:
case 0x9FFC:

// SUBA
case 0x91FC:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s32)FETCH_LONG;
	PC += 4;
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(14)
case 0x93DF:
case 0x95DF:
case 0x97DF:
case 0x99DF:
case 0x9BDF:
case 0x9DDF:
case 0x9FDF:

// SUBA
case 0x91DF:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READSX_LONG_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0x93E7:
case 0x95E7:
case 0x97E7:
case 0x99E7:
case 0x9BE7:
case 0x9DE7:
case 0x9FE7:

// SUBA
case 0x91E7:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READSX_LONG_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(16)
