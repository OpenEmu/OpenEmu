//
// Copyright (c) 2004 K. Wilkins
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                                 K. Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Generic lyynx definition header file                                     //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the definition of all of the useful hardware   //
// addreses within the Lynx.                                                //
//                                                                          //
//    K. Wilkins                                                            //
// August 1997                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 01Aug1997 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#define TMPADR			0xfc00 
#define TMPADRL			0xfc00 
#define TMPADRH			0xfc01 
#define TILTACUM		0xfc02 
#define TILTACUML		0xfc02 
#define TILTACUMH		0xfc03 
#define HOFF			0xfc04 
#define HOFFL			0xfc04 
#define HOFFH			0xfc05 
#define VOFF			0xfc06 
#define VOFFL			0xfc06 
#define VOFFH			0xfc07 
#define VIDBAS			0xfc08 
#define VIDBASL			0xfc08 
#define VIDBASH			0xfc09 
#define COLLBAS			0xfc0a 
#define COLLBASL		0xfc0a 
#define COLLBASH		0xfc0b 
#define VIDADR			0xfc0c 
#define VIDADRL			0xfc0c 
#define VIDADRH			0xfc0d 
#define COLLADR			0xfc0e 
#define COLLADRL		0xfc0e 
#define COLLADRH		0xfc0f 
#define SCBNEXT			0xfc10 
#define SCBNEXTL		0xfc10 
#define SCBNEXTH		0xfc11 
#define SPRDLINE		0xfc12 
#define SPRDLINEL		0xfc12 
#define SPRDLINEH		0xfc13 
#define HPOSSTRT		0xfc14 
#define HPOSSTRTL		0xfc14 
#define HPOSSTRTH		0xfc15 
#define VPOSSTRT		0xfc16 
#define VPOSSTRTL		0xfc16 
#define VPOSSTRTH		0xfc17 
#define SPRHSIZ			0xfc18 
#define SPRHSIZL		0xfc18 
#define SPRHSIZH		0xfc19 
#define SPRVSIZ			0xfc1a 
#define SPRVSIZL		0xfc1a 
#define SPRVSIZH		0xfc1b 
#define STRETCH			0xfc1c 
#define STRETCHL		0xfc1c 
#define STRETCHH		0xfc1d 
#define TILT			0xfc1e 
#define TILTL			0xfc1e 
#define TILTH			0xfc1f 
#define SPRDOFF			0xfc20 
#define SPRDOFFL		0xfc20 
#define SPRDOFFH		0xfc21 
#define SPRVPOS			0xfc22 
#define SPRVPOSL		0xfc22 
#define SPRVPOSH		0xfc23 
#define COLLOFF			0xfc24 
#define COLLOFFL		0xfc24 
#define COLLOFFH		0xfc25 
#define VSIZACUM		0xfc26 
#define VSIZACUML		0xfc26 
#define VSIZACUMH		0xfc27 
#define HSIZOFF			0xfc28 
#define HSIZOFFL		0xfc28 
#define HSIZOFFH		0xfc29 
#define VSIZOFF			0xfc2a 
#define VSIZOFFL		0xfc2a 
#define VSIZOFFH		0xfc2b 
#define SCBADR			0xfc2c 
#define SCBADRL			0xfc2c 
#define SCBADRH			0xfc2d 
#define PROCADR			0xfc2e 
#define PROCADRL		0xfc2e 
#define PROCADRH		0xfc2f 
#define MATHD			0xfc52 
#define MATHC			0xfc53 
#define MATHB			0xfc54 
#define MATHA			0xfc55 
#define MATHP			0xfc56 
#define MATHN			0xfc57 
#define MATHH			0xfc60 
#define MATHG			0xfc61 
#define MATHF			0xfc62 
#define MATHE			0xfc63 
#define MATHM			0xfc6c 
#define MATHL			0xfc6d 
#define MATHK			0xfc6e 
#define MATHJ			0xfc6f 
#define SPRCTL0			0xfc80 
#define SPRCTL1			0xfc81 
#define SPRCOLL			0xfc82 
#define SPRINIT			0xfc83 
#define SUZYHREV		0xfc88 
#define SUZYSREV		0xfc89 
#define SUZYBUSEN		0xfc90 
#define SPRGO			0xfc91 
#define SPRSYS			0xfc92 
#define JOYSTICK		0xfcb0 
#define SWITCHES		0xfcb1 
#define RCART0			0xfcb2 
#define RCART1			0xfcb3 
#define LEDS			0xfcc0 
#define PPORTSTAT		0xfcc2 
#define PPORTDATA		0xfcc3 
#define HOWIE			0xfcc4 
#define TIM0BKUP		0xfd00 
#define TIM0CTLA		0xfd01 
#define TIM0CNT			0xfd02 
#define TIM0CTLB		0xfd03 
#define TIM1BKUP		0xfd04 
#define TIM1CTLA		0xfd05 
#define TIM1CNT			0xfd06 
#define TIM1CTLB		0xfd07 
#define TIM2BKUP		0xfd08 
#define TIM2CTLA		0xfd09 
#define TIM2CNT			0xfd0a 
#define TIM2CTLB		0xfd0b 
#define TIM3BKUP		0xfd0c 
#define TIM3CTLA		0xfd0d 
#define TIM3CNT			0xfd0e 
#define TIM3CTLB		0xfd0f 
#define TIM4BKUP		0xfd10 
#define TIM4CTLA		0xfd11 
#define TIM4CNT			0xfd12 
#define TIM4CTLB		0xfd13 
#define TIM5BKUP		0xfd14 
#define TIM5CTLA		0xfd15 
#define TIM5CNT			0xfd16 
#define TIM5CTLB		0xfd17 
#define TIM6BKUP		0xfd18 
#define TIM6CTLA		0xfd19 
#define TIM6CNT			0xfd1a 
#define TIM6CTLB		0xfd1b 
#define TIM7BKUP		0xfd1c 
#define TIM7CTLA		0xfd1d 
#define TIM7CNT			0xfd1e 
#define TIM7CTLB		0xfd1f 

