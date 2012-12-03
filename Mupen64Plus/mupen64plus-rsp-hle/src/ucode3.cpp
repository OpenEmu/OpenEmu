/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-rsp-hle - ucode3.cpp                                      *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Richard Goedeken                                   *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

# include <string.h>
# include <stdio.h>

extern "C" {
  #include "m64p_types.h"
  #include "hle.h"
}

extern "C" void (*ABI3[])(void);

static void SPNOOP (void) {
    DebugMessage(M64MSG_ERROR, "Unknown/Unimplemented Audio Command %i in ABI 3", (int)(inst1 >> 24));
}

extern u16 ResampleLUT [0x200];

extern u32 loopval;

extern s16 Env_Dry;
extern s16 Env_Wet;
extern s16 Vol_Left;
extern s16 Vol_Right;
extern s16 VolTrg_Left;
extern s32 VolRamp_Left;
//extern u16 VolRate_Left;
extern s16 VolTrg_Right;
extern s32 VolRamp_Right;
//extern u16 VolRate_Right;


extern short hleMixerWorkArea[256];
extern u16 adpcmtable[0x88];

extern u8 BufferSpace[0x10000];

/*
static void SETVOL3 (void) { // Swapped Rate_Left and Vol
    u8 Flags = (u8)(inst1 >> 0x10);
    if (Flags & 0x4) { // 288
        if (Flags & 0x2) { // 290
            VolTrg_Left  = *(s16*)&inst1;
            VolRamp_Left = *(s32*)&inst2;
        } else {
            VolTrg_Right  = *(s16*)&inst1;
            VolRamp_Right = *(s32*)&inst2;
        }
    } else {
        Vol_Left    = *(s16*)&inst1;
        Env_Dry     = (s16)(*(s32*)&inst2 >> 0x10);
        Env_Wet     = *(s16*)&inst2;
    }
}
*/
static void SETVOL3 (void) {
    u8 Flags = (u8)(inst1 >> 0x10);
    if (Flags & 0x4) { // 288
        if (Flags & 0x2) { // 290
            Vol_Left  = (s16)inst1; // 0x50
            Env_Dry   = (s16)(inst2 >> 0x10); // 0x4E
            Env_Wet   = (s16)inst2; // 0x4C
        } else {
            VolTrg_Right  = (s16)inst1; // 0x46
            //VolRamp_Right = (u16)(inst2 >> 0x10) | (s32)(s16)(inst2 << 0x10);
            VolRamp_Right = (s32)inst2; // 0x48/0x4A
        }
    } else {
        VolTrg_Left  = (s16)inst1; // 0x40
        VolRamp_Left = (s32)inst2; // 0x42/0x44
    }
}

