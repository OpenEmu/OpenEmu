case 0x2200:
case 0x2400:
case 0x2600:
case 0x2800:
case 0x2A00:
case 0x2C00:
case 0x2E00:
case 0x2001:
case 0x2201:
case 0x2401:
case 0x2601:
case 0x2801:
case 0x2A01:
case 0x2C01:
case 0x2E01:
case 0x2002:
case 0x2202:
case 0x2402:
case 0x2602:
case 0x2802:
case 0x2A02:
case 0x2C02:
case 0x2E02:
case 0x2003:
case 0x2203:
case 0x2403:
case 0x2603:
case 0x2803:
case 0x2A03:
case 0x2C03:
case 0x2E03:
case 0x2004:
case 0x2204:
case 0x2404:
case 0x2604:
case 0x2804:
case 0x2A04:
case 0x2C04:
case 0x2E04:
case 0x2005:
case 0x2205:
case 0x2405:
case 0x2605:
case 0x2805:
case 0x2A05:
case 0x2C05:
case 0x2E05:
case 0x2006:
case 0x2206:
case 0x2406:
case 0x2606:
case 0x2806:
case 0x2A06:
case 0x2C06:
case 0x2E06:
case 0x2007:
case 0x2207:
case 0x2407:
case 0x2607:
case 0x2807:
case 0x2A07:
case 0x2C07:
case 0x2E07:

