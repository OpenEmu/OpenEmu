/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson 
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * DriveBoard.h
 *
 * Header for the CDriveBoard (force feedback emulation) class.
 */

#ifndef INCLUDED_DRIVEBOARD_H
#define INCLUDED_DRIVEBOARD_H

/*
 * CDriveBoardConfig:
 *
 * Settings used by CDriveBoard.
 */
class CDriveBoardConfig
{
public:
	bool	 forceFeedback;		// Enable drive board emulation/simulation (read only during Reset(), cannot be changed in-game)
	bool     simulateDrvBoard;  // Simulate drive board rather than emulating it
	unsigned steeringStrength;  // Setting for steering strength on DIP switches of drive board

	// Defaults
	CDriveBoardConfig(void)
	{
		forceFeedback = false;
		simulateDrvBoard = false;
		steeringStrength = 5;
	}
};

/*
 * CDriveBoard
 */
class CDriveBoard : public CBus
{
public:
	/*
	 * IsAttached(void):
	 *
	 * Returns:
	 *		True if the drive board is "attached" and should be emulated,
	 *		otherwise false.
	 */
	bool IsAttached(void);

	/*
	 * IsSimulated(void):
	 *
	 * Returns:
	 *		True if the drive board is being simulated rather than actually
	 *		emulated, otherwise false.
	 */
	bool IsSimulated(void);

	/*
	 * GetDIPSwitches(dip1, dip2):
	 *
	 * Reads the two sets of DIP switches on the drive board.
	 *
	 * Parameters:
	 *		dip1	Reference of variable to store DIP switch 1 to.
	 *		dip2	DIP switch 2.
	 */
	void GetDIPSwitches(UINT8 &dip1, UINT8 &dip2);

	/*
	 * SetDIPSwitches(dip1, dip2):
	 *
	 * Sets the DIP switches.
	 *
	 * Parameters:
	 *		dip1	DIP switch 1 value.
	 *		dip2	DIP switch 2 value.
	 */
	void SetDIPSwitches(UINT8 dip1, UINT8 dip2);

	/*
	 * GetSteeringStrength(void):
	 *
	 * Returns:
	 *		Strength of the steering based on drive board DIP switches (1-8).
	 */
	unsigned GetSteeringStrength(void);

	/*
	 * SetSteeringStrength(steeringStrength):
	 *
	 * Sets the steering strength (modifies the DIP switch setting).
	 *
	 * Parameters:
	 *		steeringStrength	A value ranging from 1 to 8.
	 */
	void SetSteeringStrength(unsigned steeringStrength);

	/*
	 * Get7SegDisplays(seg1Digit1, seg1Digit2, seg2Digit1, seg2Digit2):
	 *
	 * Reads the 7-segment displays.
	 *
	 * Parameters:
	 *		seg1Digit1	Reference of variable to store digit 1 of the first 7-
	 *					segment display to.
	 *		seg1Digit2	First display, second digit.
	 *		seg2Digit1	Second display, first digit.
	 *		seg2Digit2	Second display, second digit.
	 */
	void Get7SegDisplays(UINT8 &seg1Digit, UINT8 &seg1Digit2, UINT8 &seg2Digit1, UINT8 &seg2Digit2);

	/*
	 * GetZ80(void):
	 *
	 * Returns:
	 *		The Z80 object.
	 */
	CZ80 *GetZ80(void);

	/*
	 * SaveState(SaveState):
	 *
	 * Saves the drive board state.
	 *
	 * Parameters:
	 *		SaveState	Block file to save state information to.
	 */
	void SaveState(CBlockFile *SaveState);

	/*
	 * LoadState(SaveState):
	 *
	 * Restores the drive board state.
	 *
	 * Parameters:
	 *		SaveState	Block file to load save state information from.
	 */
	void LoadState(CBlockFile *SaveState);

	/*
	 * Init(romPtr):
	 *
	 * Initializes (and "attaches") the drive board. This should be called
	 * before other members.
	 *
	 * Parameters:
	 *		romPtr		Pointer to the drive board ROM (Z80 program). If this
	 *					is NULL, then the drive board will not be emulated.
	 *
	 * Returns:
	 *		FAIL if the drive board could not be initialized (prints own error
	 *		message), otherwise OKAY. If the drive board is not attached
	 *		because no ROM was passed to it, no error is generated and the
	 *		drive board is silently disabled (detached).
	 */
	bool Init(const UINT8 *romPtr);

	/*
	 * AttachInputs(InputsPtr, gameInputFlags):
	 *
	 * Attaches inputs to the drive board (for access to the steering wheel 
	 * position).
	 *
	 * Parameters:
	 *		InputsPtr		Pointer to the input object.
	 *		gameInputFlags	The current game's input flags.
	 */
	void AttachInputs(CInputs *InputsPtr, unsigned gameInputFlags);

