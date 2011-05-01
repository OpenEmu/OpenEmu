#ifndef __MDFN_MD_CART_H
#define __MDFN_MD_CART_H

#include "../header.h"

void MDCart_Write8(uint32 address, uint8 value);
void MDCart_Write16(uint32 address, uint16 value);
uint8 MDCart_Read8(uint32 address);
uint16 MDCart_Read16(uint32 address);

void MDCart_Reset(void);

int MDCart_Load(md_game_info *ginfo, const char *name, MDFNFILE *fp);
bool MDCart_TestMagic(const char *name, MDFNFILE *fp);
bool MDCart_LoadNV(void);
bool MDCart_Close(void);

int MDCart_StateAction(StateMem *sm, int load, int data_only);

class MD_Cart_Type
{
        public:
	MD_Cart_Type()
	{

	}
	MD_Cart_Type(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size)
	{

	}
	virtual ~MD_Cart_Type()
	{

	}

	// Hard reset
	virtual void Reset(void)
	{

	}

	virtual void Write8(uint32 A, uint8 V)
	{
	}

        virtual void Write16(uint32 A, uint8 V)
	{
	}

	virtual uint8 Read8(uint32 A)
	{
	 return(0xFF);
	}

	virtual uint16 Read16(uint32 A)
	{
	 return(0xFF);
	}

        virtual int StateAction(StateMem *sm, int load, int data_only, const char *section_name)
	{
	 return(1);
	}

        // In bytes
        virtual uint32 GetNVMemorySize(void)
	{
	 return(0);
	}

        virtual void ReadNVMemory(uint8 *buffer)
	{
	}

        virtual void WriteNVMemory(const uint8 *buffer)
	{
	}
};


#endif