// MOVEL
case 0x2000:
{
	u32 res;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0x2280:
case 0x2480:
case 0x2680:
case 0x2880:
case 0x2A80:
case 0x2C80:
case 0x2E80:
case 0x2081:
case 0x2281:
case 0x2481:
case 0x2681:
case 0x2881:
case 0x2A81:
case 0x2C81:
case 0x2E81:
case 0x2082:
case 0x2282:
case 0x2482:
case 0x2682:
case 0x2882:
case 0x2A82:
case 0x2C82:
case 0x2E82:
case 0x2083:
case 0x2283:
case 0x2483:
case 0x2683:
case 0x2883:
case 0x2A83:
case 0x2C83:
case 0x2E83:
case 0x2084:
case 0x2284:
case 0x2484:
case 0x2684:
case 0x2884:
case 0x2A84:
case 0x2C84:
case 0x2E84:
case 0x2085:
case 0x2285:
case 0x2485:
case 0x2685:
case 0x2885:
case 0x2A85:
case 0x2C85:
case 0x2E85:
case 0x2086:
case 0x2286:
case 0x2486:
case 0x2686:
case 0x2886:
case 0x2A86:
case 0x2C86:
case 0x2E86:
case 0x2087:
case 0x2287:
case 0x2487:
case 0x2687:
case 0x2887:
case 0x2A87:
case 0x2C87:
case 0x2E87:

// MOVEL
case 0x2080:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(12)
case 0x22C0:
case 0x24C0:
case 0x26C0:
case 0x28C0:
case 0x2AC0:
case 0x2CC0:
case 0x20C1:
case 0x22C1:
case 0x24C1:
case 0x26C1:
case 0x28C1:
case 0x2AC1:
case 0x2CC1:
case 0x20C2:
case 0x22C2:
case 0x24C2:
case 0x26C2:
case 0x28C2:
case 0x2AC2:
case 0x2CC2:
case 0x20C3:
case 0x22C3:
case 0x24C3:
case 0x26C3:
case 0x28C3:
case 0x2AC3:
case 0x2CC3:
case 0x20C4:
case 0x22C4:
case 0x24C4:
case 0x26C4:
case 0x28C4:
case 0x2AC4:
case 0x2CC4:
case 0x20C5:
case 0x22C5:
case 0x24C5:
case 0x26C5:
case 0x28C5:
case 0x2AC5:
case 0x2CC5:
case 0x20C6:
case 0x22C6:
case 0x24C6:
case 0x26C6:
case 0x28C6:
case 0x2AC6:
case 0x2CC6:
case 0x20C7:
case 0x22C7:
case 0x24C7:
case 0x26C7:
case 0x28C7:
case 0x2AC7:
case 0x2CC7:

// MOVEL
case 0x20C0:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(12)
case 0x2300:
case 0x2500:
case 0x2700:
case 0x2900:
case 0x2B00:
case 0x2D00:
case 0x2101:
case 0x2301:
case 0x2501:
case 0x2701:
case 0x2901:
case 0x2B01:
case 0x2D01:
case 0x2102:
case 0x2302:
case 0x2502:
case 0x2702:
case 0x2902:
case 0x2B02:
case 0x2D02:
case 0x2103:
case 0x2303:
case 0x2503:
case 0x2703:
case 0x2903:
case 0x2B03:
case 0x2D03:
case 0x2104:
case 0x2304:
case 0x2504:
case 0x2704:
case 0x2904:
case 0x2B04:
case 0x2D04:
case 0x2105:
case 0x2305:
case 0x2505:
case 0x2705:
case 0x2905:
case 0x2B05:
case 0x2D05:
case 0x2106:
case 0x2306:
case 0x2506:
case 0x2706:
case 0x2906:
case 0x2B06:
case 0x2D06:
case 0x2107:
case 0x2307:
case 0x2507:
case 0x2707:
case 0x2907:
case 0x2B07:
case 0x2D07:

// MOVEL
case 0x2100:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	PRE_IO
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(12)
case 0x2340:
case 0x2540:
case 0x2740:
case 0x2940:
case 0x2B40:
case 0x2D40:
case 0x2F40:
case 0x2141:
case 0x2341:
case 0x2541:
case 0x2741:
case 0x2941:
case 0x2B41:
case 0x2D41:
case 0x2F41:
case 0x2142:
case 0x2342:
case 0x2542:
case 0x2742:
case 0x2942:
case 0x2B42:
case 0x2D42:
case 0x2F42:
case 0x2143:
case 0x2343:
case 0x2543:
case 0x2743:
case 0x2943:
case 0x2B43:
case 0x2D43:
case 0x2F43:
case 0x2144:
case 0x2344:
case 0x2544:
case 0x2744:
case 0x2944:
case 0x2B44:
case 0x2D44:
case 0x2F44:
case 0x2145:
case 0x2345:
case 0x2545:
case 0x2745:
case 0x2945:
case 0x2B45:
case 0x2D45:
case 0x2F45:
case 0x2146:
case 0x2346:
case 0x2546:
case 0x2746:
case 0x2946:
case 0x2B46:
case 0x2D46:
case 0x2F46:
case 0x2147:
case 0x2347:
case 0x2547:
case 0x2747:
case 0x2947:
case 0x2B47:
case 0x2D47:
case 0x2F47:

// MOVEL
case 0x2140:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(16)
case 0x2380:
case 0x2580:
case 0x2780:
case 0x2980:
case 0x2B80:
case 0x2D80:
case 0x2F80:
case 0x2181:
case 0x2381:
case 0x2581:
case 0x2781:
case 0x2981:
case 0x2B81:
case 0x2D81:
case 0x2F81:
case 0x2182:
case 0x2382:
case 0x2582:
case 0x2782:
case 0x2982:
case 0x2B82:
case 0x2D82:
case 0x2F82:
case 0x2183:
case 0x2383:
case 0x2583:
case 0x2783:
case 0x2983:
case 0x2B83:
case 0x2D83:
case 0x2F83:
case 0x2184:
case 0x2384:
case 0x2584:
case 0x2784:
case 0x2984:
case 0x2B84:
case 0x2D84:
case 0x2F84:
case 0x2185:
case 0x2385:
case 0x2585:
case 0x2785:
case 0x2985:
case 0x2B85:
case 0x2D85:
case 0x2F85:
case 0x2186:
case 0x2386:
case 0x2586:
case 0x2786:
case 0x2986:
case 0x2B86:
case 0x2D86:
case 0x2F86:
case 0x2187:
case 0x2387:
case 0x2587:
case 0x2787:
case 0x2987:
case 0x2B87:
case 0x2D87:
case 0x2F87:

// MOVEL
case 0x2180:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(18)
case 0x21C1:
case 0x21C2:
case 0x21C3:
case 0x21C4:
case 0x21C5:
case 0x21C6:
case 0x21C7:

// MOVEL
case 0x21C0:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(16)
case 0x23C1:
case 0x23C2:
case 0x23C3:
case 0x23C4:
case 0x23C5:
case 0x23C6:
case 0x23C7:

// MOVEL
case 0x23C0:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x2EC1:
case 0x2EC2:
case 0x2EC3:
case 0x2EC4:
case 0x2EC5:
case 0x2EC6:
case 0x2EC7:

// MOVEL
case 0x2EC0:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(12)
case 0x2F01:
case 0x2F02:
case 0x2F03:
case 0x2F04:
case 0x2F05:
case 0x2F06:
case 0x2F07:

// MOVEL
case 0x2F00:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(12)
case 0x2208:
case 0x2408:
case 0x2608:
case 0x2808:
case 0x2A08:
case 0x2C08:
case 0x2E08:
case 0x2009:
case 0x2209:
case 0x2409:
case 0x2609:
case 0x2809:
case 0x2A09:
case 0x2C09:
case 0x2E09:
case 0x200A:
case 0x220A:
case 0x240A:
case 0x260A:
case 0x280A:
case 0x2A0A:
case 0x2C0A:
case 0x2E0A:
case 0x200B:
case 0x220B:
case 0x240B:
case 0x260B:
case 0x280B:
case 0x2A0B:
case 0x2C0B:
case 0x2E0B:
case 0x200C:
case 0x220C:
case 0x240C:
case 0x260C:
case 0x280C:
case 0x2A0C:
case 0x2C0C:
case 0x2E0C:
case 0x200D:
case 0x220D:
case 0x240D:
case 0x260D:
case 0x280D:
case 0x2A0D:
case 0x2C0D:
case 0x2E0D:
case 0x200E:
case 0x220E:
case 0x240E:
case 0x260E:
case 0x280E:
case 0x2A0E:
case 0x2C0E:
case 0x2E0E:
case 0x200F:
case 0x220F:
case 0x240F:
case 0x260F:
case 0x280F:
case 0x2A0F:
case 0x2C0F:
case 0x2E0F:

// MOVEL
case 0x2008:
{
	u32 res;
	res = (u32)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0x2288:
case 0x2488:
case 0x2688:
case 0x2888:
case 0x2A88:
case 0x2C88:
case 0x2E88:
case 0x2089:
case 0x2289:
case 0x2489:
case 0x2689:
case 0x2889:
case 0x2A89:
case 0x2C89:
case 0x2E89:
case 0x208A:
case 0x228A:
case 0x248A:
case 0x268A:
case 0x288A:
case 0x2A8A:
case 0x2C8A:
case 0x2E8A:
case 0x208B:
case 0x228B:
case 0x248B:
case 0x268B:
case 0x288B:
case 0x2A8B:
case 0x2C8B:
case 0x2E8B:
case 0x208C:
case 0x228C:
case 0x248C:
case 0x268C:
case 0x288C:
case 0x2A8C:
case 0x2C8C:
case 0x2E8C:
case 0x208D:
case 0x228D:
case 0x248D:
case 0x268D:
case 0x288D:
case 0x2A8D:
case 0x2C8D:
case 0x2E8D:
case 0x208E:
case 0x228E:
case 0x248E:
case 0x268E:
case 0x288E:
case 0x2A8E:
case 0x2C8E:
case 0x2E8E:
case 0x208F:
case 0x228F:
case 0x248F:
case 0x268F:
case 0x288F:
case 0x2A8F:
case 0x2C8F:
case 0x2E8F:

// MOVEL
case 0x2088:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(12)
case 0x22C8:
case 0x24C8:
case 0x26C8:
case 0x28C8:
case 0x2AC8:
case 0x2CC8:
case 0x20C9:
case 0x22C9:
case 0x24C9:
case 0x26C9:
case 0x28C9:
case 0x2AC9:
case 0x2CC9:
case 0x20CA:
case 0x22CA:
case 0x24CA:
case 0x26CA:
case 0x28CA:
case 0x2ACA:
case 0x2CCA:
case 0x20CB:
case 0x22CB:
case 0x24CB:
case 0x26CB:
case 0x28CB:
case 0x2ACB:
case 0x2CCB:
case 0x20CC:
case 0x22CC:
case 0x24CC:
case 0x26CC:
case 0x28CC:
case 0x2ACC:
case 0x2CCC:
case 0x20CD:
case 0x22CD:
case 0x24CD:
case 0x26CD:
case 0x28CD:
case 0x2ACD:
case 0x2CCD:
case 0x20CE:
case 0x22CE:
case 0x24CE:
case 0x26CE:
case 0x28CE:
case 0x2ACE:
case 0x2CCE:
case 0x20CF:
case 0x22CF:
case 0x24CF:
case 0x26CF:
case 0x28CF:
case 0x2ACF:
case 0x2CCF:

// MOVEL
case 0x20C8:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(12)
case 0x2308:
case 0x2508:
case 0x2708:
case 0x2908:
case 0x2B08:
case 0x2D08:
case 0x2109:
case 0x2309:
case 0x2509:
case 0x2709:
case 0x2909:
case 0x2B09:
case 0x2D09:
case 0x210A:
case 0x230A:
case 0x250A:
case 0x270A:
case 0x290A:
case 0x2B0A:
case 0x2D0A:
case 0x210B:
case 0x230B:
case 0x250B:
case 0x270B:
case 0x290B:
case 0x2B0B:
case 0x2D0B:
case 0x210C:
case 0x230C:
case 0x250C:
case 0x270C:
case 0x290C:
case 0x2B0C:
case 0x2D0C:
case 0x210D:
case 0x230D:
case 0x250D:
case 0x270D:
case 0x290D:
case 0x2B0D:
case 0x2D0D:
case 0x210E:
case 0x230E:
case 0x250E:
case 0x270E:
case 0x290E:
case 0x2B0E:
case 0x2D0E:
case 0x210F:
case 0x230F:
case 0x250F:
case 0x270F:
case 0x290F:
case 0x2B0F:
case 0x2D0F:

// MOVEL
case 0x2108:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	PRE_IO
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(12)
case 0x2348:
case 0x2548:
case 0x2748:
case 0x2948:
case 0x2B48:
case 0x2D48:
case 0x2F48:
case 0x2149:
case 0x2349:
case 0x2549:
case 0x2749:
case 0x2949:
case 0x2B49:
case 0x2D49:
case 0x2F49:
case 0x214A:
case 0x234A:
case 0x254A:
case 0x274A:
case 0x294A:
case 0x2B4A:
case 0x2D4A:
case 0x2F4A:
case 0x214B:
case 0x234B:
case 0x254B:
case 0x274B:
case 0x294B:
case 0x2B4B:
case 0x2D4B:
case 0x2F4B:
case 0x214C:
case 0x234C:
case 0x254C:
case 0x274C:
case 0x294C:
case 0x2B4C:
case 0x2D4C:
case 0x2F4C:
case 0x214D:
case 0x234D:
case 0x254D:
case 0x274D:
case 0x294D:
case 0x2B4D:
case 0x2D4D:
case 0x2F4D:
case 0x214E:
case 0x234E:
case 0x254E:
case 0x274E:
case 0x294E:
case 0x2B4E:
case 0x2D4E:
case 0x2F4E:
case 0x214F:
case 0x234F:
case 0x254F:
case 0x274F:
case 0x294F:
case 0x2B4F:
case 0x2D4F:
case 0x2F4F:

// MOVEL
case 0x2148:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(16)
case 0x2388:
case 0x2588:
case 0x2788:
case 0x2988:
case 0x2B88:
case 0x2D88:
case 0x2F88:
case 0x2189:
case 0x2389:
case 0x2589:
case 0x2789:
case 0x2989:
case 0x2B89:
case 0x2D89:
case 0x2F89:
case 0x218A:
case 0x238A:
case 0x258A:
case 0x278A:
case 0x298A:
case 0x2B8A:
case 0x2D8A:
case 0x2F8A:
case 0x218B:
case 0x238B:
case 0x258B:
case 0x278B:
case 0x298B:
case 0x2B8B:
case 0x2D8B:
case 0x2F8B:
case 0x218C:
case 0x238C:
case 0x258C:
case 0x278C:
case 0x298C:
case 0x2B8C:
case 0x2D8C:
case 0x2F8C:
case 0x218D:
case 0x238D:
case 0x258D:
case 0x278D:
case 0x298D:
case 0x2B8D:
case 0x2D8D:
case 0x2F8D:
case 0x218E:
case 0x238E:
case 0x258E:
case 0x278E:
case 0x298E:
case 0x2B8E:
case 0x2D8E:
case 0x2F8E:
case 0x218F:
case 0x238F:
case 0x258F:
case 0x278F:
case 0x298F:
case 0x2B8F:
case 0x2D8F:
case 0x2F8F:

// MOVEL
case 0x2188:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(18)
case 0x21C9:
case 0x21CA:
case 0x21CB:
case 0x21CC:
case 0x21CD:
case 0x21CE:
case 0x21CF:

// MOVEL
case 0x21C8:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(16)
case 0x23C9:
case 0x23CA:
case 0x23CB:
case 0x23CC:
case 0x23CD:
case 0x23CE:
case 0x23CF:

// MOVEL
case 0x23C8:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x2EC9:
case 0x2ECA:
case 0x2ECB:
case 0x2ECC:
case 0x2ECD:
case 0x2ECE:
case 0x2ECF:

// MOVEL
case 0x2EC8:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(12)
case 0x2F09:
case 0x2F0A:
case 0x2F0B:
case 0x2F0C:
case 0x2F0D:
case 0x2F0E:
case 0x2F0F:

// MOVEL
case 0x2F08:
{
	u32 adr;
	u32 res;
	res = (u32)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(12)
case 0x2210:
case 0x2410:
case 0x2610:
case 0x2810:
case 0x2A10:
case 0x2C10:
case 0x2E10:
case 0x2011:
case 0x2211:
case 0x2411:
case 0x2611:
case 0x2811:
case 0x2A11:
case 0x2C11:
case 0x2E11:
case 0x2012:
case 0x2212:
case 0x2412:
case 0x2612:
case 0x2812:
case 0x2A12:
case 0x2C12:
case 0x2E12:
case 0x2013:
case 0x2213:
case 0x2413:
case 0x2613:
case 0x2813:
case 0x2A13:
case 0x2C13:
case 0x2E13:
case 0x2014:
case 0x2214:
case 0x2414:
case 0x2614:
case 0x2814:
case 0x2A14:
case 0x2C14:
case 0x2E14:
case 0x2015:
case 0x2215:
case 0x2415:
case 0x2615:
case 0x2815:
case 0x2A15:
case 0x2C15:
case 0x2E15:
case 0x2016:
case 0x2216:
case 0x2416:
case 0x2616:
case 0x2816:
case 0x2A16:
case 0x2C16:
case 0x2E16:
case 0x2017:
case 0x2217:
case 0x2417:
case 0x2617:
case 0x2817:
case 0x2A17:
case 0x2C17:
case 0x2E17:

// MOVEL
case 0x2010:
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
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x2290:
case 0x2490:
case 0x2690:
case 0x2890:
case 0x2A90:
case 0x2C90:
case 0x2E90:
case 0x2091:
case 0x2291:
case 0x2491:
case 0x2691:
case 0x2891:
case 0x2A91:
case 0x2C91:
case 0x2E91:
case 0x2092:
case 0x2292:
case 0x2492:
case 0x2692:
case 0x2892:
case 0x2A92:
case 0x2C92:
case 0x2E92:
case 0x2093:
case 0x2293:
case 0x2493:
case 0x2693:
case 0x2893:
case 0x2A93:
case 0x2C93:
case 0x2E93:
case 0x2094:
case 0x2294:
case 0x2494:
case 0x2694:
case 0x2894:
case 0x2A94:
case 0x2C94:
case 0x2E94:
case 0x2095:
case 0x2295:
case 0x2495:
case 0x2695:
case 0x2895:
case 0x2A95:
case 0x2C95:
case 0x2E95:
case 0x2096:
case 0x2296:
case 0x2496:
case 0x2696:
case 0x2896:
case 0x2A96:
case 0x2C96:
case 0x2E96:
case 0x2097:
case 0x2297:
case 0x2497:
case 0x2697:
case 0x2897:
case 0x2A97:
case 0x2C97:
case 0x2E97:

// MOVEL
case 0x2090:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x22D0:
case 0x24D0:
case 0x26D0:
case 0x28D0:
case 0x2AD0:
case 0x2CD0:
case 0x20D1:
case 0x22D1:
case 0x24D1:
case 0x26D1:
case 0x28D1:
case 0x2AD1:
case 0x2CD1:
case 0x20D2:
case 0x22D2:
case 0x24D2:
case 0x26D2:
case 0x28D2:
case 0x2AD2:
case 0x2CD2:
case 0x20D3:
case 0x22D3:
case 0x24D3:
case 0x26D3:
case 0x28D3:
case 0x2AD3:
case 0x2CD3:
case 0x20D4:
case 0x22D4:
case 0x24D4:
case 0x26D4:
case 0x28D4:
case 0x2AD4:
case 0x2CD4:
case 0x20D5:
case 0x22D5:
case 0x24D5:
case 0x26D5:
case 0x28D5:
case 0x2AD5:
case 0x2CD5:
case 0x20D6:
case 0x22D6:
case 0x24D6:
case 0x26D6:
case 0x28D6:
case 0x2AD6:
case 0x2CD6:
case 0x20D7:
case 0x22D7:
case 0x24D7:
case 0x26D7:
case 0x28D7:
case 0x2AD7:
case 0x2CD7:

// MOVEL
case 0x20D0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x2310:
case 0x2510:
case 0x2710:
case 0x2910:
case 0x2B10:
case 0x2D10:
case 0x2111:
case 0x2311:
case 0x2511:
case 0x2711:
case 0x2911:
case 0x2B11:
case 0x2D11:
case 0x2112:
case 0x2312:
case 0x2512:
case 0x2712:
case 0x2912:
case 0x2B12:
case 0x2D12:
case 0x2113:
case 0x2313:
case 0x2513:
case 0x2713:
case 0x2913:
case 0x2B13:
case 0x2D13:
case 0x2114:
case 0x2314:
case 0x2514:
case 0x2714:
case 0x2914:
case 0x2B14:
case 0x2D14:
case 0x2115:
case 0x2315:
case 0x2515:
case 0x2715:
case 0x2915:
case 0x2B15:
case 0x2D15:
case 0x2116:
case 0x2316:
case 0x2516:
case 0x2716:
case 0x2916:
case 0x2B16:
case 0x2D16:
case 0x2117:
case 0x2317:
case 0x2517:
case 0x2717:
case 0x2917:
case 0x2B17:
case 0x2D17:

// MOVEL
case 0x2110:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(20)
case 0x2350:
case 0x2550:
case 0x2750:
case 0x2950:
case 0x2B50:
case 0x2D50:
case 0x2F50:
case 0x2151:
case 0x2351:
case 0x2551:
case 0x2751:
case 0x2951:
case 0x2B51:
case 0x2D51:
case 0x2F51:
case 0x2152:
case 0x2352:
case 0x2552:
case 0x2752:
case 0x2952:
case 0x2B52:
case 0x2D52:
case 0x2F52:
case 0x2153:
case 0x2353:
case 0x2553:
case 0x2753:
case 0x2953:
case 0x2B53:
case 0x2D53:
case 0x2F53:
case 0x2154:
case 0x2354:
case 0x2554:
case 0x2754:
case 0x2954:
case 0x2B54:
case 0x2D54:
case 0x2F54:
case 0x2155:
case 0x2355:
case 0x2555:
case 0x2755:
case 0x2955:
case 0x2B55:
case 0x2D55:
case 0x2F55:
case 0x2156:
case 0x2356:
case 0x2556:
case 0x2756:
case 0x2956:
case 0x2B56:
case 0x2D56:
case 0x2F56:
case 0x2157:
case 0x2357:
case 0x2557:
case 0x2757:
case 0x2957:
case 0x2B57:
case 0x2D57:
case 0x2F57:

// MOVEL
case 0x2150:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x2390:
case 0x2590:
case 0x2790:
case 0x2990:
case 0x2B90:
case 0x2D90:
case 0x2F90:
case 0x2191:
case 0x2391:
case 0x2591:
case 0x2791:
case 0x2991:
case 0x2B91:
case 0x2D91:
case 0x2F91:
case 0x2192:
case 0x2392:
case 0x2592:
case 0x2792:
case 0x2992:
case 0x2B92:
case 0x2D92:
case 0x2F92:
case 0x2193:
case 0x2393:
case 0x2593:
case 0x2793:
case 0x2993:
case 0x2B93:
case 0x2D93:
case 0x2F93:
case 0x2194:
case 0x2394:
case 0x2594:
case 0x2794:
case 0x2994:
case 0x2B94:
case 0x2D94:
case 0x2F94:
case 0x2195:
case 0x2395:
case 0x2595:
case 0x2795:
case 0x2995:
case 0x2B95:
case 0x2D95:
case 0x2F95:
case 0x2196:
case 0x2396:
case 0x2596:
case 0x2796:
case 0x2996:
case 0x2B96:
case 0x2D96:
case 0x2F96:
case 0x2197:
case 0x2397:
case 0x2597:
case 0x2797:
case 0x2997:
case 0x2B97:
case 0x2D97:
case 0x2F97:

// MOVEL
case 0x2190:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0x21D1:
case 0x21D2:
case 0x21D3:
case 0x21D4:
case 0x21D5:
case 0x21D6:
case 0x21D7:

// MOVEL
case 0x21D0:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x23D1:
case 0x23D2:
case 0x23D3:
case 0x23D4:
case 0x23D5:
case 0x23D6:
case 0x23D7:

// MOVEL
case 0x23D0:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0x2ED1:
case 0x2ED2:
case 0x2ED3:
case 0x2ED4:
case 0x2ED5:
case 0x2ED6:
case 0x2ED7:

// MOVEL
case 0x2ED0:
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
	adr = CPU->A[7];
	CPU->A[7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x2F11:
case 0x2F12:
case 0x2F13:
case 0x2F14:
case 0x2F15:
case 0x2F16:
case 0x2F17:

// MOVEL
case 0x2F10:
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
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(20)
case 0x2218:
case 0x2418:
case 0x2618:
case 0x2818:
case 0x2A18:
case 0x2C18:
case 0x2E18:
case 0x2019:
case 0x2219:
case 0x2419:
case 0x2619:
case 0x2819:
case 0x2A19:
case 0x2C19:
case 0x2E19:
case 0x201A:
case 0x221A:
case 0x241A:
case 0x261A:
case 0x281A:
case 0x2A1A:
case 0x2C1A:
case 0x2E1A:
case 0x201B:
case 0x221B:
case 0x241B:
case 0x261B:
case 0x281B:
case 0x2A1B:
case 0x2C1B:
case 0x2E1B:
case 0x201C:
case 0x221C:
case 0x241C:
case 0x261C:
case 0x281C:
case 0x2A1C:
case 0x2C1C:
case 0x2E1C:
case 0x201D:
case 0x221D:
case 0x241D:
case 0x261D:
case 0x281D:
case 0x2A1D:
case 0x2C1D:
case 0x2E1D:
case 0x201E:
case 0x221E:
case 0x241E:
case 0x261E:
case 0x281E:
case 0x2A1E:
case 0x2C1E:
case 0x2E1E:

// MOVEL
case 0x2018:
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
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x2298:
case 0x2498:
case 0x2698:
case 0x2898:
case 0x2A98:
case 0x2C98:
case 0x2E98:
case 0x2099:
case 0x2299:
case 0x2499:
case 0x2699:
case 0x2899:
case 0x2A99:
case 0x2C99:
case 0x2E99:
case 0x209A:
case 0x229A:
case 0x249A:
case 0x269A:
case 0x289A:
case 0x2A9A:
case 0x2C9A:
case 0x2E9A:
case 0x209B:
case 0x229B:
case 0x249B:
case 0x269B:
case 0x289B:
case 0x2A9B:
case 0x2C9B:
case 0x2E9B:
case 0x209C:
case 0x229C:
case 0x249C:
case 0x269C:
case 0x289C:
case 0x2A9C:
case 0x2C9C:
case 0x2E9C:
case 0x209D:
case 0x229D:
case 0x249D:
case 0x269D:
case 0x289D:
case 0x2A9D:
case 0x2C9D:
case 0x2E9D:
case 0x209E:
case 0x229E:
case 0x249E:
case 0x269E:
case 0x289E:
case 0x2A9E:
case 0x2C9E:
case 0x2E9E:

// MOVEL
case 0x2098:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x22D8:
case 0x24D8:
case 0x26D8:
case 0x28D8:
case 0x2AD8:
case 0x2CD8:
case 0x20D9:
case 0x22D9:
case 0x24D9:
case 0x26D9:
case 0x28D9:
case 0x2AD9:
case 0x2CD9:
case 0x20DA:
case 0x22DA:
case 0x24DA:
case 0x26DA:
case 0x28DA:
case 0x2ADA:
case 0x2CDA:
case 0x20DB:
case 0x22DB:
case 0x24DB:
case 0x26DB:
case 0x28DB:
case 0x2ADB:
case 0x2CDB:
case 0x20DC:
case 0x22DC:
case 0x24DC:
case 0x26DC:
case 0x28DC:
case 0x2ADC:
case 0x2CDC:
case 0x20DD:
case 0x22DD:
case 0x24DD:
case 0x26DD:
case 0x28DD:
case 0x2ADD:
case 0x2CDD:
case 0x20DE:
case 0x22DE:
case 0x24DE:
case 0x26DE:
case 0x28DE:
case 0x2ADE:
case 0x2CDE:

// MOVEL
case 0x20D8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x2318:
case 0x2518:
case 0x2718:
case 0x2918:
case 0x2B18:
case 0x2D18:
case 0x2119:
case 0x2319:
case 0x2519:
case 0x2719:
case 0x2919:
case 0x2B19:
case 0x2D19:
case 0x211A:
case 0x231A:
case 0x251A:
case 0x271A:
case 0x291A:
case 0x2B1A:
case 0x2D1A:
case 0x211B:
case 0x231B:
case 0x251B:
case 0x271B:
case 0x291B:
case 0x2B1B:
case 0x2D1B:
case 0x211C:
case 0x231C:
case 0x251C:
case 0x271C:
case 0x291C:
case 0x2B1C:
case 0x2D1C:
case 0x211D:
case 0x231D:
case 0x251D:
case 0x271D:
case 0x291D:
case 0x2B1D:
case 0x2D1D:
case 0x211E:
case 0x231E:
case 0x251E:
case 0x271E:
case 0x291E:
case 0x2B1E:
case 0x2D1E:

// MOVEL
case 0x2118:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(20)
case 0x2358:
case 0x2558:
case 0x2758:
case 0x2958:
case 0x2B58:
case 0x2D58:
case 0x2F58:
case 0x2159:
case 0x2359:
case 0x2559:
case 0x2759:
case 0x2959:
case 0x2B59:
case 0x2D59:
case 0x2F59:
case 0x215A:
case 0x235A:
case 0x255A:
case 0x275A:
case 0x295A:
case 0x2B5A:
case 0x2D5A:
case 0x2F5A:
case 0x215B:
case 0x235B:
case 0x255B:
case 0x275B:
case 0x295B:
case 0x2B5B:
case 0x2D5B:
case 0x2F5B:
case 0x215C:
case 0x235C:
case 0x255C:
case 0x275C:
case 0x295C:
case 0x2B5C:
case 0x2D5C:
case 0x2F5C:
case 0x215D:
case 0x235D:
case 0x255D:
case 0x275D:
case 0x295D:
case 0x2B5D:
case 0x2D5D:
case 0x2F5D:
case 0x215E:
case 0x235E:
case 0x255E:
case 0x275E:
case 0x295E:
case 0x2B5E:
case 0x2D5E:
case 0x2F5E:

// MOVEL
case 0x2158:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x2398:
case 0x2598:
case 0x2798:
case 0x2998:
case 0x2B98:
case 0x2D98:
case 0x2F98:
case 0x2199:
case 0x2399:
case 0x2599:
case 0x2799:
case 0x2999:
case 0x2B99:
case 0x2D99:
case 0x2F99:
case 0x219A:
case 0x239A:
case 0x259A:
case 0x279A:
case 0x299A:
case 0x2B9A:
case 0x2D9A:
case 0x2F9A:
case 0x219B:
case 0x239B:
case 0x259B:
case 0x279B:
case 0x299B:
case 0x2B9B:
case 0x2D9B:
case 0x2F9B:
case 0x219C:
case 0x239C:
case 0x259C:
case 0x279C:
case 0x299C:
case 0x2B9C:
case 0x2D9C:
case 0x2F9C:
case 0x219D:
case 0x239D:
case 0x259D:
case 0x279D:
case 0x299D:
case 0x2B9D:
case 0x2D9D:
case 0x2F9D:
case 0x219E:
case 0x239E:
case 0x259E:
case 0x279E:
case 0x299E:
case 0x2B9E:
case 0x2D9E:
case 0x2F9E:

// MOVEL
case 0x2198:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0x21D9:
case 0x21DA:
case 0x21DB:
case 0x21DC:
case 0x21DD:
case 0x21DE:

// MOVEL
case 0x21D8:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x23D9:
case 0x23DA:
case 0x23DB:
case 0x23DC:
case 0x23DD:
case 0x23DE:

// MOVEL
case 0x23D8:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0x2ED9:
case 0x2EDA:
case 0x2EDB:
case 0x2EDC:
case 0x2EDD:
case 0x2EDE:

// MOVEL
case 0x2ED8:
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
	adr = CPU->A[7];
	CPU->A[7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x2F19:
case 0x2F1A:
case 0x2F1B:
case 0x2F1C:
case 0x2F1D:
case 0x2F1E:

// MOVEL
case 0x2F18:
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
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(20)
case 0x2220:
case 0x2420:
case 0x2620:
case 0x2820:
case 0x2A20:
case 0x2C20:
case 0x2E20:
case 0x2021:
case 0x2221:
case 0x2421:
case 0x2621:
case 0x2821:
case 0x2A21:
case 0x2C21:
case 0x2E21:
case 0x2022:
case 0x2222:
case 0x2422:
case 0x2622:
case 0x2822:
case 0x2A22:
case 0x2C22:
case 0x2E22:
case 0x2023:
case 0x2223:
case 0x2423:
case 0x2623:
case 0x2823:
case 0x2A23:
case 0x2C23:
case 0x2E23:
case 0x2024:
case 0x2224:
case 0x2424:
case 0x2624:
case 0x2824:
case 0x2A24:
case 0x2C24:
case 0x2E24:
case 0x2025:
case 0x2225:
case 0x2425:
case 0x2625:
case 0x2825:
case 0x2A25:
case 0x2C25:
case 0x2E25:
case 0x2026:
case 0x2226:
case 0x2426:
case 0x2626:
case 0x2826:
case 0x2A26:
case 0x2C26:
case 0x2E26:

// MOVEL
case 0x2020:
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
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x22A0:
case 0x24A0:
case 0x26A0:
case 0x28A0:
case 0x2AA0:
case 0x2CA0:
case 0x2EA0:
case 0x20A1:
case 0x22A1:
case 0x24A1:
case 0x26A1:
case 0x28A1:
case 0x2AA1:
case 0x2CA1:
case 0x2EA1:
case 0x20A2:
case 0x22A2:
case 0x24A2:
case 0x26A2:
case 0x28A2:
case 0x2AA2:
case 0x2CA2:
case 0x2EA2:
case 0x20A3:
case 0x22A3:
case 0x24A3:
case 0x26A3:
case 0x28A3:
case 0x2AA3:
case 0x2CA3:
case 0x2EA3:
case 0x20A4:
case 0x22A4:
case 0x24A4:
case 0x26A4:
case 0x28A4:
case 0x2AA4:
case 0x2CA4:
case 0x2EA4:
case 0x20A5:
case 0x22A5:
case 0x24A5:
case 0x26A5:
case 0x28A5:
case 0x2AA5:
case 0x2CA5:
case 0x2EA5:
case 0x20A6:
case 0x22A6:
case 0x24A6:
case 0x26A6:
case 0x28A6:
case 0x2AA6:
case 0x2CA6:
case 0x2EA6:

// MOVEL
case 0x20A0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x22E0:
case 0x24E0:
case 0x26E0:
case 0x28E0:
case 0x2AE0:
case 0x2CE0:
case 0x20E1:
case 0x22E1:
case 0x24E1:
case 0x26E1:
case 0x28E1:
case 0x2AE1:
case 0x2CE1:
case 0x20E2:
case 0x22E2:
case 0x24E2:
case 0x26E2:
case 0x28E2:
case 0x2AE2:
case 0x2CE2:
case 0x20E3:
case 0x22E3:
case 0x24E3:
case 0x26E3:
case 0x28E3:
case 0x2AE3:
case 0x2CE3:
case 0x20E4:
case 0x22E4:
case 0x24E4:
case 0x26E4:
case 0x28E4:
case 0x2AE4:
case 0x2CE4:
case 0x20E5:
case 0x22E5:
case 0x24E5:
case 0x26E5:
case 0x28E5:
case 0x2AE5:
case 0x2CE5:
case 0x20E6:
case 0x22E6:
case 0x24E6:
case 0x26E6:
case 0x28E6:
case 0x2AE6:
case 0x2CE6:

// MOVEL
case 0x20E0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x2320:
case 0x2520:
case 0x2720:
case 0x2920:
case 0x2B20:
case 0x2D20:
case 0x2121:
case 0x2321:
case 0x2521:
case 0x2721:
case 0x2921:
case 0x2B21:
case 0x2D21:
case 0x2122:
case 0x2322:
case 0x2522:
case 0x2722:
case 0x2922:
case 0x2B22:
case 0x2D22:
case 0x2123:
case 0x2323:
case 0x2523:
case 0x2723:
case 0x2923:
case 0x2B23:
case 0x2D23:
case 0x2124:
case 0x2324:
case 0x2524:
case 0x2724:
case 0x2924:
case 0x2B24:
case 0x2D24:
case 0x2125:
case 0x2325:
case 0x2525:
case 0x2725:
case 0x2925:
case 0x2B25:
case 0x2D25:
case 0x2126:
case 0x2326:
case 0x2526:
case 0x2726:
case 0x2926:
case 0x2B26:
case 0x2D26:

// MOVEL
case 0x2120:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(22)
case 0x2360:
case 0x2560:
case 0x2760:
case 0x2960:
case 0x2B60:
case 0x2D60:
case 0x2F60:
case 0x2161:
case 0x2361:
case 0x2561:
case 0x2761:
case 0x2961:
case 0x2B61:
case 0x2D61:
case 0x2F61:
case 0x2162:
case 0x2362:
case 0x2562:
case 0x2762:
case 0x2962:
case 0x2B62:
case 0x2D62:
case 0x2F62:
case 0x2163:
case 0x2363:
case 0x2563:
case 0x2763:
case 0x2963:
case 0x2B63:
case 0x2D63:
case 0x2F63:
case 0x2164:
case 0x2364:
case 0x2564:
case 0x2764:
case 0x2964:
case 0x2B64:
case 0x2D64:
case 0x2F64:
case 0x2165:
case 0x2365:
case 0x2565:
case 0x2765:
case 0x2965:
case 0x2B65:
case 0x2D65:
case 0x2F65:
case 0x2166:
case 0x2366:
case 0x2566:
case 0x2766:
case 0x2966:
case 0x2B66:
case 0x2D66:
case 0x2F66:

// MOVEL
case 0x2160:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0x23A0:
case 0x25A0:
case 0x27A0:
case 0x29A0:
case 0x2BA0:
case 0x2DA0:
case 0x2FA0:
case 0x21A1:
case 0x23A1:
case 0x25A1:
case 0x27A1:
case 0x29A1:
case 0x2BA1:
case 0x2DA1:
case 0x2FA1:
case 0x21A2:
case 0x23A2:
case 0x25A2:
case 0x27A2:
case 0x29A2:
case 0x2BA2:
case 0x2DA2:
case 0x2FA2:
case 0x21A3:
case 0x23A3:
case 0x25A3:
case 0x27A3:
case 0x29A3:
case 0x2BA3:
case 0x2DA3:
case 0x2FA3:
case 0x21A4:
case 0x23A4:
case 0x25A4:
case 0x27A4:
case 0x29A4:
case 0x2BA4:
case 0x2DA4:
case 0x2FA4:
case 0x21A5:
case 0x23A5:
case 0x25A5:
case 0x27A5:
case 0x29A5:
case 0x2BA5:
case 0x2DA5:
case 0x2FA5:
case 0x21A6:
case 0x23A6:
case 0x25A6:
case 0x27A6:
case 0x29A6:
case 0x2BA6:
case 0x2DA6:
case 0x2FA6:

// MOVEL
case 0x21A0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0x21E1:
case 0x21E2:
case 0x21E3:
case 0x21E4:
case 0x21E5:
case 0x21E6:

// MOVEL
case 0x21E0:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0x23E1:
case 0x23E2:
case 0x23E3:
case 0x23E4:
case 0x23E5:
case 0x23E6:

// MOVEL
case 0x23E0:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)
case 0x2EE1:
case 0x2EE2:
case 0x2EE3:
case 0x2EE4:
case 0x2EE5:
case 0x2EE6:

// MOVEL
case 0x2EE0:
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
	adr = CPU->A[7];
	CPU->A[7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x2F21:
case 0x2F22:
case 0x2F23:
case 0x2F24:
case 0x2F25:
case 0x2F26:

// MOVEL
case 0x2F20:
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
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(22)
case 0x2228:
case 0x2428:
case 0x2628:
case 0x2828:
case 0x2A28:
case 0x2C28:
case 0x2E28:
case 0x2029:
case 0x2229:
case 0x2429:
case 0x2629:
case 0x2829:
case 0x2A29:
case 0x2C29:
case 0x2E29:
case 0x202A:
case 0x222A:
case 0x242A:
case 0x262A:
case 0x282A:
case 0x2A2A:
case 0x2C2A:
case 0x2E2A:
case 0x202B:
case 0x222B:
case 0x242B:
case 0x262B:
case 0x282B:
case 0x2A2B:
case 0x2C2B:
case 0x2E2B:
case 0x202C:
case 0x222C:
case 0x242C:
case 0x262C:
case 0x282C:
case 0x2A2C:
case 0x2C2C:
case 0x2E2C:
case 0x202D:
case 0x222D:
case 0x242D:
case 0x262D:
case 0x282D:
case 0x2A2D:
case 0x2C2D:
case 0x2E2D:
case 0x202E:
case 0x222E:
case 0x242E:
case 0x262E:
case 0x282E:
case 0x2A2E:
case 0x2C2E:
case 0x2E2E:
case 0x202F:
case 0x222F:
case 0x242F:
case 0x262F:
case 0x282F:
case 0x2A2F:
case 0x2C2F:
case 0x2E2F:

// MOVEL
case 0x2028:
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
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0x22A8:
case 0x24A8:
case 0x26A8:
case 0x28A8:
case 0x2AA8:
case 0x2CA8:
case 0x2EA8:
case 0x20A9:
case 0x22A9:
case 0x24A9:
case 0x26A9:
case 0x28A9:
case 0x2AA9:
case 0x2CA9:
case 0x2EA9:
case 0x20AA:
case 0x22AA:
case 0x24AA:
case 0x26AA:
case 0x28AA:
case 0x2AAA:
case 0x2CAA:
case 0x2EAA:
case 0x20AB:
case 0x22AB:
case 0x24AB:
case 0x26AB:
case 0x28AB:
case 0x2AAB:
case 0x2CAB:
case 0x2EAB:
case 0x20AC:
case 0x22AC:
case 0x24AC:
case 0x26AC:
case 0x28AC:
case 0x2AAC:
case 0x2CAC:
case 0x2EAC:
case 0x20AD:
case 0x22AD:
case 0x24AD:
case 0x26AD:
case 0x28AD:
case 0x2AAD:
case 0x2CAD:
case 0x2EAD:
case 0x20AE:
case 0x22AE:
case 0x24AE:
case 0x26AE:
case 0x28AE:
case 0x2AAE:
case 0x2CAE:
case 0x2EAE:
case 0x20AF:
case 0x22AF:
case 0x24AF:
case 0x26AF:
case 0x28AF:
case 0x2AAF:
case 0x2CAF:
case 0x2EAF:

// MOVEL
case 0x20A8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x22E8:
case 0x24E8:
case 0x26E8:
case 0x28E8:
case 0x2AE8:
case 0x2CE8:
case 0x20E9:
case 0x22E9:
case 0x24E9:
case 0x26E9:
case 0x28E9:
case 0x2AE9:
case 0x2CE9:
case 0x20EA:
case 0x22EA:
case 0x24EA:
case 0x26EA:
case 0x28EA:
case 0x2AEA:
case 0x2CEA:
case 0x20EB:
case 0x22EB:
case 0x24EB:
case 0x26EB:
case 0x28EB:
case 0x2AEB:
case 0x2CEB:
case 0x20EC:
case 0x22EC:
case 0x24EC:
case 0x26EC:
case 0x28EC:
case 0x2AEC:
case 0x2CEC:
case 0x20ED:
case 0x22ED:
case 0x24ED:
case 0x26ED:
case 0x28ED:
case 0x2AED:
case 0x2CED:
case 0x20EE:
case 0x22EE:
case 0x24EE:
case 0x26EE:
case 0x28EE:
case 0x2AEE:
case 0x2CEE:
case 0x20EF:
case 0x22EF:
case 0x24EF:
case 0x26EF:
case 0x28EF:
case 0x2AEF:
case 0x2CEF:

// MOVEL
case 0x20E8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x2328:
case 0x2528:
case 0x2728:
case 0x2928:
case 0x2B28:
case 0x2D28:
case 0x2129:
case 0x2329:
case 0x2529:
case 0x2729:
case 0x2929:
case 0x2B29:
case 0x2D29:
case 0x212A:
case 0x232A:
case 0x252A:
case 0x272A:
case 0x292A:
case 0x2B2A:
case 0x2D2A:
case 0x212B:
case 0x232B:
case 0x252B:
case 0x272B:
case 0x292B:
case 0x2B2B:
case 0x2D2B:
case 0x212C:
case 0x232C:
case 0x252C:
case 0x272C:
case 0x292C:
case 0x2B2C:
case 0x2D2C:
case 0x212D:
case 0x232D:
case 0x252D:
case 0x272D:
case 0x292D:
case 0x2B2D:
case 0x2D2D:
case 0x212E:
case 0x232E:
case 0x252E:
case 0x272E:
case 0x292E:
case 0x2B2E:
case 0x2D2E:
case 0x212F:
case 0x232F:
case 0x252F:
case 0x272F:
case 0x292F:
case 0x2B2F:
case 0x2D2F:

// MOVEL
case 0x2128:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(24)
case 0x2368:
case 0x2568:
case 0x2768:
case 0x2968:
case 0x2B68:
case 0x2D68:
case 0x2F68:
case 0x2169:
case 0x2369:
case 0x2569:
case 0x2769:
case 0x2969:
case 0x2B69:
case 0x2D69:
case 0x2F69:
case 0x216A:
case 0x236A:
case 0x256A:
case 0x276A:
case 0x296A:
case 0x2B6A:
case 0x2D6A:
case 0x2F6A:
case 0x216B:
case 0x236B:
case 0x256B:
case 0x276B:
case 0x296B:
case 0x2B6B:
case 0x2D6B:
case 0x2F6B:
case 0x216C:
case 0x236C:
case 0x256C:
case 0x276C:
case 0x296C:
case 0x2B6C:
case 0x2D6C:
case 0x2F6C:
case 0x216D:
case 0x236D:
case 0x256D:
case 0x276D:
case 0x296D:
case 0x2B6D:
case 0x2D6D:
case 0x2F6D:
case 0x216E:
case 0x236E:
case 0x256E:
case 0x276E:
case 0x296E:
case 0x2B6E:
case 0x2D6E:
case 0x2F6E:
case 0x216F:
case 0x236F:
case 0x256F:
case 0x276F:
case 0x296F:
case 0x2B6F:
case 0x2D6F:
case 0x2F6F:

// MOVEL
case 0x2168:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0x23A8:
case 0x25A8:
case 0x27A8:
case 0x29A8:
case 0x2BA8:
case 0x2DA8:
case 0x2FA8:
case 0x21A9:
case 0x23A9:
case 0x25A9:
case 0x27A9:
case 0x29A9:
case 0x2BA9:
case 0x2DA9:
case 0x2FA9:
case 0x21AA:
case 0x23AA:
case 0x25AA:
case 0x27AA:
case 0x29AA:
case 0x2BAA:
case 0x2DAA:
case 0x2FAA:
case 0x21AB:
case 0x23AB:
case 0x25AB:
case 0x27AB:
case 0x29AB:
case 0x2BAB:
case 0x2DAB:
case 0x2FAB:
case 0x21AC:
case 0x23AC:
case 0x25AC:
case 0x27AC:
case 0x29AC:
case 0x2BAC:
case 0x2DAC:
case 0x2FAC:
case 0x21AD:
case 0x23AD:
case 0x25AD:
case 0x27AD:
case 0x29AD:
case 0x2BAD:
case 0x2DAD:
case 0x2FAD:
case 0x21AE:
case 0x23AE:
case 0x25AE:
case 0x27AE:
case 0x29AE:
case 0x2BAE:
case 0x2DAE:
case 0x2FAE:
case 0x21AF:
case 0x23AF:
case 0x25AF:
case 0x27AF:
case 0x29AF:
case 0x2BAF:
case 0x2DAF:
case 0x2FAF:

// MOVEL
case 0x21A8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)
case 0x21E9:
case 0x21EA:
case 0x21EB:
case 0x21EC:
case 0x21ED:
case 0x21EE:
case 0x21EF:

// MOVEL
case 0x21E8:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0x23E9:
case 0x23EA:
case 0x23EB:
case 0x23EC:
case 0x23ED:
case 0x23EE:
case 0x23EF:

// MOVEL
case 0x23E8:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(32)
case 0x2EE9:
case 0x2EEA:
case 0x2EEB:
case 0x2EEC:
case 0x2EED:
case 0x2EEE:
case 0x2EEF:

// MOVEL
case 0x2EE8:
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
	adr = CPU->A[7];
	CPU->A[7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x2F29:
case 0x2F2A:
case 0x2F2B:
case 0x2F2C:
case 0x2F2D:
case 0x2F2E:
case 0x2F2F:

// MOVEL
case 0x2F28:
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
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(24)
case 0x2230:
case 0x2430:
case 0x2630:
case 0x2830:
case 0x2A30:
case 0x2C30:
case 0x2E30:
case 0x2031:
case 0x2231:
case 0x2431:
case 0x2631:
case 0x2831:
case 0x2A31:
case 0x2C31:
case 0x2E31:
case 0x2032:
case 0x2232:
case 0x2432:
case 0x2632:
case 0x2832:
case 0x2A32:
case 0x2C32:
case 0x2E32:
case 0x2033:
case 0x2233:
case 0x2433:
case 0x2633:
case 0x2833:
case 0x2A33:
case 0x2C33:
case 0x2E33:
case 0x2034:
case 0x2234:
case 0x2434:
case 0x2634:
case 0x2834:
case 0x2A34:
case 0x2C34:
case 0x2E34:
case 0x2035:
case 0x2235:
case 0x2435:
case 0x2635:
case 0x2835:
case 0x2A35:
case 0x2C35:
case 0x2E35:
case 0x2036:
case 0x2236:
case 0x2436:
case 0x2636:
case 0x2836:
case 0x2A36:
case 0x2C36:
case 0x2E36:
case 0x2037:
case 0x2237:
case 0x2437:
case 0x2637:
case 0x2837:
case 0x2A37:
case 0x2C37:
case 0x2E37:

// MOVEL
case 0x2030:
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
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(18)
case 0x22B0:
case 0x24B0:
case 0x26B0:
case 0x28B0:
case 0x2AB0:
case 0x2CB0:
case 0x2EB0:
case 0x20B1:
case 0x22B1:
case 0x24B1:
case 0x26B1:
case 0x28B1:
case 0x2AB1:
case 0x2CB1:
case 0x2EB1:
case 0x20B2:
case 0x22B2:
case 0x24B2:
case 0x26B2:
case 0x28B2:
case 0x2AB2:
case 0x2CB2:
case 0x2EB2:
case 0x20B3:
case 0x22B3:
case 0x24B3:
case 0x26B3:
case 0x28B3:
case 0x2AB3:
case 0x2CB3:
case 0x2EB3:
case 0x20B4:
case 0x22B4:
case 0x24B4:
case 0x26B4:
case 0x28B4:
case 0x2AB4:
case 0x2CB4:
case 0x2EB4:
case 0x20B5:
case 0x22B5:
case 0x24B5:
case 0x26B5:
case 0x28B5:
case 0x2AB5:
case 0x2CB5:
case 0x2EB5:
case 0x20B6:
case 0x22B6:
case 0x24B6:
case 0x26B6:
case 0x28B6:
case 0x2AB6:
case 0x2CB6:
case 0x2EB6:
case 0x20B7:
case 0x22B7:
case 0x24B7:
case 0x26B7:
case 0x28B7:
case 0x2AB7:
case 0x2CB7:
case 0x2EB7:

// MOVEL
case 0x20B0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0x22F0:
case 0x24F0:
case 0x26F0:
case 0x28F0:
case 0x2AF0:
case 0x2CF0:
case 0x20F1:
case 0x22F1:
case 0x24F1:
case 0x26F1:
case 0x28F1:
case 0x2AF1:
case 0x2CF1:
case 0x20F2:
case 0x22F2:
case 0x24F2:
case 0x26F2:
case 0x28F2:
case 0x2AF2:
case 0x2CF2:
case 0x20F3:
case 0x22F3:
case 0x24F3:
case 0x26F3:
case 0x28F3:
case 0x2AF3:
case 0x2CF3:
case 0x20F4:
case 0x22F4:
case 0x24F4:
case 0x26F4:
case 0x28F4:
case 0x2AF4:
case 0x2CF4:
case 0x20F5:
case 0x22F5:
case 0x24F5:
case 0x26F5:
case 0x28F5:
case 0x2AF5:
case 0x2CF5:
case 0x20F6:
case 0x22F6:
case 0x24F6:
case 0x26F6:
case 0x28F6:
case 0x2AF6:
case 0x2CF6:
case 0x20F7:
case 0x22F7:
case 0x24F7:
case 0x26F7:
case 0x28F7:
case 0x2AF7:
case 0x2CF7:

// MOVEL
case 0x20F0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0x2330:
case 0x2530:
case 0x2730:
case 0x2930:
case 0x2B30:
case 0x2D30:
case 0x2131:
case 0x2331:
case 0x2531:
case 0x2731:
case 0x2931:
case 0x2B31:
case 0x2D31:
case 0x2132:
case 0x2332:
case 0x2532:
case 0x2732:
case 0x2932:
case 0x2B32:
case 0x2D32:
case 0x2133:
case 0x2333:
case 0x2533:
case 0x2733:
case 0x2933:
case 0x2B33:
case 0x2D33:
case 0x2134:
case 0x2334:
case 0x2534:
case 0x2734:
case 0x2934:
case 0x2B34:
case 0x2D34:
case 0x2135:
case 0x2335:
case 0x2535:
case 0x2735:
case 0x2935:
case 0x2B35:
case 0x2D35:
case 0x2136:
case 0x2336:
case 0x2536:
case 0x2736:
case 0x2936:
case 0x2B36:
case 0x2D36:
case 0x2137:
case 0x2337:
case 0x2537:
case 0x2737:
case 0x2937:
case 0x2B37:
case 0x2D37:

// MOVEL
case 0x2130:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(26)
case 0x2370:
case 0x2570:
case 0x2770:
case 0x2970:
case 0x2B70:
case 0x2D70:
case 0x2F70:
case 0x2171:
case 0x2371:
case 0x2571:
case 0x2771:
case 0x2971:
case 0x2B71:
case 0x2D71:
case 0x2F71:
case 0x2172:
case 0x2372:
case 0x2572:
case 0x2772:
case 0x2972:
case 0x2B72:
case 0x2D72:
case 0x2F72:
case 0x2173:
case 0x2373:
case 0x2573:
case 0x2773:
case 0x2973:
case 0x2B73:
case 0x2D73:
case 0x2F73:
case 0x2174:
case 0x2374:
case 0x2574:
case 0x2774:
case 0x2974:
case 0x2B74:
case 0x2D74:
case 0x2F74:
case 0x2175:
case 0x2375:
case 0x2575:
case 0x2775:
case 0x2975:
case 0x2B75:
case 0x2D75:
case 0x2F75:
case 0x2176:
case 0x2376:
case 0x2576:
case 0x2776:
case 0x2976:
case 0x2B76:
case 0x2D76:
case 0x2F76:
case 0x2177:
case 0x2377:
case 0x2577:
case 0x2777:
case 0x2977:
case 0x2B77:
case 0x2D77:
case 0x2F77:

// MOVEL
case 0x2170:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)
case 0x23B0:
case 0x25B0:
case 0x27B0:
case 0x29B0:
case 0x2BB0:
case 0x2DB0:
case 0x2FB0:
case 0x21B1:
case 0x23B1:
case 0x25B1:
case 0x27B1:
case 0x29B1:
case 0x2BB1:
case 0x2DB1:
case 0x2FB1:
case 0x21B2:
case 0x23B2:
case 0x25B2:
case 0x27B2:
case 0x29B2:
case 0x2BB2:
case 0x2DB2:
case 0x2FB2:
case 0x21B3:
case 0x23B3:
case 0x25B3:
case 0x27B3:
case 0x29B3:
case 0x2BB3:
case 0x2DB3:
case 0x2FB3:
case 0x21B4:
case 0x23B4:
case 0x25B4:
case 0x27B4:
case 0x29B4:
case 0x2BB4:
case 0x2DB4:
case 0x2FB4:
case 0x21B5:
case 0x23B5:
case 0x25B5:
case 0x27B5:
case 0x29B5:
case 0x2BB5:
case 0x2DB5:
case 0x2FB5:
case 0x21B6:
case 0x23B6:
case 0x25B6:
case 0x27B6:
case 0x29B6:
case 0x2BB6:
case 0x2DB6:
case 0x2FB6:
case 0x21B7:
case 0x23B7:
case 0x25B7:
case 0x27B7:
case 0x29B7:
case 0x2BB7:
case 0x2DB7:
case 0x2FB7:

// MOVEL
case 0x21B0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(32)
case 0x21F1:
case 0x21F2:
case 0x21F3:
case 0x21F4:
case 0x21F5:
case 0x21F6:
case 0x21F7:

// MOVEL
case 0x21F0:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)
case 0x23F1:
case 0x23F2:
case 0x23F3:
case 0x23F4:
case 0x23F5:
case 0x23F6:
case 0x23F7:

// MOVEL
case 0x23F0:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(34)
case 0x2EF1:
case 0x2EF2:
case 0x2EF3:
case 0x2EF4:
case 0x2EF5:
case 0x2EF6:
case 0x2EF7:

// MOVEL
case 0x2EF0:
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
	adr = CPU->A[7];
	CPU->A[7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0x2F31:
case 0x2F32:
case 0x2F33:
case 0x2F34:
case 0x2F35:
case 0x2F36:
case 0x2F37:

// MOVEL
case 0x2F30:
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
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(26)
case 0x2238:
case 0x2438:
case 0x2638:
case 0x2838:
case 0x2A38:
case 0x2C38:
case 0x2E38:

// MOVEL
case 0x2038:
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
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0x22B8:
case 0x24B8:
case 0x26B8:
case 0x28B8:
case 0x2AB8:
case 0x2CB8:
case 0x2EB8:

// MOVEL
case 0x20B8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x22F8:
case 0x24F8:
case 0x26F8:
case 0x28F8:
case 0x2AF8:
case 0x2CF8:

// MOVEL
case 0x20F8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x2338:
case 0x2538:
case 0x2738:
case 0x2938:
case 0x2B38:
case 0x2D38:

// MOVEL
case 0x2138:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(24)
case 0x2378:
case 0x2578:
case 0x2778:
case 0x2978:
case 0x2B78:
case 0x2D78:
case 0x2F78:

// MOVEL
case 0x2178:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0x23B8:
case 0x25B8:
case 0x27B8:
case 0x29B8:
case 0x2BB8:
case 0x2DB8:
case 0x2FB8:

// MOVEL
case 0x21B8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)

// MOVEL
case 0x21F8:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)

// MOVEL
case 0x23F8:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(32)

// MOVEL
case 0x2EF8:
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
	adr = CPU->A[7];
	CPU->A[7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)

// MOVEL
case 0x2F38:
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
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(24)
case 0x2239:
case 0x2439:
case 0x2639:
case 0x2839:
case 0x2A39:
case 0x2C39:
case 0x2E39:

// MOVEL
case 0x2039:
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
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(20)
case 0x22B9:
case 0x24B9:
case 0x26B9:
case 0x28B9:
case 0x2AB9:
case 0x2CB9:
case 0x2EB9:

// MOVEL
case 0x20B9:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0x22F9:
case 0x24F9:
case 0x26F9:
case 0x28F9:
case 0x2AF9:
case 0x2CF9:

// MOVEL
case 0x20F9:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0x2339:
case 0x2539:
case 0x2739:
case 0x2939:
case 0x2B39:
case 0x2D39:

// MOVEL
case 0x2139:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(28)
case 0x2379:
case 0x2579:
case 0x2779:
case 0x2979:
case 0x2B79:
case 0x2D79:
case 0x2F79:

// MOVEL
case 0x2179:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(32)
case 0x23B9:
case 0x25B9:
case 0x27B9:
case 0x29B9:
case 0x2BB9:
case 0x2DB9:
case 0x2FB9:

// MOVEL
case 0x21B9:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(34)

// MOVEL
case 0x21F9:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(32)

// MOVEL
case 0x23F9:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(36)

// MOVEL
case 0x2EF9:
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
	adr = CPU->A[7];
	CPU->A[7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)

// MOVEL
case 0x2F39:
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
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(28)
case 0x223A:
case 0x243A:
case 0x263A:
case 0x283A:
case 0x2A3A:
case 0x2C3A:
case 0x2E3A:

// MOVEL
case 0x203A:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0x22BA:
case 0x24BA:
case 0x26BA:
case 0x28BA:
case 0x2ABA:
case 0x2CBA:
case 0x2EBA:

// MOVEL
case 0x20BA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x22FA:
case 0x24FA:
case 0x26FA:
case 0x28FA:
case 0x2AFA:
case 0x2CFA:

// MOVEL
case 0x20FA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x233A:
case 0x253A:
case 0x273A:
case 0x293A:
case 0x2B3A:
case 0x2D3A:

// MOVEL
case 0x213A:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(24)
case 0x237A:
case 0x257A:
case 0x277A:
case 0x297A:
case 0x2B7A:
case 0x2D7A:
case 0x2F7A:

// MOVEL
case 0x217A:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)
case 0x23BA:
case 0x25BA:
case 0x27BA:
case 0x29BA:
case 0x2BBA:
case 0x2DBA:
case 0x2FBA:

// MOVEL
case 0x21BA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)

