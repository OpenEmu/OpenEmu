case 0x1200:
case 0x1400:
case 0x1600:
case 0x1800:
case 0x1A00:
case 0x1C00:
case 0x1E00:
case 0x1001:
case 0x1201:
case 0x1401:
case 0x1601:
case 0x1801:
case 0x1A01:
case 0x1C01:
case 0x1E01:
case 0x1002:
case 0x1202:
case 0x1402:
case 0x1602:
case 0x1802:
case 0x1A02:
case 0x1C02:
case 0x1E02:
case 0x1003:
case 0x1203:
case 0x1403:
case 0x1603:
case 0x1803:
case 0x1A03:
case 0x1C03:
case 0x1E03:
case 0x1004:
case 0x1204:
case 0x1404:
case 0x1604:
case 0x1804:
case 0x1A04:
case 0x1C04:
case 0x1E04:
case 0x1005:
case 0x1205:
case 0x1405:
case 0x1605:
case 0x1805:
case 0x1A05:
case 0x1C05:
case 0x1E05:
case 0x1006:
case 0x1206:
case 0x1406:
case 0x1606:
case 0x1806:
case 0x1A06:
case 0x1C06:
case 0x1E06:
case 0x1007:
case 0x1207:
case 0x1407:
case 0x1607:
case 0x1807:
case 0x1A07:
case 0x1C07:
case 0x1E07:

// MOVEB
case 0x1000:
{
	u32 res;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0x1280:
case 0x1480:
case 0x1680:
case 0x1880:
case 0x1A80:
case 0x1C80:
case 0x1E80:
case 0x1081:
case 0x1281:
case 0x1481:
case 0x1681:
case 0x1881:
case 0x1A81:
case 0x1C81:
case 0x1E81:
case 0x1082:
case 0x1282:
case 0x1482:
case 0x1682:
case 0x1882:
case 0x1A82:
case 0x1C82:
case 0x1E82:
case 0x1083:
case 0x1283:
case 0x1483:
case 0x1683:
case 0x1883:
case 0x1A83:
case 0x1C83:
case 0x1E83:
case 0x1084:
case 0x1284:
case 0x1484:
case 0x1684:
case 0x1884:
case 0x1A84:
case 0x1C84:
case 0x1E84:
case 0x1085:
case 0x1285:
case 0x1485:
case 0x1685:
case 0x1885:
case 0x1A85:
case 0x1C85:
case 0x1E85:
case 0x1086:
case 0x1286:
case 0x1486:
case 0x1686:
case 0x1886:
case 0x1A86:
case 0x1C86:
case 0x1E86:
case 0x1087:
case 0x1287:
case 0x1487:
case 0x1687:
case 0x1887:
case 0x1A87:
case 0x1C87:
case 0x1E87:

// MOVEB
case 0x1080:
{
	u32 adr;
	u32 res;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(8)
case 0x12C0:
case 0x14C0:
case 0x16C0:
case 0x18C0:
case 0x1AC0:
case 0x1CC0:
case 0x10C1:
case 0x12C1:
case 0x14C1:
case 0x16C1:
case 0x18C1:
case 0x1AC1:
case 0x1CC1:
case 0x10C2:
case 0x12C2:
case 0x14C2:
case 0x16C2:
case 0x18C2:
case 0x1AC2:
case 0x1CC2:
case 0x10C3:
case 0x12C3:
case 0x14C3:
case 0x16C3:
case 0x18C3:
case 0x1AC3:
case 0x1CC3:
case 0x10C4:
case 0x12C4:
case 0x14C4:
case 0x16C4:
case 0x18C4:
case 0x1AC4:
case 0x1CC4:
case 0x10C5:
case 0x12C5:
case 0x14C5:
case 0x16C5:
case 0x18C5:
case 0x1AC5:
case 0x1CC5:
case 0x10C6:
case 0x12C6:
case 0x14C6:
case 0x16C6:
case 0x18C6:
case 0x1AC6:
case 0x1CC6:
case 0x10C7:
case 0x12C7:
case 0x14C7:
case 0x16C7:
case 0x18C7:
case 0x1AC7:
case 0x1CC7:

// MOVEB
case 0x10C0:
{
	u32 adr;
	u32 res;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(8)
case 0x1300:
case 0x1500:
case 0x1700:
case 0x1900:
case 0x1B00:
case 0x1D00:
case 0x1101:
case 0x1301:
case 0x1501:
case 0x1701:
case 0x1901:
case 0x1B01:
case 0x1D01:
case 0x1102:
case 0x1302:
case 0x1502:
case 0x1702:
case 0x1902:
case 0x1B02:
case 0x1D02:
case 0x1103:
case 0x1303:
case 0x1503:
case 0x1703:
case 0x1903:
case 0x1B03:
case 0x1D03:
case 0x1104:
case 0x1304:
case 0x1504:
case 0x1704:
case 0x1904:
case 0x1B04:
case 0x1D04:
case 0x1105:
case 0x1305:
case 0x1505:
case 0x1705:
case 0x1905:
case 0x1B05:
case 0x1D05:
case 0x1106:
case 0x1306:
case 0x1506:
case 0x1706:
case 0x1906:
case 0x1B06:
case 0x1D06:
case 0x1107:
case 0x1307:
case 0x1507:
case 0x1707:
case 0x1907:
case 0x1B07:
case 0x1D07:

// MOVEB
case 0x1100:
{
	u32 adr;
	u32 res;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(8)
case 0x1340:
case 0x1540:
case 0x1740:
case 0x1940:
case 0x1B40:
case 0x1D40:
case 0x1F40:
case 0x1141:
case 0x1341:
case 0x1541:
case 0x1741:
case 0x1941:
case 0x1B41:
case 0x1D41:
case 0x1F41:
case 0x1142:
case 0x1342:
case 0x1542:
case 0x1742:
case 0x1942:
case 0x1B42:
case 0x1D42:
case 0x1F42:
case 0x1143:
case 0x1343:
case 0x1543:
case 0x1743:
case 0x1943:
case 0x1B43:
case 0x1D43:
case 0x1F43:
case 0x1144:
case 0x1344:
case 0x1544:
case 0x1744:
case 0x1944:
case 0x1B44:
case 0x1D44:
case 0x1F44:
case 0x1145:
case 0x1345:
case 0x1545:
case 0x1745:
case 0x1945:
case 0x1B45:
case 0x1D45:
case 0x1F45:
case 0x1146:
case 0x1346:
case 0x1546:
case 0x1746:
case 0x1946:
case 0x1B46:
case 0x1D46:
case 0x1F46:
case 0x1147:
case 0x1347:
case 0x1547:
case 0x1747:
case 0x1947:
case 0x1B47:
case 0x1D47:
case 0x1F47:

// MOVEB
case 0x1140:
{
	u32 adr;
	u32 res;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x1380:
case 0x1580:
case 0x1780:
case 0x1980:
case 0x1B80:
case 0x1D80:
case 0x1F80:
case 0x1181:
case 0x1381:
case 0x1581:
case 0x1781:
case 0x1981:
case 0x1B81:
case 0x1D81:
case 0x1F81:
case 0x1182:
case 0x1382:
case 0x1582:
case 0x1782:
case 0x1982:
case 0x1B82:
case 0x1D82:
case 0x1F82:
case 0x1183:
case 0x1383:
case 0x1583:
case 0x1783:
case 0x1983:
case 0x1B83:
case 0x1D83:
case 0x1F83:
case 0x1184:
case 0x1384:
case 0x1584:
case 0x1784:
case 0x1984:
case 0x1B84:
case 0x1D84:
case 0x1F84:
case 0x1185:
case 0x1385:
case 0x1585:
case 0x1785:
case 0x1985:
case 0x1B85:
case 0x1D85:
case 0x1F85:
case 0x1186:
case 0x1386:
case 0x1586:
case 0x1786:
case 0x1986:
case 0x1B86:
case 0x1D86:
case 0x1F86:
case 0x1187:
case 0x1387:
case 0x1587:
case 0x1787:
case 0x1987:
case 0x1B87:
case 0x1D87:
case 0x1F87:

// MOVEB
case 0x1180:
{
	u32 adr;
	u32 res;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x11C1:
case 0x11C2:
case 0x11C3:
case 0x11C4:
case 0x11C5:
case 0x11C6:
case 0x11C7:

// MOVEB
case 0x11C0:
{
	u32 adr;
	u32 res;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x13C1:
case 0x13C2:
case 0x13C3:
case 0x13C4:
case 0x13C5:
case 0x13C6:
case 0x13C7:

// MOVEB
case 0x13C0:
{
	u32 adr;
	u32 res;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x1EC1:
case 0x1EC2:
case 0x1EC3:
case 0x1EC4:
case 0x1EC5:
case 0x1EC6:
case 0x1EC7:

// MOVEB
case 0x1EC0:
{
	u32 adr;
	u32 res;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(8)
case 0x1F01:
case 0x1F02:
case 0x1F03:
case 0x1F04:
case 0x1F05:
case 0x1F06:
case 0x1F07:

// MOVEB
case 0x1F00:
{
	u32 adr;
	u32 res;
	res = (u8)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(8)
case 0x1208:
case 0x1408:
case 0x1608:
case 0x1808:
case 0x1A08:
case 0x1C08:
case 0x1E08:
case 0x1009:
case 0x1209:
case 0x1409:
case 0x1609:
case 0x1809:
case 0x1A09:
case 0x1C09:
case 0x1E09:
case 0x100A:
case 0x120A:
case 0x140A:
case 0x160A:
case 0x180A:
case 0x1A0A:
case 0x1C0A:
case 0x1E0A:
case 0x100B:
case 0x120B:
case 0x140B:
case 0x160B:
case 0x180B:
case 0x1A0B:
case 0x1C0B:
case 0x1E0B:
case 0x100C:
case 0x120C:
case 0x140C:
case 0x160C:
case 0x180C:
case 0x1A0C:
case 0x1C0C:
case 0x1E0C:
case 0x100D:
case 0x120D:
case 0x140D:
case 0x160D:
case 0x180D:
case 0x1A0D:
case 0x1C0D:
case 0x1E0D:
case 0x100E:
case 0x120E:
case 0x140E:
case 0x160E:
case 0x180E:
case 0x1A0E:
case 0x1C0E:
case 0x1E0E:
case 0x100F:
case 0x120F:
case 0x140F:
case 0x160F:
case 0x180F:
case 0x1A0F:
case 0x1C0F:
case 0x1E0F:

// MOVEB
case 0x1008:
{
	u32 res;
	// can't read byte from Ax registers !
	CPU->Status |= C68K_FAULTED;
	CCnt = 0;
	goto C68k_Exec_Really_End;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0x1288:
case 0x1488:
case 0x1688:
case 0x1888:
case 0x1A88:
case 0x1C88:
case 0x1E88:
case 0x1089:
case 0x1289:
case 0x1489:
case 0x1689:
case 0x1889:
case 0x1A89:
case 0x1C89:
case 0x1E89:
case 0x108A:
case 0x128A:
case 0x148A:
case 0x168A:
case 0x188A:
case 0x1A8A:
case 0x1C8A:
case 0x1E8A:
case 0x108B:
case 0x128B:
case 0x148B:
case 0x168B:
case 0x188B:
case 0x1A8B:
case 0x1C8B:
case 0x1E8B:
case 0x108C:
case 0x128C:
case 0x148C:
case 0x168C:
case 0x188C:
case 0x1A8C:
case 0x1C8C:
case 0x1E8C:
case 0x108D:
case 0x128D:
case 0x148D:
case 0x168D:
case 0x188D:
case 0x1A8D:
case 0x1C8D:
case 0x1E8D:
case 0x108E:
case 0x128E:
case 0x148E:
case 0x168E:
case 0x188E:
case 0x1A8E:
case 0x1C8E:
case 0x1E8E:
case 0x108F:
case 0x128F:
case 0x148F:
case 0x168F:
case 0x188F:
case 0x1A8F:
case 0x1C8F:
case 0x1E8F:

// MOVEB
case 0x1088:
{
	u32 adr;
	u32 res;
	// can't read byte from Ax registers !
	CPU->Status |= C68K_FAULTED;
	CCnt = 0;
	goto C68k_Exec_Really_End;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(8)
case 0x12C8:
case 0x14C8:
case 0x16C8:
case 0x18C8:
case 0x1AC8:
case 0x1CC8:
case 0x10C9:
case 0x12C9:
case 0x14C9:
case 0x16C9:
case 0x18C9:
case 0x1AC9:
case 0x1CC9:
case 0x10CA:
case 0x12CA:
case 0x14CA:
case 0x16CA:
case 0x18CA:
case 0x1ACA:
case 0x1CCA:
case 0x10CB:
case 0x12CB:
case 0x14CB:
case 0x16CB:
case 0x18CB:
case 0x1ACB:
case 0x1CCB:
case 0x10CC:
case 0x12CC:
case 0x14CC:
case 0x16CC:
case 0x18CC:
case 0x1ACC:
case 0x1CCC:
case 0x10CD:
case 0x12CD:
case 0x14CD:
case 0x16CD:
case 0x18CD:
case 0x1ACD:
case 0x1CCD:
case 0x10CE:
case 0x12CE:
case 0x14CE:
case 0x16CE:
case 0x18CE:
case 0x1ACE:
case 0x1CCE:
case 0x10CF:
case 0x12CF:
case 0x14CF:
case 0x16CF:
case 0x18CF:
case 0x1ACF:
case 0x1CCF:

// MOVEB
case 0x10C8:
{
	u32 adr;
	u32 res;
	// can't read byte from Ax registers !
	CPU->Status |= C68K_FAULTED;
	CCnt = 0;
	goto C68k_Exec_Really_End;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(8)
case 0x1308:
case 0x1508:
case 0x1708:
case 0x1908:
case 0x1B08:
case 0x1D08:
case 0x1109:
case 0x1309:
case 0x1509:
case 0x1709:
case 0x1909:
case 0x1B09:
case 0x1D09:
case 0x110A:
case 0x130A:
case 0x150A:
case 0x170A:
case 0x190A:
case 0x1B0A:
case 0x1D0A:
case 0x110B:
case 0x130B:
case 0x150B:
case 0x170B:
case 0x190B:
case 0x1B0B:
case 0x1D0B:
case 0x110C:
case 0x130C:
case 0x150C:
case 0x170C:
case 0x190C:
case 0x1B0C:
case 0x1D0C:
case 0x110D:
case 0x130D:
case 0x150D:
case 0x170D:
case 0x190D:
case 0x1B0D:
case 0x1D0D:
case 0x110E:
case 0x130E:
case 0x150E:
case 0x170E:
case 0x190E:
case 0x1B0E:
case 0x1D0E:
case 0x110F:
case 0x130F:
case 0x150F:
case 0x170F:
case 0x190F:
case 0x1B0F:
case 0x1D0F:

// MOVEB
case 0x1108:
{
	u32 adr;
	u32 res;
	// can't read byte from Ax registers !
	CPU->Status |= C68K_FAULTED;
	CCnt = 0;
	goto C68k_Exec_Really_End;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(8)
case 0x1348:
case 0x1548:
case 0x1748:
case 0x1948:
case 0x1B48:
case 0x1D48:
case 0x1F48:
case 0x1149:
case 0x1349:
case 0x1549:
case 0x1749:
case 0x1949:
case 0x1B49:
case 0x1D49:
case 0x1F49:
case 0x114A:
case 0x134A:
case 0x154A:
case 0x174A:
case 0x194A:
case 0x1B4A:
case 0x1D4A:
case 0x1F4A:
case 0x114B:
case 0x134B:
case 0x154B:
case 0x174B:
case 0x194B:
case 0x1B4B:
case 0x1D4B:
case 0x1F4B:
case 0x114C:
case 0x134C:
case 0x154C:
case 0x174C:
case 0x194C:
case 0x1B4C:
case 0x1D4C:
case 0x1F4C:
case 0x114D:
case 0x134D:
case 0x154D:
case 0x174D:
case 0x194D:
case 0x1B4D:
case 0x1D4D:
case 0x1F4D:
case 0x114E:
case 0x134E:
case 0x154E:
case 0x174E:
case 0x194E:
case 0x1B4E:
case 0x1D4E:
case 0x1F4E:
case 0x114F:
case 0x134F:
case 0x154F:
case 0x174F:
case 0x194F:
case 0x1B4F:
case 0x1D4F:
case 0x1F4F:

// MOVEB
case 0x1148:
{
	u32 adr;
	u32 res;
	// can't read byte from Ax registers !
	CPU->Status |= C68K_FAULTED;
	CCnt = 0;
	goto C68k_Exec_Really_End;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x1388:
case 0x1588:
case 0x1788:
case 0x1988:
case 0x1B88:
case 0x1D88:
case 0x1F88:
case 0x1189:
case 0x1389:
case 0x1589:
case 0x1789:
case 0x1989:
case 0x1B89:
case 0x1D89:
case 0x1F89:
case 0x118A:
case 0x138A:
case 0x158A:
case 0x178A:
case 0x198A:
case 0x1B8A:
case 0x1D8A:
case 0x1F8A:
case 0x118B:
case 0x138B:
case 0x158B:
case 0x178B:
case 0x198B:
case 0x1B8B:
case 0x1D8B:
case 0x1F8B:
case 0x118C:
case 0x138C:
case 0x158C:
case 0x178C:
case 0x198C:
case 0x1B8C:
case 0x1D8C:
case 0x1F8C:
case 0x118D:
case 0x138D:
case 0x158D:
case 0x178D:
case 0x198D:
case 0x1B8D:
case 0x1D8D:
case 0x1F8D:
case 0x118E:
case 0x138E:
case 0x158E:
case 0x178E:
case 0x198E:
case 0x1B8E:
case 0x1D8E:
case 0x1F8E:
case 0x118F:
case 0x138F:
case 0x158F:
case 0x178F:
case 0x198F:
case 0x1B8F:
case 0x1D8F:
case 0x1F8F:

// MOVEB
case 0x1188:
{
	u32 adr;
	u32 res;
	// can't read byte from Ax registers !
	CPU->Status |= C68K_FAULTED;
	CCnt = 0;
	goto C68k_Exec_Really_End;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x11C9:
case 0x11CA:
case 0x11CB:
case 0x11CC:
case 0x11CD:
case 0x11CE:
case 0x11CF:

// MOVEB
case 0x11C8:
{
	u32 adr;
	u32 res;
	// can't read byte from Ax registers !
	CPU->Status |= C68K_FAULTED;
	CCnt = 0;
	goto C68k_Exec_Really_End;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x13C9:
case 0x13CA:
case 0x13CB:
case 0x13CC:
case 0x13CD:
case 0x13CE:
case 0x13CF:

// MOVEB
case 0x13C8:
{
	u32 adr;
	u32 res;
	// can't read byte from Ax registers !
	CPU->Status |= C68K_FAULTED;
	CCnt = 0;
	goto C68k_Exec_Really_End;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x1EC9:
case 0x1ECA:
case 0x1ECB:
case 0x1ECC:
case 0x1ECD:
case 0x1ECE:
case 0x1ECF:

// MOVEB
case 0x1EC8:
{
	u32 adr;
	u32 res;
	// can't read byte from Ax registers !
	CPU->Status |= C68K_FAULTED;
	CCnt = 0;
	goto C68k_Exec_Really_End;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(8)
case 0x1F09:
case 0x1F0A:
case 0x1F0B:
case 0x1F0C:
case 0x1F0D:
case 0x1F0E:
case 0x1F0F:

// MOVEB
case 0x1F08:
{
	u32 adr;
	u32 res;
	// can't read byte from Ax registers !
	CPU->Status |= C68K_FAULTED;
	CCnt = 0;
	goto C68k_Exec_Really_End;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(8)
case 0x1210:
case 0x1410:
case 0x1610:
case 0x1810:
case 0x1A10:
case 0x1C10:
case 0x1E10:
case 0x1011:
case 0x1211:
case 0x1411:
case 0x1611:
case 0x1811:
case 0x1A11:
case 0x1C11:
case 0x1E11:
case 0x1012:
case 0x1212:
case 0x1412:
case 0x1612:
case 0x1812:
case 0x1A12:
case 0x1C12:
case 0x1E12:
case 0x1013:
case 0x1213:
case 0x1413:
case 0x1613:
case 0x1813:
case 0x1A13:
case 0x1C13:
case 0x1E13:
case 0x1014:
case 0x1214:
case 0x1414:
case 0x1614:
case 0x1814:
case 0x1A14:
case 0x1C14:
case 0x1E14:
case 0x1015:
case 0x1215:
case 0x1415:
case 0x1615:
case 0x1815:
case 0x1A15:
case 0x1C15:
case 0x1E15:
case 0x1016:
case 0x1216:
case 0x1416:
case 0x1616:
case 0x1816:
case 0x1A16:
case 0x1C16:
case 0x1E16:
case 0x1017:
case 0x1217:
case 0x1417:
case 0x1617:
case 0x1817:
case 0x1A17:
case 0x1C17:
case 0x1E17:

// MOVEB
case 0x1010:
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
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x1290:
case 0x1490:
case 0x1690:
case 0x1890:
case 0x1A90:
case 0x1C90:
case 0x1E90:
case 0x1091:
case 0x1291:
case 0x1491:
case 0x1691:
case 0x1891:
case 0x1A91:
case 0x1C91:
case 0x1E91:
case 0x1092:
case 0x1292:
case 0x1492:
case 0x1692:
case 0x1892:
case 0x1A92:
case 0x1C92:
case 0x1E92:
case 0x1093:
case 0x1293:
case 0x1493:
case 0x1693:
case 0x1893:
case 0x1A93:
case 0x1C93:
case 0x1E93:
case 0x1094:
case 0x1294:
case 0x1494:
case 0x1694:
case 0x1894:
case 0x1A94:
case 0x1C94:
case 0x1E94:
case 0x1095:
case 0x1295:
case 0x1495:
case 0x1695:
case 0x1895:
case 0x1A95:
case 0x1C95:
case 0x1E95:
case 0x1096:
case 0x1296:
case 0x1496:
case 0x1696:
case 0x1896:
case 0x1A96:
case 0x1C96:
case 0x1E96:
case 0x1097:
case 0x1297:
case 0x1497:
case 0x1697:
case 0x1897:
case 0x1A97:
case 0x1C97:
case 0x1E97:

// MOVEB
case 0x1090:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x12D0:
case 0x14D0:
case 0x16D0:
case 0x18D0:
case 0x1AD0:
case 0x1CD0:
case 0x10D1:
case 0x12D1:
case 0x14D1:
case 0x16D1:
case 0x18D1:
case 0x1AD1:
case 0x1CD1:
case 0x10D2:
case 0x12D2:
case 0x14D2:
case 0x16D2:
case 0x18D2:
case 0x1AD2:
case 0x1CD2:
case 0x10D3:
case 0x12D3:
case 0x14D3:
case 0x16D3:
case 0x18D3:
case 0x1AD3:
case 0x1CD3:
case 0x10D4:
case 0x12D4:
case 0x14D4:
case 0x16D4:
case 0x18D4:
case 0x1AD4:
case 0x1CD4:
case 0x10D5:
case 0x12D5:
case 0x14D5:
case 0x16D5:
case 0x18D5:
case 0x1AD5:
case 0x1CD5:
case 0x10D6:
case 0x12D6:
case 0x14D6:
case 0x16D6:
case 0x18D6:
case 0x1AD6:
case 0x1CD6:
case 0x10D7:
case 0x12D7:
case 0x14D7:
case 0x16D7:
case 0x18D7:
case 0x1AD7:
case 0x1CD7:

// MOVEB
case 0x10D0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x1310:
case 0x1510:
case 0x1710:
case 0x1910:
case 0x1B10:
case 0x1D10:
case 0x1111:
case 0x1311:
case 0x1511:
case 0x1711:
case 0x1911:
case 0x1B11:
case 0x1D11:
case 0x1112:
case 0x1312:
case 0x1512:
case 0x1712:
case 0x1912:
case 0x1B12:
case 0x1D12:
case 0x1113:
case 0x1313:
case 0x1513:
case 0x1713:
case 0x1913:
case 0x1B13:
case 0x1D13:
case 0x1114:
case 0x1314:
case 0x1514:
case 0x1714:
case 0x1914:
case 0x1B14:
case 0x1D14:
case 0x1115:
case 0x1315:
case 0x1515:
case 0x1715:
case 0x1915:
case 0x1B15:
case 0x1D15:
case 0x1116:
case 0x1316:
case 0x1516:
case 0x1716:
case 0x1916:
case 0x1B16:
case 0x1D16:
case 0x1117:
case 0x1317:
case 0x1517:
case 0x1717:
case 0x1917:
case 0x1B17:
case 0x1D17:

// MOVEB
case 0x1110:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x1350:
case 0x1550:
case 0x1750:
case 0x1950:
case 0x1B50:
case 0x1D50:
case 0x1F50:
case 0x1151:
case 0x1351:
case 0x1551:
case 0x1751:
case 0x1951:
case 0x1B51:
case 0x1D51:
case 0x1F51:
case 0x1152:
case 0x1352:
case 0x1552:
case 0x1752:
case 0x1952:
case 0x1B52:
case 0x1D52:
case 0x1F52:
case 0x1153:
case 0x1353:
case 0x1553:
case 0x1753:
case 0x1953:
case 0x1B53:
case 0x1D53:
case 0x1F53:
case 0x1154:
case 0x1354:
case 0x1554:
case 0x1754:
case 0x1954:
case 0x1B54:
case 0x1D54:
case 0x1F54:
case 0x1155:
case 0x1355:
case 0x1555:
case 0x1755:
case 0x1955:
case 0x1B55:
case 0x1D55:
case 0x1F55:
case 0x1156:
case 0x1356:
case 0x1556:
case 0x1756:
case 0x1956:
case 0x1B56:
case 0x1D56:
case 0x1F56:
case 0x1157:
case 0x1357:
case 0x1557:
case 0x1757:
case 0x1957:
case 0x1B57:
case 0x1D57:
case 0x1F57:

// MOVEB
case 0x1150:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x1390:
case 0x1590:
case 0x1790:
case 0x1990:
case 0x1B90:
case 0x1D90:
case 0x1F90:
case 0x1191:
case 0x1391:
case 0x1591:
case 0x1791:
case 0x1991:
case 0x1B91:
case 0x1D91:
case 0x1F91:
case 0x1192:
case 0x1392:
case 0x1592:
case 0x1792:
case 0x1992:
case 0x1B92:
case 0x1D92:
case 0x1F92:
case 0x1193:
case 0x1393:
case 0x1593:
case 0x1793:
case 0x1993:
case 0x1B93:
case 0x1D93:
case 0x1F93:
case 0x1194:
case 0x1394:
case 0x1594:
case 0x1794:
case 0x1994:
case 0x1B94:
case 0x1D94:
case 0x1F94:
case 0x1195:
case 0x1395:
case 0x1595:
case 0x1795:
case 0x1995:
case 0x1B95:
case 0x1D95:
case 0x1F95:
case 0x1196:
case 0x1396:
case 0x1596:
case 0x1796:
case 0x1996:
case 0x1B96:
case 0x1D96:
case 0x1F96:
case 0x1197:
case 0x1397:
case 0x1597:
case 0x1797:
case 0x1997:
case 0x1B97:
case 0x1D97:
case 0x1F97:

// MOVEB
case 0x1190:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x11D1:
case 0x11D2:
case 0x11D3:
case 0x11D4:
case 0x11D5:
case 0x11D6:
case 0x11D7:

// MOVEB
case 0x11D0:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x13D1:
case 0x13D2:
case 0x13D3:
case 0x13D4:
case 0x13D5:
case 0x13D6:
case 0x13D7:

// MOVEB
case 0x13D0:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0x1ED1:
case 0x1ED2:
case 0x1ED3:
case 0x1ED4:
case 0x1ED5:
case 0x1ED6:
case 0x1ED7:

// MOVEB
case 0x1ED0:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x1F11:
case 0x1F12:
case 0x1F13:
case 0x1F14:
case 0x1F15:
case 0x1F16:
case 0x1F17:

// MOVEB
case 0x1F10:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x1218:
case 0x1418:
case 0x1618:
case 0x1818:
case 0x1A18:
case 0x1C18:
case 0x1E18:
case 0x1019:
case 0x1219:
case 0x1419:
case 0x1619:
case 0x1819:
case 0x1A19:
case 0x1C19:
case 0x1E19:
case 0x101A:
case 0x121A:
case 0x141A:
case 0x161A:
case 0x181A:
case 0x1A1A:
case 0x1C1A:
case 0x1E1A:
case 0x101B:
case 0x121B:
case 0x141B:
case 0x161B:
case 0x181B:
case 0x1A1B:
case 0x1C1B:
case 0x1E1B:
case 0x101C:
case 0x121C:
case 0x141C:
case 0x161C:
case 0x181C:
case 0x1A1C:
case 0x1C1C:
case 0x1E1C:
case 0x101D:
case 0x121D:
case 0x141D:
case 0x161D:
case 0x181D:
case 0x1A1D:
case 0x1C1D:
case 0x1E1D:
case 0x101E:
case 0x121E:
case 0x141E:
case 0x161E:
case 0x181E:
case 0x1A1E:
case 0x1C1E:
case 0x1E1E:

// MOVEB
case 0x1018:
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
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x1298:
case 0x1498:
case 0x1698:
case 0x1898:
case 0x1A98:
case 0x1C98:
case 0x1E98:
case 0x1099:
case 0x1299:
case 0x1499:
case 0x1699:
case 0x1899:
case 0x1A99:
case 0x1C99:
case 0x1E99:
case 0x109A:
case 0x129A:
case 0x149A:
case 0x169A:
case 0x189A:
case 0x1A9A:
case 0x1C9A:
case 0x1E9A:
case 0x109B:
case 0x129B:
case 0x149B:
case 0x169B:
case 0x189B:
case 0x1A9B:
case 0x1C9B:
case 0x1E9B:
case 0x109C:
case 0x129C:
case 0x149C:
case 0x169C:
case 0x189C:
case 0x1A9C:
case 0x1C9C:
case 0x1E9C:
case 0x109D:
case 0x129D:
case 0x149D:
case 0x169D:
case 0x189D:
case 0x1A9D:
case 0x1C9D:
case 0x1E9D:
case 0x109E:
case 0x129E:
case 0x149E:
case 0x169E:
case 0x189E:
case 0x1A9E:
case 0x1C9E:
case 0x1E9E:

// MOVEB
case 0x1098:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x12D8:
case 0x14D8:
case 0x16D8:
case 0x18D8:
case 0x1AD8:
case 0x1CD8:
case 0x10D9:
case 0x12D9:
case 0x14D9:
case 0x16D9:
case 0x18D9:
case 0x1AD9:
case 0x1CD9:
case 0x10DA:
case 0x12DA:
case 0x14DA:
case 0x16DA:
case 0x18DA:
case 0x1ADA:
case 0x1CDA:
case 0x10DB:
case 0x12DB:
case 0x14DB:
case 0x16DB:
case 0x18DB:
case 0x1ADB:
case 0x1CDB:
case 0x10DC:
case 0x12DC:
case 0x14DC:
case 0x16DC:
case 0x18DC:
case 0x1ADC:
case 0x1CDC:
case 0x10DD:
case 0x12DD:
case 0x14DD:
case 0x16DD:
case 0x18DD:
case 0x1ADD:
case 0x1CDD:
case 0x10DE:
case 0x12DE:
case 0x14DE:
case 0x16DE:
case 0x18DE:
case 0x1ADE:
case 0x1CDE:

// MOVEB
case 0x10D8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x1318:
case 0x1518:
case 0x1718:
case 0x1918:
case 0x1B18:
case 0x1D18:
case 0x1119:
case 0x1319:
case 0x1519:
case 0x1719:
case 0x1919:
case 0x1B19:
case 0x1D19:
case 0x111A:
case 0x131A:
case 0x151A:
case 0x171A:
case 0x191A:
case 0x1B1A:
case 0x1D1A:
case 0x111B:
case 0x131B:
case 0x151B:
case 0x171B:
case 0x191B:
case 0x1B1B:
case 0x1D1B:
case 0x111C:
case 0x131C:
case 0x151C:
case 0x171C:
case 0x191C:
case 0x1B1C:
case 0x1D1C:
case 0x111D:
case 0x131D:
case 0x151D:
case 0x171D:
case 0x191D:
case 0x1B1D:
case 0x1D1D:
case 0x111E:
case 0x131E:
case 0x151E:
case 0x171E:
case 0x191E:
case 0x1B1E:
case 0x1D1E:

// MOVEB
case 0x1118:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x1358:
case 0x1558:
case 0x1758:
case 0x1958:
case 0x1B58:
case 0x1D58:
case 0x1F58:
case 0x1159:
case 0x1359:
case 0x1559:
case 0x1759:
case 0x1959:
case 0x1B59:
case 0x1D59:
case 0x1F59:
case 0x115A:
case 0x135A:
case 0x155A:
case 0x175A:
case 0x195A:
case 0x1B5A:
case 0x1D5A:
case 0x1F5A:
case 0x115B:
case 0x135B:
case 0x155B:
case 0x175B:
case 0x195B:
case 0x1B5B:
case 0x1D5B:
case 0x1F5B:
case 0x115C:
case 0x135C:
case 0x155C:
case 0x175C:
case 0x195C:
case 0x1B5C:
case 0x1D5C:
case 0x1F5C:
case 0x115D:
case 0x135D:
case 0x155D:
case 0x175D:
case 0x195D:
case 0x1B5D:
case 0x1D5D:
case 0x1F5D:
case 0x115E:
case 0x135E:
case 0x155E:
case 0x175E:
case 0x195E:
case 0x1B5E:
case 0x1D5E:
case 0x1F5E:

// MOVEB
case 0x1158:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x1398:
case 0x1598:
case 0x1798:
case 0x1998:
case 0x1B98:
case 0x1D98:
case 0x1F98:
case 0x1199:
case 0x1399:
case 0x1599:
case 0x1799:
case 0x1999:
case 0x1B99:
case 0x1D99:
case 0x1F99:
case 0x119A:
case 0x139A:
case 0x159A:
case 0x179A:
case 0x199A:
case 0x1B9A:
case 0x1D9A:
case 0x1F9A:
case 0x119B:
case 0x139B:
case 0x159B:
case 0x179B:
case 0x199B:
case 0x1B9B:
case 0x1D9B:
case 0x1F9B:
case 0x119C:
case 0x139C:
case 0x159C:
case 0x179C:
case 0x199C:
case 0x1B9C:
case 0x1D9C:
case 0x1F9C:
case 0x119D:
case 0x139D:
case 0x159D:
case 0x179D:
case 0x199D:
case 0x1B9D:
case 0x1D9D:
case 0x1F9D:
case 0x119E:
case 0x139E:
case 0x159E:
case 0x179E:
case 0x199E:
case 0x1B9E:
case 0x1D9E:
case 0x1F9E:

// MOVEB
case 0x1198:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x11D9:
case 0x11DA:
case 0x11DB:
case 0x11DC:
case 0x11DD:
case 0x11DE:

// MOVEB
case 0x11D8:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x13D9:
case 0x13DA:
case 0x13DB:
case 0x13DC:
case 0x13DD:
case 0x13DE:

// MOVEB
case 0x13D8:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0x1ED9:
case 0x1EDA:
case 0x1EDB:
case 0x1EDC:
case 0x1EDD:
case 0x1EDE:

// MOVEB
case 0x1ED8:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x1F19:
case 0x1F1A:
case 0x1F1B:
case 0x1F1C:
case 0x1F1D:
case 0x1F1E:

// MOVEB
case 0x1F18:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x1220:
case 0x1420:
case 0x1620:
case 0x1820:
case 0x1A20:
case 0x1C20:
case 0x1E20:
case 0x1021:
case 0x1221:
case 0x1421:
case 0x1621:
case 0x1821:
case 0x1A21:
case 0x1C21:
case 0x1E21:
case 0x1022:
case 0x1222:
case 0x1422:
case 0x1622:
case 0x1822:
case 0x1A22:
case 0x1C22:
case 0x1E22:
case 0x1023:
case 0x1223:
case 0x1423:
case 0x1623:
case 0x1823:
case 0x1A23:
case 0x1C23:
case 0x1E23:
case 0x1024:
case 0x1224:
case 0x1424:
case 0x1624:
case 0x1824:
case 0x1A24:
case 0x1C24:
case 0x1E24:
case 0x1025:
case 0x1225:
case 0x1425:
case 0x1625:
case 0x1825:
case 0x1A25:
case 0x1C25:
case 0x1E25:
case 0x1026:
case 0x1226:
case 0x1426:
case 0x1626:
case 0x1826:
case 0x1A26:
case 0x1C26:
case 0x1E26:

// MOVEB
case 0x1020:
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
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0x12A0:
case 0x14A0:
case 0x16A0:
case 0x18A0:
case 0x1AA0:
case 0x1CA0:
case 0x1EA0:
case 0x10A1:
case 0x12A1:
case 0x14A1:
case 0x16A1:
case 0x18A1:
case 0x1AA1:
case 0x1CA1:
case 0x1EA1:
case 0x10A2:
case 0x12A2:
case 0x14A2:
case 0x16A2:
case 0x18A2:
case 0x1AA2:
case 0x1CA2:
case 0x1EA2:
case 0x10A3:
case 0x12A3:
case 0x14A3:
case 0x16A3:
case 0x18A3:
case 0x1AA3:
case 0x1CA3:
case 0x1EA3:
case 0x10A4:
case 0x12A4:
case 0x14A4:
case 0x16A4:
case 0x18A4:
case 0x1AA4:
case 0x1CA4:
case 0x1EA4:
case 0x10A5:
case 0x12A5:
case 0x14A5:
case 0x16A5:
case 0x18A5:
case 0x1AA5:
case 0x1CA5:
case 0x1EA5:
case 0x10A6:
case 0x12A6:
case 0x14A6:
case 0x16A6:
case 0x18A6:
case 0x1AA6:
case 0x1CA6:
case 0x1EA6:

// MOVEB
case 0x10A0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x12E0:
case 0x14E0:
case 0x16E0:
case 0x18E0:
case 0x1AE0:
case 0x1CE0:
case 0x10E1:
case 0x12E1:
case 0x14E1:
case 0x16E1:
case 0x18E1:
case 0x1AE1:
case 0x1CE1:
case 0x10E2:
case 0x12E2:
case 0x14E2:
case 0x16E2:
case 0x18E2:
case 0x1AE2:
case 0x1CE2:
case 0x10E3:
case 0x12E3:
case 0x14E3:
case 0x16E3:
case 0x18E3:
case 0x1AE3:
case 0x1CE3:
case 0x10E4:
case 0x12E4:
case 0x14E4:
case 0x16E4:
case 0x18E4:
case 0x1AE4:
case 0x1CE4:
case 0x10E5:
case 0x12E5:
case 0x14E5:
case 0x16E5:
case 0x18E5:
case 0x1AE5:
case 0x1CE5:
case 0x10E6:
case 0x12E6:
case 0x14E6:
case 0x16E6:
case 0x18E6:
case 0x1AE6:
case 0x1CE6:

// MOVEB
case 0x10E0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x1320:
case 0x1520:
case 0x1720:
case 0x1920:
case 0x1B20:
case 0x1D20:
case 0x1121:
case 0x1321:
case 0x1521:
case 0x1721:
case 0x1921:
case 0x1B21:
case 0x1D21:
case 0x1122:
case 0x1322:
case 0x1522:
case 0x1722:
case 0x1922:
case 0x1B22:
case 0x1D22:
case 0x1123:
case 0x1323:
case 0x1523:
case 0x1723:
case 0x1923:
case 0x1B23:
case 0x1D23:
case 0x1124:
case 0x1324:
case 0x1524:
case 0x1724:
case 0x1924:
case 0x1B24:
case 0x1D24:
case 0x1125:
case 0x1325:
case 0x1525:
case 0x1725:
case 0x1925:
case 0x1B25:
case 0x1D25:
case 0x1126:
case 0x1326:
case 0x1526:
case 0x1726:
case 0x1926:
case 0x1B26:
case 0x1D26:

// MOVEB
case 0x1120:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x1360:
case 0x1560:
case 0x1760:
case 0x1960:
case 0x1B60:
case 0x1D60:
case 0x1F60:
case 0x1161:
case 0x1361:
case 0x1561:
case 0x1761:
case 0x1961:
case 0x1B61:
case 0x1D61:
case 0x1F61:
case 0x1162:
case 0x1362:
case 0x1562:
case 0x1762:
case 0x1962:
case 0x1B62:
case 0x1D62:
case 0x1F62:
case 0x1163:
case 0x1363:
case 0x1563:
case 0x1763:
case 0x1963:
case 0x1B63:
case 0x1D63:
case 0x1F63:
case 0x1164:
case 0x1364:
case 0x1564:
case 0x1764:
case 0x1964:
case 0x1B64:
case 0x1D64:
case 0x1F64:
case 0x1165:
case 0x1365:
case 0x1565:
case 0x1765:
case 0x1965:
case 0x1B65:
case 0x1D65:
case 0x1F65:
case 0x1166:
case 0x1366:
case 0x1566:
case 0x1766:
case 0x1966:
case 0x1B66:
case 0x1D66:
case 0x1F66:

// MOVEB
case 0x1160:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x13A0:
case 0x15A0:
case 0x17A0:
case 0x19A0:
case 0x1BA0:
case 0x1DA0:
case 0x1FA0:
case 0x11A1:
case 0x13A1:
case 0x15A1:
case 0x17A1:
case 0x19A1:
case 0x1BA1:
case 0x1DA1:
case 0x1FA1:
case 0x11A2:
case 0x13A2:
case 0x15A2:
case 0x17A2:
case 0x19A2:
case 0x1BA2:
case 0x1DA2:
case 0x1FA2:
case 0x11A3:
case 0x13A3:
case 0x15A3:
case 0x17A3:
case 0x19A3:
case 0x1BA3:
case 0x1DA3:
case 0x1FA3:
case 0x11A4:
case 0x13A4:
case 0x15A4:
case 0x17A4:
case 0x19A4:
case 0x1BA4:
case 0x1DA4:
case 0x1FA4:
case 0x11A5:
case 0x13A5:
case 0x15A5:
case 0x17A5:
case 0x19A5:
case 0x1BA5:
case 0x1DA5:
case 0x1FA5:
case 0x11A6:
case 0x13A6:
case 0x15A6:
case 0x17A6:
case 0x19A6:
case 0x1BA6:
case 0x1DA6:
case 0x1FA6:

// MOVEB
case 0x11A0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0x11E1:
case 0x11E2:
case 0x11E3:
case 0x11E4:
case 0x11E5:
case 0x11E6:

// MOVEB
case 0x11E0:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x13E1:
case 0x13E2:
case 0x13E3:
case 0x13E4:
case 0x13E5:
case 0x13E6:

// MOVEB
case 0x13E0:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(22)
case 0x1EE1:
case 0x1EE2:
case 0x1EE3:
case 0x1EE4:
case 0x1EE5:
case 0x1EE6:

// MOVEB
case 0x1EE0:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x1F21:
case 0x1F22:
case 0x1F23:
case 0x1F24:
case 0x1F25:
case 0x1F26:

// MOVEB
case 0x1F20:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x1228:
case 0x1428:
case 0x1628:
case 0x1828:
case 0x1A28:
case 0x1C28:
case 0x1E28:
case 0x1029:
case 0x1229:
case 0x1429:
case 0x1629:
case 0x1829:
case 0x1A29:
case 0x1C29:
case 0x1E29:
case 0x102A:
case 0x122A:
case 0x142A:
case 0x162A:
case 0x182A:
case 0x1A2A:
case 0x1C2A:
case 0x1E2A:
case 0x102B:
case 0x122B:
case 0x142B:
case 0x162B:
case 0x182B:
case 0x1A2B:
case 0x1C2B:
case 0x1E2B:
case 0x102C:
case 0x122C:
case 0x142C:
case 0x162C:
case 0x182C:
case 0x1A2C:
case 0x1C2C:
case 0x1E2C:
case 0x102D:
case 0x122D:
case 0x142D:
case 0x162D:
case 0x182D:
case 0x1A2D:
case 0x1C2D:
case 0x1E2D:
case 0x102E:
case 0x122E:
case 0x142E:
case 0x162E:
case 0x182E:
case 0x1A2E:
case 0x1C2E:
case 0x1E2E:
case 0x102F:
case 0x122F:
case 0x142F:
case 0x162F:
case 0x182F:
case 0x1A2F:
case 0x1C2F:
case 0x1E2F:

// MOVEB
case 0x1028:
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
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x12A8:
case 0x14A8:
case 0x16A8:
case 0x18A8:
case 0x1AA8:
case 0x1CA8:
case 0x1EA8:
case 0x10A9:
case 0x12A9:
case 0x14A9:
case 0x16A9:
case 0x18A9:
case 0x1AA9:
case 0x1CA9:
case 0x1EA9:
case 0x10AA:
case 0x12AA:
case 0x14AA:
case 0x16AA:
case 0x18AA:
case 0x1AAA:
case 0x1CAA:
case 0x1EAA:
case 0x10AB:
case 0x12AB:
case 0x14AB:
case 0x16AB:
case 0x18AB:
case 0x1AAB:
case 0x1CAB:
case 0x1EAB:
case 0x10AC:
case 0x12AC:
case 0x14AC:
case 0x16AC:
case 0x18AC:
case 0x1AAC:
case 0x1CAC:
case 0x1EAC:
case 0x10AD:
case 0x12AD:
case 0x14AD:
case 0x16AD:
case 0x18AD:
case 0x1AAD:
case 0x1CAD:
case 0x1EAD:
case 0x10AE:
case 0x12AE:
case 0x14AE:
case 0x16AE:
case 0x18AE:
case 0x1AAE:
case 0x1CAE:
case 0x1EAE:
case 0x10AF:
case 0x12AF:
case 0x14AF:
case 0x16AF:
case 0x18AF:
case 0x1AAF:
case 0x1CAF:
case 0x1EAF:

// MOVEB
case 0x10A8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x12E8:
case 0x14E8:
case 0x16E8:
case 0x18E8:
case 0x1AE8:
case 0x1CE8:
case 0x10E9:
case 0x12E9:
case 0x14E9:
case 0x16E9:
case 0x18E9:
case 0x1AE9:
case 0x1CE9:
case 0x10EA:
case 0x12EA:
case 0x14EA:
case 0x16EA:
case 0x18EA:
case 0x1AEA:
case 0x1CEA:
case 0x10EB:
case 0x12EB:
case 0x14EB:
case 0x16EB:
case 0x18EB:
case 0x1AEB:
case 0x1CEB:
case 0x10EC:
case 0x12EC:
case 0x14EC:
case 0x16EC:
case 0x18EC:
case 0x1AEC:
case 0x1CEC:
case 0x10ED:
case 0x12ED:
case 0x14ED:
case 0x16ED:
case 0x18ED:
case 0x1AED:
case 0x1CED:
case 0x10EE:
case 0x12EE:
case 0x14EE:
case 0x16EE:
case 0x18EE:
case 0x1AEE:
case 0x1CEE:
case 0x10EF:
case 0x12EF:
case 0x14EF:
case 0x16EF:
case 0x18EF:
case 0x1AEF:
case 0x1CEF:

// MOVEB
case 0x10E8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x1328:
case 0x1528:
case 0x1728:
case 0x1928:
case 0x1B28:
case 0x1D28:
case 0x1129:
case 0x1329:
case 0x1529:
case 0x1729:
case 0x1929:
case 0x1B29:
case 0x1D29:
case 0x112A:
case 0x132A:
case 0x152A:
case 0x172A:
case 0x192A:
case 0x1B2A:
case 0x1D2A:
case 0x112B:
case 0x132B:
case 0x152B:
case 0x172B:
case 0x192B:
case 0x1B2B:
case 0x1D2B:
case 0x112C:
case 0x132C:
case 0x152C:
case 0x172C:
case 0x192C:
case 0x1B2C:
case 0x1D2C:
case 0x112D:
case 0x132D:
case 0x152D:
case 0x172D:
case 0x192D:
case 0x1B2D:
case 0x1D2D:
case 0x112E:
case 0x132E:
case 0x152E:
case 0x172E:
case 0x192E:
case 0x1B2E:
case 0x1D2E:
case 0x112F:
case 0x132F:
case 0x152F:
case 0x172F:
case 0x192F:
case 0x1B2F:
case 0x1D2F:

// MOVEB
case 0x1128:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x1368:
case 0x1568:
case 0x1768:
case 0x1968:
case 0x1B68:
case 0x1D68:
case 0x1F68:
case 0x1169:
case 0x1369:
case 0x1569:
case 0x1769:
case 0x1969:
case 0x1B69:
case 0x1D69:
case 0x1F69:
case 0x116A:
case 0x136A:
case 0x156A:
case 0x176A:
case 0x196A:
case 0x1B6A:
case 0x1D6A:
case 0x1F6A:
case 0x116B:
case 0x136B:
case 0x156B:
case 0x176B:
case 0x196B:
case 0x1B6B:
case 0x1D6B:
case 0x1F6B:
case 0x116C:
case 0x136C:
case 0x156C:
case 0x176C:
case 0x196C:
case 0x1B6C:
case 0x1D6C:
case 0x1F6C:
case 0x116D:
case 0x136D:
case 0x156D:
case 0x176D:
case 0x196D:
case 0x1B6D:
case 0x1D6D:
case 0x1F6D:
case 0x116E:
case 0x136E:
case 0x156E:
case 0x176E:
case 0x196E:
case 0x1B6E:
case 0x1D6E:
case 0x1F6E:
case 0x116F:
case 0x136F:
case 0x156F:
case 0x176F:
case 0x196F:
case 0x1B6F:
case 0x1D6F:
case 0x1F6F:

// MOVEB
case 0x1168:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0x13A8:
case 0x15A8:
case 0x17A8:
case 0x19A8:
case 0x1BA8:
case 0x1DA8:
case 0x1FA8:
case 0x11A9:
case 0x13A9:
case 0x15A9:
case 0x17A9:
case 0x19A9:
case 0x1BA9:
case 0x1DA9:
case 0x1FA9:
case 0x11AA:
case 0x13AA:
case 0x15AA:
case 0x17AA:
case 0x19AA:
case 0x1BAA:
case 0x1DAA:
case 0x1FAA:
case 0x11AB:
case 0x13AB:
case 0x15AB:
case 0x17AB:
case 0x19AB:
case 0x1BAB:
case 0x1DAB:
case 0x1FAB:
case 0x11AC:
case 0x13AC:
case 0x15AC:
case 0x17AC:
case 0x19AC:
case 0x1BAC:
case 0x1DAC:
case 0x1FAC:
case 0x11AD:
case 0x13AD:
case 0x15AD:
case 0x17AD:
case 0x19AD:
case 0x1BAD:
case 0x1DAD:
case 0x1FAD:
case 0x11AE:
case 0x13AE:
case 0x15AE:
case 0x17AE:
case 0x19AE:
case 0x1BAE:
case 0x1DAE:
case 0x1FAE:
case 0x11AF:
case 0x13AF:
case 0x15AF:
case 0x17AF:
case 0x19AF:
case 0x1BAF:
case 0x1DAF:
case 0x1FAF:

// MOVEB
case 0x11A8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(22)
case 0x11E9:
case 0x11EA:
case 0x11EB:
case 0x11EC:
case 0x11ED:
case 0x11EE:
case 0x11EF:

// MOVEB
case 0x11E8:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0x13E9:
case 0x13EA:
case 0x13EB:
case 0x13EC:
case 0x13ED:
case 0x13EE:
case 0x13EF:

// MOVEB
case 0x13E8:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(24)
case 0x1EE9:
case 0x1EEA:
case 0x1EEB:
case 0x1EEC:
case 0x1EED:
case 0x1EEE:
case 0x1EEF:

// MOVEB
case 0x1EE8:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x1F29:
case 0x1F2A:
case 0x1F2B:
case 0x1F2C:
case 0x1F2D:
case 0x1F2E:
case 0x1F2F:

// MOVEB
case 0x1F28:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x1230:
case 0x1430:
case 0x1630:
case 0x1830:
case 0x1A30:
case 0x1C30:
case 0x1E30:
case 0x1031:
case 0x1231:
case 0x1431:
case 0x1631:
case 0x1831:
case 0x1A31:
case 0x1C31:
case 0x1E31:
case 0x1032:
case 0x1232:
case 0x1432:
case 0x1632:
case 0x1832:
case 0x1A32:
case 0x1C32:
case 0x1E32:
case 0x1033:
case 0x1233:
case 0x1433:
case 0x1633:
case 0x1833:
case 0x1A33:
case 0x1C33:
case 0x1E33:
case 0x1034:
case 0x1234:
case 0x1434:
case 0x1634:
case 0x1834:
case 0x1A34:
case 0x1C34:
case 0x1E34:
case 0x1035:
case 0x1235:
case 0x1435:
case 0x1635:
case 0x1835:
case 0x1A35:
case 0x1C35:
case 0x1E35:
case 0x1036:
case 0x1236:
case 0x1436:
case 0x1636:
case 0x1836:
case 0x1A36:
case 0x1C36:
case 0x1E36:
case 0x1037:
case 0x1237:
case 0x1437:
case 0x1637:
case 0x1837:
case 0x1A37:
case 0x1C37:
case 0x1E37:

// MOVEB
case 0x1030:
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
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x12B0:
case 0x14B0:
case 0x16B0:
case 0x18B0:
case 0x1AB0:
case 0x1CB0:
case 0x1EB0:
case 0x10B1:
case 0x12B1:
case 0x14B1:
case 0x16B1:
case 0x18B1:
case 0x1AB1:
case 0x1CB1:
case 0x1EB1:
case 0x10B2:
case 0x12B2:
case 0x14B2:
case 0x16B2:
case 0x18B2:
case 0x1AB2:
case 0x1CB2:
case 0x1EB2:
case 0x10B3:
case 0x12B3:
case 0x14B3:
case 0x16B3:
case 0x18B3:
case 0x1AB3:
case 0x1CB3:
case 0x1EB3:
case 0x10B4:
case 0x12B4:
case 0x14B4:
case 0x16B4:
case 0x18B4:
case 0x1AB4:
case 0x1CB4:
case 0x1EB4:
case 0x10B5:
case 0x12B5:
case 0x14B5:
case 0x16B5:
case 0x18B5:
case 0x1AB5:
case 0x1CB5:
case 0x1EB5:
case 0x10B6:
case 0x12B6:
case 0x14B6:
case 0x16B6:
case 0x18B6:
case 0x1AB6:
case 0x1CB6:
case 0x1EB6:
case 0x10B7:
case 0x12B7:
case 0x14B7:
case 0x16B7:
case 0x18B7:
case 0x1AB7:
case 0x1CB7:
case 0x1EB7:

// MOVEB
case 0x10B0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x12F0:
case 0x14F0:
case 0x16F0:
case 0x18F0:
case 0x1AF0:
case 0x1CF0:
case 0x10F1:
case 0x12F1:
case 0x14F1:
case 0x16F1:
case 0x18F1:
case 0x1AF1:
case 0x1CF1:
case 0x10F2:
case 0x12F2:
case 0x14F2:
case 0x16F2:
case 0x18F2:
case 0x1AF2:
case 0x1CF2:
case 0x10F3:
case 0x12F3:
case 0x14F3:
case 0x16F3:
case 0x18F3:
case 0x1AF3:
case 0x1CF3:
case 0x10F4:
case 0x12F4:
case 0x14F4:
case 0x16F4:
case 0x18F4:
case 0x1AF4:
case 0x1CF4:
case 0x10F5:
case 0x12F5:
case 0x14F5:
case 0x16F5:
case 0x18F5:
case 0x1AF5:
case 0x1CF5:
case 0x10F6:
case 0x12F6:
case 0x14F6:
case 0x16F6:
case 0x18F6:
case 0x1AF6:
case 0x1CF6:
case 0x10F7:
case 0x12F7:
case 0x14F7:
case 0x16F7:
case 0x18F7:
case 0x1AF7:
case 0x1CF7:

// MOVEB
case 0x10F0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x1330:
case 0x1530:
case 0x1730:
case 0x1930:
case 0x1B30:
case 0x1D30:
case 0x1131:
case 0x1331:
case 0x1531:
case 0x1731:
case 0x1931:
case 0x1B31:
case 0x1D31:
case 0x1132:
case 0x1332:
case 0x1532:
case 0x1732:
case 0x1932:
case 0x1B32:
case 0x1D32:
case 0x1133:
case 0x1333:
case 0x1533:
case 0x1733:
case 0x1933:
case 0x1B33:
case 0x1D33:
case 0x1134:
case 0x1334:
case 0x1534:
case 0x1734:
case 0x1934:
case 0x1B34:
case 0x1D34:
case 0x1135:
case 0x1335:
case 0x1535:
case 0x1735:
case 0x1935:
case 0x1B35:
case 0x1D35:
case 0x1136:
case 0x1336:
case 0x1536:
case 0x1736:
case 0x1936:
case 0x1B36:
case 0x1D36:
case 0x1137:
case 0x1337:
case 0x1537:
case 0x1737:
case 0x1937:
case 0x1B37:
case 0x1D37:

// MOVEB
case 0x1130:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x1370:
case 0x1570:
case 0x1770:
case 0x1970:
case 0x1B70:
case 0x1D70:
case 0x1F70:
case 0x1171:
case 0x1371:
case 0x1571:
case 0x1771:
case 0x1971:
case 0x1B71:
case 0x1D71:
case 0x1F71:
case 0x1172:
case 0x1372:
case 0x1572:
case 0x1772:
case 0x1972:
case 0x1B72:
case 0x1D72:
case 0x1F72:
case 0x1173:
case 0x1373:
case 0x1573:
case 0x1773:
case 0x1973:
case 0x1B73:
case 0x1D73:
case 0x1F73:
case 0x1174:
case 0x1374:
case 0x1574:
case 0x1774:
case 0x1974:
case 0x1B74:
case 0x1D74:
case 0x1F74:
case 0x1175:
case 0x1375:
case 0x1575:
case 0x1775:
case 0x1975:
case 0x1B75:
case 0x1D75:
case 0x1F75:
case 0x1176:
case 0x1376:
case 0x1576:
case 0x1776:
case 0x1976:
case 0x1B76:
case 0x1D76:
case 0x1F76:
case 0x1177:
case 0x1377:
case 0x1577:
case 0x1777:
case 0x1977:
case 0x1B77:
case 0x1D77:
case 0x1F77:

// MOVEB
case 0x1170:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(22)
case 0x13B0:
case 0x15B0:
case 0x17B0:
case 0x19B0:
case 0x1BB0:
case 0x1DB0:
case 0x1FB0:
case 0x11B1:
case 0x13B1:
case 0x15B1:
case 0x17B1:
case 0x19B1:
case 0x1BB1:
case 0x1DB1:
case 0x1FB1:
case 0x11B2:
case 0x13B2:
case 0x15B2:
case 0x17B2:
case 0x19B2:
case 0x1BB2:
case 0x1DB2:
case 0x1FB2:
case 0x11B3:
case 0x13B3:
case 0x15B3:
case 0x17B3:
case 0x19B3:
case 0x1BB3:
case 0x1DB3:
case 0x1FB3:
case 0x11B4:
case 0x13B4:
case 0x15B4:
case 0x17B4:
case 0x19B4:
case 0x1BB4:
case 0x1DB4:
case 0x1FB4:
case 0x11B5:
case 0x13B5:
case 0x15B5:
case 0x17B5:
case 0x19B5:
case 0x1BB5:
case 0x1DB5:
case 0x1FB5:
case 0x11B6:
case 0x13B6:
case 0x15B6:
case 0x17B6:
case 0x19B6:
case 0x1BB6:
case 0x1DB6:
case 0x1FB6:
case 0x11B7:
case 0x13B7:
case 0x15B7:
case 0x17B7:
case 0x19B7:
case 0x1BB7:
case 0x1DB7:
case 0x1FB7:

// MOVEB
case 0x11B0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(24)
case 0x11F1:
case 0x11F2:
case 0x11F3:
case 0x11F4:
case 0x11F5:
case 0x11F6:
case 0x11F7:

// MOVEB
case 0x11F0:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(22)
case 0x13F1:
case 0x13F2:
case 0x13F3:
case 0x13F4:
case 0x13F5:
case 0x13F6:
case 0x13F7:

// MOVEB
case 0x13F0:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(26)
case 0x1EF1:
case 0x1EF2:
case 0x1EF3:
case 0x1EF4:
case 0x1EF5:
case 0x1EF6:
case 0x1EF7:

// MOVEB
case 0x1EF0:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x1F31:
case 0x1F32:
case 0x1F33:
case 0x1F34:
case 0x1F35:
case 0x1F36:
case 0x1F37:

// MOVEB
case 0x1F30:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x1238:
case 0x1438:
case 0x1638:
case 0x1838:
case 0x1A38:
case 0x1C38:
case 0x1E38:

// MOVEB
case 0x1038:
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
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x12B8:
case 0x14B8:
case 0x16B8:
case 0x18B8:
case 0x1AB8:
case 0x1CB8:
case 0x1EB8:

// MOVEB
case 0x10B8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x12F8:
case 0x14F8:
case 0x16F8:
case 0x18F8:
case 0x1AF8:
case 0x1CF8:

// MOVEB
case 0x10F8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x1338:
case 0x1538:
case 0x1738:
case 0x1938:
case 0x1B38:
case 0x1D38:

// MOVEB
case 0x1138:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x1378:
case 0x1578:
case 0x1778:
case 0x1978:
case 0x1B78:
case 0x1D78:
case 0x1F78:

// MOVEB
case 0x1178:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0x13B8:
case 0x15B8:
case 0x17B8:
case 0x19B8:
case 0x1BB8:
case 0x1DB8:
case 0x1FB8:

// MOVEB
case 0x11B8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(22)

// MOVEB
case 0x11F8:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)

// MOVEB
case 0x13F8:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(24)

// MOVEB
case 0x1EF8:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)

// MOVEB
case 0x1F38:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x1239:
case 0x1439:
case 0x1639:
case 0x1839:
case 0x1A39:
case 0x1C39:
case 0x1E39:

// MOVEB
case 0x1039:
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
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0x12B9:
case 0x14B9:
case 0x16B9:
case 0x18B9:
case 0x1AB9:
case 0x1CB9:
case 0x1EB9:

// MOVEB
case 0x10B9:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0x12F9:
case 0x14F9:
case 0x16F9:
case 0x18F9:
case 0x1AF9:
case 0x1CF9:

// MOVEB
case 0x10F9:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0x1339:
case 0x1539:
case 0x1739:
case 0x1939:
case 0x1B39:
case 0x1D39:

// MOVEB
case 0x1139:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0x1379:
case 0x1579:
case 0x1779:
case 0x1979:
case 0x1B79:
case 0x1D79:
case 0x1F79:

// MOVEB
case 0x1179:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(24)
case 0x13B9:
case 0x15B9:
case 0x17B9:
case 0x19B9:
case 0x1BB9:
case 0x1DB9:
case 0x1FB9:

// MOVEB
case 0x11B9:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(26)

// MOVEB
case 0x11F9:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(24)

// MOVEB
case 0x13F9:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(28)

// MOVEB
case 0x1EF9:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)

// MOVEB
case 0x1F39:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0x123A:
case 0x143A:
case 0x163A:
case 0x183A:
case 0x1A3A:
case 0x1C3A:
case 0x1E3A:

// MOVEB
case 0x103A:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x12BA:
case 0x14BA:
case 0x16BA:
case 0x18BA:
case 0x1ABA:
case 0x1CBA:
case 0x1EBA:

// MOVEB
case 0x10BA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x12FA:
case 0x14FA:
case 0x16FA:
case 0x18FA:
case 0x1AFA:
case 0x1CFA:

// MOVEB
case 0x10FA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x133A:
case 0x153A:
case 0x173A:
case 0x193A:
case 0x1B3A:
case 0x1D3A:

// MOVEB
case 0x113A:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x137A:
case 0x157A:
case 0x177A:
case 0x197A:
case 0x1B7A:
case 0x1D7A:
case 0x1F7A:

// MOVEB
case 0x117A:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)
case 0x13BA:
case 0x15BA:
case 0x17BA:
case 0x19BA:
case 0x1BBA:
case 0x1DBA:
case 0x1FBA:

// MOVEB
case 0x11BA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(22)

// MOVEB
case 0x11FA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)

// MOVEB
case 0x13FA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(24)

// MOVEB
case 0x1EFA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)

// MOVEB
case 0x1F3A:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x123B:
case 0x143B:
case 0x163B:
case 0x183B:
case 0x1A3B:
case 0x1C3B:
case 0x1E3B:

// MOVEB
case 0x103B:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x12BB:
case 0x14BB:
case 0x16BB:
case 0x18BB:
case 0x1ABB:
case 0x1CBB:
case 0x1EBB:

// MOVEB
case 0x10BB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x12FB:
case 0x14FB:
case 0x16FB:
case 0x18FB:
case 0x1AFB:
case 0x1CFB:

// MOVEB
case 0x10FB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x133B:
case 0x153B:
case 0x173B:
case 0x193B:
case 0x1B3B:
case 0x1D3B:

// MOVEB
case 0x113B:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x137B:
case 0x157B:
case 0x177B:
case 0x197B:
case 0x1B7B:
case 0x1D7B:
case 0x1F7B:

// MOVEB
case 0x117B:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(22)
case 0x13BB:
case 0x15BB:
case 0x17BB:
case 0x19BB:
case 0x1BBB:
case 0x1DBB:
case 0x1FBB:

// MOVEB
case 0x11BB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(24)

// MOVEB
case 0x11FB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(22)

// MOVEB
case 0x13FB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(26)

// MOVEB
case 0x1EFB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)

// MOVEB
case 0x1F3B:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_BYTE_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x123C:
case 0x143C:
case 0x163C:
case 0x183C:
case 0x1A3C:
case 0x1C3C:
case 0x1E3C:

// MOVEB
case 0x103C:
{
	u32 res;
	res = FETCH_BYTE;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0x12BC:
case 0x14BC:
case 0x16BC:
case 0x18BC:
case 0x1ABC:
case 0x1CBC:
case 0x1EBC:

// MOVEB
case 0x10BC:
{
	u32 adr;
	u32 res;
	res = FETCH_BYTE;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x12FC:
case 0x14FC:
case 0x16FC:
case 0x18FC:
case 0x1AFC:
case 0x1CFC:

// MOVEB
case 0x10FC:
{
	u32 adr;
	u32 res;
	res = FETCH_BYTE;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x133C:
case 0x153C:
case 0x173C:
case 0x193C:
case 0x1B3C:
case 0x1D3C:

// MOVEB
case 0x113C:
{
	u32 adr;
	u32 res;
	res = FETCH_BYTE;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x137C:
case 0x157C:
case 0x177C:
case 0x197C:
case 0x1B7C:
case 0x1D7C:
case 0x1F7C:

// MOVEB
case 0x117C:
{
	u32 adr;
	u32 res;
	res = FETCH_BYTE;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x13BC:
case 0x15BC:
case 0x17BC:
case 0x19BC:
case 0x1BBC:
case 0x1DBC:
case 0x1FBC:

// MOVEB
case 0x11BC:
{
	u32 adr;
	u32 res;
	res = FETCH_BYTE;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)

// MOVEB
case 0x11FC:
{
	u32 adr;
	u32 res;
	res = FETCH_BYTE;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)

// MOVEB
case 0x13FC:
{
	u32 adr;
	u32 res;
	res = FETCH_BYTE;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)

// MOVEB
case 0x1EFC:
{
	u32 adr;
	u32 res;
	res = FETCH_BYTE;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)

// MOVEB
case 0x1F3C:
{
	u32 adr;
	u32 res;
	res = FETCH_BYTE;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x121F:
case 0x141F:
case 0x161F:
case 0x181F:
case 0x1A1F:
case 0x1C1F:
case 0x1E1F:

// MOVEB
case 0x101F:
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
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x129F:
case 0x149F:
case 0x169F:
case 0x189F:
case 0x1A9F:
case 0x1C9F:
case 0x1E9F:

// MOVEB
case 0x109F:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x12DF:
case 0x14DF:
case 0x16DF:
case 0x18DF:
case 0x1ADF:
case 0x1CDF:

// MOVEB
case 0x10DF:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x131F:
case 0x151F:
case 0x171F:
case 0x191F:
case 0x1B1F:
case 0x1D1F:

// MOVEB
case 0x111F:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x135F:
case 0x155F:
case 0x175F:
case 0x195F:
case 0x1B5F:
case 0x1D5F:
case 0x1F5F:

// MOVEB
case 0x115F:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)
case 0x139F:
case 0x159F:
case 0x179F:
case 0x199F:
case 0x1B9F:
case 0x1D9F:
case 0x1F9F:

// MOVEB
case 0x119F:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)

// MOVEB
case 0x11DF:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(16)

// MOVEB
case 0x13DF:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)

// MOVEB
case 0x1EDF:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)

// MOVEB
case 0x1F1F:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(12)
case 0x1227:
case 0x1427:
case 0x1627:
case 0x1827:
case 0x1A27:
case 0x1C27:
case 0x1E27:

// MOVEB
case 0x1027:
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
	*(BYTE_OFF + (u8*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0x12A7:
case 0x14A7:
case 0x16A7:
case 0x18A7:
case 0x1AA7:
case 0x1CA7:
case 0x1EA7:

// MOVEB
case 0x10A7:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x12E7:
case 0x14E7:
case 0x16E7:
case 0x18E7:
case 0x1AE7:
case 0x1CE7:

// MOVEB
case 0x10E7:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 1;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x1327:
case 0x1527:
case 0x1727:
case 0x1927:
case 0x1B27:
case 0x1D27:

// MOVEB
case 0x1127:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 1;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
case 0x1367:
case 0x1567:
case 0x1767:
case 0x1967:
case 0x1B67:
case 0x1D67:
case 0x1F67:

// MOVEB
case 0x1167:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)
case 0x13A7:
case 0x15A7:
case 0x17A7:
case 0x19A7:
case 0x1BA7:
case 0x1DA7:
case 0x1FA7:

// MOVEB
case 0x11A7:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(20)

// MOVEB
case 0x11E7:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(18)

// MOVEB
case 0x13E7:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(22)

// MOVEB
case 0x1EE7:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)

// MOVEB
case 0x1F27:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_BYTE_F(adr, res)
	POST_IO
}
RET(14)
