case 0xC200:
case 0xC400:
case 0xC600:
case 0xC800:
case 0xCA00:
case 0xCC00:
case 0xCE00:
case 0xC001:
case 0xC201:
case 0xC401:
case 0xC601:
case 0xC801:
case 0xCA01:
case 0xCC01:
case 0xCE01:
case 0xC002:
case 0xC202:
case 0xC402:
case 0xC602:
case 0xC802:
case 0xCA02:
case 0xCC02:
case 0xCE02:
case 0xC003:
case 0xC203:
case 0xC403:
case 0xC603:
case 0xC803:
case 0xCA03:
case 0xCC03:
case 0xCE03:
case 0xC004:
case 0xC204:
case 0xC404:
case 0xC604:
case 0xC804:
case 0xCA04:
case 0xCC04:
case 0xCE04:
case 0xC005:
case 0xC205:
case 0xC405:
case 0xC605:
case 0xC805:
case 0xCA05:
case 0xCC05:
case 0xCE05:
case 0xC006:
case 0xC206:
case 0xC406:
case 0xC606:
case 0xC806:
case 0xCA06:
case 0xCC06:
case 0xCE06:
case 0xC007:
case 0xC207:
case 0xC407:
case 0xC607:
case 0xC807:
case 0xCA07:
case 0xCC07:
case 0xCE07:

