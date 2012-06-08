#include "system.h"

#include "c65c02.h"

void C65C02::Update(void)
{
		if(gSystemCPUSleep) return;
		if(gSystemIRQ && !mI && !mIRQActive)
		{
			// Push processor status
			PUSH(mPC>>8);
			PUSH(mPC&0xff);
			PUSH(PS()&0xef);		// Clear B flag on stack

			mI=TRUE;				// Stop further interrupts
			mD=FALSE;				// Clear decimal mode

			// Pick up the new PC
			mPC=CPU_PEEKW(IRQ_VECTOR);
		}
	// Fetch opcode
	mOpcode=CPU_PEEK(mPC);
	TRACE_CPU2("Update() PC=$%04x, Opcode=%02x",mPC,mOpcode);
	mPC++;

	// Execute Opcode

	switch(mOpcode)
	{
#define ADDCYC(x)	{ gSystemCycleCount += ((x) * 4); if(gSuzieDoneTime) gSuzieDoneTime += ((x) * 4); }
//
// 0x00
//
		case 0x00:
			ADDCYC(7);
			// IMPLIED
			xBRK();
			break;
		case 0x01:
			ADDCYC(6);
			xINDIRECT_X();
			xORA();
			break;
		case 0x04:
			ADDCYC(5);
			xZEROPAGE();
			xTSB();
			break;
		case 0x05:
			ADDCYC(3);
			xZEROPAGE();
			xORA();
			break;
		case 0x06:
			ADDCYC(5);
			xZEROPAGE();
			xASL();
			break;
		case 0x08:
			ADDCYC(3);
			// IMPLIED
			xPHP();
			break;
		case 0x09:
			ADDCYC(3);
			xIMMEDIATE();
			xORA();
			break;
		case 0x0A:
			ADDCYC(2);
			// IMPLIED
			xASLA();
			break;
		case 0x0C:
			ADDCYC(6);
			xABSOLUTE();
			xTSB();
			break;
		case 0x0D:
			ADDCYC(4);
			xABSOLUTE();
			xORA();
			break;
		case 0x0E:
			ADDCYC(6);
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xABSOLUTE();
			xASL();
			break;

//
// 0x10
//
		case 0x10:
			ADDCYC(2);
			// RELATIVE (IN FUNCTION)
			xBPL();
			break;
		case 0x11:
			ADDCYC(5);
			xINDIRECT_Y();
			xORA();
			break;
		case 0x12:
			ADDCYC(5);
			xINDIRECT();
			xORA();
			break;
		case 0x14:
			ADDCYC(5);
			xZEROPAGE();
			xTRB();
			break;
		case 0x15:
			ADDCYC(4);
			xZEROPAGE_X();
			xORA();
			break;
		case 0x16:
			ADDCYC(6);
			xZEROPAGE_X();
			xASL();
			break;
		case 0x18:
			ADDCYC(2);
			// IMPLIED
			xCLC();
			break;
		case 0x19:
			ADDCYC(4);
			xABSOLUTE_Y();
			xORA();
			break;
		case 0x1A:
			ADDCYC(2);
			// IMPLIED
			xINCA();
			break;
		case 0x1C:
			ADDCYC(6);
			xABSOLUTE();
			xTRB();
			break;
		case 0x1D:
			ADDCYC(4);
			xABSOLUTE_X();
			xORA();
			break;
		case 0x1E:
			ADDCYC(7);
			xABSOLUTE_X();
			xASL();
			break;

//
// 0x20
//
		case 0x20:
			ADDCYC(6);
			xABSOLUTE();
			xJSR();
			break;
		case 0x21:
			ADDCYC(6);
			xINDIRECT_X();
			xAND();
			break;
		case 0x24:
			ADDCYC(3);
			xZEROPAGE();
			xBIT();
			break;
		case 0x25:
			ADDCYC(3);
			xZEROPAGE();
			xAND();
			break;
		case 0x26:
			ADDCYC(5);
			xZEROPAGE();
			xROL();
			break;
		case 0x28:
			ADDCYC(4);
			// IMPLIED
			xPLP();
			break;
		case 0x29:
			ADDCYC(2);
			xIMMEDIATE();
			xAND();
			break;
		case 0x2A:
			ADDCYC(2);
			// IMPLIED
			xROLA();
			break;
		case 0x2C:
			ADDCYC(4);
			xABSOLUTE();
			xBIT();
			break;
		case 0x2D:
			ADDCYC(4);
			xABSOLUTE();
			xAND();
			break;
		case 0x2E:
			ADDCYC(6);
			xABSOLUTE();
			xROL();
			break;
//
// 0x30
//
		case 0x30:
			ADDCYC(2);
			// RELATIVE (IN FUNCTION)
			xBMI();
			break;
		case 0x31:
			ADDCYC(5);
			xINDIRECT_Y();
			xAND();
			break;
		case 0x32:
			ADDCYC(5);
			xINDIRECT();
			xAND();
			break;
		case 0x34:
			ADDCYC(4);
			xZEROPAGE_X();
			xBIT();
			break;
		case 0x35:
			ADDCYC(4);
			xZEROPAGE_X();
			xAND();
			break;
		case 0x36:
			ADDCYC(6);
			xZEROPAGE_X();
			xROL();
			break;
		case 0x38:
			ADDCYC(2);
			// IMPLIED
			xSEC();
			break;
		case 0x39:
			ADDCYC(4);
			xABSOLUTE_Y();
			xAND();
			break;
		case 0x3A:
			ADDCYC(2);
			// IMPLIED
			xDECA();
			break;
		case 0x3C:
			ADDCYC(4);
			xABSOLUTE_X();
			xBIT();
			break;
		case 0x3D:
			ADDCYC(4);
			xABSOLUTE_X();
			xAND();
			break;
		case 0x3E:
			ADDCYC(7);
			xABSOLUTE_X();
			xROL();
			break;
//
// 0x40
//
		case 0x40:
			ADDCYC(6);
			// IMPLIED
			xRTI();
			break;
		case 0x41:
			ADDCYC(6);
			xINDIRECT_X();
			xEOR();
			break;
		case 0x45:
			ADDCYC(3);
			xZEROPAGE();
			xEOR();
			break;
		case 0x46:
			ADDCYC(5);
			xZEROPAGE();
			xLSR();
			break;
		case 0x48:
			ADDCYC(3);
			// IMPLIED
			xPHA();
			break;
		case 0x49:
			ADDCYC(2);
			xIMMEDIATE();
			xEOR();
			break;
		case 0x4A:
			ADDCYC(2);
			// IMPLIED
			xLSRA();
			break;
		case 0x4C:
			ADDCYC(3);
			xABSOLUTE();
			xJMP();
			break;
		case 0x4D:
			ADDCYC(4);
			xABSOLUTE();
			xEOR();
			break;
		case 0x4E:
			ADDCYC(6);
			xABSOLUTE();
			xLSR();
			break;

//
// 0x50
//
		case 0x50:
			ADDCYC(2);
			// RELATIVE (IN FUNCTION)
			xBVC();
			break;
		case 0x51:
			ADDCYC(5);
			xINDIRECT_Y();
			xEOR();
			break;
		case 0x52:
			ADDCYC(5);
			xINDIRECT();
			xEOR();
			break;
		case 0x55:
			ADDCYC(4);
			xZEROPAGE_X();
			xEOR();
			break;
		case 0x56:
			ADDCYC(6);
			xZEROPAGE_X();
			xLSR();
			break;
		case 0x58:
			ADDCYC(2);
			// IMPLIED
			xCLI();
			break;
		case 0x59:
			ADDCYC(4);
			xABSOLUTE_Y();
			xEOR();
			break;
		case 0x5A:
			ADDCYC(3);
			// IMPLIED
			xPHY();
			break;
		case 0x5D:
			ADDCYC(4);
			xABSOLUTE_X();
			xEOR();
			break;
		case 0x5E:
			ADDCYC(7);
			xABSOLUTE_X();
			xLSR();
			break;

//
// 0x60
//
		case 0x60:
			ADDCYC(6);
			// IMPLIED
			xRTS();
			break;
		case 0x61:
			ADDCYC(6);
			xINDIRECT_X();
			xADC();
			break;
		case 0x64:
			ADDCYC(3);
			xZEROPAGE();
			xSTZ();
			break;
		case 0x65:
			ADDCYC(3);
			xZEROPAGE();
			xADC();
			break;
		case 0x66:
			ADDCYC(5);
			xZEROPAGE();
			xROR();
			break;
		case 0x68:
			ADDCYC(4);
			// IMPLIED
			xPLA();
			break;
		case 0x69:
			ADDCYC(2);
			xIMMEDIATE();
			xADC();
			break;
		case 0x6A:
			ADDCYC(2);
			// IMPLIED
			xRORA();
			break;
		case 0x6C:
			ADDCYC(6);
			xINDIRECT_ABSOLUTE();
			xJMP();
			break;
		case 0x6D:
			ADDCYC(4);
			xABSOLUTE();
			xADC();
			break;
		case 0x6E:
			ADDCYC(6);
			xABSOLUTE();
			xROR();
			break;
//
// 0x70
//
		case 0x70:
			ADDCYC(2);
			// RELATIVE (IN FUNCTION)
			xBVS();
			break;
		case 0x71:
			ADDCYC(5);
			xINDIRECT_Y();
			xADC();
			break;
		case 0x72:
			ADDCYC(5);
			xINDIRECT();
			xADC();
			break;
		case 0x74:
			ADDCYC(4);
			xZEROPAGE_X();
			xSTZ();
			break;
		case 0x75:
			ADDCYC(4);
			xZEROPAGE_X();
			xADC();
			break;
		case 0x76:
			ADDCYC(6);
			xZEROPAGE_X();
			xROR();
			break;
		case 0x78:
			ADDCYC(2);
			// IMPLIED
			xSEI();
			break;
		case 0x79:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_Y();
			xADC();
			break;
		case 0x7A:
			ADDCYC(4);
			// IMPLIED
			xPLY();
			break;
		case 0x7C:
			ADDCYC(6);
			xINDIRECT_ABSOLUTE_X();
			xJMP();
			break;
		case 0x7D:
			ADDCYC(4);
			xABSOLUTE_X();
			xADC();
			break;
		case 0x7E:
			ADDCYC(7);
			xABSOLUTE_X();
			xROR();
			break;
//
// 0x80
//
		case 0x80:
			ADDCYC(3);
			// RELATIVE (IN FUNCTION)
			xBRA();
			break;
		case 0x81:
			ADDCYC(6);
			xINDIRECT_X();
			xSTA();
			break;
		case 0x84:
			ADDCYC(3);
			xZEROPAGE();
			xSTY();
			break;
		case 0x85:
			ADDCYC(3);
			xZEROPAGE();
			xSTA();
			break;
		case 0x86:
			ADDCYC(3);
			xZEROPAGE();
			xSTX();
			break;
		case 0x88:
			ADDCYC(2);
			// IMPLIED
			xDEY();
			break;
		case 0x89:
			ADDCYC(3);
			xIMMEDIATE();
			xBIT();
			break;
		case 0x8A:
			ADDCYC(2);
			// IMPLIED
			xTXA();
			break;
		case 0x8C:
			ADDCYC(4);
			xABSOLUTE();
			xSTY();
			break;
		case 0x8D:
			ADDCYC(4);
			xABSOLUTE();
			xSTA();
			break;
		case 0x8E:
			ADDCYC(4);
			xABSOLUTE();
			xSTX();
			break;

//
// 0x90
//
		case 0x90:
			ADDCYC(2);
			// RELATIVE (IN FUNCTION)
			xBCC();
			break;
		case 0x91:
			ADDCYC(6);
			xINDIRECT_Y();
			xSTA();
			break;
		case 0x92:
			ADDCYC(5);
			xINDIRECT();
			xSTA();
			break;
		case 0x94:
			ADDCYC(4);
			xZEROPAGE_X();
			xSTY();
			break;
		case 0x95:
			ADDCYC(4);
			xZEROPAGE_X();
			xSTA();
			break;
		case 0x96:
			ADDCYC(4);
			xZEROPAGE_Y();
			xSTX();
			break;
		case 0x98:
			ADDCYC(2);
			// IMPLIED
			xTYA();
			break;
		case 0x99:
			ADDCYC(5);
			xABSOLUTE_Y();
			xSTA();
			break;
		case 0x9A:
			ADDCYC(2);
			// IMPLIED
			xTXS();
			break;
		case 0x9C:
			ADDCYC(4);
			xABSOLUTE();
			xSTZ();
			break;
		case 0x9D:
			ADDCYC(5);
			xABSOLUTE_X();
			xSTA();
			break;
		case 0x9E:
			ADDCYC(5);
			xABSOLUTE_X();
			xSTZ();
			break;

//
// 0xA0
//
		case 0xA0:
			ADDCYC(2);
			xIMMEDIATE();
			xLDY();
			break;
		case 0xA1:
			ADDCYC(6);
			xINDIRECT_X();
			xLDA();
			break;
		case 0xA2:
			ADDCYC(2);
			xIMMEDIATE();
			xLDX();
			break;
		case 0xA4:
			ADDCYC(3);
			xZEROPAGE();
			xLDY();
			break;
		case 0xA5:
			ADDCYC(3);
			xZEROPAGE();
			xLDA();
			break;
		case 0xA6:
			ADDCYC(3);
			xZEROPAGE();
			xLDX();
			break;
		case 0xA8:
			ADDCYC(2);
			// IMPLIED
			xTAY();
			break;
		case 0xA9:
			ADDCYC(2);
			xIMMEDIATE();
			xLDA();
			break;
		case 0xAA:
			ADDCYC(2);
			// IMPLIED
			xTAX();
			break;
		case 0xAC:
			ADDCYC(4);
			xABSOLUTE();
			xLDY();
			break;
		case 0xAD:
			ADDCYC(4);
			xABSOLUTE();
			xLDA();
			break;
		case 0xAE:
			ADDCYC(4);
			xABSOLUTE();
			xLDX();
			break;

//
// 0xB0
//
		case 0xB0:
			ADDCYC(2);
			// RELATIVE (IN FUNCTION)
			xBCS();
			break;
		case 0xB1:
			ADDCYC(5);
			xINDIRECT_Y();
			xLDA();
			break;
		case 0xB2:
			ADDCYC(5);
			xINDIRECT();
			xLDA();
			break;
		case 0xB4:
			ADDCYC(4);
			xZEROPAGE_X();
			xLDY();
			break;
		case 0xB5:
			ADDCYC(4);
			xZEROPAGE_X();
			xLDA();
			break;
		case 0xB6:
			ADDCYC(4);
			xZEROPAGE_Y();
			xLDX();
			break;
		case 0xB8:
			ADDCYC(2);
			// IMPLIED
			xCLV();
			break;
		case 0xB9:
			ADDCYC(4);
			xABSOLUTE_Y();
			xLDA();
			break;
		case 0xBA:
			ADDCYC(2);
			// IMPLIED
			xTSX();
			break;
		case 0xBC:
			ADDCYC(4);
			xABSOLUTE_X();
			xLDY();
			break;
		case 0xBD:
			ADDCYC(4);
			xABSOLUTE_X();
			xLDA();
			break;
		case 0xBE:
			ADDCYC(4);
			xABSOLUTE_Y();
			xLDX();
			break;

//
// 0xC0
//
		case 0xC0:
			ADDCYC(2);
			xIMMEDIATE();
			xCPY();
			break;
		case 0xC1:
			ADDCYC(6);
			xINDIRECT_X();
			xCMP();
			break;
		case 0xC4:
			ADDCYC(3);
			xZEROPAGE();
			xCPY();
			break;
		case 0xC5:
			ADDCYC(3);
			xZEROPAGE();
			xCMP();
			break;
		case 0xC6:
			ADDCYC(5);
			xZEROPAGE();
			xDEC();
			break;
		case 0xC8:
			ADDCYC(2);
			// IMPLIED
			xINY();
			break;
		case 0xC9:
			ADDCYC(2);
			xIMMEDIATE();
			xCMP();
			break;
		case 0xCA:
			ADDCYC(2);
			// IMPLIED
			xDEX();
			break;
		case 0xCB:
			ADDCYC(2);
			// IMPLIED
			xWAI();
			break;
		case 0xCC:
			ADDCYC(4);
			xABSOLUTE();
			xCPY();
			break;
		case 0xCD:
			ADDCYC(4);
			xABSOLUTE();
			xCMP();
			break;
		case 0xCE:
			ADDCYC(6);
			xABSOLUTE();
			xDEC();
			break;
//
// 0xD0
//
		case 0xD0:
			ADDCYC(2);
			// RELATIVE (IN FUNCTION)
			xBNE();
			break;
		case 0xD1:
			ADDCYC(5);			
			xINDIRECT_Y();
			xCMP();
			break;
		case 0xD2:
			ADDCYC(5);
			xINDIRECT();
			xCMP();
			break;
		case 0xD5:
			ADDCYC(4);
			xZEROPAGE_X();
			xCMP();
			break;
		case 0xD6:
			ADDCYC(6);
			xZEROPAGE_X();
			xDEC();
			break;
		case 0xD8:
			ADDCYC(2);
			// IMPLIED
			xCLD();
			break;
		case 0xD9:
			ADDCYC(4);
			xABSOLUTE_Y();
			xCMP();
			break;
		case 0xDA:
			ADDCYC(3);
			// IMPLIED
			xPHX();
			break;
		case 0xDB:
			ADDCYC(2);
			// IMPLIED
			xSTP();
			break;
		case 0xDD:
			ADDCYC(4);
			xABSOLUTE_X();
			xCMP();
			break;
		case 0xDE:
			ADDCYC(7);
			xABSOLUTE_X();
			xDEC();
			break;
//
// 0xE0
//
		case 0xE0:
			ADDCYC(2);
			xIMMEDIATE();
			xCPX();
			break;
		case 0xE1:
			ADDCYC(6);
			xINDIRECT_X();
			xSBC();
			break;
		case 0xE4:
			ADDCYC(3);
			xZEROPAGE();
			xCPX();
			break;
		case 0xE5:
			ADDCYC(3);
			xZEROPAGE();
			xSBC();
			break;
		case 0xE6:
			ADDCYC(5);
			xZEROPAGE();
			xINC();
			break;
		case 0xE8:
			ADDCYC(2);
			// IMPLIED
			xINX();
			break;
		case 0xE9:
			ADDCYC(2);
			xIMMEDIATE();
			xSBC();
			break;
		default:
		case 0xEA:
			ADDCYC(2);
			// IMPLIED
			xNOP();
			break;
		case 0xEC:
			ADDCYC(4);
			xABSOLUTE();
			xCPX();
			break;
		case 0xED:
			ADDCYC(4);
			xABSOLUTE();
			xSBC();
			break;
		case 0xEE:
			ADDCYC(6);
			xABSOLUTE();
			xINC();
			break;
//
// 0xF0
//
		case 0xF0:
			ADDCYC(2);
			// RELATIVE (IN FUNCTION)
			xBEQ();
			break;
		case 0xF1:
			ADDCYC(5);
			xINDIRECT_Y();
			xSBC();
			break;
		case 0xF2:
			ADDCYC(5);
			xINDIRECT();
			xSBC();
			break;
		case 0xF5:
			ADDCYC(4);
			xZEROPAGE_X();
			xSBC();
			break;
		case 0xF6:
			ADDCYC(6);
			xZEROPAGE_X();
			xINC();
			break;
		case 0xF8:
			ADDCYC(2);
			// IMPLIED
			xSED();
			break;
		case 0xF9:
			ADDCYC(4);
			xABSOLUTE_Y();
			xSBC();
			break;
		case 0xFA:
			ADDCYC(4);
			// IMPLIED
			xPLX();
			break;
		case 0xFD:
			ADDCYC(4);
			xABSOLUTE_X();
			xSBC();
			break;
		case 0xFE:
			ADDCYC(7);
			xABSOLUTE_X();
			xINC();
			break;
	}
}
