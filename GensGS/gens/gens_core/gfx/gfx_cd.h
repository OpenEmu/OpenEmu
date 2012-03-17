#ifndef GENS_GFX_CD_H
#define GENS_GFX_CD_H

#ifdef __cplusplus
extern "C" {
#endif

extern struct
{
	unsigned int Stamp_Size;
	unsigned int Stamp_Map_Adr;
	unsigned int IB_V_Cell_Size;
	unsigned int IB_Adr;
	unsigned int IB_Offset;
	unsigned int IB_H_Dot_Size;
	unsigned int IB_V_Dot_Size;
	unsigned int Vector_Adr;
	int Rotation_Running;
} Rot_Comp;

extern int Table_Rot_Time[4 * 4 * 4];

// GENS Rerecording [GENS Savestate v7]
extern int Stamp_Map_Adr, Buffer_Adr, Vector_Adr, Jmp_Adr, Float_Part, Draw_Speed;
extern int XS, YS, DXS, DYS, XD, YD, XD_Mul, H_Dot;

void Init_RS_GFX(void);
int Calcul_Rot_Comp(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_GFX_CD_H */
