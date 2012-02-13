/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2011 CaH4e3
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef COPYFAMI

#include "__serial.h"
#include "mapinc.h"
#include "mmc3.h"

//#define DEBUG_SERIAL

// *** COPY FAMICOM EMULATION ***

/*
Карта памяти

	$0000-$1FFF Основное ОЗУ
	$2000-$200F Регистры PPU
	$2010-$3FFB Системное ОЗУ
	$3FFC-$3FFF Системные регистры
	$4000-$7FFF APU регистры/свободно
	$8000-$FFF9 CART/ROM
	$FFFA-$FFFB CART/Вектор NMI
	$FFFE-$FFFF CART/Вектор IRQ

Регистры

	CTRL		R/W	$3FFC	---aenic	($00 at reset)

		c - Режим картриджа
			0 - выключен
			1 - включен
		i - Режим IRQ вектора
			0 - оригинальный вектор
			1 - вектор перехвачен
		n - Режим NMI вектора
			0 - оригинальный вектор
			1 - вектор перехвачен
		e - Запрет NMI
			0 - запрещен
			1 - разрешен
		a - Режим AROM
			0 - выключен
			1 - включен

	BANK		R/W	$3FFD	---mbbbb

		b - Номер банка внутреннеого ПЗУ
		m - Мирроринг в режиме АROM

	USBDATA		R/W	$3FFE	dddddddd

		d - Байт данных приема/передачи

	STATUS		R	$3FFF	vmnicptr

   		r - Статус данных для чтения из USB
			0 - Есть данные
			1 - Нет данных
		t - Статус буфера для записи в USB
			0 - Есть место
			1 - Нет места
		p - Статус подключения USB кабеля
			0 - Подключен
			1 - Отключен
		c - Наличие картриджа в слоте
			0 - Присутствует
			1 - Отсутствует
		i - Состояние сигнала IRQ картриджа
			0 - Активен
			1 - Неактивен
		n - Состояние сигнала NMI картриджа
			0 - Активен
			1 - Неактивен
		m - Состояние адресной шины А10 VRAM (мирроринг)
		v - Состояние VRAM
			0 - Выбрана
			1 - Не выбрана

Режим AROM

	Активируется внутренняя VRAM
	Регистры банков и мирроринга на 8000-FFFF
*/

#define CTRL     0x00
#define  CCART   0x01
#define  CVIRQ   0x02
#define  CVNMI   0x04
#define  CDNMI   0x08
#define  CAROM   0x10
#define BANK     0x01
#define  BMIRR   0x10
#define USB      0x02
#define STATUS   0x03
#define  SRX     0x01
#define  STX     0x02
#define  SPEN    0x04
#define  SCART   0x08
#define  SIRQ    0x10
#define  SNMI    0x20
#define  SA10    0x40
#define  SVRAM   0x80

#ifdef DEBUG_SERIAL
static uint8 debug_serial_data[] = { 
       0xDE, 0xAD, 0xBE, 0xEF, 0x00,
       0xDE, 0xAD, 0xBE, 0xEF, 0x01,
      
       0x02,

       0x14, 0x50, 0xB0,
       
       0x02,

       0x14, 0x50, 0xB0,

       0x02,

       };
static uint32 debug_serial_data_size = sizeof(debug_serial_data);
static uint32 debug_serial_data_pos;
#endif

static uint8 *CHRRAM=NULL;
static uint32 CHRRAMSIZE;

static uint8 regs[4];

static readfunc def_read_ram, def_read_rom;
static writefunc def_write_ram;

static SFORMAT StateRegs[]=
{
  {regs, 4, "CREGS"},
  {0}
};

static void Sync()
{
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
}

static void MCopyFamiMMC3PW(uint32 A, uint8 V)
{
  if(regs[CTRL] & CCART)
    setprg8(A,V);
  else
    setprg32r(1,0x8000,(regs[BANK]&0x0F)^0x08);
}

static void MCopyFamiMMC3CW(uint32 A, uint8 V)
{
  if((regs[STATUS] & SCART) && (regs[CTRL] & CAROM))
    setchr8r(0x10,0);
  else
    setchr1r(0,A,V);
}

static void MCopyFamiMMC3MW(uint8 V)
{
  if(regs[CTRL] & CAROM)
  {
    setmirror(MI_0+((regs[BANK]>>4)&1));
  }
  else
  {
    A000B=V;
    setmirror((V&1)^1);
  }
}

static uint32 direction = 0xffffffff;
static uint32 bytes_count = 0;

static DECLFW(MCopyFamiWriteReg)
{
  if(((A&3) == USB))
  {
	  if(direction != 0) {
		  direction = 0;
		  bytes_count = 0;
		  FCEU_printf(" >");
	  }
#ifndef DEBUG_SERIAL
	  while (!SerialSendChar(V)) {};
#endif
	  bytes_count++;
	  //    FCEU_printf(" %02X",V);
  }
  else
  {
    regs[A&3]=V;
    Sync();
  }
}