#define AUD0VOL			0xfd20 
#define AUD0SHFTFB		0xfd21 
#define AUD0OUTVAL		0xfd22 
#define AUD0L8SHFT		0xfd23 
#define AUD0TBACK		0xfd24 
#define AUD0CTL			0xfd25 
#define AUD0COUNT		0xfd26 
#define AUD0MISC		0xfd27 

#define AUD1VOL			0xfd28 
#define AUD1SHFTFB		0xfd29 
#define AUD1OUTVAL		0xfd2a 
#define AUD1L8SHFT		0xfd2b 
#define AUD1TBACK		0xfd2c 
#define AUD1CTL			0xfd2d 
#define AUD1COUNT		0xfd2e 
#define AUD1MISC		0xfd2f 

#define AUD2VOL			0xfd30 
#define AUD2SHFTFB		0xfd31 
#define AUD2OUTVAL		0xfd32 
#define AUD2L8SHFT		0xfd33 
#define AUD2TBACK		0xfd34 
#define AUD2CTL			0xfd35 
#define AUD2COUNT		0xfd36 
#define AUD2MISC		0xfd37 

#define AUD3VOL			0xfd38 
#define AUD3SHFTFB		0xfd39 
#define AUD3OUTVAL		0xfd3a 
#define AUD3L8SHFT		0xfd3b 
#define AUD3TBACK		0xfd3c 
#define AUD3CTL			0xfd3d 
#define AUD3COUNT		0xfd3e 
#define AUD3MISC		0xfd3f 

#define ATTEN_A         0xFD40	//
#define ATTEN_B         0xFD41
#define ATTEN_C         0xFD42  // Lynx2 Regs see macros/handy.equ
#define ATTEN_D         0xFD43
#define MPAN            0xFD44  //
#define MSTEREO			0xfd50 
#define INTRST			0xfd80 
#define INTSET			0xfd81 
#define MAGRDY0			0xfd84 
#define MAGRDY1			0xfd85 
#define AUDIN			0xfd86 
#define SYSCTL1			0xfd87 
#define MIKEYHREV		0xfd88 
#define MIKEYSREV		0xfd89 
#define IODIR			0xfd8a 
#define IODAT			0xfd8b 
#define SERCTL			0xfd8c 
#define SERDAT			0xfd8d 
#define SDONEACK		0xfd90 
#define CPUSLEEP		0xfd91 
#define DISPCTL			0xfd92 
#define PBKUP			0xfd93 
#define DISPADR			0xfd94 
#define DISPADRL		0xfd94 
#define DISPADRH		0xfd95 
#define Mtest0			0xfd9c 
#define Mtest1			0xfd9d 
#define Mtest2			0xfd9e 
#define GREEN0			0xfda0 
#define GREEN1			0xfda1 
#define GREEN2			0xfda2 
#define GREEN3			0xfda3 
#define GREEN4			0xfda4 
#define GREEN5			0xfda5 
#define GREEN6			0xfda6 
#define GREEN7			0xfda7 
#define GREEN8			0xfda8 
#define GREEN9			0xfda9 
#define GREENA			0xfdaa 
#define GREENB			0xfdab 
#define GREENC			0xfdac 
#define GREEND			0xfdad 
#define GREENE			0xfdae 
#define GREENF			0xfdaf 
#define BLUERED0		0xfdb0 
#define BLUERED1		0xfdb1 
#define BLUERED2		0xfdb2 
#define BLUERED3		0xfdb3 
#define BLUERED4		0xfdb4 
#define BLUERED5		0xfdb5 
#define BLUERED6		0xfdb6 
#define BLUERED7		0xfdb7 
#define BLUERED8		0xfdb8 
#define BLUERED9		0xfdb9 
#define BLUEREDA		0xfdba 
#define BLUEREDB		0xfdbb 
#define BLUEREDC		0xfdbc 
#define BLUEREDD		0xfdbd 
#define BLUEREDE		0xfdbe 
#define BLUEREDF		0xfdbf 
#define MMAPCTL			0xfff9 
#define CPUNMI			0xfffa 
#define CPUNMIL			0xfffa 
#define CPUNMIH			0xfffb 
#define CPURESET		0xfffc 
#define CPURESETL		0xfffc 
#define CPURESETH		0xfffd 
#define CPUINT			0xfffe 
#define CPUINTL			0xfffe 
#define CPUINTH			0xffff 

