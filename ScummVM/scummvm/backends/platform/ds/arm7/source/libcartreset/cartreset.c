/**********************************
  Copyright (C) Rick Wong (Lick)
  http://licklick.wordpress.com/
***********************************/
#include <cartreset_nolibfat.h>


#ifdef ARM9

bool cartSetMenuMode(u32 _deviceType)
{
    *(vu16*)(0x04000204) &= ~0x0880;    //sysSetBusOwners(true, true);
    u32 deviceType = _deviceType;

    *((vu32*)0x027FFFF8) = 0x080000C0; // ARM7 reset address

    if(deviceType == DEVICE_TYPE_EFA2)
    {
        *(u16 *)0x9FE0000 = 0xD200;
        *(u16 *)0x8000000 = 0x1500;
        *(u16 *)0x8020000 = 0xD200;
        *(u16 *)0x8040000 = 0x1500;
        *(u16 *)0x9880000 = 1 << 15;
        *(u16 *)0x9FC0000 = 0x1500;
        return true;
    }
    else if(deviceType == DEVICE_TYPE_MPCF)
    {
        return true;
    }
    else if(deviceType == DEVICE_TYPE_EZSD)
    {
        return true;
    }
    else if(deviceType == DEVICE_TYPE_M3CF || deviceType == DEVICE_TYPE_M3SD)
    {
        u32 mode = 0x00400004;
	    vu16 tmp;
        tmp = *(vu16*)(0x08E00002);
        tmp = *(vu16*)(0x0800000E);
        tmp = *(vu16*)(0x08801FFC);
        tmp = *(vu16*)(0x0800104A);
        tmp = *(vu16*)(0x08800612);
        tmp = *(vu16*)(0x08000000);
        tmp = *(vu16*)(0x08801B66);
        tmp = *(vu16*)(0x08000000 + (mode << 1));
        tmp = *(vu16*)(0x0800080E);
        tmp = *(vu16*)(0x08000000);

        tmp = *(vu16*)(0x080001E4);
        tmp = *(vu16*)(0x080001E4);
        tmp = *(vu16*)(0x08000188);
        tmp = *(vu16*)(0x08000188);
        return true;
    }
    else if(deviceType == DEVICE_TYPE_SCCF || deviceType == DEVICE_TYPE_SCSD)
    {
        *(vu16*)0x09FFFFFE = 0xA55A;
        *(vu16*)0x09FFFFFE = 0xA55A;
        *(vu16*)0x09FFFFFE = 0;
        *(vu16*)0x09FFFFFE = 0;
        *((vu32*)0x027FFFF8) = 0x08000000; // Special ARM7 reset address
        return true;
    }

    return false;
}



void passmeloopEnter()
{
    *(vu16*)(0x04000208) = 0;           //REG_IME = IME_DISABLE;
    *(vu16*)(0x04000204) |= 0x0880;     //sysSetBusOwners(false, false);
    *((vu32*)0x027FFFFC) = 0;
    *((vu32*)0x027FFE04) = (u32)0xE59FF018;
    *((vu32*)0x027FFE24) = (u32)0x027FFE04;
    asm("swi 0x00");                    //swiSoftReset();
    asm("bx lr");
}

#endif


#ifdef ARM7

//#include <nds.h>

bool passmeloopQuery()
{
    if(*((vu32*)0x027FFE24) == (u32)0x027FFE04)
        return true;
    return false;
}



void cartExecute()
{
    *(vu16*)(0x04000208) = 0;       //REG_IME = IME_DISABLE;
    *((vu32*)0x027FFE34) = *((vu32*)0x027FFFF8);
    asm("swi 0x00");                //swiSoftReset();
    asm("bx lr");
}

#endif