static void ENVMIXER3 (void) {
    u8 flags = (u8)((inst1 >> 16) & 0xff);
    u32 addy = (inst2 & 0xFFFFFF);

    short *inp=(short *)(BufferSpace+0x4F0);
    short *out=(short *)(BufferSpace+0x9D0);
    short *aux1=(short *)(BufferSpace+0xB40);
    short *aux2=(short *)(BufferSpace+0xCB0);
    short *aux3=(short *)(BufferSpace+0xE20);
    s32 MainR;
    s32 MainL;
    s32 AuxR;
    s32 AuxL;
    int i1,o1,a1,a2,a3;
    //unsigned short AuxIncRate=1;
    short zero[8];
    memset(zero,0,16);

    s32 LAdder, LAcc, LVol;
    s32 RAdder, RAcc, RVol;
    s16 RSig, LSig; // Most significant part of the Ramp Value
    s16 Wet, Dry;
    s16 LTrg, RTrg;

    Vol_Right = (s16)inst1;

    if (flags & A_INIT) {
        LAdder = VolRamp_Left / 8;
        LAcc  = 0;
        LVol  = Vol_Left;
        LSig = (s16)(VolRamp_Left >> 16);

        RAdder = VolRamp_Right / 8;
        RAcc  = 0;
        RVol  = Vol_Right;
        RSig = (s16)(VolRamp_Right >> 16);

        Wet = (s16)Env_Wet; Dry = (s16)Env_Dry; // Save Wet/Dry values
        LTrg = VolTrg_Left; RTrg = VolTrg_Right; // Save Current Left/Right Targets
    } else {
        memcpy((u8 *)hleMixerWorkArea, rsp.RDRAM+addy, 80);
        Wet    = *(s16 *)(hleMixerWorkArea +  0); // 0-1
        Dry    = *(s16 *)(hleMixerWorkArea +  2); // 2-3
        LTrg   = *(s16 *)(hleMixerWorkArea +  4); // 4-5
        RTrg   = *(s16 *)(hleMixerWorkArea +  6); // 6-7
        LAdder = *(s32 *)(hleMixerWorkArea +  8); // 8-9 (hleMixerWorkArea is a 16bit pointer)
        RAdder = *(s32 *)(hleMixerWorkArea + 10); // 10-11
        LAcc   = *(s32 *)(hleMixerWorkArea + 12); // 12-13
        RAcc   = *(s32 *)(hleMixerWorkArea + 14); // 14-15
        LVol   = *(s32 *)(hleMixerWorkArea + 16); // 16-17
        RVol   = *(s32 *)(hleMixerWorkArea + 18); // 18-19
        LSig   = *(s16 *)(hleMixerWorkArea + 20); // 20-21
        RSig   = *(s16 *)(hleMixerWorkArea + 22); // 22-23
        //u32 test  = *(s32 *)(hleMixerWorkArea + 24); // 22-23
        //if (test != 0x13371337)
    }


    //if(!(flags&A_AUX)) {
    //  AuxIncRate=0;
    //  aux2=aux3=zero;
    //}

    for (int y = 0; y < (0x170/2); y++) {

        // Left
        LAcc += LAdder;
        LVol += (LAcc >> 16);
        LAcc &= 0xFFFF;

        // Right
        RAcc += RAdder;
        RVol += (RAcc >> 16);
        RAcc &= 0xFFFF;
// ****************************************************************
        // Clamp Left
        if (LSig >= 0) { // VLT
            if (LVol > LTrg) {
                LVol = LTrg;
            }
        } else { // VGE
            if (LVol < LTrg) {
                LVol = LTrg;
            }
        }

        // Clamp Right
        if (RSig >= 0) { // VLT
            if (RVol > RTrg) {
                RVol = RTrg;
            }
        } else { // VGE
            if (RVol < RTrg) {
                RVol = RTrg;
            }
        }
// ****************************************************************
        MainL = ((Dry * LVol) + 0x4000) >> 15;
        MainR = ((Dry * RVol) + 0x4000) >> 15;

        o1 = out [y^S];
        a1 = aux1[y^S];
        i1 = inp [y^S];

        o1+=((i1*MainL)+0x4000)>>15;
        a1+=((i1*MainR)+0x4000)>>15;

// ****************************************************************

        if(o1>32767) o1=32767;
        else if(o1<-32768) o1=-32768;

        if(a1>32767) a1=32767;
        else if(a1<-32768) a1=-32768;

// ****************************************************************

        out[y^S]=o1;
        aux1[y^S]=a1;

// ****************************************************************
        //if (!(flags&A_AUX)) {
            a2 = aux2[y^S];
            a3 = aux3[y^S];

            AuxL  = ((Wet * LVol) + 0x4000) >> 15;
            AuxR  = ((Wet * RVol) + 0x4000) >> 15;

            a2+=((i1*AuxL)+0x4000)>>15;
            a3+=((i1*AuxR)+0x4000)>>15;
            
            if(a2>32767) a2=32767;
            else if(a2<-32768) a2=-32768;

            if(a3>32767) a3=32767;
            else if(a3<-32768) a3=-32768;

            aux2[y^S]=a2;
            aux3[y^S]=a3;
        }
    //}

    *(s16 *)(hleMixerWorkArea +  0) = Wet; // 0-1
    *(s16 *)(hleMixerWorkArea +  2) = Dry; // 2-3
    *(s16 *)(hleMixerWorkArea +  4) = LTrg; // 4-5
    *(s16 *)(hleMixerWorkArea +  6) = RTrg; // 6-7
    *(s32 *)(hleMixerWorkArea +  8) = LAdder; // 8-9 (hleMixerWorkArea is a 16bit pointer)
    *(s32 *)(hleMixerWorkArea + 10) = RAdder; // 10-11
    *(s32 *)(hleMixerWorkArea + 12) = LAcc; // 12-13
    *(s32 *)(hleMixerWorkArea + 14) = RAcc; // 14-15
    *(s32 *)(hleMixerWorkArea + 16) = LVol; // 16-17
    *(s32 *)(hleMixerWorkArea + 18) = RVol; // 18-19
    *(s16 *)(hleMixerWorkArea + 20) = LSig; // 20-21
    *(s16 *)(hleMixerWorkArea + 22) = RSig; // 22-23
    //*(u32 *)(hleMixerWorkArea + 24) = 0x13371337; // 22-23
    memcpy(rsp.RDRAM+addy, (u8 *)hleMixerWorkArea,80);
}