// MOVEL
case 0x21FA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)

// MOVEL
case 0x23FA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(32)

// MOVEL
case 0x2EFA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)

// MOVEL
case 0x2F3A:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(24)
case 0x223B:
case 0x243B:
case 0x263B:
case 0x283B:
case 0x2A3B:
case 0x2C3B:
case 0x2E3B:

// MOVEL
case 0x203B:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(18)
case 0x22BB:
case 0x24BB:
case 0x26BB:
case 0x28BB:
case 0x2ABB:
case 0x2CBB:
case 0x2EBB:

// MOVEL
case 0x20BB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0x22FB:
case 0x24FB:
case 0x26FB:
case 0x28FB:
case 0x2AFB:
case 0x2CFB:

// MOVEL
case 0x20FB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0x233B:
case 0x253B:
case 0x273B:
case 0x293B:
case 0x2B3B:
case 0x2D3B:

// MOVEL
case 0x213B:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(26)
case 0x237B:
case 0x257B:
case 0x277B:
case 0x297B:
case 0x2B7B:
case 0x2D7B:
case 0x2F7B:

// MOVEL
case 0x217B:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)
case 0x23BB:
case 0x25BB:
case 0x27BB:
case 0x29BB:
case 0x2BBB:
case 0x2DBB:
case 0x2FBB:

// MOVEL
case 0x21BB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(32)

// MOVEL
case 0x21FB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)

// MOVEL
case 0x23FB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(34)

// MOVEL
case 0x2EFB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)

// MOVEL
case 0x2F3B:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_LONG_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(26)
case 0x223C:
case 0x243C:
case 0x263C:
case 0x283C:
case 0x2A3C:
case 0x2C3C:
case 0x2E3C:

// MOVEL
case 0x203C:
{
	u32 res;
	res = FETCH_LONG;
	PC += 4;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(12)
case 0x22BC:
case 0x24BC:
case 0x26BC:
case 0x28BC:
case 0x2ABC:
case 0x2CBC:
case 0x2EBC:

// MOVEL
case 0x20BC:
{
	u32 adr;
	u32 res;
	res = FETCH_LONG;
	PC += 4;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x22FC:
case 0x24FC:
case 0x26FC:
case 0x28FC:
case 0x2AFC:
case 0x2CFC:

// MOVEL
case 0x20FC:
{
	u32 adr;
	u32 res;
	res = FETCH_LONG;
	PC += 4;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x233C:
case 0x253C:
case 0x273C:
case 0x293C:
case 0x2B3C:
case 0x2D3C:

// MOVEL
case 0x213C:
{
	u32 adr;
	u32 res;
	res = FETCH_LONG;
	PC += 4;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	PRE_IO
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(20)
case 0x237C:
case 0x257C:
case 0x277C:
case 0x297C:
case 0x2B7C:
case 0x2D7C:
case 0x2F7C:

// MOVEL
case 0x217C:
{
	u32 adr;
	u32 res;
	res = FETCH_LONG;
	PC += 4;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x23BC:
case 0x25BC:
case 0x27BC:
case 0x29BC:
case 0x2BBC:
case 0x2DBC:
case 0x2FBC:

// MOVEL
case 0x21BC:
{
	u32 adr;
	u32 res;
	res = FETCH_LONG;
	PC += 4;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)

// MOVEL
case 0x21FC:
{
	u32 adr;
	u32 res;
	res = FETCH_LONG;
	PC += 4;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)

// MOVEL
case 0x23FC:
{
	u32 adr;
	u32 res;
	res = FETCH_LONG;
	PC += 4;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)

// MOVEL
case 0x2EFC:
{
	u32 adr;
	u32 res;
	res = FETCH_LONG;
	PC += 4;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)

// MOVEL
case 0x2F3C:
{
	u32 adr;
	u32 res;
	res = FETCH_LONG;
	PC += 4;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 24;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(20)
case 0x221F:
case 0x241F:
case 0x261F:
case 0x281F:
case 0x2A1F:
case 0x2C1F:
case 0x2E1F:

// MOVEL
case 0x201F:
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
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x229F:
case 0x249F:
case 0x269F:
case 0x289F:
case 0x2A9F:
case 0x2C9F:
case 0x2E9F:

// MOVEL
case 0x209F:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x22DF:
case 0x24DF:
case 0x26DF:
case 0x28DF:
case 0x2ADF:
case 0x2CDF:

// MOVEL
case 0x20DF:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)
case 0x231F:
case 0x251F:
case 0x271F:
case 0x291F:
case 0x2B1F:
case 0x2D1F:

// MOVEL
case 0x211F:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(20)
case 0x235F:
case 0x255F:
case 0x275F:
case 0x295F:
case 0x2B5F:
case 0x2D5F:
case 0x2F5F:

// MOVEL
case 0x215F:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)
case 0x239F:
case 0x259F:
case 0x279F:
case 0x299F:
case 0x2B9F:
case 0x2D9F:
case 0x2F9F:

// MOVEL
case 0x219F:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)

// MOVEL
case 0x21DF:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(24)

// MOVEL
case 0x23DF:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)

// MOVEL
case 0x2EDF:
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
	adr = CPU->A[7];
	CPU->A[7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(20)

// MOVEL
case 0x2F1F:
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
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(20)
case 0x2227:
case 0x2427:
case 0x2627:
case 0x2827:
case 0x2A27:
case 0x2C27:
case 0x2E27:

// MOVEL
case 0x2027:
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
	*((u32*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x22A7:
case 0x24A7:
case 0x26A7:
case 0x28A7:
case 0x2AA7:
case 0x2CA7:
case 0x2EA7:

// MOVEL
case 0x20A7:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x22E7:
case 0x24E7:
case 0x26E7:
case 0x28E7:
case 0x2AE7:
case 0x2CE7:

// MOVEL
case 0x20E7:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)
case 0x2327:
case 0x2527:
case 0x2727:
case 0x2927:
case 0x2B27:
case 0x2D27:

// MOVEL
case 0x2127:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 4;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(22)
case 0x2367:
case 0x2567:
case 0x2767:
case 0x2967:
case 0x2B67:
case 0x2D67:
case 0x2F67:

// MOVEL
case 0x2167:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)
case 0x23A7:
case 0x25A7:
case 0x27A7:
case 0x29A7:
case 0x2BA7:
case 0x2DA7:
case 0x2FA7:

// MOVEL
case 0x21A7:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(28)

// MOVEL
case 0x21E7:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(26)

// MOVEL
case 0x23E7:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(30)

// MOVEL
case 0x2EE7:
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
	adr = CPU->A[7];
	CPU->A[7] += 4;
	WRITE_LONG_F(adr, res)
	POST_IO
}
RET(22)

// MOVEL
case 0x2F27:
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
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	WRITE_LONG_DEC_F(adr, res)
	POST_IO
}
RET(22)
case 0x2240:
case 0x2440:
case 0x2640:
case 0x2840:
case 0x2A40:
case 0x2C40:
case 0x2E40:
case 0x2041:
case 0x2241:
case 0x2441:
case 0x2641:
case 0x2841:
case 0x2A41:
case 0x2C41:
case 0x2E41:
case 0x2042:
case 0x2242:
case 0x2442:
case 0x2642:
case 0x2842:
case 0x2A42:
case 0x2C42:
case 0x2E42:
case 0x2043:
case 0x2243:
case 0x2443:
case 0x2643:
case 0x2843:
case 0x2A43:
case 0x2C43:
case 0x2E43:
case 0x2044:
case 0x2244:
case 0x2444:
case 0x2644:
case 0x2844:
case 0x2A44:
case 0x2C44:
case 0x2E44:
case 0x2045:
case 0x2245:
case 0x2445:
case 0x2645:
case 0x2845:
case 0x2A45:
case 0x2C45:
case 0x2E45:
case 0x2046:
case 0x2246:
case 0x2446:
case 0x2646:
case 0x2846:
case 0x2A46:
case 0x2C46:
case 0x2E46:
case 0x2047:
case 0x2247:
case 0x2447:
case 0x2647:
case 0x2847:
case 0x2A47:
case 0x2C47:
case 0x2E47:

// MOVEAL
case 0x2040:
{
	u32 res;
	res = (s32)(s32)CPU->D[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(4)
case 0x2248:
case 0x2448:
case 0x2648:
case 0x2848:
case 0x2A48:
case 0x2C48:
case 0x2E48:
case 0x2049:
case 0x2249:
case 0x2449:
case 0x2649:
case 0x2849:
case 0x2A49:
case 0x2C49:
case 0x2E49:
case 0x204A:
case 0x224A:
case 0x244A:
case 0x264A:
case 0x284A:
case 0x2A4A:
case 0x2C4A:
case 0x2E4A:
case 0x204B:
case 0x224B:
case 0x244B:
case 0x264B:
case 0x284B:
case 0x2A4B:
case 0x2C4B:
case 0x2E4B:
case 0x204C:
case 0x224C:
case 0x244C:
case 0x264C:
case 0x284C:
case 0x2A4C:
case 0x2C4C:
case 0x2E4C:
case 0x204D:
case 0x224D:
case 0x244D:
case 0x264D:
case 0x284D:
case 0x2A4D:
case 0x2C4D:
case 0x2E4D:
case 0x204E:
case 0x224E:
case 0x244E:
case 0x264E:
case 0x284E:
case 0x2A4E:
case 0x2C4E:
case 0x2E4E:
case 0x204F:
case 0x224F:
case 0x244F:
case 0x264F:
case 0x284F:
case 0x2A4F:
case 0x2C4F:
case 0x2E4F:

// MOVEAL
case 0x2048:
{
	u32 res;
	res = (s32)(s32)CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(4)
case 0x2250:
case 0x2450:
case 0x2650:
case 0x2850:
case 0x2A50:
case 0x2C50:
case 0x2E50:
case 0x2051:
case 0x2251:
case 0x2451:
case 0x2651:
case 0x2851:
case 0x2A51:
case 0x2C51:
case 0x2E51:
case 0x2052:
case 0x2252:
case 0x2452:
case 0x2652:
case 0x2852:
case 0x2A52:
case 0x2C52:
case 0x2E52:
case 0x2053:
case 0x2253:
case 0x2453:
case 0x2653:
case 0x2853:
case 0x2A53:
case 0x2C53:
case 0x2E53:
case 0x2054:
case 0x2254:
case 0x2454:
case 0x2654:
case 0x2854:
case 0x2A54:
case 0x2C54:
case 0x2E54:
case 0x2055:
case 0x2255:
case 0x2455:
case 0x2655:
case 0x2855:
case 0x2A55:
case 0x2C55:
case 0x2E55:
case 0x2056:
case 0x2256:
case 0x2456:
case 0x2656:
case 0x2856:
case 0x2A56:
case 0x2C56:
case 0x2E56:
case 0x2057:
case 0x2257:
case 0x2457:
case 0x2657:
case 0x2857:
case 0x2A57:
case 0x2C57:
case 0x2E57:

// MOVEAL
case 0x2050:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READSX_LONG_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(12)
case 0x2258:
case 0x2458:
case 0x2658:
case 0x2858:
case 0x2A58:
case 0x2C58:
case 0x2E58:
case 0x2059:
case 0x2259:
case 0x2459:
case 0x2659:
case 0x2859:
case 0x2A59:
case 0x2C59:
case 0x2E59:
case 0x205A:
case 0x225A:
case 0x245A:
case 0x265A:
case 0x285A:
case 0x2A5A:
case 0x2C5A:
case 0x2E5A:
case 0x205B:
case 0x225B:
case 0x245B:
case 0x265B:
case 0x285B:
case 0x2A5B:
case 0x2C5B:
case 0x2E5B:
case 0x205C:
case 0x225C:
case 0x245C:
case 0x265C:
case 0x285C:
case 0x2A5C:
case 0x2C5C:
case 0x2E5C:
case 0x205D:
case 0x225D:
case 0x245D:
case 0x265D:
case 0x285D:
case 0x2A5D:
case 0x2C5D:
case 0x2E5D:
case 0x205E:
case 0x225E:
case 0x245E:
case 0x265E:
case 0x285E:
case 0x2A5E:
case 0x2C5E:
case 0x2E5E:

// MOVEAL
case 0x2058:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 4;
	PRE_IO
	READSX_LONG_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(12)
case 0x2260:
case 0x2460:
case 0x2660:
case 0x2860:
case 0x2A60:
case 0x2C60:
case 0x2E60:
case 0x2061:
case 0x2261:
case 0x2461:
case 0x2661:
case 0x2861:
case 0x2A61:
case 0x2C61:
case 0x2E61:
case 0x2062:
case 0x2262:
case 0x2462:
case 0x2662:
case 0x2862:
case 0x2A62:
case 0x2C62:
case 0x2E62:
case 0x2063:
case 0x2263:
case 0x2463:
case 0x2663:
case 0x2863:
case 0x2A63:
case 0x2C63:
case 0x2E63:
case 0x2064:
case 0x2264:
case 0x2464:
case 0x2664:
case 0x2864:
case 0x2A64:
case 0x2C64:
case 0x2E64:
case 0x2065:
case 0x2265:
case 0x2465:
case 0x2665:
case 0x2865:
case 0x2A65:
case 0x2C65:
case 0x2E65:
case 0x2066:
case 0x2266:
case 0x2466:
case 0x2666:
case 0x2866:
case 0x2A66:
case 0x2C66:
case 0x2E66:

// MOVEAL
case 0x2060:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 4;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READSX_LONG_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0x2268:
case 0x2468:
case 0x2668:
case 0x2868:
case 0x2A68:
case 0x2C68:
case 0x2E68:
case 0x2069:
case 0x2269:
case 0x2469:
case 0x2669:
case 0x2869:
case 0x2A69:
case 0x2C69:
case 0x2E69:
case 0x206A:
case 0x226A:
case 0x246A:
case 0x266A:
case 0x286A:
case 0x2A6A:
case 0x2C6A:
case 0x2E6A:
case 0x206B:
case 0x226B:
case 0x246B:
case 0x266B:
case 0x286B:
case 0x2A6B:
case 0x2C6B:
case 0x2E6B:
case 0x206C:
case 0x226C:
case 0x246C:
case 0x266C:
case 0x286C:
case 0x2A6C:
case 0x2C6C:
case 0x2E6C:
case 0x206D:
case 0x226D:
case 0x246D:
case 0x266D:
case 0x286D:
case 0x2A6D:
case 0x2C6D:
case 0x2E6D:
case 0x206E:
case 0x226E:
case 0x246E:
case 0x266E:
case 0x286E:
case 0x2A6E:
case 0x2C6E:
case 0x2E6E:
case 0x206F:
case 0x226F:
case 0x246F:
case 0x266F:
case 0x286F:
case 0x2A6F:
case 0x2C6F:
case 0x2E6F:

// MOVEAL
case 0x2068:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_LONG_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(16)
case 0x2270:
case 0x2470:
case 0x2670:
case 0x2870:
case 0x2A70:
case 0x2C70:
case 0x2E70:
case 0x2071:
case 0x2271:
case 0x2471:
case 0x2671:
case 0x2871:
case 0x2A71:
case 0x2C71:
case 0x2E71:
case 0x2072:
case 0x2272:
case 0x2472:
case 0x2672:
case 0x2872:
case 0x2A72:
case 0x2C72:
case 0x2E72:
case 0x2073:
case 0x2273:
case 0x2473:
case 0x2673:
case 0x2873:
case 0x2A73:
case 0x2C73:
case 0x2E73:
case 0x2074:
case 0x2274:
case 0x2474:
case 0x2674:
case 0x2874:
case 0x2A74:
case 0x2C74:
case 0x2E74:
case 0x2075:
case 0x2275:
case 0x2475:
case 0x2675:
case 0x2875:
case 0x2A75:
case 0x2C75:
case 0x2E75:
case 0x2076:
case 0x2276:
case 0x2476:
case 0x2676:
case 0x2876:
case 0x2A76:
case 0x2C76:
case 0x2E76:
case 0x2077:
case 0x2277:
case 0x2477:
case 0x2677:
case 0x2877:
case 0x2A77:
case 0x2C77:
case 0x2E77:

// MOVEAL
case 0x2070:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READSX_LONG_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(18)
case 0x2278:
case 0x2478:
case 0x2678:
case 0x2878:
case 0x2A78:
case 0x2C78:
case 0x2E78:

// MOVEAL
case 0x2078:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_LONG_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(16)
case 0x2279:
case 0x2479:
case 0x2679:
case 0x2879:
case 0x2A79:
case 0x2C79:
case 0x2E79:

// MOVEAL
case 0x2079:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READSX_LONG_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(20)
case 0x227A:
case 0x247A:
case 0x267A:
case 0x287A:
case 0x2A7A:
case 0x2C7A:
case 0x2E7A:

// MOVEAL
case 0x207A:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_LONG_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(16)
case 0x227B:
case 0x247B:
case 0x267B:
case 0x287B:
case 0x2A7B:
case 0x2C7B:
case 0x2E7B:

// MOVEAL
case 0x207B:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READSX_LONG_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(18)
case 0x227C:
case 0x247C:
case 0x267C:
case 0x287C:
case 0x2A7C:
case 0x2C7C:
case 0x2E7C:

// MOVEAL
case 0x207C:
{
	u32 res;
	res = (s32)(s32)FETCH_LONG;
	PC += 4;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(12)
case 0x225F:
case 0x245F:
case 0x265F:
case 0x285F:
case 0x2A5F:
case 0x2C5F:
case 0x2E5F:

// MOVEAL
case 0x205F:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 4;
	PRE_IO
	READSX_LONG_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(12)
case 0x2267:
case 0x2467:
case 0x2667:
case 0x2867:
case 0x2A67:
case 0x2C67:
case 0x2E67:

// MOVEAL
case 0x2067:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 4;
	CPU->A[7] = adr;
	PRE_IO
	READSX_LONG_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