// ANDaD
case 0xC000:
{
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0xC210:
case 0xC410:
case 0xC610:
case 0xC810:
case 0xCA10:
case 0xCC10:
case 0xCE10:
case 0xC011:
case 0xC211:
case 0xC411:
case 0xC611:
case 0xC811:
case 0xCA11:
case 0xCC11:
case 0xCE11:
case 0xC012:
case 0xC212:
case 0xC412:
case 0xC612:
case 0xC812:
case 0xCA12:
case 0xCC12:
case 0xCE12:
case 0xC013:
case 0xC213:
case 0xC413:
case 0xC613:
case 0xC813:
case 0xCA13:
case 0xCC13:
case 0xCE13:
case 0xC014:
case 0xC214:
case 0xC414:
case 0xC614:
case 0xC814:
case 0xCA14:
case 0xCC14:
case 0xCE14:
case 0xC015:
case 0xC215:
case 0xC415:
case 0xC615:
case 0xC815:
case 0xCA15:
case 0xCC15:
case 0xCE15:
case 0xC016:
case 0xC216:
case 0xC416:
case 0xC616:
case 0xC816:
case 0xCA16:
case 0xCC16:
case 0xCE16:
case 0xC017:
case 0xC217:
case 0xC417:
case 0xC617:
case 0xC817:
case 0xCA17:
case 0xCC17:
case 0xCE17:

// ANDaD
case 0xC010:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0xC218:
case 0xC418:
case 0xC618:
case 0xC818:
case 0xCA18:
case 0xCC18:
case 0xCE18:
case 0xC019:
case 0xC219:
case 0xC419:
case 0xC619:
case 0xC819:
case 0xCA19:
case 0xCC19:
case 0xCE19:
case 0xC01A:
case 0xC21A:
case 0xC41A:
case 0xC61A:
case 0xC81A:
case 0xCA1A:
case 0xCC1A:
case 0xCE1A:
case 0xC01B:
case 0xC21B:
case 0xC41B:
case 0xC61B:
case 0xC81B:
case 0xCA1B:
case 0xCC1B:
case 0xCE1B:
case 0xC01C:
case 0xC21C:
case 0xC41C:
case 0xC61C:
case 0xC81C:
case 0xCA1C:
case 0xCC1C:
case 0xCE1C:
case 0xC01D:
case 0xC21D:
case 0xC41D:
case 0xC61D:
case 0xC81D:
case 0xCA1D:
case 0xCC1D:
case 0xCE1D:
case 0xC01E:
case 0xC21E:
case 0xC41E:
case 0xC61E:
case 0xC81E:
case 0xCA1E:
case 0xCC1E:
case 0xCE1E:

// ANDaD
case 0xC018:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0xC220:
case 0xC420:
case 0xC620:
case 0xC820:
case 0xCA20:
case 0xCC20:
case 0xCE20:
case 0xC021:
case 0xC221:
case 0xC421:
case 0xC621:
case 0xC821:
case 0xCA21:
case 0xCC21:
case 0xCE21:
case 0xC022:
case 0xC222:
case 0xC422:
case 0xC622:
case 0xC822:
case 0xCA22:
case 0xCC22:
case 0xCE22:
case 0xC023:
case 0xC223:
case 0xC423:
case 0xC623:
case 0xC823:
case 0xCA23:
case 0xCC23:
case 0xCE23:
case 0xC024:
case 0xC224:
case 0xC424:
case 0xC624:
case 0xC824:
case 0xCA24:
case 0xCC24:
case 0xCE24:
case 0xC025:
case 0xC225:
case 0xC425:
case 0xC625:
case 0xC825:
case 0xCA25:
case 0xCC25:
case 0xCE25:
case 0xC026:
case 0xC226:
case 0xC426:
case 0xC626:
case 0xC826:
case 0xCA26:
case 0xCC26:
case 0xCE26:

// ANDaD
case 0xC020:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0xC228:
case 0xC428:
case 0xC628:
case 0xC828:
case 0xCA28:
case 0xCC28:
case 0xCE28:
case 0xC029:
case 0xC229:
case 0xC429:
case 0xC629:
case 0xC829:
case 0xCA29:
case 0xCC29:
case 0xCE29:
case 0xC02A:
case 0xC22A:
case 0xC42A:
case 0xC62A:
case 0xC82A:
case 0xCA2A:
case 0xCC2A:
case 0xCE2A:
case 0xC02B:
case 0xC22B:
case 0xC42B:
case 0xC62B:
case 0xC82B:
case 0xCA2B:
case 0xCC2B:
case 0xCE2B:
case 0xC02C:
case 0xC22C:
case 0xC42C:
case 0xC62C:
case 0xC82C:
case 0xCA2C:
case 0xCC2C:
case 0xCE2C:
case 0xC02D:
case 0xC22D:
case 0xC42D:
case 0xC62D:
case 0xC82D:
case 0xCA2D:
case 0xCC2D:
case 0xCE2D:
case 0xC02E:
case 0xC22E:
case 0xC42E:
case 0xC62E:
case 0xC82E:
case 0xCA2E:
case 0xCC2E:
case 0xCE2E:
case 0xC02F:
case 0xC22F:
case 0xC42F:
case 0xC62F:
case 0xC82F:
case 0xCA2F:
case 0xCC2F:
case 0xCE2F:

// ANDaD
case 0xC028:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0xC230:
case 0xC430:
case 0xC630:
case 0xC830:
case 0xCA30:
case 0xCC30:
case 0xCE30:
case 0xC031:
case 0xC231:
case 0xC431:
case 0xC631:
case 0xC831:
case 0xCA31:
case 0xCC31:
case 0xCE31:
case 0xC032:
case 0xC232:
case 0xC432:
case 0xC632:
case 0xC832:
case 0xCA32:
case 0xCC32:
case 0xCE32:
case 0xC033:
case 0xC233:
case 0xC433:
case 0xC633:
case 0xC833:
case 0xCA33:
case 0xCC33:
case 0xCE33:
case 0xC034:
case 0xC234:
case 0xC434:
case 0xC634:
case 0xC834:
case 0xCA34:
case 0xCC34:
case 0xCE34:
case 0xC035:
case 0xC235:
case 0xC435:
case 0xC635:
case 0xC835:
case 0xCA35:
case 0xCC35:
case 0xCE35:
case 0xC036:
case 0xC236:
case 0xC436:
case 0xC636:
case 0xC836:
case 0xCA36:
case 0xCC36:
case 0xCE36:
case 0xC037:
case 0xC237:
case 0xC437:
case 0xC637:
case 0xC837:
case 0xCA37:
case 0xCC37:
case 0xCE37:

// ANDaD
case 0xC030:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0xC238:
case 0xC438:
case 0xC638:
case 0xC838:
case 0xCA38:
case 0xCC38:
case 0xCE38:

// ANDaD
case 0xC038:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0xC239:
case 0xC439:
case 0xC639:
case 0xC839:
case 0xCA39:
case 0xCC39:
case 0xCE39:

// ANDaD
case 0xC039:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0xC23A:
case 0xC43A:
case 0xC63A:
case 0xC83A:
case 0xCA3A:
case 0xCC3A:
case 0xCE3A:

// ANDaD
case 0xC03A:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0xC23B:
case 0xC43B:
case 0xC63B:
case 0xC83B:
case 0xCA3B:
case 0xCC3B:
case 0xCE3B:

// ANDaD
case 0xC03B:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0xC23C:
case 0xC43C:
case 0xC63C:
case 0xC83C:
case 0xCA3C:
case 0xCC3C:
case 0xCE3C:

// ANDaD
case 0xC03C:
{
	u32 res;
	u32 src;
	src = FETCH_BYTE;
	PC += 2;
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0xC21F:
case 0xC41F:
case 0xC61F:
case 0xC81F:
case 0xCA1F:
case 0xCC1F:
case 0xCE1F:

// ANDaD
case 0xC01F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0xC227:
case 0xC427:
case 0xC627:
case 0xC827:
case 0xCA27:
case 0xCC27:
case 0xCE27:

// ANDaD
case 0xC027:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0xC240:
case 0xC440:
case 0xC640:
case 0xC840:
case 0xCA40:
case 0xCC40:
case 0xCE40:
case 0xC041:
case 0xC241:
case 0xC441:
case 0xC641:
case 0xC841:
case 0xCA41:
case 0xCC41:
case 0xCE41:
case 0xC042:
case 0xC242:
case 0xC442:
case 0xC642:
case 0xC842:
case 0xCA42:
case 0xCC42:
case 0xCE42:
case 0xC043:
case 0xC243:
case 0xC443:
case 0xC643:
case 0xC843:
case 0xCA43:
case 0xCC43:
case 0xCE43:
case 0xC044:
case 0xC244:
case 0xC444:
case 0xC644:
case 0xC844:
case 0xCA44:
case 0xCC44:
case 0xCE44:
case 0xC045:
case 0xC245:
case 0xC445:
case 0xC645:
case 0xC845:
case 0xCA45:
case 0xCC45:
case 0xCE45:
case 0xC046:
case 0xC246:
case 0xC446:
case 0xC646:
case 0xC846:
case 0xCA46:
case 0xCC46:
case 0xCE46:
case 0xC047:
case 0xC247:
case 0xC447:
case 0xC647:
case 0xC847:
case 0xCA47:
case 0xCC47:
case 0xCE47:

// ANDaD
case 0xC040:
{
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0xC250:
case 0xC450:
case 0xC650:
case 0xC850:
case 0xCA50:
case 0xCC50:
case 0xCE50:
case 0xC051:
case 0xC251:
case 0xC451:
case 0xC651:
case 0xC851:
case 0xCA51:
case 0xCC51:
case 0xCE51:
case 0xC052:
case 0xC252:
case 0xC452:
case 0xC652:
case 0xC852:
case 0xCA52:
case 0xCC52:
case 0xCE52:
case 0xC053:
case 0xC253:
case 0xC453:
case 0xC653:
case 0xC853:
case 0xCA53:
case 0xCC53:
case 0xCE53:
case 0xC054:
case 0xC254:
case 0xC454:
case 0xC654:
case 0xC854:
case 0xCA54:
case 0xCC54:
case 0xCE54:
case 0xC055:
case 0xC255:
case 0xC455:
case 0xC655:
case 0xC855:
case 0xCA55:
case 0xCC55:
case 0xCE55:
case 0xC056:
case 0xC256:
case 0xC456:
case 0xC656:
case 0xC856:
case 0xCA56:
case 0xCC56:
case 0xCE56:
case 0xC057:
case 0xC257:
case 0xC457:
case 0xC657:
case 0xC857:
case 0xCA57:
case 0xCC57:
case 0xCE57:

// ANDaD
case 0xC050:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0xC258:
case 0xC458:
case 0xC658:
case 0xC858:
case 0xCA58:
case 0xCC58:
case 0xCE58:
case 0xC059:
case 0xC259:
case 0xC459:
case 0xC659:
case 0xC859:
case 0xCA59:
case 0xCC59:
case 0xCE59:
case 0xC05A:
case 0xC25A:
case 0xC45A:
case 0xC65A:
case 0xC85A:
case 0xCA5A:
case 0xCC5A:
case 0xCE5A:
case 0xC05B:
case 0xC25B:
case 0xC45B:
case 0xC65B:
case 0xC85B:
case 0xCA5B:
case 0xCC5B:
case 0xCE5B:
case 0xC05C:
case 0xC25C:
case 0xC45C:
case 0xC65C:
case 0xC85C:
case 0xCA5C:
case 0xCC5C:
case 0xCE5C:
case 0xC05D:
case 0xC25D:
case 0xC45D:
case 0xC65D:
case 0xC85D:
case 0xCA5D:
case 0xCC5D:
case 0xCE5D:
case 0xC05E:
case 0xC25E:
case 0xC45E:
case 0xC65E:
case 0xC85E:
case 0xCA5E:
case 0xCC5E:
case 0xCE5E:

// ANDaD
case 0xC058:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0xC260:
case 0xC460:
case 0xC660:
case 0xC860:
case 0xCA60:
case 0xCC60:
case 0xCE60:
case 0xC061:
case 0xC261:
case 0xC461:
case 0xC661:
case 0xC861:
case 0xCA61:
case 0xCC61:
case 0xCE61:
case 0xC062:
case 0xC262:
case 0xC462:
case 0xC662:
case 0xC862:
case 0xCA62:
case 0xCC62:
case 0xCE62:
case 0xC063:
case 0xC263:
case 0xC463:
case 0xC663:
case 0xC863:
case 0xCA63:
case 0xCC63:
case 0xCE63:
case 0xC064:
case 0xC264:
case 0xC464:
case 0xC664:
case 0xC864:
case 0xCA64:
case 0xCC64:
case 0xCE64:
case 0xC065:
case 0xC265:
case 0xC465:
case 0xC665:
case 0xC865:
case 0xCA65:
case 0xCC65:
case 0xCE65:
case 0xC066:
case 0xC266:
case 0xC466:
case 0xC666:
case 0xC866:
case 0xCA66:
case 0xCC66:
case 0xCE66:

// ANDaD
case 0xC060:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0xC268:
case 0xC468:
case 0xC668:
case 0xC868:
case 0xCA68:
case 0xCC68:
case 0xCE68:
case 0xC069:
case 0xC269:
case 0xC469:
case 0xC669:
case 0xC869:
case 0xCA69:
case 0xCC69:
case 0xCE69:
case 0xC06A:
case 0xC26A:
case 0xC46A:
case 0xC66A:
case 0xC86A:
case 0xCA6A:
case 0xCC6A:
case 0xCE6A:
case 0xC06B:
case 0xC26B:
case 0xC46B:
case 0xC66B:
case 0xC86B:
case 0xCA6B:
case 0xCC6B:
case 0xCE6B:
case 0xC06C:
case 0xC26C:
case 0xC46C:
case 0xC66C:
case 0xC86C:
case 0xCA6C:
case 0xCC6C:
case 0xCE6C:
case 0xC06D:
case 0xC26D:
case 0xC46D:
case 0xC66D:
case 0xC86D:
case 0xCA6D:
case 0xCC6D:
case 0xCE6D:
case 0xC06E:
case 0xC26E:
case 0xC46E:
case 0xC66E:
case 0xC86E:
case 0xCA6E:
case 0xCC6E:
case 0xCE6E:
case 0xC06F:
case 0xC26F:
case 0xC46F:
case 0xC66F:
case 0xC86F:
case 0xCA6F:
case 0xCC6F:
case 0xCE6F:

// ANDaD
case 0xC068:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0xC270:
case 0xC470:
case 0xC670:
case 0xC870:
case 0xCA70:
case 0xCC70:
case 0xCE70:
case 0xC071:
case 0xC271:
case 0xC471:
case 0xC671:
case 0xC871:
case 0xCA71:
case 0xCC71:
case 0xCE71:
case 0xC072:
case 0xC272:
case 0xC472:
case 0xC672:
case 0xC872:
case 0xCA72:
case 0xCC72:
case 0xCE72:
case 0xC073:
case 0xC273:
case 0xC473:
case 0xC673:
case 0xC873:
case 0xCA73:
case 0xCC73:
case 0xCE73:
case 0xC074:
case 0xC274:
case 0xC474:
case 0xC674:
case 0xC874:
case 0xCA74:
case 0xCC74:
case 0xCE74:
case 0xC075:
case 0xC275:
case 0xC475:
case 0xC675:
case 0xC875:
case 0xCA75:
case 0xCC75:
case 0xCE75:
case 0xC076:
case 0xC276:
case 0xC476:
case 0xC676:
case 0xC876:
case 0xCA76:
case 0xCC76:
case 0xCE76:
case 0xC077:
case 0xC277:
case 0xC477:
case 0xC677:
case 0xC877:
case 0xCA77:
case 0xCC77:
case 0xCE77:

// ANDaD
case 0xC070:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0xC278:
case 0xC478:
case 0xC678:
case 0xC878:
case 0xCA78:
case 0xCC78:
case 0xCE78:

// ANDaD
case 0xC078:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0xC279:
case 0xC479:
case 0xC679:
case 0xC879:
case 0xCA79:
case 0xCC79:
case 0xCE79:

// ANDaD
case 0xC079:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0xC27A:
case 0xC47A:
case 0xC67A:
case 0xC87A:
case 0xCA7A:
case 0xCC7A:
case 0xCE7A:

// ANDaD
case 0xC07A:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0xC27B:
case 0xC47B:
case 0xC67B:
case 0xC87B:
case 0xCA7B:
case 0xCC7B:
case 0xCE7B:

// ANDaD
case 0xC07B:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0xC27C:
case 0xC47C:
case 0xC67C:
case 0xC87C:
case 0xCA7C:
case 0xCC7C:
case 0xCE7C:

// ANDaD
case 0xC07C:
{
	u32 res;
	u32 src;
	src = FETCH_WORD;
	PC += 2;
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0xC25F:
case 0xC45F:
case 0xC65F:
case 0xC85F:
case 0xCA5F:
case 0xCC5F:
case 0xCE5F:

// ANDaD
case 0xC05F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0xC267:
case 0xC467:
case 0xC667:
case 0xC867:
case 0xCA67:
case 0xCC67:
case 0xCE67:

// ANDaD
case 0xC067:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0xC280:
case 0xC480:
case 0xC680:
case 0xC880:
case 0xCA80:
case 0xCC80:
case 0xCE80:
case 0xC081:
case 0xC281:
case 0xC481:
case 0xC681:
case 0xC881:
case 0xCA81:
case 0xCC81:
case 0xCE81:
case 0xC082:
case 0xC282:
case 0xC482:
case 0xC682:
case 0xC882:
case 0xCA82:
case 0xCC82:
case 0xCE82:
case 0xC083:
case 0xC283:
case 0xC483:
case 0xC683:
case 0xC883:
case 0xCA83:
case 0xCC83:
case 0xCE83:
case 0xC084:
case 0xC284:
case 0xC484:
case 0xC684:
case 0xC884:
case 0xCA84:
case 0xCC84:
case 0xCE84:
case 0xC085:
case 0xC285:
case 0xC485:
case 0xC685:
case 0xC885:
case 0xCA85:
case 0xCC85:
case 0xCE85:
case 0xC086:
case 0xC286:
case 0xC486:
case 0xC686:
case 0xC886:
case 0xCA86:
case 0xCC86:
case 0xCE86:
case 0xC087:
case 0xC287:
case 0xC487:
case 0xC687:
case 0xC887:
case 0xCA87:
case 0xCC87:
case 0xCE87:

// ANDaD
case 0xC080:
{
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0xC290:
case 0xC490:
case 0xC690:
case 0xC890:
case 0xCA90:
case 0xCC90:
case 0xCE90:
case 0xC091:
case 0xC291:
case 0xC491:
case 0xC691:
case 0xC891:
case 0xCA91:
case 0xCC91:
case 0xCE91:
case 0xC092:
case 0xC292:
case 0xC492:
case 0xC692:
case 0xC892:
case 0xCA92:
case 0xCC92:
case 0xCE92:
case 0xC093:
case 0xC293:
case 0xC493:
case 0xC693:
case 0xC893:
case 0xCA93:
case 0xCC93:
case 0xCE93:
case 0xC094:
case 0xC294:
case 0xC494:
case 0xC694:
case 0xC894:
case 0xCA94:
case 0xCC94:
case 0xCE94:
case 0xC095:
case 0xC295:
case 0xC495:
case 0xC695:
case 0xC895:
case 0xCA95:
case 0xCC95:
case 0xCE95:
case 0xC096:
case 0xC296:
case 0xC496:
case 0xC696:
case 0xC896:
case 0xCA96:
case 0xCC96:
case 0xCE96:
case 0xC097:
case 0xC297:
case 0xC497:
case 0xC697:
case 0xC897:
case 0xCA97:
case 0xCC97:
case 0xCE97:

// ANDaD
case 0xC090:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0xC298:
case 0xC498:
case 0xC698:
case 0xC898:
case 0xCA98:
case 0xCC98:
case 0xCE98:
case 0xC099:
case 0xC299:
case 0xC499:
case 0xC699:
case 0xC899:
case 0xCA99:
case 0xCC99:
case 0xCE99:
case 0xC09A:
case 0xC29A:
case 0xC49A:
case 0xC69A:
case 0xC89A:
case 0xCA9A:
case 0xCC9A:
case 0xCE9A:
case 0xC09B:
case 0xC29B:
case 0xC49B:
case 0xC69B:
case 0xC89B:
case 0xCA9B:
case 0xCC9B:
case 0xCE9B:
case 0xC09C:
case 0xC29C:
case 0xC49C:
case 0xC69C:
case 0xC89C:
case 0xCA9C:
case 0xCC9C:
case 0xCE9C:
case 0xC09D:
case 0xC29D:
case 0xC49D:
case 0xC69D:
case 0xC89D:
case 0xCA9D:
case 0xCC9D:
case 0xCE9D:
case 0xC09E:
case 0xC29E:
case 0xC49E:
case 0xC69E:
case 0xC89E:
case 0xCA9E:
case 0xCC9E:
case 0xCE9E:

// ANDaD
case 0xC098:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0xC2A0:
case 0xC4A0:
case 0xC6A0:
case 0xC8A0:
case 0xCAA0:
case 0xCCA0:
case 0xCEA0:
case 0xC0A1:
case 0xC2A1:
case 0xC4A1:
case 0xC6A1:
case 0xC8A1:
case 0xCAA1:
case 0xCCA1:
case 0xCEA1:
case 0xC0A2:
case 0xC2A2:
case 0xC4A2:
case 0xC6A2:
case 0xC8A2:
case 0xCAA2:
case 0xCCA2:
case 0xCEA2:
case 0xC0A3:
case 0xC2A3:
case 0xC4A3:
case 0xC6A3:
case 0xC8A3:
case 0xCAA3:
case 0xCCA3:
case 0xCEA3:
case 0xC0A4:
case 0xC2A4:
case 0xC4A4:
case 0xC6A4:
case 0xC8A4:
case 0xCAA4:
case 0xCCA4:
case 0xCEA4:
case 0xC0A5:
case 0xC2A5:
case 0xC4A5:
case 0xC6A5:
case 0xC8A5:
case 0xCAA5:
case 0xCCA5:
case 0xCEA5:
case 0xC0A6:
case 0xC2A6:
case 0xC4A6:
case 0xC6A6:
case 0xC8A6:
case 0xCAA6:
case 0xCCA6:
case 0xCEA6:

// ANDaD
case 0xC0A0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0xC2A8:
case 0xC4A8:
case 0xC6A8:
case 0xC8A8:
case 0xCAA8:
case 0xCCA8:
case 0xCEA8:
case 0xC0A9:
case 0xC2A9:
case 0xC4A9:
case 0xC6A9:
case 0xC8A9:
case 0xCAA9:
case 0xCCA9:
case 0xCEA9:
case 0xC0AA:
case 0xC2AA:
case 0xC4AA:
case 0xC6AA:
case 0xC8AA:
case 0xCAAA:
case 0xCCAA:
case 0xCEAA:
case 0xC0AB:
case 0xC2AB:
case 0xC4AB:
case 0xC6AB:
case 0xC8AB:
case 0xCAAB:
case 0xCCAB:
case 0xCEAB:
case 0xC0AC:
case 0xC2AC:
case 0xC4AC:
case 0xC6AC:
case 0xC8AC:
case 0xCAAC:
case 0xCCAC:
case 0xCEAC:
case 0xC0AD:
case 0xC2AD:
case 0xC4AD:
case 0xC6AD:
case 0xC8AD:
case 0xCAAD:
case 0xCCAD:
case 0xCEAD:
case 0xC0AE:
case 0xC2AE:
case 0xC4AE:
case 0xC6AE:
case 0xC8AE:
case 0xCAAE:
case 0xCCAE:
case 0xCEAE:
case 0xC0AF:
case 0xC2AF:
case 0xC4AF:
case 0xC6AF:
case 0xC8AF:
case 0xCAAF:
case 0xCCAF:
case 0xCEAF:

// ANDaD
case 0xC0A8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(18)
case 0xC2B0:
case 0xC4B0:
case 0xC6B0:
case 0xC8B0:
case 0xCAB0:
case 0xCCB0:
case 0xCEB0:
case 0xC0B1:
case 0xC2B1:
case 0xC4B1:
case 0xC6B1:
case 0xC8B1:
case 0xCAB1:
case 0xCCB1:
case 0xCEB1:
case 0xC0B2:
case 0xC2B2:
case 0xC4B2:
case 0xC6B2:
case 0xC8B2:
case 0xCAB2:
case 0xCCB2:
case 0xCEB2:
case 0xC0B3:
case 0xC2B3:
case 0xC4B3:
case 0xC6B3:
case 0xC8B3:
case 0xCAB3:
case 0xCCB3:
case 0xCEB3:
case 0xC0B4:
case 0xC2B4:
case 0xC4B4:
case 0xC6B4:
case 0xC8B4:
case 0xCAB4:
case 0xCCB4:
case 0xCEB4:
case 0xC0B5:
case 0xC2B5:
case 0xC4B5:
case 0xC6B5:
case 0xC8B5:
case 0xCAB5:
case 0xCCB5:
case 0xCEB5:
case 0xC0B6:
case 0xC2B6:
case 0xC4B6:
case 0xC6B6:
case 0xC8B6:
case 0xCAB6:
case 0xCCB6:
case 0xCEB6:
case 0xC0B7:
case 0xC2B7:
case 0xC4B7:
case 0xC6B7:
case 0xC8B7:
case 0xCAB7:
case 0xCCB7:
case 0xCEB7:

// ANDaD
case 0xC0B0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(20)
case 0xC2B8:
case 0xC4B8:
case 0xC6B8:
case 0xC8B8:
case 0xCAB8:
case 0xCCB8:
case 0xCEB8:

// ANDaD
case 0xC0B8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(18)
case 0xC2B9:
case 0xC4B9:
case 0xC6B9:
case 0xC8B9:
case 0xCAB9:
case 0xCCB9:
case 0xCEB9:

// ANDaD
case 0xC0B9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(22)
case 0xC2BA:
case 0xC4BA:
case 0xC6BA:
case 0xC8BA:
case 0xCABA:
case 0xCCBA:
case 0xCEBA:

// ANDaD
case 0xC0BA:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(18)
case 0xC2BB:
case 0xC4BB:
case 0xC6BB:
case 0xC8BB:
case 0xCABB:
case 0xCCBB:
case 0xCEBB:

// ANDaD
case 0xC0BB:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(20)
case 0xC2BC:
case 0xC4BC:
case 0xC6BC:
case 0xC8BC:
case 0xCABC:
case 0xCCBC:
case 0xCEBC:

// ANDaD
case 0xC0BC:
{
	u32 res;
	u32 src;
	src = FETCH_LONG;
	PC += 4;
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(16)
case 0xC29F:
case 0xC49F:
case 0xC69F:
case 0xC89F:
case 0xCA9F:
case 0xCC9F:
case 0xCE9F:

// ANDaD
case 0xC09F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0xC2A7:
case 0xC4A7:
case 0xC6A7:
case 0xC8A7:
case 0xCAA7:
case 0xCCA7:
case 0xCEA7:

// ANDaD
case 0xC0A7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0xC310:
case 0xC510:
case 0xC710:
case 0xC910:
case 0xCB10:
case 0xCD10:
case 0xCF10:
case 0xC111:
case 0xC311:
case 0xC511:
case 0xC711:
case 0xC911:
case 0xCB11:
case 0xCD11:
case 0xCF11:
case 0xC112:
case 0xC312:
case 0xC512:
case 0xC712:
case 0xC912:
case 0xCB12:
case 0xCD12:
case 0xCF12:
case 0xC113:
case 0xC313:
case 0xC513:
case 0xC713:
case 0xC913:
case 0xCB13:
case 0xCD13:
case 0xCF13:
case 0xC114:
case 0xC314:
case 0xC514:
case 0xC714:
case 0xC914:
case 0xCB14:
case 0xCD14:
case 0xCF14:
case 0xC115:
case 0xC315:
case 0xC515:
case 0xC715:
case 0xC915:
case 0xCB15:
case 0xCD15:
case 0xCF15:
case 0xC116:
case 0xC316:
case 0xC516:
case 0xC716:
case 0xC916:
case 0xCB16:
case 0xCD16:
case 0xCF16:
case 0xC117:
case 0xC317:
case 0xC517:
case 0xC717:
case 0xC917:
case 0xCB17:
case 0xCD17:
case 0xCF17:

// ANDDa
case 0xC110:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0xC318:
case 0xC518:
case 0xC718:
case 0xC918:
case 0xCB18:
case 0xCD18:
case 0xCF18:
case 0xC119:
case 0xC319:
case 0xC519:
case 0xC719:
case 0xC919:
case 0xCB19:
case 0xCD19:
case 0xCF19:
case 0xC11A:
case 0xC31A:
case 0xC51A:
case 0xC71A:
case 0xC91A:
case 0xCB1A:
case 0xCD1A:
case 0xCF1A:
case 0xC11B:
case 0xC31B:
case 0xC51B:
case 0xC71B:
case 0xC91B:
case 0xCB1B:
case 0xCD1B:
case 0xCF1B:
case 0xC11C:
case 0xC31C:
case 0xC51C:
case 0xC71C:
case 0xC91C:
case 0xCB1C:
case 0xCD1C:
case 0xCF1C:
case 0xC11D:
case 0xC31D:
case 0xC51D:
case 0xC71D:
case 0xC91D:
case 0xCB1D:
case 0xCD1D:
case 0xCF1D:
case 0xC11E:
case 0xC31E:
case 0xC51E:
case 0xC71E:
case 0xC91E:
case 0xCB1E:
case 0xCD1E:
case 0xCF1E:

// ANDDa
case 0xC118:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0xC320:
case 0xC520:
case 0xC720:
case 0xC920:
case 0xCB20:
case 0xCD20:
case 0xCF20:
case 0xC121:
case 0xC321:
case 0xC521:
case 0xC721:
case 0xC921:
case 0xCB21:
case 0xCD21:
case 0xCF21:
case 0xC122:
case 0xC322:
case 0xC522:
case 0xC722:
case 0xC922:
case 0xCB22:
case 0xCD22:
case 0xCF22:
case 0xC123:
case 0xC323:
case 0xC523:
case 0xC723:
case 0xC923:
case 0xCB23:
case 0xCD23:
case 0xCF23:
case 0xC124:
case 0xC324:
case 0xC524:
case 0xC724:
case 0xC924:
case 0xCB24:
case 0xCD24:
case 0xCF24:
case 0xC125:
case 0xC325:
case 0xC525:
case 0xC725:
case 0xC925:
case 0xCB25:
case 0xCD25:
case 0xCF25:
case 0xC126:
case 0xC326:
case 0xC526:
case 0xC726:
case 0xC926:
case 0xCB26:
case 0xCD26:
case 0xCF26:

// ANDDa
case 0xC120:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0xC328:
case 0xC528:
case 0xC728:
case 0xC928:
case 0xCB28:
case 0xCD28:
case 0xCF28:
case 0xC129:
case 0xC329:
case 0xC529:
case 0xC729:
case 0xC929:
case 0xCB29:
case 0xCD29:
case 0xCF29:
case 0xC12A:
case 0xC32A:
case 0xC52A:
case 0xC72A:
case 0xC92A:
case 0xCB2A:
case 0xCD2A:
case 0xCF2A:
case 0xC12B:
case 0xC32B:
case 0xC52B:
case 0xC72B:
case 0xC92B:
case 0xCB2B:
case 0xCD2B:
case 0xCF2B:
case 0xC12C:
case 0xC32C:
case 0xC52C:
case 0xC72C:
case 0xC92C:
case 0xCB2C:
case 0xCD2C:
case 0xCF2C:
case 0xC12D:
case 0xC32D:
case 0xC52D:
case 0xC72D:
case 0xC92D:
case 0xCB2D:
case 0xCD2D:
case 0xCF2D:
case 0xC12E:
case 0xC32E:
case 0xC52E:
case 0xC72E:
case 0xC92E:
case 0xCB2E:
case 0xCD2E:
case 0xCF2E:
case 0xC12F:
case 0xC32F:
case 0xC52F:
case 0xC72F:
case 0xC92F:
case 0xCB2F:
case 0xCD2F:
case 0xCF2F:

// ANDDa
case 0xC128:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0xC330:
case 0xC530:
case 0xC730:
case 0xC930:
case 0xCB30:
case 0xCD30:
case 0xCF30:
case 0xC131:
case 0xC331:
case 0xC531:
case 0xC731:
case 0xC931:
case 0xCB31:
case 0xCD31:
case 0xCF31:
case 0xC132:
case 0xC332:
case 0xC532:
case 0xC732:
case 0xC932:
case 0xCB32:
case 0xCD32:
case 0xCF32:
case 0xC133:
case 0xC333:
case 0xC533:
case 0xC733:
case 0xC933:
case 0xCB33:
case 0xCD33:
case 0xCF33:
case 0xC134:
case 0xC334:
case 0xC534:
case 0xC734:
case 0xC934:
case 0xCB34:
case 0xCD34:
case 0xCF34:
case 0xC135:
case 0xC335:
case 0xC535:
case 0xC735:
case 0xC935:
case 0xCB35:
case 0xCD35:
case 0xCF35:
case 0xC136:
case 0xC336:
case 0xC536:
case 0xC736:
case 0xC936:
case 0xCB36:
case 0xCD36:
case 0xCF36:
case 0xC137:
case 0xC337:
case 0xC537:
case 0xC737:
case 0xC937:
case 0xCB37:
case 0xCD37:
case 0xCF37:

// ANDDa
case 0xC130:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0xC338:
case 0xC538:
case 0xC738:
case 0xC938:
case 0xCB38:
case 0xCD38:
case 0xCF38:

// ANDDa
case 0xC138:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0xC339:
case 0xC539:
case 0xC739:
case 0xC939:
case 0xCB39:
case 0xCD39:
case 0xCF39:

// ANDDa
case 0xC139:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_BYTE_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0xC31F:
case 0xC51F:
case 0xC71F:
case 0xC91F:
case 0xCB1F:
case 0xCD1F:
case 0xCF1F:

// ANDDa
case 0xC11F:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0xC327:
case 0xC527:
case 0xC727:
case 0xC927:
case 0xCB27:
case 0xCD27:
case 0xCF27:

// ANDDa
case 0xC127:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0xC350:
case 0xC550:
case 0xC750:
case 0xC950:
case 0xCB50:
case 0xCD50:
case 0xCF50:
case 0xC151:
case 0xC351:
case 0xC551:
case 0xC751:
case 0xC951:
case 0xCB51:
case 0xCD51:
case 0xCF51:
case 0xC152:
case 0xC352:
case 0xC552:
case 0xC752:
case 0xC952:
case 0xCB52:
case 0xCD52:
case 0xCF52:
case 0xC153:
case 0xC353:
case 0xC553:
case 0xC753:
case 0xC953:
case 0xCB53:
case 0xCD53:
case 0xCF53:
case 0xC154:
case 0xC354:
case 0xC554:
case 0xC754:
case 0xC954:
case 0xCB54:
case 0xCD54:
case 0xCF54:
case 0xC155:
case 0xC355:
case 0xC555:
case 0xC755:
case 0xC955:
case 0xCB55:
case 0xCD55:
case 0xCF55:
case 0xC156:
case 0xC356:
case 0xC556:
case 0xC756:
case 0xC956:
case 0xCB56:
case 0xCD56:
case 0xCF56:
case 0xC157:
case 0xC357:
case 0xC557:
case 0xC757:
case 0xC957:
case 0xCB57:
case 0xCD57:
case 0xCF57:

// ANDDa
case 0xC150:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xC358:
case 0xC558:
case 0xC758:
case 0xC958:
case 0xCB58:
case 0xCD58:
case 0xCF58:
case 0xC159:
case 0xC359:
case 0xC559:
case 0xC759:
case 0xC959:
case 0xCB59:
case 0xCD59:
case 0xCF59:
case 0xC15A:
case 0xC35A:
case 0xC55A:
case 0xC75A:
case 0xC95A:
case 0xCB5A:
case 0xCD5A:
case 0xCF5A:
case 0xC15B:
case 0xC35B:
case 0xC55B:
case 0xC75B:
case 0xC95B:
case 0xCB5B:
case 0xCD5B:
case 0xCF5B:
case 0xC15C:
case 0xC35C:
case 0xC55C:
case 0xC75C:
case 0xC95C:
case 0xCB5C:
case 0xCD5C:
case 0xCF5C:
case 0xC15D:
case 0xC35D:
case 0xC55D:
case 0xC75D:
case 0xC95D:
case 0xCB5D:
case 0xCD5D:
case 0xCF5D:
case 0xC15E:
case 0xC35E:
case 0xC55E:
case 0xC75E:
case 0xC95E:
case 0xCB5E:
case 0xCD5E:
case 0xCF5E:

// ANDDa
case 0xC158:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xC360:
case 0xC560:
case 0xC760:
case 0xC960:
case 0xCB60:
case 0xCD60:
case 0xCF60:
case 0xC161:
case 0xC361:
case 0xC561:
case 0xC761:
case 0xC961:
case 0xCB61:
case 0xCD61:
case 0xCF61:
case 0xC162:
case 0xC362:
case 0xC562:
case 0xC762:
case 0xC962:
case 0xCB62:
case 0xCD62:
case 0xCF62:
case 0xC163:
case 0xC363:
case 0xC563:
case 0xC763:
case 0xC963:
case 0xCB63:
case 0xCD63:
case 0xCF63:
case 0xC164:
case 0xC364:
case 0xC564:
case 0xC764:
case 0xC964:
case 0xCB64:
case 0xCD64:
case 0xCF64:
case 0xC165:
case 0xC365:
case 0xC565:
case 0xC765:
case 0xC965:
case 0xCB65:
case 0xCD65:
case 0xCF65:
case 0xC166:
case 0xC366:
case 0xC566:
case 0xC766:
case 0xC966:
case 0xCB66:
case 0xCD66:
case 0xCF66:

// ANDDa
case 0xC160:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xC368:
case 0xC568:
case 0xC768:
case 0xC968:
case 0xCB68:
case 0xCD68:
case 0xCF68:
case 0xC169:
case 0xC369:
case 0xC569:
case 0xC769:
case 0xC969:
case 0xCB69:
case 0xCD69:
case 0xCF69:
case 0xC16A:
case 0xC36A:
case 0xC56A:
case 0xC76A:
case 0xC96A:
case 0xCB6A:
case 0xCD6A:
case 0xCF6A:
case 0xC16B:
case 0xC36B:
case 0xC56B:
case 0xC76B:
case 0xC96B:
case 0xCB6B:
case 0xCD6B:
case 0xCF6B:
case 0xC16C:
case 0xC36C:
case 0xC56C:
case 0xC76C:
case 0xC96C:
case 0xCB6C:
case 0xCD6C:
case 0xCF6C:
case 0xC16D:
case 0xC36D:
case 0xC56D:
case 0xC76D:
case 0xC96D:
case 0xCB6D:
case 0xCD6D:
case 0xCF6D:
case 0xC16E:
case 0xC36E:
case 0xC56E:
case 0xC76E:
case 0xC96E:
case 0xCB6E:
case 0xCD6E:
case 0xCF6E:
case 0xC16F:
case 0xC36F:
case 0xC56F:
case 0xC76F:
case 0xC96F:
case 0xCB6F:
case 0xCD6F:
case 0xCF6F:

// ANDDa
case 0xC168:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0xC370:
case 0xC570:
case 0xC770:
case 0xC970:
case 0xCB70:
case 0xCD70:
case 0xCF70:
case 0xC171:
case 0xC371:
case 0xC571:
case 0xC771:
case 0xC971:
case 0xCB71:
case 0xCD71:
case 0xCF71:
case 0xC172:
case 0xC372:
case 0xC572:
case 0xC772:
case 0xC972:
case 0xCB72:
case 0xCD72:
case 0xCF72:
case 0xC173:
case 0xC373:
case 0xC573:
case 0xC773:
case 0xC973:
case 0xCB73:
case 0xCD73:
case 0xCF73:
case 0xC174:
case 0xC374:
case 0xC574:
case 0xC774:
case 0xC974:
case 0xCB74:
case 0xCD74:
case 0xCF74:
case 0xC175:
case 0xC375:
case 0xC575:
case 0xC775:
case 0xC975:
case 0xCB75:
case 0xCD75:
case 0xCF75:
case 0xC176:
case 0xC376:
case 0xC576:
case 0xC776:
case 0xC976:
case 0xCB76:
case 0xCD76:
case 0xCF76:
case 0xC177:
case 0xC377:
case 0xC577:
case 0xC777:
case 0xC977:
case 0xCB77:
case 0xCD77:
case 0xCF77:

// ANDDa
case 0xC170:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0xC378:
case 0xC578:
case 0xC778:
case 0xC978:
case 0xCB78:
case 0xCD78:
case 0xCF78:

// ANDDa
case 0xC178:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0xC379:
case 0xC579:
case 0xC779:
case 0xC979:
case 0xCB79:
case 0xCD79:
case 0xCF79:

// ANDDa
case 0xC179:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0xC35F:
case 0xC55F:
case 0xC75F:
case 0xC95F:
case 0xCB5F:
case 0xCD5F:
case 0xCF5F:

// ANDDa
case 0xC15F:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xC367:
case 0xC567:
case 0xC767:
case 0xC967:
case 0xCB67:
case 0xCD67:
case 0xCF67:

// ANDDa
case 0xC167:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xC390:
case 0xC590:
case 0xC790:
case 0xC990:
case 0xCB90:
case 0xCD90:
case 0xCF90:
case 0xC191:
case 0xC391:
case 0xC591:
case 0xC791:
case 0xC991:
case 0xCB91:
case 0xCD91:
case 0xCF91:
case 0xC192:
case 0xC392:
case 0xC592:
case 0xC792:
case 0xC992:
case 0xCB92:
case 0xCD92:
case 0xCF92:
case 0xC193:
case 0xC393:
case 0xC593:
case 0xC793:
case 0xC993:
case 0xCB93:
case 0xCD93:
case 0xCF93:
case 0xC194:
case 0xC394:
case 0xC594:
case 0xC794:
case 0xC994:
case 0xCB94:
case 0xCD94:
case 0xCF94:
case 0xC195:
case 0xC395:
case 0xC595:
case 0xC795:
case 0xC995:
case 0xCB95:
case 0xCD95:
case 0xCF95:
case 0xC196:
case 0xC396:
case 0xC596:
case 0xC796:
case 0xC996:
case 0xCB96:
case 0xCD96:
case 0xCF96:
case 0xC197:
case 0xC397:
case 0xC597:
case 0xC797:
case 0xC997:
case 0xCB97:
case 0xCD97:
case 0xCF97:

// ANDDa
case 0xC190:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_LONG_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0xC398:
case 0xC598:
case 0xC798:
case 0xC998:
case 0xCB98:
case 0xCD98:
case 0xCF98:
case 0xC199:
case 0xC399:
case 0xC599:
case 0xC799:
case 0xC999:
case 0xCB99:
case 0xCD99:
case 0xCF99:
case 0xC19A:
case 0xC39A:
case 0xC59A:
case 0xC79A:
case 0xC99A:
case 0xCB9A:
case 0xCD9A:
case 0xCF9A:
case 0xC19B:
case 0xC39B:
case 0xC59B:
case 0xC79B:
case 0xC99B:
case 0xCB9B:
case 0xCD9B:
case 0xCF9B:
case 0xC19C:
case 0xC39C:
case 0xC59C:
case 0xC79C:
case 0xC99C:
case 0xCB9C:
case 0xCD9C:
case 0xCF9C:
case 0xC19D:
case 0xC39D:
case 0xC59D:
case 0xC79D:
case 0xC99D:
case 0xCB9D:
case 0xCD9D:
case 0xCF9D:
case 0xC19E:
case 0xC39E:
case 0xC59E:
case 0xC79E:
case 0xC99E:
case 0xCB9E:
case 0xCD9E:
case 0xCF9E:

// ANDDa
case 0xC198:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READ_LONG_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0xC3A0:
case 0xC5A0:
case 0xC7A0:
case 0xC9A0:
case 0xCBA0:
case 0xCDA0:
case 0xCFA0:
case 0xC1A1:
case 0xC3A1:
case 0xC5A1:
case 0xC7A1:
case 0xC9A1:
case 0xCBA1:
case 0xCDA1:
case 0xCFA1:
case 0xC1A2:
case 0xC3A2:
case 0xC5A2:
case 0xC7A2:
case 0xC9A2:
case 0xCBA2:
case 0xCDA2:
case 0xCFA2:
case 0xC1A3:
case 0xC3A3:
case 0xC5A3:
case 0xC7A3:
case 0xC9A3:
case 0xCBA3:
case 0xCDA3:
case 0xCFA3:
case 0xC1A4:
case 0xC3A4:
case 0xC5A4:
case 0xC7A4:
case 0xC9A4:
case 0xCBA4:
case 0xCDA4:
case 0xCFA4:
case 0xC1A5:
case 0xC3A5:
case 0xC5A5:
case 0xC7A5:
case 0xC9A5:
case 0xCBA5:
case 0xCDA5:
case 0xCFA5:
case 0xC1A6:
case 0xC3A6:
case 0xC5A6:
case 0xC7A6:
case 0xC9A6:
case 0xCBA6:
case 0xCDA6:
case 0xCFA6:

// ANDDa
case 0xC1A0:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_LONG_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0xC3A8:
case 0xC5A8:
case 0xC7A8:
case 0xC9A8:
case 0xCBA8:
case 0xCDA8:
case 0xCFA8:
case 0xC1A9:
case 0xC3A9:
case 0xC5A9:
case 0xC7A9:
case 0xC9A9:
case 0xCBA9:
case 0xCDA9:
case 0xCFA9:
case 0xC1AA:
case 0xC3AA:
case 0xC5AA:
case 0xC7AA:
case 0xC9AA:
case 0xCBAA:
case 0xCDAA:
case 0xCFAA:
case 0xC1AB:
case 0xC3AB:
case 0xC5AB:
case 0xC7AB:
case 0xC9AB:
case 0xCBAB:
case 0xCDAB:
case 0xCFAB:
case 0xC1AC:
case 0xC3AC:
case 0xC5AC:
case 0xC7AC:
case 0xC9AC:
case 0xCBAC:
case 0xCDAC:
case 0xCFAC:
case 0xC1AD:
case 0xC3AD:
case 0xC5AD:
case 0xC7AD:
case 0xC9AD:
case 0xCBAD:
case 0xCDAD:
case 0xCFAD:
case 0xC1AE:
case 0xC3AE:
case 0xC5AE:
case 0xC7AE:
case 0xC9AE:
case 0xCBAE:
case 0xCDAE:
case 0xCFAE:
case 0xC1AF:
case 0xC3AF:
case 0xC5AF:
case 0xC7AF:
case 0xC9AF:
case 0xCBAF:
case 0xCDAF:
case 0xCFAF:

// ANDDa
case 0xC1A8:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0xC3B0:
case 0xC5B0:
case 0xC7B0:
case 0xC9B0:
case 0xCBB0:
case 0xCDB0:
case 0xCFB0:
case 0xC1B1:
case 0xC3B1:
case 0xC5B1:
case 0xC7B1:
case 0xC9B1:
case 0xCBB1:
case 0xCDB1:
case 0xCFB1:
case 0xC1B2:
case 0xC3B2:
case 0xC5B2:
case 0xC7B2:
case 0xC9B2:
case 0xCBB2:
case 0xCDB2:
case 0xCFB2:
case 0xC1B3:
case 0xC3B3:
case 0xC5B3:
case 0xC7B3:
case 0xC9B3:
case 0xCBB3:
case 0xCDB3:
case 0xCFB3:
case 0xC1B4:
case 0xC3B4:
case 0xC5B4:
case 0xC7B4:
case 0xC9B4:
case 0xCBB4:
case 0xCDB4:
case 0xCFB4:
case 0xC1B5:
case 0xC3B5:
case 0xC5B5:
case 0xC7B5:
case 0xC9B5:
case 0xCBB5:
case 0xCDB5:
case 0xCFB5:
case 0xC1B6:
case 0xC3B6:
case 0xC5B6:
case 0xC7B6:
case 0xC9B6:
case 0xCBB6:
case 0xCDB6:
case 0xCFB6:
case 0xC1B7:
case 0xC3B7:
case 0xC5B7:
case 0xC7B7:
case 0xC9B7:
case 0xCBB7:
case 0xCDB7:
case 0xCFB7:

// ANDDa
case 0xC1B0:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0xC3B8:
case 0xC5B8:
case 0xC7B8:
case 0xC9B8:
case 0xCBB8:
case 0xCDB8:
case 0xCFB8:

// ANDDa
case 0xC1B8:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0xC3B9:
case 0xC5B9:
case 0xC7B9:
case 0xC9B9:
case 0xCBB9:
case 0xCDB9:
case 0xCFB9:

// ANDDa
case 0xC1B9:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_LONG_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0xC39F:
case 0xC59F:
case 0xC79F:
case 0xC99F:
case 0xCB9F:
case 0xCD9F:
case 0xCF9F:

// ANDDa
case 0xC19F:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READ_LONG_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0xC3A7:
case 0xC5A7:
case 0xC7A7:
case 0xC9A7:
case 0xCBA7:
case 0xCDA7:
case 0xCFA7:

// ANDDa
case 0xC1A7:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READ_LONG_F(adr, res)
	res &= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0xC300:
case 0xC500:
case 0xC700:
case 0xC900:
case 0xCB00:
case 0xCD00:
case 0xCF00:
case 0xC101:
case 0xC301:
case 0xC501:
case 0xC701:
case 0xC901:
case 0xCB01:
case 0xCD01:
case 0xCF01:
case 0xC102:
case 0xC302:
case 0xC502:
case 0xC702:
case 0xC902:
case 0xCB02:
case 0xCD02:
case 0xCF02:
case 0xC103:
case 0xC303:
case 0xC503:
case 0xC703:
case 0xC903:
case 0xCB03:
case 0xCD03:
case 0xCF03:
case 0xC104:
case 0xC304:
case 0xC504:
case 0xC704:
case 0xC904:
case 0xCB04:
case 0xCD04:
case 0xCF04:
case 0xC105:
case 0xC305:
case 0xC505:
case 0xC705:
case 0xC905:
case 0xCB05:
case 0xCD05:
case 0xCF05:
case 0xC106:
case 0xC306:
case 0xC506:
case 0xC706:
case 0xC906:
case 0xCB06:
case 0xCD06:
case 0xCF06:
case 0xC107:
case 0xC307:
case 0xC507:
case 0xC707:
case 0xC907:
case 0xCB07:
case 0xCD07:
case 0xCF07:

// ABCD
case 0xC100:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = (dst & 0xF) + (src & 0xF) + ((CPU->flag_X >> C68K_SR_X_SFT) & 1);
	if (res > 9) res += 6;
	res += (dst & 0xF0) + (src & 0xF0);
	if (res > 0x99)
	{
		res -= 0xA0;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_notZ |= res & 0xFF;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(6)
case 0xC308:
case 0xC508:
case 0xC708:
case 0xC908:
case 0xCB08:
case 0xCD08:
case 0xC109:
case 0xC309:
case 0xC509:
case 0xC709:
case 0xC909:
case 0xCB09:
case 0xCD09:
case 0xC10A:
case 0xC30A:
case 0xC50A:
case 0xC70A:
case 0xC90A:
case 0xCB0A:
case 0xCD0A:
case 0xC10B:
case 0xC30B:
case 0xC50B:
case 0xC70B:
case 0xC90B:
case 0xCB0B:
case 0xCD0B:
case 0xC10C:
case 0xC30C:
case 0xC50C:
case 0xC70C:
case 0xC90C:
case 0xCB0C:
case 0xCD0C:
case 0xC10D:
case 0xC30D:
case 0xC50D:
case 0xC70D:
case 0xC90D:
case 0xCB0D:
case 0xCD0D:
case 0xC10E:
case 0xC30E:
case 0xC50E:
case 0xC70E:
case 0xC90E:
case 0xCB0E:
case 0xCD0E:

// ABCDM
case 0xC108:
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
	res = (dst & 0xF) + (src & 0xF) + ((CPU->flag_X >> C68K_SR_X_SFT) & 1);
	if (res > 9) res += 6;
	res += (dst & 0xF0) + (src & 0xF0);
	if (res > 0x99)
	{
		res -= 0xA0;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_notZ |= res & 0xFF;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0xC30F:
case 0xC50F:
case 0xC70F:
case 0xC90F:
case 0xCB0F:
case 0xCD0F:

// ABCD7M
case 0xC10F:
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
	res = (dst & 0xF) + (src & 0xF) + ((CPU->flag_X >> C68K_SR_X_SFT) & 1);
	if (res > 9) res += 6;
	res += (dst & 0xF0) + (src & 0xF0);
	if (res > 0x99)
	{
		res -= 0xA0;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_notZ |= res & 0xFF;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0xCF09:
case 0xCF0A:
case 0xCF0B:
case 0xCF0C:
case 0xCF0D:
case 0xCF0E:

// ABCDM7
case 0xCF08:
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
	res = (dst & 0xF) + (src & 0xF) + ((CPU->flag_X >> C68K_SR_X_SFT) & 1);
	if (res > 9) res += 6;
	res += (dst & 0xF0) + (src & 0xF0);
	if (res > 0x99)
	{
		res -= 0xA0;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_notZ |= res & 0xFF;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)

// ABCD7M7
case 0xCF0F:
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
	res = (dst & 0xF) + (src & 0xF) + ((CPU->flag_X >> C68K_SR_X_SFT) & 1);
	if (res > 9) res += 6;
	res += (dst & 0xF0) + (src & 0xF0);
	if (res > 0x99)
	{
		res -= 0xA0;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_notZ |= res & 0xFF;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0xC2C0:
case 0xC4C0:
case 0xC6C0:
case 0xC8C0:
case 0xCAC0:
case 0xCCC0:
case 0xCEC0:
case 0xC0C1:
case 0xC2C1:
case 0xC4C1:
case 0xC6C1:
case 0xC8C1:
case 0xCAC1:
case 0xCCC1:
case 0xCEC1:
case 0xC0C2:
case 0xC2C2:
case 0xC4C2:
case 0xC6C2:
case 0xC8C2:
case 0xCAC2:
case 0xCCC2:
case 0xCEC2:
case 0xC0C3:
case 0xC2C3:
case 0xC4C3:
case 0xC6C3:
case 0xC8C3:
case 0xCAC3:
case 0xCCC3:
case 0xCEC3:
case 0xC0C4:
case 0xC2C4:
case 0xC4C4:
case 0xC6C4:
case 0xC8C4:
case 0xCAC4:
case 0xCCC4:
case 0xCEC4:
case 0xC0C5:
case 0xC2C5:
case 0xC4C5:
case 0xC6C5:
case 0xC8C5:
case 0xCAC5:
case 0xCCC5:
case 0xCEC5:
case 0xC0C6:
case 0xC2C6:
case 0xC4C6:
case 0xC6C6:
case 0xC8C6:
case 0xCAC6:
case 0xCCC6:
case 0xCEC6:
case 0xC0C7:
case 0xC2C7:
case 0xC4C7:
case 0xC6C7:
case 0xC8C7:
case 0xCAC7:
case 0xCCC7:
case 0xCEC7:

// MULU
case 0xC0C0:
{
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res *= src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(50)
case 0xC2D0:
case 0xC4D0:
case 0xC6D0:
case 0xC8D0:
case 0xCAD0:
case 0xCCD0:
case 0xCED0:
case 0xC0D1:
case 0xC2D1:
case 0xC4D1:
case 0xC6D1:
case 0xC8D1:
case 0xCAD1:
case 0xCCD1:
case 0xCED1:
case 0xC0D2:
case 0xC2D2:
case 0xC4D2:
case 0xC6D2:
case 0xC8D2:
case 0xCAD2:
case 0xCCD2:
case 0xCED2:
case 0xC0D3:
case 0xC2D3:
case 0xC4D3:
case 0xC6D3:
case 0xC8D3:
case 0xCAD3:
case 0xCCD3:
case 0xCED3:
case 0xC0D4:
case 0xC2D4:
case 0xC4D4:
case 0xC6D4:
case 0xC8D4:
case 0xCAD4:
case 0xCCD4:
case 0xCED4:
case 0xC0D5:
case 0xC2D5:
case 0xC4D5:
case 0xC6D5:
case 0xC8D5:
case 0xCAD5:
case 0xCCD5:
case 0xCED5:
case 0xC0D6:
case 0xC2D6:
case 0xC4D6:
case 0xC6D6:
case 0xC8D6:
case 0xCAD6:
case 0xCCD6:
case 0xCED6:
case 0xC0D7:
case 0xC2D7:
case 0xC4D7:
case 0xC6D7:
case 0xC8D7:
case 0xCAD7:
case 0xCCD7:
case 0xCED7:

// MULU
case 0xC0D0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res *= src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(54)
case 0xC2D8:
case 0xC4D8:
case 0xC6D8:
case 0xC8D8:
case 0xCAD8:
case 0xCCD8:
case 0xCED8:
case 0xC0D9:
case 0xC2D9:
case 0xC4D9:
case 0xC6D9:
case 0xC8D9:
case 0xCAD9:
case 0xCCD9:
case 0xCED9:
case 0xC0DA:
case 0xC2DA:
case 0xC4DA:
case 0xC6DA:
case 0xC8DA:
case 0xCADA:
case 0xCCDA:
case 0xCEDA:
case 0xC0DB:
case 0xC2DB:
case 0xC4DB:
case 0xC6DB:
case 0xC8DB:
case 0xCADB:
case 0xCCDB:
case 0xCEDB:
case 0xC0DC:
case 0xC2DC:
case 0xC4DC:
case 0xC6DC:
case 0xC8DC:
case 0xCADC:
case 0xCCDC:
case 0xCEDC:
case 0xC0DD:
case 0xC2DD:
case 0xC4DD:
case 0xC6DD:
case 0xC8DD:
case 0xCADD:
case 0xCCDD:
case 0xCEDD:
case 0xC0DE:
case 0xC2DE:
case 0xC4DE:
case 0xC6DE:
case 0xC8DE:
case 0xCADE:
case 0xCCDE:
case 0xCEDE:

// MULU
case 0xC0D8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res *= src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(54)
case 0xC2E0:
case 0xC4E0:
case 0xC6E0:
case 0xC8E0:
case 0xCAE0:
case 0xCCE0:
case 0xCEE0:
case 0xC0E1:
case 0xC2E1:
case 0xC4E1:
case 0xC6E1:
case 0xC8E1:
case 0xCAE1:
case 0xCCE1:
case 0xCEE1:
case 0xC0E2:
case 0xC2E2:
case 0xC4E2:
case 0xC6E2:
case 0xC8E2:
case 0xCAE2:
case 0xCCE2:
case 0xCEE2:
case 0xC0E3:
case 0xC2E3:
case 0xC4E3:
case 0xC6E3:
case 0xC8E3:
case 0xCAE3:
case 0xCCE3:
case 0xCEE3:
case 0xC0E4:
case 0xC2E4:
case 0xC4E4:
case 0xC6E4:
case 0xC8E4:
case 0xCAE4:
case 0xCCE4:
case 0xCEE4:
case 0xC0E5:
case 0xC2E5:
case 0xC4E5:
case 0xC6E5:
case 0xC8E5:
case 0xCAE5:
case 0xCCE5:
case 0xCEE5:
case 0xC0E6:
case 0xC2E6:
case 0xC4E6:
case 0xC6E6:
case 0xC8E6:
case 0xCAE6:
case 0xCCE6:
case 0xCEE6:

// MULU
case 0xC0E0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res *= src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(56)
case 0xC2E8:
case 0xC4E8:
case 0xC6E8:
case 0xC8E8:
case 0xCAE8:
case 0xCCE8:
case 0xCEE8:
case 0xC0E9:
case 0xC2E9:
case 0xC4E9:
case 0xC6E9:
case 0xC8E9:
case 0xCAE9:
case 0xCCE9:
case 0xCEE9:
case 0xC0EA:
case 0xC2EA:
case 0xC4EA:
case 0xC6EA:
case 0xC8EA:
case 0xCAEA:
case 0xCCEA:
case 0xCEEA:
case 0xC0EB:
case 0xC2EB:
case 0xC4EB:
case 0xC6EB:
case 0xC8EB:
case 0xCAEB:
case 0xCCEB:
case 0xCEEB:
case 0xC0EC:
case 0xC2EC:
case 0xC4EC:
case 0xC6EC:
case 0xC8EC:
case 0xCAEC:
case 0xCCEC:
case 0xCEEC:
case 0xC0ED:
case 0xC2ED:
case 0xC4ED:
case 0xC6ED:
case 0xC8ED:
case 0xCAED:
case 0xCCED:
case 0xCEED:
case 0xC0EE:
case 0xC2EE:
case 0xC4EE:
case 0xC6EE:
case 0xC8EE:
case 0xCAEE:
case 0xCCEE:
case 0xCEEE:
case 0xC0EF:
case 0xC2EF:
case 0xC4EF:
case 0xC6EF:
case 0xC8EF:
case 0xCAEF:
case 0xCCEF:
case 0xCEEF:

// MULU
case 0xC0E8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res *= src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(58)
case 0xC2F0:
case 0xC4F0:
case 0xC6F0:
case 0xC8F0:
case 0xCAF0:
case 0xCCF0:
case 0xCEF0:
case 0xC0F1:
case 0xC2F1:
case 0xC4F1:
case 0xC6F1:
case 0xC8F1:
case 0xCAF1:
case 0xCCF1:
case 0xCEF1:
case 0xC0F2:
case 0xC2F2:
case 0xC4F2:
case 0xC6F2:
case 0xC8F2:
case 0xCAF2:
case 0xCCF2:
case 0xCEF2:
case 0xC0F3:
case 0xC2F3:
case 0xC4F3:
case 0xC6F3:
case 0xC8F3:
case 0xCAF3:
case 0xCCF3:
case 0xCEF3:
case 0xC0F4:
case 0xC2F4:
case 0xC4F4:
case 0xC6F4:
case 0xC8F4:
case 0xCAF4:
case 0xCCF4:
case 0xCEF4:
case 0xC0F5:
case 0xC2F5:
case 0xC4F5:
case 0xC6F5:
case 0xC8F5:
case 0xCAF5:
case 0xCCF5:
case 0xCEF5:
case 0xC0F6:
case 0xC2F6:
case 0xC4F6:
case 0xC6F6:
case 0xC8F6:
case 0xCAF6:
case 0xCCF6:
case 0xCEF6:
case 0xC0F7:
case 0xC2F7:
case 0xC4F7:
case 0xC6F7:
case 0xC8F7:
case 0xCAF7:
case 0xCCF7:
case 0xCEF7:

// MULU
case 0xC0F0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res *= src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(60)
case 0xC2F8:
case 0xC4F8:
case 0xC6F8:
case 0xC8F8:
case 0xCAF8:
case 0xCCF8:
case 0xCEF8:

// MULU
case 0xC0F8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res *= src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(58)
case 0xC2F9:
case 0xC4F9:
case 0xC6F9:
case 0xC8F9:
case 0xCAF9:
case 0xCCF9:
case 0xCEF9:

// MULU
case 0xC0F9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res *= src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(62)
case 0xC2FA:
case 0xC4FA:
case 0xC6FA:
case 0xC8FA:
case 0xCAFA:
case 0xCCFA:
case 0xCEFA:

// MULU
case 0xC0FA:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res *= src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(58)
case 0xC2FB:
case 0xC4FB:
case 0xC6FB:
case 0xC8FB:
case 0xCAFB:
case 0xCCFB:
case 0xCEFB:

// MULU
case 0xC0FB:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res *= src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(60)
case 0xC2FC:
case 0xC4FC:
case 0xC6FC:
case 0xC8FC:
case 0xCAFC:
case 0xCCFC:
case 0xCEFC:

// MULU
case 0xC0FC:
{
	u32 res;
	u32 src;
	src = FETCH_WORD;
	PC += 2;
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res *= src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(54)
case 0xC2DF:
case 0xC4DF:
case 0xC6DF:
case 0xC8DF:
case 0xCADF:
case 0xCCDF:
case 0xCEDF:

// MULU
case 0xC0DF:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res *= src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(54)
case 0xC2E7:
case 0xC4E7:
case 0xC6E7:
case 0xC8E7:
case 0xCAE7:
case 0xCCE7:
case 0xCEE7:

// MULU
case 0xC0E7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res *= src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(56)
case 0xC3C0:
case 0xC5C0:
case 0xC7C0:
case 0xC9C0:
case 0xCBC0:
case 0xCDC0:
case 0xCFC0:
case 0xC1C1:
case 0xC3C1:
case 0xC5C1:
case 0xC7C1:
case 0xC9C1:
case 0xCBC1:
case 0xCDC1:
case 0xCFC1:
case 0xC1C2:
case 0xC3C2:
case 0xC5C2:
case 0xC7C2:
case 0xC9C2:
case 0xCBC2:
case 0xCDC2:
case 0xCFC2:
case 0xC1C3:
case 0xC3C3:
case 0xC5C3:
case 0xC7C3:
case 0xC9C3:
case 0xCBC3:
case 0xCDC3:
case 0xCFC3:
case 0xC1C4:
case 0xC3C4:
case 0xC5C4:
case 0xC7C4:
case 0xC9C4:
case 0xCBC4:
case 0xCDC4:
case 0xCFC4:
case 0xC1C5:
case 0xC3C5:
case 0xC5C5:
case 0xC7C5:
case 0xC9C5:
case 0xCBC5:
case 0xCDC5:
case 0xCFC5:
case 0xC1C6:
case 0xC3C6:
case 0xC5C6:
case 0xC7C6:
case 0xC9C6:
case 0xCBC6:
case 0xCDC6:
case 0xCFC6:
case 0xC1C7:
case 0xC3C7:
case 0xC5C7:
case 0xC7C7:
case 0xC9C7:
case 0xCBC7:
case 0xCDC7:
case 0xCFC7:

// MULS
case 0xC1C0:
{
	u32 res;
	u32 src;
	src = (s32)(s16)CPU->D[(Opcode >> 0) & 7];
	res = (s32)(s16)CPU->D[(Opcode >> 9) & 7];
	res *= (s32)src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(50)
case 0xC3D0:
case 0xC5D0:
case 0xC7D0:
case 0xC9D0:
case 0xCBD0:
case 0xCDD0:
case 0xCFD0:
case 0xC1D1:
case 0xC3D1:
case 0xC5D1:
case 0xC7D1:
case 0xC9D1:
case 0xCBD1:
case 0xCDD1:
case 0xCFD1:
case 0xC1D2:
case 0xC3D2:
case 0xC5D2:
case 0xC7D2:
case 0xC9D2:
case 0xCBD2:
case 0xCDD2:
case 0xCFD2:
case 0xC1D3:
case 0xC3D3:
case 0xC5D3:
case 0xC7D3:
case 0xC9D3:
case 0xCBD3:
case 0xCDD3:
case 0xCFD3:
case 0xC1D4:
case 0xC3D4:
case 0xC5D4:
case 0xC7D4:
case 0xC9D4:
case 0xCBD4:
case 0xCDD4:
case 0xCFD4:
case 0xC1D5:
case 0xC3D5:
case 0xC5D5:
case 0xC7D5:
case 0xC9D5:
case 0xCBD5:
case 0xCDD5:
case 0xCFD5:
case 0xC1D6:
case 0xC3D6:
case 0xC5D6:
case 0xC7D6:
case 0xC9D6:
case 0xCBD6:
case 0xCDD6:
case 0xCFD6:
case 0xC1D7:
case 0xC3D7:
case 0xC5D7:
case 0xC7D7:
case 0xC9D7:
case 0xCBD7:
case 0xCDD7:
case 0xCFD7:

// MULS
case 0xC1D0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READSX_WORD_F(adr, src)
	res = (s32)(s16)CPU->D[(Opcode >> 9) & 7];
	res *= (s32)src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(54)
case 0xC3D8:
case 0xC5D8:
case 0xC7D8:
case 0xC9D8:
case 0xCBD8:
case 0xCDD8:
case 0xCFD8:
case 0xC1D9:
case 0xC3D9:
case 0xC5D9:
case 0xC7D9:
case 0xC9D9:
case 0xCBD9:
case 0xCDD9:
case 0xCFD9:
case 0xC1DA:
case 0xC3DA:
case 0xC5DA:
case 0xC7DA:
case 0xC9DA:
case 0xCBDA:
case 0xCDDA:
case 0xCFDA:
case 0xC1DB:
case 0xC3DB:
case 0xC5DB:
case 0xC7DB:
case 0xC9DB:
case 0xCBDB:
case 0xCDDB:
case 0xCFDB:
case 0xC1DC:
case 0xC3DC:
case 0xC5DC:
case 0xC7DC:
case 0xC9DC:
case 0xCBDC:
case 0xCDDC:
case 0xCFDC:
case 0xC1DD:
case 0xC3DD:
case 0xC5DD:
case 0xC7DD:
case 0xC9DD:
case 0xCBDD:
case 0xCDDD:
case 0xCFDD:
case 0xC1DE:
case 0xC3DE:
case 0xC5DE:
case 0xC7DE:
case 0xC9DE:
case 0xCBDE:
case 0xCDDE:
case 0xCFDE:

// MULS
case 0xC1D8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	res = (s32)(s16)CPU->D[(Opcode >> 9) & 7];
	res *= (s32)src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(54)
case 0xC3E0:
case 0xC5E0:
case 0xC7E0:
case 0xC9E0:
case 0xCBE0:
case 0xCDE0:
case 0xCFE0:
case 0xC1E1:
case 0xC3E1:
case 0xC5E1:
case 0xC7E1:
case 0xC9E1:
case 0xCBE1:
case 0xCDE1:
case 0xCFE1:
case 0xC1E2:
case 0xC3E2:
case 0xC5E2:
case 0xC7E2:
case 0xC9E2:
case 0xCBE2:
case 0xCDE2:
case 0xCFE2:
case 0xC1E3:
case 0xC3E3:
case 0xC5E3:
case 0xC7E3:
case 0xC9E3:
case 0xCBE3:
case 0xCDE3:
case 0xCFE3:
case 0xC1E4:
case 0xC3E4:
case 0xC5E4:
case 0xC7E4:
case 0xC9E4:
case 0xCBE4:
case 0xCDE4:
case 0xCFE4:
case 0xC1E5:
case 0xC3E5:
case 0xC5E5:
case 0xC7E5:
case 0xC9E5:
case 0xCBE5:
case 0xCDE5:
case 0xCFE5:
case 0xC1E6:
case 0xC3E6:
case 0xC5E6:
case 0xC7E6:
case 0xC9E6:
case 0xCBE6:
case 0xCDE6:
case 0xCFE6:

// MULS
case 0xC1E0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READSX_WORD_F(adr, src)
	res = (s32)(s16)CPU->D[(Opcode >> 9) & 7];
	res *= (s32)src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(56)
case 0xC3E8:
case 0xC5E8:
case 0xC7E8:
case 0xC9E8:
case 0xCBE8:
case 0xCDE8:
case 0xCFE8:
case 0xC1E9:
case 0xC3E9:
case 0xC5E9:
case 0xC7E9:
case 0xC9E9:
case 0xCBE9:
case 0xCDE9:
case 0xCFE9:
case 0xC1EA:
case 0xC3EA:
case 0xC5EA:
case 0xC7EA:
case 0xC9EA:
case 0xCBEA:
case 0xCDEA:
case 0xCFEA:
case 0xC1EB:
case 0xC3EB:
case 0xC5EB:
case 0xC7EB:
case 0xC9EB:
case 0xCBEB:
case 0xCDEB:
case 0xCFEB:
case 0xC1EC:
case 0xC3EC:
case 0xC5EC:
case 0xC7EC:
case 0xC9EC:
case 0xCBEC:
case 0xCDEC:
case 0xCFEC:
case 0xC1ED:
case 0xC3ED:
case 0xC5ED:
case 0xC7ED:
case 0xC9ED:
case 0xCBED:
case 0xCDED:
case 0xCFED:
case 0xC1EE:
case 0xC3EE:
case 0xC5EE:
case 0xC7EE:
case 0xC9EE:
case 0xCBEE:
case 0xCDEE:
case 0xCFEE:
case 0xC1EF:
case 0xC3EF:
case 0xC5EF:
case 0xC7EF:
case 0xC9EF:
case 0xCBEF:
case 0xCDEF:
case 0xCFEF:

// MULS
case 0xC1E8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	res = (s32)(s16)CPU->D[(Opcode >> 9) & 7];
	res *= (s32)src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(58)
case 0xC3F0:
case 0xC5F0:
case 0xC7F0:
case 0xC9F0:
case 0xCBF0:
case 0xCDF0:
case 0xCFF0:
case 0xC1F1:
case 0xC3F1:
case 0xC5F1:
case 0xC7F1:
case 0xC9F1:
case 0xCBF1:
case 0xCDF1:
case 0xCFF1:
case 0xC1F2:
case 0xC3F2:
case 0xC5F2:
case 0xC7F2:
case 0xC9F2:
case 0xCBF2:
case 0xCDF2:
case 0xCFF2:
case 0xC1F3:
case 0xC3F3:
case 0xC5F3:
case 0xC7F3:
case 0xC9F3:
case 0xCBF3:
case 0xCDF3:
case 0xCFF3:
case 0xC1F4:
case 0xC3F4:
case 0xC5F4:
case 0xC7F4:
case 0xC9F4:
case 0xCBF4:
case 0xCDF4:
case 0xCFF4:
case 0xC1F5:
case 0xC3F5:
case 0xC5F5:
case 0xC7F5:
case 0xC9F5:
case 0xCBF5:
case 0xCDF5:
case 0xCFF5:
case 0xC1F6:
case 0xC3F6:
case 0xC5F6:
case 0xC7F6:
case 0xC9F6:
case 0xCBF6:
case 0xCDF6:
case 0xCFF6:
case 0xC1F7:
case 0xC3F7:
case 0xC5F7:
case 0xC7F7:
case 0xC9F7:
case 0xCBF7:
case 0xCDF7:
case 0xCFF7:

// MULS
case 0xC1F0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READSX_WORD_F(adr, src)
	res = (s32)(s16)CPU->D[(Opcode >> 9) & 7];
	res *= (s32)src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(60)
case 0xC3F8:
case 0xC5F8:
case 0xC7F8:
case 0xC9F8:
case 0xCBF8:
case 0xCDF8:
case 0xCFF8:

// MULS
case 0xC1F8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	res = (s32)(s16)CPU->D[(Opcode >> 9) & 7];
	res *= (s32)src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(58)
case 0xC3F9:
case 0xC5F9:
case 0xC7F9:
case 0xC9F9:
case 0xCBF9:
case 0xCDF9:
case 0xCFF9:

// MULS
case 0xC1F9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READSX_WORD_F(adr, src)
	res = (s32)(s16)CPU->D[(Opcode >> 9) & 7];
	res *= (s32)src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(62)
case 0xC3FA:
case 0xC5FA:
case 0xC7FA:
case 0xC9FA:
case 0xCBFA:
case 0xCDFA:
case 0xCFFA:

// MULS
case 0xC1FA:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	res = (s32)(s16)CPU->D[(Opcode >> 9) & 7];
	res *= (s32)src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(58)
case 0xC3FB:
case 0xC5FB:
case 0xC7FB:
case 0xC9FB:
case 0xCBFB:
case 0xCDFB:
case 0xCFFB:

// MULS
case 0xC1FB:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READSX_WORD_F(adr, src)
	res = (s32)(s16)CPU->D[(Opcode >> 9) & 7];
	res *= (s32)src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(60)
case 0xC3FC:
case 0xC5FC:
case 0xC7FC:
case 0xC9FC:
case 0xCBFC:
case 0xCDFC:
case 0xCFFC:

// MULS
case 0xC1FC:
{
	u32 res;
	u32 src;
	src = (s32)(s16)FETCH_WORD;
	PC += 2;
	res = (s32)(s16)CPU->D[(Opcode >> 9) & 7];
	res *= (s32)src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(54)
case 0xC3DF:
case 0xC5DF:
case 0xC7DF:
case 0xC9DF:
case 0xCBDF:
case 0xCDDF:
case 0xCFDF:

// MULS
case 0xC1DF:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	res = (s32)(s16)CPU->D[(Opcode >> 9) & 7];
	res *= (s32)src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(54)
case 0xC3E7:
case 0xC5E7:
case 0xC7E7:
case 0xC9E7:
case 0xCBE7:
case 0xCDE7:
case 0xCFE7:

// MULS
case 0xC1E7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READSX_WORD_F(adr, src)
	res = (s32)(s16)CPU->D[(Opcode >> 9) & 7];
	res *= (s32)src;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	CPU->flag_V = CPU->flag_C = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(56)
case 0xC340:
case 0xC540:
case 0xC740:
case 0xC940:
case 0xCB40:
case 0xCD40:
case 0xCF40:
case 0xC141:
case 0xC341:
case 0xC541:
case 0xC741:
case 0xC941:
case 0xCB41:
case 0xCD41:
case 0xCF41:
case 0xC142:
case 0xC342:
case 0xC542:
case 0xC742:
case 0xC942:
case 0xCB42:
case 0xCD42:
case 0xCF42:
case 0xC143:
case 0xC343:
case 0xC543:
case 0xC743:
case 0xC943:
case 0xCB43:
case 0xCD43:
case 0xCF43:
case 0xC144:
case 0xC344:
case 0xC544:
case 0xC744:
case 0xC944:
case 0xCB44:
case 0xCD44:
case 0xCF44:
case 0xC145:
case 0xC345:
case 0xC545:
case 0xC745:
case 0xC945:
case 0xCB45:
case 0xCD45:
case 0xCF45:
case 0xC146:
case 0xC346:
case 0xC546:
case 0xC746:
case 0xC946:
case 0xCB46:
case 0xCD46:
case 0xCF46:
case 0xC147:
case 0xC347:
case 0xC547:
case 0xC747:
case 0xC947:
case 0xCB47:
case 0xCD47:
case 0xCF47:

// EXGDD
case 0xC140:
{
	u32 res;
	u32 src;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	res = src;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xC348:
case 0xC548:
case 0xC748:
case 0xC948:
case 0xCB48:
case 0xCD48:
case 0xCF48:
case 0xC149:
case 0xC349:
case 0xC549:
case 0xC749:
case 0xC949:
case 0xCB49:
case 0xCD49:
case 0xCF49:
case 0xC14A:
case 0xC34A:
case 0xC54A:
case 0xC74A:
case 0xC94A:
case 0xCB4A:
case 0xCD4A:
case 0xCF4A:
case 0xC14B:
case 0xC34B:
case 0xC54B:
case 0xC74B:
case 0xC94B:
case 0xCB4B:
case 0xCD4B:
case 0xCF4B:
case 0xC14C:
case 0xC34C:
case 0xC54C:
case 0xC74C:
case 0xC94C:
case 0xCB4C:
case 0xCD4C:
case 0xCF4C:
case 0xC14D:
case 0xC34D:
case 0xC54D:
case 0xC74D:
case 0xC94D:
case 0xCB4D:
case 0xCD4D:
case 0xCF4D:
case 0xC14E:
case 0xC34E:
case 0xC54E:
case 0xC74E:
case 0xC94E:
case 0xCB4E:
case 0xCD4E:
case 0xCF4E:
case 0xC14F:
case 0xC34F:
case 0xC54F:
case 0xC74F:
case 0xC94F:
case 0xCB4F:
case 0xCD4F:
case 0xCF4F:

// EXGAA
case 0xC148:
{
	u32 res;
	u32 src;
	res = (u32)CPU->A[(Opcode >> 0) & 7];
	src = (u32)CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] = res;
	res = src;
	CPU->A[(Opcode >> 0) & 7] = res;
}
RET(6)
case 0xC388:
case 0xC588:
case 0xC788:
case 0xC988:
case 0xCB88:
case 0xCD88:
case 0xCF88:
case 0xC189:
case 0xC389:
case 0xC589:
case 0xC789:
case 0xC989:
case 0xCB89:
case 0xCD89:
case 0xCF89:
case 0xC18A:
case 0xC38A:
case 0xC58A:
case 0xC78A:
case 0xC98A:
case 0xCB8A:
case 0xCD8A:
case 0xCF8A:
case 0xC18B:
case 0xC38B:
case 0xC58B:
case 0xC78B:
case 0xC98B:
case 0xCB8B:
case 0xCD8B:
case 0xCF8B:
case 0xC18C:
case 0xC38C:
case 0xC58C:
case 0xC78C:
case 0xC98C:
case 0xCB8C:
case 0xCD8C:
case 0xCF8C:
case 0xC18D:
case 0xC38D:
case 0xC58D:
case 0xC78D:
case 0xC98D:
case 0xCB8D:
case 0xCD8D:
case 0xCF8D:
case 0xC18E:
case 0xC38E:
case 0xC58E:
case 0xC78E:
case 0xC98E:
case 0xCB8E:
case 0xCD8E:
case 0xCF8E:
case 0xC18F:
case 0xC38F:
case 0xC58F:
case 0xC78F:
case 0xC98F:
case 0xCB8F:
case 0xCD8F:
case 0xCF8F:

// EXGAD
case 0xC188:
{
	u32 res;
	u32 src;
	res = (u32)CPU->A[(Opcode >> 0) & 7];
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	res = src;
	CPU->A[(Opcode >> 0) & 7] = res;
}
RET(6)
