/*
  www.freedo.org
The first and only working 3DO multiplayer emulator.

The FreeDO licensed under modified GNU LGPL, with following notes:

*   The owners and original authors of the FreeDO have full right to develop closed source derivative work.
*   Any non-commercial uses of the FreeDO sources or any knowledge obtained by studying or reverse engineering
    of the sources, or any other material published by FreeDO have to be accompanied with full credits.
*   Any commercial uses of FreeDO sources or any knowledge obtained by studying or reverse engineering of the sources,
    or any other material published by FreeDO is strictly forbidden without owners approval.

The above notes are taking precedence over GNU LGPL in conflicting situations.

Project authors:

Alexander Troosh
Maxim Grishin
Allen Wright
John Sammons
Felix Lazarev
*/

#ifndef BITOPCLASS_DEFINITION_HEADER
#define BITOPCLASS_DEFINITION_HEADER

#include "types.h"

class BitReaderBig
{
protected:
	uint32 buf;
	uint32 point;
	int32 bitpoint;
	int32 bitset;
public:
	BitReaderBig()
    {
        buf=0;
        bitset=1;
        point=0;
		bitpoint=0;
    };
	BitReaderBig(uint32 buff)
	{
		buf=buff;
		point=0;
		bitpoint=0;
		bitset=1;
	};
    void AttachBuffer(uint32 buff)
	{
                buf=buff;
		point=0;
		bitpoint=0;
    };
	void SetBitRate(uint8 bits)
	{
		bitset=bits;
                if(bitset>32)bitset=32;
                if(!bitset)bitset=1;
	};
    
	void SetPosition(uint32 bytepos, uint8 bitpos)
	{
		point=bytepos;
		bitpoint=bitpos;
	};
	
    void SetPos(uint32 bitpos){SetPosition(bitpos>>3,bitpos&7);};

	uint32 GetBytePose(){return point;};
    
	uint32 Read();
    uint32 Read(uint8 bits);

	void Skip(uint32 bits)
	{
		bits+=bitpoint;
		point+=(bits>>3);
		bitpoint=bits&7;
	};
	
	
};




#endif
