/////////////////////////////////////////////////////////////////////////
// $Id: dis_tables.h,v 1.29 2006/04/27 15:11:45 sshwarts Exp $
/////////////////////////////////////////////////////////////////////////

#ifndef _BX_DISASM_TABLES_
#define _BX_DISASM_TABLES_

// opcode table attributes
#define _GROUPN        1
#define _SPLIT11B      2
#define _GRPFP         3
#define _GRPSSE        5
#define _GRPRM         6
#define _GRP3BOP       7
#define _GRP3BTAB      8

/* ************************************************************************ */
#define GRPSSE(n)       _GRPSSE,   BxDisasmGroupSSE_##n
#define GRPN(n)         _GROUPN,   BxDisasmGroup##n
#define GRPRM(n)        _GRPRM,    BxDisasmGroupRm##n
#define GRPMOD(n)       _SPLIT11B, BxDisasmGroupMod##n
#define GRPFP(n)        _GRPFP,    BxDisasmFPGroup##n
#define GR3BOP(n)       _GRP3BOP,  BxDisasm3ByteOp##n
#define GR3BTAB(n)      _GRP3BTAB, BxDisasm3ByteTable##n
/* ************************************************************************ */

#define Apw &disassembler::Apw
#define Apd &disassembler::Apd

#define  AL &disassembler::AL
#define  CL &disassembler::CL
#define  AX &disassembler::AX
#define  DX &disassembler::DX

#define EAX &disassembler::EAX
#define RAX &disassembler::RAX

#define  CS &disassembler::CS
#define  DS &disassembler::DS
#define  ES &disassembler::ES
#define  SS &disassembler::SS
#define  FS &disassembler::FS
#define  GS &disassembler::GS

#define  Sw &disassembler::Sw

#define  Td &disassembler::Td

#define  Cd &disassembler::Cd
#define  Cq &disassembler::Cq

#define  Dd &disassembler::Dd
#define  Dq &disassembler::Dq

#define  R8 &disassembler::R8
#define  RX &disassembler::RX

#define  Eb &disassembler::Eb
#define  Ew &disassembler::Ew
#define  Ed &disassembler::Ed
#define  Eq &disassembler::Eq

#define  Gb &disassembler::Gb
#define  Gw &disassembler::Gw
#define  Gd &disassembler::Gd
#define  Gq &disassembler::Gq

#define  I1 &disassembler::I1
#define  Ib &disassembler::Ib
#define  Iw &disassembler::Iw
#define  Id &disassembler::Id
#define  Iq &disassembler::Iq

#define IwIb &disassembler::IwIb

#define sIbw &disassembler::sIbw
#define sIbd &disassembler::sIbd
#define sIbq &disassembler::sIbq
#define sIdq &disassembler::sIdq

#define ST0 &disassembler::ST0
#define STi &disassembler::STi

#define  Rw &disassembler::Rw
#define  Rd &disassembler::Rd
#define  Rq &disassembler::Rq

#define  Pq &disassembler::Pq
#define  Qd &disassembler::Qd
#define  Qq &disassembler::Qq
#define  Nq &disassembler::Nq

#define  Vq &disassembler::Vq
#define Vdq &disassembler::Vdq
#define Vss &disassembler::Vss
#define Vsd &disassembler::Vsd
#define Vps &disassembler::Vps
#define Vpd &disassembler::Vpd
#define Udq &disassembler::Udq

#define  Wq &disassembler::Wq
#define Wdq &disassembler::Wdq
#define Wss &disassembler::Wss
#define Wsd &disassembler::Wsd
#define Wps &disassembler::Wps
#define Wpd &disassembler::Wpd

#define Ob  &disassembler::Ob
#define Ow  &disassembler::Ow
#define Od  &disassembler::Od
#define Oq  &disassembler::Oq

#define  Ma &disassembler::Ma
#define  Mp &disassembler::Mp
#define  Ms &disassembler::Ms
#define  Mx &disassembler::Mx
#define  Mb &disassembler::Mb
#define  Mw &disassembler::Mw
#define  Md &disassembler::Md
#define  Mq &disassembler::Mq
#define  Mt &disassembler::Mt
#define Mdq &disassembler::Mdq
#define Mps &disassembler::Mps
#define Mpd &disassembler::Mpd

#define  Xb &disassembler::Xb
#define  Xw &disassembler::Xw
#define  Xd &disassembler::Xd
#define  Xq &disassembler::Xq

#define  Yb &disassembler::Yb
#define  Yw &disassembler::Yw
#define  Yd &disassembler::Yd
#define  Yq &disassembler::Yq

#define  Jb &disassembler::Jb
#define  Jw &disassembler::Jw
#define  Jd &disassembler::Jd

#define  XX 0

const struct BxDisasmOpcodeInfo_t
#include "opcodes.inc"
#include "dis_tables.inc"

#undef XX

#endif
