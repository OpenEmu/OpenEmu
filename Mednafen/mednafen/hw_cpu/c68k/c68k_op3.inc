case 0x3200:
case 0x3400:
case 0x3600:
case 0x3800:
case 0x3A00:
case 0x3C00:
case 0x3E00:
case 0x3001:
case 0x3201:
case 0x3401:
case 0x3601:
case 0x3801:
case 0x3A01:
case 0x3C01:
case 0x3E01:
case 0x3002:
case 0x3202:
case 0x3402:
case 0x3602:
case 0x3802:
case 0x3A02:
case 0x3C02:
case 0x3E02:
case 0x3003:
case 0x3203:
case 0x3403:
case 0x3603:
case 0x3803:
case 0x3A03:
case 0x3C03:
case 0x3E03:
case 0x3004:
case 0x3204:
case 0x3404:
case 0x3604:
case 0x3804:
case 0x3A04:
case 0x3C04:
case 0x3E04:
case 0x3005:
case 0x3205:
case 0x3405:
case 0x3605:
case 0x3805:
case 0x3A05:
case 0x3C05:
case 0x3E05:
case 0x3006:
case 0x3206:
case 0x3406:
case 0x3606:
case 0x3806:
case 0x3A06:
case 0x3C06:
case 0x3E06:
case 0x3007:
case 0x3207:
case 0x3407:
case 0x3607:
case 0x3807:
case 0x3A07:
case 0x3C07:
case 0x3E07:

// MOVEW
case 0x3000:
{
	u32 res;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0x3280:
case 0x3480:
case 0x3680:
case 0x3880:
case 0x3A80:
case 0x3C80:
case 0x3E80:
case 0x3081:
case 0x3281:
case 0x3481:
case 0x3681:
case 0x3881:
case 0x3A81:
case 0x3C81:
case 0x3E81:
case 0x3082:
case 0x3282:
case 0x3482:
case 0x3682:
case 0x3882:
case 0x3A82:
case 0x3C82:
case 0x3E82:
case 0x3083:
case 0x3283:
case 0x3483:
case 0x3683:
case 0x3883:
case 0x3A83:
case 0x3C83:
case 0x3E83:
case 0x3084:
case 0x3284:
case 0x3484:
case 0x3684:
case 0x3884:
case 0x3A84:
case 0x3C84:
case 0x3E84:
case 0x3085:
case 0x3285:
case 0x3485:
case 0x3685:
case 0x3885:
case 0x3A85:
case 0x3C85:
case 0x3E85:
case 0x3086:
case 0x3286:
case 0x3486:
case 0x3686:
case 0x3886:
case 0x3A86:
case 0x3C86:
case 0x3E86:
case 0x3087:
case 0x3287:
case 0x3487:
case 0x3687:
case 0x3887:
case 0x3A87:
case 0x3C87:
case 0x3E87:

// MOVEW
case 0x3080:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(8)
case 0x32C0:
case 0x34C0:
case 0x36C0:
case 0x38C0:
case 0x3AC0:
case 0x3CC0:
case 0x30C1:
case 0x32C1:
case 0x34C1:
case 0x36C1:
case 0x38C1:
case 0x3AC1:
case 0x3CC1:
case 0x30C2:
case 0x32C2:
case 0x34C2:
case 0x36C2:
case 0x38C2:
case 0x3AC2:
case 0x3CC2:
case 0x30C3:
case 0x32C3:
case 0x34C3:
case 0x36C3:
case 0x38C3:
case 0x3AC3:
case 0x3CC3:
case 0x30C4:
case 0x32C4:
case 0x34C4:
case 0x36C4:
case 0x38C4:
case 0x3AC4:
case 0x3CC4:
case 0x30C5:
case 0x32C5:
case 0x34C5:
case 0x36C5:
case 0x38C5:
case 0x3AC5:
case 0x3CC5:
case 0x30C6:
case 0x32C6:
case 0x34C6:
case 0x36C6:
case 0x38C6:
case 0x3AC6:
case 0x3CC6:
case 0x30C7:
case 0x32C7:
case 0x34C7:
case 0x36C7:
case 0x38C7:
case 0x3AC7:
case 0x3CC7:

// MOVEW
case 0x30C0:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(8)
case 0x3300:
case 0x3500:
case 0x3700:
case 0x3900:
case 0x3B00:
case 0x3D00:
case 0x3101:
case 0x3301:
case 0x3501:
case 0x3701:
case 0x3901:
case 0x3B01:
case 0x3D01:
case 0x3102:
case 0x3302:
case 0x3502:
case 0x3702:
case 0x3902:
case 0x3B02:
case 0x3D02:
case 0x3103:
case 0x3303:
case 0x3503:
case 0x3703:
case 0x3903:
case 0x3B03:
case 0x3D03:
case 0x3104:
case 0x3304:
case 0x3504:
case 0x3704:
case 0x3904:
case 0x3B04:
case 0x3D04:
case 0x3105:
case 0x3305:
case 0x3505:
case 0x3705:
case 0x3905:
case 0x3B05:
case 0x3D05:
case 0x3106:
case 0x3306:
case 0x3506:
case 0x3706:
case 0x3906:
case 0x3B06:
case 0x3D06:
case 0x3107:
case 0x3307:
case 0x3507:
case 0x3707:
case 0x3907:
case 0x3B07:
case 0x3D07:

// MOVEW
case 0x3100:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(8)
case 0x3340:
case 0x3540:
case 0x3740:
case 0x3940:
case 0x3B40:
case 0x3D40:
case 0x3F40:
case 0x3141:
case 0x3341:
case 0x3541:
case 0x3741:
case 0x3941:
case 0x3B41:
case 0x3D41:
case 0x3F41:
case 0x3142:
case 0x3342:
case 0x3542:
case 0x3742:
case 0x3942:
case 0x3B42:
case 0x3D42:
case 0x3F42:
case 0x3143:
case 0x3343:
case 0x3543:
case 0x3743:
case 0x3943:
case 0x3B43:
case 0x3D43:
case 0x3F43:
case 0x3144:
case 0x3344:
case 0x3544:
case 0x3744:
case 0x3944:
case 0x3B44:
case 0x3D44:
case 0x3F44:
case 0x3145:
case 0x3345:
case 0x3545:
case 0x3745:
case 0x3945:
case 0x3B45:
case 0x3D45:
case 0x3F45:
case 0x3146:
case 0x3346:
case 0x3546:
case 0x3746:
case 0x3946:
case 0x3B46:
case 0x3D46:
case 0x3F46:
case 0x3147:
case 0x3347:
case 0x3547:
case 0x3747:
case 0x3947:
case 0x3B47:
case 0x3D47:
case 0x3F47:

// MOVEW
case 0x3140:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x3380:
case 0x3580:
case 0x3780:
case 0x3980:
case 0x3B80:
case 0x3D80:
case 0x3F80:
case 0x3181:
case 0x3381:
case 0x3581:
case 0x3781:
case 0x3981:
case 0x3B81:
case 0x3D81:
case 0x3F81:
case 0x3182:
case 0x3382:
case 0x3582:
case 0x3782:
case 0x3982:
case 0x3B82:
case 0x3D82:
case 0x3F82:
case 0x3183:
case 0x3383:
case 0x3583:
case 0x3783:
case 0x3983:
case 0x3B83:
case 0x3D83:
case 0x3F83:
case 0x3184:
case 0x3384:
case 0x3584:
case 0x3784:
case 0x3984:
case 0x3B84:
case 0x3D84:
case 0x3F84:
case 0x3185:
case 0x3385:
case 0x3585:
case 0x3785:
case 0x3985:
case 0x3B85:
case 0x3D85:
case 0x3F85:
case 0x3186:
case 0x3386:
case 0x3586:
case 0x3786:
case 0x3986:
case 0x3B86:
case 0x3D86:
case 0x3F86:
case 0x3187:
case 0x3387:
case 0x3587:
case 0x3787:
case 0x3987:
case 0x3B87:
case 0x3D87:
case 0x3F87:

// MOVEW
case 0x3180:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x31C1:
case 0x31C2:
case 0x31C3:
case 0x31C4:
case 0x31C5:
case 0x31C6:
case 0x31C7:

// MOVEW
case 0x31C0:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x33C1:
case 0x33C2:
case 0x33C3:
case 0x33C4:
case 0x33C5:
case 0x33C6:
case 0x33C7:

// MOVEW
case 0x33C0:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x3EC1:
case 0x3EC2:
case 0x3EC3:
case 0x3EC4:
case 0x3EC5:
case 0x3EC6:
case 0x3EC7:

// MOVEW
case 0x3EC0:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(8)
case 0x3F01:
case 0x3F02:
case 0x3F03:
case 0x3F04:
case 0x3F05:
case 0x3F06:
case 0x3F07:

// MOVEW
case 0x3F00:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->D[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(8)
case 0x3208:
case 0x3408:
case 0x3608:
case 0x3808:
case 0x3A08:
case 0x3C08:
case 0x3E08:
case 0x3009:
case 0x3209:
case 0x3409:
case 0x3609:
case 0x3809:
case 0x3A09:
case 0x3C09:
case 0x3E09:
case 0x300A:
case 0x320A:
case 0x340A:
case 0x360A:
case 0x380A:
case 0x3A0A:
case 0x3C0A:
case 0x3E0A:
case 0x300B:
case 0x320B:
case 0x340B:
case 0x360B:
case 0x380B:
case 0x3A0B:
case 0x3C0B:
case 0x3E0B:
case 0x300C:
case 0x320C:
case 0x340C:
case 0x360C:
case 0x380C:
case 0x3A0C:
case 0x3C0C:
case 0x3E0C:
case 0x300D:
case 0x320D:
case 0x340D:
case 0x360D:
case 0x380D:
case 0x3A0D:
case 0x3C0D:
case 0x3E0D:
case 0x300E:
case 0x320E:
case 0x340E:
case 0x360E:
case 0x380E:
case 0x3A0E:
case 0x3C0E:
case 0x3E0E:
case 0x300F:
case 0x320F:
case 0x340F:
case 0x360F:
case 0x380F:
case 0x3A0F:
case 0x3C0F:
case 0x3E0F:

// MOVEW
case 0x3008:
{
	u32 res;
	res = (u16)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(4)
case 0x3288:
case 0x3488:
case 0x3688:
case 0x3888:
case 0x3A88:
case 0x3C88:
case 0x3E88:
case 0x3089:
case 0x3289:
case 0x3489:
case 0x3689:
case 0x3889:
case 0x3A89:
case 0x3C89:
case 0x3E89:
case 0x308A:
case 0x328A:
case 0x348A:
case 0x368A:
case 0x388A:
case 0x3A8A:
case 0x3C8A:
case 0x3E8A:
case 0x308B:
case 0x328B:
case 0x348B:
case 0x368B:
case 0x388B:
case 0x3A8B:
case 0x3C8B:
case 0x3E8B:
case 0x308C:
case 0x328C:
case 0x348C:
case 0x368C:
case 0x388C:
case 0x3A8C:
case 0x3C8C:
case 0x3E8C:
case 0x308D:
case 0x328D:
case 0x348D:
case 0x368D:
case 0x388D:
case 0x3A8D:
case 0x3C8D:
case 0x3E8D:
case 0x308E:
case 0x328E:
case 0x348E:
case 0x368E:
case 0x388E:
case 0x3A8E:
case 0x3C8E:
case 0x3E8E:
case 0x308F:
case 0x328F:
case 0x348F:
case 0x368F:
case 0x388F:
case 0x3A8F:
case 0x3C8F:
case 0x3E8F:

// MOVEW
case 0x3088:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(8)
case 0x32C8:
case 0x34C8:
case 0x36C8:
case 0x38C8:
case 0x3AC8:
case 0x3CC8:
case 0x30C9:
case 0x32C9:
case 0x34C9:
case 0x36C9:
case 0x38C9:
case 0x3AC9:
case 0x3CC9:
case 0x30CA:
case 0x32CA:
case 0x34CA:
case 0x36CA:
case 0x38CA:
case 0x3ACA:
case 0x3CCA:
case 0x30CB:
case 0x32CB:
case 0x34CB:
case 0x36CB:
case 0x38CB:
case 0x3ACB:
case 0x3CCB:
case 0x30CC:
case 0x32CC:
case 0x34CC:
case 0x36CC:
case 0x38CC:
case 0x3ACC:
case 0x3CCC:
case 0x30CD:
case 0x32CD:
case 0x34CD:
case 0x36CD:
case 0x38CD:
case 0x3ACD:
case 0x3CCD:
case 0x30CE:
case 0x32CE:
case 0x34CE:
case 0x36CE:
case 0x38CE:
case 0x3ACE:
case 0x3CCE:
case 0x30CF:
case 0x32CF:
case 0x34CF:
case 0x36CF:
case 0x38CF:
case 0x3ACF:
case 0x3CCF:

// MOVEW
case 0x30C8:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(8)
case 0x3308:
case 0x3508:
case 0x3708:
case 0x3908:
case 0x3B08:
case 0x3D08:
case 0x3109:
case 0x3309:
case 0x3509:
case 0x3709:
case 0x3909:
case 0x3B09:
case 0x3D09:
case 0x310A:
case 0x330A:
case 0x350A:
case 0x370A:
case 0x390A:
case 0x3B0A:
case 0x3D0A:
case 0x310B:
case 0x330B:
case 0x350B:
case 0x370B:
case 0x390B:
case 0x3B0B:
case 0x3D0B:
case 0x310C:
case 0x330C:
case 0x350C:
case 0x370C:
case 0x390C:
case 0x3B0C:
case 0x3D0C:
case 0x310D:
case 0x330D:
case 0x350D:
case 0x370D:
case 0x390D:
case 0x3B0D:
case 0x3D0D:
case 0x310E:
case 0x330E:
case 0x350E:
case 0x370E:
case 0x390E:
case 0x3B0E:
case 0x3D0E:
case 0x310F:
case 0x330F:
case 0x350F:
case 0x370F:
case 0x390F:
case 0x3B0F:
case 0x3D0F:

// MOVEW
case 0x3108:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(8)
case 0x3348:
case 0x3548:
case 0x3748:
case 0x3948:
case 0x3B48:
case 0x3D48:
case 0x3F48:
case 0x3149:
case 0x3349:
case 0x3549:
case 0x3749:
case 0x3949:
case 0x3B49:
case 0x3D49:
case 0x3F49:
case 0x314A:
case 0x334A:
case 0x354A:
case 0x374A:
case 0x394A:
case 0x3B4A:
case 0x3D4A:
case 0x3F4A:
case 0x314B:
case 0x334B:
case 0x354B:
case 0x374B:
case 0x394B:
case 0x3B4B:
case 0x3D4B:
case 0x3F4B:
case 0x314C:
case 0x334C:
case 0x354C:
case 0x374C:
case 0x394C:
case 0x3B4C:
case 0x3D4C:
case 0x3F4C:
case 0x314D:
case 0x334D:
case 0x354D:
case 0x374D:
case 0x394D:
case 0x3B4D:
case 0x3D4D:
case 0x3F4D:
case 0x314E:
case 0x334E:
case 0x354E:
case 0x374E:
case 0x394E:
case 0x3B4E:
case 0x3D4E:
case 0x3F4E:
case 0x314F:
case 0x334F:
case 0x354F:
case 0x374F:
case 0x394F:
case 0x3B4F:
case 0x3D4F:
case 0x3F4F:

// MOVEW
case 0x3148:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x3388:
case 0x3588:
case 0x3788:
case 0x3988:
case 0x3B88:
case 0x3D88:
case 0x3F88:
case 0x3189:
case 0x3389:
case 0x3589:
case 0x3789:
case 0x3989:
case 0x3B89:
case 0x3D89:
case 0x3F89:
case 0x318A:
case 0x338A:
case 0x358A:
case 0x378A:
case 0x398A:
case 0x3B8A:
case 0x3D8A:
case 0x3F8A:
case 0x318B:
case 0x338B:
case 0x358B:
case 0x378B:
case 0x398B:
case 0x3B8B:
case 0x3D8B:
case 0x3F8B:
case 0x318C:
case 0x338C:
case 0x358C:
case 0x378C:
case 0x398C:
case 0x3B8C:
case 0x3D8C:
case 0x3F8C:
case 0x318D:
case 0x338D:
case 0x358D:
case 0x378D:
case 0x398D:
case 0x3B8D:
case 0x3D8D:
case 0x3F8D:
case 0x318E:
case 0x338E:
case 0x358E:
case 0x378E:
case 0x398E:
case 0x3B8E:
case 0x3D8E:
case 0x3F8E:
case 0x318F:
case 0x338F:
case 0x358F:
case 0x378F:
case 0x398F:
case 0x3B8F:
case 0x3D8F:
case 0x3F8F:

// MOVEW
case 0x3188:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x31C9:
case 0x31CA:
case 0x31CB:
case 0x31CC:
case 0x31CD:
case 0x31CE:
case 0x31CF:

// MOVEW
case 0x31C8:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x33C9:
case 0x33CA:
case 0x33CB:
case 0x33CC:
case 0x33CD:
case 0x33CE:
case 0x33CF:

// MOVEW
case 0x33C8:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x3EC9:
case 0x3ECA:
case 0x3ECB:
case 0x3ECC:
case 0x3ECD:
case 0x3ECE:
case 0x3ECF:

// MOVEW
case 0x3EC8:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(8)
case 0x3F09:
case 0x3F0A:
case 0x3F0B:
case 0x3F0C:
case 0x3F0D:
case 0x3F0E:
case 0x3F0F:

// MOVEW
case 0x3F08:
{
	u32 adr;
	u32 res;
	res = (u16)CPU->A[(Opcode >> 0) & 7];
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(8)
case 0x3210:
case 0x3410:
case 0x3610:
case 0x3810:
case 0x3A10:
case 0x3C10:
case 0x3E10:
case 0x3011:
case 0x3211:
case 0x3411:
case 0x3611:
case 0x3811:
case 0x3A11:
case 0x3C11:
case 0x3E11:
case 0x3012:
case 0x3212:
case 0x3412:
case 0x3612:
case 0x3812:
case 0x3A12:
case 0x3C12:
case 0x3E12:
case 0x3013:
case 0x3213:
case 0x3413:
case 0x3613:
case 0x3813:
case 0x3A13:
case 0x3C13:
case 0x3E13:
case 0x3014:
case 0x3214:
case 0x3414:
case 0x3614:
case 0x3814:
case 0x3A14:
case 0x3C14:
case 0x3E14:
case 0x3015:
case 0x3215:
case 0x3415:
case 0x3615:
case 0x3815:
case 0x3A15:
case 0x3C15:
case 0x3E15:
case 0x3016:
case 0x3216:
case 0x3416:
case 0x3616:
case 0x3816:
case 0x3A16:
case 0x3C16:
case 0x3E16:
case 0x3017:
case 0x3217:
case 0x3417:
case 0x3617:
case 0x3817:
case 0x3A17:
case 0x3C17:
case 0x3E17:

// MOVEW
case 0x3010:
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
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x3290:
case 0x3490:
case 0x3690:
case 0x3890:
case 0x3A90:
case 0x3C90:
case 0x3E90:
case 0x3091:
case 0x3291:
case 0x3491:
case 0x3691:
case 0x3891:
case 0x3A91:
case 0x3C91:
case 0x3E91:
case 0x3092:
case 0x3292:
case 0x3492:
case 0x3692:
case 0x3892:
case 0x3A92:
case 0x3C92:
case 0x3E92:
case 0x3093:
case 0x3293:
case 0x3493:
case 0x3693:
case 0x3893:
case 0x3A93:
case 0x3C93:
case 0x3E93:
case 0x3094:
case 0x3294:
case 0x3494:
case 0x3694:
case 0x3894:
case 0x3A94:
case 0x3C94:
case 0x3E94:
case 0x3095:
case 0x3295:
case 0x3495:
case 0x3695:
case 0x3895:
case 0x3A95:
case 0x3C95:
case 0x3E95:
case 0x3096:
case 0x3296:
case 0x3496:
case 0x3696:
case 0x3896:
case 0x3A96:
case 0x3C96:
case 0x3E96:
case 0x3097:
case 0x3297:
case 0x3497:
case 0x3697:
case 0x3897:
case 0x3A97:
case 0x3C97:
case 0x3E97:

// MOVEW
case 0x3090:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x32D0:
case 0x34D0:
case 0x36D0:
case 0x38D0:
case 0x3AD0:
case 0x3CD0:
case 0x30D1:
case 0x32D1:
case 0x34D1:
case 0x36D1:
case 0x38D1:
case 0x3AD1:
case 0x3CD1:
case 0x30D2:
case 0x32D2:
case 0x34D2:
case 0x36D2:
case 0x38D2:
case 0x3AD2:
case 0x3CD2:
case 0x30D3:
case 0x32D3:
case 0x34D3:
case 0x36D3:
case 0x38D3:
case 0x3AD3:
case 0x3CD3:
case 0x30D4:
case 0x32D4:
case 0x34D4:
case 0x36D4:
case 0x38D4:
case 0x3AD4:
case 0x3CD4:
case 0x30D5:
case 0x32D5:
case 0x34D5:
case 0x36D5:
case 0x38D5:
case 0x3AD5:
case 0x3CD5:
case 0x30D6:
case 0x32D6:
case 0x34D6:
case 0x36D6:
case 0x38D6:
case 0x3AD6:
case 0x3CD6:
case 0x30D7:
case 0x32D7:
case 0x34D7:
case 0x36D7:
case 0x38D7:
case 0x3AD7:
case 0x3CD7:

// MOVEW
case 0x30D0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x3310:
case 0x3510:
case 0x3710:
case 0x3910:
case 0x3B10:
case 0x3D10:
case 0x3111:
case 0x3311:
case 0x3511:
case 0x3711:
case 0x3911:
case 0x3B11:
case 0x3D11:
case 0x3112:
case 0x3312:
case 0x3512:
case 0x3712:
case 0x3912:
case 0x3B12:
case 0x3D12:
case 0x3113:
case 0x3313:
case 0x3513:
case 0x3713:
case 0x3913:
case 0x3B13:
case 0x3D13:
case 0x3114:
case 0x3314:
case 0x3514:
case 0x3714:
case 0x3914:
case 0x3B14:
case 0x3D14:
case 0x3115:
case 0x3315:
case 0x3515:
case 0x3715:
case 0x3915:
case 0x3B15:
case 0x3D15:
case 0x3116:
case 0x3316:
case 0x3516:
case 0x3716:
case 0x3916:
case 0x3B16:
case 0x3D16:
case 0x3117:
case 0x3317:
case 0x3517:
case 0x3717:
case 0x3917:
case 0x3B17:
case 0x3D17:

// MOVEW
case 0x3110:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x3350:
case 0x3550:
case 0x3750:
case 0x3950:
case 0x3B50:
case 0x3D50:
case 0x3F50:
case 0x3151:
case 0x3351:
case 0x3551:
case 0x3751:
case 0x3951:
case 0x3B51:
case 0x3D51:
case 0x3F51:
case 0x3152:
case 0x3352:
case 0x3552:
case 0x3752:
case 0x3952:
case 0x3B52:
case 0x3D52:
case 0x3F52:
case 0x3153:
case 0x3353:
case 0x3553:
case 0x3753:
case 0x3953:
case 0x3B53:
case 0x3D53:
case 0x3F53:
case 0x3154:
case 0x3354:
case 0x3554:
case 0x3754:
case 0x3954:
case 0x3B54:
case 0x3D54:
case 0x3F54:
case 0x3155:
case 0x3355:
case 0x3555:
case 0x3755:
case 0x3955:
case 0x3B55:
case 0x3D55:
case 0x3F55:
case 0x3156:
case 0x3356:
case 0x3556:
case 0x3756:
case 0x3956:
case 0x3B56:
case 0x3D56:
case 0x3F56:
case 0x3157:
case 0x3357:
case 0x3557:
case 0x3757:
case 0x3957:
case 0x3B57:
case 0x3D57:
case 0x3F57:

// MOVEW
case 0x3150:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x3390:
case 0x3590:
case 0x3790:
case 0x3990:
case 0x3B90:
case 0x3D90:
case 0x3F90:
case 0x3191:
case 0x3391:
case 0x3591:
case 0x3791:
case 0x3991:
case 0x3B91:
case 0x3D91:
case 0x3F91:
case 0x3192:
case 0x3392:
case 0x3592:
case 0x3792:
case 0x3992:
case 0x3B92:
case 0x3D92:
case 0x3F92:
case 0x3193:
case 0x3393:
case 0x3593:
case 0x3793:
case 0x3993:
case 0x3B93:
case 0x3D93:
case 0x3F93:
case 0x3194:
case 0x3394:
case 0x3594:
case 0x3794:
case 0x3994:
case 0x3B94:
case 0x3D94:
case 0x3F94:
case 0x3195:
case 0x3395:
case 0x3595:
case 0x3795:
case 0x3995:
case 0x3B95:
case 0x3D95:
case 0x3F95:
case 0x3196:
case 0x3396:
case 0x3596:
case 0x3796:
case 0x3996:
case 0x3B96:
case 0x3D96:
case 0x3F96:
case 0x3197:
case 0x3397:
case 0x3597:
case 0x3797:
case 0x3997:
case 0x3B97:
case 0x3D97:
case 0x3F97:

// MOVEW
case 0x3190:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x31D1:
case 0x31D2:
case 0x31D3:
case 0x31D4:
case 0x31D5:
case 0x31D6:
case 0x31D7:

// MOVEW
case 0x31D0:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x33D1:
case 0x33D2:
case 0x33D3:
case 0x33D4:
case 0x33D5:
case 0x33D6:
case 0x33D7:

// MOVEW
case 0x33D0:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0x3ED1:
case 0x3ED2:
case 0x3ED3:
case 0x3ED4:
case 0x3ED5:
case 0x3ED6:
case 0x3ED7:

// MOVEW
case 0x3ED0:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x3F11:
case 0x3F12:
case 0x3F13:
case 0x3F14:
case 0x3F15:
case 0x3F16:
case 0x3F17:

// MOVEW
case 0x3F10:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x3218:
case 0x3418:
case 0x3618:
case 0x3818:
case 0x3A18:
case 0x3C18:
case 0x3E18:
case 0x3019:
case 0x3219:
case 0x3419:
case 0x3619:
case 0x3819:
case 0x3A19:
case 0x3C19:
case 0x3E19:
case 0x301A:
case 0x321A:
case 0x341A:
case 0x361A:
case 0x381A:
case 0x3A1A:
case 0x3C1A:
case 0x3E1A:
case 0x301B:
case 0x321B:
case 0x341B:
case 0x361B:
case 0x381B:
case 0x3A1B:
case 0x3C1B:
case 0x3E1B:
case 0x301C:
case 0x321C:
case 0x341C:
case 0x361C:
case 0x381C:
case 0x3A1C:
case 0x3C1C:
case 0x3E1C:
case 0x301D:
case 0x321D:
case 0x341D:
case 0x361D:
case 0x381D:
case 0x3A1D:
case 0x3C1D:
case 0x3E1D:
case 0x301E:
case 0x321E:
case 0x341E:
case 0x361E:
case 0x381E:
case 0x3A1E:
case 0x3C1E:
case 0x3E1E:

// MOVEW
case 0x3018:
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
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x3298:
case 0x3498:
case 0x3698:
case 0x3898:
case 0x3A98:
case 0x3C98:
case 0x3E98:
case 0x3099:
case 0x3299:
case 0x3499:
case 0x3699:
case 0x3899:
case 0x3A99:
case 0x3C99:
case 0x3E99:
case 0x309A:
case 0x329A:
case 0x349A:
case 0x369A:
case 0x389A:
case 0x3A9A:
case 0x3C9A:
case 0x3E9A:
case 0x309B:
case 0x329B:
case 0x349B:
case 0x369B:
case 0x389B:
case 0x3A9B:
case 0x3C9B:
case 0x3E9B:
case 0x309C:
case 0x329C:
case 0x349C:
case 0x369C:
case 0x389C:
case 0x3A9C:
case 0x3C9C:
case 0x3E9C:
case 0x309D:
case 0x329D:
case 0x349D:
case 0x369D:
case 0x389D:
case 0x3A9D:
case 0x3C9D:
case 0x3E9D:
case 0x309E:
case 0x329E:
case 0x349E:
case 0x369E:
case 0x389E:
case 0x3A9E:
case 0x3C9E:
case 0x3E9E:

// MOVEW
case 0x3098:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x32D8:
case 0x34D8:
case 0x36D8:
case 0x38D8:
case 0x3AD8:
case 0x3CD8:
case 0x30D9:
case 0x32D9:
case 0x34D9:
case 0x36D9:
case 0x38D9:
case 0x3AD9:
case 0x3CD9:
case 0x30DA:
case 0x32DA:
case 0x34DA:
case 0x36DA:
case 0x38DA:
case 0x3ADA:
case 0x3CDA:
case 0x30DB:
case 0x32DB:
case 0x34DB:
case 0x36DB:
case 0x38DB:
case 0x3ADB:
case 0x3CDB:
case 0x30DC:
case 0x32DC:
case 0x34DC:
case 0x36DC:
case 0x38DC:
case 0x3ADC:
case 0x3CDC:
case 0x30DD:
case 0x32DD:
case 0x34DD:
case 0x36DD:
case 0x38DD:
case 0x3ADD:
case 0x3CDD:
case 0x30DE:
case 0x32DE:
case 0x34DE:
case 0x36DE:
case 0x38DE:
case 0x3ADE:
case 0x3CDE:

// MOVEW
case 0x30D8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x3318:
case 0x3518:
case 0x3718:
case 0x3918:
case 0x3B18:
case 0x3D18:
case 0x3119:
case 0x3319:
case 0x3519:
case 0x3719:
case 0x3919:
case 0x3B19:
case 0x3D19:
case 0x311A:
case 0x331A:
case 0x351A:
case 0x371A:
case 0x391A:
case 0x3B1A:
case 0x3D1A:
case 0x311B:
case 0x331B:
case 0x351B:
case 0x371B:
case 0x391B:
case 0x3B1B:
case 0x3D1B:
case 0x311C:
case 0x331C:
case 0x351C:
case 0x371C:
case 0x391C:
case 0x3B1C:
case 0x3D1C:
case 0x311D:
case 0x331D:
case 0x351D:
case 0x371D:
case 0x391D:
case 0x3B1D:
case 0x3D1D:
case 0x311E:
case 0x331E:
case 0x351E:
case 0x371E:
case 0x391E:
case 0x3B1E:
case 0x3D1E:

// MOVEW
case 0x3118:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x3358:
case 0x3558:
case 0x3758:
case 0x3958:
case 0x3B58:
case 0x3D58:
case 0x3F58:
case 0x3159:
case 0x3359:
case 0x3559:
case 0x3759:
case 0x3959:
case 0x3B59:
case 0x3D59:
case 0x3F59:
case 0x315A:
case 0x335A:
case 0x355A:
case 0x375A:
case 0x395A:
case 0x3B5A:
case 0x3D5A:
case 0x3F5A:
case 0x315B:
case 0x335B:
case 0x355B:
case 0x375B:
case 0x395B:
case 0x3B5B:
case 0x3D5B:
case 0x3F5B:
case 0x315C:
case 0x335C:
case 0x355C:
case 0x375C:
case 0x395C:
case 0x3B5C:
case 0x3D5C:
case 0x3F5C:
case 0x315D:
case 0x335D:
case 0x355D:
case 0x375D:
case 0x395D:
case 0x3B5D:
case 0x3D5D:
case 0x3F5D:
case 0x315E:
case 0x335E:
case 0x355E:
case 0x375E:
case 0x395E:
case 0x3B5E:
case 0x3D5E:
case 0x3F5E:

// MOVEW
case 0x3158:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x3398:
case 0x3598:
case 0x3798:
case 0x3998:
case 0x3B98:
case 0x3D98:
case 0x3F98:
case 0x3199:
case 0x3399:
case 0x3599:
case 0x3799:
case 0x3999:
case 0x3B99:
case 0x3D99:
case 0x3F99:
case 0x319A:
case 0x339A:
case 0x359A:
case 0x379A:
case 0x399A:
case 0x3B9A:
case 0x3D9A:
case 0x3F9A:
case 0x319B:
case 0x339B:
case 0x359B:
case 0x379B:
case 0x399B:
case 0x3B9B:
case 0x3D9B:
case 0x3F9B:
case 0x319C:
case 0x339C:
case 0x359C:
case 0x379C:
case 0x399C:
case 0x3B9C:
case 0x3D9C:
case 0x3F9C:
case 0x319D:
case 0x339D:
case 0x359D:
case 0x379D:
case 0x399D:
case 0x3B9D:
case 0x3D9D:
case 0x3F9D:
case 0x319E:
case 0x339E:
case 0x359E:
case 0x379E:
case 0x399E:
case 0x3B9E:
case 0x3D9E:
case 0x3F9E:

// MOVEW
case 0x3198:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x31D9:
case 0x31DA:
case 0x31DB:
case 0x31DC:
case 0x31DD:
case 0x31DE:

// MOVEW
case 0x31D8:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x33D9:
case 0x33DA:
case 0x33DB:
case 0x33DC:
case 0x33DD:
case 0x33DE:

// MOVEW
case 0x33D8:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0x3ED9:
case 0x3EDA:
case 0x3EDB:
case 0x3EDC:
case 0x3EDD:
case 0x3EDE:

// MOVEW
case 0x3ED8:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x3F19:
case 0x3F1A:
case 0x3F1B:
case 0x3F1C:
case 0x3F1D:
case 0x3F1E:

// MOVEW
case 0x3F18:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x3220:
case 0x3420:
case 0x3620:
case 0x3820:
case 0x3A20:
case 0x3C20:
case 0x3E20:
case 0x3021:
case 0x3221:
case 0x3421:
case 0x3621:
case 0x3821:
case 0x3A21:
case 0x3C21:
case 0x3E21:
case 0x3022:
case 0x3222:
case 0x3422:
case 0x3622:
case 0x3822:
case 0x3A22:
case 0x3C22:
case 0x3E22:
case 0x3023:
case 0x3223:
case 0x3423:
case 0x3623:
case 0x3823:
case 0x3A23:
case 0x3C23:
case 0x3E23:
case 0x3024:
case 0x3224:
case 0x3424:
case 0x3624:
case 0x3824:
case 0x3A24:
case 0x3C24:
case 0x3E24:
case 0x3025:
case 0x3225:
case 0x3425:
case 0x3625:
case 0x3825:
case 0x3A25:
case 0x3C25:
case 0x3E25:
case 0x3026:
case 0x3226:
case 0x3426:
case 0x3626:
case 0x3826:
case 0x3A26:
case 0x3C26:
case 0x3E26:

// MOVEW
case 0x3020:
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
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0x32A0:
case 0x34A0:
case 0x36A0:
case 0x38A0:
case 0x3AA0:
case 0x3CA0:
case 0x3EA0:
case 0x30A1:
case 0x32A1:
case 0x34A1:
case 0x36A1:
case 0x38A1:
case 0x3AA1:
case 0x3CA1:
case 0x3EA1:
case 0x30A2:
case 0x32A2:
case 0x34A2:
case 0x36A2:
case 0x38A2:
case 0x3AA2:
case 0x3CA2:
case 0x3EA2:
case 0x30A3:
case 0x32A3:
case 0x34A3:
case 0x36A3:
case 0x38A3:
case 0x3AA3:
case 0x3CA3:
case 0x3EA3:
case 0x30A4:
case 0x32A4:
case 0x34A4:
case 0x36A4:
case 0x38A4:
case 0x3AA4:
case 0x3CA4:
case 0x3EA4:
case 0x30A5:
case 0x32A5:
case 0x34A5:
case 0x36A5:
case 0x38A5:
case 0x3AA5:
case 0x3CA5:
case 0x3EA5:
case 0x30A6:
case 0x32A6:
case 0x34A6:
case 0x36A6:
case 0x38A6:
case 0x3AA6:
case 0x3CA6:
case 0x3EA6:

// MOVEW
case 0x30A0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x32E0:
case 0x34E0:
case 0x36E0:
case 0x38E0:
case 0x3AE0:
case 0x3CE0:
case 0x30E1:
case 0x32E1:
case 0x34E1:
case 0x36E1:
case 0x38E1:
case 0x3AE1:
case 0x3CE1:
case 0x30E2:
case 0x32E2:
case 0x34E2:
case 0x36E2:
case 0x38E2:
case 0x3AE2:
case 0x3CE2:
case 0x30E3:
case 0x32E3:
case 0x34E3:
case 0x36E3:
case 0x38E3:
case 0x3AE3:
case 0x3CE3:
case 0x30E4:
case 0x32E4:
case 0x34E4:
case 0x36E4:
case 0x38E4:
case 0x3AE4:
case 0x3CE4:
case 0x30E5:
case 0x32E5:
case 0x34E5:
case 0x36E5:
case 0x38E5:
case 0x3AE5:
case 0x3CE5:
case 0x30E6:
case 0x32E6:
case 0x34E6:
case 0x36E6:
case 0x38E6:
case 0x3AE6:
case 0x3CE6:

// MOVEW
case 0x30E0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x3320:
case 0x3520:
case 0x3720:
case 0x3920:
case 0x3B20:
case 0x3D20:
case 0x3121:
case 0x3321:
case 0x3521:
case 0x3721:
case 0x3921:
case 0x3B21:
case 0x3D21:
case 0x3122:
case 0x3322:
case 0x3522:
case 0x3722:
case 0x3922:
case 0x3B22:
case 0x3D22:
case 0x3123:
case 0x3323:
case 0x3523:
case 0x3723:
case 0x3923:
case 0x3B23:
case 0x3D23:
case 0x3124:
case 0x3324:
case 0x3524:
case 0x3724:
case 0x3924:
case 0x3B24:
case 0x3D24:
case 0x3125:
case 0x3325:
case 0x3525:
case 0x3725:
case 0x3925:
case 0x3B25:
case 0x3D25:
case 0x3126:
case 0x3326:
case 0x3526:
case 0x3726:
case 0x3926:
case 0x3B26:
case 0x3D26:

// MOVEW
case 0x3120:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x3360:
case 0x3560:
case 0x3760:
case 0x3960:
case 0x3B60:
case 0x3D60:
case 0x3F60:
case 0x3161:
case 0x3361:
case 0x3561:
case 0x3761:
case 0x3961:
case 0x3B61:
case 0x3D61:
case 0x3F61:
case 0x3162:
case 0x3362:
case 0x3562:
case 0x3762:
case 0x3962:
case 0x3B62:
case 0x3D62:
case 0x3F62:
case 0x3163:
case 0x3363:
case 0x3563:
case 0x3763:
case 0x3963:
case 0x3B63:
case 0x3D63:
case 0x3F63:
case 0x3164:
case 0x3364:
case 0x3564:
case 0x3764:
case 0x3964:
case 0x3B64:
case 0x3D64:
case 0x3F64:
case 0x3165:
case 0x3365:
case 0x3565:
case 0x3765:
case 0x3965:
case 0x3B65:
case 0x3D65:
case 0x3F65:
case 0x3166:
case 0x3366:
case 0x3566:
case 0x3766:
case 0x3966:
case 0x3B66:
case 0x3D66:
case 0x3F66:

// MOVEW
case 0x3160:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x33A0:
case 0x35A0:
case 0x37A0:
case 0x39A0:
case 0x3BA0:
case 0x3DA0:
case 0x3FA0:
case 0x31A1:
case 0x33A1:
case 0x35A1:
case 0x37A1:
case 0x39A1:
case 0x3BA1:
case 0x3DA1:
case 0x3FA1:
case 0x31A2:
case 0x33A2:
case 0x35A2:
case 0x37A2:
case 0x39A2:
case 0x3BA2:
case 0x3DA2:
case 0x3FA2:
case 0x31A3:
case 0x33A3:
case 0x35A3:
case 0x37A3:
case 0x39A3:
case 0x3BA3:
case 0x3DA3:
case 0x3FA3:
case 0x31A4:
case 0x33A4:
case 0x35A4:
case 0x37A4:
case 0x39A4:
case 0x3BA4:
case 0x3DA4:
case 0x3FA4:
case 0x31A5:
case 0x33A5:
case 0x35A5:
case 0x37A5:
case 0x39A5:
case 0x3BA5:
case 0x3DA5:
case 0x3FA5:
case 0x31A6:
case 0x33A6:
case 0x35A6:
case 0x37A6:
case 0x39A6:
case 0x3BA6:
case 0x3DA6:
case 0x3FA6:

// MOVEW
case 0x31A0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0x31E1:
case 0x31E2:
case 0x31E3:
case 0x31E4:
case 0x31E5:
case 0x31E6:

// MOVEW
case 0x31E0:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x33E1:
case 0x33E2:
case 0x33E3:
case 0x33E4:
case 0x33E5:
case 0x33E6:

// MOVEW
case 0x33E0:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(22)
case 0x3EE1:
case 0x3EE2:
case 0x3EE3:
case 0x3EE4:
case 0x3EE5:
case 0x3EE6:

// MOVEW
case 0x3EE0:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x3F21:
case 0x3F22:
case 0x3F23:
case 0x3F24:
case 0x3F25:
case 0x3F26:

// MOVEW
case 0x3F20:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x3228:
case 0x3428:
case 0x3628:
case 0x3828:
case 0x3A28:
case 0x3C28:
case 0x3E28:
case 0x3029:
case 0x3229:
case 0x3429:
case 0x3629:
case 0x3829:
case 0x3A29:
case 0x3C29:
case 0x3E29:
case 0x302A:
case 0x322A:
case 0x342A:
case 0x362A:
case 0x382A:
case 0x3A2A:
case 0x3C2A:
case 0x3E2A:
case 0x302B:
case 0x322B:
case 0x342B:
case 0x362B:
case 0x382B:
case 0x3A2B:
case 0x3C2B:
case 0x3E2B:
case 0x302C:
case 0x322C:
case 0x342C:
case 0x362C:
case 0x382C:
case 0x3A2C:
case 0x3C2C:
case 0x3E2C:
case 0x302D:
case 0x322D:
case 0x342D:
case 0x362D:
case 0x382D:
case 0x3A2D:
case 0x3C2D:
case 0x3E2D:
case 0x302E:
case 0x322E:
case 0x342E:
case 0x362E:
case 0x382E:
case 0x3A2E:
case 0x3C2E:
case 0x3E2E:
case 0x302F:
case 0x322F:
case 0x342F:
case 0x362F:
case 0x382F:
case 0x3A2F:
case 0x3C2F:
case 0x3E2F:

// MOVEW
case 0x3028:
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
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x32A8:
case 0x34A8:
case 0x36A8:
case 0x38A8:
case 0x3AA8:
case 0x3CA8:
case 0x3EA8:
case 0x30A9:
case 0x32A9:
case 0x34A9:
case 0x36A9:
case 0x38A9:
case 0x3AA9:
case 0x3CA9:
case 0x3EA9:
case 0x30AA:
case 0x32AA:
case 0x34AA:
case 0x36AA:
case 0x38AA:
case 0x3AAA:
case 0x3CAA:
case 0x3EAA:
case 0x30AB:
case 0x32AB:
case 0x34AB:
case 0x36AB:
case 0x38AB:
case 0x3AAB:
case 0x3CAB:
case 0x3EAB:
case 0x30AC:
case 0x32AC:
case 0x34AC:
case 0x36AC:
case 0x38AC:
case 0x3AAC:
case 0x3CAC:
case 0x3EAC:
case 0x30AD:
case 0x32AD:
case 0x34AD:
case 0x36AD:
case 0x38AD:
case 0x3AAD:
case 0x3CAD:
case 0x3EAD:
case 0x30AE:
case 0x32AE:
case 0x34AE:
case 0x36AE:
case 0x38AE:
case 0x3AAE:
case 0x3CAE:
case 0x3EAE:
case 0x30AF:
case 0x32AF:
case 0x34AF:
case 0x36AF:
case 0x38AF:
case 0x3AAF:
case 0x3CAF:
case 0x3EAF:

// MOVEW
case 0x30A8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x32E8:
case 0x34E8:
case 0x36E8:
case 0x38E8:
case 0x3AE8:
case 0x3CE8:
case 0x30E9:
case 0x32E9:
case 0x34E9:
case 0x36E9:
case 0x38E9:
case 0x3AE9:
case 0x3CE9:
case 0x30EA:
case 0x32EA:
case 0x34EA:
case 0x36EA:
case 0x38EA:
case 0x3AEA:
case 0x3CEA:
case 0x30EB:
case 0x32EB:
case 0x34EB:
case 0x36EB:
case 0x38EB:
case 0x3AEB:
case 0x3CEB:
case 0x30EC:
case 0x32EC:
case 0x34EC:
case 0x36EC:
case 0x38EC:
case 0x3AEC:
case 0x3CEC:
case 0x30ED:
case 0x32ED:
case 0x34ED:
case 0x36ED:
case 0x38ED:
case 0x3AED:
case 0x3CED:
case 0x30EE:
case 0x32EE:
case 0x34EE:
case 0x36EE:
case 0x38EE:
case 0x3AEE:
case 0x3CEE:
case 0x30EF:
case 0x32EF:
case 0x34EF:
case 0x36EF:
case 0x38EF:
case 0x3AEF:
case 0x3CEF:

// MOVEW
case 0x30E8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x3328:
case 0x3528:
case 0x3728:
case 0x3928:
case 0x3B28:
case 0x3D28:
case 0x3129:
case 0x3329:
case 0x3529:
case 0x3729:
case 0x3929:
case 0x3B29:
case 0x3D29:
case 0x312A:
case 0x332A:
case 0x352A:
case 0x372A:
case 0x392A:
case 0x3B2A:
case 0x3D2A:
case 0x312B:
case 0x332B:
case 0x352B:
case 0x372B:
case 0x392B:
case 0x3B2B:
case 0x3D2B:
case 0x312C:
case 0x332C:
case 0x352C:
case 0x372C:
case 0x392C:
case 0x3B2C:
case 0x3D2C:
case 0x312D:
case 0x332D:
case 0x352D:
case 0x372D:
case 0x392D:
case 0x3B2D:
case 0x3D2D:
case 0x312E:
case 0x332E:
case 0x352E:
case 0x372E:
case 0x392E:
case 0x3B2E:
case 0x3D2E:
case 0x312F:
case 0x332F:
case 0x352F:
case 0x372F:
case 0x392F:
case 0x3B2F:
case 0x3D2F:

// MOVEW
case 0x3128:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x3368:
case 0x3568:
case 0x3768:
case 0x3968:
case 0x3B68:
case 0x3D68:
case 0x3F68:
case 0x3169:
case 0x3369:
case 0x3569:
case 0x3769:
case 0x3969:
case 0x3B69:
case 0x3D69:
case 0x3F69:
case 0x316A:
case 0x336A:
case 0x356A:
case 0x376A:
case 0x396A:
case 0x3B6A:
case 0x3D6A:
case 0x3F6A:
case 0x316B:
case 0x336B:
case 0x356B:
case 0x376B:
case 0x396B:
case 0x3B6B:
case 0x3D6B:
case 0x3F6B:
case 0x316C:
case 0x336C:
case 0x356C:
case 0x376C:
case 0x396C:
case 0x3B6C:
case 0x3D6C:
case 0x3F6C:
case 0x316D:
case 0x336D:
case 0x356D:
case 0x376D:
case 0x396D:
case 0x3B6D:
case 0x3D6D:
case 0x3F6D:
case 0x316E:
case 0x336E:
case 0x356E:
case 0x376E:
case 0x396E:
case 0x3B6E:
case 0x3D6E:
case 0x3F6E:
case 0x316F:
case 0x336F:
case 0x356F:
case 0x376F:
case 0x396F:
case 0x3B6F:
case 0x3D6F:
case 0x3F6F:

// MOVEW
case 0x3168:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0x33A8:
case 0x35A8:
case 0x37A8:
case 0x39A8:
case 0x3BA8:
case 0x3DA8:
case 0x3FA8:
case 0x31A9:
case 0x33A9:
case 0x35A9:
case 0x37A9:
case 0x39A9:
case 0x3BA9:
case 0x3DA9:
case 0x3FA9:
case 0x31AA:
case 0x33AA:
case 0x35AA:
case 0x37AA:
case 0x39AA:
case 0x3BAA:
case 0x3DAA:
case 0x3FAA:
case 0x31AB:
case 0x33AB:
case 0x35AB:
case 0x37AB:
case 0x39AB:
case 0x3BAB:
case 0x3DAB:
case 0x3FAB:
case 0x31AC:
case 0x33AC:
case 0x35AC:
case 0x37AC:
case 0x39AC:
case 0x3BAC:
case 0x3DAC:
case 0x3FAC:
case 0x31AD:
case 0x33AD:
case 0x35AD:
case 0x37AD:
case 0x39AD:
case 0x3BAD:
case 0x3DAD:
case 0x3FAD:
case 0x31AE:
case 0x33AE:
case 0x35AE:
case 0x37AE:
case 0x39AE:
case 0x3BAE:
case 0x3DAE:
case 0x3FAE:
case 0x31AF:
case 0x33AF:
case 0x35AF:
case 0x37AF:
case 0x39AF:
case 0x3BAF:
case 0x3DAF:
case 0x3FAF:

// MOVEW
case 0x31A8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(22)
case 0x31E9:
case 0x31EA:
case 0x31EB:
case 0x31EC:
case 0x31ED:
case 0x31EE:
case 0x31EF:

// MOVEW
case 0x31E8:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0x33E9:
case 0x33EA:
case 0x33EB:
case 0x33EC:
case 0x33ED:
case 0x33EE:
case 0x33EF:

// MOVEW
case 0x33E8:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(24)
case 0x3EE9:
case 0x3EEA:
case 0x3EEB:
case 0x3EEC:
case 0x3EED:
case 0x3EEE:
case 0x3EEF:

// MOVEW
case 0x3EE8:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x3F29:
case 0x3F2A:
case 0x3F2B:
case 0x3F2C:
case 0x3F2D:
case 0x3F2E:
case 0x3F2F:

// MOVEW
case 0x3F28:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x3230:
case 0x3430:
case 0x3630:
case 0x3830:
case 0x3A30:
case 0x3C30:
case 0x3E30:
case 0x3031:
case 0x3231:
case 0x3431:
case 0x3631:
case 0x3831:
case 0x3A31:
case 0x3C31:
case 0x3E31:
case 0x3032:
case 0x3232:
case 0x3432:
case 0x3632:
case 0x3832:
case 0x3A32:
case 0x3C32:
case 0x3E32:
case 0x3033:
case 0x3233:
case 0x3433:
case 0x3633:
case 0x3833:
case 0x3A33:
case 0x3C33:
case 0x3E33:
case 0x3034:
case 0x3234:
case 0x3434:
case 0x3634:
case 0x3834:
case 0x3A34:
case 0x3C34:
case 0x3E34:
case 0x3035:
case 0x3235:
case 0x3435:
case 0x3635:
case 0x3835:
case 0x3A35:
case 0x3C35:
case 0x3E35:
case 0x3036:
case 0x3236:
case 0x3436:
case 0x3636:
case 0x3836:
case 0x3A36:
case 0x3C36:
case 0x3E36:
case 0x3037:
case 0x3237:
case 0x3437:
case 0x3637:
case 0x3837:
case 0x3A37:
case 0x3C37:
case 0x3E37:

// MOVEW
case 0x3030:
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
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x32B0:
case 0x34B0:
case 0x36B0:
case 0x38B0:
case 0x3AB0:
case 0x3CB0:
case 0x3EB0:
case 0x30B1:
case 0x32B1:
case 0x34B1:
case 0x36B1:
case 0x38B1:
case 0x3AB1:
case 0x3CB1:
case 0x3EB1:
case 0x30B2:
case 0x32B2:
case 0x34B2:
case 0x36B2:
case 0x38B2:
case 0x3AB2:
case 0x3CB2:
case 0x3EB2:
case 0x30B3:
case 0x32B3:
case 0x34B3:
case 0x36B3:
case 0x38B3:
case 0x3AB3:
case 0x3CB3:
case 0x3EB3:
case 0x30B4:
case 0x32B4:
case 0x34B4:
case 0x36B4:
case 0x38B4:
case 0x3AB4:
case 0x3CB4:
case 0x3EB4:
case 0x30B5:
case 0x32B5:
case 0x34B5:
case 0x36B5:
case 0x38B5:
case 0x3AB5:
case 0x3CB5:
case 0x3EB5:
case 0x30B6:
case 0x32B6:
case 0x34B6:
case 0x36B6:
case 0x38B6:
case 0x3AB6:
case 0x3CB6:
case 0x3EB6:
case 0x30B7:
case 0x32B7:
case 0x34B7:
case 0x36B7:
case 0x38B7:
case 0x3AB7:
case 0x3CB7:
case 0x3EB7:

// MOVEW
case 0x30B0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x32F0:
case 0x34F0:
case 0x36F0:
case 0x38F0:
case 0x3AF0:
case 0x3CF0:
case 0x30F1:
case 0x32F1:
case 0x34F1:
case 0x36F1:
case 0x38F1:
case 0x3AF1:
case 0x3CF1:
case 0x30F2:
case 0x32F2:
case 0x34F2:
case 0x36F2:
case 0x38F2:
case 0x3AF2:
case 0x3CF2:
case 0x30F3:
case 0x32F3:
case 0x34F3:
case 0x36F3:
case 0x38F3:
case 0x3AF3:
case 0x3CF3:
case 0x30F4:
case 0x32F4:
case 0x34F4:
case 0x36F4:
case 0x38F4:
case 0x3AF4:
case 0x3CF4:
case 0x30F5:
case 0x32F5:
case 0x34F5:
case 0x36F5:
case 0x38F5:
case 0x3AF5:
case 0x3CF5:
case 0x30F6:
case 0x32F6:
case 0x34F6:
case 0x36F6:
case 0x38F6:
case 0x3AF6:
case 0x3CF6:
case 0x30F7:
case 0x32F7:
case 0x34F7:
case 0x36F7:
case 0x38F7:
case 0x3AF7:
case 0x3CF7:

// MOVEW
case 0x30F0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x3330:
case 0x3530:
case 0x3730:
case 0x3930:
case 0x3B30:
case 0x3D30:
case 0x3131:
case 0x3331:
case 0x3531:
case 0x3731:
case 0x3931:
case 0x3B31:
case 0x3D31:
case 0x3132:
case 0x3332:
case 0x3532:
case 0x3732:
case 0x3932:
case 0x3B32:
case 0x3D32:
case 0x3133:
case 0x3333:
case 0x3533:
case 0x3733:
case 0x3933:
case 0x3B33:
case 0x3D33:
case 0x3134:
case 0x3334:
case 0x3534:
case 0x3734:
case 0x3934:
case 0x3B34:
case 0x3D34:
case 0x3135:
case 0x3335:
case 0x3535:
case 0x3735:
case 0x3935:
case 0x3B35:
case 0x3D35:
case 0x3136:
case 0x3336:
case 0x3536:
case 0x3736:
case 0x3936:
case 0x3B36:
case 0x3D36:
case 0x3137:
case 0x3337:
case 0x3537:
case 0x3737:
case 0x3937:
case 0x3B37:
case 0x3D37:

// MOVEW
case 0x3130:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x3370:
case 0x3570:
case 0x3770:
case 0x3970:
case 0x3B70:
case 0x3D70:
case 0x3F70:
case 0x3171:
case 0x3371:
case 0x3571:
case 0x3771:
case 0x3971:
case 0x3B71:
case 0x3D71:
case 0x3F71:
case 0x3172:
case 0x3372:
case 0x3572:
case 0x3772:
case 0x3972:
case 0x3B72:
case 0x3D72:
case 0x3F72:
case 0x3173:
case 0x3373:
case 0x3573:
case 0x3773:
case 0x3973:
case 0x3B73:
case 0x3D73:
case 0x3F73:
case 0x3174:
case 0x3374:
case 0x3574:
case 0x3774:
case 0x3974:
case 0x3B74:
case 0x3D74:
case 0x3F74:
case 0x3175:
case 0x3375:
case 0x3575:
case 0x3775:
case 0x3975:
case 0x3B75:
case 0x3D75:
case 0x3F75:
case 0x3176:
case 0x3376:
case 0x3576:
case 0x3776:
case 0x3976:
case 0x3B76:
case 0x3D76:
case 0x3F76:
case 0x3177:
case 0x3377:
case 0x3577:
case 0x3777:
case 0x3977:
case 0x3B77:
case 0x3D77:
case 0x3F77:

// MOVEW
case 0x3170:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(22)
case 0x33B0:
case 0x35B0:
case 0x37B0:
case 0x39B0:
case 0x3BB0:
case 0x3DB0:
case 0x3FB0:
case 0x31B1:
case 0x33B1:
case 0x35B1:
case 0x37B1:
case 0x39B1:
case 0x3BB1:
case 0x3DB1:
case 0x3FB1:
case 0x31B2:
case 0x33B2:
case 0x35B2:
case 0x37B2:
case 0x39B2:
case 0x3BB2:
case 0x3DB2:
case 0x3FB2:
case 0x31B3:
case 0x33B3:
case 0x35B3:
case 0x37B3:
case 0x39B3:
case 0x3BB3:
case 0x3DB3:
case 0x3FB3:
case 0x31B4:
case 0x33B4:
case 0x35B4:
case 0x37B4:
case 0x39B4:
case 0x3BB4:
case 0x3DB4:
case 0x3FB4:
case 0x31B5:
case 0x33B5:
case 0x35B5:
case 0x37B5:
case 0x39B5:
case 0x3BB5:
case 0x3DB5:
case 0x3FB5:
case 0x31B6:
case 0x33B6:
case 0x35B6:
case 0x37B6:
case 0x39B6:
case 0x3BB6:
case 0x3DB6:
case 0x3FB6:
case 0x31B7:
case 0x33B7:
case 0x35B7:
case 0x37B7:
case 0x39B7:
case 0x3BB7:
case 0x3DB7:
case 0x3FB7:

// MOVEW
case 0x31B0:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(24)
case 0x31F1:
case 0x31F2:
case 0x31F3:
case 0x31F4:
case 0x31F5:
case 0x31F6:
case 0x31F7:

// MOVEW
case 0x31F0:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(22)
case 0x33F1:
case 0x33F2:
case 0x33F3:
case 0x33F4:
case 0x33F5:
case 0x33F6:
case 0x33F7:

// MOVEW
case 0x33F0:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(26)
case 0x3EF1:
case 0x3EF2:
case 0x3EF3:
case 0x3EF4:
case 0x3EF5:
case 0x3EF6:
case 0x3EF7:

// MOVEW
case 0x3EF0:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x3F31:
case 0x3F32:
case 0x3F33:
case 0x3F34:
case 0x3F35:
case 0x3F36:
case 0x3F37:

// MOVEW
case 0x3F30:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x3238:
case 0x3438:
case 0x3638:
case 0x3838:
case 0x3A38:
case 0x3C38:
case 0x3E38:

// MOVEW
case 0x3038:
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
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x32B8:
case 0x34B8:
case 0x36B8:
case 0x38B8:
case 0x3AB8:
case 0x3CB8:
case 0x3EB8:

// MOVEW
case 0x30B8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x32F8:
case 0x34F8:
case 0x36F8:
case 0x38F8:
case 0x3AF8:
case 0x3CF8:

// MOVEW
case 0x30F8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x3338:
case 0x3538:
case 0x3738:
case 0x3938:
case 0x3B38:
case 0x3D38:

// MOVEW
case 0x3138:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x3378:
case 0x3578:
case 0x3778:
case 0x3978:
case 0x3B78:
case 0x3D78:
case 0x3F78:

// MOVEW
case 0x3178:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0x33B8:
case 0x35B8:
case 0x37B8:
case 0x39B8:
case 0x3BB8:
case 0x3DB8:
case 0x3FB8:

// MOVEW
case 0x31B8:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(22)

// MOVEW
case 0x31F8:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// MOVEW
case 0x33F8:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(24)

// MOVEW
case 0x3EF8:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// MOVEW
case 0x3F38:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x3239:
case 0x3439:
case 0x3639:
case 0x3839:
case 0x3A39:
case 0x3C39:
case 0x3E39:

// MOVEW
case 0x3039:
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
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(16)
case 0x32B9:
case 0x34B9:
case 0x36B9:
case 0x38B9:
case 0x3AB9:
case 0x3CB9:
case 0x3EB9:

// MOVEW
case 0x30B9:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0x32F9:
case 0x34F9:
case 0x36F9:
case 0x38F9:
case 0x3AF9:
case 0x3CF9:

// MOVEW
case 0x30F9:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0x3339:
case 0x3539:
case 0x3739:
case 0x3939:
case 0x3B39:
case 0x3D39:

// MOVEW
case 0x3139:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0x3379:
case 0x3579:
case 0x3779:
case 0x3979:
case 0x3B79:
case 0x3D79:
case 0x3F79:

// MOVEW
case 0x3179:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(24)
case 0x33B9:
case 0x35B9:
case 0x37B9:
case 0x39B9:
case 0x3BB9:
case 0x3DB9:
case 0x3FB9:

// MOVEW
case 0x31B9:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(26)

// MOVEW
case 0x31F9:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(24)

// MOVEW
case 0x33F9:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(28)

// MOVEW
case 0x3EF9:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// MOVEW
case 0x3F39:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0x323A:
case 0x343A:
case 0x363A:
case 0x383A:
case 0x3A3A:
case 0x3C3A:
case 0x3E3A:

// MOVEW
case 0x303A:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(12)
case 0x32BA:
case 0x34BA:
case 0x36BA:
case 0x38BA:
case 0x3ABA:
case 0x3CBA:
case 0x3EBA:

// MOVEW
case 0x30BA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x32FA:
case 0x34FA:
case 0x36FA:
case 0x38FA:
case 0x3AFA:
case 0x3CFA:

// MOVEW
case 0x30FA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x333A:
case 0x353A:
case 0x373A:
case 0x393A:
case 0x3B3A:
case 0x3D3A:

// MOVEW
case 0x313A:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x337A:
case 0x357A:
case 0x377A:
case 0x397A:
case 0x3B7A:
case 0x3D7A:
case 0x3F7A:

// MOVEW
case 0x317A:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)
case 0x33BA:
case 0x35BA:
case 0x37BA:
case 0x39BA:
case 0x3BBA:
case 0x3DBA:
case 0x3FBA:

// MOVEW
case 0x31BA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(22)

// MOVEW
case 0x31FA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// MOVEW
case 0x33FA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(24)

// MOVEW
case 0x3EFA:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// MOVEW
case 0x3F3A:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x323B:
case 0x343B:
case 0x363B:
case 0x383B:
case 0x3A3B:
case 0x3C3B:
case 0x3E3B:

// MOVEW
case 0x303B:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(14)
case 0x32BB:
case 0x34BB:
case 0x36BB:
case 0x38BB:
case 0x3ABB:
case 0x3CBB:
case 0x3EBB:

// MOVEW
case 0x30BB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x32FB:
case 0x34FB:
case 0x36FB:
case 0x38FB:
case 0x3AFB:
case 0x3CFB:

// MOVEW
case 0x30FB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x333B:
case 0x353B:
case 0x373B:
case 0x393B:
case 0x3B3B:
case 0x3D3B:

// MOVEW
case 0x313B:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x337B:
case 0x357B:
case 0x377B:
case 0x397B:
case 0x3B7B:
case 0x3D7B:
case 0x3F7B:

// MOVEW
case 0x317B:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(22)
case 0x33BB:
case 0x35BB:
case 0x37BB:
case 0x39BB:
case 0x3BBB:
case 0x3DBB:
case 0x3FBB:

// MOVEW
case 0x31BB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(24)

// MOVEW
case 0x31FB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(22)

// MOVEW
case 0x33FB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(26)

// MOVEW
case 0x3EFB:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// MOVEW
case 0x3F3B:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READ_WORD_F(adr, res)
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x323C:
case 0x343C:
case 0x363C:
case 0x383C:
case 0x3A3C:
case 0x3C3C:
case 0x3E3C:

// MOVEW
case 0x303C:
{
	u32 res;
	res = FETCH_WORD;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
}
RET(8)
case 0x32BC:
case 0x34BC:
case 0x36BC:
case 0x38BC:
case 0x3ABC:
case 0x3CBC:
case 0x3EBC:

// MOVEW
case 0x30BC:
{
	u32 adr;
	u32 res;
	res = FETCH_WORD;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x32FC:
case 0x34FC:
case 0x36FC:
case 0x38FC:
case 0x3AFC:
case 0x3CFC:

// MOVEW
case 0x30FC:
{
	u32 adr;
	u32 res;
	res = FETCH_WORD;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x333C:
case 0x353C:
case 0x373C:
case 0x393C:
case 0x3B3C:
case 0x3D3C:

// MOVEW
case 0x313C:
{
	u32 adr;
	u32 res;
	res = FETCH_WORD;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x337C:
case 0x357C:
case 0x377C:
case 0x397C:
case 0x3B7C:
case 0x3D7C:
case 0x3F7C:

// MOVEW
case 0x317C:
{
	u32 adr;
	u32 res;
	res = FETCH_WORD;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x33BC:
case 0x35BC:
case 0x37BC:
case 0x39BC:
case 0x3BBC:
case 0x3DBC:
case 0x3FBC:

// MOVEW
case 0x31BC:
{
	u32 adr;
	u32 res;
	res = FETCH_WORD;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// MOVEW
case 0x31FC:
{
	u32 adr;
	u32 res;
	res = FETCH_WORD;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// MOVEW
case 0x33FC:
{
	u32 adr;
	u32 res;
	res = FETCH_WORD;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// MOVEW
case 0x3EFC:
{
	u32 adr;
	u32 res;
	res = FETCH_WORD;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// MOVEW
case 0x3F3C:
{
	u32 adr;
	u32 res;
	res = FETCH_WORD;
	PC += 2;
	CPU->flag_C = 0;
	CPU->flag_V = 0;
	CPU->flag_notZ = res;
	CPU->flag_N = res >> 8;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x321F:
case 0x341F:
case 0x361F:
case 0x381F:
case 0x3A1F:
case 0x3C1F:
case 0x3E1F:

// MOVEW
case 0x301F:
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
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(8)
case 0x329F:
case 0x349F:
case 0x369F:
case 0x389F:
case 0x3A9F:
case 0x3C9F:
case 0x3E9F:

// MOVEW
case 0x309F:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x32DF:
case 0x34DF:
case 0x36DF:
case 0x38DF:
case 0x3ADF:
case 0x3CDF:

// MOVEW
case 0x30DF:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x331F:
case 0x351F:
case 0x371F:
case 0x391F:
case 0x3B1F:
case 0x3D1F:

// MOVEW
case 0x311F:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x335F:
case 0x355F:
case 0x375F:
case 0x395F:
case 0x3B5F:
case 0x3D5F:
case 0x3F5F:

// MOVEW
case 0x315F:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)
case 0x339F:
case 0x359F:
case 0x379F:
case 0x399F:
case 0x3B9F:
case 0x3D9F:
case 0x3F9F:

// MOVEW
case 0x319F:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// MOVEW
case 0x31DF:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(16)

// MOVEW
case 0x33DF:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// MOVEW
case 0x3EDF:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)

// MOVEW
case 0x3F1F:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(12)
case 0x3227:
case 0x3427:
case 0x3627:
case 0x3827:
case 0x3A27:
case 0x3C27:
case 0x3E27:

// MOVEW
case 0x3027:
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
	*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> 9) & 7])) = res;
	POST_IO
}
RET(10)
case 0x32A7:
case 0x34A7:
case 0x36A7:
case 0x38A7:
case 0x3AA7:
case 0x3CA7:
case 0x3EA7:

// MOVEW
case 0x30A7:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x32E7:
case 0x34E7:
case 0x36E7:
case 0x38E7:
case 0x3AE7:
case 0x3CE7:

// MOVEW
case 0x30E7:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	CPU->A[(Opcode >> 9) & 7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x3327:
case 0x3527:
case 0x3727:
case 0x3927:
case 0x3B27:
case 0x3D27:

// MOVEW
case 0x3127:
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
	adr = CPU->A[(Opcode >> 9) & 7] - 2;
	CPU->A[(Opcode >> 9) & 7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x3367:
case 0x3567:
case 0x3767:
case 0x3967:
case 0x3B67:
case 0x3D67:
case 0x3F67:

// MOVEW
case 0x3167:
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
	adr = CPU->A[(Opcode >> 9) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)
case 0x33A7:
case 0x35A7:
case 0x37A7:
case 0x39A7:
case 0x3BA7:
case 0x3DA7:
case 0x3FA7:

// MOVEW
case 0x31A7:
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
	adr = CPU->A[(Opcode >> 9) & 7];
	DECODE_EXT_WORD
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(20)

// MOVEW
case 0x31E7:
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
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(18)

// MOVEW
case 0x33E7:
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
	adr = (s32)FETCH_LONG;
	PC += 4;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(22)

// MOVEW
case 0x3EE7:
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
	adr = CPU->A[7];
	CPU->A[7] += 2;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)

// MOVEW
case 0x3F27:
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
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	WRITE_WORD_F(adr, res)
	POST_IO
}
RET(14)
case 0x3240:
case 0x3440:
case 0x3640:
case 0x3840:
case 0x3A40:
case 0x3C40:
case 0x3E40:
case 0x3041:
case 0x3241:
case 0x3441:
case 0x3641:
case 0x3841:
case 0x3A41:
case 0x3C41:
case 0x3E41:
case 0x3042:
case 0x3242:
case 0x3442:
case 0x3642:
case 0x3842:
case 0x3A42:
case 0x3C42:
case 0x3E42:
case 0x3043:
case 0x3243:
case 0x3443:
case 0x3643:
case 0x3843:
case 0x3A43:
case 0x3C43:
case 0x3E43:
case 0x3044:
case 0x3244:
case 0x3444:
case 0x3644:
case 0x3844:
case 0x3A44:
case 0x3C44:
case 0x3E44:
case 0x3045:
case 0x3245:
case 0x3445:
case 0x3645:
case 0x3845:
case 0x3A45:
case 0x3C45:
case 0x3E45:
case 0x3046:
case 0x3246:
case 0x3446:
case 0x3646:
case 0x3846:
case 0x3A46:
case 0x3C46:
case 0x3E46:
case 0x3047:
case 0x3247:
case 0x3447:
case 0x3647:
case 0x3847:
case 0x3A47:
case 0x3C47:
case 0x3E47:

// MOVEAW
case 0x3040:
{
	u32 res;
	res = (s32)(s16)CPU->D[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(4)
case 0x3248:
case 0x3448:
case 0x3648:
case 0x3848:
case 0x3A48:
case 0x3C48:
case 0x3E48:
case 0x3049:
case 0x3249:
case 0x3449:
case 0x3649:
case 0x3849:
case 0x3A49:
case 0x3C49:
case 0x3E49:
case 0x304A:
case 0x324A:
case 0x344A:
case 0x364A:
case 0x384A:
case 0x3A4A:
case 0x3C4A:
case 0x3E4A:
case 0x304B:
case 0x324B:
case 0x344B:
case 0x364B:
case 0x384B:
case 0x3A4B:
case 0x3C4B:
case 0x3E4B:
case 0x304C:
case 0x324C:
case 0x344C:
case 0x364C:
case 0x384C:
case 0x3A4C:
case 0x3C4C:
case 0x3E4C:
case 0x304D:
case 0x324D:
case 0x344D:
case 0x364D:
case 0x384D:
case 0x3A4D:
case 0x3C4D:
case 0x3E4D:
case 0x304E:
case 0x324E:
case 0x344E:
case 0x364E:
case 0x384E:
case 0x3A4E:
case 0x3C4E:
case 0x3E4E:
case 0x304F:
case 0x324F:
case 0x344F:
case 0x364F:
case 0x384F:
case 0x3A4F:
case 0x3C4F:
case 0x3E4F:

// MOVEAW
case 0x3048:
{
	u32 res;
	res = (s32)(s16)CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(4)
case 0x3250:
case 0x3450:
case 0x3650:
case 0x3850:
case 0x3A50:
case 0x3C50:
case 0x3E50:
case 0x3051:
case 0x3251:
case 0x3451:
case 0x3651:
case 0x3851:
case 0x3A51:
case 0x3C51:
case 0x3E51:
case 0x3052:
case 0x3252:
case 0x3452:
case 0x3652:
case 0x3852:
case 0x3A52:
case 0x3C52:
case 0x3E52:
case 0x3053:
case 0x3253:
case 0x3453:
case 0x3653:
case 0x3853:
case 0x3A53:
case 0x3C53:
case 0x3E53:
case 0x3054:
case 0x3254:
case 0x3454:
case 0x3654:
case 0x3854:
case 0x3A54:
case 0x3C54:
case 0x3E54:
case 0x3055:
case 0x3255:
case 0x3455:
case 0x3655:
case 0x3855:
case 0x3A55:
case 0x3C55:
case 0x3E55:
case 0x3056:
case 0x3256:
case 0x3456:
case 0x3656:
case 0x3856:
case 0x3A56:
case 0x3C56:
case 0x3E56:
case 0x3057:
case 0x3257:
case 0x3457:
case 0x3657:
case 0x3857:
case 0x3A57:
case 0x3C57:
case 0x3E57:

// MOVEAW
case 0x3050:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	PRE_IO
	READSX_WORD_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(8)
case 0x3258:
case 0x3458:
case 0x3658:
case 0x3858:
case 0x3A58:
case 0x3C58:
case 0x3E58:
case 0x3059:
case 0x3259:
case 0x3459:
case 0x3659:
case 0x3859:
case 0x3A59:
case 0x3C59:
case 0x3E59:
case 0x305A:
case 0x325A:
case 0x345A:
case 0x365A:
case 0x385A:
case 0x3A5A:
case 0x3C5A:
case 0x3E5A:
case 0x305B:
case 0x325B:
case 0x345B:
case 0x365B:
case 0x385B:
case 0x3A5B:
case 0x3C5B:
case 0x3E5B:
case 0x305C:
case 0x325C:
case 0x345C:
case 0x365C:
case 0x385C:
case 0x3A5C:
case 0x3C5C:
case 0x3E5C:
case 0x305D:
case 0x325D:
case 0x345D:
case 0x365D:
case 0x385D:
case 0x3A5D:
case 0x3C5D:
case 0x3E5D:
case 0x305E:
case 0x325E:
case 0x345E:
case 0x365E:
case 0x385E:
case 0x3A5E:
case 0x3C5E:
case 0x3E5E:

// MOVEAW
case 0x3058:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	CPU->A[(Opcode >> 0) & 7] += 2;
	PRE_IO
	READSX_WORD_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(8)
case 0x3260:
case 0x3460:
case 0x3660:
case 0x3860:
case 0x3A60:
case 0x3C60:
case 0x3E60:
case 0x3061:
case 0x3261:
case 0x3461:
case 0x3661:
case 0x3861:
case 0x3A61:
case 0x3C61:
case 0x3E61:
case 0x3062:
case 0x3262:
case 0x3462:
case 0x3662:
case 0x3862:
case 0x3A62:
case 0x3C62:
case 0x3E62:
case 0x3063:
case 0x3263:
case 0x3463:
case 0x3663:
case 0x3863:
case 0x3A63:
case 0x3C63:
case 0x3E63:
case 0x3064:
case 0x3264:
case 0x3464:
case 0x3664:
case 0x3864:
case 0x3A64:
case 0x3C64:
case 0x3E64:
case 0x3065:
case 0x3265:
case 0x3465:
case 0x3665:
case 0x3865:
case 0x3A65:
case 0x3C65:
case 0x3E65:
case 0x3066:
case 0x3266:
case 0x3466:
case 0x3666:
case 0x3866:
case 0x3A66:
case 0x3C66:
case 0x3E66:

// MOVEAW
case 0x3060:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] - 2;
	CPU->A[(Opcode >> 0) & 7] = adr;
	PRE_IO
	READSX_WORD_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(10)
case 0x3268:
case 0x3468:
case 0x3668:
case 0x3868:
case 0x3A68:
case 0x3C68:
case 0x3E68:
case 0x3069:
case 0x3269:
case 0x3469:
case 0x3669:
case 0x3869:
case 0x3A69:
case 0x3C69:
case 0x3E69:
case 0x306A:
case 0x326A:
case 0x346A:
case 0x366A:
case 0x386A:
case 0x3A6A:
case 0x3C6A:
case 0x3E6A:
case 0x306B:
case 0x326B:
case 0x346B:
case 0x366B:
case 0x386B:
case 0x3A6B:
case 0x3C6B:
case 0x3E6B:
case 0x306C:
case 0x326C:
case 0x346C:
case 0x366C:
case 0x386C:
case 0x3A6C:
case 0x3C6C:
case 0x3E6C:
case 0x306D:
case 0x326D:
case 0x346D:
case 0x366D:
case 0x386D:
case 0x3A6D:
case 0x3C6D:
case 0x3E6D:
case 0x306E:
case 0x326E:
case 0x346E:
case 0x366E:
case 0x386E:
case 0x3A6E:
case 0x3C6E:
case 0x3E6E:
case 0x306F:
case 0x326F:
case 0x346F:
case 0x366F:
case 0x386F:
case 0x3A6F:
case 0x3C6F:
case 0x3E6F:

// MOVEAW
case 0x3068:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7] + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_WORD_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(12)
case 0x3270:
case 0x3470:
case 0x3670:
case 0x3870:
case 0x3A70:
case 0x3C70:
case 0x3E70:
case 0x3071:
case 0x3271:
case 0x3471:
case 0x3671:
case 0x3871:
case 0x3A71:
case 0x3C71:
case 0x3E71:
case 0x3072:
case 0x3272:
case 0x3472:
case 0x3672:
case 0x3872:
case 0x3A72:
case 0x3C72:
case 0x3E72:
case 0x3073:
case 0x3273:
case 0x3473:
case 0x3673:
case 0x3873:
case 0x3A73:
case 0x3C73:
case 0x3E73:
case 0x3074:
case 0x3274:
case 0x3474:
case 0x3674:
case 0x3874:
case 0x3A74:
case 0x3C74:
case 0x3E74:
case 0x3075:
case 0x3275:
case 0x3475:
case 0x3675:
case 0x3875:
case 0x3A75:
case 0x3C75:
case 0x3E75:
case 0x3076:
case 0x3276:
case 0x3476:
case 0x3676:
case 0x3876:
case 0x3A76:
case 0x3C76:
case 0x3E76:
case 0x3077:
case 0x3277:
case 0x3477:
case 0x3677:
case 0x3877:
case 0x3A77:
case 0x3C77:
case 0x3E77:

// MOVEAW
case 0x3070:
{
	u32 adr;
	u32 res;
	adr = CPU->A[(Opcode >> 0) & 7];
	DECODE_EXT_WORD
	PRE_IO
	READSX_WORD_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0x3278:
case 0x3478:
case 0x3678:
case 0x3878:
case 0x3A78:
case 0x3C78:
case 0x3E78:

// MOVEAW
case 0x3078:
{
	u32 adr;
	u32 res;
	adr = (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_WORD_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(12)
case 0x3279:
case 0x3479:
case 0x3679:
case 0x3879:
case 0x3A79:
case 0x3C79:
case 0x3E79:

// MOVEAW
case 0x3079:
{
	u32 adr;
	u32 res;
	adr = (s32)FETCH_LONG;
	PC += 4;
	PRE_IO
	READSX_WORD_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(16)
case 0x327A:
case 0x347A:
case 0x367A:
case 0x387A:
case 0x3A7A:
case 0x3C7A:
case 0x3E7A:

// MOVEAW
case 0x307A:
{
	u32 adr;
	u32 res;
	adr = PC + (s32)(s16)FETCH_WORD;
	PC += 2;
	PRE_IO
	READSX_WORD_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(12)
case 0x327B:
case 0x347B:
case 0x367B:
case 0x387B:
case 0x3A7B:
case 0x3C7B:
case 0x3E7B:

// MOVEAW
case 0x307B:
{
	u32 adr;
	u32 res;
	adr = PC;
	DECODE_EXT_WORD
	PRE_IO
	READSX_WORD_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(14)
case 0x327C:
case 0x347C:
case 0x367C:
case 0x387C:
case 0x3A7C:
case 0x3C7C:
case 0x3E7C:

// MOVEAW
case 0x307C:
{
	u32 res;
	res = (s32)(s16)FETCH_WORD;
	PC += 2;
	CPU->A[(Opcode >> 9) & 7] = res;
}
RET(8)
case 0x325F:
case 0x345F:
case 0x365F:
case 0x385F:
case 0x3A5F:
case 0x3C5F:
case 0x3E5F:

// MOVEAW
case 0x305F:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7];
	CPU->A[7] += 2;
	PRE_IO
	READSX_WORD_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(8)
case 0x3267:
case 0x3467:
case 0x3667:
case 0x3867:
case 0x3A67:
case 0x3C67:
case 0x3E67:

// MOVEAW
case 0x3067:
{
	u32 adr;
	u32 res;
	adr = CPU->A[7] - 2;
	CPU->A[7] = adr;
	PRE_IO
	READSX_WORD_F(adr, res)
	CPU->A[(Opcode >> 9) & 7] = res;
	POST_IO
}
RET(10)