static void CLEARBUFF3 (void) {
    u16 addr = (u16)(inst1 & 0xffff);
    u16 count = (u16)(inst2 & 0xffff);
    memset(BufferSpace+addr+0x4f0, 0, count);
}

static void MIXER3 (void) { // Needs accuracy verification...
    u16 dmemin  = (u16)(inst2 >> 0x10)  + 0x4f0;
    u16 dmemout = (u16)(inst2 & 0xFFFF) + 0x4f0;
    //u8  flags   = (u8)((inst1 >> 16) & 0xff);
    s32 gain    = (s16)(inst1 & 0xFFFF);
    s32 temp;

    for (int x=0; x < 0x170; x+=2) { // I think I can do this a lot easier
        temp = (*(s16 *)(BufferSpace+dmemin+x) * gain) >> 15;
        temp += *(s16 *)(BufferSpace+dmemout+x);
            
        if ((s32)temp > 32767) 
            temp = 32767;
        if ((s32)temp < -32768) 
            temp = -32768;

        *(u16 *)(BufferSpace+dmemout+x) = (u16)(temp & 0xFFFF);
    }
}

static void LOADBUFF3 (void) {
    u32 v0;
    u32 cnt = (((inst1 >> 0xC)+3)&0xFFC);
    v0 = (inst2 & 0xfffffc);
    u32 src = (inst1&0xffc)+0x4f0;
    memcpy (BufferSpace+src, rsp.RDRAM+v0, cnt);
}

static void SAVEBUFF3 (void) {
    u32 v0;
    u32 cnt = (((inst1 >> 0xC)+3)&0xFFC);
    v0 = (inst2 & 0xfffffc);
    u32 src = (inst1&0xffc)+0x4f0;
    memcpy (rsp.RDRAM+v0, BufferSpace+src, cnt);
}

static void LOADADPCM3 (void) { // Loads an ADPCM table - Works 100% Now 03-13-01
    u32 v0;
    v0 = (inst2 & 0xffffff);
    //memcpy (dmem+0x3f0, rsp.RDRAM+v0, inst1&0xffff); 
    //assert ((inst1&0xffff) <= 0x80);
    u16 *table = (u16 *)(rsp.RDRAM+v0);
    for (u32 x = 0; x < ((inst1&0xffff)>>0x4); x++) {
        adpcmtable[(0x0+(x<<3))^S] = table[0];
        adpcmtable[(0x1+(x<<3))^S] = table[1];

        adpcmtable[(0x2+(x<<3))^S] = table[2];
        adpcmtable[(0x3+(x<<3))^S] = table[3];

        adpcmtable[(0x4+(x<<3))^S] = table[4];
        adpcmtable[(0x5+(x<<3))^S] = table[5];

        adpcmtable[(0x6+(x<<3))^S] = table[6];
        adpcmtable[(0x7+(x<<3))^S] = table[7];
        table += 8;
    }
}

static void DMEMMOVE3 (void) { // Needs accuracy verification...
    u32 v0, v1;
    u32 cnt;
    v0 = (inst1 & 0xFFFF) + 0x4f0;
    v1 = (inst2 >> 0x10) + 0x4f0;
    u32 count = ((inst2+3) & 0xfffc);

    //memcpy (dmem+v1, dmem+v0, count-1);
    for (cnt = 0; cnt < count; cnt++) {
        *(u8 *)(BufferSpace+((cnt+v1)^S8)) = *(u8 *)(BufferSpace+((cnt+v0)^S8));
    }
}

static void SETLOOP3 (void) {
    loopval = (inst2 & 0xffffff);
}

