/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

void ANROM_Init(CartInfo *info);

void HKROM_Init(CartInfo *info);

void ETROM_Init(CartInfo *info);
void EKROM_Init(CartInfo *info);
void ELROM_Init(CartInfo *info);
void EWROM_Init(CartInfo *info);

void SAROM_Init(CartInfo *info);
void SBROM_Init(CartInfo *info);
void SCROM_Init(CartInfo *info);
void SEROM_Init(CartInfo *info);
void SGROM_Init(CartInfo *info);
void SKROM_Init(CartInfo *info);
void SLROM_Init(CartInfo *info);
void SL1ROM_Init(CartInfo *info);
void SNROM_Init(CartInfo *info);
void SOROM_Init(CartInfo *info);

void NROM_Init(CartInfo *info);
void MHROM_Init(CartInfo *info);
void UNROM_Init(CartInfo *info);
void SUNSOFT_UNROM_Init(CartInfo *info); /* "Shanghi" original version mapper*/
void MALEE_Init(CartInfo *info);
void CNROM_Init(CartInfo *info);
void CPROM_Init(CartInfo *info);
void GNROM_Init(CartInfo *info);
void UNL3DBlock_Init(CartInfo *info);

void TBROM_Init(CartInfo *info);
void TEROM_Init(CartInfo *info);
void TFROM_Init(CartInfo *info);
void TGROM_Init(CartInfo *info);
void TKROM_Init(CartInfo *info);
void TSROM_Init(CartInfo *info);
void TLROM_Init(CartInfo *info);
void TLSROM_Init(CartInfo *info);
void TKSROM_Init(CartInfo *info);
void TQROM_Init(CartInfo *info);
void TQROM_Init(CartInfo *info);

void DEIROM_Init(CartInfo *info);

void TCA01_Init(CartInfo *info);
void TCU01_Init(CartInfo *info);
void TCU02_Init(CartInfo *info);
void S8259A_Init(CartInfo *info);
void S8259B_Init(CartInfo *info);
void S8259C_Init(CartInfo *info);
void S8259D_Init(CartInfo *info);
void S74LS374N_Init(CartInfo *info);
void S74LS374NA_Init(CartInfo *info);
void SA0161M_Init(CartInfo *info);

void SA72007_Init(CartInfo *info);
void SA72008_Init(CartInfo *info);
void SA009_Init(CartInfo *info);
void SA0036_Init(CartInfo *info);
void SA0037_Init(CartInfo *info);

void Supervision16_Init(CartInfo *info);
void Super24_Init(CartInfo *info);
void Novel_Init(CartInfo *info);

void BMC64in1nr_Init(CartInfo *info);
void BMC70in1_Init(CartInfo *info);
void BMC70in1B_Init(CartInfo *info);
void BMC13in1JY110_Init(CartInfo *info);
void BMCT262_Init(CartInfo *info);
void BMCFK23C_Init(CartInfo *info);
void BMCFK23CA_Init(CartInfo *info);
void BMCD1038_Init(CartInfo *info);
void BMCA65AS_Init(CartInfo *info);
void BMCGK192_Init(CartInfo *info);
void BMCGS2004_Init(CartInfo *info);
void BMCGS2013_Init(CartInfo *info);
void BMC190in1_Init(CartInfo *info);
void BMCGhostbusters63in1_Init(CartInfo *info);
void BMCBS5_Init(CartInfo *info);

void DreamTech01_Init(CartInfo *info);
void Mapper190_Init(CartInfo *info);
void UNLCC21_Init(CartInfo *info);
void UNLYOKO_Init(CartInfo *info);
void UNLFS304_Init(CartInfo *info);
void UNL43272_Init(CartInfo *info);
void UNLSL1632_Init(CartInfo *info);
void UNLSL12_Init(CartInfo *info);
void UNLKOF97_Init(CartInfo *info);
void UNLA9746_Init(CartInfo *info);
void UNLSHeroes_Init(CartInfo *info);
void UNLH2288_Init(CartInfo *info);
void UNL8237_Init(CartInfo *info);
void UNL8237A_Init(CartInfo *info);
void UNL8157_Init(CartInfo *info);
void UNL22211_Init(CartInfo *info);
void UNLTF1201_Init(CartInfo *info);
void UNLKS7057_Init(CartInfo *info);
void UNLKS7037_Init(CartInfo *info);
void UNLKS7032_Init(CartInfo *info);
void UNLKS7017_Init(CartInfo *info);
void UNLKS7012_Init(CartInfo *info);
void UNLKS7013B_Init(CartInfo *info);
void UNLT230_Init(CartInfo *info);
void UNLAX5705_Init(CartInfo *info);
void UNLDANCE_Init(CartInfo *info);
void UNLOneBus_Init(CartInfo *info);
void UNLSC127_Init(CartInfo *info);
void UNLPEC586Init(CartInfo *info);
void UNLVRC7_Init(CartInfo *info);
void UNLD2000_Init(CartInfo *info);
void BMC810544CA1_Init(CartInfo *info);
void BMCNTD03_Init(CartInfo *info);

void UNLEDU2000_Init(CartInfo *info);
void UNL6035052_Init(CartInfo *info);
void UNLN625092_Init(CartInfo *info);
void UNLSMB2J_Init(CartInfo *info);
/*void UNL09034A_Init(CartInfo *info);*/
void BMC411120C_Init(CartInfo *info);
void BMC830118C_Init(CartInfo *info);
void BMCT2271_Init(CartInfo *info);
void BMC12IN1_Init(CartInfo *info);
void Transformer_Init(CartInfo *info);
void LE05_Init(CartInfo *info);
void AC08_Init(CartInfo *info);
void LH10_Init(CartInfo *info);

#ifdef COPYFAMI
void MapperCopyFamiMMC3_Init(CartInfo *info);
void MapperCopyFami_Init(CartInfo *info);
#endif

/* So I can stop CHR RAM* bank switcherooing with certain boards...*/
extern uint8 *UNIFchrrama;
