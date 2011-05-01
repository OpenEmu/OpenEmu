case 0xE200:
case 0xE400:
case 0xE600:
case 0xE800:
case 0xEA00:
case 0xEC00:
case 0xEE00:
case 0xE001:
case 0xE201:
case 0xE401:
case 0xE601:
case 0xE801:
case 0xEA01:
case 0xEC01:
case 0xEE01:
case 0xE002:
case 0xE202:
case 0xE402:
case 0xE602:
case 0xE802:
case 0xEA02:
case 0xEC02:
case 0xEE02:
case 0xE003:
case 0xE203:
case 0xE403:
case 0xE603:
case 0xE803:
case 0xEA03:
case 0xEC03:
case 0xEE03:
case 0xE004:
case 0xE204:
case 0xE404:
case 0xE604:
case 0xE804:
case 0xEA04:
case 0xEC04:
case 0xEE04:
case 0xE005:
case 0xE205:
case 0xE405:
case 0xE605:
case 0xE805:
case 0xEA05:
case 0xEC05:
case 0xEE05:
case 0xE006:
case 0xE206:
case 0xE406:
case 0xE606:
case 0xE806:
case 0xEA06:
case 0xEC06:
case 0xEE06:
case 0xE007:
case 0xE207:
case 0xE407:
case 0xE607:
case 0xE807:
case 0xEA07:
case 0xEC07:
case 0xEE07:

// ASRk
case 0xE000:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (s32)(s8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);
	res = ((s32)src) >> sft;
	CPU->flag_N = res >> 0;
	CPU->flag_notZ = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xE240:
case 0xE440:
case 0xE640:
case 0xE840:
case 0xEA40:
case 0xEC40:
case 0xEE40:
case 0xE041:
case 0xE241:
case 0xE441:
case 0xE641:
case 0xE841:
case 0xEA41:
case 0xEC41:
case 0xEE41:
case 0xE042:
case 0xE242:
case 0xE442:
case 0xE642:
case 0xE842:
case 0xEA42:
case 0xEC42:
case 0xEE42:
case 0xE043:
case 0xE243:
case 0xE443:
case 0xE643:
case 0xE843:
case 0xEA43:
case 0xEC43:
case 0xEE43:
case 0xE044:
case 0xE244:
case 0xE444:
case 0xE644:
case 0xE844:
case 0xEA44:
case 0xEC44:
case 0xEE44:
case 0xE045:
case 0xE245:
case 0xE445:
case 0xE645:
case 0xE845:
case 0xEA45:
case 0xEC45:
case 0xEE45:
case 0xE046:
case 0xE246:
case 0xE446:
case 0xE646:
case 0xE846:
case 0xEA46:
case 0xEC46:
case 0xEE46:
case 0xE047:
case 0xE247:
case 0xE447:
case 0xE647:
case 0xE847:
case 0xEA47:
case 0xEC47:
case 0xEE47:

// ASRk
case 0xE040:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (s32)(s16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);
	res = ((s32)src) >> sft;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xE280:
case 0xE480:
case 0xE680:
case 0xE880:
case 0xEA80:
case 0xEC80:
case 0xEE80:
case 0xE081:
case 0xE281:
case 0xE481:
case 0xE681:
case 0xE881:
case 0xEA81:
case 0xEC81:
case 0xEE81:
case 0xE082:
case 0xE282:
case 0xE482:
case 0xE682:
case 0xE882:
case 0xEA82:
case 0xEC82:
case 0xEE82:
case 0xE083:
case 0xE283:
case 0xE483:
case 0xE683:
case 0xE883:
case 0xEA83:
case 0xEC83:
case 0xEE83:
case 0xE084:
case 0xE284:
case 0xE484:
case 0xE684:
case 0xE884:
case 0xEA84:
case 0xEC84:
case 0xEE84:
case 0xE085:
case 0xE285:
case 0xE485:
case 0xE685:
case 0xE885:
case 0xEA85:
case 0xEC85:
case 0xEE85:
case 0xE086:
case 0xE286:
case 0xE486:
case 0xE686:
case 0xE886:
case 0xEA86:
case 0xEC86:
case 0xEE86:
case 0xE087:
case 0xE287:
case 0xE487:
case 0xE687:
case 0xE887:
case 0xEA87:
case 0xEC87:
case 0xEE87:

// ASRk
case 0xE080:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (s32)(s32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);
	res = ((s32)src) >> sft;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0xE208:
case 0xE408:
case 0xE608:
case 0xE808:
case 0xEA08:
case 0xEC08:
case 0xEE08:
case 0xE009:
case 0xE209:
case 0xE409:
case 0xE609:
case 0xE809:
case 0xEA09:
case 0xEC09:
case 0xEE09:
case 0xE00A:
case 0xE20A:
case 0xE40A:
case 0xE60A:
case 0xE80A:
case 0xEA0A:
case 0xEC0A:
case 0xEE0A:
case 0xE00B:
case 0xE20B:
case 0xE40B:
case 0xE60B:
case 0xE80B:
case 0xEA0B:
case 0xEC0B:
case 0xEE0B:
case 0xE00C:
case 0xE20C:
case 0xE40C:
case 0xE60C:
case 0xE80C:
case 0xEA0C:
case 0xEC0C:
case 0xEE0C:
case 0xE00D:
case 0xE20D:
case 0xE40D:
case 0xE60D:
case 0xE80D:
case 0xEA0D:
case 0xEC0D:
case 0xEE0D:
case 0xE00E:
case 0xE20E:
case 0xE40E:
case 0xE60E:
case 0xE80E:
case 0xEA0E:
case 0xEC0E:
case 0xEE0E:
case 0xE00F:
case 0xE20F:
case 0xE40F:
case 0xE60F:
case 0xE80F:
case 0xEA0F:
case 0xEC0F:
case 0xEE0F:

// LSRk
case 0xE008:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_N = CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);
	res = src >> sft;
	CPU->flag_notZ = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xE248:
case 0xE448:
case 0xE648:
case 0xE848:
case 0xEA48:
case 0xEC48:
case 0xEE48:
case 0xE049:
case 0xE249:
case 0xE449:
case 0xE649:
case 0xE849:
case 0xEA49:
case 0xEC49:
case 0xEE49:
case 0xE04A:
case 0xE24A:
case 0xE44A:
case 0xE64A:
case 0xE84A:
case 0xEA4A:
case 0xEC4A:
case 0xEE4A:
case 0xE04B:
case 0xE24B:
case 0xE44B:
case 0xE64B:
case 0xE84B:
case 0xEA4B:
case 0xEC4B:
case 0xEE4B:
case 0xE04C:
case 0xE24C:
case 0xE44C:
case 0xE64C:
case 0xE84C:
case 0xEA4C:
case 0xEC4C:
case 0xEE4C:
case 0xE04D:
case 0xE24D:
case 0xE44D:
case 0xE64D:
case 0xE84D:
case 0xEA4D:
case 0xEC4D:
case 0xEE4D:
case 0xE04E:
case 0xE24E:
case 0xE44E:
case 0xE64E:
case 0xE84E:
case 0xEA4E:
case 0xEC4E:
case 0xEE4E:
case 0xE04F:
case 0xE24F:
case 0xE44F:
case 0xE64F:
case 0xE84F:
case 0xEA4F:
case 0xEC4F:
case 0xEE4F:

// LSRk
case 0xE048:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_N = CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);
	res = src >> sft;
	CPU->flag_notZ = res;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xE288:
case 0xE488:
case 0xE688:
case 0xE888:
case 0xEA88:
case 0xEC88:
case 0xEE88:
case 0xE089:
case 0xE289:
case 0xE489:
case 0xE689:
case 0xE889:
case 0xEA89:
case 0xEC89:
case 0xEE89:
case 0xE08A:
case 0xE28A:
case 0xE48A:
case 0xE68A:
case 0xE88A:
case 0xEA8A:
case 0xEC8A:
case 0xEE8A:
case 0xE08B:
case 0xE28B:
case 0xE48B:
case 0xE68B:
case 0xE88B:
case 0xEA8B:
case 0xEC8B:
case 0xEE8B:
case 0xE08C:
case 0xE28C:
case 0xE48C:
case 0xE68C:
case 0xE88C:
case 0xEA8C:
case 0xEC8C:
case 0xEE8C:
case 0xE08D:
case 0xE28D:
case 0xE48D:
case 0xE68D:
case 0xE88D:
case 0xEA8D:
case 0xEC8D:
case 0xEE8D:
case 0xE08E:
case 0xE28E:
case 0xE48E:
case 0xE68E:
case 0xE88E:
case 0xEA8E:
case 0xEC8E:
case 0xEE8E:
case 0xE08F:
case 0xE28F:
case 0xE48F:
case 0xE68F:
case 0xE88F:
case 0xEA8F:
case 0xEC8F:
case 0xEE8F:

// LSRk
case 0xE088:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_N = CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);
	res = src >> sft;
	CPU->flag_notZ = res;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0xE210:
case 0xE410:
case 0xE610:
case 0xE810:
case 0xEA10:
case 0xEC10:
case 0xEE10:
case 0xE011:
case 0xE211:
case 0xE411:
case 0xE611:
case 0xE811:
case 0xEA11:
case 0xEC11:
case 0xEE11:
case 0xE012:
case 0xE212:
case 0xE412:
case 0xE612:
case 0xE812:
case 0xEA12:
case 0xEC12:
case 0xEE12:
case 0xE013:
case 0xE213:
case 0xE413:
case 0xE613:
case 0xE813:
case 0xEA13:
case 0xEC13:
case 0xEE13:
case 0xE014:
case 0xE214:
case 0xE414:
case 0xE614:
case 0xE814:
case 0xEA14:
case 0xEC14:
case 0xEE14:
case 0xE015:
case 0xE215:
case 0xE415:
case 0xE615:
case 0xE815:
case 0xEA15:
case 0xEC15:
case 0xEE15:
case 0xE016:
case 0xE216:
case 0xE416:
case 0xE616:
case 0xE816:
case 0xEA16:
case 0xEC16:
case 0xEE16:
case 0xE017:
case 0xE217:
case 0xE417:
case 0xE617:
case 0xE817:
case 0xEA17:
case 0xEC17:
case 0xEE17:

// ROXRk
case 0xE010:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	src |= (CPU->flag_X & C68K_SR_X) << 0;
	res = (src >> sft) | (src << (9 - sft));
	CPU->flag_X = CPU->flag_C = res >> 0;
	CPU->flag_V = 0;
	CPU->flag_N = res >> 0;
	CPU->flag_notZ = res & 0x000000FF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xE250:
case 0xE450:
case 0xE650:
case 0xE850:
case 0xEA50:
case 0xEC50:
case 0xEE50:
case 0xE051:
case 0xE251:
case 0xE451:
case 0xE651:
case 0xE851:
case 0xEA51:
case 0xEC51:
case 0xEE51:
case 0xE052:
case 0xE252:
case 0xE452:
case 0xE652:
case 0xE852:
case 0xEA52:
case 0xEC52:
case 0xEE52:
case 0xE053:
case 0xE253:
case 0xE453:
case 0xE653:
case 0xE853:
case 0xEA53:
case 0xEC53:
case 0xEE53:
case 0xE054:
case 0xE254:
case 0xE454:
case 0xE654:
case 0xE854:
case 0xEA54:
case 0xEC54:
case 0xEE54:
case 0xE055:
case 0xE255:
case 0xE455:
case 0xE655:
case 0xE855:
case 0xEA55:
case 0xEC55:
case 0xEE55:
case 0xE056:
case 0xE256:
case 0xE456:
case 0xE656:
case 0xE856:
case 0xEA56:
case 0xEC56:
case 0xEE56:
case 0xE057:
case 0xE257:
case 0xE457:
case 0xE657:
case 0xE857:
case 0xEA57:
case 0xEC57:
case 0xEE57:

// ROXRk
case 0xE050:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	src |= (CPU->flag_X & C68K_SR_X) << 8;
	res = (src >> sft) | (src << (17 - sft));
	CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_V = 0;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xE290:
case 0xE490:
case 0xE690:
case 0xE890:
case 0xEA90:
case 0xEC90:
case 0xEE90:
case 0xE091:
case 0xE291:
case 0xE491:
case 0xE691:
case 0xE891:
case 0xEA91:
case 0xEC91:
case 0xEE91:
case 0xE092:
case 0xE292:
case 0xE492:
case 0xE692:
case 0xE892:
case 0xEA92:
case 0xEC92:
case 0xEE92:
case 0xE093:
case 0xE293:
case 0xE493:
case 0xE693:
case 0xE893:
case 0xEA93:
case 0xEC93:
case 0xEE93:
case 0xE094:
case 0xE294:
case 0xE494:
case 0xE694:
case 0xE894:
case 0xEA94:
case 0xEC94:
case 0xEE94:
case 0xE095:
case 0xE295:
case 0xE495:
case 0xE695:
case 0xE895:
case 0xEA95:
case 0xEC95:
case 0xEE95:
case 0xE096:
case 0xE296:
case 0xE496:
case 0xE696:
case 0xE896:
case 0xEA96:
case 0xEC96:
case 0xEE96:
case 0xE097:
case 0xE297:
case 0xE497:
case 0xE697:
case 0xE897:
case 0xEA97:
case 0xEC97:
case 0xEE97:

// ROXRk
case 0xE090:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);
	if (sft == 1) res = (src >> 1) | ((CPU->flag_X & C68K_SR_X) << (32 - (C68K_SR_X_SFT + 1)));
	else res = (src >> sft) | (src << (33 - sft)) | ((CPU->flag_X & C68K_SR_X) << (32 - (C68K_SR_X_SFT + sft)));
	CPU->flag_X = CPU->flag_C;
	CPU->flag_V = 0;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0xE218:
case 0xE418:
case 0xE618:
case 0xE818:
case 0xEA18:
case 0xEC18:
case 0xEE18:
case 0xE019:
case 0xE219:
case 0xE419:
case 0xE619:
case 0xE819:
case 0xEA19:
case 0xEC19:
case 0xEE19:
case 0xE01A:
case 0xE21A:
case 0xE41A:
case 0xE61A:
case 0xE81A:
case 0xEA1A:
case 0xEC1A:
case 0xEE1A:
case 0xE01B:
case 0xE21B:
case 0xE41B:
case 0xE61B:
case 0xE81B:
case 0xEA1B:
case 0xEC1B:
case 0xEE1B:
case 0xE01C:
case 0xE21C:
case 0xE41C:
case 0xE61C:
case 0xE81C:
case 0xEA1C:
case 0xEC1C:
case 0xEE1C:
case 0xE01D:
case 0xE21D:
case 0xE41D:
case 0xE61D:
case 0xE81D:
case 0xEA1D:
case 0xEC1D:
case 0xEE1D:
case 0xE01E:
case 0xE21E:
case 0xE41E:
case 0xE61E:
case 0xE81E:
case 0xEA1E:
case 0xEC1E:
case 0xEE1E:
case 0xE01F:
case 0xE21F:
case 0xE41F:
case 0xE61F:
case 0xE81F:
case 0xEA1F:
case 0xEC1F:
case 0xEE1F:

// RORk
case 0xE018:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_V = 0;
	CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);
	res = (src >> sft) | (src << (8 - sft));
	CPU->flag_N = res >> 0;
	CPU->flag_notZ = res & 0x000000FF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xE258:
case 0xE458:
case 0xE658:
case 0xE858:
case 0xEA58:
case 0xEC58:
case 0xEE58:
case 0xE059:
case 0xE259:
case 0xE459:
case 0xE659:
case 0xE859:
case 0xEA59:
case 0xEC59:
case 0xEE59:
case 0xE05A:
case 0xE25A:
case 0xE45A:
case 0xE65A:
case 0xE85A:
case 0xEA5A:
case 0xEC5A:
case 0xEE5A:
case 0xE05B:
case 0xE25B:
case 0xE45B:
case 0xE65B:
case 0xE85B:
case 0xEA5B:
case 0xEC5B:
case 0xEE5B:
case 0xE05C:
case 0xE25C:
case 0xE45C:
case 0xE65C:
case 0xE85C:
case 0xEA5C:
case 0xEC5C:
case 0xEE5C:
case 0xE05D:
case 0xE25D:
case 0xE45D:
case 0xE65D:
case 0xE85D:
case 0xEA5D:
case 0xEC5D:
case 0xEE5D:
case 0xE05E:
case 0xE25E:
case 0xE45E:
case 0xE65E:
case 0xE85E:
case 0xEA5E:
case 0xEC5E:
case 0xEE5E:
case 0xE05F:
case 0xE25F:
case 0xE45F:
case 0xE65F:
case 0xE85F:
case 0xEA5F:
case 0xEC5F:
case 0xEE5F:

// RORk
case 0xE058:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_V = 0;
	CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);
	res = (src >> sft) | (src << (16 - sft));
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xE298:
case 0xE498:
case 0xE698:
case 0xE898:
case 0xEA98:
case 0xEC98:
case 0xEE98:
case 0xE099:
case 0xE299:
case 0xE499:
case 0xE699:
case 0xE899:
case 0xEA99:
case 0xEC99:
case 0xEE99:
case 0xE09A:
case 0xE29A:
case 0xE49A:
case 0xE69A:
case 0xE89A:
case 0xEA9A:
case 0xEC9A:
case 0xEE9A:
case 0xE09B:
case 0xE29B:
case 0xE49B:
case 0xE69B:
case 0xE89B:
case 0xEA9B:
case 0xEC9B:
case 0xEE9B:
case 0xE09C:
case 0xE29C:
case 0xE49C:
case 0xE69C:
case 0xE89C:
case 0xEA9C:
case 0xEC9C:
case 0xEE9C:
case 0xE09D:
case 0xE29D:
case 0xE49D:
case 0xE69D:
case 0xE89D:
case 0xEA9D:
case 0xEC9D:
case 0xEE9D:
case 0xE09E:
case 0xE29E:
case 0xE49E:
case 0xE69E:
case 0xE89E:
case 0xEA9E:
case 0xEC9E:
case 0xEE9E:
case 0xE09F:
case 0xE29F:
case 0xE49F:
case 0xE69F:
case 0xE89F:
case 0xEA9F:
case 0xEC9F:
case 0xEE9F:

// RORk
case 0xE098:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_V = 0;
	CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);
	res = (src >> sft) | (src << (32 - sft));
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0xE300:
case 0xE500:
case 0xE700:
case 0xE900:
case 0xEB00:
case 0xED00:
case 0xEF00:
case 0xE101:
case 0xE301:
case 0xE501:
case 0xE701:
case 0xE901:
case 0xEB01:
case 0xED01:
case 0xEF01:
case 0xE102:
case 0xE302:
case 0xE502:
case 0xE702:
case 0xE902:
case 0xEB02:
case 0xED02:
case 0xEF02:
case 0xE103:
case 0xE303:
case 0xE503:
case 0xE703:
case 0xE903:
case 0xEB03:
case 0xED03:
case 0xEF03:
case 0xE104:
case 0xE304:
case 0xE504:
case 0xE704:
case 0xE904:
case 0xEB04:
case 0xED04:
case 0xEF04:
case 0xE105:
case 0xE305:
case 0xE505:
case 0xE705:
case 0xE905:
case 0xEB05:
case 0xED05:
case 0xEF05:
case 0xE106:
case 0xE306:
case 0xE506:
case 0xE706:
case 0xE906:
case 0xEB06:
case 0xED06:
case 0xEF06:
case 0xE107:
case 0xE307:
case 0xE507:
case 0xE707:
case 0xE907:
case 0xEB07:
case 0xED07:
case 0xEF07:

// ASLk
case 0xE100:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	if (sft < 8)
	{
		CPU->flag_X = CPU->flag_C = src << (0 + sft);
		res = src << sft;
		CPU->flag_N = res >> 0;
		CPU->flag_notZ = res & 0x000000FF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
		CPU->flag_V = 0;
		if ((sft > 7) && (src)) CPU->flag_V = C68K_SR_V;
		else
		{
			u32 msk = (((s32)0x80000000) >> (sft + 24)) & 0x000000FF;
			src &= msk;
			if ((src) && (src != msk)) CPU->flag_V = C68K_SR_V;
		}
	RET(6)
	}

	if (src) CPU->flag_V = C68K_SR_V;
	else CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	CPU->flag_N = 0;
	CPU->flag_notZ = 0;
}
RET(6)
case 0xE340:
case 0xE540:
case 0xE740:
case 0xE940:
case 0xEB40:
case 0xED40:
case 0xEF40:
case 0xE141:
case 0xE341:
case 0xE541:
case 0xE741:
case 0xE941:
case 0xEB41:
case 0xED41:
case 0xEF41:
case 0xE142:
case 0xE342:
case 0xE542:
case 0xE742:
case 0xE942:
case 0xEB42:
case 0xED42:
case 0xEF42:
case 0xE143:
case 0xE343:
case 0xE543:
case 0xE743:
case 0xE943:
case 0xEB43:
case 0xED43:
case 0xEF43:
case 0xE144:
case 0xE344:
case 0xE544:
case 0xE744:
case 0xE944:
case 0xEB44:
case 0xED44:
case 0xEF44:
case 0xE145:
case 0xE345:
case 0xE545:
case 0xE745:
case 0xE945:
case 0xEB45:
case 0xED45:
case 0xEF45:
case 0xE146:
case 0xE346:
case 0xE546:
case 0xE746:
case 0xE946:
case 0xEB46:
case 0xED46:
case 0xEF46:
case 0xE147:
case 0xE347:
case 0xE547:
case 0xE747:
case 0xE947:
case 0xEB47:
case 0xED47:
case 0xEF47:

// ASLk
case 0xE140:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
		CPU->flag_X = CPU->flag_C = src >> (8 - sft);
		res = src << sft;
		CPU->flag_N = res >> 8;
		CPU->flag_notZ = res & 0x0000FFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
		CPU->flag_V = 0;
		{
			u32 msk = (((s32)0x80000000) >> (sft + 16)) & 0x0000FFFF;
			src &= msk;
			if ((src) && (src != msk)) CPU->flag_V = C68K_SR_V;
		}
}
RET(6)
case 0xE380:
case 0xE580:
case 0xE780:
case 0xE980:
case 0xEB80:
case 0xED80:
case 0xEF80:
case 0xE181:
case 0xE381:
case 0xE581:
case 0xE781:
case 0xE981:
case 0xEB81:
case 0xED81:
case 0xEF81:
case 0xE182:
case 0xE382:
case 0xE582:
case 0xE782:
case 0xE982:
case 0xEB82:
case 0xED82:
case 0xEF82:
case 0xE183:
case 0xE383:
case 0xE583:
case 0xE783:
case 0xE983:
case 0xEB83:
case 0xED83:
case 0xEF83:
case 0xE184:
case 0xE384:
case 0xE584:
case 0xE784:
case 0xE984:
case 0xEB84:
case 0xED84:
case 0xEF84:
case 0xE185:
case 0xE385:
case 0xE585:
case 0xE785:
case 0xE985:
case 0xEB85:
case 0xED85:
case 0xEF85:
case 0xE186:
case 0xE386:
case 0xE586:
case 0xE786:
case 0xE986:
case 0xEB86:
case 0xED86:
case 0xEF86:
case 0xE187:
case 0xE387:
case 0xE587:
case 0xE787:
case 0xE987:
case 0xEB87:
case 0xED87:
case 0xEF87:

// ASLk
case 0xE180:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
		CPU->flag_X = CPU->flag_C = src >> (24 - sft);
		res = src << sft;
		CPU->flag_N = res >> 24;
		CPU->flag_notZ = res & 0xFFFFFFFF;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
		CPU->flag_V = 0;
		{
			u32 msk = (((s32)0x80000000) >> (sft + 0)) & 0xFFFFFFFF;
			src &= msk;
			if ((src) && (src != msk)) CPU->flag_V = C68K_SR_V;
		}
}
RET(8)
case 0xE308:
case 0xE508:
case 0xE708:
case 0xE908:
case 0xEB08:
case 0xED08:
case 0xEF08:
case 0xE109:
case 0xE309:
case 0xE509:
case 0xE709:
case 0xE909:
case 0xEB09:
case 0xED09:
case 0xEF09:
case 0xE10A:
case 0xE30A:
case 0xE50A:
case 0xE70A:
case 0xE90A:
case 0xEB0A:
case 0xED0A:
case 0xEF0A:
case 0xE10B:
case 0xE30B:
case 0xE50B:
case 0xE70B:
case 0xE90B:
case 0xEB0B:
case 0xED0B:
case 0xEF0B:
case 0xE10C:
case 0xE30C:
case 0xE50C:
case 0xE70C:
case 0xE90C:
case 0xEB0C:
case 0xED0C:
case 0xEF0C:
case 0xE10D:
case 0xE30D:
case 0xE50D:
case 0xE70D:
case 0xE90D:
case 0xEB0D:
case 0xED0D:
case 0xEF0D:
case 0xE10E:
case 0xE30E:
case 0xE50E:
case 0xE70E:
case 0xE90E:
case 0xEB0E:
case 0xED0E:
case 0xEF0E:
case 0xE10F:
case 0xE30F:
case 0xE50F:
case 0xE70F:
case 0xE90F:
case 0xEB0F:
case 0xED0F:
case 0xEF0F:

// LSLk
case 0xE108:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << (0 + sft);
	res = src << sft;
	CPU->flag_N = res >> 0;
	CPU->flag_notZ = res & 0x000000FF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xE348:
case 0xE548:
case 0xE748:
case 0xE948:
case 0xEB48:
case 0xED48:
case 0xEF48:
case 0xE149:
case 0xE349:
case 0xE549:
case 0xE749:
case 0xE949:
case 0xEB49:
case 0xED49:
case 0xEF49:
case 0xE14A:
case 0xE34A:
case 0xE54A:
case 0xE74A:
case 0xE94A:
case 0xEB4A:
case 0xED4A:
case 0xEF4A:
case 0xE14B:
case 0xE34B:
case 0xE54B:
case 0xE74B:
case 0xE94B:
case 0xEB4B:
case 0xED4B:
case 0xEF4B:
case 0xE14C:
case 0xE34C:
case 0xE54C:
case 0xE74C:
case 0xE94C:
case 0xEB4C:
case 0xED4C:
case 0xEF4C:
case 0xE14D:
case 0xE34D:
case 0xE54D:
case 0xE74D:
case 0xE94D:
case 0xEB4D:
case 0xED4D:
case 0xEF4D:
case 0xE14E:
case 0xE34E:
case 0xE54E:
case 0xE74E:
case 0xE94E:
case 0xEB4E:
case 0xED4E:
case 0xEF4E:
case 0xE14F:
case 0xE34F:
case 0xE54F:
case 0xE74F:
case 0xE94F:
case 0xEB4F:
case 0xED4F:
case 0xEF4F:

// LSLk
case 0xE148:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src >> (8 - sft);
	res = src << sft;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xE388:
case 0xE588:
case 0xE788:
case 0xE988:
case 0xEB88:
case 0xED88:
case 0xEF88:
case 0xE189:
case 0xE389:
case 0xE589:
case 0xE789:
case 0xE989:
case 0xEB89:
case 0xED89:
case 0xEF89:
case 0xE18A:
case 0xE38A:
case 0xE58A:
case 0xE78A:
case 0xE98A:
case 0xEB8A:
case 0xED8A:
case 0xEF8A:
case 0xE18B:
case 0xE38B:
case 0xE58B:
case 0xE78B:
case 0xE98B:
case 0xEB8B:
case 0xED8B:
case 0xEF8B:
case 0xE18C:
case 0xE38C:
case 0xE58C:
case 0xE78C:
case 0xE98C:
case 0xEB8C:
case 0xED8C:
case 0xEF8C:
case 0xE18D:
case 0xE38D:
case 0xE58D:
case 0xE78D:
case 0xE98D:
case 0xEB8D:
case 0xED8D:
case 0xEF8D:
case 0xE18E:
case 0xE38E:
case 0xE58E:
case 0xE78E:
case 0xE98E:
case 0xEB8E:
case 0xED8E:
case 0xEF8E:
case 0xE18F:
case 0xE38F:
case 0xE58F:
case 0xE78F:
case 0xE98F:
case 0xEB8F:
case 0xED8F:
case 0xEF8F:

// LSLk
case 0xE188:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src >> (24 - sft);
	res = src << sft;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res & 0xFFFFFFFF;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0xE310:
case 0xE510:
case 0xE710:
case 0xE910:
case 0xEB10:
case 0xED10:
case 0xEF10:
case 0xE111:
case 0xE311:
case 0xE511:
case 0xE711:
case 0xE911:
case 0xEB11:
case 0xED11:
case 0xEF11:
case 0xE112:
case 0xE312:
case 0xE512:
case 0xE712:
case 0xE912:
case 0xEB12:
case 0xED12:
case 0xEF12:
case 0xE113:
case 0xE313:
case 0xE513:
case 0xE713:
case 0xE913:
case 0xEB13:
case 0xED13:
case 0xEF13:
case 0xE114:
case 0xE314:
case 0xE514:
case 0xE714:
case 0xE914:
case 0xEB14:
case 0xED14:
case 0xEF14:
case 0xE115:
case 0xE315:
case 0xE515:
case 0xE715:
case 0xE915:
case 0xEB15:
case 0xED15:
case 0xEF15:
case 0xE116:
case 0xE316:
case 0xE516:
case 0xE716:
case 0xE916:
case 0xEB16:
case 0xED16:
case 0xEF16:
case 0xE117:
case 0xE317:
case 0xE517:
case 0xE717:
case 0xE917:
case 0xEB17:
case 0xED17:
case 0xEF17:

// ROXLk
case 0xE110:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	src |= (CPU->flag_X & C68K_SR_X) << 0;
	res = (src << sft) | (src >> (9 - sft));
	CPU->flag_X = CPU->flag_C = res >> 0;
	CPU->flag_V = 0;
	CPU->flag_N = res >> 0;
	CPU->flag_notZ = res & 0x000000FF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xE350:
case 0xE550:
case 0xE750:
case 0xE950:
case 0xEB50:
case 0xED50:
case 0xEF50:
case 0xE151:
case 0xE351:
case 0xE551:
case 0xE751:
case 0xE951:
case 0xEB51:
case 0xED51:
case 0xEF51:
case 0xE152:
case 0xE352:
case 0xE552:
case 0xE752:
case 0xE952:
case 0xEB52:
case 0xED52:
case 0xEF52:
case 0xE153:
case 0xE353:
case 0xE553:
case 0xE753:
case 0xE953:
case 0xEB53:
case 0xED53:
case 0xEF53:
case 0xE154:
case 0xE354:
case 0xE554:
case 0xE754:
case 0xE954:
case 0xEB54:
case 0xED54:
case 0xEF54:
case 0xE155:
case 0xE355:
case 0xE555:
case 0xE755:
case 0xE955:
case 0xEB55:
case 0xED55:
case 0xEF55:
case 0xE156:
case 0xE356:
case 0xE556:
case 0xE756:
case 0xE956:
case 0xEB56:
case 0xED56:
case 0xEF56:
case 0xE157:
case 0xE357:
case 0xE557:
case 0xE757:
case 0xE957:
case 0xEB57:
case 0xED57:
case 0xEF57:

// ROXLk
case 0xE150:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	src |= (CPU->flag_X & C68K_SR_X) << 8;
	res = (src << sft) | (src >> (17 - sft));
	CPU->flag_X = CPU->flag_C = res >> 8;
	CPU->flag_V = 0;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xE390:
case 0xE590:
case 0xE790:
case 0xE990:
case 0xEB90:
case 0xED90:
case 0xEF90:
case 0xE191:
case 0xE391:
case 0xE591:
case 0xE791:
case 0xE991:
case 0xEB91:
case 0xED91:
case 0xEF91:
case 0xE192:
case 0xE392:
case 0xE592:
case 0xE792:
case 0xE992:
case 0xEB92:
case 0xED92:
case 0xEF92:
case 0xE193:
case 0xE393:
case 0xE593:
case 0xE793:
case 0xE993:
case 0xEB93:
case 0xED93:
case 0xEF93:
case 0xE194:
case 0xE394:
case 0xE594:
case 0xE794:
case 0xE994:
case 0xEB94:
case 0xED94:
case 0xEF94:
case 0xE195:
case 0xE395:
case 0xE595:
case 0xE795:
case 0xE995:
case 0xEB95:
case 0xED95:
case 0xEF95:
case 0xE196:
case 0xE396:
case 0xE596:
case 0xE796:
case 0xE996:
case 0xEB96:
case 0xED96:
case 0xEF96:
case 0xE197:
case 0xE397:
case 0xE597:
case 0xE797:
case 0xE997:
case 0xEB97:
case 0xED97:
case 0xEF97:

// ROXLk
case 0xE190:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = src >> ((32 - C68K_SR_C_SFT) - sft);
	if (sft == 1) res = (src << 1) | ((CPU->flag_X & C68K_SR_X) >> ((C68K_SR_X_SFT + 1) - 1));
	else res = (src << sft) | (src >> (33 - sft)) | ((CPU->flag_X & C68K_SR_X) >> ((C68K_SR_X_SFT + 1) - sft));
	CPU->flag_X = CPU->flag_C;
	CPU->flag_V = 0;
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0xE318:
case 0xE518:
case 0xE718:
case 0xE918:
case 0xEB18:
case 0xED18:
case 0xEF18:
case 0xE119:
case 0xE319:
case 0xE519:
case 0xE719:
case 0xE919:
case 0xEB19:
case 0xED19:
case 0xEF19:
case 0xE11A:
case 0xE31A:
case 0xE51A:
case 0xE71A:
case 0xE91A:
case 0xEB1A:
case 0xED1A:
case 0xEF1A:
case 0xE11B:
case 0xE31B:
case 0xE51B:
case 0xE71B:
case 0xE91B:
case 0xEB1B:
case 0xED1B:
case 0xEF1B:
case 0xE11C:
case 0xE31C:
case 0xE51C:
case 0xE71C:
case 0xE91C:
case 0xEB1C:
case 0xED1C:
case 0xEF1C:
case 0xE11D:
case 0xE31D:
case 0xE51D:
case 0xE71D:
case 0xE91D:
case 0xEB1D:
case 0xED1D:
case 0xEF1D:
case 0xE11E:
case 0xE31E:
case 0xE51E:
case 0xE71E:
case 0xE91E:
case 0xEB1E:
case 0xED1E:
case 0xEF1E:
case 0xE11F:
case 0xE31F:
case 0xE51F:
case 0xE71F:
case 0xE91F:
case 0xEB1F:
case 0xED1F:
case 0xEF1F:

// ROLk
case 0xE118:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_V = 0;
	CPU->flag_C = src << (0 + sft);
	res = (src << sft) | (src >> (8 - sft));
	CPU->flag_N = res >> 0;
	CPU->flag_notZ = res & 0x000000FF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xE358:
case 0xE558:
case 0xE758:
case 0xE958:
case 0xEB58:
case 0xED58:
case 0xEF58:
case 0xE159:
case 0xE359:
case 0xE559:
case 0xE759:
case 0xE959:
case 0xEB59:
case 0xED59:
case 0xEF59:
case 0xE15A:
case 0xE35A:
case 0xE55A:
case 0xE75A:
case 0xE95A:
case 0xEB5A:
case 0xED5A:
case 0xEF5A:
case 0xE15B:
case 0xE35B:
case 0xE55B:
case 0xE75B:
case 0xE95B:
case 0xEB5B:
case 0xED5B:
case 0xEF5B:
case 0xE15C:
case 0xE35C:
case 0xE55C:
case 0xE75C:
case 0xE95C:
case 0xEB5C:
case 0xED5C:
case 0xEF5C:
case 0xE15D:
case 0xE35D:
case 0xE55D:
case 0xE75D:
case 0xE95D:
case 0xEB5D:
case 0xED5D:
case 0xEF5D:
case 0xE15E:
case 0xE35E:
case 0xE55E:
case 0xE75E:
case 0xE95E:
case 0xEB5E:
case 0xED5E:
case 0xEF5E:
case 0xE15F:
case 0xE35F:
case 0xE55F:
case 0xE75F:
case 0xE95F:
case 0xEB5F:
case 0xED5F:
case 0xEF5F:

// ROLk
case 0xE158:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_V = 0;
	CPU->flag_C = src >> (8 - sft);
	res = (src << sft) | (src >> (16 - sft));
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(6)
case 0xE398:
case 0xE598:
case 0xE798:
case 0xE998:
case 0xEB98:
case 0xED98:
case 0xEF98:
case 0xE199:
case 0xE399:
case 0xE599:
case 0xE799:
case 0xE999:
case 0xEB99:
case 0xED99:
case 0xEF99:
case 0xE19A:
case 0xE39A:
case 0xE59A:
case 0xE79A:
case 0xE99A:
case 0xEB9A:
case 0xED9A:
case 0xEF9A:
case 0xE19B:
case 0xE39B:
case 0xE59B:
case 0xE79B:
case 0xE99B:
case 0xEB9B:
case 0xED9B:
case 0xEF9B:
case 0xE19C:
case 0xE39C:
case 0xE59C:
case 0xE79C:
case 0xE99C:
case 0xEB9C:
case 0xED9C:
case 0xEF9C:
case 0xE19D:
case 0xE39D:
case 0xE59D:
case 0xE79D:
case 0xE99D:
case 0xEB9D:
case 0xED9D:
case 0xEF9D:
case 0xE19E:
case 0xE39E:
case 0xE59E:
case 0xE79E:
case 0xE99E:
case 0xEB9E:
case 0xED9E:
case 0xEF9E:
case 0xE19F:
case 0xE39F:
case 0xE59F:
case 0xE79F:
case 0xE99F:
case 0xEB9F:
case 0xED9F:
case 0xEF9F:

// ROLk
case 0xE198:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = (((Opcode >> 9) - 1) & 7) + 1;
	CCnt -= sft * 2;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_V = 0;
	CPU->flag_C = src >> (24 - sft);
	res = (src << sft) | (src >> (32 - sft));
	CPU->flag_N = res >> 24;
	CPU->flag_notZ = res;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
}
RET(8)
case 0xE220:
case 0xE420:
case 0xE620:
case 0xE820:
case 0xEA20:
case 0xEC20:
case 0xEE20:
case 0xE021:
case 0xE221:
case 0xE421:
case 0xE621:
case 0xE821:
case 0xEA21:
case 0xEC21:
case 0xEE21:
case 0xE022:
case 0xE222:
case 0xE422:
case 0xE622:
case 0xE822:
case 0xEA22:
case 0xEC22:
case 0xEE22:
case 0xE023:
case 0xE223:
case 0xE423:
case 0xE623:
case 0xE823:
case 0xEA23:
case 0xEC23:
case 0xEE23:
case 0xE024:
case 0xE224:
case 0xE424:
case 0xE624:
case 0xE824:
case 0xEA24:
case 0xEC24:
case 0xEE24:
case 0xE025:
case 0xE225:
case 0xE425:
case 0xE625:
case 0xE825:
case 0xEA25:
case 0xEC25:
case 0xEE25:
case 0xE026:
case 0xE226:
case 0xE426:
case 0xE626:
case 0xE826:
case 0xEA26:
case 0xEC26:
case 0xEE26:
case 0xE027:
case 0xE227:
case 0xE427:
case 0xE627:
case 0xE827:
case 0xEA27:
case 0xEC27:
case 0xEE27:

// ASRD
case 0xE020:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (s32)(s8)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft < 8)
		{
			CPU->flag_V = 0;
			CPU->flag_X = CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);
			res = ((s32)src) >> sft;
			CPU->flag_N = res >> 0;
			CPU->flag_notZ = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		if (src & (1 << 7))
		{
			CPU->flag_N = C68K_SR_N;
			CPU->flag_notZ = 1;
			CPU->flag_V = 0;
			CPU->flag_C = C68K_SR_C;
			CPU->flag_X = C68K_SR_X;
			res = 0x000000FF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		CPU->flag_N = 0;
		CPU->flag_notZ = 0;
		CPU->flag_V = 0;
		CPU->flag_C = 0;
		CPU->flag_X = 0;
		res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 0;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE260:
case 0xE460:
case 0xE660:
case 0xE860:
case 0xEA60:
case 0xEC60:
case 0xEE60:
case 0xE061:
case 0xE261:
case 0xE461:
case 0xE661:
case 0xE861:
case 0xEA61:
case 0xEC61:
case 0xEE61:
case 0xE062:
case 0xE262:
case 0xE462:
case 0xE662:
case 0xE862:
case 0xEA62:
case 0xEC62:
case 0xEE62:
case 0xE063:
case 0xE263:
case 0xE463:
case 0xE663:
case 0xE863:
case 0xEA63:
case 0xEC63:
case 0xEE63:
case 0xE064:
case 0xE264:
case 0xE464:
case 0xE664:
case 0xE864:
case 0xEA64:
case 0xEC64:
case 0xEE64:
case 0xE065:
case 0xE265:
case 0xE465:
case 0xE665:
case 0xE865:
case 0xEA65:
case 0xEC65:
case 0xEE65:
case 0xE066:
case 0xE266:
case 0xE466:
case 0xE666:
case 0xE866:
case 0xEA66:
case 0xEC66:
case 0xEE66:
case 0xE067:
case 0xE267:
case 0xE467:
case 0xE667:
case 0xE867:
case 0xEA67:
case 0xEC67:
case 0xEE67:

// ASRD
case 0xE060:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (s32)(s16)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft < 16)
		{
			CPU->flag_V = 0;
			CPU->flag_X = CPU->flag_C = (src >> (sft - 1)) << C68K_SR_C_SFT;
			res = ((s32)src) >> sft;
			CPU->flag_N = res >> 8;
			CPU->flag_notZ = res;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		if (src & (1 << 15))
		{
			CPU->flag_N = C68K_SR_N;
			CPU->flag_notZ = 1;
			CPU->flag_V = 0;
			CPU->flag_C = C68K_SR_C;
			CPU->flag_X = C68K_SR_X;
			res = 0x0000FFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		CPU->flag_N = 0;
		CPU->flag_notZ = 0;
		CPU->flag_V = 0;
		CPU->flag_C = 0;
		CPU->flag_X = 0;
		res = 0;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 8;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE2A0:
case 0xE4A0:
case 0xE6A0:
case 0xE8A0:
case 0xEAA0:
case 0xECA0:
case 0xEEA0:
case 0xE0A1:
case 0xE2A1:
case 0xE4A1:
case 0xE6A1:
case 0xE8A1:
case 0xEAA1:
case 0xECA1:
case 0xEEA1:
case 0xE0A2:
case 0xE2A2:
case 0xE4A2:
case 0xE6A2:
case 0xE8A2:
case 0xEAA2:
case 0xECA2:
case 0xEEA2:
case 0xE0A3:
case 0xE2A3:
case 0xE4A3:
case 0xE6A3:
case 0xE8A3:
case 0xEAA3:
case 0xECA3:
case 0xEEA3:
case 0xE0A4:
case 0xE2A4:
case 0xE4A4:
case 0xE6A4:
case 0xE8A4:
case 0xEAA4:
case 0xECA4:
case 0xEEA4:
case 0xE0A5:
case 0xE2A5:
case 0xE4A5:
case 0xE6A5:
case 0xE8A5:
case 0xEAA5:
case 0xECA5:
case 0xEEA5:
case 0xE0A6:
case 0xE2A6:
case 0xE4A6:
case 0xE6A6:
case 0xE8A6:
case 0xEAA6:
case 0xECA6:
case 0xEEA6:
case 0xE0A7:
case 0xE2A7:
case 0xE4A7:
case 0xE6A7:
case 0xE8A7:
case 0xEAA7:
case 0xECA7:
case 0xEEA7:

// ASRD
case 0xE0A0:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (s32)(s32)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft < 32)
		{
			CPU->flag_V = 0;
			CPU->flag_X = CPU->flag_C = (src >> (sft - 1)) << C68K_SR_C_SFT;
			res = ((s32)src) >> sft;
			CPU->flag_N = res >> 24;
			CPU->flag_notZ = res;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		if (src & (1 << 31))
		{
			CPU->flag_N = C68K_SR_N;
			CPU->flag_notZ = 1;
			CPU->flag_V = 0;
			CPU->flag_C = C68K_SR_C;
			CPU->flag_X = C68K_SR_X;
			res = 0xFFFFFFFF;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		CPU->flag_N = 0;
		CPU->flag_notZ = 0;
		CPU->flag_V = 0;
		CPU->flag_C = 0;
		CPU->flag_X = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 24;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE228:
case 0xE428:
case 0xE628:
case 0xE828:
case 0xEA28:
case 0xEC28:
case 0xEE28:
case 0xE029:
case 0xE229:
case 0xE429:
case 0xE629:
case 0xE829:
case 0xEA29:
case 0xEC29:
case 0xEE29:
case 0xE02A:
case 0xE22A:
case 0xE42A:
case 0xE62A:
case 0xE82A:
case 0xEA2A:
case 0xEC2A:
case 0xEE2A:
case 0xE02B:
case 0xE22B:
case 0xE42B:
case 0xE62B:
case 0xE82B:
case 0xEA2B:
case 0xEC2B:
case 0xEE2B:
case 0xE02C:
case 0xE22C:
case 0xE42C:
case 0xE62C:
case 0xE82C:
case 0xEA2C:
case 0xEC2C:
case 0xEE2C:
case 0xE02D:
case 0xE22D:
case 0xE42D:
case 0xE62D:
case 0xE82D:
case 0xEA2D:
case 0xEC2D:
case 0xEE2D:
case 0xE02E:
case 0xE22E:
case 0xE42E:
case 0xE62E:
case 0xE82E:
case 0xEA2E:
case 0xEC2E:
case 0xEE2E:
case 0xE02F:
case 0xE22F:
case 0xE42F:
case 0xE62F:
case 0xE82F:
case 0xEA2F:
case 0xEC2F:
case 0xEE2F:

// LSRD
case 0xE028:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft <= 8)
		{
			CPU->flag_N = CPU->flag_V = 0;
			CPU->flag_X = CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);
			res = src >> sft;
			CPU->flag_notZ = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		CPU->flag_X = CPU->flag_C = 0;
		CPU->flag_N = 0;
		CPU->flag_notZ = 0;
		CPU->flag_V = 0;
		res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 0;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE268:
case 0xE468:
case 0xE668:
case 0xE868:
case 0xEA68:
case 0xEC68:
case 0xEE68:
case 0xE069:
case 0xE269:
case 0xE469:
case 0xE669:
case 0xE869:
case 0xEA69:
case 0xEC69:
case 0xEE69:
case 0xE06A:
case 0xE26A:
case 0xE46A:
case 0xE66A:
case 0xE86A:
case 0xEA6A:
case 0xEC6A:
case 0xEE6A:
case 0xE06B:
case 0xE26B:
case 0xE46B:
case 0xE66B:
case 0xE86B:
case 0xEA6B:
case 0xEC6B:
case 0xEE6B:
case 0xE06C:
case 0xE26C:
case 0xE46C:
case 0xE66C:
case 0xE86C:
case 0xEA6C:
case 0xEC6C:
case 0xEE6C:
case 0xE06D:
case 0xE26D:
case 0xE46D:
case 0xE66D:
case 0xE86D:
case 0xEA6D:
case 0xEC6D:
case 0xEE6D:
case 0xE06E:
case 0xE26E:
case 0xE46E:
case 0xE66E:
case 0xE86E:
case 0xEA6E:
case 0xEC6E:
case 0xEE6E:
case 0xE06F:
case 0xE26F:
case 0xE46F:
case 0xE66F:
case 0xE86F:
case 0xEA6F:
case 0xEC6F:
case 0xEE6F:

// LSRD
case 0xE068:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft <= 16)
		{
			CPU->flag_N = CPU->flag_V = 0;
			CPU->flag_X = CPU->flag_C = (src >> (sft - 1)) << C68K_SR_C_SFT;
			res = src >> sft;
			CPU->flag_notZ = res;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		CPU->flag_X = CPU->flag_C = 0;
		CPU->flag_N = 0;
		CPU->flag_notZ = 0;
		CPU->flag_V = 0;
		res = 0;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 8;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE2A8:
case 0xE4A8:
case 0xE6A8:
case 0xE8A8:
case 0xEAA8:
case 0xECA8:
case 0xEEA8:
case 0xE0A9:
case 0xE2A9:
case 0xE4A9:
case 0xE6A9:
case 0xE8A9:
case 0xEAA9:
case 0xECA9:
case 0xEEA9:
case 0xE0AA:
case 0xE2AA:
case 0xE4AA:
case 0xE6AA:
case 0xE8AA:
case 0xEAAA:
case 0xECAA:
case 0xEEAA:
case 0xE0AB:
case 0xE2AB:
case 0xE4AB:
case 0xE6AB:
case 0xE8AB:
case 0xEAAB:
case 0xECAB:
case 0xEEAB:
case 0xE0AC:
case 0xE2AC:
case 0xE4AC:
case 0xE6AC:
case 0xE8AC:
case 0xEAAC:
case 0xECAC:
case 0xEEAC:
case 0xE0AD:
case 0xE2AD:
case 0xE4AD:
case 0xE6AD:
case 0xE8AD:
case 0xEAAD:
case 0xECAD:
case 0xEEAD:
case 0xE0AE:
case 0xE2AE:
case 0xE4AE:
case 0xE6AE:
case 0xE8AE:
case 0xEAAE:
case 0xECAE:
case 0xEEAE:
case 0xE0AF:
case 0xE2AF:
case 0xE4AF:
case 0xE6AF:
case 0xE8AF:
case 0xEAAF:
case 0xECAF:
case 0xEEAF:

// LSRD
case 0xE0A8:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft < 32)
		{
			CPU->flag_N = CPU->flag_V = 0;
			CPU->flag_X = CPU->flag_C = (src >> (sft - 1)) << C68K_SR_C_SFT;
			res = src >> sft;
			CPU->flag_notZ = res;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		if (sft == 32) CPU->flag_C = src >> (31 - C68K_SR_C_SFT);
		else CPU->flag_C = 0;
		CPU->flag_X = CPU->flag_C;
		CPU->flag_N = 0;
		CPU->flag_notZ = 0;
		CPU->flag_V = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 24;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE230:
case 0xE430:
case 0xE630:
case 0xE830:
case 0xEA30:
case 0xEC30:
case 0xEE30:
case 0xE031:
case 0xE231:
case 0xE431:
case 0xE631:
case 0xE831:
case 0xEA31:
case 0xEC31:
case 0xEE31:
case 0xE032:
case 0xE232:
case 0xE432:
case 0xE632:
case 0xE832:
case 0xEA32:
case 0xEC32:
case 0xEE32:
case 0xE033:
case 0xE233:
case 0xE433:
case 0xE633:
case 0xE833:
case 0xEA33:
case 0xEC33:
case 0xEE33:
case 0xE034:
case 0xE234:
case 0xE434:
case 0xE634:
case 0xE834:
case 0xEA34:
case 0xEC34:
case 0xEE34:
case 0xE035:
case 0xE235:
case 0xE435:
case 0xE635:
case 0xE835:
case 0xEA35:
case 0xEC35:
case 0xEE35:
case 0xE036:
case 0xE236:
case 0xE436:
case 0xE636:
case 0xE836:
case 0xEA36:
case 0xEC36:
case 0xEE36:
case 0xE037:
case 0xE237:
case 0xE437:
case 0xE637:
case 0xE837:
case 0xEA37:
case 0xEC37:
case 0xEE37:

// ROXRD
case 0xE030:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		sft %= 9;

		src |= (CPU->flag_X & C68K_SR_X) << 0;
		res = (src >> sft) | (src << (9 - sft));
		CPU->flag_X = CPU->flag_C = res >> 0;
		CPU->flag_V = 0;
		CPU->flag_N = res >> 0;
		CPU->flag_notZ = res & 0x000000FF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = CPU->flag_X;
	CPU->flag_N = src >> 0;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE270:
case 0xE470:
case 0xE670:
case 0xE870:
case 0xEA70:
case 0xEC70:
case 0xEE70:
case 0xE071:
case 0xE271:
case 0xE471:
case 0xE671:
case 0xE871:
case 0xEA71:
case 0xEC71:
case 0xEE71:
case 0xE072:
case 0xE272:
case 0xE472:
case 0xE672:
case 0xE872:
case 0xEA72:
case 0xEC72:
case 0xEE72:
case 0xE073:
case 0xE273:
case 0xE473:
case 0xE673:
case 0xE873:
case 0xEA73:
case 0xEC73:
case 0xEE73:
case 0xE074:
case 0xE274:
case 0xE474:
case 0xE674:
case 0xE874:
case 0xEA74:
case 0xEC74:
case 0xEE74:
case 0xE075:
case 0xE275:
case 0xE475:
case 0xE675:
case 0xE875:
case 0xEA75:
case 0xEC75:
case 0xEE75:
case 0xE076:
case 0xE276:
case 0xE476:
case 0xE676:
case 0xE876:
case 0xEA76:
case 0xEC76:
case 0xEE76:
case 0xE077:
case 0xE277:
case 0xE477:
case 0xE677:
case 0xE877:
case 0xEA77:
case 0xEC77:
case 0xEE77:

// ROXRD
case 0xE070:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		sft %= 17;

		src |= (CPU->flag_X & C68K_SR_X) << 8;
		res = (src >> sft) | (src << (17 - sft));
		CPU->flag_X = CPU->flag_C = res >> 8;
		CPU->flag_V = 0;
		CPU->flag_N = res >> 8;
		CPU->flag_notZ = res & 0x0000FFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = CPU->flag_X;
	CPU->flag_N = src >> 8;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE2B0:
case 0xE4B0:
case 0xE6B0:
case 0xE8B0:
case 0xEAB0:
case 0xECB0:
case 0xEEB0:
case 0xE0B1:
case 0xE2B1:
case 0xE4B1:
case 0xE6B1:
case 0xE8B1:
case 0xEAB1:
case 0xECB1:
case 0xEEB1:
case 0xE0B2:
case 0xE2B2:
case 0xE4B2:
case 0xE6B2:
case 0xE8B2:
case 0xEAB2:
case 0xECB2:
case 0xEEB2:
case 0xE0B3:
case 0xE2B3:
case 0xE4B3:
case 0xE6B3:
case 0xE8B3:
case 0xEAB3:
case 0xECB3:
case 0xEEB3:
case 0xE0B4:
case 0xE2B4:
case 0xE4B4:
case 0xE6B4:
case 0xE8B4:
case 0xEAB4:
case 0xECB4:
case 0xEEB4:
case 0xE0B5:
case 0xE2B5:
case 0xE4B5:
case 0xE6B5:
case 0xE8B5:
case 0xEAB5:
case 0xECB5:
case 0xEEB5:
case 0xE0B6:
case 0xE2B6:
case 0xE4B6:
case 0xE6B6:
case 0xE8B6:
case 0xEAB6:
case 0xECB6:
case 0xEEB6:
case 0xE0B7:
case 0xE2B7:
case 0xE4B7:
case 0xE6B7:
case 0xE8B7:
case 0xEAB7:
case 0xECB7:
case 0xEEB7:

// ROXRD
case 0xE0B0:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		sft %= 33;

		if (sft != 0)
		{
			if (sft == 1) res = (src >> 1) | ((CPU->flag_X & C68K_SR_X) << (32 - (C68K_SR_X_SFT + 1)));
			else res = (src >> sft) | (src << (33 - sft)) | (((CPU->flag_X & C68K_SR_X) << (32 - (C68K_SR_X_SFT + 1))) >> (sft - 1));
			CPU->flag_X = (src >> (32 - sft)) << C68K_SR_X_SFT;
		}
		else res = src;
		CPU->flag_C = CPU->flag_X;
		CPU->flag_V = 0;
		CPU->flag_N = res >> 24;
		CPU->flag_notZ = res;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = CPU->flag_X;
	CPU->flag_N = src >> 24;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE238:
case 0xE438:
case 0xE638:
case 0xE838:
case 0xEA38:
case 0xEC38:
case 0xEE38:
case 0xE039:
case 0xE239:
case 0xE439:
case 0xE639:
case 0xE839:
case 0xEA39:
case 0xEC39:
case 0xEE39:
case 0xE03A:
case 0xE23A:
case 0xE43A:
case 0xE63A:
case 0xE83A:
case 0xEA3A:
case 0xEC3A:
case 0xEE3A:
case 0xE03B:
case 0xE23B:
case 0xE43B:
case 0xE63B:
case 0xE83B:
case 0xEA3B:
case 0xEC3B:
case 0xEE3B:
case 0xE03C:
case 0xE23C:
case 0xE43C:
case 0xE63C:
case 0xE83C:
case 0xEA3C:
case 0xEC3C:
case 0xEE3C:
case 0xE03D:
case 0xE23D:
case 0xE43D:
case 0xE63D:
case 0xE83D:
case 0xEA3D:
case 0xEC3D:
case 0xEE3D:
case 0xE03E:
case 0xE23E:
case 0xE43E:
case 0xE63E:
case 0xE83E:
case 0xEA3E:
case 0xEC3E:
case 0xEE3E:
case 0xE03F:
case 0xE23F:
case 0xE43F:
case 0xE63F:
case 0xE83F:
case 0xEA3F:
case 0xEC3F:
case 0xEE3F:

// RORD
case 0xE038:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		sft &= 0x07;
		
		CPU->flag_C = src << (C68K_SR_C_SFT - ((sft - 1) & 7));
		res = (src >> sft) | (src << (8 - sft));
		CPU->flag_V = 0;
		CPU->flag_N = res >> 0;
		CPU->flag_notZ = res & 0x000000FF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 0;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE278:
case 0xE478:
case 0xE678:
case 0xE878:
case 0xEA78:
case 0xEC78:
case 0xEE78:
case 0xE079:
case 0xE279:
case 0xE479:
case 0xE679:
case 0xE879:
case 0xEA79:
case 0xEC79:
case 0xEE79:
case 0xE07A:
case 0xE27A:
case 0xE47A:
case 0xE67A:
case 0xE87A:
case 0xEA7A:
case 0xEC7A:
case 0xEE7A:
case 0xE07B:
case 0xE27B:
case 0xE47B:
case 0xE67B:
case 0xE87B:
case 0xEA7B:
case 0xEC7B:
case 0xEE7B:
case 0xE07C:
case 0xE27C:
case 0xE47C:
case 0xE67C:
case 0xE87C:
case 0xEA7C:
case 0xEC7C:
case 0xEE7C:
case 0xE07D:
case 0xE27D:
case 0xE47D:
case 0xE67D:
case 0xE87D:
case 0xEA7D:
case 0xEC7D:
case 0xEE7D:
case 0xE07E:
case 0xE27E:
case 0xE47E:
case 0xE67E:
case 0xE87E:
case 0xEA7E:
case 0xEC7E:
case 0xEE7E:
case 0xE07F:
case 0xE27F:
case 0xE47F:
case 0xE67F:
case 0xE87F:
case 0xEA7F:
case 0xEC7F:
case 0xEE7F:

// RORD
case 0xE078:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		sft &= 0x0F;
		
		CPU->flag_C = (src >> ((sft - 1) & 15)) << C68K_SR_C_SFT;
		res = (src >> sft) | (src << (16 - sft));
		CPU->flag_V = 0;
		CPU->flag_N = res >> 8;
		CPU->flag_notZ = res & 0x0000FFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 8;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE2B8:
case 0xE4B8:
case 0xE6B8:
case 0xE8B8:
case 0xEAB8:
case 0xECB8:
case 0xEEB8:
case 0xE0B9:
case 0xE2B9:
case 0xE4B9:
case 0xE6B9:
case 0xE8B9:
case 0xEAB9:
case 0xECB9:
case 0xEEB9:
case 0xE0BA:
case 0xE2BA:
case 0xE4BA:
case 0xE6BA:
case 0xE8BA:
case 0xEABA:
case 0xECBA:
case 0xEEBA:
case 0xE0BB:
case 0xE2BB:
case 0xE4BB:
case 0xE6BB:
case 0xE8BB:
case 0xEABB:
case 0xECBB:
case 0xEEBB:
case 0xE0BC:
case 0xE2BC:
case 0xE4BC:
case 0xE6BC:
case 0xE8BC:
case 0xEABC:
case 0xECBC:
case 0xEEBC:
case 0xE0BD:
case 0xE2BD:
case 0xE4BD:
case 0xE6BD:
case 0xE8BD:
case 0xEABD:
case 0xECBD:
case 0xEEBD:
case 0xE0BE:
case 0xE2BE:
case 0xE4BE:
case 0xE6BE:
case 0xE8BE:
case 0xEABE:
case 0xECBE:
case 0xEEBE:
case 0xE0BF:
case 0xE2BF:
case 0xE4BF:
case 0xE6BF:
case 0xE8BF:
case 0xEABF:
case 0xECBF:
case 0xEEBF:

// RORD
case 0xE0B8:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		sft &= 0x1F;
		
		CPU->flag_C = (src >> ((sft - 1) & 31)) << C68K_SR_C_SFT;
		res = (src >> sft) | (src << (32 - sft));
		CPU->flag_V = 0;
		CPU->flag_N = res >> 24;
		CPU->flag_notZ = res;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 24;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE320:
case 0xE520:
case 0xE720:
case 0xE920:
case 0xEB20:
case 0xED20:
case 0xEF20:
case 0xE121:
case 0xE321:
case 0xE521:
case 0xE721:
case 0xE921:
case 0xEB21:
case 0xED21:
case 0xEF21:
case 0xE122:
case 0xE322:
case 0xE522:
case 0xE722:
case 0xE922:
case 0xEB22:
case 0xED22:
case 0xEF22:
case 0xE123:
case 0xE323:
case 0xE523:
case 0xE723:
case 0xE923:
case 0xEB23:
case 0xED23:
case 0xEF23:
case 0xE124:
case 0xE324:
case 0xE524:
case 0xE724:
case 0xE924:
case 0xEB24:
case 0xED24:
case 0xEF24:
case 0xE125:
case 0xE325:
case 0xE525:
case 0xE725:
case 0xE925:
case 0xEB25:
case 0xED25:
case 0xEF25:
case 0xE126:
case 0xE326:
case 0xE526:
case 0xE726:
case 0xE926:
case 0xEB26:
case 0xED26:
case 0xEF26:
case 0xE127:
case 0xE327:
case 0xE527:
case 0xE727:
case 0xE927:
case 0xEB27:
case 0xED27:
case 0xEF27:

// ASLD
case 0xE120:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft < 8)
		{
			CPU->flag_X = CPU->flag_C = (src << sft) >> 0;
			res = (src << sft) & 0x000000FF;
			CPU->flag_N = res >> 0;
			CPU->flag_notZ = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
			CPU->flag_V = 0;
			{
				u32 msk = (((s32)0x80000000) >> (sft + 24)) & 0x000000FF;
				src &= msk;
				if ((src) && (src != msk)) CPU->flag_V = C68K_SR_V;
			}
	RET(6)
		}

		if (sft == 256) CPU->flag_C = src << C68K_SR_C_SFT;
		else CPU->flag_C = 0;
		CPU->flag_X = CPU->flag_C;
		if (src) CPU->flag_V = C68K_SR_V;
		else CPU->flag_V = 0;
		res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
		CPU->flag_N = 0;
		CPU->flag_notZ = 0;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 0;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE360:
case 0xE560:
case 0xE760:
case 0xE960:
case 0xEB60:
case 0xED60:
case 0xEF60:
case 0xE161:
case 0xE361:
case 0xE561:
case 0xE761:
case 0xE961:
case 0xEB61:
case 0xED61:
case 0xEF61:
case 0xE162:
case 0xE362:
case 0xE562:
case 0xE762:
case 0xE962:
case 0xEB62:
case 0xED62:
case 0xEF62:
case 0xE163:
case 0xE363:
case 0xE563:
case 0xE763:
case 0xE963:
case 0xEB63:
case 0xED63:
case 0xEF63:
case 0xE164:
case 0xE364:
case 0xE564:
case 0xE764:
case 0xE964:
case 0xEB64:
case 0xED64:
case 0xEF64:
case 0xE165:
case 0xE365:
case 0xE565:
case 0xE765:
case 0xE965:
case 0xEB65:
case 0xED65:
case 0xEF65:
case 0xE166:
case 0xE366:
case 0xE566:
case 0xE766:
case 0xE966:
case 0xEB66:
case 0xED66:
case 0xEF66:
case 0xE167:
case 0xE367:
case 0xE567:
case 0xE767:
case 0xE967:
case 0xEB67:
case 0xED67:
case 0xEF67:

// ASLD
case 0xE160:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft < 16)
		{
			CPU->flag_X = CPU->flag_C = (src << sft) >> 8;
			res = (src << sft) & 0x0000FFFF;
			CPU->flag_N = res >> 8;
			CPU->flag_notZ = res;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
			CPU->flag_V = 0;
			{
				u32 msk = (((s32)0x80000000) >> (sft + 16)) & 0x0000FFFF;
				src &= msk;
				if ((src) && (src != msk)) CPU->flag_V = C68K_SR_V;
			}
	RET(6)
		}

		if (sft == 65536) CPU->flag_C = src << C68K_SR_C_SFT;
		else CPU->flag_C = 0;
		CPU->flag_X = CPU->flag_C;
		if (src) CPU->flag_V = C68K_SR_V;
		else CPU->flag_V = 0;
		res = 0;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
		CPU->flag_N = 0;
		CPU->flag_notZ = 0;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 8;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE3A0:
case 0xE5A0:
case 0xE7A0:
case 0xE9A0:
case 0xEBA0:
case 0xEDA0:
case 0xEFA0:
case 0xE1A1:
case 0xE3A1:
case 0xE5A1:
case 0xE7A1:
case 0xE9A1:
case 0xEBA1:
case 0xEDA1:
case 0xEFA1:
case 0xE1A2:
case 0xE3A2:
case 0xE5A2:
case 0xE7A2:
case 0xE9A2:
case 0xEBA2:
case 0xEDA2:
case 0xEFA2:
case 0xE1A3:
case 0xE3A3:
case 0xE5A3:
case 0xE7A3:
case 0xE9A3:
case 0xEBA3:
case 0xEDA3:
case 0xEFA3:
case 0xE1A4:
case 0xE3A4:
case 0xE5A4:
case 0xE7A4:
case 0xE9A4:
case 0xEBA4:
case 0xEDA4:
case 0xEFA4:
case 0xE1A5:
case 0xE3A5:
case 0xE5A5:
case 0xE7A5:
case 0xE9A5:
case 0xEBA5:
case 0xEDA5:
case 0xEFA5:
case 0xE1A6:
case 0xE3A6:
case 0xE5A6:
case 0xE7A6:
case 0xE9A6:
case 0xEBA6:
case 0xEDA6:
case 0xEFA6:
case 0xE1A7:
case 0xE3A7:
case 0xE5A7:
case 0xE7A7:
case 0xE9A7:
case 0xEBA7:
case 0xEDA7:
case 0xEFA7:

// ASLD
case 0xE1A0:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft < 32)
		{
			CPU->flag_X = CPU->flag_C = (src >> (32 - sft)) << C68K_SR_C_SFT;
			res = src << sft;
			CPU->flag_N = res >> 24;
			CPU->flag_notZ = res;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
			CPU->flag_V = 0;
			{
				u32 msk = (((s32)0x80000000) >> (sft + 0)) & 0xFFFFFFFF;
				src &= msk;
				if ((src) && (src != msk)) CPU->flag_V = C68K_SR_V;
			}
	RET(6)
		}

		if (sft == 0) CPU->flag_C = src << C68K_SR_C_SFT;
		else CPU->flag_C = 0;
		CPU->flag_X = CPU->flag_C;
		if (src) CPU->flag_V = C68K_SR_V;
		else CPU->flag_V = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
		CPU->flag_N = 0;
		CPU->flag_notZ = 0;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 24;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE328:
case 0xE528:
case 0xE728:
case 0xE928:
case 0xEB28:
case 0xED28:
case 0xEF28:
case 0xE129:
case 0xE329:
case 0xE529:
case 0xE729:
case 0xE929:
case 0xEB29:
case 0xED29:
case 0xEF29:
case 0xE12A:
case 0xE32A:
case 0xE52A:
case 0xE72A:
case 0xE92A:
case 0xEB2A:
case 0xED2A:
case 0xEF2A:
case 0xE12B:
case 0xE32B:
case 0xE52B:
case 0xE72B:
case 0xE92B:
case 0xEB2B:
case 0xED2B:
case 0xEF2B:
case 0xE12C:
case 0xE32C:
case 0xE52C:
case 0xE72C:
case 0xE92C:
case 0xEB2C:
case 0xED2C:
case 0xEF2C:
case 0xE12D:
case 0xE32D:
case 0xE52D:
case 0xE72D:
case 0xE92D:
case 0xEB2D:
case 0xED2D:
case 0xEF2D:
case 0xE12E:
case 0xE32E:
case 0xE52E:
case 0xE72E:
case 0xE92E:
case 0xEB2E:
case 0xED2E:
case 0xEF2E:
case 0xE12F:
case 0xE32F:
case 0xE52F:
case 0xE72F:
case 0xE92F:
case 0xEB2F:
case 0xED2F:
case 0xEF2F:

// LSLD
case 0xE128:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft <= 8)
		{
			CPU->flag_X = CPU->flag_C = (src << sft) >> 0;
			res = (src << sft) & 0x000000FF;
			CPU->flag_V = 0;
			CPU->flag_N = res >> 0;
			CPU->flag_notZ = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		CPU->flag_X = CPU->flag_C = 0;
		CPU->flag_N = 0;
		CPU->flag_notZ = 0;
		CPU->flag_V = 0;
		res = 0;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 0;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE368:
case 0xE568:
case 0xE768:
case 0xE968:
case 0xEB68:
case 0xED68:
case 0xEF68:
case 0xE169:
case 0xE369:
case 0xE569:
case 0xE769:
case 0xE969:
case 0xEB69:
case 0xED69:
case 0xEF69:
case 0xE16A:
case 0xE36A:
case 0xE56A:
case 0xE76A:
case 0xE96A:
case 0xEB6A:
case 0xED6A:
case 0xEF6A:
case 0xE16B:
case 0xE36B:
case 0xE56B:
case 0xE76B:
case 0xE96B:
case 0xEB6B:
case 0xED6B:
case 0xEF6B:
case 0xE16C:
case 0xE36C:
case 0xE56C:
case 0xE76C:
case 0xE96C:
case 0xEB6C:
case 0xED6C:
case 0xEF6C:
case 0xE16D:
case 0xE36D:
case 0xE56D:
case 0xE76D:
case 0xE96D:
case 0xEB6D:
case 0xED6D:
case 0xEF6D:
case 0xE16E:
case 0xE36E:
case 0xE56E:
case 0xE76E:
case 0xE96E:
case 0xEB6E:
case 0xED6E:
case 0xEF6E:
case 0xE16F:
case 0xE36F:
case 0xE56F:
case 0xE76F:
case 0xE96F:
case 0xEB6F:
case 0xED6F:
case 0xEF6F:

// LSLD
case 0xE168:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft <= 16)
		{
			CPU->flag_X = CPU->flag_C = (src << sft) >> 8;
			res = (src << sft) & 0x0000FFFF;
			CPU->flag_V = 0;
			CPU->flag_N = res >> 8;
			CPU->flag_notZ = res;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		CPU->flag_X = CPU->flag_C = 0;
		CPU->flag_N = 0;
		CPU->flag_notZ = 0;
		CPU->flag_V = 0;
		res = 0;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 8;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE3A8:
case 0xE5A8:
case 0xE7A8:
case 0xE9A8:
case 0xEBA8:
case 0xEDA8:
case 0xEFA8:
case 0xE1A9:
case 0xE3A9:
case 0xE5A9:
case 0xE7A9:
case 0xE9A9:
case 0xEBA9:
case 0xEDA9:
case 0xEFA9:
case 0xE1AA:
case 0xE3AA:
case 0xE5AA:
case 0xE7AA:
case 0xE9AA:
case 0xEBAA:
case 0xEDAA:
case 0xEFAA:
case 0xE1AB:
case 0xE3AB:
case 0xE5AB:
case 0xE7AB:
case 0xE9AB:
case 0xEBAB:
case 0xEDAB:
case 0xEFAB:
case 0xE1AC:
case 0xE3AC:
case 0xE5AC:
case 0xE7AC:
case 0xE9AC:
case 0xEBAC:
case 0xEDAC:
case 0xEFAC:
case 0xE1AD:
case 0xE3AD:
case 0xE5AD:
case 0xE7AD:
case 0xE9AD:
case 0xEBAD:
case 0xEDAD:
case 0xEFAD:
case 0xE1AE:
case 0xE3AE:
case 0xE5AE:
case 0xE7AE:
case 0xE9AE:
case 0xEBAE:
case 0xEDAE:
case 0xEFAE:
case 0xE1AF:
case 0xE3AF:
case 0xE5AF:
case 0xE7AF:
case 0xE9AF:
case 0xEBAF:
case 0xEDAF:
case 0xEFAF:

// LSLD
case 0xE1A8:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft < 32)
		{
			CPU->flag_X = CPU->flag_C = (src >> (32 - sft)) << C68K_SR_C_SFT;
			res = src << sft;
			CPU->flag_V = 0;
			CPU->flag_N = res >> 24;
			CPU->flag_notZ = res;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		if (sft == 32) CPU->flag_C = src << C68K_SR_C_SFT;
		else CPU->flag_C = 0;
		CPU->flag_X = CPU->flag_C;
		CPU->flag_N = 0;
		CPU->flag_notZ = 0;
		CPU->flag_V = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 24;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE330:
case 0xE530:
case 0xE730:
case 0xE930:
case 0xEB30:
case 0xED30:
case 0xEF30:
case 0xE131:
case 0xE331:
case 0xE531:
case 0xE731:
case 0xE931:
case 0xEB31:
case 0xED31:
case 0xEF31:
case 0xE132:
case 0xE332:
case 0xE532:
case 0xE732:
case 0xE932:
case 0xEB32:
case 0xED32:
case 0xEF32:
case 0xE133:
case 0xE333:
case 0xE533:
case 0xE733:
case 0xE933:
case 0xEB33:
case 0xED33:
case 0xEF33:
case 0xE134:
case 0xE334:
case 0xE534:
case 0xE734:
case 0xE934:
case 0xEB34:
case 0xED34:
case 0xEF34:
case 0xE135:
case 0xE335:
case 0xE535:
case 0xE735:
case 0xE935:
case 0xEB35:
case 0xED35:
case 0xEF35:
case 0xE136:
case 0xE336:
case 0xE536:
case 0xE736:
case 0xE936:
case 0xEB36:
case 0xED36:
case 0xEF36:
case 0xE137:
case 0xE337:
case 0xE537:
case 0xE737:
case 0xE937:
case 0xEB37:
case 0xED37:
case 0xEF37:

// ROXLD
case 0xE130:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		sft %= 9;

		src |= (CPU->flag_X & C68K_SR_X) << 0;
		res = (src << sft) | (src >> (9 - sft));
		CPU->flag_X = CPU->flag_C = res >> 0;
		CPU->flag_V = 0;
		CPU->flag_N = res >> 0;
		CPU->flag_notZ = res & 0x000000FF;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = CPU->flag_X;
	CPU->flag_N = src >> 0;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE370:
case 0xE570:
case 0xE770:
case 0xE970:
case 0xEB70:
case 0xED70:
case 0xEF70:
case 0xE171:
case 0xE371:
case 0xE571:
case 0xE771:
case 0xE971:
case 0xEB71:
case 0xED71:
case 0xEF71:
case 0xE172:
case 0xE372:
case 0xE572:
case 0xE772:
case 0xE972:
case 0xEB72:
case 0xED72:
case 0xEF72:
case 0xE173:
case 0xE373:
case 0xE573:
case 0xE773:
case 0xE973:
case 0xEB73:
case 0xED73:
case 0xEF73:
case 0xE174:
case 0xE374:
case 0xE574:
case 0xE774:
case 0xE974:
case 0xEB74:
case 0xED74:
case 0xEF74:
case 0xE175:
case 0xE375:
case 0xE575:
case 0xE775:
case 0xE975:
case 0xEB75:
case 0xED75:
case 0xEF75:
case 0xE176:
case 0xE376:
case 0xE576:
case 0xE776:
case 0xE976:
case 0xEB76:
case 0xED76:
case 0xEF76:
case 0xE177:
case 0xE377:
case 0xE577:
case 0xE777:
case 0xE977:
case 0xEB77:
case 0xED77:
case 0xEF77:

// ROXLD
case 0xE170:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		sft %= 17;

		src |= (CPU->flag_X & C68K_SR_X) << 8;
		res = (src << sft) | (src >> (17 - sft));
		CPU->flag_X = CPU->flag_C = res >> 8;
		CPU->flag_V = 0;
		CPU->flag_N = res >> 8;
		CPU->flag_notZ = res & 0x0000FFFF;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = CPU->flag_X;
	CPU->flag_N = src >> 8;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE3B0:
case 0xE5B0:
case 0xE7B0:
case 0xE9B0:
case 0xEBB0:
case 0xEDB0:
case 0xEFB0:
case 0xE1B1:
case 0xE3B1:
case 0xE5B1:
case 0xE7B1:
case 0xE9B1:
case 0xEBB1:
case 0xEDB1:
case 0xEFB1:
case 0xE1B2:
case 0xE3B2:
case 0xE5B2:
case 0xE7B2:
case 0xE9B2:
case 0xEBB2:
case 0xEDB2:
case 0xEFB2:
case 0xE1B3:
case 0xE3B3:
case 0xE5B3:
case 0xE7B3:
case 0xE9B3:
case 0xEBB3:
case 0xEDB3:
case 0xEFB3:
case 0xE1B4:
case 0xE3B4:
case 0xE5B4:
case 0xE7B4:
case 0xE9B4:
case 0xEBB4:
case 0xEDB4:
case 0xEFB4:
case 0xE1B5:
case 0xE3B5:
case 0xE5B5:
case 0xE7B5:
case 0xE9B5:
case 0xEBB5:
case 0xEDB5:
case 0xEFB5:
case 0xE1B6:
case 0xE3B6:
case 0xE5B6:
case 0xE7B6:
case 0xE9B6:
case 0xEBB6:
case 0xEDB6:
case 0xEFB6:
case 0xE1B7:
case 0xE3B7:
case 0xE5B7:
case 0xE7B7:
case 0xE9B7:
case 0xEBB7:
case 0xEDB7:
case 0xEFB7:

// ROXLD
case 0xE1B0:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		sft %= 33;

		if (sft != 0)
		{
			if (sft == 1) res = (src << 1) | ((CPU->flag_X >> ((C68K_SR_X_SFT + 1) - 1)) & 1);
			else res = (src << sft) | (src >> (33 - sft)) | (((CPU->flag_X >> ((C68K_SR_X_SFT + 1) - 1)) & 1) << (sft - 1));
			CPU->flag_X = (src >> (32 - sft)) << C68K_SR_X_SFT;
		}
		else res = src;
		CPU->flag_C = CPU->flag_X;
		CPU->flag_V = 0;
		CPU->flag_N = res >> 24;
		CPU->flag_notZ = res;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = CPU->flag_X;
	CPU->flag_N = src >> 24;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE338:
case 0xE538:
case 0xE738:
case 0xE938:
case 0xEB38:
case 0xED38:
case 0xEF38:
case 0xE139:
case 0xE339:
case 0xE539:
case 0xE739:
case 0xE939:
case 0xEB39:
case 0xED39:
case 0xEF39:
case 0xE13A:
case 0xE33A:
case 0xE53A:
case 0xE73A:
case 0xE93A:
case 0xEB3A:
case 0xED3A:
case 0xEF3A:
case 0xE13B:
case 0xE33B:
case 0xE53B:
case 0xE73B:
case 0xE93B:
case 0xEB3B:
case 0xED3B:
case 0xEF3B:
case 0xE13C:
case 0xE33C:
case 0xE53C:
case 0xE73C:
case 0xE93C:
case 0xEB3C:
case 0xED3C:
case 0xEF3C:
case 0xE13D:
case 0xE33D:
case 0xE53D:
case 0xE73D:
case 0xE93D:
case 0xEB3D:
case 0xED3D:
case 0xEF3D:
case 0xE13E:
case 0xE33E:
case 0xE53E:
case 0xE73E:
case 0xE93E:
case 0xEB3E:
case 0xED3E:
case 0xEF3E:
case 0xE13F:
case 0xE33F:
case 0xE53F:
case 0xE73F:
case 0xE93F:
case 0xEB3F:
case 0xED3F:
case 0xEF3F:

// ROLD
case 0xE138:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft &= 0x07)
		{
			CPU->flag_C = (src << sft) >> 0;
			res = ((src << sft) | (src >> (8 - sft))) & 0x000000FF;
			CPU->flag_V = 0;
			CPU->flag_N = res >> 0;
			CPU->flag_notZ = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		CPU->flag_V = 0;
		CPU->flag_C = src << C68K_SR_C_SFT;
		CPU->flag_N = src >> 0;
		CPU->flag_notZ = src;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 0;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE378:
case 0xE578:
case 0xE778:
case 0xE978:
case 0xEB78:
case 0xED78:
case 0xEF78:
case 0xE179:
case 0xE379:
case 0xE579:
case 0xE779:
case 0xE979:
case 0xEB79:
case 0xED79:
case 0xEF79:
case 0xE17A:
case 0xE37A:
case 0xE57A:
case 0xE77A:
case 0xE97A:
case 0xEB7A:
case 0xED7A:
case 0xEF7A:
case 0xE17B:
case 0xE37B:
case 0xE57B:
case 0xE77B:
case 0xE97B:
case 0xEB7B:
case 0xED7B:
case 0xEF7B:
case 0xE17C:
case 0xE37C:
case 0xE57C:
case 0xE77C:
case 0xE97C:
case 0xEB7C:
case 0xED7C:
case 0xEF7C:
case 0xE17D:
case 0xE37D:
case 0xE57D:
case 0xE77D:
case 0xE97D:
case 0xEB7D:
case 0xED7D:
case 0xEF7D:
case 0xE17E:
case 0xE37E:
case 0xE57E:
case 0xE77E:
case 0xE97E:
case 0xEB7E:
case 0xED7E:
case 0xEF7E:
case 0xE17F:
case 0xE37F:
case 0xE57F:
case 0xE77F:
case 0xE97F:
case 0xEB7F:
case 0xED7F:
case 0xEF7F:

// ROLD
case 0xE178:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft &= 0x0F)
		{
			CPU->flag_C = (src << sft) >> 8;
			res = ((src << sft) | (src >> (16 - sft))) & 0x0000FFFF;
			CPU->flag_V = 0;
			CPU->flag_N = res >> 8;
			CPU->flag_notZ = res;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		CPU->flag_V = 0;
		CPU->flag_C = src << C68K_SR_C_SFT;
		CPU->flag_N = src >> 8;
		CPU->flag_notZ = src;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 8;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE3B8:
case 0xE5B8:
case 0xE7B8:
case 0xE9B8:
case 0xEBB8:
case 0xEDB8:
case 0xEFB8:
case 0xE1B9:
case 0xE3B9:
case 0xE5B9:
case 0xE7B9:
case 0xE9B9:
case 0xEBB9:
case 0xEDB9:
case 0xEFB9:
case 0xE1BA:
case 0xE3BA:
case 0xE5BA:
case 0xE7BA:
case 0xE9BA:
case 0xEBBA:
case 0xEDBA:
case 0xEFBA:
case 0xE1BB:
case 0xE3BB:
case 0xE5BB:
case 0xE7BB:
case 0xE9BB:
case 0xEBBB:
case 0xEDBB:
case 0xEFBB:
case 0xE1BC:
case 0xE3BC:
case 0xE5BC:
case 0xE7BC:
case 0xE9BC:
case 0xEBBC:
case 0xEDBC:
case 0xEFBC:
case 0xE1BD:
case 0xE3BD:
case 0xE5BD:
case 0xE7BD:
case 0xE9BD:
case 0xEBBD:
case 0xEDBD:
case 0xEFBD:
case 0xE1BE:
case 0xE3BE:
case 0xE5BE:
case 0xE7BE:
case 0xE9BE:
case 0xEBBE:
case 0xEDBE:
case 0xEFBE:
case 0xE1BF:
case 0xE3BF:
case 0xE5BF:
case 0xE7BF:
case 0xE9BF:
case 0xEBBF:
case 0xEDBF:
case 0xEFBF:

// ROLD
case 0xE1B8:
{
	u32 res;
	u32 src;
	u32 sft;

	sft = CPU->D[(Opcode >> 9) & 7] & 0x3F;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	if (sft)
	{
	CCnt -= sft * 2;
		if (sft &= 0x1F)
		{
			CPU->flag_C = (src >> (32 - sft)) << C68K_SR_C_SFT;
			res = (src << sft) | (src >> (32 - sft));
			CPU->flag_V = 0;
			CPU->flag_N = res >> 24;
			CPU->flag_notZ = res;
	*((u32*)(&CPU->D[(Opcode >> 0) & 7])) = res;
	RET(6)
		}

		CPU->flag_V = 0;
		CPU->flag_C = src << C68K_SR_C_SFT;
		CPU->flag_N = src >> 24;
		CPU->flag_notZ = src;
	RET(6)
	}

	CPU->flag_V = 0;
	CPU->flag_C = 0;
	CPU->flag_N = src >> 24;
	CPU->flag_notZ = src;
}
RET(6)
case 0xE0D1:
case 0xE0D2:
case 0xE0D3:
case 0xE0D4:
case 0xE0D5:
case 0xE0D6:
case 0xE0D7:

// ASR
case 0xE0D0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src & (1 << 15));
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE0D9:
case 0xE0DA:
case 0xE0DB:
case 0xE0DC:
case 0xE0DD:
case 0xE0DE:

// ASR
case 0xE0D8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src & (1 << 15));
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE0E1:
case 0xE0E2:
case 0xE0E3:
case 0xE0E4:
case 0xE0E5:
case 0xE0E6:

// ASR
case 0xE0E0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src & (1 << 15));
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE0E9:
case 0xE0EA:
case 0xE0EB:
case 0xE0EC:
case 0xE0ED:
case 0xE0EE:
case 0xE0EF:

// ASR
case 0xE0E8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src & (1 << 15));
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0xE0F1:
case 0xE0F2:
case 0xE0F3:
case 0xE0F4:
case 0xE0F5:
case 0xE0F6:
case 0xE0F7:

// ASR
case 0xE0F0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src & (1 << 15));
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// ASR
case 0xE0F8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src & (1 << 15));
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// ASR
case 0xE0F9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src & (1 << 15));
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// ASR
case 0xE0DF:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src & (1 << 15));
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// ASR
case 0xE0E7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src & (1 << 15));
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE2D1:
case 0xE2D2:
case 0xE2D3:
case 0xE2D4:
case 0xE2D5:
case 0xE2D6:
case 0xE2D7:

// LSR
case 0xE2D0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_N = CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = src >> 1;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE2D9:
case 0xE2DA:
case 0xE2DB:
case 0xE2DC:
case 0xE2DD:
case 0xE2DE:

// LSR
case 0xE2D8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_N = CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = src >> 1;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE2E1:
case 0xE2E2:
case 0xE2E3:
case 0xE2E4:
case 0xE2E5:
case 0xE2E6:

// LSR
case 0xE2E0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_N = CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = src >> 1;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE2E9:
case 0xE2EA:
case 0xE2EB:
case 0xE2EC:
case 0xE2ED:
case 0xE2EE:
case 0xE2EF:

// LSR
case 0xE2E8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_N = CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = src >> 1;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0xE2F1:
case 0xE2F2:
case 0xE2F3:
case 0xE2F4:
case 0xE2F5:
case 0xE2F6:
case 0xE2F7:

// LSR
case 0xE2F0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_N = CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = src >> 1;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// LSR
case 0xE2F8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_N = CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = src >> 1;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// LSR
case 0xE2F9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_N = CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = src >> 1;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// LSR
case 0xE2DF:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_N = CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = src >> 1;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// LSR
case 0xE2E7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_N = CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;
	res = src >> 1;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE4D1:
case 0xE4D2:
case 0xE4D3:
case 0xE4D4:
case 0xE4D5:
case 0xE4D6:
case 0xE4D7:

// ROXR
case 0xE4D0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src >> 1) | ((CPU->flag_X & C68K_SR_X) << 7);
	CPU->flag_C = CPU->flag_X = src << C68K_SR_C_SFT;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE4D9:
case 0xE4DA:
case 0xE4DB:
case 0xE4DC:
case 0xE4DD:
case 0xE4DE:

// ROXR
case 0xE4D8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src >> 1) | ((CPU->flag_X & C68K_SR_X) << 7);
	CPU->flag_C = CPU->flag_X = src << C68K_SR_C_SFT;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE4E1:
case 0xE4E2:
case 0xE4E3:
case 0xE4E4:
case 0xE4E5:
case 0xE4E6:

// ROXR
case 0xE4E0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src >> 1) | ((CPU->flag_X & C68K_SR_X) << 7);
	CPU->flag_C = CPU->flag_X = src << C68K_SR_C_SFT;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE4E9:
case 0xE4EA:
case 0xE4EB:
case 0xE4EC:
case 0xE4ED:
case 0xE4EE:
case 0xE4EF:

// ROXR
case 0xE4E8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src >> 1) | ((CPU->flag_X & C68K_SR_X) << 7);
	CPU->flag_C = CPU->flag_X = src << C68K_SR_C_SFT;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0xE4F1:
case 0xE4F2:
case 0xE4F3:
case 0xE4F4:
case 0xE4F5:
case 0xE4F6:
case 0xE4F7:

// ROXR
case 0xE4F0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src >> 1) | ((CPU->flag_X & C68K_SR_X) << 7);
	CPU->flag_C = CPU->flag_X = src << C68K_SR_C_SFT;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// ROXR
case 0xE4F8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src >> 1) | ((CPU->flag_X & C68K_SR_X) << 7);
	CPU->flag_C = CPU->flag_X = src << C68K_SR_C_SFT;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// ROXR
case 0xE4F9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src >> 1) | ((CPU->flag_X & C68K_SR_X) << 7);
	CPU->flag_C = CPU->flag_X = src << C68K_SR_C_SFT;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// ROXR
case 0xE4DF:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src >> 1) | ((CPU->flag_X & C68K_SR_X) << 7);
	CPU->flag_C = CPU->flag_X = src << C68K_SR_C_SFT;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// ROXR
case 0xE4E7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src >> 1) | ((CPU->flag_X & C68K_SR_X) << 7);
	CPU->flag_C = CPU->flag_X = src << C68K_SR_C_SFT;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE6D1:
case 0xE6D2:
case 0xE6D3:
case 0xE6D4:
case 0xE6D5:
case 0xE6D6:
case 0xE6D7:

// ROR
case 0xE6D0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src << 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE6D9:
case 0xE6DA:
case 0xE6DB:
case 0xE6DC:
case 0xE6DD:
case 0xE6DE:

// ROR
case 0xE6D8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src << 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE6E1:
case 0xE6E2:
case 0xE6E3:
case 0xE6E4:
case 0xE6E5:
case 0xE6E6:

// ROR
case 0xE6E0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src << 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE6E9:
case 0xE6EA:
case 0xE6EB:
case 0xE6EC:
case 0xE6ED:
case 0xE6EE:
case 0xE6EF:

// ROR
case 0xE6E8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src << 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0xE6F1:
case 0xE6F2:
case 0xE6F3:
case 0xE6F4:
case 0xE6F5:
case 0xE6F6:
case 0xE6F7:

// ROR
case 0xE6F0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src << 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// ROR
case 0xE6F8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src << 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// ROR
case 0xE6F9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src << 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// ROR
case 0xE6DF:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src << 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// ROR
case 0xE6E7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src << C68K_SR_C_SFT;
	res = (src >> 1) | (src << 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE1D1:
case 0xE1D2:
case 0xE1D3:
case 0xE1D4:
case 0xE1D5:
case 0xE1D6:
case 0xE1D7:

// ASL
case 0xE1D0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_V = (src ^ res) >> 8;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE1D9:
case 0xE1DA:
case 0xE1DB:
case 0xE1DC:
case 0xE1DD:
case 0xE1DE:

// ASL
case 0xE1D8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_V = (src ^ res) >> 8;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE1E1:
case 0xE1E2:
case 0xE1E3:
case 0xE1E4:
case 0xE1E5:
case 0xE1E6:

// ASL
case 0xE1E0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_V = (src ^ res) >> 8;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE1E9:
case 0xE1EA:
case 0xE1EB:
case 0xE1EC:
case 0xE1ED:
case 0xE1EE:
case 0xE1EF:

// ASL
case 0xE1E8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_V = (src ^ res) >> 8;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0xE1F1:
case 0xE1F2:
case 0xE1F3:
case 0xE1F4:
case 0xE1F5:
case 0xE1F6:
case 0xE1F7:

// ASL
case 0xE1F0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_V = (src ^ res) >> 8;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// ASL
case 0xE1F8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_V = (src ^ res) >> 8;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// ASL
case 0xE1F9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_V = (src ^ res) >> 8;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// ASL
case 0xE1DF:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_V = (src ^ res) >> 8;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// ASL
case 0xE1E7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_V = (src ^ res) >> 8;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE3D1:
case 0xE3D2:
case 0xE3D3:
case 0xE3D4:
case 0xE3D5:
case 0xE3D6:
case 0xE3D7:

// LSL
case 0xE3D0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE3D9:
case 0xE3DA:
case 0xE3DB:
case 0xE3DC:
case 0xE3DD:
case 0xE3DE:

// LSL
case 0xE3D8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE3E1:
case 0xE3E2:
case 0xE3E3:
case 0xE3E4:
case 0xE3E5:
case 0xE3E6:

// LSL
case 0xE3E0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE3E9:
case 0xE3EA:
case 0xE3EB:
case 0xE3EC:
case 0xE3ED:
case 0xE3EE:
case 0xE3EF:

// LSL
case 0xE3E8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0xE3F1:
case 0xE3F2:
case 0xE3F3:
case 0xE3F4:
case 0xE3F5:
case 0xE3F6:
case 0xE3F7:

// LSL
case 0xE3F0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// LSL
case 0xE3F8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// LSL
case 0xE3F9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// LSL
case 0xE3DF:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// LSL
case 0xE3E7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_X = CPU->flag_C = src >> 7;
	res = src << 1;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE5D1:
case 0xE5D2:
case 0xE5D3:
case 0xE5D4:
case 0xE5D5:
case 0xE5D6:
case 0xE5D7:

// ROXL
case 0xE5D0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src << 1) | ((CPU->flag_X & C68K_SR_X) >> 8);
	CPU->flag_X = CPU->flag_C = src >> 7;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE5D9:
case 0xE5DA:
case 0xE5DB:
case 0xE5DC:
case 0xE5DD:
case 0xE5DE:

// ROXL
case 0xE5D8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src << 1) | ((CPU->flag_X & C68K_SR_X) >> 8);
	CPU->flag_X = CPU->flag_C = src >> 7;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE5E1:
case 0xE5E2:
case 0xE5E3:
case 0xE5E4:
case 0xE5E5:
case 0xE5E6:

// ROXL
case 0xE5E0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src << 1) | ((CPU->flag_X & C68K_SR_X) >> 8);
	CPU->flag_X = CPU->flag_C = src >> 7;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE5E9:
case 0xE5EA:
case 0xE5EB:
case 0xE5EC:
case 0xE5ED:
case 0xE5EE:
case 0xE5EF:

// ROXL
case 0xE5E8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src << 1) | ((CPU->flag_X & C68K_SR_X) >> 8);
	CPU->flag_X = CPU->flag_C = src >> 7;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0xE5F1:
case 0xE5F2:
case 0xE5F3:
case 0xE5F4:
case 0xE5F5:
case 0xE5F6:
case 0xE5F7:

// ROXL
case 0xE5F0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src << 1) | ((CPU->flag_X & C68K_SR_X) >> 8);
	CPU->flag_X = CPU->flag_C = src >> 7;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// ROXL
case 0xE5F8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src << 1) | ((CPU->flag_X & C68K_SR_X) >> 8);
	CPU->flag_X = CPU->flag_C = src >> 7;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// ROXL
case 0xE5F9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src << 1) | ((CPU->flag_X & C68K_SR_X) >> 8);
	CPU->flag_X = CPU->flag_C = src >> 7;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// ROXL
case 0xE5DF:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src << 1) | ((CPU->flag_X & C68K_SR_X) >> 8);
	CPU->flag_X = CPU->flag_C = src >> 7;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// ROXL
case 0xE5E7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	res = (src << 1) | ((CPU->flag_X & C68K_SR_X) >> 8);
	CPU->flag_X = CPU->flag_C = src >> 7;
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE7D1:
case 0xE7D2:
case 0xE7D3:
case 0xE7D4:
case 0xE7D5:
case 0xE7D6:
case 0xE7D7:

// ROL
case 0xE7D0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src >> 7;
	res = (src << 1) | (src >> 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE7D9:
case 0xE7DA:
case 0xE7DB:
case 0xE7DC:
case 0xE7DD:
case 0xE7DE:

// ROL
case 0xE7D8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src >> 7;
	res = (src << 1) | (src >> 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0xE7E1:
case 0xE7E2:
case 0xE7E3:
case 0xE7E4:
case 0xE7E5:
case 0xE7E6:

// ROL
case 0xE7E0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src >> 7;
	res = (src << 1) | (src >> 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0xE7E9:
case 0xE7EA:
case 0xE7EB:
case 0xE7EC:
case 0xE7ED:
case 0xE7EE:
case 0xE7EF:

// ROL
case 0xE7E8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src >> 7;
	res = (src << 1) | (src >> 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0xE7F1:
case 0xE7F2:
case 0xE7F3:
case 0xE7F4:
case 0xE7F5:
case 0xE7F6:
case 0xE7F7:

// ROL
case 0xE7F0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src >> 7;
	res = (src << 1) | (src >> 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// ROL
case 0xE7F8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src >> 7;
	res = (src << 1) | (src >> 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// ROL
case 0xE7F9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src >> 7;
	res = (src << 1) | (src >> 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// ROL
case 0xE7DF:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src >> 7;
	res = (src << 1) | (src >> 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// ROL
case 0xE7E7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	CPU->flag_V = 0;
	CPU->flag_C = src >> 7;
	res = (src << 1) | (src >> 15);
	CPU->flag_N = res >> 8;
	CPU->flag_notZ = res & 0x0000FFFF;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
