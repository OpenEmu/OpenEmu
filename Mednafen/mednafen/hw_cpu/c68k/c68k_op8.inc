case 0x8200:
case 0x8400:
case 0x8600:
case 0x8800:
case 0x8A00:
case 0x8C00:
case 0x8E00:
case 0x8001:
case 0x8201:
case 0x8401:
case 0x8601:
case 0x8801:
case 0x8A01:
case 0x8C01:
case 0x8E01:
case 0x8002:
case 0x8202:
case 0x8402:
case 0x8602:
case 0x8802:
case 0x8A02:
case 0x8C02:
case 0x8E02:
case 0x8003:
case 0x8203:
case 0x8403:
case 0x8603:
case 0x8803:
case 0x8A03:
case 0x8C03:
case 0x8E03:
case 0x8004:
case 0x8204:
case 0x8404:
case 0x8604:
case 0x8804:
case 0x8A04:
case 0x8C04:
case 0x8E04:
case 0x8005:
case 0x8205:
case 0x8405:
case 0x8605:
case 0x8805:
case 0x8A05:
case 0x8C05:
case 0x8E05:
case 0x8006:
case 0x8206:
case 0x8406:
case 0x8606:
case 0x8806:
case 0x8A06:
case 0x8C06:
case 0x8E06:
case 0x8007:
case 0x8207:
case 0x8407:
case 0x8607:
case 0x8807:
case 0x8A07:
case 0x8C07:
case 0x8E07:

// ORaD
case 0x8000:
{
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0x8210:
case 0x8410:
case 0x8610:
case 0x8810:
case 0x8A10:
case 0x8C10:
case 0x8E10:
case 0x8011:
case 0x8211:
case 0x8411:
case 0x8611:
case 0x8811:
case 0x8A11:
case 0x8C11:
case 0x8E11:
case 0x8012:
case 0x8212:
case 0x8412:
case 0x8612:
case 0x8812:
case 0x8A12:
case 0x8C12:
case 0x8E12:
case 0x8013:
case 0x8213:
case 0x8413:
case 0x8613:
case 0x8813:
case 0x8A13:
case 0x8C13:
case 0x8E13:
case 0x8014:
case 0x8214:
case 0x8414:
case 0x8614:
case 0x8814:
case 0x8A14:
case 0x8C14:
case 0x8E14:
case 0x8015:
case 0x8215:
case 0x8415:
case 0x8615:
case 0x8815:
case 0x8A15:
case 0x8C15:
case 0x8E15:
case 0x8016:
case 0x8216:
case 0x8416:
case 0x8616:
case 0x8816:
case 0x8A16:
case 0x8C16:
case 0x8E16:
case 0x8017:
case 0x8217:
case 0x8417:
case 0x8617:
case 0x8817:
case 0x8A17:
case 0x8C17:
case 0x8E17:

// ORaD
case 0x8010:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x8218:
case 0x8418:
case 0x8618:
case 0x8818:
case 0x8A18:
case 0x8C18:
case 0x8E18:
case 0x8019:
case 0x8219:
case 0x8419:
case 0x8619:
case 0x8819:
case 0x8A19:
case 0x8C19:
case 0x8E19:
case 0x801A:
case 0x821A:
case 0x841A:
case 0x861A:
case 0x881A:
case 0x8A1A:
case 0x8C1A:
case 0x8E1A:
case 0x801B:
case 0x821B:
case 0x841B:
case 0x861B:
case 0x881B:
case 0x8A1B:
case 0x8C1B:
case 0x8E1B:
case 0x801C:
case 0x821C:
case 0x841C:
case 0x861C:
case 0x881C:
case 0x8A1C:
case 0x8C1C:
case 0x8E1C:
case 0x801D:
case 0x821D:
case 0x841D:
case 0x861D:
case 0x881D:
case 0x8A1D:
case 0x8C1D:
case 0x8E1D:
case 0x801E:
case 0x821E:
case 0x841E:
case 0x861E:
case 0x881E:
case 0x8A1E:
case 0x8C1E:
case 0x8E1E:

// ORaD
case 0x8018:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x8220:
case 0x8420:
case 0x8620:
case 0x8820:
case 0x8A20:
case 0x8C20:
case 0x8E20:
case 0x8021:
case 0x8221:
case 0x8421:
case 0x8621:
case 0x8821:
case 0x8A21:
case 0x8C21:
case 0x8E21:
case 0x8022:
case 0x8222:
case 0x8422:
case 0x8622:
case 0x8822:
case 0x8A22:
case 0x8C22:
case 0x8E22:
case 0x8023:
case 0x8223:
case 0x8423:
case 0x8623:
case 0x8823:
case 0x8A23:
case 0x8C23:
case 0x8E23:
case 0x8024:
case 0x8224:
case 0x8424:
case 0x8624:
case 0x8824:
case 0x8A24:
case 0x8C24:
case 0x8E24:
case 0x8025:
case 0x8225:
case 0x8425:
case 0x8625:
case 0x8825:
case 0x8A25:
case 0x8C25:
case 0x8E25:
case 0x8026:
case 0x8226:
case 0x8426:
case 0x8626:
case 0x8826:
case 0x8A26:
case 0x8C26:
case 0x8E26:

// ORaD
case 0x8020:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0x8228:
case 0x8428:
case 0x8628:
case 0x8828:
case 0x8A28:
case 0x8C28:
case 0x8E28:
case 0x8029:
case 0x8229:
case 0x8429:
case 0x8629:
case 0x8829:
case 0x8A29:
case 0x8C29:
case 0x8E29:
case 0x802A:
case 0x822A:
case 0x842A:
case 0x862A:
case 0x882A:
case 0x8A2A:
case 0x8C2A:
case 0x8E2A:
case 0x802B:
case 0x822B:
case 0x842B:
case 0x862B:
case 0x882B:
case 0x8A2B:
case 0x8C2B:
case 0x8E2B:
case 0x802C:
case 0x822C:
case 0x842C:
case 0x862C:
case 0x882C:
case 0x8A2C:
case 0x8C2C:
case 0x8E2C:
case 0x802D:
case 0x822D:
case 0x842D:
case 0x862D:
case 0x882D:
case 0x8A2D:
case 0x8C2D:
case 0x8E2D:
case 0x802E:
case 0x822E:
case 0x842E:
case 0x862E:
case 0x882E:
case 0x8A2E:
case 0x8C2E:
case 0x8E2E:
case 0x802F:
case 0x822F:
case 0x842F:
case 0x862F:
case 0x882F:
case 0x8A2F:
case 0x8C2F:
case 0x8E2F:

// ORaD
case 0x8028:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x8230:
case 0x8430:
case 0x8630:
case 0x8830:
case 0x8A30:
case 0x8C30:
case 0x8E30:
case 0x8031:
case 0x8231:
case 0x8431:
case 0x8631:
case 0x8831:
case 0x8A31:
case 0x8C31:
case 0x8E31:
case 0x8032:
case 0x8232:
case 0x8432:
case 0x8632:
case 0x8832:
case 0x8A32:
case 0x8C32:
case 0x8E32:
case 0x8033:
case 0x8233:
case 0x8433:
case 0x8633:
case 0x8833:
case 0x8A33:
case 0x8C33:
case 0x8E33:
case 0x8034:
case 0x8234:
case 0x8434:
case 0x8634:
case 0x8834:
case 0x8A34:
case 0x8C34:
case 0x8E34:
case 0x8035:
case 0x8235:
case 0x8435:
case 0x8635:
case 0x8835:
case 0x8A35:
case 0x8C35:
case 0x8E35:
case 0x8036:
case 0x8236:
case 0x8436:
case 0x8636:
case 0x8836:
case 0x8A36:
case 0x8C36:
case 0x8E36:
case 0x8037:
case 0x8237:
case 0x8437:
case 0x8637:
case 0x8837:
case 0x8A37:
case 0x8C37:
case 0x8E37:

// ORaD
case 0x8030:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x8238:
case 0x8438:
case 0x8638:
case 0x8838:
case 0x8A38:
case 0x8C38:
case 0x8E38:

// ORaD
case 0x8038:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x8239:
case 0x8439:
case 0x8639:
case 0x8839:
case 0x8A39:
case 0x8C39:
case 0x8E39:

// ORaD
case 0x8039:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0x823A:
case 0x843A:
case 0x863A:
case 0x883A:
case 0x8A3A:
case 0x8C3A:
case 0x8E3A:

// ORaD
case 0x803A:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x823B:
case 0x843B:
case 0x863B:
case 0x883B:
case 0x8A3B:
case 0x8C3B:
case 0x8E3B:

// ORaD
case 0x803B:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x823C:
case 0x843C:
case 0x863C:
case 0x883C:
case 0x8A3C:
case 0x8C3C:
case 0x8E3C:

// ORaD
case 0x803C:
{
	u32 res;
	u32 src;
	src = FETCH_BYTE;
	PC += 2;
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0x821F:
case 0x841F:
case 0x861F:
case 0x881F:
case 0x8A1F:
case 0x8C1F:
case 0x8E1F:

// ORaD
case 0x801F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x8227:
case 0x8427:
case 0x8627:
case 0x8827:
case 0x8A27:
case 0x8C27:
case 0x8E27:

// ORaD
case 0x8027:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, src)
	res = (u8)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0x8240:
case 0x8440:
case 0x8640:
case 0x8840:
case 0x8A40:
case 0x8C40:
case 0x8E40:
case 0x8041:
case 0x8241:
case 0x8441:
case 0x8641:
case 0x8841:
case 0x8A41:
case 0x8C41:
case 0x8E41:
case 0x8042:
case 0x8242:
case 0x8442:
case 0x8642:
case 0x8842:
case 0x8A42:
case 0x8C42:
case 0x8E42:
case 0x8043:
case 0x8243:
case 0x8443:
case 0x8643:
case 0x8843:
case 0x8A43:
case 0x8C43:
case 0x8E43:
case 0x8044:
case 0x8244:
case 0x8444:
case 0x8644:
case 0x8844:
case 0x8A44:
case 0x8C44:
case 0x8E44:
case 0x8045:
case 0x8245:
case 0x8445:
case 0x8645:
case 0x8845:
case 0x8A45:
case 0x8C45:
case 0x8E45:
case 0x8046:
case 0x8246:
case 0x8446:
case 0x8646:
case 0x8846:
case 0x8A46:
case 0x8C46:
case 0x8E46:
case 0x8047:
case 0x8247:
case 0x8447:
case 0x8647:
case 0x8847:
case 0x8A47:
case 0x8C47:
case 0x8E47:

// ORaD
case 0x8040:
{
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0x8250:
case 0x8450:
case 0x8650:
case 0x8850:
case 0x8A50:
case 0x8C50:
case 0x8E50:
case 0x8051:
case 0x8251:
case 0x8451:
case 0x8651:
case 0x8851:
case 0x8A51:
case 0x8C51:
case 0x8E51:
case 0x8052:
case 0x8252:
case 0x8452:
case 0x8652:
case 0x8852:
case 0x8A52:
case 0x8C52:
case 0x8E52:
case 0x8053:
case 0x8253:
case 0x8453:
case 0x8653:
case 0x8853:
case 0x8A53:
case 0x8C53:
case 0x8E53:
case 0x8054:
case 0x8254:
case 0x8454:
case 0x8654:
case 0x8854:
case 0x8A54:
case 0x8C54:
case 0x8E54:
case 0x8055:
case 0x8255:
case 0x8455:
case 0x8655:
case 0x8855:
case 0x8A55:
case 0x8C55:
case 0x8E55:
case 0x8056:
case 0x8256:
case 0x8456:
case 0x8656:
case 0x8856:
case 0x8A56:
case 0x8C56:
case 0x8E56:
case 0x8057:
case 0x8257:
case 0x8457:
case 0x8657:
case 0x8857:
case 0x8A57:
case 0x8C57:
case 0x8E57:

// ORaD
case 0x8050:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x8258:
case 0x8458:
case 0x8658:
case 0x8858:
case 0x8A58:
case 0x8C58:
case 0x8E58:
case 0x8059:
case 0x8259:
case 0x8459:
case 0x8659:
case 0x8859:
case 0x8A59:
case 0x8C59:
case 0x8E59:
case 0x805A:
case 0x825A:
case 0x845A:
case 0x865A:
case 0x885A:
case 0x8A5A:
case 0x8C5A:
case 0x8E5A:
case 0x805B:
case 0x825B:
case 0x845B:
case 0x865B:
case 0x885B:
case 0x8A5B:
case 0x8C5B:
case 0x8E5B:
case 0x805C:
case 0x825C:
case 0x845C:
case 0x865C:
case 0x885C:
case 0x8A5C:
case 0x8C5C:
case 0x8E5C:
case 0x805D:
case 0x825D:
case 0x845D:
case 0x865D:
case 0x885D:
case 0x8A5D:
case 0x8C5D:
case 0x8E5D:
case 0x805E:
case 0x825E:
case 0x845E:
case 0x865E:
case 0x885E:
case 0x8A5E:
case 0x8C5E:
case 0x8E5E:

// ORaD
case 0x8058:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x8260:
case 0x8460:
case 0x8660:
case 0x8860:
case 0x8A60:
case 0x8C60:
case 0x8E60:
case 0x8061:
case 0x8261:
case 0x8461:
case 0x8661:
case 0x8861:
case 0x8A61:
case 0x8C61:
case 0x8E61:
case 0x8062:
case 0x8262:
case 0x8462:
case 0x8662:
case 0x8862:
case 0x8A62:
case 0x8C62:
case 0x8E62:
case 0x8063:
case 0x8263:
case 0x8463:
case 0x8663:
case 0x8863:
case 0x8A63:
case 0x8C63:
case 0x8E63:
case 0x8064:
case 0x8264:
case 0x8464:
case 0x8664:
case 0x8864:
case 0x8A64:
case 0x8C64:
case 0x8E64:
case 0x8065:
case 0x8265:
case 0x8465:
case 0x8665:
case 0x8865:
case 0x8A65:
case 0x8C65:
case 0x8E65:
case 0x8066:
case 0x8266:
case 0x8466:
case 0x8666:
case 0x8866:
case 0x8A66:
case 0x8C66:
case 0x8E66:

// ORaD
case 0x8060:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0x8268:
case 0x8468:
case 0x8668:
case 0x8868:
case 0x8A68:
case 0x8C68:
case 0x8E68:
case 0x8069:
case 0x8269:
case 0x8469:
case 0x8669:
case 0x8869:
case 0x8A69:
case 0x8C69:
case 0x8E69:
case 0x806A:
case 0x826A:
case 0x846A:
case 0x866A:
case 0x886A:
case 0x8A6A:
case 0x8C6A:
case 0x8E6A:
case 0x806B:
case 0x826B:
case 0x846B:
case 0x866B:
case 0x886B:
case 0x8A6B:
case 0x8C6B:
case 0x8E6B:
case 0x806C:
case 0x826C:
case 0x846C:
case 0x866C:
case 0x886C:
case 0x8A6C:
case 0x8C6C:
case 0x8E6C:
case 0x806D:
case 0x826D:
case 0x846D:
case 0x866D:
case 0x886D:
case 0x8A6D:
case 0x8C6D:
case 0x8E6D:
case 0x806E:
case 0x826E:
case 0x846E:
case 0x866E:
case 0x886E:
case 0x8A6E:
case 0x8C6E:
case 0x8E6E:
case 0x806F:
case 0x826F:
case 0x846F:
case 0x866F:
case 0x886F:
case 0x8A6F:
case 0x8C6F:
case 0x8E6F:

// ORaD
case 0x8068:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x8270:
case 0x8470:
case 0x8670:
case 0x8870:
case 0x8A70:
case 0x8C70:
case 0x8E70:
case 0x8071:
case 0x8271:
case 0x8471:
case 0x8671:
case 0x8871:
case 0x8A71:
case 0x8C71:
case 0x8E71:
case 0x8072:
case 0x8272:
case 0x8472:
case 0x8672:
case 0x8872:
case 0x8A72:
case 0x8C72:
case 0x8E72:
case 0x8073:
case 0x8273:
case 0x8473:
case 0x8673:
case 0x8873:
case 0x8A73:
case 0x8C73:
case 0x8E73:
case 0x8074:
case 0x8274:
case 0x8474:
case 0x8674:
case 0x8874:
case 0x8A74:
case 0x8C74:
case 0x8E74:
case 0x8075:
case 0x8275:
case 0x8475:
case 0x8675:
case 0x8875:
case 0x8A75:
case 0x8C75:
case 0x8E75:
case 0x8076:
case 0x8276:
case 0x8476:
case 0x8676:
case 0x8876:
case 0x8A76:
case 0x8C76:
case 0x8E76:
case 0x8077:
case 0x8277:
case 0x8477:
case 0x8677:
case 0x8877:
case 0x8A77:
case 0x8C77:
case 0x8E77:

// ORaD
case 0x8070:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x8278:
case 0x8478:
case 0x8678:
case 0x8878:
case 0x8A78:
case 0x8C78:
case 0x8E78:

// ORaD
case 0x8078:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x8279:
case 0x8479:
case 0x8679:
case 0x8879:
case 0x8A79:
case 0x8C79:
case 0x8E79:

// ORaD
case 0x8079:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0x827A:
case 0x847A:
case 0x867A:
case 0x887A:
case 0x8A7A:
case 0x8C7A:
case 0x8E7A:

// ORaD
case 0x807A:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x827B:
case 0x847B:
case 0x867B:
case 0x887B:
case 0x8A7B:
case 0x8C7B:
case 0x8E7B:

// ORaD
case 0x807B:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x827C:
case 0x847C:
case 0x867C:
case 0x887C:
case 0x8A7C:
case 0x8C7C:
case 0x8E7C:

// ORaD
case 0x807C:
{
	u32 res;
	u32 src;
	src = FETCH_WORD;
	PC += 2;
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0x825F:
case 0x845F:
case 0x865F:
case 0x885F:
case 0x8A5F:
case 0x8C5F:
case 0x8E5F:

// ORaD
case 0x805F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x8267:
case 0x8467:
case 0x8667:
case 0x8867:
case 0x8A67:
case 0x8C67:
case 0x8E67:

// ORaD
case 0x8067:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	res = (u16)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0x8280:
case 0x8480:
case 0x8680:
case 0x8880:
case 0x8A80:
case 0x8C80:
case 0x8E80:
case 0x8081:
case 0x8281:
case 0x8481:
case 0x8681:
case 0x8881:
case 0x8A81:
case 0x8C81:
case 0x8E81:
case 0x8082:
case 0x8282:
case 0x8482:
case 0x8682:
case 0x8882:
case 0x8A82:
case 0x8C82:
case 0x8E82:
case 0x8083:
case 0x8283:
case 0x8483:
case 0x8683:
case 0x8883:
case 0x8A83:
case 0x8C83:
case 0x8E83:
case 0x8084:
case 0x8284:
case 0x8484:
case 0x8684:
case 0x8884:
case 0x8A84:
case 0x8C84:
case 0x8E84:
case 0x8085:
case 0x8285:
case 0x8485:
case 0x8685:
case 0x8885:
case 0x8A85:
case 0x8C85:
case 0x8E85:
case 0x8086:
case 0x8286:
case 0x8486:
case 0x8686:
case 0x8886:
case 0x8A86:
case 0x8C86:
case 0x8E86:
case 0x8087:
case 0x8287:
case 0x8487:
case 0x8687:
case 0x8887:
case 0x8A87:
case 0x8C87:
case 0x8E87:

// ORaD
case 0x8080:
{
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 0) & 7];
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0x8290:
case 0x8490:
case 0x8690:
case 0x8890:
case 0x8A90:
case 0x8C90:
case 0x8E90:
case 0x8091:
case 0x8291:
case 0x8491:
case 0x8691:
case 0x8891:
case 0x8A91:
case 0x8C91:
case 0x8E91:
case 0x8092:
case 0x8292:
case 0x8492:
case 0x8692:
case 0x8892:
case 0x8A92:
case 0x8C92:
case 0x8E92:
case 0x8093:
case 0x8293:
case 0x8493:
case 0x8693:
case 0x8893:
case 0x8A93:
case 0x8C93:
case 0x8E93:
case 0x8094:
case 0x8294:
case 0x8494:
case 0x8694:
case 0x8894:
case 0x8A94:
case 0x8C94:
case 0x8E94:
case 0x8095:
case 0x8295:
case 0x8495:
case 0x8695:
case 0x8895:
case 0x8A95:
case 0x8C95:
case 0x8E95:
case 0x8096:
case 0x8296:
case 0x8496:
case 0x8696:
case 0x8896:
case 0x8A96:
case 0x8C96:
case 0x8E96:
case 0x8097:
case 0x8297:
case 0x8497:
case 0x8697:
case 0x8897:
case 0x8A97:
case 0x8C97:
case 0x8E97:

// ORaD
case 0x8090:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x8298:
case 0x8498:
case 0x8698:
case 0x8898:
case 0x8A98:
case 0x8C98:
case 0x8E98:
case 0x8099:
case 0x8299:
case 0x8499:
case 0x8699:
case 0x8899:
case 0x8A99:
case 0x8C99:
case 0x8E99:
case 0x809A:
case 0x829A:
case 0x849A:
case 0x869A:
case 0x889A:
case 0x8A9A:
case 0x8C9A:
case 0x8E9A:
case 0x809B:
case 0x829B:
case 0x849B:
case 0x869B:
case 0x889B:
case 0x8A9B:
case 0x8C9B:
case 0x8E9B:
case 0x809C:
case 0x829C:
case 0x849C:
case 0x869C:
case 0x889C:
case 0x8A9C:
case 0x8C9C:
case 0x8E9C:
case 0x809D:
case 0x829D:
case 0x849D:
case 0x869D:
case 0x889D:
case 0x8A9D:
case 0x8C9D:
case 0x8E9D:
case 0x809E:
case 0x829E:
case 0x849E:
case 0x869E:
case 0x889E:
case 0x8A9E:
case 0x8C9E:
case 0x8E9E:

// ORaD
case 0x8098:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x82A0:
case 0x84A0:
case 0x86A0:
case 0x88A0:
case 0x8AA0:
case 0x8CA0:
case 0x8EA0:
case 0x80A1:
case 0x82A1:
case 0x84A1:
case 0x86A1:
case 0x88A1:
case 0x8AA1:
case 0x8CA1:
case 0x8EA1:
case 0x80A2:
case 0x82A2:
case 0x84A2:
case 0x86A2:
case 0x88A2:
case 0x8AA2:
case 0x8CA2:
case 0x8EA2:
case 0x80A3:
case 0x82A3:
case 0x84A3:
case 0x86A3:
case 0x88A3:
case 0x8AA3:
case 0x8CA3:
case 0x8EA3:
case 0x80A4:
case 0x82A4:
case 0x84A4:
case 0x86A4:
case 0x88A4:
case 0x8AA4:
case 0x8CA4:
case 0x8EA4:
case 0x80A5:
case 0x82A5:
case 0x84A5:
case 0x86A5:
case 0x88A5:
case 0x8AA5:
case 0x8CA5:
case 0x8EA5:
case 0x80A6:
case 0x82A6:
case 0x84A6:
case 0x86A6:
case 0x88A6:
case 0x8AA6:
case 0x8CA6:
case 0x8EA6:

// ORaD
case 0x80A0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0x82A8:
case 0x84A8:
case 0x86A8:
case 0x88A8:
case 0x8AA8:
case 0x8CA8:
case 0x8EA8:
case 0x80A9:
case 0x82A9:
case 0x84A9:
case 0x86A9:
case 0x88A9:
case 0x8AA9:
case 0x8CA9:
case 0x8EA9:
case 0x80AA:
case 0x82AA:
case 0x84AA:
case 0x86AA:
case 0x88AA:
case 0x8AAA:
case 0x8CAA:
case 0x8EAA:
case 0x80AB:
case 0x82AB:
case 0x84AB:
case 0x86AB:
case 0x88AB:
case 0x8AAB:
case 0x8CAB:
case 0x8EAB:
case 0x80AC:
case 0x82AC:
case 0x84AC:
case 0x86AC:
case 0x88AC:
case 0x8AAC:
case 0x8CAC:
case 0x8EAC:
case 0x80AD:
case 0x82AD:
case 0x84AD:
case 0x86AD:
case 0x88AD:
case 0x8AAD:
case 0x8CAD:
case 0x8EAD:
case 0x80AE:
case 0x82AE:
case 0x84AE:
case 0x86AE:
case 0x88AE:
case 0x8AAE:
case 0x8CAE:
case 0x8EAE:
case 0x80AF:
case 0x82AF:
case 0x84AF:
case 0x86AF:
case 0x88AF:
case 0x8AAF:
case 0x8CAF:
case 0x8EAF:

// ORaD
case 0x80A8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(18)
case 0x82B0:
case 0x84B0:
case 0x86B0:
case 0x88B0:
case 0x8AB0:
case 0x8CB0:
case 0x8EB0:
case 0x80B1:
case 0x82B1:
case 0x84B1:
case 0x86B1:
case 0x88B1:
case 0x8AB1:
case 0x8CB1:
case 0x8EB1:
case 0x80B2:
case 0x82B2:
case 0x84B2:
case 0x86B2:
case 0x88B2:
case 0x8AB2:
case 0x8CB2:
case 0x8EB2:
case 0x80B3:
case 0x82B3:
case 0x84B3:
case 0x86B3:
case 0x88B3:
case 0x8AB3:
case 0x8CB3:
case 0x8EB3:
case 0x80B4:
case 0x82B4:
case 0x84B4:
case 0x86B4:
case 0x88B4:
case 0x8AB4:
case 0x8CB4:
case 0x8EB4:
case 0x80B5:
case 0x82B5:
case 0x84B5:
case 0x86B5:
case 0x88B5:
case 0x8AB5:
case 0x8CB5:
case 0x8EB5:
case 0x80B6:
case 0x82B6:
case 0x84B6:
case 0x86B6:
case 0x88B6:
case 0x8AB6:
case 0x8CB6:
case 0x8EB6:
case 0x80B7:
case 0x82B7:
case 0x84B7:
case 0x86B7:
case 0x88B7:
case 0x8AB7:
case 0x8CB7:
case 0x8EB7:

// ORaD
case 0x80B0:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(20)
case 0x82B8:
case 0x84B8:
case 0x86B8:
case 0x88B8:
case 0x8AB8:
case 0x8CB8:
case 0x8EB8:

// ORaD
case 0x80B8:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(18)
case 0x82B9:
case 0x84B9:
case 0x86B9:
case 0x88B9:
case 0x8AB9:
case 0x8CB9:
case 0x8EB9:

// ORaD
case 0x80B9:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(22)
case 0x82BA:
case 0x84BA:
case 0x86BA:
case 0x88BA:
case 0x8ABA:
case 0x8CBA:
case 0x8EBA:

// ORaD
case 0x80BA:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(18)
case 0x82BB:
case 0x84BB:
case 0x86BB:
case 0x88BB:
case 0x8ABB:
case 0x8CBB:
case 0x8EBB:

// ORaD
case 0x80BB:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(20)
case 0x82BC:
case 0x84BC:
case 0x86BC:
case 0x88BC:
case 0x8ABC:
case 0x8CBC:
case 0x8EBC:

// ORaD
case 0x80BC:
{
	u32 res;
	u32 src;
	src = FETCH_LONG;
	PC += 4;
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(16)
case 0x829F:
case 0x849F:
case 0x869F:
case 0x889F:
case 0x8A9F:
case 0x8C9F:
case 0x8E9F:

// ORaD
case 0x809F:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x82A7:
case 0x84A7:
case 0x86A7:
case 0x88A7:
case 0x8AA7:
case 0x8CA7:
case 0x8EA7:

// ORaD
case 0x80A7:
{
	u32 adr;
	u32 res;
	u32 src;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READ_LONG_F(adr, src)
	res = (u32)CPU->D[(Opcode >> 9) & 7];
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0x8310:
case 0x8510:
case 0x8710:
case 0x8910:
case 0x8B10:
case 0x8D10:
case 0x8F10:
case 0x8111:
case 0x8311:
case 0x8511:
case 0x8711:
case 0x8911:
case 0x8B11:
case 0x8D11:
case 0x8F11:
case 0x8112:
case 0x8312:
case 0x8512:
case 0x8712:
case 0x8912:
case 0x8B12:
case 0x8D12:
case 0x8F12:
case 0x8113:
case 0x8313:
case 0x8513:
case 0x8713:
case 0x8913:
case 0x8B13:
case 0x8D13:
case 0x8F13:
case 0x8114:
case 0x8314:
case 0x8514:
case 0x8714:
case 0x8914:
case 0x8B14:
case 0x8D14:
case 0x8F14:
case 0x8115:
case 0x8315:
case 0x8515:
case 0x8715:
case 0x8915:
case 0x8B15:
case 0x8D15:
case 0x8F15:
case 0x8116:
case 0x8316:
case 0x8516:
case 0x8716:
case 0x8916:
case 0x8B16:
case 0x8D16:
case 0x8F16:
case 0x8117:
case 0x8317:
case 0x8517:
case 0x8717:
case 0x8917:
case 0x8B17:
case 0x8D17:
case 0x8F17:

// ORDa
case 0x8110:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_BYTE_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x8318:
case 0x8518:
case 0x8718:
case 0x8918:
case 0x8B18:
case 0x8D18:
case 0x8F18:
case 0x8119:
case 0x8319:
case 0x8519:
case 0x8719:
case 0x8919:
case 0x8B19:
case 0x8D19:
case 0x8F19:
case 0x811A:
case 0x831A:
case 0x851A:
case 0x871A:
case 0x891A:
case 0x8B1A:
case 0x8D1A:
case 0x8F1A:
case 0x811B:
case 0x831B:
case 0x851B:
case 0x871B:
case 0x891B:
case 0x8B1B:
case 0x8D1B:
case 0x8F1B:
case 0x811C:
case 0x831C:
case 0x851C:
case 0x871C:
case 0x891C:
case 0x8B1C:
case 0x8D1C:
case 0x8F1C:
case 0x811D:
case 0x831D:
case 0x851D:
case 0x871D:
case 0x891D:
case 0x8B1D:
case 0x8D1D:
case 0x8F1D:
case 0x811E:
case 0x831E:
case 0x851E:
case 0x871E:
case 0x891E:
case 0x8B1E:
case 0x8D1E:
case 0x8F1E:

// ORDa
case 0x8118:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 1;
	PRE_IO
	READ_BYTE_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x8320:
case 0x8520:
case 0x8720:
case 0x8920:
case 0x8B20:
case 0x8D20:
case 0x8F20:
case 0x8121:
case 0x8321:
case 0x8521:
case 0x8721:
case 0x8921:
case 0x8B21:
case 0x8D21:
case 0x8F21:
case 0x8122:
case 0x8322:
case 0x8522:
case 0x8722:
case 0x8922:
case 0x8B22:
case 0x8D22:
case 0x8F22:
case 0x8123:
case 0x8323:
case 0x8523:
case 0x8723:
case 0x8923:
case 0x8B23:
case 0x8D23:
case 0x8F23:
case 0x8124:
case 0x8324:
case 0x8524:
case 0x8724:
case 0x8924:
case 0x8B24:
case 0x8D24:
case 0x8F24:
case 0x8125:
case 0x8325:
case 0x8525:
case 0x8725:
case 0x8925:
case 0x8B25:
case 0x8D25:
case 0x8F25:
case 0x8126:
case 0x8326:
case 0x8526:
case 0x8726:
case 0x8926:
case 0x8B26:
case 0x8D26:
case 0x8F26:

// ORDa
case 0x8120:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] - 1;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_BYTE_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x8328:
case 0x8528:
case 0x8728:
case 0x8928:
case 0x8B28:
case 0x8D28:
case 0x8F28:
case 0x8129:
case 0x8329:
case 0x8529:
case 0x8729:
case 0x8929:
case 0x8B29:
case 0x8D29:
case 0x8F29:
case 0x812A:
case 0x832A:
case 0x852A:
case 0x872A:
case 0x892A:
case 0x8B2A:
case 0x8D2A:
case 0x8F2A:
case 0x812B:
case 0x832B:
case 0x852B:
case 0x872B:
case 0x892B:
case 0x8B2B:
case 0x8D2B:
case 0x8F2B:
case 0x812C:
case 0x832C:
case 0x852C:
case 0x872C:
case 0x892C:
case 0x8B2C:
case 0x8D2C:
case 0x8F2C:
case 0x812D:
case 0x832D:
case 0x852D:
case 0x872D:
case 0x892D:
case 0x8B2D:
case 0x8D2D:
case 0x8F2D:
case 0x812E:
case 0x832E:
case 0x852E:
case 0x872E:
case 0x892E:
case 0x8B2E:
case 0x8D2E:
case 0x8F2E:
case 0x812F:
case 0x832F:
case 0x852F:
case 0x872F:
case 0x892F:
case 0x8B2F:
case 0x8D2F:
case 0x8F2F:

// ORDa
case 0x8128:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x8330:
case 0x8530:
case 0x8730:
case 0x8930:
case 0x8B30:
case 0x8D30:
case 0x8F30:
case 0x8131:
case 0x8331:
case 0x8531:
case 0x8731:
case 0x8931:
case 0x8B31:
case 0x8D31:
case 0x8F31:
case 0x8132:
case 0x8332:
case 0x8532:
case 0x8732:
case 0x8932:
case 0x8B32:
case 0x8D32:
case 0x8F32:
case 0x8133:
case 0x8333:
case 0x8533:
case 0x8733:
case 0x8933:
case 0x8B33:
case 0x8D33:
case 0x8F33:
case 0x8134:
case 0x8334:
case 0x8534:
case 0x8734:
case 0x8934:
case 0x8B34:
case 0x8D34:
case 0x8F34:
case 0x8135:
case 0x8335:
case 0x8535:
case 0x8735:
case 0x8935:
case 0x8B35:
case 0x8D35:
case 0x8F35:
case 0x8136:
case 0x8336:
case 0x8536:
case 0x8736:
case 0x8936:
case 0x8B36:
case 0x8D36:
case 0x8F36:
case 0x8137:
case 0x8337:
case 0x8537:
case 0x8737:
case 0x8937:
case 0x8B37:
case 0x8D37:
case 0x8F37:

// ORDa
case 0x8130:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x8338:
case 0x8538:
case 0x8738:
case 0x8938:
case 0x8B38:
case 0x8D38:
case 0x8F38:

// ORDa
case 0x8138:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x8339:
case 0x8539:
case 0x8739:
case 0x8939:
case 0x8B39:
case 0x8D39:
case 0x8F39:

// ORDa
case 0x8139:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_BYTE_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0x831F:
case 0x851F:
case 0x871F:
case 0x891F:
case 0x8B1F:
case 0x8D1F:
case 0x8F1F:

// ORDa
case 0x811F:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x8327:
case 0x8527:
case 0x8727:
case 0x8927:
case 0x8B27:
case 0x8D27:
case 0x8F27:

// ORDa
case 0x8127:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_BYTE_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x8350:
case 0x8550:
case 0x8750:
case 0x8950:
case 0x8B50:
case 0x8D50:
case 0x8F50:
case 0x8151:
case 0x8351:
case 0x8551:
case 0x8751:
case 0x8951:
case 0x8B51:
case 0x8D51:
case 0x8F51:
case 0x8152:
case 0x8352:
case 0x8552:
case 0x8752:
case 0x8952:
case 0x8B52:
case 0x8D52:
case 0x8F52:
case 0x8153:
case 0x8353:
case 0x8553:
case 0x8753:
case 0x8953:
case 0x8B53:
case 0x8D53:
case 0x8F53:
case 0x8154:
case 0x8354:
case 0x8554:
case 0x8754:
case 0x8954:
case 0x8B54:
case 0x8D54:
case 0x8F54:
case 0x8155:
case 0x8355:
case 0x8555:
case 0x8755:
case 0x8955:
case 0x8B55:
case 0x8D55:
case 0x8F55:
case 0x8156:
case 0x8356:
case 0x8556:
case 0x8756:
case 0x8956:
case 0x8B56:
case 0x8D56:
case 0x8F56:
case 0x8157:
case 0x8357:
case 0x8557:
case 0x8757:
case 0x8957:
case 0x8B57:
case 0x8D57:
case 0x8F57:

// ORDa
case 0x8150:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x8358:
case 0x8558:
case 0x8758:
case 0x8958:
case 0x8B58:
case 0x8D58:
case 0x8F58:
case 0x8159:
case 0x8359:
case 0x8559:
case 0x8759:
case 0x8959:
case 0x8B59:
case 0x8D59:
case 0x8F59:
case 0x815A:
case 0x835A:
case 0x855A:
case 0x875A:
case 0x895A:
case 0x8B5A:
case 0x8D5A:
case 0x8F5A:
case 0x815B:
case 0x835B:
case 0x855B:
case 0x875B:
case 0x895B:
case 0x8B5B:
case 0x8D5B:
case 0x8F5B:
case 0x815C:
case 0x835C:
case 0x855C:
case 0x875C:
case 0x895C:
case 0x8B5C:
case 0x8D5C:
case 0x8F5C:
case 0x815D:
case 0x835D:
case 0x855D:
case 0x875D:
case 0x895D:
case 0x8B5D:
case 0x8D5D:
case 0x8F5D:
case 0x815E:
case 0x835E:
case 0x855E:
case 0x875E:
case 0x895E:
case 0x8B5E:
case 0x8D5E:
case 0x8F5E:

// ORDa
case 0x8158:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x8360:
case 0x8560:
case 0x8760:
case 0x8960:
case 0x8B60:
case 0x8D60:
case 0x8F60:
case 0x8161:
case 0x8361:
case 0x8561:
case 0x8761:
case 0x8961:
case 0x8B61:
case 0x8D61:
case 0x8F61:
case 0x8162:
case 0x8362:
case 0x8562:
case 0x8762:
case 0x8962:
case 0x8B62:
case 0x8D62:
case 0x8F62:
case 0x8163:
case 0x8363:
case 0x8563:
case 0x8763:
case 0x8963:
case 0x8B63:
case 0x8D63:
case 0x8F63:
case 0x8164:
case 0x8364:
case 0x8564:
case 0x8764:
case 0x8964:
case 0x8B64:
case 0x8D64:
case 0x8F64:
case 0x8165:
case 0x8365:
case 0x8565:
case 0x8765:
case 0x8965:
case 0x8B65:
case 0x8D65:
case 0x8F65:
case 0x8166:
case 0x8366:
case 0x8566:
case 0x8766:
case 0x8966:
case 0x8B66:
case 0x8D66:
case 0x8F66:

// ORDa
case 0x8160:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x8368:
case 0x8568:
case 0x8768:
case 0x8968:
case 0x8B68:
case 0x8D68:
case 0x8F68:
case 0x8169:
case 0x8369:
case 0x8569:
case 0x8769:
case 0x8969:
case 0x8B69:
case 0x8D69:
case 0x8F69:
case 0x816A:
case 0x836A:
case 0x856A:
case 0x876A:
case 0x896A:
case 0x8B6A:
case 0x8D6A:
case 0x8F6A:
case 0x816B:
case 0x836B:
case 0x856B:
case 0x876B:
case 0x896B:
case 0x8B6B:
case 0x8D6B:
case 0x8F6B:
case 0x816C:
case 0x836C:
case 0x856C:
case 0x876C:
case 0x896C:
case 0x8B6C:
case 0x8D6C:
case 0x8F6C:
case 0x816D:
case 0x836D:
case 0x856D:
case 0x876D:
case 0x896D:
case 0x8B6D:
case 0x8D6D:
case 0x8F6D:
case 0x816E:
case 0x836E:
case 0x856E:
case 0x876E:
case 0x896E:
case 0x8B6E:
case 0x8D6E:
case 0x8F6E:
case 0x816F:
case 0x836F:
case 0x856F:
case 0x876F:
case 0x896F:
case 0x8B6F:
case 0x8D6F:
case 0x8F6F:

// ORDa
case 0x8168:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x8370:
case 0x8570:
case 0x8770:
case 0x8970:
case 0x8B70:
case 0x8D70:
case 0x8F70:
case 0x8171:
case 0x8371:
case 0x8571:
case 0x8771:
case 0x8971:
case 0x8B71:
case 0x8D71:
case 0x8F71:
case 0x8172:
case 0x8372:
case 0x8572:
case 0x8772:
case 0x8972:
case 0x8B72:
case 0x8D72:
case 0x8F72:
case 0x8173:
case 0x8373:
case 0x8573:
case 0x8773:
case 0x8973:
case 0x8B73:
case 0x8D73:
case 0x8F73:
case 0x8174:
case 0x8374:
case 0x8574:
case 0x8774:
case 0x8974:
case 0x8B74:
case 0x8D74:
case 0x8F74:
case 0x8175:
case 0x8375:
case 0x8575:
case 0x8775:
case 0x8975:
case 0x8B75:
case 0x8D75:
case 0x8F75:
case 0x8176:
case 0x8376:
case 0x8576:
case 0x8776:
case 0x8976:
case 0x8B76:
case 0x8D76:
case 0x8F76:
case 0x8177:
case 0x8377:
case 0x8577:
case 0x8777:
case 0x8977:
case 0x8B77:
case 0x8D77:
case 0x8F77:

// ORDa
case 0x8170:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x8378:
case 0x8578:
case 0x8778:
case 0x8978:
case 0x8B78:
case 0x8D78:
case 0x8F78:

// ORDa
case 0x8178:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x8379:
case 0x8579:
case 0x8779:
case 0x8979:
case 0x8B79:
case 0x8D79:
case 0x8F79:

// ORDa
case 0x8179:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0x835F:
case 0x855F:
case 0x875F:
case 0x895F:
case 0x8B5F:
case 0x8D5F:
case 0x8F5F:

// ORDa
case 0x815F:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x8367:
case 0x8567:
case 0x8767:
case 0x8967:
case 0x8B67:
case 0x8D67:
case 0x8F67:

// ORDa
case 0x8167:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x8390:
case 0x8590:
case 0x8790:
case 0x8990:
case 0x8B90:
case 0x8D90:
case 0x8F90:
case 0x8191:
case 0x8391:
case 0x8591:
case 0x8791:
case 0x8991:
case 0x8B91:
case 0x8D91:
case 0x8F91:
case 0x8192:
case 0x8392:
case 0x8592:
case 0x8792:
case 0x8992:
case 0x8B92:
case 0x8D92:
case 0x8F92:
case 0x8193:
case 0x8393:
case 0x8593:
case 0x8793:
case 0x8993:
case 0x8B93:
case 0x8D93:
case 0x8F93:
case 0x8194:
case 0x8394:
case 0x8594:
case 0x8794:
case 0x8994:
case 0x8B94:
case 0x8D94:
case 0x8F94:
case 0x8195:
case 0x8395:
case 0x8595:
case 0x8795:
case 0x8995:
case 0x8B95:
case 0x8D95:
case 0x8F95:
case 0x8196:
case 0x8396:
case 0x8596:
case 0x8796:
case 0x8996:
case 0x8B96:
case 0x8D96:
case 0x8F96:
case 0x8197:
case 0x8397:
case 0x8597:
case 0x8797:
case 0x8997:
case 0x8B97:
case 0x8D97:
case 0x8F97:

// ORDa
case 0x8190:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_LONG_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x8398:
case 0x8598:
case 0x8798:
case 0x8998:
case 0x8B98:
case 0x8D98:
case 0x8F98:
case 0x8199:
case 0x8399:
case 0x8599:
case 0x8799:
case 0x8999:
case 0x8B99:
case 0x8D99:
case 0x8F99:
case 0x819A:
case 0x839A:
case 0x859A:
case 0x879A:
case 0x899A:
case 0x8B9A:
case 0x8D9A:
case 0x8F9A:
case 0x819B:
case 0x839B:
case 0x859B:
case 0x879B:
case 0x899B:
case 0x8B9B:
case 0x8D9B:
case 0x8F9B:
case 0x819C:
case 0x839C:
case 0x859C:
case 0x879C:
case 0x899C:
case 0x8B9C:
case 0x8D9C:
case 0x8F9C:
case 0x819D:
case 0x839D:
case 0x859D:
case 0x879D:
case 0x899D:
case 0x8B9D:
case 0x8D9D:
case 0x8F9D:
case 0x819E:
case 0x839E:
case 0x859E:
case 0x879E:
case 0x899E:
case 0x8B9E:
case 0x8D9E:
case 0x8F9E:

// ORDa
case 0x8198:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READ_LONG_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x83A0:
case 0x85A0:
case 0x87A0:
case 0x89A0:
case 0x8BA0:
case 0x8DA0:
case 0x8FA0:
case 0x81A1:
case 0x83A1:
case 0x85A1:
case 0x87A1:
case 0x89A1:
case 0x8BA1:
case 0x8DA1:
case 0x8FA1:
case 0x81A2:
case 0x83A2:
case 0x85A2:
case 0x87A2:
case 0x89A2:
case 0x8BA2:
case 0x8DA2:
case 0x8FA2:
case 0x81A3:
case 0x83A3:
case 0x85A3:
case 0x87A3:
case 0x89A3:
case 0x8BA3:
case 0x8DA3:
case 0x8FA3:
case 0x81A4:
case 0x83A4:
case 0x85A4:
case 0x87A4:
case 0x89A4:
case 0x8BA4:
case 0x8DA4:
case 0x8FA4:
case 0x81A5:
case 0x83A5:
case 0x85A5:
case 0x87A5:
case 0x89A5:
case 0x8BA5:
case 0x8DA5:
case 0x8FA5:
case 0x81A6:
case 0x83A6:
case 0x85A6:
case 0x87A6:
case 0x89A6:
case 0x8BA6:
case 0x8DA6:
case 0x8FA6:

// ORDa
case 0x81A0:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_LONG_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x83A8:
case 0x85A8:
case 0x87A8:
case 0x89A8:
case 0x8BA8:
case 0x8DA8:
case 0x8FA8:
case 0x81A9:
case 0x83A9:
case 0x85A9:
case 0x87A9:
case 0x89A9:
case 0x8BA9:
case 0x8DA9:
case 0x8FA9:
case 0x81AA:
case 0x83AA:
case 0x85AA:
case 0x87AA:
case 0x89AA:
case 0x8BAA:
case 0x8DAA:
case 0x8FAA:
case 0x81AB:
case 0x83AB:
case 0x85AB:
case 0x87AB:
case 0x89AB:
case 0x8BAB:
case 0x8DAB:
case 0x8FAB:
case 0x81AC:
case 0x83AC:
case 0x85AC:
case 0x87AC:
case 0x89AC:
case 0x8BAC:
case 0x8DAC:
case 0x8FAC:
case 0x81AD:
case 0x83AD:
case 0x85AD:
case 0x87AD:
case 0x89AD:
case 0x8BAD:
case 0x8DAD:
case 0x8FAD:
case 0x81AE:
case 0x83AE:
case 0x85AE:
case 0x87AE:
case 0x89AE:
case 0x8BAE:
case 0x8DAE:
case 0x8FAE:
case 0x81AF:
case 0x83AF:
case 0x85AF:
case 0x87AF:
case 0x89AF:
case 0x8BAF:
case 0x8DAF:
case 0x8FAF:

// ORDa
case 0x81A8:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x83B0:
case 0x85B0:
case 0x87B0:
case 0x89B0:
case 0x8BB0:
case 0x8DB0:
case 0x8FB0:
case 0x81B1:
case 0x83B1:
case 0x85B1:
case 0x87B1:
case 0x89B1:
case 0x8BB1:
case 0x8DB1:
case 0x8FB1:
case 0x81B2:
case 0x83B2:
case 0x85B2:
case 0x87B2:
case 0x89B2:
case 0x8BB2:
case 0x8DB2:
case 0x8FB2:
case 0x81B3:
case 0x83B3:
case 0x85B3:
case 0x87B3:
case 0x89B3:
case 0x8BB3:
case 0x8DB3:
case 0x8FB3:
case 0x81B4:
case 0x83B4:
case 0x85B4:
case 0x87B4:
case 0x89B4:
case 0x8BB4:
case 0x8DB4:
case 0x8FB4:
case 0x81B5:
case 0x83B5:
case 0x85B5:
case 0x87B5:
case 0x89B5:
case 0x8BB5:
case 0x8DB5:
case 0x8FB5:
case 0x81B6:
case 0x83B6:
case 0x85B6:
case 0x87B6:
case 0x89B6:
case 0x8BB6:
case 0x8DB6:
case 0x8FB6:
case 0x81B7:
case 0x83B7:
case 0x85B7:
case 0x87B7:
case 0x89B7:
case 0x8BB7:
case 0x8DB7:
case 0x8FB7:

// ORDa
case 0x81B0:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0x83B8:
case 0x85B8:
case 0x87B8:
case 0x89B8:
case 0x8BB8:
case 0x8DB8:
case 0x8FB8:

// ORDa
case 0x81B8:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x83B9:
case 0x85B9:
case 0x87B9:
case 0x89B9:
case 0x8BB9:
case 0x8DB9:
case 0x8FB9:

// ORDa
case 0x81B9:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_LONG_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0x839F:
case 0x859F:
case 0x879F:
case 0x899F:
case 0x8B9F:
case 0x8D9F:
case 0x8F9F:

// ORDa
case 0x819F:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READ_LONG_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x83A7:
case 0x85A7:
case 0x87A7:
case 0x89A7:
case 0x8BA7:
case 0x8DA7:
case 0x8FA7:

// ORDa
case 0x81A7:
{
	u32 adr;
	u32 res;
	u32 src;
	src = (u32)CPU->D[(Opcode >> 9) & 7];
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READ_LONG_F(adr, res)
	res |= src;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x8300:
case 0x8500:
case 0x8700:
case 0x8900:
case 0x8B00:
case 0x8D00:
case 0x8F00:
case 0x8101:
case 0x8301:
case 0x8501:
case 0x8701:
case 0x8901:
case 0x8B01:
case 0x8D01:
case 0x8F01:
case 0x8102:
case 0x8302:
case 0x8502:
case 0x8702:
case 0x8902:
case 0x8B02:
case 0x8D02:
case 0x8F02:
case 0x8103:
case 0x8303:
case 0x8503:
case 0x8703:
case 0x8903:
case 0x8B03:
case 0x8D03:
case 0x8F03:
case 0x8104:
case 0x8304:
case 0x8504:
case 0x8704:
case 0x8904:
case 0x8B04:
case 0x8D04:
case 0x8F04:
case 0x8105:
case 0x8305:
case 0x8505:
case 0x8705:
case 0x8905:
case 0x8B05:
case 0x8D05:
case 0x8F05:
case 0x8106:
case 0x8306:
case 0x8506:
case 0x8706:
case 0x8906:
case 0x8B06:
case 0x8D06:
case 0x8F06:
case 0x8107:
case 0x8307:
case 0x8507:
case 0x8707:
case 0x8907:
case 0x8B07:
case 0x8D07:
case 0x8F07:

// SBCD
case 0x8100:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u8)CPU->D[(Opcode >> 0) & 7];
	dst = (u8)CPU->D[(Opcode >> 9) & 7];
	res = (dst & 0xF) - (src & 0xF) - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);
	if (res > 9) res -= 6;
	res += (dst & 0xF0) - (src & 0xF0);
	if (res > 0x99)
	{
		res += 0xA0;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_notZ |= res & 0xFF;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(6)
case 0x8308:
case 0x8508:
case 0x8708:
case 0x8908:
case 0x8B08:
case 0x8D08:
case 0x8109:
case 0x8309:
case 0x8509:
case 0x8709:
case 0x8909:
case 0x8B09:
case 0x8D09:
case 0x810A:
case 0x830A:
case 0x850A:
case 0x870A:
case 0x890A:
case 0x8B0A:
case 0x8D0A:
case 0x810B:
case 0x830B:
case 0x850B:
case 0x870B:
case 0x890B:
case 0x8B0B:
case 0x8D0B:
case 0x810C:
case 0x830C:
case 0x850C:
case 0x870C:
case 0x890C:
case 0x8B0C:
case 0x8D0C:
case 0x810D:
case 0x830D:
case 0x850D:
case 0x870D:
case 0x890D:
case 0x8B0D:
case 0x8D0D:
case 0x810E:
case 0x830E:
case 0x850E:
case 0x870E:
case 0x890E:
case 0x8B0E:
case 0x8D0E:

// SBCDM
case 0x8108:
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
	res = (dst & 0xF) - (src & 0xF) - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);
	if (res > 9) res -= 6;
	res += (dst & 0xF0) - (src & 0xF0);
	if (res > 0x99)
	{
		res += 0xA0;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_notZ |= res & 0xFF;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x830F:
case 0x850F:
case 0x870F:
case 0x890F:
case 0x8B0F:
case 0x8D0F:

// SBCD7M
case 0x810F:
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
	res = (dst & 0xF) - (src & 0xF) - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);
	if (res > 9) res -= 6;
	res += (dst & 0xF0) - (src & 0xF0);
	if (res > 0x99)
	{
		res += 0xA0;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_notZ |= res & 0xFF;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x8F09:
case 0x8F0A:
case 0x8F0B:
case 0x8F0C:
case 0x8F0D:
case 0x8F0E:

// SBCDM7
case 0x8F08:
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
	res = (dst & 0xF) - (src & 0xF) - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);
	if (res > 9) res -= 6;
	res += (dst & 0xF0) - (src & 0xF0);
	if (res > 0x99)
	{
		res += 0xA0;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_notZ |= res & 0xFF;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)

// SBCD7M7
case 0x8F0F:
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
	res = (dst & 0xF) - (src & 0xF) - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);
	if (res > 9) res -= 6;
	res += (dst & 0xF0) - (src & 0xF0);
	if (res > 0x99)
	{
		res += 0xA0;
		CPU->flag_X = CPU->flag_C = C68K_SR_C;
	}
	else CPU->flag_X = CPU->flag_C = 0;
	CPU->flag_notZ |= res & 0xFF;
	CPU->flag_N = res;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x82C0:
case 0x84C0:
case 0x86C0:
case 0x88C0:
case 0x8AC0:
case 0x8CC0:
case 0x8EC0:
case 0x80C1:
case 0x82C1:
case 0x84C1:
case 0x86C1:
case 0x88C1:
case 0x8AC1:
case 0x8CC1:
case 0x8EC1:
case 0x80C2:
case 0x82C2:
case 0x84C2:
case 0x86C2:
case 0x88C2:
case 0x8AC2:
case 0x8CC2:
case 0x8EC2:
case 0x80C3:
case 0x82C3:
case 0x84C3:
case 0x86C3:
case 0x88C3:
case 0x8AC3:
case 0x8CC3:
case 0x8EC3:
case 0x80C4:
case 0x82C4:
case 0x84C4:
case 0x86C4:
case 0x88C4:
case 0x8AC4:
case 0x8CC4:
case 0x8EC4:
case 0x80C5:
case 0x82C5:
case 0x84C5:
case 0x86C5:
case 0x88C5:
case 0x8AC5:
case 0x8CC5:
case 0x8EC5:
case 0x80C6:
case 0x82C6:
case 0x84C6:
case 0x86C6:
case 0x88C6:
case 0x8AC6:
case 0x8CC6:
case 0x8EC6:
case 0x80C7:
case 0x82C7:
case 0x84C7:
case 0x86C7:
case 0x88C7:
case 0x8AC7:
case 0x8CC7:
case 0x8EC7:

// DIVU
case 0x80C0:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (u16)CPU->D[(Opcode >> 0) & 7];
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(10)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	{
		u32 q, r;

		q = dst / src;
		r = dst % src;

		if (q & 0xFFFF0000)
		{
			CPU->flag_V = C68K_SR_V;
	RET(70)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(90)
case 0x82D0:
case 0x84D0:
case 0x86D0:
case 0x88D0:
case 0x8AD0:
case 0x8CD0:
case 0x8ED0:
case 0x80D1:
case 0x82D1:
case 0x84D1:
case 0x86D1:
case 0x88D1:
case 0x8AD1:
case 0x8CD1:
case 0x8ED1:
case 0x80D2:
case 0x82D2:
case 0x84D2:
case 0x86D2:
case 0x88D2:
case 0x8AD2:
case 0x8CD2:
case 0x8ED2:
case 0x80D3:
case 0x82D3:
case 0x84D3:
case 0x86D3:
case 0x88D3:
case 0x8AD3:
case 0x8CD3:
case 0x8ED3:
case 0x80D4:
case 0x82D4:
case 0x84D4:
case 0x86D4:
case 0x88D4:
case 0x8AD4:
case 0x8CD4:
case 0x8ED4:
case 0x80D5:
case 0x82D5:
case 0x84D5:
case 0x86D5:
case 0x88D5:
case 0x8AD5:
case 0x8CD5:
case 0x8ED5:
case 0x80D6:
case 0x82D6:
case 0x84D6:
case 0x86D6:
case 0x88D6:
case 0x8AD6:
case 0x8CD6:
case 0x8ED6:
case 0x80D7:
case 0x82D7:
case 0x84D7:
case 0x86D7:
case 0x88D7:
case 0x8AD7:
case 0x8CD7:
case 0x8ED7:

// DIVU
case 0x80D0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READ_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(14)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	{
		u32 q, r;

		q = dst / src;
		r = dst % src;

		if (q & 0xFFFF0000)
		{
			CPU->flag_V = C68K_SR_V;
	RET(74)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(94)
case 0x82D8:
case 0x84D8:
case 0x86D8:
case 0x88D8:
case 0x8AD8:
case 0x8CD8:
case 0x8ED8:
case 0x80D9:
case 0x82D9:
case 0x84D9:
case 0x86D9:
case 0x88D9:
case 0x8AD9:
case 0x8CD9:
case 0x8ED9:
case 0x80DA:
case 0x82DA:
case 0x84DA:
case 0x86DA:
case 0x88DA:
case 0x8ADA:
case 0x8CDA:
case 0x8EDA:
case 0x80DB:
case 0x82DB:
case 0x84DB:
case 0x86DB:
case 0x88DB:
case 0x8ADB:
case 0x8CDB:
case 0x8EDB:
case 0x80DC:
case 0x82DC:
case 0x84DC:
case 0x86DC:
case 0x88DC:
case 0x8ADC:
case 0x8CDC:
case 0x8EDC:
case 0x80DD:
case 0x82DD:
case 0x84DD:
case 0x86DD:
case 0x88DD:
case 0x8ADD:
case 0x8CDD:
case 0x8EDD:
case 0x80DE:
case 0x82DE:
case 0x84DE:
case 0x86DE:
case 0x88DE:
case 0x8ADE:
case 0x8CDE:
case 0x8EDE:

// DIVU
case 0x80D8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(14)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	{
		u32 q, r;

		q = dst / src;
		r = dst % src;

		if (q & 0xFFFF0000)
		{
			CPU->flag_V = C68K_SR_V;
	RET(74)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(94)
case 0x82E0:
case 0x84E0:
case 0x86E0:
case 0x88E0:
case 0x8AE0:
case 0x8CE0:
case 0x8EE0:
case 0x80E1:
case 0x82E1:
case 0x84E1:
case 0x86E1:
case 0x88E1:
case 0x8AE1:
case 0x8CE1:
case 0x8EE1:
case 0x80E2:
case 0x82E2:
case 0x84E2:
case 0x86E2:
case 0x88E2:
case 0x8AE2:
case 0x8CE2:
case 0x8EE2:
case 0x80E3:
case 0x82E3:
case 0x84E3:
case 0x86E3:
case 0x88E3:
case 0x8AE3:
case 0x8CE3:
case 0x8EE3:
case 0x80E4:
case 0x82E4:
case 0x84E4:
case 0x86E4:
case 0x88E4:
case 0x8AE4:
case 0x8CE4:
case 0x8EE4:
case 0x80E5:
case 0x82E5:
case 0x84E5:
case 0x86E5:
case 0x88E5:
case 0x8AE5:
case 0x8CE5:
case 0x8EE5:
case 0x80E6:
case 0x82E6:
case 0x84E6:
case 0x86E6:
case 0x88E6:
case 0x8AE6:
case 0x8CE6:
case 0x8EE6:

// DIVU
case 0x80E0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(16)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	{
		u32 q, r;

		q = dst / src;
		r = dst % src;

		if (q & 0xFFFF0000)
		{
			CPU->flag_V = C68K_SR_V;
	RET(76)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(96)
case 0x82E8:
case 0x84E8:
case 0x86E8:
case 0x88E8:
case 0x8AE8:
case 0x8CE8:
case 0x8EE8:
case 0x80E9:
case 0x82E9:
case 0x84E9:
case 0x86E9:
case 0x88E9:
case 0x8AE9:
case 0x8CE9:
case 0x8EE9:
case 0x80EA:
case 0x82EA:
case 0x84EA:
case 0x86EA:
case 0x88EA:
case 0x8AEA:
case 0x8CEA:
case 0x8EEA:
case 0x80EB:
case 0x82EB:
case 0x84EB:
case 0x86EB:
case 0x88EB:
case 0x8AEB:
case 0x8CEB:
case 0x8EEB:
case 0x80EC:
case 0x82EC:
case 0x84EC:
case 0x86EC:
case 0x88EC:
case 0x8AEC:
case 0x8CEC:
case 0x8EEC:
case 0x80ED:
case 0x82ED:
case 0x84ED:
case 0x86ED:
case 0x88ED:
case 0x8AED:
case 0x8CED:
case 0x8EED:
case 0x80EE:
case 0x82EE:
case 0x84EE:
case 0x86EE:
case 0x88EE:
case 0x8AEE:
case 0x8CEE:
case 0x8EEE:
case 0x80EF:
case 0x82EF:
case 0x84EF:
case 0x86EF:
case 0x88EF:
case 0x8AEF:
case 0x8CEF:
case 0x8EEF:

// DIVU
case 0x80E8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(18)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	{
		u32 q, r;

		q = dst / src;
		r = dst % src;

		if (q & 0xFFFF0000)
		{
			CPU->flag_V = C68K_SR_V;
	RET(78)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(98)
case 0x82F0:
case 0x84F0:
case 0x86F0:
case 0x88F0:
case 0x8AF0:
case 0x8CF0:
case 0x8EF0:
case 0x80F1:
case 0x82F1:
case 0x84F1:
case 0x86F1:
case 0x88F1:
case 0x8AF1:
case 0x8CF1:
case 0x8EF1:
case 0x80F2:
case 0x82F2:
case 0x84F2:
case 0x86F2:
case 0x88F2:
case 0x8AF2:
case 0x8CF2:
case 0x8EF2:
case 0x80F3:
case 0x82F3:
case 0x84F3:
case 0x86F3:
case 0x88F3:
case 0x8AF3:
case 0x8CF3:
case 0x8EF3:
case 0x80F4:
case 0x82F4:
case 0x84F4:
case 0x86F4:
case 0x88F4:
case 0x8AF4:
case 0x8CF4:
case 0x8EF4:
case 0x80F5:
case 0x82F5:
case 0x84F5:
case 0x86F5:
case 0x88F5:
case 0x8AF5:
case 0x8CF5:
case 0x8EF5:
case 0x80F6:
case 0x82F6:
case 0x84F6:
case 0x86F6:
case 0x88F6:
case 0x8AF6:
case 0x8CF6:
case 0x8EF6:
case 0x80F7:
case 0x82F7:
case 0x84F7:
case 0x86F7:
case 0x88F7:
case 0x8AF7:
case 0x8CF7:
case 0x8EF7:

// DIVU
case 0x80F0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(20)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	{
		u32 q, r;

		q = dst / src;
		r = dst % src;

		if (q & 0xFFFF0000)
		{
			CPU->flag_V = C68K_SR_V;
	RET(80)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(100)
case 0x82F8:
case 0x84F8:
case 0x86F8:
case 0x88F8:
case 0x8AF8:
case 0x8CF8:
case 0x8EF8:

// DIVU
case 0x80F8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(18)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	{
		u32 q, r;

		q = dst / src;
		r = dst % src;

		if (q & 0xFFFF0000)
		{
			CPU->flag_V = C68K_SR_V;
	RET(78)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(98)
case 0x82F9:
case 0x84F9:
case 0x86F9:
case 0x88F9:
case 0x8AF9:
case 0x8CF9:
case 0x8EF9:

// DIVU
case 0x80F9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READ_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(22)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	{
		u32 q, r;

		q = dst / src;
		r = dst % src;

		if (q & 0xFFFF0000)
		{
			CPU->flag_V = C68K_SR_V;
	RET(82)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(102)
case 0x82FA:
case 0x84FA:
case 0x86FA:
case 0x88FA:
case 0x8AFA:
case 0x8CFA:
case 0x8EFA:

// DIVU
case 0x80FA:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(18)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	{
		u32 q, r;

		q = dst / src;
		r = dst % src;

		if (q & 0xFFFF0000)
		{
			CPU->flag_V = C68K_SR_V;
	RET(78)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(98)
case 0x82FB:
case 0x84FB:
case 0x86FB:
case 0x88FB:
case 0x8AFB:
case 0x8CFB:
case 0x8EFB:

// DIVU
case 0x80FB:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(20)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	{
		u32 q, r;

		q = dst / src;
		r = dst % src;

		if (q & 0xFFFF0000)
		{
			CPU->flag_V = C68K_SR_V;
	RET(80)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(100)
case 0x82FC:
case 0x84FC:
case 0x86FC:
case 0x88FC:
case 0x8AFC:
case 0x8CFC:
case 0x8EFC:

// DIVU
case 0x80FC:
{
	u32 res;
	u32 dst;
	u32 src;
	src = FETCH_WORD;
	PC += 2;
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(14)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	{
		u32 q, r;

		q = dst / src;
		r = dst % src;

		if (q & 0xFFFF0000)
		{
			CPU->flag_V = C68K_SR_V;
	RET(74)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(94)
case 0x82DF:
case 0x84DF:
case 0x86DF:
case 0x88DF:
case 0x8ADF:
case 0x8CDF:
case 0x8EDF:

// DIVU
case 0x80DF:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READ_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(14)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	{
		u32 q, r;

		q = dst / src;
		r = dst % src;

		if (q & 0xFFFF0000)
		{
			CPU->flag_V = C68K_SR_V;
	RET(74)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(94)
case 0x82E7:
case 0x84E7:
case 0x86E7:
case 0x88E7:
case 0x8AE7:
case 0x8CE7:
case 0x8EE7:

// DIVU
case 0x80E7:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READ_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(16)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	{
		u32 q, r;

		q = dst / src;
		r = dst % src;

		if (q & 0xFFFF0000)
		{
			CPU->flag_V = C68K_SR_V;
	RET(76)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(96)
case 0x83C0:
case 0x85C0:
case 0x87C0:
case 0x89C0:
case 0x8BC0:
case 0x8DC0:
case 0x8FC0:
case 0x81C1:
case 0x83C1:
case 0x85C1:
case 0x87C1:
case 0x89C1:
case 0x8BC1:
case 0x8DC1:
case 0x8FC1:
case 0x81C2:
case 0x83C2:
case 0x85C2:
case 0x87C2:
case 0x89C2:
case 0x8BC2:
case 0x8DC2:
case 0x8FC2:
case 0x81C3:
case 0x83C3:
case 0x85C3:
case 0x87C3:
case 0x89C3:
case 0x8BC3:
case 0x8DC3:
case 0x8FC3:
case 0x81C4:
case 0x83C4:
case 0x85C4:
case 0x87C4:
case 0x89C4:
case 0x8BC4:
case 0x8DC4:
case 0x8FC4:
case 0x81C5:
case 0x83C5:
case 0x85C5:
case 0x87C5:
case 0x89C5:
case 0x8BC5:
case 0x8DC5:
case 0x8FC5:
case 0x81C6:
case 0x83C6:
case 0x85C6:
case 0x87C6:
case 0x89C6:
case 0x8BC6:
case 0x8DC6:
case 0x8FC6:
case 0x81C7:
case 0x83C7:
case 0x85C7:
case 0x87C7:
case 0x89C7:
case 0x8BC7:
case 0x8DC7:
case 0x8FC7:

// DIVS
case 0x81C0:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s16)CPU->D[(Opcode >> 0) & 7];
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(10)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	if ((dst == 0x80000000) && (src == -1))
	{
		CPU->flag_notZ = CPU->flag_N = 0;
		CPU->flag_V = CPU->flag_C = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	RET(50)
	}
	{
		s32 q, r;

		q = (s32)dst / (s32)src;
		r = (s32)dst % (s32)src;

		if ((q > 0x7FFF) || (q < -0x8000))
		{
			CPU->flag_V = C68K_SR_V;
	RET(80)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(108)
case 0x83D0:
case 0x85D0:
case 0x87D0:
case 0x89D0:
case 0x8BD0:
case 0x8DD0:
case 0x8FD0:
case 0x81D1:
case 0x83D1:
case 0x85D1:
case 0x87D1:
case 0x89D1:
case 0x8BD1:
case 0x8DD1:
case 0x8FD1:
case 0x81D2:
case 0x83D2:
case 0x85D2:
case 0x87D2:
case 0x89D2:
case 0x8BD2:
case 0x8DD2:
case 0x8FD2:
case 0x81D3:
case 0x83D3:
case 0x85D3:
case 0x87D3:
case 0x89D3:
case 0x8BD3:
case 0x8DD3:
case 0x8FD3:
case 0x81D4:
case 0x83D4:
case 0x85D4:
case 0x87D4:
case 0x89D4:
case 0x8BD4:
case 0x8DD4:
case 0x8FD4:
case 0x81D5:
case 0x83D5:
case 0x85D5:
case 0x87D5:
case 0x89D5:
case 0x8BD5:
case 0x8DD5:
case 0x8FD5:
case 0x81D6:
case 0x83D6:
case 0x85D6:
case 0x87D6:
case 0x89D6:
case 0x8BD6:
case 0x8DD6:
case 0x8FD6:
case 0x81D7:
case 0x83D7:
case 0x85D7:
case 0x87D7:
case 0x89D7:
case 0x8BD7:
case 0x8DD7:
case 0x8FD7:

// DIVS
case 0x81D0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READSX_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(14)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	if ((dst == 0x80000000) && (src == -1))
	{
		CPU->flag_notZ = CPU->flag_N = 0;
		CPU->flag_V = CPU->flag_C = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	RET(54)
	}
	{
		s32 q, r;

		q = (s32)dst / (s32)src;
		r = (s32)dst % (s32)src;

		if ((q > 0x7FFF) || (q < -0x8000))
		{
			CPU->flag_V = C68K_SR_V;
	RET(84)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(112)
case 0x83D8:
case 0x85D8:
case 0x87D8:
case 0x89D8:
case 0x8BD8:
case 0x8DD8:
case 0x8FD8:
case 0x81D9:
case 0x83D9:
case 0x85D9:
case 0x87D9:
case 0x89D9:
case 0x8BD9:
case 0x8DD9:
case 0x8FD9:
case 0x81DA:
case 0x83DA:
case 0x85DA:
case 0x87DA:
case 0x89DA:
case 0x8BDA:
case 0x8DDA:
case 0x8FDA:
case 0x81DB:
case 0x83DB:
case 0x85DB:
case 0x87DB:
case 0x89DB:
case 0x8BDB:
case 0x8DDB:
case 0x8FDB:
case 0x81DC:
case 0x83DC:
case 0x85DC:
case 0x87DC:
case 0x89DC:
case 0x8BDC:
case 0x8DDC:
case 0x8FDC:
case 0x81DD:
case 0x83DD:
case 0x85DD:
case 0x87DD:
case 0x89DD:
case 0x8BDD:
case 0x8DDD:
case 0x8FDD:
case 0x81DE:
case 0x83DE:
case 0x85DE:
case 0x87DE:
case 0x89DE:
case 0x8BDE:
case 0x8DDE:
case 0x8FDE:

// DIVS
case 0x81D8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(14)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	if ((dst == 0x80000000) && (src == -1))
	{
		CPU->flag_notZ = CPU->flag_N = 0;
		CPU->flag_V = CPU->flag_C = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	RET(54)
	}
	{
		s32 q, r;

		q = (s32)dst / (s32)src;
		r = (s32)dst % (s32)src;

		if ((q > 0x7FFF) || (q < -0x8000))
		{
			CPU->flag_V = C68K_SR_V;
	RET(84)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(112)
case 0x83E0:
case 0x85E0:
case 0x87E0:
case 0x89E0:
case 0x8BE0:
case 0x8DE0:
case 0x8FE0:
case 0x81E1:
case 0x83E1:
case 0x85E1:
case 0x87E1:
case 0x89E1:
case 0x8BE1:
case 0x8DE1:
case 0x8FE1:
case 0x81E2:
case 0x83E2:
case 0x85E2:
case 0x87E2:
case 0x89E2:
case 0x8BE2:
case 0x8DE2:
case 0x8FE2:
case 0x81E3:
case 0x83E3:
case 0x85E3:
case 0x87E3:
case 0x89E3:
case 0x8BE3:
case 0x8DE3:
case 0x8FE3:
case 0x81E4:
case 0x83E4:
case 0x85E4:
case 0x87E4:
case 0x89E4:
case 0x8BE4:
case 0x8DE4:
case 0x8FE4:
case 0x81E5:
case 0x83E5:
case 0x85E5:
case 0x87E5:
case 0x89E5:
case 0x8BE5:
case 0x8DE5:
case 0x8FE5:
case 0x81E6:
case 0x83E6:
case 0x85E6:
case 0x87E6:
case 0x89E6:
case 0x8BE6:
case 0x8DE6:
case 0x8FE6:

// DIVS
case 0x81E0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READSX_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(16)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	if ((dst == 0x80000000) && (src == -1))
	{
		CPU->flag_notZ = CPU->flag_N = 0;
		CPU->flag_V = CPU->flag_C = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	RET(56)
	}
	{
		s32 q, r;

		q = (s32)dst / (s32)src;
		r = (s32)dst % (s32)src;

		if ((q > 0x7FFF) || (q < -0x8000))
		{
			CPU->flag_V = C68K_SR_V;
	RET(86)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(114)
case 0x83E8:
case 0x85E8:
case 0x87E8:
case 0x89E8:
case 0x8BE8:
case 0x8DE8:
case 0x8FE8:
case 0x81E9:
case 0x83E9:
case 0x85E9:
case 0x87E9:
case 0x89E9:
case 0x8BE9:
case 0x8DE9:
case 0x8FE9:
case 0x81EA:
case 0x83EA:
case 0x85EA:
case 0x87EA:
case 0x89EA:
case 0x8BEA:
case 0x8DEA:
case 0x8FEA:
case 0x81EB:
case 0x83EB:
case 0x85EB:
case 0x87EB:
case 0x89EB:
case 0x8BEB:
case 0x8DEB:
case 0x8FEB:
case 0x81EC:
case 0x83EC:
case 0x85EC:
case 0x87EC:
case 0x89EC:
case 0x8BEC:
case 0x8DEC:
case 0x8FEC:
case 0x81ED:
case 0x83ED:
case 0x85ED:
case 0x87ED:
case 0x89ED:
case 0x8BED:
case 0x8DED:
case 0x8FED:
case 0x81EE:
case 0x83EE:
case 0x85EE:
case 0x87EE:
case 0x89EE:
case 0x8BEE:
case 0x8DEE:
case 0x8FEE:
case 0x81EF:
case 0x83EF:
case 0x85EF:
case 0x87EF:
case 0x89EF:
case 0x8BEF:
case 0x8DEF:
case 0x8FEF:

// DIVS
case 0x81E8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(18)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	if ((dst == 0x80000000) && (src == -1))
	{
		CPU->flag_notZ = CPU->flag_N = 0;
		CPU->flag_V = CPU->flag_C = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	RET(58)
	}
	{
		s32 q, r;

		q = (s32)dst / (s32)src;
		r = (s32)dst % (s32)src;

		if ((q > 0x7FFF) || (q < -0x8000))
		{
			CPU->flag_V = C68K_SR_V;
	RET(88)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(116)
case 0x83F0:
case 0x85F0:
case 0x87F0:
case 0x89F0:
case 0x8BF0:
case 0x8DF0:
case 0x8FF0:
case 0x81F1:
case 0x83F1:
case 0x85F1:
case 0x87F1:
case 0x89F1:
case 0x8BF1:
case 0x8DF1:
case 0x8FF1:
case 0x81F2:
case 0x83F2:
case 0x85F2:
case 0x87F2:
case 0x89F2:
case 0x8BF2:
case 0x8DF2:
case 0x8FF2:
case 0x81F3:
case 0x83F3:
case 0x85F3:
case 0x87F3:
case 0x89F3:
case 0x8BF3:
case 0x8DF3:
case 0x8FF3:
case 0x81F4:
case 0x83F4:
case 0x85F4:
case 0x87F4:
case 0x89F4:
case 0x8BF4:
case 0x8DF4:
case 0x8FF4:
case 0x81F5:
case 0x83F5:
case 0x85F5:
case 0x87F5:
case 0x89F5:
case 0x8BF5:
case 0x8DF5:
case 0x8FF5:
case 0x81F6:
case 0x83F6:
case 0x85F6:
case 0x87F6:
case 0x89F6:
case 0x8BF6:
case 0x8DF6:
case 0x8FF6:
case 0x81F7:
case 0x83F7:
case 0x85F7:
case 0x87F7:
case 0x89F7:
case 0x8BF7:
case 0x8DF7:
case 0x8FF7:

// DIVS
case 0x81F0:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READSX_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(20)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	if ((dst == 0x80000000) && (src == -1))
	{
		CPU->flag_notZ = CPU->flag_N = 0;
		CPU->flag_V = CPU->flag_C = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	RET(60)
	}
	{
		s32 q, r;

		q = (s32)dst / (s32)src;
		r = (s32)dst % (s32)src;

		if ((q > 0x7FFF) || (q < -0x8000))
		{
			CPU->flag_V = C68K_SR_V;
	RET(90)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(118)
case 0x83F8:
case 0x85F8:
case 0x87F8:
case 0x89F8:
case 0x8BF8:
case 0x8DF8:
case 0x8FF8:

// DIVS
case 0x81F8:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(18)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	if ((dst == 0x80000000) && (src == -1))
	{
		CPU->flag_notZ = CPU->flag_N = 0;
		CPU->flag_V = CPU->flag_C = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	RET(58)
	}
	{
		s32 q, r;

		q = (s32)dst / (s32)src;
		r = (s32)dst % (s32)src;

		if ((q > 0x7FFF) || (q < -0x8000))
		{
			CPU->flag_V = C68K_SR_V;
	RET(88)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(116)
case 0x83F9:
case 0x85F9:
case 0x87F9:
case 0x89F9:
case 0x8BF9:
case 0x8DF9:
case 0x8FF9:

// DIVS
case 0x81F9:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READSX_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(22)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	if ((dst == 0x80000000) && (src == -1))
	{
		CPU->flag_notZ = CPU->flag_N = 0;
		CPU->flag_V = CPU->flag_C = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	RET(62)
	}
	{
		s32 q, r;

		q = (s32)dst / (s32)src;
		r = (s32)dst % (s32)src;

		if ((q > 0x7FFF) || (q < -0x8000))
		{
			CPU->flag_V = C68K_SR_V;
	RET(92)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(120)
case 0x83FA:
case 0x85FA:
case 0x87FA:
case 0x89FA:
case 0x8BFA:
case 0x8DFA:
case 0x8FFA:

// DIVS
case 0x81FA:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(18)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	if ((dst == 0x80000000) && (src == -1))
	{
		CPU->flag_notZ = CPU->flag_N = 0;
		CPU->flag_V = CPU->flag_C = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	RET(58)
	}
	{
		s32 q, r;

		q = (s32)dst / (s32)src;
		r = (s32)dst % (s32)src;

		if ((q > 0x7FFF) || (q < -0x8000))
		{
			CPU->flag_V = C68K_SR_V;
	RET(88)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(116)
case 0x83FB:
case 0x85FB:
case 0x87FB:
case 0x89FB:
case 0x8BFB:
case 0x8DFB:
case 0x8FFB:

// DIVS
case 0x81FB:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READSX_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(20)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	if ((dst == 0x80000000) && (src == -1))
	{
		CPU->flag_notZ = CPU->flag_N = 0;
		CPU->flag_V = CPU->flag_C = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	RET(60)
	}
	{
		s32 q, r;

		q = (s32)dst / (s32)src;
		r = (s32)dst % (s32)src;

		if ((q > 0x7FFF) || (q < -0x8000))
		{
			CPU->flag_V = C68K_SR_V;
	RET(90)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(118)
case 0x83FC:
case 0x85FC:
case 0x87FC:
case 0x89FC:
case 0x8BFC:
case 0x8DFC:
case 0x8FFC:

// DIVS
case 0x81FC:
{
	u32 res;
	u32 dst;
	u32 src;
	src = (s32)(s16)FETCH_WORD;
	PC += 2;
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(14)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	if ((dst == 0x80000000) && (src == -1))
	{
		CPU->flag_notZ = CPU->flag_N = 0;
		CPU->flag_V = CPU->flag_C = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	RET(54)
	}
	{
		s32 q, r;

		q = (s32)dst / (s32)src;
		r = (s32)dst % (s32)src;

		if ((q > 0x7FFF) || (q < -0x8000))
		{
			CPU->flag_V = C68K_SR_V;
	RET(84)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(112)
case 0x83DF:
case 0x85DF:
case 0x87DF:
case 0x89DF:
case 0x8BDF:
case 0x8DDF:
case 0x8FDF:

// DIVS
case 0x81DF:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READSX_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(14)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	if ((dst == 0x80000000) && (src == -1))
	{
		CPU->flag_notZ = CPU->flag_N = 0;
		CPU->flag_V = CPU->flag_C = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	RET(54)
	}
	{
		s32 q, r;

		q = (s32)dst / (s32)src;
		r = (s32)dst % (s32)src;

		if ((q > 0x7FFF) || (q < -0x8000))
		{
			CPU->flag_V = C68K_SR_V;
	RET(84)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(112)
case 0x83E7:
case 0x85E7:
case 0x87E7:
case 0x89E7:
case 0x8BE7:
case 0x8DE7:
case 0x8FE7:

// DIVS
case 0x81E7:
{
	u32 adr;
	u32 res;
	u32 dst;
	u32 src;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READSX_WORD_F(adr, src)
	if (src == 0)
	{
		if (!CPU->flag_S)
		{
			res = CPU->USP;
			CPU->USP = CPU->A[7];
			CPU->A[7] = res;
		}
		res = C68K_ZERO_DIVIDE_EX;
	POST_IO
	CCnt -= c68k_exception_cycle_table[res];
	PRE_IO
		PUSH_32_F(PC)
		PUSH_16_F(GET_SR)
		CPU->flag_S = C68K_SR_S;
		READ_LONG_F(res * 4, PC)
		SET_PC(PC)
	POST_IO
	RET(16)
	}
	dst = (u32)CPU->D[(Opcode >> 9) & 7];
	if ((dst == 0x80000000) && (src == -1))
	{
		CPU->flag_notZ = CPU->flag_N = 0;
		CPU->flag_V = CPU->flag_C = 0;
		res = 0;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	RET(56)
	}
	{
		s32 q, r;

		q = (s32)dst / (s32)src;
		r = (s32)dst % (s32)src;

		if ((q > 0x7FFF) || (q < -0x8000))
		{
			CPU->flag_V = C68K_SR_V;
	RET(86)
		}
		q &= 0x0000FFFF;
		CPU->flag_notZ = q;
		CPU->flag_N = q >> 8;
		CPU->flag_V = CPU->flag_C = 0;
		res = q | (r << 16);
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	}
}
RET(114)