	/*
	 * Reset(void):
	 *
	 * Resets the drive board.
	 */
	void Reset(void);

	/*
	 * Read():
	 *
	 * Reads data from the drive board.
	 *
	 * Returns:
	 *		Data read.
	 */
	UINT8 Read(void);

	/*
	 * Write(data):
	 *
	 * Writes data to the drive board.
	 *
	 * Parameters:
	 *		data	Data to send.
	 */
	void Write(UINT8 data);

	/*
	 * RunFrame(void):
	 *
	 * Emulates a single frame's worth of time on the drive board.
	 */
	void RunFrame(void);

	/*
	 * CDriveBoard():
	 * ~CDriveBoard():
	 *
	 * Constructor and destructor. Memory is freed by destructor.
	 */
	CDriveBoard();
	~CDriveBoard(void);

	/*
	 * Read8(addr):
	 * IORead8(portNum):
	 *
	 * Methods for reading from Z80's memory and IO space. Required by CBus.
	 *
	 * Parameters:
	 *		addr		Address in memory (0-0xFFFF).
	 *		portNum		Port address (0-255).
	 *
	 * Returns:
	 *		A byte of data from the address or port.
	 */
	UINT8 Read8(UINT32 addr);
	UINT8 IORead8(UINT32 portNum);
	
	/*
	 * Write8(addr, data):
	 * IORead8(portNum, data):
	 *
	 * Methods for writing to Z80's memory and IO space. Required by CBus.
	 *
	 * Parameters:
	 *		addr		Address in memory (0-0xFFFF).
	 *		portNum		Port address (0-255).
	 *		data		Byte to write.
	 */
	void Write8(UINT32 addr, UINT8 data);
	void IOWrite8(UINT32 portNum, UINT8 data);
	
private:
	bool m_attached;		// True if drive board is attached
	bool m_tmpDisabled;	    // True if temporarily disabled by loading an incompatible save state
	bool m_simulated;       // True if drive board should be simulated rather than emulated

	UINT8 m_dip1;		    // Value of DIP switch 1
	UINT8 m_dip2;	        // Value of DIP switch 2

	const UINT8* m_rom;	    // 32k ROM 
	UINT8* m_ram;           // 8k RAM

	CZ80 m_z80;             // Z80 CPU @ 4MHz

	CInputs *m_inputs;      
	unsigned m_inputFlags;
	
	// Emulation state
	bool m_initialized;     // True if drive board has finished initialization
	bool m_allowInterrupts; // True if drive board has enabled NMI interrupts

	UINT8 m_seg1Digit1;		// Current value of left digit on 7-segment display 1
	UINT8 m_seg1Digit2;		// Current value of right digit on 7-segment display 1
	UINT8 m_seg2Digit1;		// Current value of left digit on 7-segment display 2
	UINT8 m_seg2Digit2;     // Current value of right digit on 7-segment display 2

	UINT8 m_dataSent;       // Last command sent by main board
	UINT8 m_dataReceived;   // Data to send back to main board

	UINT16 m_adcPortRead;   // ADC port currently reading from
	UINT8 m_adcPortBit;     // Bit number currently reading on ADC port

	UINT8 m_port42Out;      // Last value sent to Z80 I/O port 42 (encoder motor data)
	UINT8 m_port46Out;      // Last value sent to Z80 I/O port 46 (encoder motor control)

	UINT8 m_prev42Out;      // Previous value sent to Z80 I/O port 42
	UINT8 m_prev46Out;      // Previous value sent to Z80 I/O port 46

	UINT8 m_uncenterVal1;   // First part of pending uncenter command
	UINT8 m_uncenterVal2;   // Second part of pending uncenter command

	// Simulation state
	UINT8 m_initState;
	UINT8 m_statusFlags;
	UINT8 m_boardMode;
	UINT8 m_readMode;
	UINT8 m_wheelCenter;
	UINT8 m_cockpitCenter;
	UINT8 m_echoVal;

	// Feedback state
	INT8 m_lastConstForce;  // Last constant force command sent
	UINT8 m_lastSelfCenter; // Last self center command sent
	UINT8 m_lastFriction;   // Last friction command sent
	UINT8 m_lastVibrate;    // Last vibrate command sent

	UINT8 SimulateRead(void);

	void SimulateWrite(UINT8 data);

	void SimulateFrame(void);

	void EmulateFrame(void);

	void ProcessEncoderCmd(void);

	void SendStopAll(void);

	void SendConstantForce(INT8 val);

	void SendSelfCenter(UINT8 val);

	void SendFriction(UINT8 val);

	void SendVibrate(UINT8 val);
};

#endif	// INCLUDED_DRIVEBOARD_H
