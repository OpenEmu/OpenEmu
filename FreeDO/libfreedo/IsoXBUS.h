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

#ifndef	ISO_3DO_HEADER
#define ISO_3DO_HEADER 0x00000001

#define XBP_INIT		0	//plugin init, returns plugin version
#define XBP_RESET		1	//plugin reset with parameter(image path)
#define XBP_SET_COMMAND	2	//XBUS
#define XBP_FIQ			3	//check interrupt form device
#define XBP_SET_DATA	4	//XBUS
#define XBP_GET_DATA	5	//XBUS
#define XBP_GET_STATUS	6	//XBUS
#define XBP_SET_POLL	7	//XBUS
#define XBP_GET_POLL	8	//XBUS
#define XBP_SELECT		9   //selects device by Opera
#define XBP_RESERV		10  //reserved reading from device
#define XBP_DESTROY		11  //plugin destroy

#define XBP_GET_SAVESIZE	19	//save support from emulator side
#define XBP_GET_SAVEDATA	20
#define XBP_SET_SAVEDATA	21

#ifdef XBUS_EXPORTS
#define XBUS_API __declspec(dllexport)
#else
#define XBUS_API __declspec(dllimport)
#endif

/*extern "C"
{
XBUS_API void* _xbplug_MainDevice(int proc, void* data);

#ifdef XBUS_EXPORTS
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 );
#endif
};

#ifndef XBUS_EXPORTS
#pragma comment(lib, "IsoXBUS.lib")
#endif*/

#endif 
