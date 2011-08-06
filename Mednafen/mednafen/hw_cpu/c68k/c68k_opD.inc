case 0xD200:
case 0xD400:
case 0xD600:
case 0xD800:
case 0xDA00:
case 0xDC00:
case 0xDE00:
case 0xD001:
case 0xD201:
case 0xD401:
case 0xD601:
case 0xD801:
case 0xDA01:
case 0xDC01:
case 0xDE01:
case 0xD002:
case 0xD202:
case 0xD402:
case 0xD602:
case 0xD802:
case 0xDA02:
case 0xDC02:
case 0xDE02:
case 0xD003:
case 0xD203:
case 0xD403:
case 0xD603:
case 0xD803:
case 0xDA03:
case 0xDC03:
case 0xDE03:
case 0xD004:
case 0xD204:
case 0xD404:
case 0xD604:
case 0xD804:
case 0xDA04:
case 0xDC04:
case 0xDE04:
case 0xD005:
case 0xD205:
case 0xD405:
case 0xD605:
case 0xD805:
case 0xDA05:
case 0xDC05:
case 0xDE05:
case 0xD006:
case 0xD206:
case 0xD406:
case 0xD606:
case 0xD806:
case 0xDA06:
case 0xDC06:
case 0xDE06:
case 0xD007:
case 0xD207:
case 0xD407:
case 0xD607:
case 0xD807:
case 0xDA07:
case 0xDC07:
case 0xDE07:

// ADDaD
case 0xD000:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0xD208:
case 0xD408:
case 0xD608:
case 0xD808:
case 0xDA08:
case 0xDC08:
case 0xDE08:
case 0xD009:
case 0xD209:
case 0xD409:
case 0xD609:
case 0xD809:
case 0xDA09:
case 0xDC09:
case 0xDE09:
case 0xD00A:
case 0xD20A:
case 0xD40A:
case 0xD60A:
case 0xD80A:
case 0xDA0A:
case 0xDC0A:
case 0xDE0A:
case 0xD00B:
case 0xD20B:
case 0xD40B:
case 0xD60B:
case 0xD80B:
case 0xDA0B:
case 0xDC0B:
case 0xDE0B:
case 0xD00C:
case 0xD20C:
case 0xD40C:
case 0xD60C:
case 0xD80C:
case 0xDA0C:
case 0xDC0C:
case 0xDE0C:
case 0xD00D:
case 0xD20D:
case 0xD40D:
case 0xD60D:
case 0xD80D:
case 0xDA0D:
case 0xDC0D:
case 0xDE0D:
case 0xD00E:
case 0xD20E:
case 0xD40E:
case 0xD60E:
case 0xD80E:
case 0xDA0E:
case 0xDC0E:
case 0xDE0E:
case 0xD00F:
case 0xD20F:
case 0xD40F:
case 0xD60F:
case 0xD80F:
case 0xDA0F:
case 0xDC0F:
case 0xDE0F:

// ADDaD
case 0xD008:
{
	u32 res;
	u32 dst;
	u32 src;
	// can't read byte from Ax registers !
	CPU->Status |= C68K_FAULTED;
	CCnt = 0;
	goto C68k_Exec_Really_End;
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0xD210:
case 0xD410:
case 0xD610:
case 0xD810:
case 0xDA10:
case 0xDC10:
case 0xDE10:
case 0xD011:
case 0xD211:
case 0xD411:
case 0xD611:
case 0xD811:
case 0xDA11:
case 0xDC11:
case 0xDE11:
case 0xD012:
case 0xD212:
case 0xD412:
case 0xD612:
case 0xD812:
case 0xDA12:
case 0xDC12:
case 0xDE12:
case 0xD013:
case 0xD213:
case 0xD413:
case 0xD613:
case 0xD813:
case 0xDA13:
case 0xDC13:
case 0xDE13:
case 0xD014:
case 0xD214:
case 0xD414:
case 0xD614:
case 0xD814:
case 0xDA14:
case 0xDC14:
case 0xDE14:
case 0xD015:
case 0xD215:
case 0xD415:
case 0xD615:
case 0xD815:
case 0xDA15:
case 0xDC15:
case 0xDE15:
case 0xD016:
case 0xD216:
case 0xD416:
case 0xD616:
case 0xD816:
case 0xDA16:
case 0xDC16:
case 0xDE16:
case 0xD017:
case 0xD217:
case 0xD417:
case 0xD617:
case 0xD817:
case 0xDA17:
case 0xDC17:
case 0xDE17:

// ADDaD
case 0xD010:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, src)
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0xD218:
case 0xD418:
case 0xD618:
case 0xD818:
case 0xDA18:
case 0xDC18:
case 0xDE18:
case 0xD019:
case 0xD219:
case 0xD419:
case 0xD619:
case 0xD819:
case 0xDA19:
case 0xDC19:
case 0xDE19:
case 0xD01A:
case 0xD21A:
case 0xD41A:
case 0xD61A:
case 0xD81A:
case 0xDA1A:
case 0xDC1A:
case 0xDE1A:
case 0xD01B:
case 0xD21B:
case 0xD41B:
case 0xD61B:
case 0xD81B:
case 0xDA1B:
case 0xDC1B:
case 0xDE1B:
case 0xD01C:
case 0xD21C:
case 0xD41C:
case 0xD61C:
case 0xD81C:
case 0xDA1C:
case 0xDC1C:
case 0xDE1C:
case 0xD01D:
case 0xD21D:
case 0xD41D:
case 0xD61D:
case 0xD81D:
case 0xDA1D:
case 0xDC1D:
case 0xDE1D:
case 0xD01E:
case 0xD21E:
case 0xD41E:
case 0xD61E:
case 0xD81E:
case 0xDA1E:
case 0xDC1E:
case 0xDE1E:

// ADDaD
case 0xD018:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0xD220:
case 0xD420:
case 0xD620:
case 0xD820:
case 0xDA20:
case 0xDC20:
case 0xDE20:
case 0xD021:
case 0xD221:
case 0xD421:
case 0xD621:
case 0xD821:
case 0xDA21:
case 0xDC21:
case 0xDE21:
case 0xD022:
case 0xD222:
case 0xD422:
case 0xD622:
case 0xD822:
case 0xDA22:
case 0xDC22:
case 0xDE22:
case 0xD023:
case 0xD223:
case 0xD423:
case 0xD623:
case 0xD823:
case 0xDA23:
case 0xDC23:
case 0xDE23:
case 0xD024:
case 0xD224:
case 0xD424:
case 0xD624:
case 0xD824:
case 0xDA24:
case 0xDC24:
case 0xDE24:
case 0xD025:
case 0xD225:
case 0xD425:
case 0xD625:
case 0xD825:
case 0xDA25:
case 0xDC25:
case 0xDE25:
case 0xD026:
case 0xD226:
case 0xD426:
case 0xD626:
case 0xD826:
case 0xDA26:
case 0xDC26:
case 0xDE26:

// ADDaD
case 0xD020:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0xD228:
case 0xD428:
case 0xD628:
case 0xD828:
case 0xDA28:
case 0xDC28:
case 0xDE28:
case 0xD029:
case 0xD229:
case 0xD429:
case 0xD629:
case 0xD829:
case 0xDA29:
case 0xDC29:
case 0xDE29:
case 0xD02A:
case 0xD22A:
case 0xD42A:
case 0xD62A:
case 0xD82A:
case 0xDA2A:
case 0xDC2A:
case 0xDE2A:
case 0xD02B:
case 0xD22B:
case 0xD42B:
case 0xD62B:
case 0xD82B:
case 0xDA2B:
case 0xDC2B:
case 0xDE2B:
case 0xD02C:
case 0xD22C:
case 0xD42C:
case 0xD62C:
case 0xD82C:
case 0xDA2C:
case 0xDC2C:
case 0xDE2C:
case 0xD02D:
case 0xD22D:
case 0xD42D:
case 0xD62D:
case 0xD82D:
case 0xDA2D:
case 0xDC2D:
case 0xDE2D:
case 0xD02E:
case 0xD22E:
case 0xD42E:
case 0xD62E:
case 0xD82E:
case 0xDA2E:
case 0xDC2E:
case 0xDE2E:
case 0xD02F:
case 0xD22F:
case 0xD42F:
case 0xD62F:
case 0xD82F:
case 0xDA2F:
case 0xDC2F:
case 0xDE2F:

// ADDaD
case 0xD028:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0xD230:
case 0xD430:
case 0xD630:
case 0xD830:
case 0xDA30:
case 0xDC30:
case 0xDE30:
case 0xD031:
case 0xD231:
case 0xD431:
case 0xD631:
case 0xD831:
case 0xDA31:
case 0xDC31:
case 0xDE31:
case 0xD032:
case 0xD232:
case 0xD432:
case 0xD632:
case 0xD832:
case 0xDA32:
case 0xDC32:
case 0xDE32:
case 0xD033:
case 0xD233:
case 0xD433:
case 0xD633:
case 0xD833:
case 0xDA33:
case 0xDC33:
case 0xDE33:
case 0xD034:
case 0xD234:
case 0xD434:
case 0xD634:
case 0xD834:
case 0xDA34:
case 0xDC34:
case 0xDE34:
case 0xD035:
case 0xD235:
case 0xD435:
case 0xD635:
case 0xD835:
case 0xDA35:
case 0xDC35:
case 0xDE35:
case 0xD036:
case 0xD236:
case 0xD436:
case 0xD636:
case 0xD836:
case 0xDA36:
case 0xDC36:
case 0xDE36:
case 0xD037:
case 0xD237:
case 0xD437:
case 0xD637:
case 0xD837:
case 0xDA37:
case 0xDC37:
case 0xDE37:

// ADDaD
case 0xD030:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0xD238:
case 0xD438:
case 0xD638:
case 0xD838:
case 0xDA38:
case 0xDC38:
case 0xDE38:

// ADDaD
case 0xD038:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0xD239:
case 0xD439:
case 0xD639:
case 0xD839:
case 0xDA39:
case 0xDC39:
case 0xDE39:

// ADDaD
case 0xD039:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0xD23A:
case 0xD43A:
case 0xD63A:
case 0xD83A:
case 0xDA3A:
case 0xDC3A:
case 0xDE3A:

// ADDaD
case 0xD03A:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0xD23B:
case 0xD43B:
case 0xD63B:
case 0xD83B:
case 0xDA3B:
case 0xDC3B:
case 0xDE3B:

// ADDaD
case 0xD03B:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0xD23C:
case 0xD43C:
case 0xD63C:
case 0xD83C:
case 0xDA3C:
case 0xDC3C:
case 0xDE3C:

// ADDaD
case 0xD03C:
{
	u32 res;
	u32 dst;
	u32 src;
	src = FETCH_BYTE;
	PC += 2;
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0xD21F:
case 0xD41F:
case 0xD61F:
case 0xD81F:
case 0xDA1F:
case 0xDC1F:
case 0xDE1F:

// ADDaD
case 0xD01F:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0xD227:
case 0xD427:
case 0xD627:
case 0xD827:
case 0xDA27:
case 0xDC27:
case 0xDE27:

// ADDaD
case 0xD027:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0xD240:
case 0xD440:
case 0xD640:
case 0xD840:
case 0xDA40:
case 0xDC40:
case 0xDE40:
case 0xD041:
case 0xD241:
case 0xD441:
case 0xD641:
case 0xD841:
case 0xDA41:
case 0xDC41:
case 0xDE41:
case 0xD042:
case 0xD242:
case 0xD442:
case 0xD642:
case 0xD842:
case 0xDA42:
case 0xDC42:
case 0xDE42:
case 0xD043:
case 0xD243:
case 0xD443:
case 0xD643:
case 0xD843:
case 0xDA43:
case 0xDC43:
case 0xDE43:
case 0xD044:
case 0xD244:
case 0xD444:
case 0xD644:
case 0xD844:
case 0xDA44:
case 0xDC44:
case 0xDE44:
case 0xD045:
case 0xD245:
case 0xD445:
case 0xD645:
case 0xD845:
case 0xDA45:
case 0xDC45:
case 0xDE45:
case 0xD046:
case 0xD246:
case 0xD446:
case 0xD646:
case 0xD846:
case 0xDA46:
case 0xDC46:
case 0xDE46:
case 0xD047:
case 0xD247:
case 0xD447:
case 0xD647:
case 0xD847:
case 0xDA47:
case 0xDC47:
case 0xDE47:

// ADDaD
case 0xD040:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0xD248:
case 0xD448:
case 0xD648:
case 0xD848:
case 0xDA48:
case 0xDC48:
case 0xDE48:
case 0xD049:
case 0xD249:
case 0xD449:
case 0xD649:
case 0xD849:
case 0xDA49:
case 0xDC49:
case 0xDE49:
case 0xD04A:
case 0xD24A:
case 0xD44A:
case 0xD64A:
case 0xD84A:
case 0xDA4A:
case 0xDC4A:
case 0xDE4A:
case 0xD04B:
case 0xD24B:
case 0xD44B:
case 0xD64B:
case 0xD84B:
case 0xDA4B:
case 0xDC4B:
case 0xDE4B:
case 0xD04C:
case 0xD24C:
case 0xD44C:
case 0xD64C:
case 0xD84C:
case 0xDA4C:
case 0xDC4C:
case 0xDE4C:
case 0xD04D:
case 0xD24D:
case 0xD44D:
case 0xD64D:
case 0xD84D:
case 0xDA4D:
case 0xDC4D:
case 0xDE4D:
case 0xD04E:
case 0xD24E:
case 0xD44E:
case 0xD64E:
case 0xD84E:
case 0xDA4E:
case 0xDC4E:
case 0xDE4E:
case 0xD04F:
case 0xD24F:
case 0xD44F:
case 0xD64F:
case 0xD84F:
case 0xDA4F:
case 0xDC4F:
case 0xDE4F:

// ADDaD
case 0xD048:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->A[(Opcode >> 0) & 7];
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0xD250:
case 0xD450:
case 0xD650:
case 0xD850:
case 0xDA50:
case 0xDC50:
case 0xDE50:
case 0xD051:
case 0xD251:
case 0xD451:
case 0xD651:
case 0xD851:
case 0xDA51:
case 0xDC51:
case 0xDE51:
case 0xD052:
case 0xD252:
case 0xD452:
case 0xD652:
case 0xD852:
case 0xDA52:
case 0xDC52:
case 0xDE52:
case 0xD053:
case 0xD253:
case 0xD453:
case 0xD653:
case 0xD853:
case 0xDA53:
case 0xDC53:
case 0xDE53:
case 0xD054:
case 0xD254:
case 0xD454:
case 0xD654:
case 0xD854:
case 0xDA54:
case 0xDC54:
case 0xDE54:
case 0xD055:
case 0xD255:
case 0xD455:
case 0xD655:
case 0xD855:
case 0xDA55:
case 0xDC55:
case 0xDE55:
case 0xD056:
case 0xD256:
case 0xD456:
case 0xD656:
case 0xD856:
case 0xDA56:
case 0xDC56:
case 0xDE56:
case 0xD057:
case 0xD257:
case 0xD457:
case 0xD657:
case 0xD857:
case 0xDA57:
case 0xDC57:
case 0xDE57:

// ADDaD
case 0xD050:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0xD258:
case 0xD458:
case 0xD658:
case 0xD858:
case 0xDA58:
case 0xDC58:
case 0xDE58:
case 0xD059:
case 0xD259:
case 0xD459:
case 0xD659:
case 0xD859:
case 0xDA59:
case 0xDC59:
case 0xDE59:
case 0xD05A:
case 0xD25A:
case 0xD45A:
case 0xD65A:
case 0xD85A:
case 0xDA5A:
case 0xDC5A:
case 0xDE5A:
case 0xD05B:
case 0xD25B:
case 0xD45B:
case 0xD65B:
case 0xD85B:
case 0xDA5B:
case 0xDC5B:
case 0xDE5B:
case 0xD05C:
case 0xD25C:
case 0xD45C:
case 0xD65C:
case 0xD85C:
case 0xDA5C:
case 0xDC5C:
case 0xDE5C:
case 0xD05D:
case 0xD25D:
case 0xD45D:
case 0xD65D:
case 0xD85D:
case 0xDA5D:
case 0xDC5D:
case 0xDE5D:
case 0xD05E:
case 0xD25E:
case 0xD45E:
case 0xD65E:
case 0xD85E:
case 0xDA5E:
case 0xDC5E:
case 0xDE5E:

// ADDaD
case 0xD058:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0xD260:
case 0xD460:
case 0xD660:
case 0xD860:
case 0xDA60:
case 0xDC60:
case 0xDE60:
case 0xD061:
case 0xD261:
case 0xD461:
case 0xD661:
case 0xD861:
case 0xDA61:
case 0xDC61:
case 0xDE61:
case 0xD062:
case 0xD262:
case 0xD462:
case 0xD662:
case 0xD862:
case 0xDA62:
case 0xDC62:
case 0xDE62:
case 0xD063:
case 0xD263:
case 0xD463:
case 0xD663:
case 0xD863:
case 0xDA63:
case 0xDC63:
case 0xDE63:
case 0xD064:
case 0xD264:
case 0xD464:
case 0xD664:
case 0xD864:
case 0xDA64:
case 0xDC64:
case 0xDE64:
case 0xD065:
case 0xD265:
case 0xD465:
case 0xD665:
case 0xD865:
case 0xDA65:
case 0xDC65:
case 0xDE65:
case 0xD066:
case 0xD266:
case 0xD466:
case 0xD666:
case 0xD866:
case 0xDA66:
case 0xDC66:
case 0xDE66:

// ADDaD
case 0xD060:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0xD268:
case 0xD468:
case 0xD668:
case 0xD868:
case 0xDA68:
case 0xDC68:
case 0xDE68:
case 0xD069:
case 0xD269:
case 0xD469:
case 0xD669:
case 0xD869:
case 0xDA69:
case 0xDC69:
case 0xDE69:
case 0xD06A:
case 0xD26A:
case 0xD46A:
case 0xD66A:
case 0xD86A:
case 0xDA6A:
case 0xDC6A:
case 0xDE6A:
case 0xD06B:
case 0xD26B:
case 0xD46B:
case 0xD66B:
case 0xD86B:
case 0xDA6B:
case 0xDC6B:
case 0xDE6B:
case 0xD06C:
case 0xD26C:
case 0xD46C:
case 0xD66C:
case 0xD86C:
case 0xDA6C:
case 0xDC6C:
case 0xDE6C:
case 0xD06D:
case 0xD26D:
case 0xD46D:
case 0xD66D:
case 0xD86D:
case 0xDA6D:
case 0xDC6D:
case 0xDE6D:
case 0xD06E:
case 0xD26E:
case 0xD46E:
case 0xD66E:
case 0xD86E:
case 0xDA6E:
case 0xDC6E:
case 0xDE6E:
case 0xD06F:
case 0xD26F:
case 0xD46F:
case 0xD66F:
case 0xD86F:
case 0xDA6F:
case 0xDC6F:
case 0xDE6F:

// ADDaD
case 0xD068:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0xD270:
case 0xD470:
case 0xD670:
case 0xD870:
case 0xDA70:
case 0xDC70:
case 0xDE70:
case 0xD071:
case 0xD271:
case 0xD471:
case 0xD671:
case 0xD871:
case 0xDA71:
case 0xDC71:
case 0xDE71:
case 0xD072:
case 0xD272:
case 0xD472:
case 0xD672:
case 0xD872:
case 0xDA72:
case 0xDC72:
case 0xDE72:
case 0xD073:
case 0xD273:
case 0xD473:
case 0xD673:
case 0xD873:
case 0xDA73:
case 0xDC73:
case 0xDE73:
case 0xD074:
case 0xD274:
case 0xD474:
case 0xD674:
case 0xD874:
case 0xDA74:
case 0xDC74:
case 0xDE74:
case 0xD075:
case 0xD275:
case 0xD475:
case 0xD675:
case 0xD875:
case 0xDA75:
case 0xDC75:
case 0xDE75:
case 0xD076:
case 0xD276:
case 0xD476:
case 0xD676:
case 0xD876:
case 0xDA76:
case 0xDC76:
case 0xDE76:
case 0xD077:
case 0xD277:
case 0xD477:
case 0xD677:
case 0xD877:
case 0xDA77:
case 0xDC77:
case 0xDE77:

// ADDaD
case 0xD070:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0xD278:
case 0xD478:
case 0xD678:
case 0xD878:
case 0xDA78:
case 0xDC78:
case 0xDE78:

// ADDaD
case 0xD078:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0xD279:
case 0xD479:
case 0xD679:
case 0xD879:
case 0xDA79:
case 0xDC79:
case 0xDE79:

// ADDaD
case 0xD079:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0xD27A:
case 0xD47A:
case 0xD67A:
case 0xD87A:
case 0xDA7A:
case 0xDC7A:
case 0xDE7A:

// ADDaD
case 0xD07A:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0xD27B:
case 0xD47B:
case 0xD67B:
case 0xD87B:
case 0xDA7B:
case 0xDC7B:
case 0xDE7B:

// ADDaD
case 0xD07B:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0xD27C:
case 0xD47C:
case 0xD67C:
case 0xD87C:
case 0xDA7C:
case 0xDC7C:
case 0xDE7C:

// ADDaD
case 0xD07C:
{
	u32 res;
	u32 dst;
	u32 src;
	src = FETCH_WORD;
	PC += 2;
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0xD25F:
case 0xD45F:
case 0xD65F:
case 0xD85F:
case 0xDA5F:
case 0xDC5F:
case 0xDE5F:

// ADDaD
case 0xD05F:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0xD267:
case 0xD467:
case 0xD667:
case 0xD867:
case 0xDA67:
case 0xDC67:
case 0xDE67:

// ADDaD
case 0xD067:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0xD280:
case 0xD480:
case 0xD680:
case 0xD880:
case 0xDA80:
case 0xDC80:
case 0xDE80:
case 0xD081:
case 0xD281:
case 0xD481:
case 0xD681:
case 0xD881:
case 0xDA81:
case 0xDC81:
case 0xDE81:
case 0xD082:
case 0xD282:
case 0xD482:
case 0xD682:
case 0xD882:
case 0xDA82:
case 0xDC82:
case 0xDE82:
case 0xD083:
case 0xD283:
case 0xD483:
case 0xD683:
case 0xD883:
case 0xDA83:
case 0xDC83:
case 0xDE83:
case 0xD084:
case 0xD284:
case 0xD484:
case 0xD684:
case 0xD884:
case 0xDA84:
case 0xDC84:
case 0xDE84:
case 0xD085:
case 0xD285:
case 0xD485:
case 0xD685:
case 0xD885:
case 0xDA85:
case 0xDC85:
case 0xDE85:
case 0xD086:
case 0xD286:
case 0xD486:
case 0xD686:
case 0xD886:
case 0xDA86:
case 0xDC86:
case 0xDE86:
case 0xD087:
case 0xD287:
case 0xD487:
case 0xD687:
case 0xD887:
case 0xDA87:
case 0xDC87:
case 0xDE87:

// ADDaD
case 0xD080:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0xD288:
case 0xD488:
case 0xD688:
case 0xD888:
case 0xDA88:
case 0xDC88:
case 0xDE88:
case 0xD089:
case 0xD289:
case 0xD489:
case 0xD689:
case 0xD889:
case 0xDA89:
case 0xDC89:
case 0xDE89:
case 0xD08A:
case 0xD28A:
case 0xD48A:
case 0xD68A:
case 0xD88A:
case 0xDA8A:
case 0xDC8A:
case 0xDE8A:
case 0xD08B:
case 0xD28B:
case 0xD48B:
case 0xD68B:
case 0xD88B:
case 0xDA8B:
case 0xDC8B:
case 0xDE8B:
case 0xD08C:
case 0xD28C:
case 0xD48C:
case 0xD68C:
case 0xD88C:
case 0xDA8C:
case 0xDC8C:
case 0xDE8C:
case 0xD08D:
case 0xD28D:
case 0xD48D:
case 0xD68D:
case 0xD88D:
case 0xDA8D:
case 0xDC8D:
case 0xDE8D:
case 0xD08E:
case 0xD28E:
case 0xD48E:
case 0xD68E:
case 0xD88E:
case 0xDA8E:
case 0xDC8E:
case 0xDE8E:
case 0xD08F:
case 0xD28F:
case 0xD48F:
case 0xD68F:
case 0xD88F:
case 0xDA8F:
case 0xDC8F:
case 0xDE8F:

// ADDaD
case 0xD088:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->A[(Opcode >> 0) & 7];
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0xD290:
case 0xD490:
case 0xD690:
case 0xD890:
case 0xDA90:
case 0xDC90:
case 0xDE90:
case 0xD091:
case 0xD291:
case 0xD491:
case 0xD691:
case 0xD891:
case 0xDA91:
case 0xDC91:
case 0xDE91:
case 0xD092:
case 0xD292:
case 0xD492:
case 0xD692:
case 0xD892:
case 0xDA92:
case 0xDC92:
case 0xDE92:
case 0xD093:
case 0xD293:
case 0xD493:
case 0xD693:
case 0xD893:
case 0xDA93:
case 0xDC93:
case 0xDE93:
case 0xD094:
case 0xD294:
case 0xD494:
case 0xD694:
case 0xD894:
case 0xDA94:
case 0xDC94:
case 0xDE94:
case 0xD095:
case 0xD295:
case 0xD495:
case 0xD695:
case 0xD895:
case 0xDA95:
case 0xDC95:
case 0xDE95:
case 0xD096:
case 0xD296:
case 0xD496:
case 0xD696:
case 0xD896:
case 0xDA96:
case 0xDC96:
case 0xDE96:
case 0xD097:
case 0xD297:
case 0xD497:
case 0xD697:
case 0xD897:
case 0xDA97:
case 0xDC97:
case 0xDE97:

// ADDaD
case 0xD090:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_LONG_F(adr, src)
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0xD298:
case 0xD498:
case 0xD698:
case 0xD898:
case 0xDA98:
case 0xDC98:
case 0xDE98:
case 0xD099:
case 0xD299:
case 0xD499:
case 0xD699:
case 0xD899:
case 0xDA99:
case 0xDC99:
case 0xDE99:
case 0xD09A:
case 0xD29A:
case 0xD49A:
case 0xD69A:
case 0xD89A:
case 0xDA9A:
case 0xDC9A:
case 0xDE9A:
case 0xD09B:
case 0xD29B:
case 0xD49B:
case 0xD69B:
case 0xD89B:
case 0xDA9B:
case 0xDC9B:
case 0xDE9B:
case 0xD09C:
case 0xD29C:
case 0xD49C:
case 0xD69C:
case 0xD89C:
case 0xDA9C:
case 0xDC9C:
case 0xDE9C:
case 0xD09D:
case 0xD29D:
case 0xD49D:
case 0xD69D:
case 0xD89D:
case 0xDA9D:
case 0xDC9D:
case 0xDE9D:
case 0xD09E:
case 0xD29E:
case 0xD49E:
case 0xD69E:
case 0xD89E:
case 0xDA9E:
case 0xDC9E:
case 0xDE9E:

// ADDaD
case 0xD098:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0xD2A0:
case 0xD4A0:
case 0xD6A0:
case 0xD8A0:
case 0xDAA0:
case 0xDCA0:
case 0xDEA0:
case 0xD0A1:
case 0xD2A1:
case 0xD4A1:
case 0xD6A1:
case 0xD8A1:
case 0xDAA1:
case 0xDCA1:
case 0xDEA1:
case 0xD0A2:
case 0xD2A2:
case 0xD4A2:
case 0xD6A2:
case 0xD8A2:
case 0xDAA2:
case 0xDCA2:
case 0xDEA2:
case 0xD0A3:
case 0xD2A3:
case 0xD4A3:
case 0xD6A3:
case 0xD8A3:
case 0xDAA3:
case 0xDCA3:
case 0xDEA3:
case 0xD0A4:
case 0xD2A4:
case 0xD4A4:
case 0xD6A4:
case 0xD8A4:
case 0xDAA4:
case 0xDCA4:
case 0xDEA4:
case 0xD0A5:
case 0xD2A5:
case 0xD4A5:
case 0xD6A5:
case 0xD8A5:
case 0xDAA5:
case 0xDCA5:
case 0xDEA5:
case 0xD0A6:
case 0xD2A6:
case 0xD4A6:
case 0xD6A6:
case 0xD8A6:
case 0xDAA6:
case 0xDCA6:
case 0xDEA6:

// ADDaD
case 0xD0A0:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0xD2A8:
case 0xD4A8:
case 0xD6A8:
case 0xD8A8:
case 0xDAA8:
case 0xDCA8:
case 0xDEA8:
case 0xD0A9:
case 0xD2A9:
case 0xD4A9:
case 0xD6A9:
case 0xD8A9:
case 0xDAA9:
case 0xDCA9:
case 0xDEA9:
case 0xD0AA:
case 0xD2AA:
case 0xD4AA:
case 0xD6AA:
case 0xD8AA:
case 0xDAAA:
case 0xDCAA:
case 0xDEAA:
case 0xD0AB:
case 0xD2AB:
case 0xD4AB:
case 0xD6AB:
case 0xD8AB:
case 0xDAAB:
case 0xDCAB:
case 0xDEAB:
case 0xD0AC:
case 0xD2AC:
case 0xD4AC:
case 0xD6AC:
case 0xD8AC:
case 0xDAAC:
case 0xDCAC:
case 0xDEAC:
case 0xD0AD:
case 0xD2AD:
case 0xD4AD:
case 0xD6AD:
case 0xD8AD:
case 0xDAAD:
case 0xDCAD:
case 0xDEAD:
case 0xD0AE:
case 0xD2AE:
case 0xD4AE:
case 0xD6AE:
case 0xD8AE:
case 0xDAAE:
case 0xDCAE:
case 0xDEAE:
case 0xD0AF:
case 0xD2AF:
case 0xD4AF:
case 0xD6AF:
case 0xD8AF:
case 0xDAAF:
case 0xDCAF:
case 0xDEAF:

// ADDaD
case 0xD0A8:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(18)
case 0xD2B0:
case 0xD4B0:
case 0xD6B0:
case 0xD8B0:
case 0xDAB0:
case 0xDCB0:
case 0xDEB0:
case 0xD0B1:
case 0xD2B1:
case 0xD4B1:
case 0xD6B1:
case 0xD8B1:
case 0xDAB1:
case 0xDCB1:
case 0xDEB1:
case 0xD0B2:
case 0xD2B2:
case 0xD4B2:
case 0xD6B2:
case 0xD8B2:
case 0xDAB2:
case 0xDCB2:
case 0xDEB2:
case 0xD0B3:
case 0xD2B3:
case 0xD4B3:
case 0xD6B3:
case 0xD8B3:
case 0xDAB3:
case 0xDCB3:
case 0xDEB3:
case 0xD0B4:
case 0xD2B4:
case 0xD4B4:
case 0xD6B4:
case 0xD8B4:
case 0xDAB4:
case 0xDCB4:
case 0xDEB4:
case 0xD0B5:
case 0xD2B5:
case 0xD4B5:
case 0xD6B5:
case 0xD8B5:
case 0xDAB5:
case 0xDCB5:
case 0xDEB5:
case 0xD0B6:
case 0xD2B6:
case 0xD4B6:
case 0xD6B6:
case 0xD8B6:
case 0xDAB6:
case 0xDCB6:
case 0xDEB6:
case 0xD0B7:
case 0xD2B7:
case 0xD4B7:
case 0xD6B7:
case 0xD8B7:
case 0xDAB7:
case 0xDCB7:
case 0xDEB7:

// ADDaD
case 0xD0B0:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(20)
case 0xD2B8:
case 0xD4B8:
case 0xD6B8:
case 0xD8B8:
case 0xDAB8:
case 0xDCB8:
case 0xDEB8:

// ADDaD
case 0xD0B8:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(18)
case 0xD2B9:
case 0xD4B9:
case 0xD6B9:
case 0xD8B9:
case 0xDAB9:
case 0xDCB9:
case 0xDEB9:

// ADDaD
case 0xD0B9:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(22)
case 0xD2BA:
case 0xD4BA:
case 0xD6BA:
case 0xD8BA:
case 0xDABA:
case 0xDCBA:
case 0xDEBA:

// ADDaD
case 0xD0BA:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(18)
case 0xD2BB:
case 0xD4BB:
case 0xD6BB:
case 0xD8BB:
case 0xDABB:
case 0xDCBB:
case 0xDEBB:

// ADDaD
case 0xD0BB:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(20)
case 0xD2BC:
case 0xD4BC:
case 0xD6BC:
case 0xD8BC:
case 0xDABC:
case 0xDCBC:
case 0xDEBC:

// ADDaD
case 0xD0BC:
{
	u32 res;
	u32 dst;
	u32 src;
	src = FETCH_LONG;
	PC += 4;
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(16)
case 0xD29F:
case 0xD49F:
case 0xD69F:
case 0xD89F:
case 0xDA9F:
case 0xDC9F:
case 0xDE9F:

// ADDaD
case 0xD09F:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0xD2A7:
case 0xD4A7:
case 0xD6A7:
case 0xD8A7:
case 0xDAA7:
case 0xDCA7:
case 0xDEA7:

// ADDaD
case 0xD0A7:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0xD310:
case 0xD510:
case 0xD710:
case 0xD910:
case 0xDB10:
case 0xDD10:
case 0xDF10:
case 0xD111:
case 0xD311:
case 0xD511:
case 0xD711:
case 0xD911:
case 0xDB11:
case 0xDD11:
case 0xDF11:
case 0xD112:
case 0xD312:
case 0xD512:
case 0xD712:
case 0xD912:
case 0xDB12:
case 0xDD12:
case 0xDF12:
case 0xD113:
case 0xD313:
case 0xD513:
case 0xD713:
case 0xD913:
case 0xDB13:
case 0xDD13:
case 0xDF13:
case 0xD114:
case 0xD314:
case 0xD514:
case 0xD714:
case 0xD914:
case 0xDB14:
case 0xDD14:
case 0xDF14:
case 0xD115:
case 0xD315:
case 0xD515:
case 0xD715:
case 0xD915:
case 0xDB15:
case 0xDD15:
case 0xDF15:
case 0xD116:
case 0xD316:
case 0xD516:
case 0xD716:
case 0xD916:
case 0xDB16:
case 0xDD16:
case 0xDF16:
case 0xD117:
case 0xD317:
case 0xD517:
case 0xD717:
case 0xD917:
case 0xDB17:
case 0xDD17:
case 0xDF17:

// ADDDa
case 0xD110:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
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
case 0xD318:
case 0xD518:
case 0xD718:
case 0xD918:
case 0xDB18:
case 0xDD18:
case 0xDF18:
case 0xD119:
case 0xD319:
case 0xD519:
case 0xD719:
case 0xD919:
case 0xDB19:
case 0xDD19:
case 0xDF19:
case 0xD11A:
case 0xD31A:
case 0xD51A:
case 0xD71A:
case 0xD91A:
case 0xDB1A:
case 0xDD1A:
case 0xDF1A:
case 0xD11B:
case 0xD31B:
case 0xD51B:
case 0xD71B:
case 0xD91B:
case 0xDB1B:
case 0xDD1B:
case 0xDF1B:
case 0xD11C:
case 0xD31C:
case 0xD51C:
case 0xD71C:
case 0xD91C:
case 0xDB1C:
case 0xDD1C:
case 0xDF1C:
case 0xD11D:
case 0xD31D:
case 0xD51D:
case 0xD71D:
case 0xD91D:
case 0xDB1D:
case 0xDD1D:
case 0xDF1D:
case 0xD11E:
case 0xD31E:
case 0xD51E:
case 0xD71E:
case 0xD91E:
case 0xDB1E:
case 0xDD1E:
case 0xDF1E:

// ADDDa
case 0xD118:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0xD320:
case 0xD520:
case 0xD720:
case 0xD920:
case 0xDB20:
case 0xDD20:
case 0xDF20:
case 0xD121:
case 0xD321:
case 0xD521:
case 0xD721:
case 0xD921:
case 0xDB21:
case 0xDD21:
case 0xDF21:
case 0xD122:
case 0xD322:
case 0xD522:
case 0xD722:
case 0xD922:
case 0xDB22:
case 0xDD22:
case 0xDF22:
case 0xD123:
case 0xD323:
case 0xD523:
case 0xD723:
case 0xD923:
case 0xDB23:
case 0xDD23:
case 0xDF23:
case 0xD124:
case 0xD324:
case 0xD524:
case 0xD724:
case 0xD924:
case 0xDB24:
case 0xDD24:
case 0xDF24:
case 0xD125:
case 0xD325:
case 0xD525:
case 0xD725:
case 0xD925:
case 0xDB25:
case 0xDD25:
case 0xDF25:
case 0xD126:
case 0xD326:
case 0xD526:
case 0xD726:
case 0xD926:
case 0xDB26:
case 0xDD26:
case 0xDF26:

// ADDDa
case 0xD120:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0xD328:
case 0xD528:
case 0xD728:
case 0xD928:
case 0xDB28:
case 0xDD28:
case 0xDF28:
case 0xD129:
case 0xD329:
case 0xD529:
case 0xD729:
case 0xD929:
case 0xDB29:
case 0xDD29:
case 0xDF29:
case 0xD12A:
case 0xD32A:
case 0xD52A:
case 0xD72A:
case 0xD92A:
case 0xDB2A:
case 0xDD2A:
case 0xDF2A:
case 0xD12B:
case 0xD32B:
case 0xD52B:
case 0xD72B:
case 0xD92B:
case 0xDB2B:
case 0xDD2B:
case 0xDF2B:
case 0xD12C:
case 0xD32C:
case 0xD52C:
case 0xD72C:
case 0xD92C:
case 0xDB2C:
case 0xDD2C:
case 0xDF2C:
case 0xD12D:
case 0xD32D:
case 0xD52D:
case 0xD72D:
case 0xD92D:
case 0xDB2D:
case 0xDD2D:
case 0xDF2D:
case 0xD12E:
case 0xD32E:
case 0xD52E:
case 0xD72E:
case 0xD92E:
case 0xDB2E:
case 0xDD2E:
case 0xDF2E:
case 0xD12F:
case 0xD32F:
case 0xD52F:
case 0xD72F:
case 0xD92F:
case 0xDB2F:
case 0xDD2F:
case 0xDF2F:

// ADDDa
case 0xD128:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0xD330:
case 0xD530:
case 0xD730:
case 0xD930:
case 0xDB30:
case 0xDD30:
case 0xDF30:
case 0xD131:
case 0xD331:
case 0xD531:
case 0xD731:
case 0xD931:
case 0xDB31:
case 0xDD31:
case 0xDF31:
case 0xD132:
case 0xD332:
case 0xD532:
case 0xD732:
case 0xD932:
case 0xDB32:
case 0xDD32:
case 0xDF32:
case 0xD133:
case 0xD333:
case 0xD533:
case 0xD733:
case 0xD933:
case 0xDB33:
case 0xDD33:
case 0xDF33:
case 0xD134:
case 0xD334:
case 0xD534:
case 0xD734:
case 0xD934:
case 0xDB34:
case 0xDD34:
case 0xDF34:
case 0xD135:
case 0xD335:
case 0xD535:
case 0xD735:
case 0xD935:
case 0xDB35:
case 0xDD35:
case 0xDF35:
case 0xD136:
case 0xD336:
case 0xD536:
case 0xD736:
case 0xD936:
case 0xDB36:
case 0xDD36:
case 0xDF36:
case 0xD137:
case 0xD337:
case 0xD537:
case 0xD737:
case 0xD937:
case 0xDB37:
case 0xDD37:
case 0xDF37:

// ADDDa
case 0xD130:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0xD338:
case 0xD538:
case 0xD738:
case 0xD938:
case 0xDB38:
case 0xDD38:
case 0xDF38:

// ADDDa
case 0xD138:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0xD339:
case 0xD539:
case 0xD739:
case 0xD939:
case 0xDB39:
case 0xDD39:
case 0xDF39:

// ADDDa
case 0xD139:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0xD31F:
case 0xD51F:
case 0xD71F:
case 0xD91F:
case 0xDB1F:
case 0xDD1F:
case 0xDF1F:

// ADDDa
case 0xD11F:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0xD327:
case 0xD527:
case 0xD727:
case 0xD927:
case 0xDB27:
case 0xDD27:
case 0xDF27:

// ADDDa
case 0xD127:
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
	res = dst + src;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ = res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0xD350:
case 0xD550:
case 0xD750:
case 0xD950:
case 0xDB50:
case 0xDD50:
case 0xDF50:
case 0xD151:
case 0xD351:
case 0xD551:
case 0xD751:
case 0xD951:
case 0xDB51:
case 0xDD51:
case 0xDF51:
case 0xD152:
case 0xD352:
case 0xD552:
case 0xD752:
case 0xD952:
case 0xDB52:
case 0xDD52:
case 0xDF52:
case 0xD153:
case 0xD353:
case 0xD553:
case 0xD753:
case 0xD953:
case 0xDB53:
case 0xDD53:
case 0xDF53:
case 0xD154:
case 0xD354:
case 0xD554:
case 0xD754:
case 0xD954:
case 0xDB54:
case 0xDD54:
case 0xDF54:
case 0xD155:
case 0xD355:
case 0xD555:
case 0xD755:
case 0xD955:
case 0xDB55:
case 0xDD55:
case 0xDF55:
case 0xD156:
case 0xD356:
case 0xD556:
case 0xD756:
case 0xD956:
case 0xDB56:
case 0xDD56:
case 0xDF56:
case 0xD157:
case 0xD357:
case 0xD557:
case 0xD757:
case 0xD957:
case 0xDB57:
case 0xDD57:
case 0xDF57:

// ADDDa
case 0xD150:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
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
case 0xD358:
case 0xD558:
case 0xD758:
case 0xD958:
case 0xDB58:
case 0xDD58:
case 0xDF58:
case 0xD159:
case 0xD359:
case 0xD559:
case 0xD759:
case 0xD959:
case 0xDB59:
case 0xDD59:
case 0xDF59:
case 0xD15A:
case 0xD35A:
case 0xD55A:
case 0xD75A:
case 0xD95A:
case 0xDB5A:
case 0xDD5A:
case 0xDF5A:
case 0xD15B:
case 0xD35B:
case 0xD55B:
case 0xD75B:
case 0xD95B:
case 0xDB5B:
case 0xDD5B:
case 0xDF5B:
case 0xD15C:
case 0xD35C:
case 0xD55C:
case 0xD75C:
case 0xD95C:
case 0xDB5C:
case 0xDD5C:
case 0xDF5C:
case 0xD15D:
case 0xD35D:
case 0xD55D:
case 0xD75D:
case 0xD95D:
case 0xDB5D:
case 0xDD5D:
case 0xDF5D:
case 0xD15E:
case 0xD35E:
case 0xD55E:
case 0xD75E:
case 0xD95E:
case 0xDB5E:
case 0xDD5E:
case 0xDF5E:

// ADDDa
case 0xD158:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xD360:
case 0xD560:
case 0xD760:
case 0xD960:
case 0xDB60:
case 0xDD60:
case 0xDF60:
case 0xD161:
case 0xD361:
case 0xD561:
case 0xD761:
case 0xD961:
case 0xDB61:
case 0xDD61:
case 0xDF61:
case 0xD162:
case 0xD362:
case 0xD562:
case 0xD762:
case 0xD962:
case 0xDB62:
case 0xDD62:
case 0xDF62:
case 0xD163:
case 0xD363:
case 0xD563:
case 0xD763:
case 0xD963:
case 0xDB63:
case 0xDD63:
case 0xDF63:
case 0xD164:
case 0xD364:
case 0xD564:
case 0xD764:
case 0xD964:
case 0xDB64:
case 0xDD64:
case 0xDF64:
case 0xD165:
case 0xD365:
case 0xD565:
case 0xD765:
case 0xD965:
case 0xDB65:
case 0xDD65:
case 0xDF65:
case 0xD166:
case 0xD366:
case 0xD566:
case 0xD766:
case 0xD966:
case 0xDB66:
case 0xDD66:
case 0xDF66:

// ADDDa
case 0xD160:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xD368:
case 0xD568:
case 0xD768:
case 0xD968:
case 0xDB68:
case 0xDD68:
case 0xDF68:
case 0xD169:
case 0xD369:
case 0xD569:
case 0xD769:
case 0xD969:
case 0xDB69:
case 0xDD69:
case 0xDF69:
case 0xD16A:
case 0xD36A:
case 0xD56A:
case 0xD76A:
case 0xD96A:
case 0xDB6A:
case 0xDD6A:
case 0xDF6A:
case 0xD16B:
case 0xD36B:
case 0xD56B:
case 0xD76B:
case 0xD96B:
case 0xDB6B:
case 0xDD6B:
case 0xDF6B:
case 0xD16C:
case 0xD36C:
case 0xD56C:
case 0xD76C:
case 0xD96C:
case 0xDB6C:
case 0xDD6C:
case 0xDF6C:
case 0xD16D:
case 0xD36D:
case 0xD56D:
case 0xD76D:
case 0xD96D:
case 0xDB6D:
case 0xDD6D:
case 0xDF6D:
case 0xD16E:
case 0xD36E:
case 0xD56E:
case 0xD76E:
case 0xD96E:
case 0xDB6E:
case 0xDD6E:
case 0xDF6E:
case 0xD16F:
case 0xD36F:
case 0xD56F:
case 0xD76F:
case 0xD96F:
case 0xDB6F:
case 0xDD6F:
case 0xDF6F:

// ADDDa
case 0xD168:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0xD370:
case 0xD570:
case 0xD770:
case 0xD970:
case 0xDB70:
case 0xDD70:
case 0xDF70:
case 0xD171:
case 0xD371:
case 0xD571:
case 0xD771:
case 0xD971:
case 0xDB71:
case 0xDD71:
case 0xDF71:
case 0xD172:
case 0xD372:
case 0xD572:
case 0xD772:
case 0xD972:
case 0xDB72:
case 0xDD72:
case 0xDF72:
case 0xD173:
case 0xD373:
case 0xD573:
case 0xD773:
case 0xD973:
case 0xDB73:
case 0xDD73:
case 0xDF73:
case 0xD174:
case 0xD374:
case 0xD574:
case 0xD774:
case 0xD974:
case 0xDB74:
case 0xDD74:
case 0xDF74:
case 0xD175:
case 0xD375:
case 0xD575:
case 0xD775:
case 0xD975:
case 0xDB75:
case 0xDD75:
case 0xDF75:
case 0xD176:
case 0xD376:
case 0xD576:
case 0xD776:
case 0xD976:
case 0xDB76:
case 0xDD76:
case 0xDF76:
case 0xD177:
case 0xD377:
case 0xD577:
case 0xD777:
case 0xD977:
case 0xDB77:
case 0xDD77:
case 0xDF77:

// ADDDa
case 0xD170:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0xD378:
case 0xD578:
case 0xD778:
case 0xD978:
case 0xDB78:
case 0xDD78:
case 0xDF78:

// ADDDa
case 0xD178:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0xD379:
case 0xD579:
case 0xD779:
case 0xD979:
case 0xDB79:
case 0xDD79:
case 0xDF79:

// ADDDa
case 0xD179:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0xD35F:
case 0xD55F:
case 0xD75F:
case 0xD95F:
case 0xDB5F:
case 0xDD5F:
case 0xDF5F:

// ADDDa
case 0xD15F:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xD367:
case 0xD567:
case 0xD767:
case 0xD967:
case 0xDB67:
case 0xDD67:
case 0xDF67:

// ADDDa
case 0xD167:
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
	res = dst + src;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ = res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xD390:
case 0xD590:
case 0xD790:
case 0xD990:
case 0xDB90:
case 0xDD90:
case 0xDF90:
case 0xD191:
case 0xD391:
case 0xD591:
case 0xD791:
case 0xD991:
case 0xDB91:
case 0xDD91:
case 0xDF91:
case 0xD192:
case 0xD392:
case 0xD592:
case 0xD792:
case 0xD992:
case 0xDB92:
case 0xDD92:
case 0xDF92:
case 0xD193:
case 0xD393:
case 0xD593:
case 0xD793:
case 0xD993:
case 0xDB93:
case 0xDD93:
case 0xDF93:
case 0xD194:
case 0xD394:
case 0xD594:
case 0xD794:
case 0xD994:
case 0xDB94:
case 0xDD94:
case 0xDF94:
case 0xD195:
case 0xD395:
case 0xD595:
case 0xD795:
case 0xD995:
case 0xDB95:
case 0xDD95:
case 0xDF95:
case 0xD196:
case 0xD396:
case 0xD596:
case 0xD796:
case 0xD996:
case 0xDB96:
case 0xDD96:
case 0xDF96:
case 0xD197:
case 0xD397:
case 0xD597:
case 0xD797:
case 0xD997:
case 0xDB97:
case 0xDD97:
case 0xDF97:

// ADDDa
case 0xD190:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
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
case 0xD398:
case 0xD598:
case 0xD798:
case 0xD998:
case 0xDB98:
case 0xDD98:
case 0xDF98:
case 0xD199:
case 0xD399:
case 0xD599:
case 0xD799:
case 0xD999:
case 0xDB99:
case 0xDD99:
case 0xDF99:
case 0xD19A:
case 0xD39A:
case 0xD59A:
case 0xD79A:
case 0xD99A:
case 0xDB9A:
case 0xDD9A:
case 0xDF9A:
case 0xD19B:
case 0xD39B:
case 0xD59B:
case 0xD79B:
case 0xD99B:
case 0xDB9B:
case 0xDD9B:
case 0xDF9B:
case 0xD19C:
case 0xD39C:
case 0xD59C:
case 0xD79C:
case 0xD99C:
case 0xDB9C:
case 0xDD9C:
case 0xDF9C:
case 0xD19D:
case 0xD39D:
case 0xD59D:
case 0xD79D:
case 0xD99D:
case 0xDB9D:
case 0xDD9D:
case 0xDF9D:
case 0xD19E:
case 0xD39E:
case 0xD59E:
case 0xD79E:
case 0xD99E:
case 0xDB9E:
case 0xDD9E:
case 0xDF9E:

// ADDDa
case 0xD198:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0xD3A0:
case 0xD5A0:
case 0xD7A0:
case 0xD9A0:
case 0xDBA0:
case 0xDDA0:
case 0xDFA0:
case 0xD1A1:
case 0xD3A1:
case 0xD5A1:
case 0xD7A1:
case 0xD9A1:
case 0xDBA1:
case 0xDDA1:
case 0xDFA1:
case 0xD1A2:
case 0xD3A2:
case 0xD5A2:
case 0xD7A2:
case 0xD9A2:
case 0xDBA2:
case 0xDDA2:
case 0xDFA2:
case 0xD1A3:
case 0xD3A3:
case 0xD5A3:
case 0xD7A3:
case 0xD9A3:
case 0xDBA3:
case 0xDDA3:
case 0xDFA3:
case 0xD1A4:
case 0xD3A4:
case 0xD5A4:
case 0xD7A4:
case 0xD9A4:
case 0xDBA4:
case 0xDDA4:
case 0xDFA4:
case 0xD1A5:
case 0xD3A5:
case 0xD5A5:
case 0xD7A5:
case 0xD9A5:
case 0xDBA5:
case 0xDDA5:
case 0xDFA5:
case 0xD1A6:
case 0xD3A6:
case 0xD5A6:
case 0xD7A6:
case 0xD9A6:
case 0xDBA6:
case 0xDDA6:
case 0xDFA6:

// ADDDa
case 0xD1A0:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0xD3A8:
case 0xD5A8:
case 0xD7A8:
case 0xD9A8:
case 0xDBA8:
case 0xDDA8:
case 0xDFA8:
case 0xD1A9:
case 0xD3A9:
case 0xD5A9:
case 0xD7A9:
case 0xD9A9:
case 0xDBA9:
case 0xDDA9:
case 0xDFA9:
case 0xD1AA:
case 0xD3AA:
case 0xD5AA:
case 0xD7AA:
case 0xD9AA:
case 0xDBAA:
case 0xDDAA:
case 0xDFAA:
case 0xD1AB:
case 0xD3AB:
case 0xD5AB:
case 0xD7AB:
case 0xD9AB:
case 0xDBAB:
case 0xDDAB:
case 0xDFAB:
case 0xD1AC:
case 0xD3AC:
case 0xD5AC:
case 0xD7AC:
case 0xD9AC:
case 0xDBAC:
case 0xDDAC:
case 0xDFAC:
case 0xD1AD:
case 0xD3AD:
case 0xD5AD:
case 0xD7AD:
case 0xD9AD:
case 0xDBAD:
case 0xDDAD:
case 0xDFAD:
case 0xD1AE:
case 0xD3AE:
case 0xD5AE:
case 0xD7AE:
case 0xD9AE:
case 0xDBAE:
case 0xDDAE:
case 0xDFAE:
case 0xD1AF:
case 0xD3AF:
case 0xD5AF:
case 0xD7AF:
case 0xD9AF:
case 0xDBAF:
case 0xDDAF:
case 0xDFAF:

// ADDDa
case 0xD1A8:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0xD3B0:
case 0xD5B0:
case 0xD7B0:
case 0xD9B0:
case 0xDBB0:
case 0xDDB0:
case 0xDFB0:
case 0xD1B1:
case 0xD3B1:
case 0xD5B1:
case 0xD7B1:
case 0xD9B1:
case 0xDBB1:
case 0xDDB1:
case 0xDFB1:
case 0xD1B2:
case 0xD3B2:
case 0xD5B2:
case 0xD7B2:
case 0xD9B2:
case 0xDBB2:
case 0xDDB2:
case 0xDFB2:
case 0xD1B3:
case 0xD3B3:
case 0xD5B3:
case 0xD7B3:
case 0xD9B3:
case 0xDBB3:
case 0xDDB3:
case 0xDFB3:
case 0xD1B4:
case 0xD3B4:
case 0xD5B4:
case 0xD7B4:
case 0xD9B4:
case 0xDBB4:
case 0xDDB4:
case 0xDFB4:
case 0xD1B5:
case 0xD3B5:
case 0xD5B5:
case 0xD7B5:
case 0xD9B5:
case 0xDBB5:
case 0xDDB5:
case 0xDFB5:
case 0xD1B6:
case 0xD3B6:
case 0xD5B6:
case 0xD7B6:
case 0xD9B6:
case 0xDBB6:
case 0xDDB6:
case 0xDFB6:
case 0xD1B7:
case 0xD3B7:
case 0xD5B7:
case 0xD7B7:
case 0xD9B7:
case 0xDBB7:
case 0xDDB7:
case 0xDFB7:

// ADDDa
case 0xD1B0:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0xD3B8:
case 0xD5B8:
case 0xD7B8:
case 0xD9B8:
case 0xDBB8:
case 0xDDB8:
case 0xDFB8:

// ADDDa
case 0xD1B8:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0xD3B9:
case 0xD5B9:
case 0xD7B9:
case 0xD9B9:
case 0xDBB9:
case 0xDDB9:
case 0xDFB9:

// ADDDa
case 0xD1B9:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0xD39F:
case 0xD59F:
case 0xD79F:
case 0xD99F:
case 0xDB9F:
case 0xDD9F:
case 0xDF9F:

// ADDDa
case 0xD19F:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0xD3A7:
case 0xD5A7:
case 0xD7A7:
case 0xD9A7:
case 0xDBA7:
case 0xDDA7:
case 0xDFA7:

// ADDDa
case 0xD1A7:
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
	res = dst + src;
	CPU->flag_notZ = res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0xD300:
case 0xD500:
case 0xD700:
case 0xD900:
case 0xDB00:
case 0xDD00:
case 0xDF00:
case 0xD101:
case 0xD301:
case 0xD501:
case 0xD701:
case 0xD901:
case 0xDB01:
case 0xDD01:
case 0xDF01:
case 0xD102:
case 0xD302:
case 0xD502:
case 0xD702:
case 0xD902:
case 0xDB02:
case 0xDD02:
case 0xDF02:
case 0xD103:
case 0xD303:
case 0xD503:
case 0xD703:
case 0xD903:
case 0xDB03:
case 0xDD03:
case 0xDF03:
case 0xD104:
case 0xD304:
case 0xD504:
case 0xD704:
case 0xD904:
case 0xDB04:
case 0xDD04:
case 0xDF04:
case 0xD105:
case 0xD305:
case 0xD505:
case 0xD705:
case 0xD905:
case 0xDB05:
case 0xDD05:
case 0xDF05:
case 0xD106:
case 0xD306:
case 0xD506:
case 0xD706:
case 0xD906:
case 0xDB06:
case 0xDD06:
case 0xDF06:
case 0xD107:
case 0xD307:
case 0xD507:
case 0xD707:
case 0xD907:
case 0xDB07:
case 0xDD07:
case 0xDF07:

// ADDX
case 0xD100:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ |= res & 0xFF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0xD340:
case 0xD540:
case 0xD740:
case 0xD940:
case 0xDB40:
case 0xDD40:
case 0xDF40:
case 0xD141:
case 0xD341:
case 0xD541:
case 0xD741:
case 0xD941:
case 0xDB41:
case 0xDD41:
case 0xDF41:
case 0xD142:
case 0xD342:
case 0xD542:
case 0xD742:
case 0xD942:
case 0xDB42:
case 0xDD42:
case 0xDF42:
case 0xD143:
case 0xD343:
case 0xD543:
case 0xD743:
case 0xD943:
case 0xDB43:
case 0xDD43:
case 0xDF43:
case 0xD144:
case 0xD344:
case 0xD544:
case 0xD744:
case 0xD944:
case 0xDB44:
case 0xDD44:
case 0xDF44:
case 0xD145:
case 0xD345:
case 0xD545:
case 0xD745:
case 0xD945:
case 0xDB45:
case 0xDD45:
case 0xDF45:
case 0xD146:
case 0xD346:
case 0xD546:
case 0xD746:
case 0xD946:
case 0xDB46:
case 0xDD46:
case 0xDF46:
case 0xD147:
case 0xD347:
case 0xD547:
case 0xD747:
case 0xD947:
case 0xDB47:
case 0xDD47:
case 0xDF47:

// ADDX
case 0xD140:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	dst = (u16)CPU->D[(Opcode >> 9) & 7];
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0xD380:
case 0xD580:
case 0xD780:
case 0xD980:
case 0xDB80:
case 0xDD80:
case 0xDF80:
case 0xD181:
case 0xD381:
case 0xD581:
case 0xD781:
case 0xD981:
case 0xDB81:
case 0xDD81:
case 0xDF81:
case 0xD182:
case 0xD382:
case 0xD582:
case 0xD782:
case 0xD982:
case 0xDB82:
case 0xDD82:
case 0xDF82:
case 0xD183:
case 0xD383:
case 0xD583:
case 0xD783:
case 0xD983:
case 0xDB83:
case 0xDD83:
case 0xDF83:
case 0xD184:
case 0xD384:
case 0xD584:
case 0xD784:
case 0xD984:
case 0xDB84:
case 0xDD84:
case 0xDF84:
case 0xD185:
case 0xD385:
case 0xD585:
case 0xD785:
case 0xD985:
case 0xDB85:
case 0xDD85:
case 0xDF85:
case 0xD186:
case 0xD386:
case 0xD586:
case 0xD786:
case 0xD986:
case 0xDB86:
case 0xDD86:
case 0xDF86:
case 0xD187:
case 0xD387:
case 0xD587:
case 0xD787:
case 0xD987:
case 0xDB87:
case 0xDD87:
case 0xDF87:

// ADDX
case 0xD180:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0xD308:
case 0xD508:
case 0xD708:
case 0xD908:
case 0xDB08:
case 0xDD08:
case 0xD109:
case 0xD309:
case 0xD509:
case 0xD709:
case 0xD909:
case 0xDB09:
case 0xDD09:
case 0xD10A:
case 0xD30A:
case 0xD50A:
case 0xD70A:
case 0xD90A:
case 0xDB0A:
case 0xDD0A:
case 0xD10B:
case 0xD30B:
case 0xD50B:
case 0xD70B:
case 0xD90B:
case 0xDB0B:
case 0xDD0B:
case 0xD10C:
case 0xD30C:
case 0xD50C:
case 0xD70C:
case 0xD90C:
case 0xDB0C:
case 0xDD0C:
case 0xD10D:
case 0xD30D:
case 0xD50D:
case 0xD70D:
case 0xD90D:
case 0xDB0D:
case 0xDD0D:
case 0xD10E:
case 0xD30E:
case 0xD50E:
case 0xD70E:
case 0xD90E:
case 0xDB0E:
case 0xDD0E:

// ADDXM
case 0xD108:
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
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0xD348:
case 0xD548:
case 0xD748:
case 0xD948:
case 0xDB48:
case 0xDD48:
case 0xD149:
case 0xD349:
case 0xD549:
case 0xD749:
case 0xD949:
case 0xDB49:
case 0xDD49:
case 0xD14A:
case 0xD34A:
case 0xD54A:
case 0xD74A:
case 0xD94A:
case 0xDB4A:
case 0xDD4A:
case 0xD14B:
case 0xD34B:
case 0xD54B:
case 0xD74B:
case 0xD94B:
case 0xDB4B:
case 0xDD4B:
case 0xD14C:
case 0xD34C:
case 0xD54C:
case 0xD74C:
case 0xD94C:
case 0xDB4C:
case 0xDD4C:
case 0xD14D:
case 0xD34D:
case 0xD54D:
case 0xD74D:
case 0xD94D:
case 0xDB4D:
case 0xDD4D:
case 0xD14E:
case 0xD34E:
case 0xD54E:
case 0xD74E:
case 0xD94E:
case 0xDB4E:
case 0xDD4E:

// ADDXM
case 0xD148:
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
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0xD388:
case 0xD588:
case 0xD788:
case 0xD988:
case 0xDB88:
case 0xDD88:
case 0xD189:
case 0xD389:
case 0xD589:
case 0xD789:
case 0xD989:
case 0xDB89:
case 0xDD89:
case 0xD18A:
case 0xD38A:
case 0xD58A:
case 0xD78A:
case 0xD98A:
case 0xDB8A:
case 0xDD8A:
case 0xD18B:
case 0xD38B:
case 0xD58B:
case 0xD78B:
case 0xD98B:
case 0xDB8B:
case 0xDD8B:
case 0xD18C:
case 0xD38C:
case 0xD58C:
case 0xD78C:
case 0xD98C:
case 0xDB8C:
case 0xDD8C:
case 0xD18D:
case 0xD38D:
case 0xD58D:
case 0xD78D:
case 0xD98D:
case 0xDB8D:
case 0xDD8D:
case 0xD18E:
case 0xD38E:
case 0xD58E:
case 0xD78E:
case 0xD98E:
case 0xDB8E:
case 0xDD8E:

// ADDXM
case 0xD188:
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
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)
case 0xD30F:
case 0xD50F:
case 0xD70F:
case 0xD90F:
case 0xDB0F:
case 0xDD0F:

// ADDX7M
case 0xD10F:
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
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0xD34F:
case 0xD54F:
case 0xD74F:
case 0xD94F:
case 0xDB4F:
case 0xDD4F:

// ADDX7M
case 0xD14F:
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
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0xD38F:
case 0xD58F:
case 0xD78F:
case 0xD98F:
case 0xDB8F:
case 0xDD8F:

// ADDX7M
case 0xD18F:
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
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)
case 0xDF09:
case 0xDF0A:
case 0xDF0B:
case 0xDF0C:
case 0xDF0D:
case 0xDF0E:

// ADDXM7
case 0xDF08:
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
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0xDF49:
case 0xDF4A:
case 0xDF4B:
case 0xDF4C:
case 0xDF4D:
case 0xDF4E:

// ADDXM7
case 0xDF48:
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
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0xDF89:
case 0xDF8A:
case 0xDF8B:
case 0xDF8C:
case 0xDF8D:
case 0xDF8E:

// ADDXM7
case 0xDF88:
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
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)

