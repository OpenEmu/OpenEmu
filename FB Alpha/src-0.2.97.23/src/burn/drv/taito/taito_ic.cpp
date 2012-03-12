#include "burnint.h"
#include "taito_ic.h"

INT32 TaitoIC_SupermanCChipInUse = 0;
INT32 TaitoIC_MegabCChipInUse = 0;
INT32 TaitoIC_RainbowCChipInUse = 0;
INT32 TaitoIC_OpwolfCChipInUse = 0;
INT32 TaitoIC_VolfiedCChipInUse = 0;

INT32 TaitoIC_PC080SNInUse = 0;
INT32 TaitoIC_PC090OJInUse = 0;
INT32 TaitoIC_TC0100SCNInUse = 0;
INT32 TaitoIC_TC0110PCRInUse = 0;
INT32 TaitoIC_TC0140SYTInUse = 0;
INT32 TaitoIC_TC0150RODInUse = 0;
INT32 TaitoIC_TC0180VCUInUse = 0;
INT32 TaitoIC_TC0220IOCInUse = 0;
INT32 TaitoIC_TC0280GRDInUse = 0;
INT32 TaitoIC_TC0360PRIInUse = 0;
INT32 TaitoIC_TC0430GRWInUse = 0;
INT32 TaitoIC_TC0480SCPInUse = 0;
INT32 TaitoIC_TC0510NIOInUse = 0;
INT32 TaitoIC_TC0640FIOInUse = 0;

INT32 TaitoWatchdog;

void TaitoICReset()
{
	if (TaitoIC_SupermanCChipInUse) SupermanCChipReset();
	if (TaitoIC_MegabCChipInUse) MegabCChipReset();
	if (TaitoIC_RainbowCChipInUse) RainbowCChipReset();
	if (TaitoIC_OpwolfCChipInUse) OpwolfCChipReset();
	if (TaitoIC_VolfiedCChipInUse) VolfiedCChipReset();

	if (TaitoIC_PC080SNInUse) PC080SNReset();
	if (TaitoIC_PC090OJInUse) PC090OJReset();
	if (TaitoIC_TC0100SCNInUse) TC0100SCNReset();
	if (TaitoIC_TC0110PCRInUse) TC0110PCRReset();
	if (TaitoIC_TC0140SYTInUse) TC0140SYTReset();
	if (TaitoIC_TC0150RODInUse) TC0150RODReset();
	if (TaitoIC_TC0180VCUInUse) TC0180VCUReset();
	if (TaitoIC_TC0220IOCInUse) TC0220IOCReset();
	if (TaitoIC_TC0280GRDInUse) TC0280GRDReset();
	if (TaitoIC_TC0360PRIInUse) TC0360PRIReset();
	if (TaitoIC_TC0430GRWInUse) TC0430GRWReset();
	if (TaitoIC_TC0480SCPInUse) TC0480SCPReset();
	if (TaitoIC_TC0510NIOInUse) TC0510NIOReset();
	if (TaitoIC_TC0640FIOInUse) TC0640FIOReset();

	TaitoWatchdog = 0;
}

void TaitoICExit()
{
	if (TaitoIC_SupermanCChipInUse) SupermanCChipExit();
	if (TaitoIC_MegabCChipInUse) MegabCChipExit();
	if (TaitoIC_RainbowCChipInUse) RainbowCChipExit();
	if (TaitoIC_OpwolfCChipInUse) OpwolfCChipExit();
	if (TaitoIC_VolfiedCChipInUse) VolfiedCChipExit();

	if (TaitoIC_PC080SNInUse) PC080SNExit();
	if (TaitoIC_PC090OJInUse) PC090OJExit();
	if (TaitoIC_TC0100SCNInUse) TC0100SCNExit();
	if (TaitoIC_TC0110PCRInUse) TC0110PCRExit();
	if (TaitoIC_TC0140SYTInUse) TC0140SYTExit();
	if (TaitoIC_TC0150RODInUse) TC0150RODExit();
	if (TaitoIC_TC0180VCUInUse) TC0180VCUExit();
	if (TaitoIC_TC0220IOCInUse) TC0220IOCExit();
	if (TaitoIC_TC0280GRDInUse) TC0280GRDExit();
	if (TaitoIC_TC0360PRIInUse) TC0360PRIExit();
	if (TaitoIC_TC0430GRWInUse) TC0430GRWExit();
	if (TaitoIC_TC0480SCPInUse) TC0480SCPExit();
	if (TaitoIC_TC0510NIOInUse) TC0510NIOExit();
	if (TaitoIC_TC0640FIOInUse) TC0640FIOExit();	
	
	TaitoIC_SupermanCChipInUse = 0;
	TaitoIC_MegabCChipInUse = 0;
	TaitoIC_RainbowCChipInUse = 0;
	TaitoIC_OpwolfCChipInUse = 0;
	TaitoIC_VolfiedCChipInUse = 0;

	TaitoIC_PC080SNInUse = 0;
	TaitoIC_PC090OJInUse = 0;
	TaitoIC_TC0100SCNInUse = 0;
	TaitoIC_TC0110PCRInUse = 0;
	TaitoIC_TC0140SYTInUse = 0;
	TaitoIC_TC0150RODInUse = 0;
	TaitoIC_TC0180VCUInUse = 0;
	TaitoIC_TC0220IOCInUse = 0;
	TaitoIC_TC0280GRDInUse = 0;
	TaitoIC_TC0360PRIInUse = 0;
	TaitoIC_TC0430GRWInUse = 0;
	TaitoIC_TC0480SCPInUse = 0;
	TaitoIC_TC0510NIOInUse = 0;
	TaitoIC_TC0640FIOInUse = 0;

	TaitoWatchdog = 0;
}

void TaitoICScan(INT32 nAction)
{
	if (TaitoIC_SupermanCChipInUse) SupermanCChipScan(nAction);
	if (TaitoIC_MegabCChipInUse) MegabCChipScan(nAction);
	if (TaitoIC_RainbowCChipInUse) RainbowCChipScan(nAction);
	if (TaitoIC_OpwolfCChipInUse) OpwolfCChipScan(nAction);
	if (TaitoIC_VolfiedCChipInUse) VolfiedCChipScan(nAction);

	if (TaitoIC_PC080SNInUse) PC080SNScan(nAction);
	if (TaitoIC_PC090OJInUse) PC090OJScan(nAction);
	if (TaitoIC_TC0100SCNInUse) TC0100SCNScan(nAction);
	if (TaitoIC_TC0110PCRInUse) TC0110PCRScan(nAction);
	if (TaitoIC_TC0140SYTInUse) TC0140SYTScan(nAction);
	if (TaitoIC_TC0150RODInUse) TC0150RODScan(nAction);
	if (TaitoIC_TC0180VCUInUse) TC0180VCUScan(nAction);
	if (TaitoIC_TC0220IOCInUse) TC0220IOCScan(nAction);
	if (TaitoIC_TC0280GRDInUse) TC0280GRDScan(nAction);
	if (TaitoIC_TC0360PRIInUse) TC0360PRIScan(nAction);
	if (TaitoIC_TC0430GRWInUse) TC0430GRWScan(nAction);
	if (TaitoIC_TC0480SCPInUse) TC0480SCPScan(nAction);
	if (TaitoIC_TC0510NIOInUse) TC0510NIOScan(nAction);
	if (TaitoIC_TC0640FIOInUse) TC0640FIOScan(nAction);

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(TaitoWatchdog);
	}
}