static void ADPCM3 (void) { // Verified to be 100% Accurate...
    unsigned char Flags=(u8)(inst2>>0x1c)&0xff;
    //unsigned short Gain=(u16)(inst1&0xffff);
    unsigned int Address=(inst1 & 0xffffff);// + SEGMENTS[(inst2>>24)&0xf];
    unsigned short inPtr=(inst2>>12)&0xf;
    //short *out=(s16 *)(testbuff+(AudioOutBuffer>>2));
    short *out=(short *)(BufferSpace+(inst2&0xfff)+0x4f0);
    //unsigned char *in=(unsigned char *)(BufferSpace+((inst2>>12)&0xf)+0x4f0);
    short count=(short)((inst2 >> 16)&0xfff);
    unsigned char icode;
    unsigned char code;
    int vscale;
    unsigned short index;
    unsigned short j;
    int a[8];
    short *book1,*book2;

    memset(out,0,32);

    if(!(Flags&0x1))
    {
        if(Flags&0x2)
        {/*
            for(int i=0;i<16;i++)
            {
                out[i]=*(short *)&rsp.RDRAM[(loopval+i*2)^2];
            }*/
            memcpy(out,&rsp.RDRAM[loopval],32);
        }
        else
        {/*
            for(int i=0;i<16;i++)
            {
                out[i]=*(short *)&rsp.RDRAM[(Address+i*2)^2];
            }*/
            memcpy(out,&rsp.RDRAM[Address],32);
        }
    }

    int l1=out[14^S];
    int l2=out[15^S];
    int inp1[8];
    int inp2[8];
    out+=16;
    while(count>0)
    {
                                                    // the first interation through, these values are
                                                    // either 0 in the case of A_INIT, from a special
                                                    // area of memory in the case of A_LOOP or just
                                                    // the values we calculated the last time

        code=BufferSpace[(0x4f0+inPtr)^S8];
        index=code&0xf;
        index<<=4;                                  // index into the adpcm code table
        book1=(short *)&adpcmtable[index];
        book2=book1+8;
        code>>=4;                                   // upper nibble is scale
        vscale=(0x8000>>((12-code)-1));         // very strange. 0x8000 would be .5 in 16:16 format
                                                    // so this appears to be a fractional scale based
                                                    // on the 12 based inverse of the scale value.  note
                                                    // that this could be negative, in which case we do
                                                    // not use the calculated vscale value... see the 
                                                    // if(code>12) check below

        inPtr++;                                    // coded adpcm data lies next
        j=0;
        while(j<8)                                  // loop of 8, for 8 coded nibbles from 4 bytes
                                                    // which yields 8 short pcm values
        {
            icode=BufferSpace[(0x4f0+inPtr)^S8];
            inPtr++;

            inp1[j]=(s16)((icode&0xf0)<<8);         // this will in effect be signed
            if(code<12)
                inp1[j]=((int)((int)inp1[j]*(int)vscale)>>16);
            /*else
                int catchme=1;*/
            j++;

            inp1[j]=(s16)((icode&0xf)<<12);
            if(code<12)
                inp1[j]=((int)((int)inp1[j]*(int)vscale)>>16);
            /*else
                int catchme=1;*/
            j++;
        }
        j=0;
        while(j<8)
        {
            icode=BufferSpace[(0x4f0+inPtr)^S8];
            inPtr++;

            inp2[j]=(short)((icode&0xf0)<<8);           // this will in effect be signed
            if(code<12)
                inp2[j]=((int)((int)inp2[j]*(int)vscale)>>16);
            /*else
                int catchme=1;*/
            j++;

            inp2[j]=(short)((icode&0xf)<<12);
            if(code<12)
                inp2[j]=((int)((int)inp2[j]*(int)vscale)>>16);
            /*else
                int catchme=1;*/
            j++;
        }

        a[0]= (int)book1[0]*(int)l1;
        a[0]+=(int)book2[0]*(int)l2;
        a[0]+=(int)inp1[0]*(int)2048;

        a[1] =(int)book1[1]*(int)l1;
        a[1]+=(int)book2[1]*(int)l2;
        a[1]+=(int)book2[0]*inp1[0];
        a[1]+=(int)inp1[1]*(int)2048;

        a[2] =(int)book1[2]*(int)l1;
        a[2]+=(int)book2[2]*(int)l2;
        a[2]+=(int)book2[1]*inp1[0];
        a[2]+=(int)book2[0]*inp1[1];
        a[2]+=(int)inp1[2]*(int)2048;

        a[3] =(int)book1[3]*(int)l1;
        a[3]+=(int)book2[3]*(int)l2;
        a[3]+=(int)book2[2]*inp1[0];
        a[3]+=(int)book2[1]*inp1[1];
        a[3]+=(int)book2[0]*inp1[2];
        a[3]+=(int)inp1[3]*(int)2048;

        a[4] =(int)book1[4]*(int)l1;
        a[4]+=(int)book2[4]*(int)l2;
        a[4]+=(int)book2[3]*inp1[0];
        a[4]+=(int)book2[2]*inp1[1];
        a[4]+=(int)book2[1]*inp1[2];
        a[4]+=(int)book2[0]*inp1[3];
        a[4]+=(int)inp1[4]*(int)2048;

        a[5] =(int)book1[5]*(int)l1;
        a[5]+=(int)book2[5]*(int)l2;
        a[5]+=(int)book2[4]*inp1[0];
        a[5]+=(int)book2[3]*inp1[1];
        a[5]+=(int)book2[2]*inp1[2];
        a[5]+=(int)book2[1]*inp1[3];
        a[5]+=(int)book2[0]*inp1[4];
        a[5]+=(int)inp1[5]*(int)2048;

        a[6] =(int)book1[6]*(int)l1;
        a[6]+=(int)book2[6]*(int)l2;
        a[6]+=(int)book2[5]*inp1[0];
        a[6]+=(int)book2[4]*inp1[1];
        a[6]+=(int)book2[3]*inp1[2];
        a[6]+=(int)book2[2]*inp1[3];
        a[6]+=(int)book2[1]*inp1[4];
        a[6]+=(int)book2[0]*inp1[5];
        a[6]+=(int)inp1[6]*(int)2048;

        a[7] =(int)book1[7]*(int)l1;
        a[7]+=(int)book2[7]*(int)l2;
        a[7]+=(int)book2[6]*inp1[0];
        a[7]+=(int)book2[5]*inp1[1];
        a[7]+=(int)book2[4]*inp1[2];
        a[7]+=(int)book2[3]*inp1[3];
        a[7]+=(int)book2[2]*inp1[4];
        a[7]+=(int)book2[1]*inp1[5];
        a[7]+=(int)book2[0]*inp1[6];
        a[7]+=(int)inp1[7]*(int)2048;

        for(j=0;j<8;j++)
        {
            a[j^S]>>=11;
            if(a[j^S]>32767) a[j^S]=32767;
            else if(a[j^S]<-32768) a[j^S]=-32768;
            *(out++)=a[j^S];
            //*(out+j)=a[j^S];
        }
        //out += 0x10;
        l1=a[6];
        l2=a[7];

        a[0]= (int)book1[0]*(int)l1;
        a[0]+=(int)book2[0]*(int)l2;
        a[0]+=(int)inp2[0]*(int)2048;

        a[1] =(int)book1[1]*(int)l1;
        a[1]+=(int)book2[1]*(int)l2;
        a[1]+=(int)book2[0]*inp2[0];
        a[1]+=(int)inp2[1]*(int)2048;

        a[2] =(int)book1[2]*(int)l1;
        a[2]+=(int)book2[2]*(int)l2;
        a[2]+=(int)book2[1]*inp2[0];
        a[2]+=(int)book2[0]*inp2[1];
        a[2]+=(int)inp2[2]*(int)2048;

        a[3] =(int)book1[3]*(int)l1;
        a[3]+=(int)book2[3]*(int)l2;
        a[3]+=(int)book2[2]*inp2[0];
        a[3]+=(int)book2[1]*inp2[1];
        a[3]+=(int)book2[0]*inp2[2];
        a[3]+=(int)inp2[3]*(int)2048;

        a[4] =(int)book1[4]*(int)l1;
        a[4]+=(int)book2[4]*(int)l2;
        a[4]+=(int)book2[3]*inp2[0];
        a[4]+=(int)book2[2]*inp2[1];
        a[4]+=(int)book2[1]*inp2[2];
        a[4]+=(int)book2[0]*inp2[3];
        a[4]+=(int)inp2[4]*(int)2048;

        a[5] =(int)book1[5]*(int)l1;
        a[5]+=(int)book2[5]*(int)l2;
        a[5]+=(int)book2[4]*inp2[0];
        a[5]+=(int)book2[3]*inp2[1];
        a[5]+=(int)book2[2]*inp2[2];
        a[5]+=(int)book2[1]*inp2[3];
        a[5]+=(int)book2[0]*inp2[4];
        a[5]+=(int)inp2[5]*(int)2048;

        a[6] =(int)book1[6]*(int)l1;
        a[6]+=(int)book2[6]*(int)l2;
        a[6]+=(int)book2[5]*inp2[0];
        a[6]+=(int)book2[4]*inp2[1];
        a[6]+=(int)book2[3]*inp2[2];
        a[6]+=(int)book2[2]*inp2[3];
        a[6]+=(int)book2[1]*inp2[4];
        a[6]+=(int)book2[0]*inp2[5];
        a[6]+=(int)inp2[6]*(int)2048;

        a[7] =(int)book1[7]*(int)l1;
        a[7]+=(int)book2[7]*(int)l2;
        a[7]+=(int)book2[6]*inp2[0];
        a[7]+=(int)book2[5]*inp2[1];
        a[7]+=(int)book2[4]*inp2[2];
        a[7]+=(int)book2[3]*inp2[3];
        a[7]+=(int)book2[2]*inp2[4];
        a[7]+=(int)book2[1]*inp2[5];
        a[7]+=(int)book2[0]*inp2[6];
        a[7]+=(int)inp2[7]*(int)2048;

        for(j=0;j<8;j++)
        {
            a[j^S]>>=11;
            if(a[j^S]>32767) a[j^S]=32767;
            else if(a[j^S]<-32768) a[j^S]=-32768;
            *(out++)=a[j^S];
            //*(out+j+0x1f8)=a[j^S];
        }
        l1=a[6];
        l2=a[7];

        count-=32;
    }
    out-=16;
    memcpy(&rsp.RDRAM[Address],out,32);
}

