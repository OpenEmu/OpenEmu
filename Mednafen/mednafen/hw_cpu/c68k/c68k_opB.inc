case 0xB200:
case 0xB400:
case 0xB600:
case 0xB800:
case 0xBA00:
case 0xBC00:
case 0xBE00:
case 0xB001:
case 0xB201:
case 0xB401:
case 0xB601:
case 0xB801:
case 0xBA01:
case 0xBC01:
case 0xBE01:
case 0xB002:
case 0xB202:
case 0xB402:
case 0xB602:
case 0xB802:
case 0xBA02:
case 0xBC02:
case 0xBE02:
case 0xB003:
case 0xB203:
case 0xB403:
case 0xB603:
case 0xB803:
case 0xBA03:
case 0xBC03:
case 0xBE03:
case 0xB004:
case 0xB204:
case 0xB404:
case 0xB604:
case 0xB804:
case 0xBA04:
case 0xBC04:
case 0xBE04:
case 0xB005:
case 0xB205:
case 0xB405:
case 0xB605:
case 0xB805:
case 0xBA05:
case 0xBC05:
case 0xBE05:
case 0xB006:
case 0xB206:
case 0xB406:
case 0xB606:
case 0xB806:
case 0xBA06:
case 0xBC06:
case 0xBE06:
case 0xB007:
case 0xB207:
case 0xB407:
case 0xB607:
case 0xB807:
case 0xBA07:
case 0xBC07:
case 0xBE07:

// CMP
case 0xB000:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
}
RET(4)
case 0xB208:
case 0xB408:
case 0xB608:
case 0xB808:
case 0xBA08:
case 0xBC08:
case 0xBE08:
case 0xB009:
case 0xB209:
case 0xB409:
case 0xB609:
case 0xB809:
case 0xBA09:
case 0xBC09:
case 0xBE09:
case 0xB00A:
case 0xB20A:
case 0xB40A:
case 0xB60A:
case 0xB80A:
case 0xBA0A:
case 0xBC0A:
case 0xBE0A:
case 0xB00B:
case 0xB20B:
case 0xB40B:
case 0xB60B:
case 0xB80B:
case 0xBA0B:
case 0xBC0B:
case 0xBE0B:
case 0xB00C:
case 0xB20C:
case 0xB40C:
case 0xB60C:
case 0xB80C:
case 0xBA0C:
case 0xBC0C:
case 0xBE0C:
case 0xB00D:
case 0xB20D:
case 0xB40D:
case 0xB60D:
case 0xB80D:
case 0xBA0D:
case 0xBC0D:
case 0xBE0D:
case 0xB00E:
case 0xB20E:
case 0xB40E:
case 0xB60E:
case 0xB80E:
case 0xBA0E:
case 0xBC0E:
case 0xBE0E:
case 0xB00F:
case 0xB20F:
case 0xB40F:
case 0xB60F:
case 0xB80F:
case 0xBA0F:
case 0xBC0F:
case 0xBE0F:

// CMP
case 0xB008:
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
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
}
RET(4)
case 0xB210:
case 0xB410:
case 0xB610:
case 0xB810:
case 0xBA10:
case 0xBC10:
case 0xBE10:
case 0xB011:
case 0xB211:
case 0xB411:
case 0xB611:
case 0xB811:
case 0xBA11:
case 0xBC11:
case 0xBE11:
case 0xB012:
case 0xB212:
case 0xB412:
case 0xB612:
case 0xB812:
case 0xBA12:
case 0xBC12:
case 0xBE12:
case 0xB013:
case 0xB213:
case 0xB413:
case 0xB613:
case 0xB813:
case 0xBA13:
case 0xBC13:
case 0xBE13:
case 0xB014:
case 0xB214:
case 0xB414:
case 0xB614:
case 0xB814:
case 0xBA14:
case 0xBC14:
case 0xBE14:
case 0xB015:
case 0xB215:
case 0xB415:
case 0xB615:
case 0xB815:
case 0xBA15:
case 0xBC15:
case 0xBE15:
case 0xB016:
case 0xB216:
case 0xB416:
case 0xB616:
case 0xB816:
case 0xBA16:
case 0xBC16:
case 0xBE16:
case 0xB017:
case 0xB217:
case 0xB417:
case 0xB617:
case 0xB817:
case 0xBA17:
case 0xBC17:
case 0xBE17:

// CMP
case 0xB010:
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
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(8)
case 0xB218:
case 0xB418:
case 0xB618:
case 0xB818:
case 0xBA18:
case 0xBC18:
case 0xBE18:
case 0xB019:
case 0xB219:
case 0xB419:
case 0xB619:
case 0xB819:
case 0xBA19:
case 0xBC19:
case 0xBE19:
case 0xB01A:
case 0xB21A:
case 0xB41A:
case 0xB61A:
case 0xB81A:
case 0xBA1A:
case 0xBC1A:
case 0xBE1A:
case 0xB01B:
case 0xB21B:
case 0xB41B:
case 0xB61B:
case 0xB81B:
case 0xBA1B:
case 0xBC1B:
case 0xBE1B:
case 0xB01C:
case 0xB21C:
case 0xB41C:
case 0xB61C:
case 0xB81C:
case 0xBA1C:
case 0xBC1C:
case 0xBE1C:
case 0xB01D:
case 0xB21D:
case 0xB41D:
case 0xB61D:
case 0xB81D:
case 0xBA1D:
case 0xBC1D:
case 0xBE1D:
case 0xB01E:
case 0xB21E:
case 0xB41E:
case 0xB61E:
case 0xB81E:
case 0xBA1E:
case 0xBC1E:
case 0xBE1E:

// CMP
case 0xB018:
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
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(8)
case 0xB220:
case 0xB420:
case 0xB620:
case 0xB820:
case 0xBA20:
case 0xBC20:
case 0xBE20:
case 0xB021:
case 0xB221:
case 0xB421:
case 0xB621:
case 0xB821:
case 0xBA21:
case 0xBC21:
case 0xBE21:
case 0xB022:
case 0xB222:
case 0xB422:
case 0xB622:
case 0xB822:
case 0xBA22:
case 0xBC22:
case 0xBE22:
case 0xB023:
case 0xB223:
case 0xB423:
case 0xB623:
case 0xB823:
case 0xBA23:
case 0xBC23:
case 0xBE23:
case 0xB024:
case 0xB224:
case 0xB424:
case 0xB624:
case 0xB824:
case 0xBA24:
case 0xBC24:
case 0xBE24:
case 0xB025:
case 0xB225:
case 0xB425:
case 0xB625:
case 0xB825:
case 0xBA25:
case 0xBC25:
case 0xBE25:
case 0xB026:
case 0xB226:
case 0xB426:
case 0xB626:
case 0xB826:
case 0xBA26:
case 0xBC26:
case 0xBE26:

// CMP
case 0xB020:
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
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(10)
case 0xB228:
case 0xB428:
case 0xB628:
case 0xB828:
case 0xBA28:
case 0xBC28:
case 0xBE28:
case 0xB029:
case 0xB229:
case 0xB429:
case 0xB629:
case 0xB829:
case 0xBA29:
case 0xBC29:
case 0xBE29:
case 0xB02A:
case 0xB22A:
case 0xB42A:
case 0xB62A:
case 0xB82A:
case 0xBA2A:
case 0xBC2A:
case 0xBE2A:
case 0xB02B:
case 0xB22B:
case 0xB42B:
case 0xB62B:
case 0xB82B:
case 0xBA2B:
case 0xBC2B:
case 0xBE2B:
case 0xB02C:
case 0xB22C:
case 0xB42C:
case 0xB62C:
case 0xB82C:
case 0xBA2C:
case 0xBC2C:
case 0xBE2C:
case 0xB02D:
case 0xB22D:
case 0xB42D:
case 0xB62D:
case 0xB82D:
case 0xBA2D:
case 0xBC2D:
case 0xBE2D:
case 0xB02E:
case 0xB22E:
case 0xB42E:
case 0xB62E:
case 0xB82E:
case 0xBA2E:
case 0xBC2E:
case 0xBE2E:
case 0xB02F:
case 0xB22F:
case 0xB42F:
case 0xB62F:
case 0xB82F:
case 0xBA2F:
case 0xBC2F:
case 0xBE2F:

// CMP
case 0xB028:
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
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(12)
case 0xB230:
case 0xB430:
case 0xB630:
case 0xB830:
case 0xBA30:
case 0xBC30:
case 0xBE30:
case 0xB031:
case 0xB231:
case 0xB431:
case 0xB631:
case 0xB831:
case 0xBA31:
case 0xBC31:
case 0xBE31:
case 0xB032:
case 0xB232:
case 0xB432:
case 0xB632:
case 0xB832:
case 0xBA32:
case 0xBC32:
case 0xBE32:
case 0xB033:
case 0xB233:
case 0xB433:
case 0xB633:
case 0xB833:
case 0xBA33:
case 0xBC33:
case 0xBE33:
case 0xB034:
case 0xB234:
case 0xB434:
case 0xB634:
case 0xB834:
case 0xBA34:
case 0xBC34:
case 0xBE34:
case 0xB035:
case 0xB235:
case 0xB435:
case 0xB635:
case 0xB835:
case 0xBA35:
case 0xBC35:
case 0xBE35:
case 0xB036:
case 0xB236:
case 0xB436:
case 0xB636:
case 0xB836:
case 0xBA36:
case 0xBC36:
case 0xBE36:
case 0xB037:
case 0xB237:
case 0xB437:
case 0xB637:
case 0xB837:
case 0xBA37:
case 0xBC37:
case 0xBE37:

// CMP
case 0xB030:
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
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(14)
case 0xB238:
case 0xB438:
case 0xB638:
case 0xB838:
case 0xBA38:
case 0xBC38:
case 0xBE38:

// CMP
case 0xB038:
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
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(12)
case 0xB239:
case 0xB439:
case 0xB639:
case 0xB839:
case 0xBA39:
case 0xBC39:
case 0xBE39:

// CMP
case 0xB039:
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
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(16)
case 0xB23A:
case 0xB43A:
case 0xB63A:
case 0xB83A:
case 0xBA3A:
case 0xBC3A:
case 0xBE3A:

// CMP
case 0xB03A:
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
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(12)
case 0xB23B:
case 0xB43B:
case 0xB63B:
case 0xB83B:
case 0xBA3B:
case 0xBC3B:
case 0xBE3B:

// CMP
case 0xB03B:
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
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(14)
case 0xB23C:
case 0xB43C:
case 0xB63C:
case 0xB83C:
case 0xBA3C:
case 0xBC3C:
case 0xBE3C:

// CMP
case 0xB03C:
{
	u32 res;
	u32 dst;
	u32 src;
	src = FETCH_BYTE;
	PC += 2;
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
}
RET(8)
case 0xB21F:
case 0xB41F:
case 0xB61F:
case 0xB81F:
case 0xBA1F:
case 0xBC1F:
case 0xBE1F:

// CMP
case 0xB01F:
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
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(8)
case 0xB227:
case 0xB427:
case 0xB627:
case 0xB827:
case 0xBA27:
case 0xBC27:
case 0xBE27:

// CMP
case 0xB027:
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
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(10)
case 0xB240:
case 0xB440:
case 0xB640:
case 0xB840:
case 0xBA40:
case 0xBC40:
case 0xBE40:
case 0xB041:
case 0xB241:
case 0xB441:
case 0xB641:
case 0xB841:
case 0xBA41:
case 0xBC41:
case 0xBE41:
case 0xB042:
case 0xB242:
case 0xB442:
case 0xB642:
case 0xB842:
case 0xBA42:
case 0xBC42:
case 0xBE42:
case 0xB043:
case 0xB243:
case 0xB443:
case 0xB643:
case 0xB843:
case 0xBA43:
case 0xBC43:
case 0xBE43:
case 0xB044:
case 0xB244:
case 0xB444:
case 0xB644:
case 0xB844:
case 0xBA44:
case 0xBC44:
case 0xBE44:
case 0xB045:
case 0xB245:
case 0xB445:
case 0xB645:
case 0xB845:
case 0xBA45:
case 0xBC45:
case 0xBE45:
case 0xB046:
case 0xB246:
case 0xB446:
case 0xB646:
case 0xB846:
case 0xBA46:
case 0xBC46:
case 0xBE46:
case 0xB047:
case 0xB247:
case 0xB447:
case 0xB647:
case 0xB847:
case 0xBA47:
case 0xBC47:
case 0xBE47:

// CMP
case 0xB040:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
}
RET(4)
case 0xB248:
case 0xB448:
case 0xB648:
case 0xB848:
case 0xBA48:
case 0xBC48:
case 0xBE48:
case 0xB049:
case 0xB249:
case 0xB449:
case 0xB649:
case 0xB849:
case 0xBA49:
case 0xBC49:
case 0xBE49:
case 0xB04A:
case 0xB24A:
case 0xB44A:
case 0xB64A:
case 0xB84A:
case 0xBA4A:
case 0xBC4A:
case 0xBE4A:
case 0xB04B:
case 0xB24B:
case 0xB44B:
case 0xB64B:
case 0xB84B:
case 0xBA4B:
case 0xBC4B:
case 0xBE4B:
case 0xB04C:
case 0xB24C:
case 0xB44C:
case 0xB64C:
case 0xB84C:
case 0xBA4C:
case 0xBC4C:
case 0xBE4C:
case 0xB04D:
case 0xB24D:
case 0xB44D:
case 0xB64D:
case 0xB84D:
case 0xBA4D:
case 0xBC4D:
case 0xBE4D:
case 0xB04E:
case 0xB24E:
case 0xB44E:
case 0xB64E:
case 0xB84E:
case 0xBA4E:
case 0xBC4E:
case 0xBE4E:
case 0xB04F:
case 0xB24F:
case 0xB44F:
case 0xB64F:
case 0xB84F:
case 0xBA4F:
case 0xBC4F:
case 0xBE4F:

// CMP
case 0xB048:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->A[(Opcode >> 0) & 7];
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
}
RET(4)
case 0xB250:
case 0xB450:
case 0xB650:
case 0xB850:
case 0xBA50:
case 0xBC50:
case 0xBE50:
case 0xB051:
case 0xB251:
case 0xB451:
case 0xB651:
case 0xB851:
case 0xBA51:
case 0xBC51:
case 0xBE51:
case 0xB052:
case 0xB252:
case 0xB452:
case 0xB652:
case 0xB852:
case 0xBA52:
case 0xBC52:
case 0xBE52:
case 0xB053:
case 0xB253:
case 0xB453:
case 0xB653:
case 0xB853:
case 0xBA53:
case 0xBC53:
case 0xBE53:
case 0xB054:
case 0xB254:
case 0xB454:
case 0xB654:
case 0xB854:
case 0xBA54:
case 0xBC54:
case 0xBE54:
case 0xB055:
case 0xB255:
case 0xB455:
case 0xB655:
case 0xB855:
case 0xBA55:
case 0xBC55:
case 0xBE55:
case 0xB056:
case 0xB256:
case 0xB456:
case 0xB656:
case 0xB856:
case 0xBA56:
case 0xBC56:
case 0xBE56:
case 0xB057:
case 0xB257:
case 0xB457:
case 0xB657:
case 0xB857:
case 0xBA57:
case 0xBC57:
case 0xBE57:

// CMP
case 0xB050:
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
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(8)
case 0xB258:
case 0xB458:
case 0xB658:
case 0xB858:
case 0xBA58:
case 0xBC58:
case 0xBE58:
case 0xB059:
case 0xB259:
case 0xB459:
case 0xB659:
case 0xB859:
case 0xBA59:
case 0xBC59:
case 0xBE59:
case 0xB05A:
case 0xB25A:
case 0xB45A:
case 0xB65A:
case 0xB85A:
case 0xBA5A:
case 0xBC5A:
case 0xBE5A:
case 0xB05B:
case 0xB25B:
case 0xB45B:
case 0xB65B:
case 0xB85B:
case 0xBA5B:
case 0xBC5B:
case 0xBE5B:
case 0xB05C:
case 0xB25C:
case 0xB45C:
case 0xB65C:
case 0xB85C:
case 0xBA5C:
case 0xBC5C:
case 0xBE5C:
case 0xB05D:
case 0xB25D:
case 0xB45D:
case 0xB65D:
case 0xB85D:
case 0xBA5D:
case 0xBC5D:
case 0xBE5D:
case 0xB05E:
case 0xB25E:
case 0xB45E:
case 0xB65E:
case 0xB85E:
case 0xBA5E:
case 0xBC5E:
case 0xBE5E:

// CMP
case 0xB058:
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
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(8)
case 0xB260:
case 0xB460:
case 0xB660:
case 0xB860:
case 0xBA60:
case 0xBC60:
case 0xBE60:
case 0xB061:
case 0xB261:
case 0xB461:
case 0xB661:
case 0xB861:
case 0xBA61:
case 0xBC61:
case 0xBE61:
case 0xB062:
case 0xB262:
case 0xB462:
case 0xB662:
case 0xB862:
case 0xBA62:
case 0xBC62:
case 0xBE62:
case 0xB063:
case 0xB263:
case 0xB463:
case 0xB663:
case 0xB863:
case 0xBA63:
case 0xBC63:
case 0xBE63:
case 0xB064:
case 0xB264:
case 0xB464:
case 0xB664:
case 0xB864:
case 0xBA64:
case 0xBC64:
case 0xBE64:
case 0xB065:
case 0xB265:
case 0xB465:
case 0xB665:
case 0xB865:
case 0xBA65:
case 0xBC65:
case 0xBE65:
case 0xB066:
case 0xB266:
case 0xB466:
case 0xB666:
case 0xB866:
case 0xBA66:
case 0xBC66:
case 0xBE66:

// CMP
case 0xB060:
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
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(10)
case 0xB268:
case 0xB468:
case 0xB668:
case 0xB868:
case 0xBA68:
case 0xBC68:
case 0xBE68:
case 0xB069:
case 0xB269:
case 0xB469:
case 0xB669:
case 0xB869:
case 0xBA69:
case 0xBC69:
case 0xBE69:
case 0xB06A:
case 0xB26A:
case 0xB46A:
case 0xB66A:
case 0xB86A:
case 0xBA6A:
case 0xBC6A:
case 0xBE6A:
case 0xB06B:
case 0xB26B:
case 0xB46B:
case 0xB66B:
case 0xB86B:
case 0xBA6B:
case 0xBC6B:
case 0xBE6B:
case 0xB06C:
case 0xB26C:
case 0xB46C:
case 0xB66C:
case 0xB86C:
case 0xBA6C:
case 0xBC6C:
case 0xBE6C:
case 0xB06D:
case 0xB26D:
case 0xB46D:
case 0xB66D:
case 0xB86D:
case 0xBA6D:
case 0xBC6D:
case 0xBE6D:
case 0xB06E:
case 0xB26E:
case 0xB46E:
case 0xB66E:
case 0xB86E:
case 0xBA6E:
case 0xBC6E:
case 0xBE6E:
case 0xB06F:
case 0xB26F:
case 0xB46F:
case 0xB66F:
case 0xB86F:
case 0xBA6F:
case 0xBC6F:
case 0xBE6F:

// CMP
case 0xB068:
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
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(12)
case 0xB270:
case 0xB470:
case 0xB670:
case 0xB870:
case 0xBA70:
case 0xBC70:
case 0xBE70:
case 0xB071:
case 0xB271:
case 0xB471:
case 0xB671:
case 0xB871:
case 0xBA71:
case 0xBC71:
case 0xBE71:
case 0xB072:
case 0xB272:
case 0xB472:
case 0xB672:
case 0xB872:
case 0xBA72:
case 0xBC72:
case 0xBE72:
case 0xB073:
case 0xB273:
case 0xB473:
case 0xB673:
case 0xB873:
case 0xBA73:
case 0xBC73:
case 0xBE73:
case 0xB074:
case 0xB274:
case 0xB474:
case 0xB674:
case 0xB874:
case 0xBA74:
case 0xBC74:
case 0xBE74:
case 0xB075:
case 0xB275:
case 0xB475:
case 0xB675:
case 0xB875:
case 0xBA75:
case 0xBC75:
case 0xBE75:
case 0xB076:
case 0xB276:
case 0xB476:
case 0xB676:
case 0xB876:
case 0xBA76:
case 0xBC76:
case 0xBE76:
case 0xB077:
case 0xB277:
case 0xB477:
case 0xB677:
case 0xB877:
case 0xBA77:
case 0xBC77:
case 0xBE77:

// CMP
case 0xB070:
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
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(14)
case 0xB278:
case 0xB478:
case 0xB678:
case 0xB878:
case 0xBA78:
case 0xBC78:
case 0xBE78:

// CMP
case 0xB078:
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
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(12)
case 0xB279:
case 0xB479:
case 0xB679:
case 0xB879:
case 0xBA79:
case 0xBC79:
case 0xBE79:

// CMP
case 0xB079:
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
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(16)
case 0xB27A:
case 0xB47A:
case 0xB67A:
case 0xB87A:
case 0xBA7A:
case 0xBC7A:
case 0xBE7A:

// CMP
case 0xB07A:
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
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(12)
case 0xB27B:
case 0xB47B:
case 0xB67B:
case 0xB87B:
case 0xBA7B:
case 0xBC7B:
case 0xBE7B:

// CMP
case 0xB07B:
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
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(14)
case 0xB27C:
case 0xB47C:
case 0xB67C:
case 0xB87C:
case 0xBA7C:
case 0xBC7C:
case 0xBE7C:

// CMP
case 0xB07C:
{
	u32 res;
	u32 dst;
	u32 src;
	src = FETCH_WORD;
	PC += 2;
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
}
RET(8)
case 0xB25F:
case 0xB45F:
case 0xB65F:
case 0xB85F:
case 0xBA5F:
case 0xBC5F:
case 0xBE5F:

// CMP
case 0xB05F:
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
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(8)
case 0xB267:
case 0xB467:
case 0xB667:
case 0xB867:
case 0xBA67:
case 0xBC67:
case 0xBE67:

// CMP
case 0xB067:
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
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(10)
case 0xB280:
case 0xB480:
case 0xB680:
case 0xB880:
case 0xBA80:
case 0xBC80:
case 0xBE80:
case 0xB081:
case 0xB281:
case 0xB481:
case 0xB681:
case 0xB881:
case 0xBA81:
case 0xBC81:
case 0xBE81:
case 0xB082:
case 0xB282:
case 0xB482:
case 0xB682:
case 0xB882:
case 0xBA82:
case 0xBC82:
case 0xBE82:
case 0xB083:
case 0xB283:
case 0xB483:
case 0xB683:
case 0xB883:
case 0xBA83:
case 0xBC83:
case 0xBE83:
case 0xB084:
case 0xB284:
case 0xB484:
case 0xB684:
case 0xB884:
case 0xBA84:
case 0xBC84:
case 0xBE84:
case 0xB085:
case 0xB285:
case 0xB485:
case 0xB685:
case 0xB885:
case 0xBA85:
case 0xBC85:
case 0xBE85:
case 0xB086:
case 0xB286:
case 0xB486:
case 0xB686:
case 0xB886:
case 0xBA86:
case 0xBC86:
case 0xBE86:
case 0xB087:
case 0xB287:
case 0xB487:
case 0xB687:
case 0xB887:
case 0xBA87:
case 0xBC87:
case 0xBE87:

// CMP
case 0xB080:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(6)
case 0xB288:
case 0xB488:
case 0xB688:
case 0xB888:
case 0xBA88:
case 0xBC88:
case 0xBE88:
case 0xB089:
case 0xB289:
case 0xB489:
case 0xB689:
case 0xB889:
case 0xBA89:
case 0xBC89:
case 0xBE89:
case 0xB08A:
case 0xB28A:
case 0xB48A:
case 0xB68A:
case 0xB88A:
case 0xBA8A:
case 0xBC8A:
case 0xBE8A:
case 0xB08B:
case 0xB28B:
case 0xB48B:
case 0xB68B:
case 0xB88B:
case 0xBA8B:
case 0xBC8B:
case 0xBE8B:
case 0xB08C:
case 0xB28C:
case 0xB48C:
case 0xB68C:
case 0xB88C:
case 0xBA8C:
case 0xBC8C:
case 0xBE8C:
case 0xB08D:
case 0xB28D:
case 0xB48D:
case 0xB68D:
case 0xB88D:
case 0xBA8D:
case 0xBC8D:
case 0xBE8D:
case 0xB08E:
case 0xB28E:
case 0xB48E:
case 0xB68E:
case 0xB88E:
case 0xBA8E:
case 0xBC8E:
case 0xBE8E:
case 0xB08F:
case 0xB28F:
case 0xB48F:
case 0xB68F:
case 0xB88F:
case 0xBA8F:
case 0xBC8F:
case 0xBE8F:

// CMP
case 0xB088:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->A[(Opcode >> 0) & 7];
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(6)
case 0xB290:
case 0xB490:
case 0xB690:
case 0xB890:
case 0xBA90:
case 0xBC90:
case 0xBE90:
case 0xB091:
case 0xB291:
case 0xB491:
case 0xB691:
case 0xB891:
case 0xBA91:
case 0xBC91:
case 0xBE91:
case 0xB092:
case 0xB292:
case 0xB492:
case 0xB692:
case 0xB892:
case 0xBA92:
case 0xBC92:
case 0xBE92:
case 0xB093:
case 0xB293:
case 0xB493:
case 0xB693:
case 0xB893:
case 0xBA93:
case 0xBC93:
case 0xBE93:
case 0xB094:
case 0xB294:
case 0xB494:
case 0xB694:
case 0xB894:
case 0xBA94:
case 0xBC94:
case 0xBE94:
case 0xB095:
case 0xB295:
case 0xB495:
case 0xB695:
case 0xB895:
case 0xBA95:
case 0xBC95:
case 0xBE95:
case 0xB096:
case 0xB296:
case 0xB496:
case 0xB696:
case 0xB896:
case 0xBA96:
case 0xBC96:
case 0xBE96:
case 0xB097:
case 0xB297:
case 0xB497:
case 0xB697:
case 0xB897:
case 0xBA97:
case 0xBC97:
case 0xBE97:

// CMP
case 0xB090:
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
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(14)
case 0xB298:
case 0xB498:
case 0xB698:
case 0xB898:
case 0xBA98:
case 0xBC98:
case 0xBE98:
case 0xB099:
case 0xB299:
case 0xB499:
case 0xB699:
case 0xB899:
case 0xBA99:
case 0xBC99:
case 0xBE99:
case 0xB09A:
case 0xB29A:
case 0xB49A:
case 0xB69A:
case 0xB89A:
case 0xBA9A:
case 0xBC9A:
case 0xBE9A:
case 0xB09B:
case 0xB29B:
case 0xB49B:
case 0xB69B:
case 0xB89B:
case 0xBA9B:
case 0xBC9B:
case 0xBE9B:
case 0xB09C:
case 0xB29C:
case 0xB49C:
case 0xB69C:
case 0xB89C:
case 0xBA9C:
case 0xBC9C:
case 0xBE9C:
case 0xB09D:
case 0xB29D:
case 0xB49D:
case 0xB69D:
case 0xB89D:
case 0xBA9D:
case 0xBC9D:
case 0xBE9D:
case 0xB09E:
case 0xB29E:
case 0xB49E:
case 0xB69E:
case 0xB89E:
case 0xBA9E:
case 0xBC9E:
case 0xBE9E:

// CMP
case 0xB098:
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
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(14)
case 0xB2A0:
case 0xB4A0:
case 0xB6A0:
case 0xB8A0:
case 0xBAA0:
case 0xBCA0:
case 0xBEA0:
case 0xB0A1:
case 0xB2A1:
case 0xB4A1:
case 0xB6A1:
case 0xB8A1:
case 0xBAA1:
case 0xBCA1:
case 0xBEA1:
case 0xB0A2:
case 0xB2A2:
case 0xB4A2:
case 0xB6A2:
case 0xB8A2:
case 0xBAA2:
case 0xBCA2:
case 0xBEA2:
case 0xB0A3:
case 0xB2A3:
case 0xB4A3:
case 0xB6A3:
case 0xB8A3:
case 0xBAA3:
case 0xBCA3:
case 0xBEA3:
case 0xB0A4:
case 0xB2A4:
case 0xB4A4:
case 0xB6A4:
case 0xB8A4:
case 0xBAA4:
case 0xBCA4:
case 0xBEA4:
case 0xB0A5:
case 0xB2A5:
case 0xB4A5:
case 0xB6A5:
case 0xB8A5:
case 0xBAA5:
case 0xBCA5:
case 0xBEA5:
case 0xB0A6:
case 0xB2A6:
case 0xB4A6:
case 0xB6A6:
case 0xB8A6:
case 0xBAA6:
case 0xBCA6:
case 0xBEA6:

// CMP
case 0xB0A0:
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
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(16)
case 0xB2A8:
case 0xB4A8:
case 0xB6A8:
case 0xB8A8:
case 0xBAA8:
case 0xBCA8:
case 0xBEA8:
case 0xB0A9:
case 0xB2A9:
case 0xB4A9:
case 0xB6A9:
case 0xB8A9:
case 0xBAA9:
case 0xBCA9:
case 0xBEA9:
case 0xB0AA:
case 0xB2AA:
case 0xB4AA:
case 0xB6AA:
case 0xB8AA:
case 0xBAAA:
case 0xBCAA:
case 0xBEAA:
case 0xB0AB:
case 0xB2AB:
case 0xB4AB:
case 0xB6AB:
case 0xB8AB:
case 0xBAAB:
case 0xBCAB:
case 0xBEAB:
case 0xB0AC:
case 0xB2AC:
case 0xB4AC:
case 0xB6AC:
case 0xB8AC:
case 0xBAAC:
case 0xBCAC:
case 0xBEAC:
case 0xB0AD:
case 0xB2AD:
case 0xB4AD:
case 0xB6AD:
case 0xB8AD:
case 0xBAAD:
case 0xBCAD:
case 0xBEAD:
case 0xB0AE:
case 0xB2AE:
case 0xB4AE:
case 0xB6AE:
case 0xB8AE:
case 0xBAAE:
case 0xBCAE:
case 0xBEAE:
case 0xB0AF:
case 0xB2AF:
case 0xB4AF:
case 0xB6AF:
case 0xB8AF:
case 0xBAAF:
case 0xBCAF:
case 0xBEAF:

// CMP
case 0xB0A8:
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
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(18)
case 0xB2B0:
case 0xB4B0:
case 0xB6B0:
case 0xB8B0:
case 0xBAB0:
case 0xBCB0:
case 0xBEB0:
case 0xB0B1:
case 0xB2B1:
case 0xB4B1:
case 0xB6B1:
case 0xB8B1:
case 0xBAB1:
case 0xBCB1:
case 0xBEB1:
case 0xB0B2:
case 0xB2B2:
case 0xB4B2:
case 0xB6B2:
case 0xB8B2:
case 0xBAB2:
case 0xBCB2:
case 0xBEB2:
case 0xB0B3:
case 0xB2B3:
case 0xB4B3:
case 0xB6B3:
case 0xB8B3:
case 0xBAB3:
case 0xBCB3:
case 0xBEB3:
case 0xB0B4:
case 0xB2B4:
case 0xB4B4:
case 0xB6B4:
case 0xB8B4:
case 0xBAB4:
case 0xBCB4:
case 0xBEB4:
case 0xB0B5:
case 0xB2B5:
case 0xB4B5:
case 0xB6B5:
case 0xB8B5:
case 0xBAB5:
case 0xBCB5:
case 0xBEB5:
case 0xB0B6:
case 0xB2B6:
case 0xB4B6:
case 0xB6B6:
case 0xB8B6:
case 0xBAB6:
case 0xBCB6:
case 0xBEB6:
case 0xB0B7:
case 0xB2B7:
case 0xB4B7:
case 0xB6B7:
case 0xB8B7:
case 0xBAB7:
case 0xBCB7:
case 0xBEB7:

// CMP
case 0xB0B0:
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
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(20)
case 0xB2B8:
case 0xB4B8:
case 0xB6B8:
case 0xB8B8:
case 0xBAB8:
case 0xBCB8:
case 0xBEB8:

// CMP
case 0xB0B8:
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
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(18)
case 0xB2B9:
case 0xB4B9:
case 0xB6B9:
case 0xB8B9:
case 0xBAB9:
case 0xBCB9:
case 0xBEB9:

// CMP
case 0xB0B9:
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
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(22)
case 0xB2BA:
case 0xB4BA:
case 0xB6BA:
case 0xB8BA:
case 0xBABA:
case 0xBCBA:
case 0xBEBA:

// CMP
case 0xB0BA:
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
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(18)
case 0xB2BB:
case 0xB4BB:
case 0xB6BB:
case 0xB8BB:
case 0xBABB:
case 0xBCBB:
case 0xBEBB:

// CMP
case 0xB0BB:
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
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(20)
case 0xB2BC:
case 0xB4BC:
case 0xB6BC:
case 0xB8BC:
case 0xBABC:
case 0xBCBC:
case 0xBEBC:

// CMP
case 0xB0BC:
{
	u32 res;
	u32 dst;
	u32 src;
	src = FETCH_LONG;
	PC += 4;
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(14)
case 0xB29F:
case 0xB49F:
case 0xB69F:
case 0xB89F:
case 0xBA9F:
case 0xBC9F:
case 0xBE9F:

// CMP
case 0xB09F:
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
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(14)
case 0xB2A7:
case 0xB4A7:
case 0xB6A7:
case 0xB8A7:
case 0xBAA7:
case 0xBCA7:
case 0xBEA7:

// CMP
case 0xB0A7:
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
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(16)
case 0xB308:
case 0xB508:
case 0xB708:
case 0xB908:
case 0xBB08:
case 0xBD08:
case 0xB109:
case 0xB309:
case 0xB509:
case 0xB709:
case 0xB909:
case 0xBB09:
case 0xBD09:
case 0xB10A:
case 0xB30A:
case 0xB50A:
case 0xB70A:
case 0xB90A:
case 0xBB0A:
case 0xBD0A:
case 0xB10B:
case 0xB30B:
case 0xB50B:
case 0xB70B:
case 0xB90B:
case 0xBB0B:
case 0xBD0B:
case 0xB10C:
case 0xB30C:
case 0xB50C:
case 0xB70C:
case 0xB90C:
case 0xBB0C:
case 0xBD0C:
case 0xB10D:
case 0xB30D:
case 0xB50D:
case 0xB70D:
case 0xB90D:
case 0xBB0D:
case 0xBD0D:
case 0xB10E:
case 0xB30E:
case 0xB50E:
case 0xB70E:
case 0xB90E:
case 0xBB0E:
case 0xBD0E:

// CMPM
case 0xB108:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, src)
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	READ_BYTE_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(12)
case 0xB348:
case 0xB548:
case 0xB748:
case 0xB948:
case 0xBB48:
case 0xBD48:
case 0xB149:
case 0xB349:
case 0xB549:
case 0xB749:
case 0xB949:
case 0xBB49:
case 0xBD49:
case 0xB14A:
case 0xB34A:
case 0xB54A:
case 0xB74A:
case 0xB94A:
case 0xBB4A:
case 0xBD4A:
case 0xB14B:
case 0xB34B:
case 0xB54B:
case 0xB74B:
case 0xB94B:
case 0xBB4B:
case 0xBD4B:
case 0xB14C:
case 0xB34C:
case 0xB54C:
case 0xB74C:
case 0xB94C:
case 0xBB4C:
case 0xBD4C:
case 0xB14D:
case 0xB34D:
case 0xB54D:
case 0xB74D:
case 0xB94D:
case 0xBB4D:
case 0xBD4D:
case 0xB14E:
case 0xB34E:
case 0xB54E:
case 0xB74E:
case 0xB94E:
case 0xBB4E:
case 0xBD4E:

// CMPM
case 0xB148:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	READ_WORD_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(12)
case 0xB388:
case 0xB588:
case 0xB788:
case 0xB988:
case 0xBB88:
case 0xBD88:
case 0xB189:
case 0xB389:
case 0xB589:
case 0xB789:
case 0xB989:
case 0xBB89:
case 0xBD89:
case 0xB18A:
case 0xB38A:
case 0xB58A:
case 0xB78A:
case 0xB98A:
case 0xBB8A:
case 0xBD8A:
case 0xB18B:
case 0xB38B:
case 0xB58B:
case 0xB78B:
case 0xB98B:
case 0xBB8B:
case 0xBD8B:
case 0xB18C:
case 0xB38C:
case 0xB58C:
case 0xB78C:
case 0xB98C:
case 0xBB8C:
case 0xBD8C:
case 0xB18D:
case 0xB38D:
case 0xB58D:
case 0xB78D:
case 0xB98D:
case 0xBB8D:
case 0xBD8D:
case 0xB18E:
case 0xB38E:
case 0xB58E:
case 0xB78E:
case 0xB98E:
case 0xBB8E:
case 0xBD8E:

// CMPM
case 0xB188:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	READ_LONG_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(20)
case 0xB30F:
case 0xB50F:
case 0xB70F:
case 0xB90F:
case 0xBB0F:
case 0xBD0F:

// CMP7M
case 0xB10F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	READ_BYTE_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(12)
case 0xB34F:
case 0xB54F:
case 0xB74F:
case 0xB94F:
case 0xBB4F:
case 0xBD4F:

// CMP7M
case 0xB14F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	READ_WORD_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(12)
case 0xB38F:
case 0xB58F:
case 0xB78F:
case 0xB98F:
case 0xBB8F:
case 0xBD8F:

// CMP7M
case 0xB18F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	READ_LONG_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(20)
case 0xBF09:
case 0xBF0A:
case 0xBF0B:
case 0xBF0C:
case 0xBF0D:
case 0xBF0E:

// CMPM7
case 0xBF08:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, src)
	adr = CPU->A[7];
	CPU->A[7] += 2;
	READ_BYTE_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(12)
case 0xBF49:
case 0xBF4A:
case 0xBF4B:
case 0xBF4C:
case 0xBF4D:
case 0xBF4E:

// CMPM7
case 0xBF48:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	adr = CPU->A[7];
	CPU->A[7] += 2;
	READ_WORD_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(12)
case 0xBF89:
case 0xBF8A:
case 0xBF8B:
case 0xBF8C:
case 0xBF8D:
case 0xBF8E:

// CMPM7
case 0xBF88:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	adr = CPU->A[7];
	CPU->A[7] += 4;
	READ_LONG_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(20)

// CMP7M7
case 0xBF0F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	adr = CPU->A[7];
	CPU->A[7] += 2;
	READ_BYTE_F(adr, dst)
	res = dst - src;
	CPU->flag_N = CPU->flag_C = res;
	CPU->flag_V = (src ^ dst) & (res ^ dst);
	CPU->flag_notZ = res & 0xFF;
	POST_IO
}
RET(12)

// CMP7M7
case 0xBF4F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	adr = CPU->A[7];
	CPU->A[7] += 2;
	READ_WORD_F(adr, dst)
	res = dst - src;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;
	CPU->flag_N = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	POST_IO
}
RET(12)

// CMP7M7
case 0xBF8F:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	adr = CPU->A[7];
	CPU->A[7] += 4;
	READ_LONG_F(adr, dst)
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(20)
case 0xB300:
case 0xB500:
case 0xB700:
case 0xB900:
case 0xBB00:
case 0xBD00:
case 0xBF00:
case 0xB101:
case 0xB301:
case 0xB501:
case 0xB701:
case 0xB901:
case 0xBB01:
case 0xBD01:
case 0xBF01:
case 0xB102:
case 0xB302:
case 0xB502:
case 0xB702:
case 0xB902:
case 0xBB02:
case 0xBD02:
case 0xBF02:
case 0xB103:
case 0xB303:
case 0xB503:
case 0xB703:
case 0xB903:
case 0xBB03:
case 0xBD03:
case 0xBF03:
case 0xB104:
case 0xB304:
case 0xB504:
case 0xB704:
case 0xB904:
case 0xBB04:
case 0xBD04:
case 0xBF04:
case 0xB105:
case 0xB305:
case 0xB505:
case 0xB705:
case 0xB905:
case 0xBB05:
case 0xBD05:
case 0xBF05:
case 0xB106:
case 0xB306:
case 0xB506:
case 0xB706:
case 0xB906:
case 0xBB06:
case 0xBD06:
case 0xBF06:
case 0xB107:
case 0xB307:
case 0xB507:
case 0xB707:
case 0xB907:
case 0xBB07:
case 0xBD07:
case 0xBF07:

// EORDa
case 0xB100:
{
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0xB310:
case 0xB510:
case 0xB710:
case 0xB910:
case 0xBB10:
case 0xBD10:
case 0xBF10:
case 0xB111:
case 0xB311:
case 0xB511:
case 0xB711:
case 0xB911:
case 0xBB11:
case 0xBD11:
case 0xBF11:
case 0xB112:
case 0xB312:
case 0xB512:
case 0xB712:
case 0xB912:
case 0xBB12:
case 0xBD12:
case 0xBF12:
case 0xB113:
case 0xB313:
case 0xB513:
case 0xB713:
case 0xB913:
case 0xBB13:
case 0xBD13:
case 0xBF13:
case 0xB114:
case 0xB314:
case 0xB514:
case 0xB714:
case 0xB914:
case 0xBB14:
case 0xBD14:
case 0xBF14:
case 0xB115:
case 0xB315:
case 0xB515:
case 0xB715:
case 0xB915:
case 0xBB15:
case 0xBD15:
case 0xBF15:
case 0xB116:
case 0xB316:
case 0xB516:
case 0xB716:
case 0xB916:
case 0xBB16:
case 0xBD16:
case 0xBF16:
case 0xB117:
case 0xB317:
case 0xB517:
case 0xB717:
case 0xB917:
case 0xBB17:
case 0xBD17:
case 0xBF17:

// EORDa
case 0xB110:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0xB318:
case 0xB518:
case 0xB718:
case 0xB918:
case 0xBB18:
case 0xBD18:
case 0xBF18:
case 0xB119:
case 0xB319:
case 0xB519:
case 0xB719:
case 0xB919:
case 0xBB19:
case 0xBD19:
case 0xBF19:
case 0xB11A:
case 0xB31A:
case 0xB51A:
case 0xB71A:
case 0xB91A:
case 0xBB1A:
case 0xBD1A:
case 0xBF1A:
case 0xB11B:
case 0xB31B:
case 0xB51B:
case 0xB71B:
case 0xB91B:
case 0xBB1B:
case 0xBD1B:
case 0xBF1B:
case 0xB11C:
case 0xB31C:
case 0xB51C:
case 0xB71C:
case 0xB91C:
case 0xBB1C:
case 0xBD1C:
case 0xBF1C:
case 0xB11D:
case 0xB31D:
case 0xB51D:
case 0xB71D:
case 0xB91D:
case 0xBB1D:
case 0xBD1D:
case 0xBF1D:
case 0xB11E:
case 0xB31E:
case 0xB51E:
case 0xB71E:
case 0xB91E:
case 0xBB1E:
case 0xBD1E:
case 0xBF1E:

// EORDa
case 0xB118:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0xB320:
case 0xB520:
case 0xB720:
case 0xB920:
case 0xBB20:
case 0xBD20:
case 0xBF20:
case 0xB121:
case 0xB321:
case 0xB521:
case 0xB721:
case 0xB921:
case 0xBB21:
case 0xBD21:
case 0xBF21:
case 0xB122:
case 0xB322:
case 0xB522:
case 0xB722:
case 0xB922:
case 0xBB22:
case 0xBD22:
case 0xBF22:
case 0xB123:
case 0xB323:
case 0xB523:
case 0xB723:
case 0xB923:
case 0xBB23:
case 0xBD23:
case 0xBF23:
case 0xB124:
case 0xB324:
case 0xB524:
case 0xB724:
case 0xB924:
case 0xBB24:
case 0xBD24:
case 0xBF24:
case 0xB125:
case 0xB325:
case 0xB525:
case 0xB725:
case 0xB925:
case 0xBB25:
case 0xBD25:
case 0xBF25:
case 0xB126:
case 0xB326:
case 0xB526:
case 0xB726:
case 0xB926:
case 0xBB26:
case 0xBD26:
case 0xBF26:

// EORDa
case 0xB120:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0xB328:
case 0xB528:
case 0xB728:
case 0xB928:
case 0xBB28:
case 0xBD28:
case 0xBF28:
case 0xB129:
case 0xB329:
case 0xB529:
case 0xB729:
case 0xB929:
case 0xBB29:
case 0xBD29:
case 0xBF29:
case 0xB12A:
case 0xB32A:
case 0xB52A:
case 0xB72A:
case 0xB92A:
case 0xBB2A:
case 0xBD2A:
case 0xBF2A:
case 0xB12B:
case 0xB32B:
case 0xB52B:
case 0xB72B:
case 0xB92B:
case 0xBB2B:
case 0xBD2B:
case 0xBF2B:
case 0xB12C:
case 0xB32C:
case 0xB52C:
case 0xB72C:
case 0xB92C:
case 0xBB2C:
case 0xBD2C:
case 0xBF2C:
case 0xB12D:
case 0xB32D:
case 0xB52D:
case 0xB72D:
case 0xB92D:
case 0xBB2D:
case 0xBD2D:
case 0xBF2D:
case 0xB12E:
case 0xB32E:
case 0xB52E:
case 0xB72E:
case 0xB92E:
case 0xBB2E:
case 0xBD2E:
case 0xBF2E:
case 0xB12F:
case 0xB32F:
case 0xB52F:
case 0xB72F:
case 0xB92F:
case 0xBB2F:
case 0xBD2F:
case 0xBF2F:

// EORDa
case 0xB128:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0xB330:
case 0xB530:
case 0xB730:
case 0xB930:
case 0xBB30:
case 0xBD30:
case 0xBF30:
case 0xB131:
case 0xB331:
case 0xB531:
case 0xB731:
case 0xB931:
case 0xBB31:
case 0xBD31:
case 0xBF31:
case 0xB132:
case 0xB332:
case 0xB532:
case 0xB732:
case 0xB932:
case 0xBB32:
case 0xBD32:
case 0xBF32:
case 0xB133:
case 0xB333:
case 0xB533:
case 0xB733:
case 0xB933:
case 0xBB33:
case 0xBD33:
case 0xBF33:
case 0xB134:
case 0xB334:
case 0xB534:
case 0xB734:
case 0xB934:
case 0xBB34:
case 0xBD34:
case 0xBF34:
case 0xB135:
case 0xB335:
case 0xB535:
case 0xB735:
case 0xB935:
case 0xBB35:
case 0xBD35:
case 0xBF35:
case 0xB136:
case 0xB336:
case 0xB536:
case 0xB736:
case 0xB936:
case 0xBB36:
case 0xBD36:
case 0xBF36:
case 0xB137:
case 0xB337:
case 0xB537:
case 0xB737:
case 0xB937:
case 0xBB37:
case 0xBD37:
case 0xBF37:

// EORDa
case 0xB130:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0xB338:
case 0xB538:
case 0xB738:
case 0xB938:
case 0xBB38:
case 0xBD38:
case 0xBF38:

// EORDa
case 0xB138:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0xB339:
case 0xB539:
case 0xB739:
case 0xB939:
case 0xBB39:
case 0xBD39:
case 0xBF39:

// EORDa
case 0xB139:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_BYTE_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0xB31F:
case 0xB51F:
case 0xB71F:
case 0xB91F:
case 0xBB1F:
case 0xBD1F:
case 0xBF1F:

// EORDa
case 0xB11F:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0xB327:
case 0xB527:
case 0xB727:
case 0xB927:
case 0xBB27:
case 0xBD27:
case 0xBF27:

// EORDa
case 0xB127:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0xB340:
case 0xB540:
case 0xB740:
case 0xB940:
case 0xBB40:
case 0xBD40:
case 0xBF40:
case 0xB141:
case 0xB341:
case 0xB541:
case 0xB741:
case 0xB941:
case 0xBB41:
case 0xBD41:
case 0xBF41:
case 0xB142:
case 0xB342:
case 0xB542:
case 0xB742:
case 0xB942:
case 0xBB42:
case 0xBD42:
case 0xBF42:
case 0xB143:
case 0xB343:
case 0xB543:
case 0xB743:
case 0xB943:
case 0xBB43:
case 0xBD43:
case 0xBF43:
case 0xB144:
case 0xB344:
case 0xB544:
case 0xB744:
case 0xB944:
case 0xBB44:
case 0xBD44:
case 0xBF44:
case 0xB145:
case 0xB345:
case 0xB545:
case 0xB745:
case 0xB945:
case 0xBB45:
case 0xBD45:
case 0xBF45:
case 0xB146:
case 0xB346:
case 0xB546:
case 0xB746:
case 0xB946:
case 0xBB46:
case 0xBD46:
case 0xBF46:
case 0xB147:
case 0xB347:
case 0xB547:
case 0xB747:
case 0xB947:
case 0xBB47:
case 0xBD47:
case 0xBF47:

// EORDa
case 0xB140:
{
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(4)
case 0xB350:
case 0xB550:
case 0xB750:
case 0xB950:
case 0xBB50:
case 0xBD50:
case 0xBF50:
case 0xB151:
case 0xB351:
case 0xB551:
case 0xB751:
case 0xB951:
case 0xBB51:
case 0xBD51:
case 0xBF51:
case 0xB152:
case 0xB352:
case 0xB552:
case 0xB752:
case 0xB952:
case 0xBB52:
case 0xBD52:
case 0xBF52:
case 0xB153:
case 0xB353:
case 0xB553:
case 0xB753:
case 0xB953:
case 0xBB53:
case 0xBD53:
case 0xBF53:
case 0xB154:
case 0xB354:
case 0xB554:
case 0xB754:
case 0xB954:
case 0xBB54:
case 0xBD54:
case 0xBF54:
case 0xB155:
case 0xB355:
case 0xB555:
case 0xB755:
case 0xB955:
case 0xBB55:
case 0xBD55:
case 0xBF55:
case 0xB156:
case 0xB356:
case 0xB556:
case 0xB756:
case 0xB956:
case 0xBB56:
case 0xBD56:
case 0xBF56:
case 0xB157:
case 0xB357:
case 0xB557:
case 0xB757:
case 0xB957:
case 0xBB57:
case 0xBD57:
case 0xBF57:

// EORDa
case 0xB150:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xB358:
case 0xB558:
case 0xB758:
case 0xB958:
case 0xBB58:
case 0xBD58:
case 0xBF58:
case 0xB159:
case 0xB359:
case 0xB559:
case 0xB759:
case 0xB959:
case 0xBB59:
case 0xBD59:
case 0xBF59:
case 0xB15A:
case 0xB35A:
case 0xB55A:
case 0xB75A:
case 0xB95A:
case 0xBB5A:
case 0xBD5A:
case 0xBF5A:
case 0xB15B:
case 0xB35B:
case 0xB55B:
case 0xB75B:
case 0xB95B:
case 0xBB5B:
case 0xBD5B:
case 0xBF5B:
case 0xB15C:
case 0xB35C:
case 0xB55C:
case 0xB75C:
case 0xB95C:
case 0xBB5C:
case 0xBD5C:
case 0xBF5C:
case 0xB15D:
case 0xB35D:
case 0xB55D:
case 0xB75D:
case 0xB95D:
case 0xBB5D:
case 0xBD5D:
case 0xBF5D:
case 0xB15E:
case 0xB35E:
case 0xB55E:
case 0xB75E:
case 0xB95E:
case 0xBB5E:
case 0xBD5E:
case 0xBF5E:

// EORDa
case 0xB158:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xB360:
case 0xB560:
case 0xB760:
case 0xB960:
case 0xBB60:
case 0xBD60:
case 0xBF60:
case 0xB161:
case 0xB361:
case 0xB561:
case 0xB761:
case 0xB961:
case 0xBB61:
case 0xBD61:
case 0xBF61:
case 0xB162:
case 0xB362:
case 0xB562:
case 0xB762:
case 0xB962:
case 0xBB62:
case 0xBD62:
case 0xBF62:
case 0xB163:
case 0xB363:
case 0xB563:
case 0xB763:
case 0xB963:
case 0xBB63:
case 0xBD63:
case 0xBF63:
case 0xB164:
case 0xB364:
case 0xB564:
case 0xB764:
case 0xB964:
case 0xBB64:
case 0xBD64:
case 0xBF64:
case 0xB165:
case 0xB365:
case 0xB565:
case 0xB765:
case 0xB965:
case 0xBB65:
case 0xBD65:
case 0xBF65:
case 0xB166:
case 0xB366:
case 0xB566:
case 0xB766:
case 0xB966:
case 0xBB66:
case 0xBD66:
case 0xBF66:

// EORDa
case 0xB160:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xB368:
case 0xB568:
case 0xB768:
case 0xB968:
case 0xBB68:
case 0xBD68:
case 0xBF68:
case 0xB169:
case 0xB369:
case 0xB569:
case 0xB769:
case 0xB969:
case 0xBB69:
case 0xBD69:
case 0xBF69:
case 0xB16A:
case 0xB36A:
case 0xB56A:
case 0xB76A:
case 0xB96A:
case 0xBB6A:
case 0xBD6A:
case 0xBF6A:
case 0xB16B:
case 0xB36B:
case 0xB56B:
case 0xB76B:
case 0xB96B:
case 0xBB6B:
case 0xBD6B:
case 0xBF6B:
case 0xB16C:
case 0xB36C:
case 0xB56C:
case 0xB76C:
case 0xB96C:
case 0xBB6C:
case 0xBD6C:
case 0xBF6C:
case 0xB16D:
case 0xB36D:
case 0xB56D:
case 0xB76D:
case 0xB96D:
case 0xBB6D:
case 0xBD6D:
case 0xBF6D:
case 0xB16E:
case 0xB36E:
case 0xB56E:
case 0xB76E:
case 0xB96E:
case 0xBB6E:
case 0xBD6E:
case 0xBF6E:
case 0xB16F:
case 0xB36F:
case 0xB56F:
case 0xB76F:
case 0xB96F:
case 0xBB6F:
case 0xBD6F:
case 0xBF6F:

// EORDa
case 0xB168:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0xB370:
case 0xB570:
case 0xB770:
case 0xB970:
case 0xBB70:
case 0xBD70:
case 0xBF70:
case 0xB171:
case 0xB371:
case 0xB571:
case 0xB771:
case 0xB971:
case 0xBB71:
case 0xBD71:
case 0xBF71:
case 0xB172:
case 0xB372:
case 0xB572:
case 0xB772:
case 0xB972:
case 0xBB72:
case 0xBD72:
case 0xBF72:
case 0xB173:
case 0xB373:
case 0xB573:
case 0xB773:
case 0xB973:
case 0xBB73:
case 0xBD73:
case 0xBF73:
case 0xB174:
case 0xB374:
case 0xB574:
case 0xB774:
case 0xB974:
case 0xBB74:
case 0xBD74:
case 0xBF74:
case 0xB175:
case 0xB375:
case 0xB575:
case 0xB775:
case 0xB975:
case 0xBB75:
case 0xBD75:
case 0xBF75:
case 0xB176:
case 0xB376:
case 0xB576:
case 0xB776:
case 0xB976:
case 0xBB76:
case 0xBD76:
case 0xBF76:
case 0xB177:
case 0xB377:
case 0xB577:
case 0xB777:
case 0xB977:
case 0xBB77:
case 0xBD77:
case 0xBF77:

// EORDa
case 0xB170:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0xB378:
case 0xB578:
case 0xB778:
case 0xB978:
case 0xBB78:
case 0xBD78:
case 0xBF78:

// EORDa
case 0xB178:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0xB379:
case 0xB579:
case 0xB779:
case 0xB979:
case 0xBB79:
case 0xBD79:
case 0xBF79:

// EORDa
case 0xB179:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0xB35F:
case 0xB55F:
case 0xB75F:
case 0xB95F:
case 0xBB5F:
case 0xBD5F:
case 0xBF5F:

// EORDa
case 0xB15F:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xB367:
case 0xB567:
case 0xB767:
case 0xB967:
case 0xBB67:
case 0xBD67:
case 0xBF67:

// EORDa
case 0xB167:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xB380:
case 0xB580:
case 0xB780:
case 0xB980:
case 0xBB80:
case 0xBD80:
case 0xBF80:
case 0xB181:
case 0xB381:
case 0xB581:
case 0xB781:
case 0xB981:
case 0xBB81:
case 0xBD81:
case 0xBF81:
case 0xB182:
case 0xB382:
case 0xB582:
case 0xB782:
case 0xB982:
case 0xBB82:
case 0xBD82:
case 0xBF82:
case 0xB183:
case 0xB383:
case 0xB583:
case 0xB783:
case 0xB983:
case 0xBB83:
case 0xBD83:
case 0xBF83:
case 0xB184:
case 0xB384:
case 0xB584:
case 0xB784:
case 0xB984:
case 0xBB84:
case 0xBD84:
case 0xBF84:
case 0xB185:
case 0xB385:
case 0xB585:
case 0xB785:
case 0xB985:
case 0xBB85:
case 0xBD85:
case 0xBF85:
case 0xB186:
case 0xB386:
case 0xB586:
case 0xB786:
case 0xB986:
case 0xBB86:
case 0xBD86:
case 0xBF86:
case 0xB187:
case 0xB387:
case 0xB587:
case 0xB787:
case 0xB987:
case 0xBB87:
case 0xBD87:
case 0xBF87:

// EORDa
case 0xB180:
{
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0xB390:
case 0xB590:
case 0xB790:
case 0xB990:
case 0xBB90:
case 0xBD90:
case 0xBF90:
case 0xB191:
case 0xB391:
case 0xB591:
case 0xB791:
case 0xB991:
case 0xBB91:
case 0xBD91:
case 0xBF91:
case 0xB192:
case 0xB392:
case 0xB592:
case 0xB792:
case 0xB992:
case 0xBB92:
case 0xBD92:
case 0xBF92:
case 0xB193:
case 0xB393:
case 0xB593:
case 0xB793:
case 0xB993:
case 0xBB93:
case 0xBD93:
case 0xBF93:
case 0xB194:
case 0xB394:
case 0xB594:
case 0xB794:
case 0xB994:
case 0xBB94:
case 0xBD94:
case 0xBF94:
case 0xB195:
case 0xB395:
case 0xB595:
case 0xB795:
case 0xB995:
case 0xBB95:
case 0xBD95:
case 0xBF95:
case 0xB196:
case 0xB396:
case 0xB596:
case 0xB796:
case 0xB996:
case 0xBB96:
case 0xBD96:
case 0xBF96:
case 0xB197:
case 0xB397:
case 0xB597:
case 0xB797:
case 0xB997:
case 0xBB97:
case 0xBD97:
case 0xBF97:

// EORDa
case 0xB190:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_LONG_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0xB398:
case 0xB598:
case 0xB798:
case 0xB998:
case 0xBB98:
case 0xBD98:
case 0xBF98:
case 0xB199:
case 0xB399:
case 0xB599:
case 0xB799:
case 0xB999:
case 0xBB99:
case 0xBD99:
case 0xBF99:
case 0xB19A:
case 0xB39A:
case 0xB59A:
case 0xB79A:
case 0xB99A:
case 0xBB9A:
case 0xBD9A:
case 0xBF9A:
case 0xB19B:
case 0xB39B:
case 0xB59B:
case 0xB79B:
case 0xB99B:
case 0xBB9B:
case 0xBD9B:
case 0xBF9B:
case 0xB19C:
case 0xB39C:
case 0xB59C:
case 0xB79C:
case 0xB99C:
case 0xBB9C:
case 0xBD9C:
case 0xBF9C:
case 0xB19D:
case 0xB39D:
case 0xB59D:
case 0xB79D:
case 0xB99D:
case 0xBB9D:
case 0xBD9D:
case 0xBF9D:
case 0xB19E:
case 0xB39E:
case 0xB59E:
case 0xB79E:
case 0xB99E:
case 0xBB9E:
case 0xBD9E:
case 0xBF9E:

// EORDa
case 0xB198:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READ_LONG_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0xB3A0:
case 0xB5A0:
case 0xB7A0:
case 0xB9A0:
case 0xBBA0:
case 0xBDA0:
case 0xBFA0:
case 0xB1A1:
case 0xB3A1:
case 0xB5A1:
case 0xB7A1:
case 0xB9A1:
case 0xBBA1:
case 0xBDA1:
case 0xBFA1:
case 0xB1A2:
case 0xB3A2:
case 0xB5A2:
case 0xB7A2:
case 0xB9A2:
case 0xBBA2:
case 0xBDA2:
case 0xBFA2:
case 0xB1A3:
case 0xB3A3:
case 0xB5A3:
case 0xB7A3:
case 0xB9A3:
case 0xBBA3:
case 0xBDA3:
case 0xBFA3:
case 0xB1A4:
case 0xB3A4:
case 0xB5A4:
case 0xB7A4:
case 0xB9A4:
case 0xBBA4:
case 0xBDA4:
case 0xBFA4:
case 0xB1A5:
case 0xB3A5:
case 0xB5A5:
case 0xB7A5:
case 0xB9A5:
case 0xBBA5:
case 0xBDA5:
case 0xBFA5:
case 0xB1A6:
case 0xB3A6:
case 0xB5A6:
case 0xB7A6:
case 0xB9A6:
case 0xBBA6:
case 0xBDA6:
case 0xBFA6:

// EORDa
case 0xB1A0:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_LONG_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0xB3A8:
case 0xB5A8:
case 0xB7A8:
case 0xB9A8:
case 0xBBA8:
case 0xBDA8:
case 0xBFA8:
case 0xB1A9:
case 0xB3A9:
case 0xB5A9:
case 0xB7A9:
case 0xB9A9:
case 0xBBA9:
case 0xBDA9:
case 0xBFA9:
case 0xB1AA:
case 0xB3AA:
case 0xB5AA:
case 0xB7AA:
case 0xB9AA:
case 0xBBAA:
case 0xBDAA:
case 0xBFAA:
case 0xB1AB:
case 0xB3AB:
case 0xB5AB:
case 0xB7AB:
case 0xB9AB:
case 0xBBAB:
case 0xBDAB:
case 0xBFAB:
case 0xB1AC:
case 0xB3AC:
case 0xB5AC:
case 0xB7AC:
case 0xB9AC:
case 0xBBAC:
case 0xBDAC:
case 0xBFAC:
case 0xB1AD:
case 0xB3AD:
case 0xB5AD:
case 0xB7AD:
case 0xB9AD:
case 0xBBAD:
case 0xBDAD:
case 0xBFAD:
case 0xB1AE:
case 0xB3AE:
case 0xB5AE:
case 0xB7AE:
case 0xB9AE:
case 0xBBAE:
case 0xBDAE:
case 0xBFAE:
case 0xB1AF:
case 0xB3AF:
case 0xB5AF:
case 0xB7AF:
case 0xB9AF:
case 0xBBAF:
case 0xBDAF:
case 0xBFAF:

// EORDa
case 0xB1A8:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0xB3B0:
case 0xB5B0:
case 0xB7B0:
case 0xB9B0:
case 0xBBB0:
case 0xBDB0:
case 0xBFB0:
case 0xB1B1:
case 0xB3B1:
case 0xB5B1:
case 0xB7B1:
case 0xB9B1:
case 0xBBB1:
case 0xBDB1:
case 0xBFB1:
case 0xB1B2:
case 0xB3B2:
case 0xB5B2:
case 0xB7B2:
case 0xB9B2:
case 0xBBB2:
case 0xBDB2:
case 0xBFB2:
case 0xB1B3:
case 0xB3B3:
case 0xB5B3:
case 0xB7B3:
case 0xB9B3:
case 0xBBB3:
case 0xBDB3:
case 0xBFB3:
case 0xB1B4:
case 0xB3B4:
case 0xB5B4:
case 0xB7B4:
case 0xB9B4:
case 0xBBB4:
case 0xBDB4:
case 0xBFB4:
case 0xB1B5:
case 0xB3B5:
case 0xB5B5:
case 0xB7B5:
case 0xB9B5:
case 0xBBB5:
case 0xBDB5:
case 0xBFB5:
case 0xB1B6:
case 0xB3B6:
case 0xB5B6:
case 0xB7B6:
case 0xB9B6:
case 0xBBB6:
case 0xBDB6:
case 0xBFB6:
case 0xB1B7:
case 0xB3B7:
case 0xB5B7:
case 0xB7B7:
case 0xB9B7:
case 0xBBB7:
case 0xBDB7:
case 0xBFB7:

// EORDa
case 0xB1B0:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0xB3B8:
case 0xB5B8:
case 0xB7B8:
case 0xB9B8:
case 0xBBB8:
case 0xBDB8:
case 0xBFB8:

// EORDa
case 0xB1B8:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0xB3B9:
case 0xB5B9:
case 0xB7B9:
case 0xB9B9:
case 0xBBB9:
case 0xBDB9:
case 0xBFB9:

// EORDa
case 0xB1B9:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_LONG_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0xB39F:
case 0xB59F:
case 0xB79F:
case 0xB99F:
case 0xBB9F:
case 0xBD9F:
case 0xBF9F:

// EORDa
case 0xB19F:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READ_LONG_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0xB3A7:
case 0xB5A7:
case 0xB7A7:
case 0xB9A7:
case 0xBBA7:
case 0xBDA7:
case 0xBFA7:

// EORDa
case 0xB1A7:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READ_LONG_F(adr, res)
	res ^= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0xB2C0:
case 0xB4C0:
case 0xB6C0:
case 0xB8C0:
case 0xBAC0:
case 0xBCC0:
case 0xBEC0:
case 0xB0C1:
case 0xB2C1:
case 0xB4C1:
case 0xB6C1:
case 0xB8C1:
case 0xBAC1:
case 0xBCC1:
case 0xBEC1:
case 0xB0C2:
case 0xB2C2:
case 0xB4C2:
case 0xB6C2:
case 0xB8C2:
case 0xBAC2:
case 0xBCC2:
case 0xBEC2:
case 0xB0C3:
case 0xB2C3:
case 0xB4C3:
case 0xB6C3:
case 0xB8C3:
case 0xBAC3:
case 0xBCC3:
case 0xBEC3:
case 0xB0C4:
case 0xB2C4:
case 0xB4C4:
case 0xB6C4:
case 0xB8C4:
case 0xBAC4:
case 0xBCC4:
case 0xBEC4:
case 0xB0C5:
case 0xB2C5:
case 0xB4C5:
case 0xB6C5:
case 0xB8C5:
case 0xBAC5:
case 0xBCC5:
case 0xBEC5:
case 0xB0C6:
case 0xB2C6:
case 0xB4C6:
case 0xB6C6:
case 0xB8C6:
case 0xBAC6:
case 0xBCC6:
case 0xBEC6:
case 0xB0C7:
case 0xB2C7:
case 0xB4C7:
case 0xB6C7:
case 0xB8C7:
case 0xBAC7:
case 0xBCC7:
case 0xBEC7:

// CMPA
case 0xB0C0:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s16)CPU->D[(Opcode >> 0) & 7];
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(6)
case 0xB2C8:
case 0xB4C8:
case 0xB6C8:
case 0xB8C8:
case 0xBAC8:
case 0xBCC8:
case 0xBEC8:
case 0xB0C9:
case 0xB2C9:
case 0xB4C9:
case 0xB6C9:
case 0xB8C9:
case 0xBAC9:
case 0xBCC9:
case 0xBEC9:
case 0xB0CA:
case 0xB2CA:
case 0xB4CA:
case 0xB6CA:
case 0xB8CA:
case 0xBACA:
case 0xBCCA:
case 0xBECA:
case 0xB0CB:
case 0xB2CB:
case 0xB4CB:
case 0xB6CB:
case 0xB8CB:
case 0xBACB:
case 0xBCCB:
case 0xBECB:
case 0xB0CC:
case 0xB2CC:
case 0xB4CC:
case 0xB6CC:
case 0xB8CC:
case 0xBACC:
case 0xBCCC:
case 0xBECC:
case 0xB0CD:
case 0xB2CD:
case 0xB4CD:
case 0xB6CD:
case 0xB8CD:
case 0xBACD:
case 0xBCCD:
case 0xBECD:
case 0xB0CE:
case 0xB2CE:
case 0xB4CE:
case 0xB6CE:
case 0xB8CE:
case 0xBACE:
case 0xBCCE:
case 0xBECE:
case 0xB0CF:
case 0xB2CF:
case 0xB4CF:
case 0xB6CF:
case 0xB8CF:
case 0xBACF:
case 0xBCCF:
case 0xBECF:

// CMPA
case 0xB0C8:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s16)CPU->A[(Opcode >> 0) & 7];
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(6)
case 0xB2D0:
case 0xB4D0:
case 0xB6D0:
case 0xB8D0:
case 0xBAD0:
case 0xBCD0:
case 0xBED0:
case 0xB0D1:
case 0xB2D1:
case 0xB4D1:
case 0xB6D1:
case 0xB8D1:
case 0xBAD1:
case 0xBCD1:
case 0xBED1:
case 0xB0D2:
case 0xB2D2:
case 0xB4D2:
case 0xB6D2:
case 0xB8D2:
case 0xBAD2:
case 0xBCD2:
case 0xBED2:
case 0xB0D3:
case 0xB2D3:
case 0xB4D3:
case 0xB6D3:
case 0xB8D3:
case 0xBAD3:
case 0xBCD3:
case 0xBED3:
case 0xB0D4:
case 0xB2D4:
case 0xB4D4:
case 0xB6D4:
case 0xB8D4:
case 0xBAD4:
case 0xBCD4:
case 0xBED4:
case 0xB0D5:
case 0xB2D5:
case 0xB4D5:
case 0xB6D5:
case 0xB8D5:
case 0xBAD5:
case 0xBCD5:
case 0xBED5:
case 0xB0D6:
case 0xB2D6:
case 0xB4D6:
case 0xB6D6:
case 0xB8D6:
case 0xBAD6:
case 0xBCD6:
case 0xBED6:
case 0xB0D7:
case 0xB2D7:
case 0xB4D7:
case 0xB6D7:
case 0xB8D7:
case 0xBAD7:
case 0xBCD7:
case 0xBED7:

// CMPA
case 0xB0D0:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(10)
case 0xB2D8:
case 0xB4D8:
case 0xB6D8:
case 0xB8D8:
case 0xBAD8:
case 0xBCD8:
case 0xBED8:
case 0xB0D9:
case 0xB2D9:
case 0xB4D9:
case 0xB6D9:
case 0xB8D9:
case 0xBAD9:
case 0xBCD9:
case 0xBED9:
case 0xB0DA:
case 0xB2DA:
case 0xB4DA:
case 0xB6DA:
case 0xB8DA:
case 0xBADA:
case 0xBCDA:
case 0xBEDA:
case 0xB0DB:
case 0xB2DB:
case 0xB4DB:
case 0xB6DB:
case 0xB8DB:
case 0xBADB:
case 0xBCDB:
case 0xBEDB:
case 0xB0DC:
case 0xB2DC:
case 0xB4DC:
case 0xB6DC:
case 0xB8DC:
case 0xBADC:
case 0xBCDC:
case 0xBEDC:
case 0xB0DD:
case 0xB2DD:
case 0xB4DD:
case 0xB6DD:
case 0xB8DD:
case 0xBADD:
case 0xBCDD:
case 0xBEDD:
case 0xB0DE:
case 0xB2DE:
case 0xB4DE:
case 0xB6DE:
case 0xB8DE:
case 0xBADE:
case 0xBCDE:
case 0xBEDE:

// CMPA
case 0xB0D8:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(10)
case 0xB2E0:
case 0xB4E0:
case 0xB6E0:
case 0xB8E0:
case 0xBAE0:
case 0xBCE0:
case 0xBEE0:
case 0xB0E1:
case 0xB2E1:
case 0xB4E1:
case 0xB6E1:
case 0xB8E1:
case 0xBAE1:
case 0xBCE1:
case 0xBEE1:
case 0xB0E2:
case 0xB2E2:
case 0xB4E2:
case 0xB6E2:
case 0xB8E2:
case 0xBAE2:
case 0xBCE2:
case 0xBEE2:
case 0xB0E3:
case 0xB2E3:
case 0xB4E3:
case 0xB6E3:
case 0xB8E3:
case 0xBAE3:
case 0xBCE3:
case 0xBEE3:
case 0xB0E4:
case 0xB2E4:
case 0xB4E4:
case 0xB6E4:
case 0xB8E4:
case 0xBAE4:
case 0xBCE4:
case 0xBEE4:
case 0xB0E5:
case 0xB2E5:
case 0xB4E5:
case 0xB6E5:
case 0xB8E5:
case 0xBAE5:
case 0xBCE5:
case 0xBEE5:
case 0xB0E6:
case 0xB2E6:
case 0xB4E6:
case 0xB6E6:
case 0xB8E6:
case 0xBAE6:
case 0xBCE6:
case 0xBEE6:

// CMPA
case 0xB0E0:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(12)
case 0xB2E8:
case 0xB4E8:
case 0xB6E8:
case 0xB8E8:
case 0xBAE8:
case 0xBCE8:
case 0xBEE8:
case 0xB0E9:
case 0xB2E9:
case 0xB4E9:
case 0xB6E9:
case 0xB8E9:
case 0xBAE9:
case 0xBCE9:
case 0xBEE9:
case 0xB0EA:
case 0xB2EA:
case 0xB4EA:
case 0xB6EA:
case 0xB8EA:
case 0xBAEA:
case 0xBCEA:
case 0xBEEA:
case 0xB0EB:
case 0xB2EB:
case 0xB4EB:
case 0xB6EB:
case 0xB8EB:
case 0xBAEB:
case 0xBCEB:
case 0xBEEB:
case 0xB0EC:
case 0xB2EC:
case 0xB4EC:
case 0xB6EC:
case 0xB8EC:
case 0xBAEC:
case 0xBCEC:
case 0xBEEC:
case 0xB0ED:
case 0xB2ED:
case 0xB4ED:
case 0xB6ED:
case 0xB8ED:
case 0xBAED:
case 0xBCED:
case 0xBEED:
case 0xB0EE:
case 0xB2EE:
case 0xB4EE:
case 0xB6EE:
case 0xB8EE:
case 0xBAEE:
case 0xBCEE:
case 0xBEEE:
case 0xB0EF:
case 0xB2EF:
case 0xB4EF:
case 0xB6EF:
case 0xB8EF:
case 0xBAEF:
case 0xBCEF:
case 0xBEEF:

// CMPA
case 0xB0E8:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(14)
case 0xB2F0:
case 0xB4F0:
case 0xB6F0:
case 0xB8F0:
case 0xBAF0:
case 0xBCF0:
case 0xBEF0:
case 0xB0F1:
case 0xB2F1:
case 0xB4F1:
case 0xB6F1:
case 0xB8F1:
case 0xBAF1:
case 0xBCF1:
case 0xBEF1:
case 0xB0F2:
case 0xB2F2:
case 0xB4F2:
case 0xB6F2:
case 0xB8F2:
case 0xBAF2:
case 0xBCF2:
case 0xBEF2:
case 0xB0F3:
case 0xB2F3:
case 0xB4F3:
case 0xB6F3:
case 0xB8F3:
case 0xBAF3:
case 0xBCF3:
case 0xBEF3:
case 0xB0F4:
case 0xB2F4:
case 0xB4F4:
case 0xB6F4:
case 0xB8F4:
case 0xBAF4:
case 0xBCF4:
case 0xBEF4:
case 0xB0F5:
case 0xB2F5:
case 0xB4F5:
case 0xB6F5:
case 0xB8F5:
case 0xBAF5:
case 0xBCF5:
case 0xBEF5:
case 0xB0F6:
case 0xB2F6:
case 0xB4F6:
case 0xB6F6:
case 0xB8F6:
case 0xBAF6:
case 0xBCF6:
case 0xBEF6:
case 0xB0F7:
case 0xB2F7:
case 0xB4F7:
case 0xB6F7:
case 0xB8F7:
case 0xBAF7:
case 0xBCF7:
case 0xBEF7:

// CMPA
case 0xB0F0:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(16)
case 0xB2F8:
case 0xB4F8:
case 0xB6F8:
case 0xB8F8:
case 0xBAF8:
case 0xBCF8:
case 0xBEF8:

// CMPA
case 0xB0F8:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(14)
case 0xB2F9:
case 0xB4F9:
case 0xB6F9:
case 0xB8F9:
case 0xBAF9:
case 0xBCF9:
case 0xBEF9:

// CMPA
case 0xB0F9:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(18)
case 0xB2FA:
case 0xB4FA:
case 0xB6FA:
case 0xB8FA:
case 0xBAFA:
case 0xBCFA:
case 0xBEFA:

// CMPA
case 0xB0FA:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(14)
case 0xB2FB:
case 0xB4FB:
case 0xB6FB:
case 0xB8FB:
case 0xBAFB:
case 0xBCFB:
case 0xBEFB:

// CMPA
case 0xB0FB:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(16)
case 0xB2FC:
case 0xB4FC:
case 0xB6FC:
case 0xB8FC:
case 0xBAFC:
case 0xBCFC:
case 0xBEFC:

// CMPA
case 0xB0FC:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s16)FETCH_WORD;
	PC += 2;
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(10)
case 0xB2DF:
case 0xB4DF:
case 0xB6DF:
case 0xB8DF:
case 0xBADF:
case 0xBCDF:
case 0xBEDF:

// CMPA
case 0xB0DF:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(10)
case 0xB2E7:
case 0xB4E7:
case 0xB6E7:
case 0xB8E7:
case 0xBAE7:
case 0xBCE7:
case 0xBEE7:

// CMPA
case 0xB0E7:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(12)
case 0xB3C0:
case 0xB5C0:
case 0xB7C0:
case 0xB9C0:
case 0xBBC0:
case 0xBDC0:
case 0xBFC0:
case 0xB1C1:
case 0xB3C1:
case 0xB5C1:
case 0xB7C1:
case 0xB9C1:
case 0xBBC1:
case 0xBDC1:
case 0xBFC1:
case 0xB1C2:
case 0xB3C2:
case 0xB5C2:
case 0xB7C2:
case 0xB9C2:
case 0xBBC2:
case 0xBDC2:
case 0xBFC2:
case 0xB1C3:
case 0xB3C3:
case 0xB5C3:
case 0xB7C3:
case 0xB9C3:
case 0xBBC3:
case 0xBDC3:
case 0xBFC3:
case 0xB1C4:
case 0xB3C4:
case 0xB5C4:
case 0xB7C4:
case 0xB9C4:
case 0xBBC4:
case 0xBDC4:
case 0xBFC4:
case 0xB1C5:
case 0xB3C5:
case 0xB5C5:
case 0xB7C5:
case 0xB9C5:
case 0xBBC5:
case 0xBDC5:
case 0xBFC5:
case 0xB1C6:
case 0xB3C6:
case 0xB5C6:
case 0xB7C6:
case 0xB9C6:
case 0xBBC6:
case 0xBDC6:
case 0xBFC6:
case 0xB1C7:
case 0xB3C7:
case 0xB5C7:
case 0xB7C7:
case 0xB9C7:
case 0xBBC7:
case 0xBDC7:
case 0xBFC7:

// CMPA
case 0xB1C0:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s32)CPU->D[(Opcode >> 0) & 7];
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(6)
case 0xB3C8:
case 0xB5C8:
case 0xB7C8:
case 0xB9C8:
case 0xBBC8:
case 0xBDC8:
case 0xBFC8:
case 0xB1C9:
case 0xB3C9:
case 0xB5C9:
case 0xB7C9:
case 0xB9C9:
case 0xBBC9:
case 0xBDC9:
case 0xBFC9:
case 0xB1CA:
case 0xB3CA:
case 0xB5CA:
case 0xB7CA:
case 0xB9CA:
case 0xBBCA:
case 0xBDCA:
case 0xBFCA:
case 0xB1CB:
case 0xB3CB:
case 0xB5CB:
case 0xB7CB:
case 0xB9CB:
case 0xBBCB:
case 0xBDCB:
case 0xBFCB:
case 0xB1CC:
case 0xB3CC:
case 0xB5CC:
case 0xB7CC:
case 0xB9CC:
case 0xBBCC:
case 0xBDCC:
case 0xBFCC:
case 0xB1CD:
case 0xB3CD:
case 0xB5CD:
case 0xB7CD:
case 0xB9CD:
case 0xBBCD:
case 0xBDCD:
case 0xBFCD:
case 0xB1CE:
case 0xB3CE:
case 0xB5CE:
case 0xB7CE:
case 0xB9CE:
case 0xBBCE:
case 0xBDCE:
case 0xBFCE:
case 0xB1CF:
case 0xB3CF:
case 0xB5CF:
case 0xB7CF:
case 0xB9CF:
case 0xBBCF:
case 0xBDCF:
case 0xBFCF:

// CMPA
case 0xB1C8:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s32)CPU->A[(Opcode >> 0) & 7];
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(6)
case 0xB3D0:
case 0xB5D0:
case 0xB7D0:
case 0xB9D0:
case 0xBBD0:
case 0xBDD0:
case 0xBFD0:
case 0xB1D1:
case 0xB3D1:
case 0xB5D1:
case 0xB7D1:
case 0xB9D1:
case 0xBBD1:
case 0xBDD1:
case 0xBFD1:
case 0xB1D2:
case 0xB3D2:
case 0xB5D2:
case 0xB7D2:
case 0xB9D2:
case 0xBBD2:
case 0xBDD2:
case 0xBFD2:
case 0xB1D3:
case 0xB3D3:
case 0xB5D3:
case 0xB7D3:
case 0xB9D3:
case 0xBBD3:
case 0xBDD3:
case 0xBFD3:
case 0xB1D4:
case 0xB3D4:
case 0xB5D4:
case 0xB7D4:
case 0xB9D4:
case 0xBBD4:
case 0xBDD4:
case 0xBFD4:
case 0xB1D5:
case 0xB3D5:
case 0xB5D5:
case 0xB7D5:
case 0xB9D5:
case 0xBBD5:
case 0xBDD5:
case 0xBFD5:
case 0xB1D6:
case 0xB3D6:
case 0xB5D6:
case 0xB7D6:
case 0xB9D6:
case 0xBBD6:
case 0xBDD6:
case 0xBFD6:
case 0xB1D7:
case 0xB3D7:
case 0xB5D7:
case 0xB7D7:
case 0xB9D7:
case 0xBBD7:
case 0xBDD7:
case 0xBFD7:

// CMPA
case 0xB1D0:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(14)
case 0xB3D8:
case 0xB5D8:
case 0xB7D8:
case 0xB9D8:
case 0xBBD8:
case 0xBDD8:
case 0xBFD8:
case 0xB1D9:
case 0xB3D9:
case 0xB5D9:
case 0xB7D9:
case 0xB9D9:
case 0xBBD9:
case 0xBDD9:
case 0xBFD9:
case 0xB1DA:
case 0xB3DA:
case 0xB5DA:
case 0xB7DA:
case 0xB9DA:
case 0xBBDA:
case 0xBDDA:
case 0xBFDA:
case 0xB1DB:
case 0xB3DB:
case 0xB5DB:
case 0xB7DB:
case 0xB9DB:
case 0xBBDB:
case 0xBDDB:
case 0xBFDB:
case 0xB1DC:
case 0xB3DC:
case 0xB5DC:
case 0xB7DC:
case 0xB9DC:
case 0xBBDC:
case 0xBDDC:
case 0xBFDC:
case 0xB1DD:
case 0xB3DD:
case 0xB5DD:
case 0xB7DD:
case 0xB9DD:
case 0xBBDD:
case 0xBDDD:
case 0xBFDD:
case 0xB1DE:
case 0xB3DE:
case 0xB5DE:
case 0xB7DE:
case 0xB9DE:
case 0xBBDE:
case 0xBDDE:
case 0xBFDE:

// CMPA
case 0xB1D8:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(14)
case 0xB3E0:
case 0xB5E0:
case 0xB7E0:
case 0xB9E0:
case 0xBBE0:
case 0xBDE0:
case 0xBFE0:
case 0xB1E1:
case 0xB3E1:
case 0xB5E1:
case 0xB7E1:
case 0xB9E1:
case 0xBBE1:
case 0xBDE1:
case 0xBFE1:
case 0xB1E2:
case 0xB3E2:
case 0xB5E2:
case 0xB7E2:
case 0xB9E2:
case 0xBBE2:
case 0xBDE2:
case 0xBFE2:
case 0xB1E3:
case 0xB3E3:
case 0xB5E3:
case 0xB7E3:
case 0xB9E3:
case 0xBBE3:
case 0xBDE3:
case 0xBFE3:
case 0xB1E4:
case 0xB3E4:
case 0xB5E4:
case 0xB7E4:
case 0xB9E4:
case 0xBBE4:
case 0xBDE4:
case 0xBFE4:
case 0xB1E5:
case 0xB3E5:
case 0xB5E5:
case 0xB7E5:
case 0xB9E5:
case 0xBBE5:
case 0xBDE5:
case 0xBFE5:
case 0xB1E6:
case 0xB3E6:
case 0xB5E6:
case 0xB7E6:
case 0xB9E6:
case 0xBBE6:
case 0xBDE6:
case 0xBFE6:

// CMPA
case 0xB1E0:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(16)
case 0xB3E8:
case 0xB5E8:
case 0xB7E8:
case 0xB9E8:
case 0xBBE8:
case 0xBDE8:
case 0xBFE8:
case 0xB1E9:
case 0xB3E9:
case 0xB5E9:
case 0xB7E9:
case 0xB9E9:
case 0xBBE9:
case 0xBDE9:
case 0xBFE9:
case 0xB1EA:
case 0xB3EA:
case 0xB5EA:
case 0xB7EA:
case 0xB9EA:
case 0xBBEA:
case 0xBDEA:
case 0xBFEA:
case 0xB1EB:
case 0xB3EB:
case 0xB5EB:
case 0xB7EB:
case 0xB9EB:
case 0xBBEB:
case 0xBDEB:
case 0xBFEB:
case 0xB1EC:
case 0xB3EC:
case 0xB5EC:
case 0xB7EC:
case 0xB9EC:
case 0xBBEC:
case 0xBDEC:
case 0xBFEC:
case 0xB1ED:
case 0xB3ED:
case 0xB5ED:
case 0xB7ED:
case 0xB9ED:
case 0xBBED:
case 0xBDED:
case 0xBFED:
case 0xB1EE:
case 0xB3EE:
case 0xB5EE:
case 0xB7EE:
case 0xB9EE:
case 0xBBEE:
case 0xBDEE:
case 0xBFEE:
case 0xB1EF:
case 0xB3EF:
case 0xB5EF:
case 0xB7EF:
case 0xB9EF:
case 0xBBEF:
case 0xBDEF:
case 0xBFEF:

// CMPA
case 0xB1E8:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(18)
case 0xB3F0:
case 0xB5F0:
case 0xB7F0:
case 0xB9F0:
case 0xBBF0:
case 0xBDF0:
case 0xBFF0:
case 0xB1F1:
case 0xB3F1:
case 0xB5F1:
case 0xB7F1:
case 0xB9F1:
case 0xBBF1:
case 0xBDF1:
case 0xBFF1:
case 0xB1F2:
case 0xB3F2:
case 0xB5F2:
case 0xB7F2:
case 0xB9F2:
case 0xBBF2:
case 0xBDF2:
case 0xBFF2:
case 0xB1F3:
case 0xB3F3:
case 0xB5F3:
case 0xB7F3:
case 0xB9F3:
case 0xBBF3:
case 0xBDF3:
case 0xBFF3:
case 0xB1F4:
case 0xB3F4:
case 0xB5F4:
case 0xB7F4:
case 0xB9F4:
case 0xBBF4:
case 0xBDF4:
case 0xBFF4:
case 0xB1F5:
case 0xB3F5:
case 0xB5F5:
case 0xB7F5:
case 0xB9F5:
case 0xBBF5:
case 0xBDF5:
case 0xBFF5:
case 0xB1F6:
case 0xB3F6:
case 0xB5F6:
case 0xB7F6:
case 0xB9F6:
case 0xBBF6:
case 0xBDF6:
case 0xBFF6:
case 0xB1F7:
case 0xB3F7:
case 0xB5F7:
case 0xB7F7:
case 0xB9F7:
case 0xBBF7:
case 0xBDF7:
case 0xBFF7:

// CMPA
case 0xB1F0:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(20)
case 0xB3F8:
case 0xB5F8:
case 0xB7F8:
case 0xB9F8:
case 0xBBF8:
case 0xBDF8:
case 0xBFF8:

// CMPA
case 0xB1F8:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(18)
case 0xB3F9:
case 0xB5F9:
case 0xB7F9:
case 0xB9F9:
case 0xBBF9:
case 0xBDF9:
case 0xBFF9:

// CMPA
case 0xB1F9:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(22)
case 0xB3FA:
case 0xB5FA:
case 0xB7FA:
case 0xB9FA:
case 0xBBFA:
case 0xBDFA:
case 0xBFFA:

// CMPA
case 0xB1FA:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(18)
case 0xB3FB:
case 0xB5FB:
case 0xB7FB:
case 0xB9FB:
case 0xBBFB:
case 0xBDFB:
case 0xBFFB:

// CMPA
case 0xB1FB:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(20)
case 0xB3FC:
case 0xB5FC:
case 0xB7FC:
case 0xB9FC:
case 0xBBFC:
case 0xBDFC:
case 0xBFFC:

// CMPA
case 0xB1FC:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s32)FETCH_LONG;
	PC += 4;
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst - src;
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
}
RET(14)
case 0xB3DF:
case 0xB5DF:
case 0xB7DF:
case 0xB9DF:
case 0xBBDF:
case 0xBDDF:
case 0xBFDF:

// CMPA
case 0xB1DF:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(14)
case 0xB3E7:
case 0xB5E7:
case 0xB7E7:
case 0xB9E7:
case 0xBBE7:
case 0xBDE7:
case 0xBFE7:

// CMPA
case 0xB1E7:
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
	CPU->flag_notZ = res;
	CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;
	CPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;
	CPU->flag_N = res >> 24;
	POST_IO
}
RET(16)