static DECLFR(MCopyFamiReadReg)
{
#ifdef DEBUG_SERIAL
  if(debug_serial_data_pos == debug_serial_data_size)
    regs[STATUS] |= SRX;
  else
    regs[STATUS] &= ~SRX;
#endif
#ifndef DEBUG_SERIAL
    if((A&3) == STATUS)
    {
      int data;
      if((data = SerialGetChar()) == EOF)
        regs[STATUS] |= SRX;
      else
        regs[STATUS] &= ~SRX;
      regs[USB] = data & 0xff;
    } else
#endif
    if((A&3) == USB)
    {
#ifdef DEBUG_SERIAL
      regs[USB] = debug_serial_data[debug_serial_data_pos++];
#endif
      if(direction != 1) {
        if(direction != 0xffffffff) FCEU_printf(" bytes sent: %08x",bytes_count);
        direction = 1;
        bytes_count = 0;
        FCEU_printf("\n<");
      }
    FCEU_printf(" %02X",regs[USB]);
    }
  return regs[A&3];
}

static DECLFW(MCopyFamiMMC3Write)
{
  if(regs[CTRL] & CAROM)
  {
    regs[BANK] = V & 0x1F;
    Sync();
  }
  else
  {
    if(A >= 0xC000)
     MMC3_IRQWrite(A,V);
    else
     MMC3_CMDWrite(A,V);
  }
}

static DECLFW(MCopyFamiMMC3WriteNMI)
{
  if(regs[CTRL] & CVNMI)
    def_write_ram(0x3FFC + (A & 1), V);
  else
    MCopyFamiMMC3Write(A, V);
}

static DECLFW(MCopyFamiMMC3WriteIRQ)
{
  if(regs[CTRL] & CVIRQ)
    def_write_ram(0x3FFE + (A & 1), V);
  else
    MCopyFamiMMC3Write(A, V);
}

static DECLFR(MCopyFamiReadNMI)
{
  if(regs[CTRL] & CVNMI)
    return def_read_ram(0x3FFC + (A & 1));
  else
    return def_read_rom(A);
}

static DECLFR(MCopyFamiReadIRQ)
{
  if(regs[CTRL] & CVIRQ)
    return def_read_ram(0x3FFE + (A & 1));
  else
    return def_read_rom(A);
}

static void MCopyFamiMMC3Power(void)
{
  regs[CTRL] = regs[USB] = 0;
  regs[STATUS] = SIRQ | SNMI | SVRAM;
  regs[BANK] = 0x08;
#ifdef DEBUG_SERIAL
  debug_serial_data_pos = 0;
#endif
  GenMMC3Power();
  Sync();

  def_write_ram = GetWriteHandler(0x3FFC);
  SetWriteHandler(0x3FFC,0x3FFF,MCopyFamiWriteReg);
  def_read_ram = GetReadHandler(0x3FFC);
  SetReadHandler(0x3FFC,0x3FFF,MCopyFamiReadReg);

  SetWriteHandler(0x8000,0xFFF9,MCopyFamiMMC3Write);
  SetWriteHandler(0xFFFA,0xFFFB,MCopyFamiMMC3WriteNMI);
  SetWriteHandler(0xFFFE,0xFFFF,MCopyFamiMMC3WriteIRQ);

  def_read_rom = GetReadHandler(0xFFFA);
  SetReadHandler(0xFFFA,0xFFFB,MCopyFamiReadNMI);
  SetReadHandler(0xFFFE,0xFFFF,MCopyFamiReadIRQ);
}

static void MCopyFamiMMC3Reset(void)
{
  regs[CTRL] = regs[USB] = 0;
  regs[STATUS] = SIRQ | SNMI | SVRAM;
  regs[BANK] = 0x08;
#ifdef DEBUG_SERIAL
  debug_serial_data_pos = 0;
#endif
  MMC3RegReset();
  Sync();
}

static void MCopyFamiClose(void)
{
  if(CHRRAM)
    free(CHRRAM);
  CHRRAM=NULL;
  SerialClose();
}

static void StateRestore(int version)
{
  Sync();
}

void MapperCopyFamiMMC3_Init(CartInfo *info)
{
  GenMMC3_Init(info, 512, 512, 8, 0);

  cwrap=MCopyFamiMMC3CW;
  pwrap=MCopyFamiMMC3PW;
  mwrap=MCopyFamiMMC3MW;

  info->Reset=MCopyFamiMMC3Reset;
  info->Power=MCopyFamiMMC3Power;
  info->Close=MCopyFamiClose;
  GameStateRestore=StateRestore;

  CHRRAMSIZE=8192;
  CHRRAM=(uint8*)FCEU_gmalloc(CHRRAMSIZE);
  SetupCartCHRMapping(0x10,CHRRAM,CHRRAMSIZE,1);
  AddExState(CHRRAM, CHRRAMSIZE, 0, "CHRRAM");

#ifndef DEBUG_SERIAL
  FCEU_printf("WAITING FOR PORT...\n");
  
  while(!SerialOpen(20, 921600)) {}

  FCEU_printf("PORT READY.\n");
#endif

  AddExState(&StateRegs, ~0, 0, 0);
}

#endif