static void RESAMPLE3 (void) {
    unsigned char Flags=(u8)((inst2>>0x1e));
    unsigned int Pitch=((inst2>>0xe)&0xffff)<<1;
    u32 addy = (inst1 & 0xffffff);
    unsigned int Accum=0;
    unsigned int location;
    s16 *lut;
    short *dst;
    s16 *src;
    dst=(short *)(BufferSpace);
    src=(s16 *)(BufferSpace);
    u32 srcPtr=((((inst2>>2)&0xfff)+0x4f0)/2);
    u32 dstPtr;//=(AudioOutBuffer/2);
    s32 temp;
    s32 accum;

    //if (addy > (1024*1024*8))
    //  addy = (inst2 & 0xffffff);

    srcPtr -= 4;

    if (inst2 & 0x3) {
        dstPtr = 0x660/2;
    } else {
        dstPtr = 0x4f0/2;
    }

    if ((Flags & 0x1) == 0) {   
        for (int x=0; x < 4; x++) //memcpy (src+srcPtr, rsp.RDRAM+addy, 0x8);
            src[(srcPtr+x)^S] = ((u16 *)rsp.RDRAM)[((addy/2)+x)^S];
        Accum = *(u16 *)(rsp.RDRAM+addy+10);
    } else {
        for (int x=0; x < 4; x++)
            src[(srcPtr+x)^S] = 0;//*(u16 *)(rsp.RDRAM+((addy+x)^2));
    }

    for(int i=0;i < 0x170/2;i++)    {
        location = (((Accum * 0x40) >> 0x10) * 8);
        //location = (Accum >> 0xa) << 0x3;
        lut = (s16 *)(((u8 *)ResampleLUT) + location);

        temp =  ((s32)*(s16*)(src+((srcPtr+0)^S))*((s32)((s16)lut[0])));
        accum = (s32)(temp >> 15);

        temp = ((s32)*(s16*)(src+((srcPtr+1)^S))*((s32)((s16)lut[1])));
        accum += (s32)(temp >> 15);

        temp = ((s32)*(s16*)(src+((srcPtr+2)^S))*((s32)((s16)lut[2])));
        accum += (s32)(temp >> 15);
        
        temp = ((s32)*(s16*)(src+((srcPtr+3)^S))*((s32)((s16)lut[3])));
        accum += (s32)(temp >> 15);
/*      temp =  ((s64)*(s16*)(src+((srcPtr+0)^S))*((s64)((s16)lut[0]<<1)));
        if (temp & 0x8000) temp = (temp^0x8000) + 0x10000;
        else temp = (temp^0x8000);
        temp = (s32)(temp >> 16);
        if ((s32)temp > 32767) temp = 32767;
        if ((s32)temp < -32768) temp = -32768;
        accum = (s32)(s16)temp;

        temp = ((s64)*(s16*)(src+((srcPtr+1)^S))*((s64)((s16)lut[1]<<1)));
        if (temp & 0x8000) temp = (temp^0x8000) + 0x10000;
        else temp = (temp^0x8000);
        temp = (s32)(temp >> 16);
        if ((s32)temp > 32767) temp = 32767;
        if ((s32)temp < -32768) temp = -32768;
        accum += (s32)(s16)temp;

        temp = ((s64)*(s16*)(src+((srcPtr+2)^S))*((s64)((s16)lut[2]<<1)));
        if (temp & 0x8000) temp = (temp^0x8000) + 0x10000;
        else temp = (temp^0x8000);
        temp = (s32)(temp >> 16);
        if ((s32)temp > 32767) temp = 32767;
        if ((s32)temp < -32768) temp = -32768;
        accum += (s32)(s16)temp;

        temp = ((s64)*(s16*)(src+((srcPtr+3)^S))*((s64)((s16)lut[3]<<1)));
        if (temp & 0x8000) temp = (temp^0x8000) + 0x10000;
        else temp = (temp^0x8000);
        temp = (s32)(temp >> 16);
        if ((s32)temp > 32767) temp = 32767;
        if ((s32)temp < -32768) temp = -32768;
        accum += (s32)(s16)temp;*/

        if (accum > 32767) accum = 32767;
        if (accum < -32768) accum = -32768;

        dst[dstPtr^S] = (accum);
        dstPtr++;
        Accum += Pitch;
        srcPtr += (Accum>>16);
        Accum&=0xffff;
    }
    for (int x=0; x < 4; x++)
        ((u16 *)rsp.RDRAM)[((addy/2)+x)^S] = src[(srcPtr+x)^S];
    *(u16 *)(rsp.RDRAM+addy+10) = Accum;
}