// ADDX7M7
case 0xDF0F:
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
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res;
	CPU->flag_V = (src ^ res) & (dst ^ res);
	CPU->flag_notZ |= res & 0xFF;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)

// ADDX7M7
case 0xDF4F:
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
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;
	CPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_notZ |= res & 0xFFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// ADDX7M7
case 0xDF8F:
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
	res = dst + src + ((CPU->flag_X >> 8) & 1);
	CPU->flag_notZ |= res;
	CPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;
	CPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)
case 0xD2C0:
case 0xD4C0:
case 0xD6C0:
case 0xD8C0:
case 0xDAC0:
case 0xDCC0:
case 0xDEC0:
case 0xD0C1:
case 0xD2C1:
case 0xD4C1:
case 0xD6C1:
case 0xD8C1:
case 0xDAC1:
case 0xDCC1:
case 0xDEC1:
case 0xD0C2:
case 0xD2C2:
case 0xD4C2:
case 0xD6C2:
case 0xD8C2:
case 0xDAC2:
case 0xDCC2:
case 0xDEC2:
case 0xD0C3:
case 0xD2C3:
case 0xD4C3:
case 0xD6C3:
case 0xD8C3:
case 0xDAC3:
case 0xDCC3:
case 0xDEC3:
case 0xD0C4:
case 0xD2C4:
case 0xD4C4:
case 0xD6C4:
case 0xD8C4:
case 0xDAC4:
case 0xDCC4:
case 0xDEC4:
case 0xD0C5:
case 0xD2C5:
case 0xD4C5:
case 0xD6C5:
case 0xD8C5:
case 0xDAC5:
case 0xDCC5:
case 0xDEC5:
case 0xD0C6:
case 0xD2C6:
case 0xD4C6:
case 0xD6C6:
case 0xD8C6:
case 0xDAC6:
case 0xDCC6:
case 0xDEC6:
case 0xD0C7:
case 0xD2C7:
case 0xD4C7:
case 0xD6C7:
case 0xD8C7:
case 0xDAC7:
case 0xDCC7:
case 0xDEC7:

// ADDA
case 0xD0C0:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s16)CPU->D[(Opcode >> 0) & 7];
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(8)
case 0xD2C8:
case 0xD4C8:
case 0xD6C8:
case 0xD8C8:
case 0xDAC8:
case 0xDCC8:
case 0xDEC8:
case 0xD0C9:
case 0xD2C9:
case 0xD4C9:
case 0xD6C9:
case 0xD8C9:
case 0xDAC9:
case 0xDCC9:
case 0xDEC9:
case 0xD0CA:
case 0xD2CA:
case 0xD4CA:
case 0xD6CA:
case 0xD8CA:
case 0xDACA:
case 0xDCCA:
case 0xDECA:
case 0xD0CB:
case 0xD2CB:
case 0xD4CB:
case 0xD6CB:
case 0xD8CB:
case 0xDACB:
case 0xDCCB:
case 0xDECB:
case 0xD0CC:
case 0xD2CC:
case 0xD4CC:
case 0xD6CC:
case 0xD8CC:
case 0xDACC:
case 0xDCCC:
case 0xDECC:
case 0xD0CD:
case 0xD2CD:
case 0xD4CD:
case 0xD6CD:
case 0xD8CD:
case 0xDACD:
case 0xDCCD:
case 0xDECD:
case 0xD0CE:
case 0xD2CE:
case 0xD4CE:
case 0xD6CE:
case 0xD8CE:
case 0xDACE:
case 0xDCCE:
case 0xDECE:
case 0xD0CF:
case 0xD2CF:
case 0xD4CF:
case 0xD6CF:
case 0xD8CF:
case 0xDACF:
case 0xDCCF:
case 0xDECF:

// ADDA
case 0xD0C8:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s16)CPU->A[(Opcode >> 0) & 7];
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(8)
case 0xD2D0:
case 0xD4D0:
case 0xD6D0:
case 0xD8D0:
case 0xDAD0:
case 0xDCD0:
case 0xDED0:
case 0xD0D1:
case 0xD2D1:
case 0xD4D1:
case 0xD6D1:
case 0xD8D1:
case 0xDAD1:
case 0xDCD1:
case 0xDED1:
case 0xD0D2:
case 0xD2D2:
case 0xD4D2:
case 0xD6D2:
case 0xD8D2:
case 0xDAD2:
case 0xDCD2:
case 0xDED2:
case 0xD0D3:
case 0xD2D3:
case 0xD4D3:
case 0xD6D3:
case 0xD8D3:
case 0xDAD3:
case 0xDCD3:
case 0xDED3:
case 0xD0D4:
case 0xD2D4:
case 0xD4D4:
case 0xD6D4:
case 0xD8D4:
case 0xDAD4:
case 0xDCD4:
case 0xDED4:
case 0xD0D5:
case 0xD2D5:
case 0xD4D5:
case 0xD6D5:
case 0xD8D5:
case 0xDAD5:
case 0xDCD5:
case 0xDED5:
case 0xD0D6:
case 0xD2D6:
case 0xD4D6:
case 0xD6D6:
case 0xD8D6:
case 0xDAD6:
case 0xDCD6:
case 0xDED6:
case 0xD0D7:
case 0xD2D7:
case 0xD4D7:
case 0xD6D7:
case 0xD8D7:
case 0xDAD7:
case 0xDCD7:
case 0xDED7:

// ADDA
case 0xD0D0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READSX_WORD_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(10)
case 0xD2D8:
case 0xD4D8:
case 0xD6D8:
case 0xD8D8:
case 0xDAD8:
case 0xDCD8:
case 0xDED8:
case 0xD0D9:
case 0xD2D9:
case 0xD4D9:
case 0xD6D9:
case 0xD8D9:
case 0xDAD9:
case 0xDCD9:
case 0xDED9:
case 0xD0DA:
case 0xD2DA:
case 0xD4DA:
case 0xD6DA:
case 0xD8DA:
case 0xDADA:
case 0xDCDA:
case 0xDEDA:
case 0xD0DB:
case 0xD2DB:
case 0xD4DB:
case 0xD6DB:
case 0xD8DB:
case 0xDADB:
case 0xDCDB:
case 0xDEDB:
case 0xD0DC:
case 0xD2DC:
case 0xD4DC:
case 0xD6DC:
case 0xD8DC:
case 0xDADC:
case 0xDCDC:
case 0xDEDC:
case 0xD0DD:
case 0xD2DD:
case 0xD4DD:
case 0xD6DD:
case 0xD8DD:
case 0xDADD:
case 0xDCDD:
case 0xDEDD:
case 0xD0DE:
case 0xD2DE:
case 0xD4DE:
case 0xD6DE:
case 0xD8DE:
case 0xDADE:
case 0xDCDE:
case 0xDEDE:

// ADDA
case 0xD0D8:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(10)
case 0xD2E0:
case 0xD4E0:
case 0xD6E0:
case 0xD8E0:
case 0xDAE0:
case 0xDCE0:
case 0xDEE0:
case 0xD0E1:
case 0xD2E1:
case 0xD4E1:
case 0xD6E1:
case 0xD8E1:
case 0xDAE1:
case 0xDCE1:
case 0xDEE1:
case 0xD0E2:
case 0xD2E2:
case 0xD4E2:
case 0xD6E2:
case 0xD8E2:
case 0xDAE2:
case 0xDCE2:
case 0xDEE2:
case 0xD0E3:
case 0xD2E3:
case 0xD4E3:
case 0xD6E3:
case 0xD8E3:
case 0xDAE3:
case 0xDCE3:
case 0xDEE3:
case 0xD0E4:
case 0xD2E4:
case 0xD4E4:
case 0xD6E4:
case 0xD8E4:
case 0xDAE4:
case 0xDCE4:
case 0xDEE4:
case 0xD0E5:
case 0xD2E5:
case 0xD4E5:
case 0xD6E5:
case 0xD8E5:
case 0xDAE5:
case 0xDCE5:
case 0xDEE5:
case 0xD0E6:
case 0xD2E6:
case 0xD4E6:
case 0xD6E6:
case 0xD8E6:
case 0xDAE6:
case 0xDCE6:
case 0xDEE6:

// ADDA
case 0xD0E0:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(12)
case 0xD2E8:
case 0xD4E8:
case 0xD6E8:
case 0xD8E8:
case 0xDAE8:
case 0xDCE8:
case 0xDEE8:
case 0xD0E9:
case 0xD2E9:
case 0xD4E9:
case 0xD6E9:
case 0xD8E9:
case 0xDAE9:
case 0xDCE9:
case 0xDEE9:
case 0xD0EA:
case 0xD2EA:
case 0xD4EA:
case 0xD6EA:
case 0xD8EA:
case 0xDAEA:
case 0xDCEA:
case 0xDEEA:
case 0xD0EB:
case 0xD2EB:
case 0xD4EB:
case 0xD6EB:
case 0xD8EB:
case 0xDAEB:
case 0xDCEB:
case 0xDEEB:
case 0xD0EC:
case 0xD2EC:
case 0xD4EC:
case 0xD6EC:
case 0xD8EC:
case 0xDAEC:
case 0xDCEC:
case 0xDEEC:
case 0xD0ED:
case 0xD2ED:
case 0xD4ED:
case 0xD6ED:
case 0xD8ED:
case 0xDAED:
case 0xDCED:
case 0xDEED:
case 0xD0EE:
case 0xD2EE:
case 0xD4EE:
case 0xD6EE:
case 0xD8EE:
case 0xDAEE:
case 0xDCEE:
case 0xDEEE:
case 0xD0EF:
case 0xD2EF:
case 0xD4EF:
case 0xD6EF:
case 0xD8EF:
case 0xDAEF:
case 0xDCEF:
case 0xDEEF:

// ADDA
case 0xD0E8:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0xD2F0:
case 0xD4F0:
case 0xD6F0:
case 0xD8F0:
case 0xDAF0:
case 0xDCF0:
case 0xDEF0:
case 0xD0F1:
case 0xD2F1:
case 0xD4F1:
case 0xD6F1:
case 0xD8F1:
case 0xDAF1:
case 0xDCF1:
case 0xDEF1:
case 0xD0F2:
case 0xD2F2:
case 0xD4F2:
case 0xD6F2:
case 0xD8F2:
case 0xDAF2:
case 0xDCF2:
case 0xDEF2:
case 0xD0F3:
case 0xD2F3:
case 0xD4F3:
case 0xD6F3:
case 0xD8F3:
case 0xDAF3:
case 0xDCF3:
case 0xDEF3:
case 0xD0F4:
case 0xD2F4:
case 0xD4F4:
case 0xD6F4:
case 0xD8F4:
case 0xDAF4:
case 0xDCF4:
case 0xDEF4:
case 0xD0F5:
case 0xD2F5:
case 0xD4F5:
case 0xD6F5:
case 0xD8F5:
case 0xDAF5:
case 0xDCF5:
case 0xDEF5:
case 0xD0F6:
case 0xD2F6:
case 0xD4F6:
case 0xD6F6:
case 0xD8F6:
case 0xDAF6:
case 0xDCF6:
case 0xDEF6:
case 0xD0F7:
case 0xD2F7:
case 0xD4F7:
case 0xD6F7:
case 0xD8F7:
case 0xDAF7:
case 0xDCF7:
case 0xDEF7:

// ADDA
case 0xD0F0:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(16)
case 0xD2F8:
case 0xD4F8:
case 0xD6F8:
case 0xD8F8:
case 0xDAF8:
case 0xDCF8:
case 0xDEF8:

// ADDA
case 0xD0F8:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0xD2F9:
case 0xD4F9:
case 0xD6F9:
case 0xD8F9:
case 0xDAF9:
case 0xDCF9:
case 0xDEF9:

// ADDA
case 0xD0F9:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(18)
case 0xD2FA:
case 0xD4FA:
case 0xD6FA:
case 0xD8FA:
case 0xDAFA:
case 0xDCFA:
case 0xDEFA:

// ADDA
case 0xD0FA:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0xD2FB:
case 0xD4FB:
case 0xD6FB:
case 0xD8FB:
case 0xDAFB:
case 0xDCFB:
case 0xDEFB:

// ADDA
case 0xD0FB:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(16)
case 0xD2FC:
case 0xD4FC:
case 0xD6FC:
case 0xD8FC:
case 0xDAFC:
case 0xDCFC:
case 0xDEFC:

// ADDA
case 0xD0FC:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s16)FETCH_WORD;
	PC += 2;
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(12)
case 0xD2DF:
case 0xD4DF:
case 0xD6DF:
case 0xD8DF:
case 0xDADF:
case 0xDCDF:
case 0xDEDF:

// ADDA
case 0xD0DF:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(10)
case 0xD2E7:
case 0xD4E7:
case 0xD6E7:
case 0xD8E7:
case 0xDAE7:
case 0xDCE7:
case 0xDEE7:

// ADDA
case 0xD0E7:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(12)
case 0xD3C0:
case 0xD5C0:
case 0xD7C0:
case 0xD9C0:
case 0xDBC0:
case 0xDDC0:
case 0xDFC0:
case 0xD1C1:
case 0xD3C1:
case 0xD5C1:
case 0xD7C1:
case 0xD9C1:
case 0xDBC1:
case 0xDDC1:
case 0xDFC1:
case 0xD1C2:
case 0xD3C2:
case 0xD5C2:
case 0xD7C2:
case 0xD9C2:
case 0xDBC2:
case 0xDDC2:
case 0xDFC2:
case 0xD1C3:
case 0xD3C3:
case 0xD5C3:
case 0xD7C3:
case 0xD9C3:
case 0xDBC3:
case 0xDDC3:
case 0xDFC3:
case 0xD1C4:
case 0xD3C4:
case 0xD5C4:
case 0xD7C4:
case 0xD9C4:
case 0xDBC4:
case 0xDDC4:
case 0xDFC4:
case 0xD1C5:
case 0xD3C5:
case 0xD5C5:
case 0xD7C5:
case 0xD9C5:
case 0xDBC5:
case 0xDDC5:
case 0xDFC5:
case 0xD1C6:
case 0xD3C6:
case 0xD5C6:
case 0xD7C6:
case 0xD9C6:
case 0xDBC6:
case 0xDDC6:
case 0xDFC6:
case 0xD1C7:
case 0xD3C7:
case 0xD5C7:
case 0xD7C7:
case 0xD9C7:
case 0xDBC7:
case 0xDDC7:
case 0xDFC7:

// ADDA
case 0xD1C0:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s32)CPU->D[(Opcode >> 0) & 7];
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(6)
case 0xD3C8:
case 0xD5C8:
case 0xD7C8:
case 0xD9C8:
case 0xDBC8:
case 0xDDC8:
case 0xDFC8:
case 0xD1C9:
case 0xD3C9:
case 0xD5C9:
case 0xD7C9:
case 0xD9C9:
case 0xDBC9:
case 0xDDC9:
case 0xDFC9:
case 0xD1CA:
case 0xD3CA:
case 0xD5CA:
case 0xD7CA:
case 0xD9CA:
case 0xDBCA:
case 0xDDCA:
case 0xDFCA:
case 0xD1CB:
case 0xD3CB:
case 0xD5CB:
case 0xD7CB:
case 0xD9CB:
case 0xDBCB:
case 0xDDCB:
case 0xDFCB:
case 0xD1CC:
case 0xD3CC:
case 0xD5CC:
case 0xD7CC:
case 0xD9CC:
case 0xDBCC:
case 0xDDCC:
case 0xDFCC:
case 0xD1CD:
case 0xD3CD:
case 0xD5CD:
case 0xD7CD:
case 0xD9CD:
case 0xDBCD:
case 0xDDCD:
case 0xDFCD:
case 0xD1CE:
case 0xD3CE:
case 0xD5CE:
case 0xD7CE:
case 0xD9CE:
case 0xDBCE:
case 0xDDCE:
case 0xDFCE:
case 0xD1CF:
case 0xD3CF:
case 0xD5CF:
case 0xD7CF:
case 0xD9CF:
case 0xDBCF:
case 0xDDCF:
case 0xDFCF:

// ADDA
case 0xD1C8:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s32)CPU->A[(Opcode >> 0) & 7];
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(6)
case 0xD3D0:
case 0xD5D0:
case 0xD7D0:
case 0xD9D0:
case 0xDBD0:
case 0xDDD0:
case 0xDFD0:
case 0xD1D1:
case 0xD3D1:
case 0xD5D1:
case 0xD7D1:
case 0xD9D1:
case 0xDBD1:
case 0xDDD1:
case 0xDFD1:
case 0xD1D2:
case 0xD3D2:
case 0xD5D2:
case 0xD7D2:
case 0xD9D2:
case 0xDBD2:
case 0xDDD2:
case 0xDFD2:
case 0xD1D3:
case 0xD3D3:
case 0xD5D3:
case 0xD7D3:
case 0xD9D3:
case 0xDBD3:
case 0xDDD3:
case 0xDFD3:
case 0xD1D4:
case 0xD3D4:
case 0xD5D4:
case 0xD7D4:
case 0xD9D4:
case 0xDBD4:
case 0xDDD4:
case 0xDFD4:
case 0xD1D5:
case 0xD3D5:
case 0xD5D5:
case 0xD7D5:
case 0xD9D5:
case 0xDBD5:
case 0xDDD5:
case 0xDFD5:
case 0xD1D6:
case 0xD3D6:
case 0xD5D6:
case 0xD7D6:
case 0xD9D6:
case 0xDBD6:
case 0xDDD6:
case 0xDFD6:
case 0xD1D7:
case 0xD3D7:
case 0xD5D7:
case 0xD7D7:
case 0xD9D7:
case 0xDBD7:
case 0xDDD7:
case 0xDFD7:

// ADDA
case 0xD1D0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READSX_LONG_F(adr, src)
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0xD3D8:
case 0xD5D8:
case 0xD7D8:
case 0xD9D8:
case 0xDBD8:
case 0xDDD8:
case 0xDFD8:
case 0xD1D9:
case 0xD3D9:
case 0xD5D9:
case 0xD7D9:
case 0xD9D9:
case 0xDBD9:
case 0xDDD9:
case 0xDFD9:
case 0xD1DA:
case 0xD3DA:
case 0xD5DA:
case 0xD7DA:
case 0xD9DA:
case 0xDBDA:
case 0xDDDA:
case 0xDFDA:
case 0xD1DB:
case 0xD3DB:
case 0xD5DB:
case 0xD7DB:
case 0xD9DB:
case 0xDBDB:
case 0xDDDB:
case 0xDFDB:
case 0xD1DC:
case 0xD3DC:
case 0xD5DC:
case 0xD7DC:
case 0xD9DC:
case 0xDBDC:
case 0xDDDC:
case 0xDFDC:
case 0xD1DD:
case 0xD3DD:
case 0xD5DD:
case 0xD7DD:
case 0xD9DD:
case 0xDBDD:
case 0xDDDD:
case 0xDFDD:
case 0xD1DE:
case 0xD3DE:
case 0xD5DE:
case 0xD7DE:
case 0xD9DE:
case 0xDBDE:
case 0xDDDE:
case 0xDFDE:

// ADDA
case 0xD1D8:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0xD3E0:
case 0xD5E0:
case 0xD7E0:
case 0xD9E0:
case 0xDBE0:
case 0xDDE0:
case 0xDFE0:
case 0xD1E1:
case 0xD3E1:
case 0xD5E1:
case 0xD7E1:
case 0xD9E1:
case 0xDBE1:
case 0xDDE1:
case 0xDFE1:
case 0xD1E2:
case 0xD3E2:
case 0xD5E2:
case 0xD7E2:
case 0xD9E2:
case 0xDBE2:
case 0xDDE2:
case 0xDFE2:
case 0xD1E3:
case 0xD3E3:
case 0xD5E3:
case 0xD7E3:
case 0xD9E3:
case 0xDBE3:
case 0xDDE3:
case 0xDFE3:
case 0xD1E4:
case 0xD3E4:
case 0xD5E4:
case 0xD7E4:
case 0xD9E4:
case 0xDBE4:
case 0xDDE4:
case 0xDFE4:
case 0xD1E5:
case 0xD3E5:
case 0xD5E5:
case 0xD7E5:
case 0xD9E5:
case 0xDBE5:
case 0xDDE5:
case 0xDFE5:
case 0xD1E6:
case 0xD3E6:
case 0xD5E6:
case 0xD7E6:
case 0xD9E6:
case 0xDBE6:
case 0xDDE6:
case 0xDFE6:

// ADDA
case 0xD1E0:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(16)
case 0xD3E8:
case 0xD5E8:
case 0xD7E8:
case 0xD9E8:
case 0xDBE8:
case 0xDDE8:
case 0xDFE8:
case 0xD1E9:
case 0xD3E9:
case 0xD5E9:
case 0xD7E9:
case 0xD9E9:
case 0xDBE9:
case 0xDDE9:
case 0xDFE9:
case 0xD1EA:
case 0xD3EA:
case 0xD5EA:
case 0xD7EA:
case 0xD9EA:
case 0xDBEA:
case 0xDDEA:
case 0xDFEA:
case 0xD1EB:
case 0xD3EB:
case 0xD5EB:
case 0xD7EB:
case 0xD9EB:
case 0xDBEB:
case 0xDDEB:
case 0xDFEB:
case 0xD1EC:
case 0xD3EC:
case 0xD5EC:
case 0xD7EC:
case 0xD9EC:
case 0xDBEC:
case 0xDDEC:
case 0xDFEC:
case 0xD1ED:
case 0xD3ED:
case 0xD5ED:
case 0xD7ED:
case 0xD9ED:
case 0xDBED:
case 0xDDED:
case 0xDFED:
case 0xD1EE:
case 0xD3EE:
case 0xD5EE:
case 0xD7EE:
case 0xD9EE:
case 0xDBEE:
case 0xDDEE:
case 0xDFEE:
case 0xD1EF:
case 0xD3EF:
case 0xD5EF:
case 0xD7EF:
case 0xD9EF:
case 0xDBEF:
case 0xDDEF:
case 0xDFEF:

// ADDA
case 0xD1E8:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(18)
case 0xD3F0:
case 0xD5F0:
case 0xD7F0:
case 0xD9F0:
case 0xDBF0:
case 0xDDF0:
case 0xDFF0:
case 0xD1F1:
case 0xD3F1:
case 0xD5F1:
case 0xD7F1:
case 0xD9F1:
case 0xDBF1:
case 0xDDF1:
case 0xDFF1:
case 0xD1F2:
case 0xD3F2:
case 0xD5F2:
case 0xD7F2:
case 0xD9F2:
case 0xDBF2:
case 0xDDF2:
case 0xDFF2:
case 0xD1F3:
case 0xD3F3:
case 0xD5F3:
case 0xD7F3:
case 0xD9F3:
case 0xDBF3:
case 0xDDF3:
case 0xDFF3:
case 0xD1F4:
case 0xD3F4:
case 0xD5F4:
case 0xD7F4:
case 0xD9F4:
case 0xDBF4:
case 0xDDF4:
case 0xDFF4:
case 0xD1F5:
case 0xD3F5:
case 0xD5F5:
case 0xD7F5:
case 0xD9F5:
case 0xDBF5:
case 0xDDF5:
case 0xDFF5:
case 0xD1F6:
case 0xD3F6:
case 0xD5F6:
case 0xD7F6:
case 0xD9F6:
case 0xDBF6:
case 0xDDF6:
case 0xDFF6:
case 0xD1F7:
case 0xD3F7:
case 0xD5F7:
case 0xD7F7:
case 0xD9F7:
case 0xDBF7:
case 0xDDF7:
case 0xDFF7:

// ADDA
case 0xD1F0:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(20)
case 0xD3F8:
case 0xD5F8:
case 0xD7F8:
case 0xD9F8:
case 0xDBF8:
case 0xDDF8:
case 0xDFF8:

// ADDA
case 0xD1F8:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(18)
case 0xD3F9:
case 0xD5F9:
case 0xD7F9:
case 0xD9F9:
case 0xDBF9:
case 0xDDF9:
case 0xDFF9:

// ADDA
case 0xD1F9:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(22)
case 0xD3FA:
case 0xD5FA:
case 0xD7FA:
case 0xD9FA:
case 0xDBFA:
case 0xDDFA:
case 0xDFFA:

// ADDA
case 0xD1FA:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(18)
case 0xD3FB:
case 0xD5FB:
case 0xD7FB:
case 0xD9FB:
case 0xDBFB:
case 0xDDFB:
case 0xDFFB:

// ADDA
case 0xD1FB:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(20)
case 0xD3FC:
case 0xD5FC:
case 0xD7FC:
case 0xD9FC:
case 0xDBFC:
case 0xDDFC:
case 0xDFFC:

// ADDA
case 0xD1FC:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s32)FETCH_LONG;
	PC += 4;
	dst = (u32)CPU->A[(Opcode >> 9) & 7];
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(14)
case 0xD3DF:
case 0xD5DF:
case 0xD7DF:
case 0xD9DF:
case 0xDBDF:
case 0xDDDF:
case 0xDFDF:

// ADDA
case 0xD1DF:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0xD3E7:
case 0xD5E7:
case 0xD7E7:
case 0xD9E7:
case 0xDBE7:
case 0xDDE7:
case 0xDFE7:

// ADDA
case 0xD1E7:
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
	res = dst + src;
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(16)