static void INTERLEAVE3 (void) { // Needs accuracy verification...
    //u32 inL, inR;
    u16 *outbuff = (u16 *)(BufferSpace + 0x4f0);//(u16 *)(AudioOutBuffer+dmem);
    u16 *inSrcR;
    u16 *inSrcL;
    u16 Left, Right, Left2, Right2;

    //inR = inst2 & 0xFFFF;
    //inL = (inst2 >> 16) & 0xFFFF;

    inSrcR = (u16 *)(BufferSpace+0xb40);
    inSrcL = (u16 *)(BufferSpace+0x9d0);

    for (int x = 0; x < (0x170/4); x++) {
        Left=*(inSrcL++);
        Right=*(inSrcR++);
        Left2=*(inSrcL++);
        Right2=*(inSrcR++);

#ifdef M64P_BIG_ENDIAN
        *(outbuff++)=Right;
        *(outbuff++)=Left;
        *(outbuff++)=Right2;
        *(outbuff++)=Left2;
#else
        *(outbuff++)=Right2;
        *(outbuff++)=Left2;
        *(outbuff++)=Right;
        *(outbuff++)=Left;
#endif
/*
        Left=*(inSrcL++);
        Right=*(inSrcR++);
        *(outbuff++)=(u16)Left;
        Left >>= 16;
        *(outbuff++)=(u16)Right;
        Right >>= 16;
        *(outbuff++)=(u16)Left;
        *(outbuff++)=(u16)Right;*/
    }
}

//static void UNKNOWN (void);
/*
typedef struct {
    unsigned char sync;

    unsigned char error_protection   : 1;    //  0=yes, 1=no
    unsigned char lay                : 2;    // 4-lay = layerI, II or III
    unsigned char version            : 1;    // 3=mpeg 1.0, 2=mpeg 2.5 0=mpeg 2.0
    unsigned char sync2              : 4;

    unsigned char extension          : 1;    // Unknown
    unsigned char padding            : 1;    // padding
    unsigned char sampling_freq      : 2;    // see table below
    unsigned char bitrate_index      : 4;    //     see table below

    unsigned char emphasis           : 2;    //see table below
    unsigned char original           : 1;    // 0=no 1=yes
    unsigned char copyright          : 1;    // 0=no 1=yes
    unsigned char mode_ext           : 2;    // used with "joint stereo" mode
    unsigned char mode               : 2;    // Channel Mode
} mp3struct;

mp3struct mp3;
FILE *mp3dat;
*/

static void WHATISTHIS (void) {
}

//static FILE *fp = fopen ("d:\\mp3info.txt", "wt");
u32 setaddr;
static void MP3ADDY (void) {
    setaddr = (inst2 & 0xffffff);
}

extern "C" {
    void rsp_run(void);
    void mp3setup (unsigned int inst1, unsigned int inst2, unsigned int t8);
}

extern u32 base, dmembase;
extern "C" {
    extern char *pDMEM;
}
void MP3 (void);
/*
 {
//  return;
    // Setup Registers...
    mp3setup (inst1, inst2, 0xFA0);
    
    // Setup Memory Locations...
    //u32 base = ((u32*)dmem)[0xFD0/4]; // Should be 000291A0
    memcpy (BufferSpace, dmembase+rsp.RDRAM, 0x10);
    ((u32*)BufferSpace)[0x0] = base;
    ((u32*)BufferSpace)[0x008/4] += base;
    ((u32*)BufferSpace)[0xFFC/4] = loopval;
    ((u32*)BufferSpace)[0xFF8/4] = dmembase;

    memcpy (imem+0x238, rsp.RDRAM+((u32*)BufferSpace)[0x008/4], 0x9C0);
    ((u32*)BufferSpace)[0xFF4/4] = setaddr;
    pDMEM = (char *)BufferSpace;
    rsp_run (void);
    dmembase = ((u32*)BufferSpace)[0xFF8/4];
    loopval  = ((u32*)BufferSpace)[0xFFC/4];
//0x1A98  SW       S1, 0x0FF4 (R0)
//0x1A9C  SW       S0, 0x0FF8 (R0)
//0x1AA0  SW       T7, 0x0FFC (R0)
//0x1AA4  SW       T3, 0x0FF0 (R0)
    //fprintf (fp, "mp3: inst1: %08X, inst2: %08X\n", inst1, inst2);
}*/
/*
FFT = Fast Fourier Transform
DCT = Discrete Cosine Transform
MPEG-1 Layer 3 retains Layer 2's 1152-sample window, as well as the FFT polyphase filter for
backward compatibility, but adds a modified DCT filter. DCT's advantages over DFTs (discrete
Fourier transforms) include half as many multiply-accumulate operations and half the 
generated coefficients because the sinusoidal portion of the calculation is absent, and DCT 
generally involves simpler math. The finite lengths of a conventional DCTs' bandpass impulse
responses, however, may result in block-boundary effects. MDCTs overlap the analysis blocks 
and lowpass-filter the decoded audio to remove aliases, eliminating these effects. MDCTs also 
have a higher transform coding gain than the standard DCT, and their basic functions 
correspond to better bandpass response. 

MPEG-1 Layer 3's DCT sub-bands are unequally sized, and correspond to the human auditory 
system's critical bands. In Layer 3 decoders must support both constant- and variable-bit-rate 
bit streams. (However, many Layer 1 and 2 decoders also handle variable bit rates). Finally, 
Layer 3 encoders Huffman-code the quantized coefficients before archiving or transmission for 
additional lossless compression. Bit streams range from 32 to 320 kbps, and 128-kbps rates 
achieve near-CD quality, an important specification to enable dual-channel ISDN 
(integrated-services-digital-network) to be the future high-bandwidth pipe to the home. 

*/
static void DISABLE (void) {
    //MessageBox (NULL, "Help", "ABI 3 Command 0", MB_OK);
    //ChangeABI (5);
}


void (*ABI3[0x20])(void) = {
    DISABLE , ADPCM3 , CLEARBUFF3,  ENVMIXER3  , LOADBUFF3, RESAMPLE3  , SAVEBUFF3, MP3,
    MP3ADDY, SETVOL3, DMEMMOVE3 , LOADADPCM3 , MIXER3   , INTERLEAVE3, WHATISTHIS   , SETLOOP3,
    SPNOOP , SPNOOP, SPNOOP   , SPNOOP    , SPNOOP  , SPNOOP    , SPNOOP  , SPNOOP,
    SPNOOP , SPNOOP, SPNOOP   , SPNOOP    , SPNOOP  , SPNOOP    , SPNOOP  , SPNOOP
};


