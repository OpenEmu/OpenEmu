/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - assem_arm.c                                             *
 *   Copyright (C) 2009-2011 Ari64                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

extern int cycle_count;
extern int last_count;
extern int pcaddr;
extern int pending_exception;
extern int branch_target;
extern int ram_offset;
extern uint64_t readmem_dword;
extern precomp_instr fake_pc;
extern void *dynarec_local;
extern u_int memory_map[1048576];
extern u_int mini_ht[32][2];
extern u_int rounding_modes[4];

static u_int literals[1024][2];

void indirect_jump_indexed();
void indirect_jump();
void do_interrupt();
void jump_vaddr();
void jump_vaddr_r0();
void jump_vaddr_r1();
void jump_vaddr_r2();
void jump_vaddr_r3();
void jump_vaddr_r4();
void jump_vaddr_r5();
void jump_vaddr_r6();
void jump_vaddr_r7();
void jump_vaddr_r8();
void jump_vaddr_r9();
void jump_vaddr_r10();
void jump_vaddr_r12();

const u_int jump_vaddr_reg[16] = {
  (int)jump_vaddr_r0,
  (int)jump_vaddr_r1,
  (int)jump_vaddr_r2,
  (int)jump_vaddr_r3,
  (int)jump_vaddr_r4,
  (int)jump_vaddr_r5,
  (int)jump_vaddr_r6,
  (int)jump_vaddr_r7,
  (int)jump_vaddr_r8,
  (int)jump_vaddr_r9,
  (int)jump_vaddr_r10,
  0,
  (int)jump_vaddr_r12,
  0,
  0,
  0};

void invalidate_addr_r0();
void invalidate_addr_r1();
void invalidate_addr_r2();
void invalidate_addr_r3();
void invalidate_addr_r4();
void invalidate_addr_r5();
void invalidate_addr_r6();
void invalidate_addr_r7();
void invalidate_addr_r8();
void invalidate_addr_r9();
void invalidate_addr_r10();
void invalidate_addr_r12();

const u_int invalidate_addr_reg[16] = {
  (int)invalidate_addr_r0,
  (int)invalidate_addr_r1,
  (int)invalidate_addr_r2,
  (int)invalidate_addr_r3,
  (int)invalidate_addr_r4,
  (int)invalidate_addr_r5,
  (int)invalidate_addr_r6,
  (int)invalidate_addr_r7,
  (int)invalidate_addr_r8,
  (int)invalidate_addr_r9,
  (int)invalidate_addr_r10,
  0,
  (int)invalidate_addr_r12,
  0,
  0,
  0};

#include "../fpu.h"

static u_int jump_table_symbols[] = {
  (int)invalidate_addr,
  (int)jump_vaddr,
  (int)dyna_linker,
  (int)dyna_linker_ds,
  (int)verify_code,
  (int)verify_code_vm,
  (int)verify_code_ds,
  (int)cc_interrupt,
  (int)fp_exception,
  (int)fp_exception_ds,
  (int)jump_syscall,
  (int)jump_eret,
  (int)indirect_jump_indexed,
  (int)indirect_jump,
  (int)do_interrupt,
  (int)NULL /*MFC0*/,
  (int)NULL /*MTC0*/,
  (int)NULL /*TLBR*/,
  (int)NULL /*TLBP*/,
  (int)TLBWI_new,
  (int)TLBWR_new,
  (int)jump_vaddr_r0,
  (int)jump_vaddr_r1,
  (int)jump_vaddr_r2,
  (int)jump_vaddr_r3,
  (int)jump_vaddr_r4,
  (int)jump_vaddr_r5,
  (int)jump_vaddr_r6,
  (int)jump_vaddr_r7,
  (int)jump_vaddr_r8,
  (int)jump_vaddr_r9,
  (int)jump_vaddr_r10,
  (int)jump_vaddr_r12,
  (int)invalidate_addr_r0,
  (int)invalidate_addr_r1,
  (int)invalidate_addr_r2,
  (int)invalidate_addr_r3,
  (int)invalidate_addr_r4,
  (int)invalidate_addr_r5,
  (int)invalidate_addr_r6,
  (int)invalidate_addr_r7,
  (int)invalidate_addr_r8,
  (int)invalidate_addr_r9,
  (int)invalidate_addr_r10,
  (int)invalidate_addr_r12,
  (int)mult64,
  (int)multu64,
  (int)div64,
  (int)divu64,
  (int)cvt_s_w,
  (int)cvt_d_w,
  (int)cvt_s_l,
  (int)cvt_d_l,
  (int)cvt_w_s,
  (int)cvt_w_d,
  (int)cvt_l_s,
  (int)cvt_l_d,
  (int)cvt_d_s,
  (int)cvt_s_d,
  (int)round_l_s,
  (int)round_w_s,
  (int)trunc_l_s,
  (int)trunc_w_s,
  (int)ceil_l_s,
  (int)ceil_w_s,
  (int)floor_l_s,
  (int)floor_w_s,
  (int)round_l_d,
  (int)round_w_d,
  (int)trunc_l_d,
  (int)trunc_w_d,
  (int)ceil_l_d,
  (int)ceil_w_d,
  (int)floor_l_d,
  (int)floor_w_d,
  (int)c_f_s,
  (int)c_un_s,
  (int)c_eq_s,
  (int)c_ueq_s,
  (int)c_olt_s,
  (int)c_ult_s,
  (int)c_ole_s,
  (int)c_ule_s,
  (int)c_sf_s,
  (int)c_ngle_s,
  (int)c_seq_s,
  (int)c_ngl_s,
  (int)c_lt_s,
  (int)c_nge_s,
  (int)c_le_s,
  (int)c_ngt_s,
  (int)c_f_d,
  (int)c_un_d,
  (int)c_eq_d,
  (int)c_ueq_d,
  (int)c_olt_d,
  (int)c_ult_d,
  (int)c_ole_d,
  (int)c_ule_d,
  (int)c_sf_d,
  (int)c_ngle_d,
  (int)c_seq_d,
  (int)c_ngl_d,
  (int)c_lt_d,
  (int)c_nge_d,
  (int)c_le_d,
  (int)c_ngt_d,
  (int)add_s,
  (int)sub_s,
  (int)mul_s,
  (int)div_s,
  (int)sqrt_s,
  (int)abs_s,
  (int)mov_s,
  (int)neg_s,
  (int)add_d,
  (int)sub_d,
  (int)mul_d,
  (int)div_d,
  (int)sqrt_d,
  (int)abs_d,
  (int)mov_d,
  (int)neg_d
};

static unsigned int needs_clear_cache[1<<(TARGET_SIZE_2-17)];

#define JUMP_TABLE_SIZE (sizeof(jump_table_symbols)*2)

/* Linker */

static void set_jump_target(int addr,u_int target)
{
  u_char *ptr=(u_char *)addr;
  u_int *ptr2=(u_int *)ptr;
  if(ptr[3]==0xe2) {
    assert((target-(u_int)ptr2-8)<1024);
    assert((addr&3)==0);
    assert((target&3)==0);
    *ptr2=(*ptr2&0xFFFFF000)|((target-(u_int)ptr2-8)>>2)|0xF00;
    //DebugMessage(M64MSG_VERBOSE, "target=%x addr=%x insn=%x",target,addr,*ptr2);
  }
  else if(ptr[3]==0x72) {
    // generated by emit_jno_unlikely
    if((target-(u_int)ptr2-8)<1024) {
      assert((addr&3)==0);
      assert((target&3)==0);
      *ptr2=(*ptr2&0xFFFFF000)|((target-(u_int)ptr2-8)>>2)|0xF00;
    }
    else if((target-(u_int)ptr2-8)<4096&&!((target-(u_int)ptr2-8)&15)) {
      assert((addr&3)==0);
      assert((target&3)==0);
      *ptr2=(*ptr2&0xFFFFF000)|((target-(u_int)ptr2-8)>>4)|0xE00;
    }
    else *ptr2=(0x7A000000)|(((target-(u_int)ptr2-8)<<6)>>8);
  }
  else {
    assert((ptr[3]&0x0e)==0xa);
    *ptr2=(*ptr2&0xFF000000)|(((target-(u_int)ptr2-8)<<6)>>8);
  }
}

// This optionally copies the instruction from the target of the branch into
// the space before the branch.  Works, but the difference in speed is
// usually insignificant.
/*
static void set_jump_target_fillslot(int addr,u_int target,int copy)
{
  u_char *ptr=(u_char *)addr;
  u_int *ptr2=(u_int *)ptr;
  assert(!copy||ptr2[-1]==0xe28dd000);
  if(ptr[3]==0xe2) {
    assert(!copy);
    assert((target-(u_int)ptr2-8)<4096);
    *ptr2=(*ptr2&0xFFFFF000)|(target-(u_int)ptr2-8);
  }
  else {
    assert((ptr[3]&0x0e)==0xa);
    u_int target_insn=*(u_int *)target;
    if((target_insn&0x0e100000)==0) { // ALU, no immediate, no flags
      copy=0;
    }
    if((target_insn&0x0c100000)==0x04100000) { // Load
      copy=0;
    }
    if(target_insn&0x08000000) {
      copy=0;
    }
    if(copy) {
      ptr2[-1]=target_insn;
      target+=4;
    }
    *ptr2=(*ptr2&0xFF000000)|(((target-(u_int)ptr2-8)<<6)>>8);
  }
}
*/

/* Literal pool */
static void add_literal(int addr,int val)
{
  literals[literalcount][0]=addr;
  literals[literalcount][1]=val;
  literalcount++; 
} 

static void *kill_pointer(void *stub)
{
  int *ptr=(int *)(stub+4);
  assert((*ptr&0x0ff00000)==0x05900000);
  u_int offset=*ptr&0xfff;
  int **l_ptr=(void *)ptr+offset+8;
  int *i_ptr=*l_ptr;
  set_jump_target((int)i_ptr,(int)stub);
  return i_ptr;
}

static int get_pointer(void *stub)
{
  //DebugMessage(M64MSG_VERBOSE, "get_pointer(%x)",(int)stub);
  int *ptr=(int *)(stub+4);
  assert((*ptr&0x0ff00000)==0x05900000);
  u_int offset=*ptr&0xfff;
  int **l_ptr=(void *)ptr+offset+8;
  int *i_ptr=*l_ptr;
  assert((*i_ptr&0x0f000000)==0x0a000000);
  return (int)i_ptr+((*i_ptr<<8)>>6)+8;
}

// Find the "clean" entry point from a "dirty" entry point
// by skipping past the call to verify_code
static u_int get_clean_addr(int addr)
{
  int *ptr=(int *)addr;
  #ifdef ARMv5_ONLY
  ptr+=4;
  #else
  ptr+=6;
  #endif
  if((*ptr&0xFF000000)!=0xeb000000) ptr++;
  assert((*ptr&0xFF000000)==0xeb000000); // bl instruction
  ptr++;
  if((*ptr&0xFF000000)==0xea000000) {
    return (int)ptr+((*ptr<<8)>>6)+8; // follow jump
  }
  return (u_int)ptr;
}

static int verify_dirty(void *addr)
{
  u_int *ptr=(u_int *)addr;
  #ifdef ARMv5_ONLY
  // get from literal pool
  assert((*ptr&0xFFF00000)==0xe5900000);
  u_int offset=*ptr&0xfff;
  u_int *l_ptr=(void *)ptr+offset+8;
  u_int source=l_ptr[0];
  u_int copy=l_ptr[1];
  u_int len=l_ptr[2];
  ptr+=4;
  #else
  // ARMv7 movw/movt
  assert((*ptr&0xFFF00000)==0xe3000000);
  u_int source=(ptr[0]&0xFFF)+((ptr[0]>>4)&0xF000)+((ptr[2]<<16)&0xFFF0000)+((ptr[2]<<12)&0xF0000000);
  u_int copy=(ptr[1]&0xFFF)+((ptr[1]>>4)&0xF000)+((ptr[3]<<16)&0xFFF0000)+((ptr[3]<<12)&0xF0000000);
  u_int len=(ptr[4]&0xFFF)+((ptr[4]>>4)&0xF000);
  ptr+=6;
  #endif
  if((*ptr&0xFF000000)!=0xeb000000) ptr++;
  assert((*ptr&0xFF000000)==0xeb000000); // bl instruction
  u_int verifier=(int)ptr+((signed int)(*ptr<<8)>>6)+8; // get target of bl
  if(verifier==(u_int)verify_code_vm||verifier==(u_int)verify_code_ds) {
    unsigned int page=source>>12;
    unsigned int map_value=memory_map[page];
    if(map_value>=0x80000000) return 0;
    while(page<((source+len-1)>>12)) {
      if((memory_map[++page]<<2)!=(map_value<<2)) return 0;
    }
    source = source+(map_value<<2);
  }
  //DebugMessage(M64MSG_VERBOSE, "verify_dirty: %x %x %x",source,copy,len);
  return !memcmp((void *)source,(void *)copy,len);
}

// This doesn't necessarily find all clean entry points, just
// guarantees that it's not dirty
static int isclean(int addr)
{
  #ifdef ARMv5_ONLY
  int *ptr=((u_int *)addr)+4;
  #else
  int *ptr=((u_int *)addr)+6;
  #endif
  if((*ptr&0xFF000000)!=0xeb000000) ptr++;
  if((*ptr&0xFF000000)!=0xeb000000) return 1; // bl instruction
  if((int)ptr+((*ptr<<8)>>6)+8==(int)verify_code) return 0;
  if((int)ptr+((*ptr<<8)>>6)+8==(int)verify_code_vm) return 0;
  if((int)ptr+((*ptr<<8)>>6)+8==(int)verify_code_ds) return 0;
  return 1;
}

static void get_bounds(int addr,u_int *start,u_int *end)
{
  u_int *ptr=(u_int *)addr;
  #ifdef ARMv5_ONLY
  // get from literal pool
  assert((*ptr&0xFFF00000)==0xe5900000);
  u_int offset=*ptr&0xfff;
  u_int *l_ptr=(void *)ptr+offset+8;
  u_int source=l_ptr[0];
  //u_int copy=l_ptr[1];
  u_int len=l_ptr[2];
  ptr+=4;
  #else
  // ARMv7 movw/movt
  assert((*ptr&0xFFF00000)==0xe3000000);
  u_int source=(ptr[0]&0xFFF)+((ptr[0]>>4)&0xF000)+((ptr[2]<<16)&0xFFF0000)+((ptr[2]<<12)&0xF0000000);
  //u_int copy=(ptr[1]&0xFFF)+((ptr[1]>>4)&0xF000)+((ptr[3]<<16)&0xFFF0000)+((ptr[3]<<12)&0xF0000000);
  u_int len=(ptr[4]&0xFFF)+((ptr[4]>>4)&0xF000);
  ptr+=6;
  #endif
  if((*ptr&0xFF000000)!=0xeb000000) ptr++;
  assert((*ptr&0xFF000000)==0xeb000000); // bl instruction
  u_int verifier=(int)ptr+((signed int)(*ptr<<8)>>6)+8; // get target of bl
  if(verifier==(u_int)verify_code_vm||verifier==(u_int)verify_code_ds) {
    if(memory_map[source>>12]>=0x80000000) source = 0;
    else source = source+(memory_map[source>>12]<<2);
  }
  *start=source;
  *end=source+len;
}

/* Register allocation */

// Note: registers are allocated clean (unmodified state)
// if you intend to modify the register, you must call dirty_reg().
static void alloc_reg(struct regstat *cur,int i,signed char reg)
{
  int r,hr;
  int preferred_reg = (reg&7);
  if(reg==CCREG) preferred_reg=HOST_CCREG;
  if(reg==PTEMP||reg==FTEMP) preferred_reg=12;
  
  // Don't allocate unused registers
  if((cur->u>>reg)&1) return;
  
  // see if it's already allocated
  for(hr=0;hr<HOST_REGS;hr++)
  {
    if(cur->regmap[hr]==reg) return;
  }
  
  // Keep the same mapping if the register was already allocated in a loop
  preferred_reg = loop_reg(i,reg,preferred_reg);
  
  // Try to allocate the preferred register
  if(cur->regmap[preferred_reg]==-1) {
    cur->regmap[preferred_reg]=reg;
    cur->dirty&=~(1<<preferred_reg);
    cur->isconst&=~(1<<preferred_reg);
    return;
  }
  r=cur->regmap[preferred_reg];
  if(r<64&&((cur->u>>r)&1)) {
    cur->regmap[preferred_reg]=reg;
    cur->dirty&=~(1<<preferred_reg);
    cur->isconst&=~(1<<preferred_reg);
    return;
  }
  if(r>=64&&((cur->uu>>(r&63))&1)) {
    cur->regmap[preferred_reg]=reg;
    cur->dirty&=~(1<<preferred_reg);
    cur->isconst&=~(1<<preferred_reg);
    return;
  }
  
  // Clear any unneeded registers
  // We try to keep the mapping consistent, if possible, because it
  // makes branches easier (especially loops).  So we try to allocate
  // first (see above) before removing old mappings.  If this is not
  // possible then go ahead and clear out the registers that are no
  // longer needed.
  for(hr=0;hr<HOST_REGS;hr++)
  {
    r=cur->regmap[hr];
    if(r>=0) {
      if(r<64) {
        if((cur->u>>r)&1) {cur->regmap[hr]=-1;break;}
      }
      else
      {
        if((cur->uu>>(r&63))&1) {cur->regmap[hr]=-1;break;}
      }
    }
  }
  // Try to allocate any available register, but prefer
  // registers that have not been used recently.
  if(i>0) {
    for(hr=0;hr<HOST_REGS;hr++) {
      if(hr!=EXCLUDE_REG&&cur->regmap[hr]==-1) {
        if(regs[i-1].regmap[hr]!=rs1[i-1]&&regs[i-1].regmap[hr]!=rs2[i-1]&&regs[i-1].regmap[hr]!=rt1[i-1]&&regs[i-1].regmap[hr]!=rt2[i-1]) {
          cur->regmap[hr]=reg;
          cur->dirty&=~(1<<hr);
          cur->isconst&=~(1<<hr);
          return;
        }
      }
    }
  }
  // Try to allocate any available register
  for(hr=0;hr<HOST_REGS;hr++) {
    if(hr!=EXCLUDE_REG&&cur->regmap[hr]==-1) {
      cur->regmap[hr]=reg;
      cur->dirty&=~(1<<hr);
      cur->isconst&=~(1<<hr);
      return;
    }
  }
  
  // Ok, now we have to evict someone
  // Pick a register we hopefully won't need soon
  u_char hsn[MAXREG+1];
  memset(hsn,10,sizeof(hsn));
  int j;
  lsn(hsn,i,&preferred_reg);
  //DebugMessage(M64MSG_VERBOSE, "eax=%d ecx=%d edx=%d ebx=%d ebp=%d esi=%d edi=%d",cur->regmap[0],cur->regmap[1],cur->regmap[2],cur->regmap[3],cur->regmap[5],cur->regmap[6],cur->regmap[7]);
  //DebugMessage(M64MSG_VERBOSE, "hsn(%x): %d %d %d %d %d %d %d",start+i*4,hsn[cur->regmap[0]&63],hsn[cur->regmap[1]&63],hsn[cur->regmap[2]&63],hsn[cur->regmap[3]&63],hsn[cur->regmap[5]&63],hsn[cur->regmap[6]&63],hsn[cur->regmap[7]&63]);
  if(i>0) {
    // Don't evict the cycle count at entry points, otherwise the entry
    // stub will have to write it.
    if(bt[i]&&hsn[CCREG]>2) hsn[CCREG]=2;
    if(i>1&&hsn[CCREG]>2&&(itype[i-2]==RJUMP||itype[i-2]==UJUMP||itype[i-2]==CJUMP||itype[i-2]==SJUMP||itype[i-2]==FJUMP)) hsn[CCREG]=2;
    for(j=10;j>=3;j--)
    {
      // Alloc preferred register if available
      if(hsn[r=cur->regmap[preferred_reg]&63]==j) {
        for(hr=0;hr<HOST_REGS;hr++) {
          // Evict both parts of a 64-bit register
          if((cur->regmap[hr]&63)==r) {
            cur->regmap[hr]=-1;
            cur->dirty&=~(1<<hr);
            cur->isconst&=~(1<<hr);
          }
        }
        cur->regmap[preferred_reg]=reg;
        return;
      }
      for(r=1;r<=MAXREG;r++)
      {
        if(hsn[r]==j&&r!=rs1[i-1]&&r!=rs2[i-1]&&r!=rt1[i-1]&&r!=rt2[i-1]) {
          for(hr=0;hr<HOST_REGS;hr++) {
            if(hr!=HOST_CCREG||j<hsn[CCREG]) {
              if(cur->regmap[hr]==r+64) {
                cur->regmap[hr]=reg;
                cur->dirty&=~(1<<hr);
                cur->isconst&=~(1<<hr);
                return;
              }
            }
          }
          for(hr=0;hr<HOST_REGS;hr++) {
            if(hr!=HOST_CCREG||j<hsn[CCREG]) {
              if(cur->regmap[hr]==r) {
                cur->regmap[hr]=reg;
                cur->dirty&=~(1<<hr);
                cur->isconst&=~(1<<hr);
                return;
              }
            }
          }
        }
      }
    }
  }
  for(j=10;j>=0;j--)
  {
    for(r=1;r<=MAXREG;r++)
    {
      if(hsn[r]==j) {
        for(hr=0;hr<HOST_REGS;hr++) {
          if(cur->regmap[hr]==r+64) {
            cur->regmap[hr]=reg;
            cur->dirty&=~(1<<hr);
            cur->isconst&=~(1<<hr);
            return;
          }
        }
        for(hr=0;hr<HOST_REGS;hr++) {
          if(cur->regmap[hr]==r) {
            cur->regmap[hr]=reg;
            cur->dirty&=~(1<<hr);
            cur->isconst&=~(1<<hr);
            return;
          }
        }
      }
    }
  }
  DebugMessage(M64MSG_ERROR, "This shouldn't happen (alloc_reg)");exit(1);
}

static void alloc_reg64(struct regstat *cur,int i,signed char reg)
{
  int preferred_reg = 8+(reg&1);
  int r,hr;
  
  // allocate the lower 32 bits
  alloc_reg(cur,i,reg);
  
  // Don't allocate unused registers
  if((cur->uu>>reg)&1) return;
  
  // see if the upper half is already allocated
  for(hr=0;hr<HOST_REGS;hr++)
  {
    if(cur->regmap[hr]==reg+64) return;
  }
  
  // Keep the same mapping if the register was already allocated in a loop
  preferred_reg = loop_reg(i,reg,preferred_reg);
  
  // Try to allocate the preferred register
  if(cur->regmap[preferred_reg]==-1) {
    cur->regmap[preferred_reg]=reg|64;
    cur->dirty&=~(1<<preferred_reg);
    cur->isconst&=~(1<<preferred_reg);
    return;
  }
  r=cur->regmap[preferred_reg];
  if(r<64&&((cur->u>>r)&1)) {
    cur->regmap[preferred_reg]=reg|64;
    cur->dirty&=~(1<<preferred_reg);
    cur->isconst&=~(1<<preferred_reg);
    return;
  }
  if(r>=64&&((cur->uu>>(r&63))&1)) {
    cur->regmap[preferred_reg]=reg|64;
    cur->dirty&=~(1<<preferred_reg);
    cur->isconst&=~(1<<preferred_reg);
    return;
  }
  
  // Clear any unneeded registers
  // We try to keep the mapping consistent, if possible, because it
  // makes branches easier (especially loops).  So we try to allocate
  // first (see above) before removing old mappings.  If this is not
  // possible then go ahead and clear out the registers that are no
  // longer needed.
  for(hr=HOST_REGS-1;hr>=0;hr--)
  {
    r=cur->regmap[hr];
    if(r>=0) {
      if(r<64) {
        if((cur->u>>r)&1) {cur->regmap[hr]=-1;break;}
      }
      else
      {
        if((cur->uu>>(r&63))&1) {cur->regmap[hr]=-1;break;}
      }
    }
  }
  // Try to allocate any available register, but prefer
  // registers that have not been used recently.
  if(i>0) {
    for(hr=0;hr<HOST_REGS;hr++) {
      if(hr!=EXCLUDE_REG&&cur->regmap[hr]==-1) {
        if(regs[i-1].regmap[hr]!=rs1[i-1]&&regs[i-1].regmap[hr]!=rs2[i-1]&&regs[i-1].regmap[hr]!=rt1[i-1]&&regs[i-1].regmap[hr]!=rt2[i-1]) {
          cur->regmap[hr]=reg|64;
          cur->dirty&=~(1<<hr);
          cur->isconst&=~(1<<hr);
          return;
        }
      }
    }
  }
  // Try to allocate any available register
  for(hr=0;hr<HOST_REGS;hr++) {
    if(hr!=EXCLUDE_REG&&cur->regmap[hr]==-1) {
      cur->regmap[hr]=reg|64;
      cur->dirty&=~(1<<hr);
      cur->isconst&=~(1<<hr);
      return;
    }
  }
  
  // Ok, now we have to evict someone
  // Pick a register we hopefully won't need soon
  u_char hsn[MAXREG+1];
  memset(hsn,10,sizeof(hsn));
  int j;
  lsn(hsn,i,&preferred_reg);
  //DebugMessage(M64MSG_VERBOSE, "eax=%d ecx=%d edx=%d ebx=%d ebp=%d esi=%d edi=%d",cur->regmap[0],cur->regmap[1],cur->regmap[2],cur->regmap[3],cur->regmap[5],cur->regmap[6],cur->regmap[7]);
  //DebugMessage(M64MSG_VERBOSE, "hsn(%x): %d %d %d %d %d %d %d",start+i*4,hsn[cur->regmap[0]&63],hsn[cur->regmap[1]&63],hsn[cur->regmap[2]&63],hsn[cur->regmap[3]&63],hsn[cur->regmap[5]&63],hsn[cur->regmap[6]&63],hsn[cur->regmap[7]&63]);
  if(i>0) {
    // Don't evict the cycle count at entry points, otherwise the entry
    // stub will have to write it.
    if(bt[i]&&hsn[CCREG]>2) hsn[CCREG]=2;
    if(i>1&&hsn[CCREG]>2&&(itype[i-2]==RJUMP||itype[i-2]==UJUMP||itype[i-2]==CJUMP||itype[i-2]==SJUMP||itype[i-2]==FJUMP)) hsn[CCREG]=2;
    for(j=10;j>=3;j--)
    {
      // Alloc preferred register if available
      if(hsn[r=cur->regmap[preferred_reg]&63]==j) {
        for(hr=0;hr<HOST_REGS;hr++) {
          // Evict both parts of a 64-bit register
          if((cur->regmap[hr]&63)==r) {
            cur->regmap[hr]=-1;
            cur->dirty&=~(1<<hr);
            cur->isconst&=~(1<<hr);
          }
        }
        cur->regmap[preferred_reg]=reg|64;
        return;
      }
      for(r=1;r<=MAXREG;r++)
      {
        if(hsn[r]==j&&r!=rs1[i-1]&&r!=rs2[i-1]&&r!=rt1[i-1]&&r!=rt2[i-1]) {
          for(hr=0;hr<HOST_REGS;hr++) {
            if(hr!=HOST_CCREG||j<hsn[CCREG]) {
              if(cur->regmap[hr]==r+64) {
                cur->regmap[hr]=reg|64;
                cur->dirty&=~(1<<hr);
                cur->isconst&=~(1<<hr);
                return;
              }
            }
          }
          for(hr=0;hr<HOST_REGS;hr++) {
            if(hr!=HOST_CCREG||j<hsn[CCREG]) {
              if(cur->regmap[hr]==r) {
                cur->regmap[hr]=reg|64;
                cur->dirty&=~(1<<hr);
                cur->isconst&=~(1<<hr);
                return;
              }
            }
          }
        }
      }
    }
  }
  for(j=10;j>=0;j--)
  {
    for(r=1;r<=MAXREG;r++)
    {
      if(hsn[r]==j) {
        for(hr=0;hr<HOST_REGS;hr++) {
          if(cur->regmap[hr]==r+64) {
            cur->regmap[hr]=reg|64;
            cur->dirty&=~(1<<hr);
            cur->isconst&=~(1<<hr);
            return;
          }
        }
        for(hr=0;hr<HOST_REGS;hr++) {
          if(cur->regmap[hr]==r) {
            cur->regmap[hr]=reg|64;
            cur->dirty&=~(1<<hr);
            cur->isconst&=~(1<<hr);
            return;
          }
        }
      }
    }
  }
  DebugMessage(M64MSG_ERROR, "This shouldn't happen");exit(1);
}

// Allocate a temporary register.  This is done without regard to
// dirty status or whether the register we request is on the unneeded list
// Note: This will only allocate one register, even if called multiple times
static void alloc_reg_temp(struct regstat *cur,int i,signed char reg)
{
  int r,hr;
  int preferred_reg = -1;
  
  // see if it's already allocated
  for(hr=0;hr<HOST_REGS;hr++)
  {
    if(hr!=EXCLUDE_REG&&cur->regmap[hr]==reg) return;
  }
  
  // Try to allocate any available register
  for(hr=HOST_REGS-1;hr>=0;hr--) {
    if(hr!=EXCLUDE_REG&&cur->regmap[hr]==-1) {
      cur->regmap[hr]=reg;
      cur->dirty&=~(1<<hr);
      cur->isconst&=~(1<<hr);
      return;
    }
  }
  
  // Find an unneeded register
  for(hr=HOST_REGS-1;hr>=0;hr--)
  {
    r=cur->regmap[hr];
    if(r>=0) {
      if(r<64) {
        if((cur->u>>r)&1) {
          if(i==0||((unneeded_reg[i-1]>>r)&1)) {
            cur->regmap[hr]=reg;
            cur->dirty&=~(1<<hr);
            cur->isconst&=~(1<<hr);
            return;
          }
        }
      }
      else
      {
        if((cur->uu>>(r&63))&1) {
          if(i==0||((unneeded_reg_upper[i-1]>>(r&63))&1)) {
            cur->regmap[hr]=reg;
            cur->dirty&=~(1<<hr);
            cur->isconst&=~(1<<hr);
            return;
          }
        }
      }
    }
  }
  
  // Ok, now we have to evict someone
  // Pick a register we hopefully won't need soon
  // TODO: we might want to follow unconditional jumps here
  // TODO: get rid of dupe code and make this into a function
  u_char hsn[MAXREG+1];
  memset(hsn,10,sizeof(hsn));
  int j;
  lsn(hsn,i,&preferred_reg);
  //DebugMessage(M64MSG_VERBOSE, "hsn: %d %d %d %d %d %d %d",hsn[cur->regmap[0]&63],hsn[cur->regmap[1]&63],hsn[cur->regmap[2]&63],hsn[cur->regmap[3]&63],hsn[cur->regmap[5]&63],hsn[cur->regmap[6]&63],hsn[cur->regmap[7]&63]);
  if(i>0) {
    // Don't evict the cycle count at entry points, otherwise the entry
    // stub will have to write it.
    if(bt[i]&&hsn[CCREG]>2) hsn[CCREG]=2;
    if(i>1&&hsn[CCREG]>2&&(itype[i-2]==RJUMP||itype[i-2]==UJUMP||itype[i-2]==CJUMP||itype[i-2]==SJUMP||itype[i-2]==FJUMP)) hsn[CCREG]=2;
    for(j=10;j>=3;j--)
    {
      for(r=1;r<=MAXREG;r++)
      {
        if(hsn[r]==j&&r!=rs1[i-1]&&r!=rs2[i-1]&&r!=rt1[i-1]&&r!=rt2[i-1]) {
          for(hr=0;hr<HOST_REGS;hr++) {
            if(hr!=HOST_CCREG||hsn[CCREG]>2) {
              if(cur->regmap[hr]==r+64) {
                cur->regmap[hr]=reg;
                cur->dirty&=~(1<<hr);
                cur->isconst&=~(1<<hr);
                return;
              }
            }
          }
          for(hr=0;hr<HOST_REGS;hr++) {
            if(hr!=HOST_CCREG||hsn[CCREG]>2) {
              if(cur->regmap[hr]==r) {
                cur->regmap[hr]=reg;
                cur->dirty&=~(1<<hr);
                cur->isconst&=~(1<<hr);
                return;
              }
            }
          }
        }
      }
    }
  }
  for(j=10;j>=0;j--)
  {
    for(r=1;r<=MAXREG;r++)
    {
      if(hsn[r]==j) {
        for(hr=0;hr<HOST_REGS;hr++) {
          if(cur->regmap[hr]==r+64) {
            cur->regmap[hr]=reg;
            cur->dirty&=~(1<<hr);
            cur->isconst&=~(1<<hr);
            return;
          }
        }
        for(hr=0;hr<HOST_REGS;hr++) {
          if(cur->regmap[hr]==r) {
            cur->regmap[hr]=reg;
            cur->dirty&=~(1<<hr);
            cur->isconst&=~(1<<hr);
            return;
          }
        }
      }
    }
  }
  DebugMessage(M64MSG_ERROR, "This shouldn't happen");exit(1);
}
// Allocate a specific ARM register.
static void alloc_arm_reg(struct regstat *cur,int i,signed char reg,char hr)
{
  int n;
  int dirty=0;
  
  // see if it's already allocated (and dealloc it)
  for(n=0;n<HOST_REGS;n++)
  {
    if(n!=EXCLUDE_REG&&cur->regmap[n]==reg) {
      dirty=(cur->dirty>>n)&1;
      cur->regmap[n]=-1;
    }
  }
  
  cur->regmap[hr]=reg;
  cur->dirty&=~(1<<hr);
  cur->dirty|=dirty<<hr;
  cur->isconst&=~(1<<hr);
}

// Alloc cycle count into dedicated register
static void alloc_cc(struct regstat *cur,int i)
{
  alloc_arm_reg(cur,i,CCREG,HOST_CCREG);
}

/* Special alloc */


/* Assembler */

static char regname[16][4] = {
 "r0",
 "r1",
 "r2",
 "r3",
 "r4",
 "r5",
 "r6",
 "r7",
 "r8",
 "r9",
 "r10",
 "fp",
 "r12",
 "sp",
 "lr",
 "pc"};

static void output_byte(u_char byte)
{
  *(out++)=byte;
}
static void output_modrm(u_char mod,u_char rm,u_char ext)
{
  assert(mod<4);
  assert(rm<8);
  assert(ext<8);
  u_char byte=(mod<<6)|(ext<<3)|rm;
  *(out++)=byte;
}

static void output_w32(u_int word)
{
  *((u_int *)out)=word;
  out+=4;
}
static u_int rd_rn_rm(u_int rd, u_int rn, u_int rm)
{
  assert(rd<16);
  assert(rn<16);
  assert(rm<16);
  return((rn<<16)|(rd<<12)|rm);
}
static u_int rd_rn_imm_shift(u_int rd, u_int rn, u_int imm, u_int shift)
{
  assert(rd<16);
  assert(rn<16);
  assert(imm<256);
  assert((shift&1)==0);
  return((rn<<16)|(rd<<12)|(((32-shift)&30)<<7)|imm);
}
static u_int genimm(u_int imm,u_int *encoded)
{
  if(imm==0) {*encoded=0;return 1;}
  int i=32;
  while(i>0)
  {
    if(imm<256) {
      *encoded=((i&30)<<7)|imm;
      return 1;
    }
    imm=(imm>>2)|(imm<<30);i-=2;
  }
  return 0;
}
static u_int genjmp(u_int addr)
{
  if(addr<4) return 0;
  int offset=addr-(int)out-8;
  if(offset<-33554432||offset>=33554432) {
    int n;
    for (n=0;n<sizeof(jump_table_symbols)/4;n++)
    {
      if(addr==jump_table_symbols[n])
      {
        offset=BASE_ADDR+(1<<TARGET_SIZE_2)-JUMP_TABLE_SIZE+n*8-(int)out-8;
        break;
      }
    }
  }
  assert(offset>=-33554432&&offset<33554432);
  return ((u_int)offset>>2)&0xffffff;
}

static void emit_mov(int rs,int rt)
{
  assem_debug("mov %s,%s",regname[rt],regname[rs]);
  output_w32(0xe1a00000|rd_rn_rm(rt,0,rs));
}

static void emit_movs(int rs,int rt)
{
  assem_debug("movs %s,%s",regname[rt],regname[rs]);
  output_w32(0xe1b00000|rd_rn_rm(rt,0,rs));
}

static void emit_add(int rs1,int rs2,int rt)
{
  assem_debug("add %s,%s,%s",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe0800000|rd_rn_rm(rt,rs1,rs2));
}

static void emit_adds(int rs1,int rs2,int rt)
{
  assem_debug("adds %s,%s,%s",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe0900000|rd_rn_rm(rt,rs1,rs2));
}

static void emit_adcs(int rs1,int rs2,int rt)
{
  assem_debug("adcs %s,%s,%s",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe0b00000|rd_rn_rm(rt,rs1,rs2));
}

static void emit_sbc(int rs1,int rs2,int rt)
{
  assem_debug("sbc %s,%s,%s",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe0c00000|rd_rn_rm(rt,rs1,rs2));
}

static void emit_sbcs(int rs1,int rs2,int rt)
{
  assem_debug("sbcs %s,%s,%s",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe0d00000|rd_rn_rm(rt,rs1,rs2));
}

static void emit_neg(int rs, int rt)
{
  assem_debug("rsb %s,%s,#0",regname[rt],regname[rs]);
  output_w32(0xe2600000|rd_rn_rm(rt,rs,0));
}

static void emit_negs(int rs, int rt)
{
  assem_debug("rsbs %s,%s,#0",regname[rt],regname[rs]);
  output_w32(0xe2700000|rd_rn_rm(rt,rs,0));
}

static void emit_sub(int rs1,int rs2,int rt)
{
  assem_debug("sub %s,%s,%s",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe0400000|rd_rn_rm(rt,rs1,rs2));
}

static void emit_subs(int rs1,int rs2,int rt)
{
  assem_debug("subs %s,%s,%s",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe0500000|rd_rn_rm(rt,rs1,rs2));
}

static void emit_zeroreg(int rt)
{
  assem_debug("mov %s,#0",regname[rt]);
  output_w32(0xe3a00000|rd_rn_rm(rt,0,0));
}

static void emit_loadlp(u_int imm,u_int rt)
{
  add_literal((int)out,imm);
  assem_debug("ldr %s,pc+? [=%x]",regname[rt],imm);
  output_w32(0xe5900000|rd_rn_rm(rt,15,0));
}
static void emit_movw(u_int imm,u_int rt)
{
  assert(imm<65536);
  assem_debug("movw %s,#%d (0x%x)",regname[rt],imm,imm);
  output_w32(0xe3000000|rd_rn_rm(rt,0,0)|(imm&0xfff)|((imm<<4)&0xf0000));
}
static void emit_movt(u_int imm,u_int rt)
{
  assem_debug("movt %s,#%d (0x%x)",regname[rt],imm&0xffff0000,imm&0xffff0000);
  output_w32(0xe3400000|rd_rn_rm(rt,0,0)|((imm>>16)&0xfff)|((imm>>12)&0xf0000));
}
static void emit_movimm(u_int imm,u_int rt)
{
  u_int armval;
  if(genimm(imm,&armval)) {
    assem_debug("mov %s,#%d",regname[rt],imm);
    output_w32(0xe3a00000|rd_rn_rm(rt,0,0)|armval);
  }else if(genimm(~imm,&armval)) {
    assem_debug("mvn %s,#%d",regname[rt],imm);
    output_w32(0xe3e00000|rd_rn_rm(rt,0,0)|armval);
  }else if(imm<65536) {
    #ifdef ARMv5_ONLY
    assem_debug("mov %s,#%d",regname[rt],imm&0xFF00);
    output_w32(0xe3a00000|rd_rn_imm_shift(rt,0,imm>>8,8));
    assem_debug("add %s,%s,#%d",regname[rt],regname[rt],imm&0xFF);
    output_w32(0xe2800000|rd_rn_imm_shift(rt,rt,imm&0xff,0));
    #else
    emit_movw(imm,rt);
    #endif
  }else{
    #ifdef ARMv5_ONLY
    emit_loadlp(imm,rt);
    #else
    emit_movw(imm&0x0000FFFF,rt);
    emit_movt(imm&0xFFFF0000,rt);
    #endif
  }
}
static void emit_pcreladdr(u_int rt)
{
  assem_debug("add %s,pc,#?",regname[rt]);
  output_w32(0xe2800000|rd_rn_rm(rt,15,0));
}

static void emit_loadreg(int r, int hr)
{
  if((r&63)==0)
    emit_zeroreg(hr);
  else if(r==MMREG)
    emit_movimm(((int)memory_map-(int)&dynarec_local)>>2,hr);
  else {
    int addr=((int)reg)+((r&63)<<3)+((r&64)>>4);
    if((r&63)==HIREG) addr=(int)&hi+((r&64)>>4);
    if((r&63)==LOREG) addr=(int)&lo+((r&64)>>4);
    if(r==CCREG) addr=(int)&cycle_count;
    if(r==CSREG) addr=(int)&Status;
    if(r==FSREG) addr=(int)&FCR31;
    if(r==INVCP) addr=(int)&invc_ptr;
    if(r==ROREG) addr=(int)&ram_offset;
    u_int offset = addr-(u_int)&dynarec_local;
    assert(offset<4096);
    assem_debug("ldr %s,fp+%d",regname[hr],offset);
    output_w32(0xe5900000|rd_rn_rm(hr,FP,0)|offset);
  }
}
static void emit_storereg(int r, int hr)
{
  int addr=((int)reg)+((r&63)<<3)+((r&64)>>4);
  if((r&63)==HIREG) addr=(int)&hi+((r&64)>>4);
  if((r&63)==LOREG) addr=(int)&lo+((r&64)>>4);
  if(r==CCREG) addr=(int)&cycle_count;
  if(r==FSREG) addr=(int)&FCR31;
  u_int offset = addr-(u_int)&dynarec_local;
  assert(offset<4096);
  assem_debug("str %s,fp+%d",regname[hr],offset);
  output_w32(0xe5800000|rd_rn_rm(hr,FP,0)|offset);
}

static void emit_test(int rs, int rt)
{
  assem_debug("tst %s,%s",regname[rs],regname[rt]);
  output_w32(0xe1100000|rd_rn_rm(0,rs,rt));
}

static void emit_testimm(int rs,int imm)
{
  u_int armval, ret;
  assem_debug("tst %s,#%d",regname[rs],imm);
  ret = genimm(imm,&armval);
  assert(ret);
  output_w32(0xe3100000|rd_rn_rm(0,rs,0)|armval);
}

static void emit_not(int rs,int rt)
{
  assem_debug("mvn %s,%s",regname[rt],regname[rs]);
  output_w32(0xe1e00000|rd_rn_rm(rt,0,rs));
}

static void emit_and(u_int rs1,u_int rs2,u_int rt)
{
  assem_debug("and %s,%s,%s",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe0000000|rd_rn_rm(rt,rs1,rs2));
}

static void emit_or(u_int rs1,u_int rs2,u_int rt)
{
  assem_debug("orr %s,%s,%s",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe1800000|rd_rn_rm(rt,rs1,rs2));
}
static void emit_or_and_set_flags(int rs1,int rs2,int rt)
{
  assem_debug("orrs %s,%s,%s",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe1900000|rd_rn_rm(rt,rs1,rs2));
}

static void emit_xor(u_int rs1,u_int rs2,u_int rt)
{
  assem_debug("eor %s,%s,%s",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe0200000|rd_rn_rm(rt,rs1,rs2));
}

static void emit_addimm(u_int rs,int imm,u_int rt)
{
  assert(rs<16);
  assert(rt<16);
  if(imm!=0) {
    assert(imm>-65536&&imm<65536);
    u_int armval;
    if(genimm(imm,&armval)) {
      assem_debug("add %s,%s,#%d",regname[rt],regname[rs],imm);
      output_w32(0xe2800000|rd_rn_rm(rt,rs,0)|armval);
    }else if(genimm(-imm,&armval)) {
      assem_debug("sub %s,%s,#%d",regname[rt],regname[rs],imm);
      output_w32(0xe2400000|rd_rn_rm(rt,rs,0)|armval);
    }else if(imm<0) {
      assem_debug("sub %s,%s,#%d",regname[rt],regname[rs],(-imm)&0xFF00);
      assem_debug("sub %s,%s,#%d",regname[rt],regname[rt],(-imm)&0xFF);
      output_w32(0xe2400000|rd_rn_imm_shift(rt,rs,(-imm)>>8,8));
      output_w32(0xe2400000|rd_rn_imm_shift(rt,rt,(-imm)&0xff,0));
    }else{
      assem_debug("add %s,%s,#%d",regname[rt],regname[rs],imm&0xFF00);
      assem_debug("add %s,%s,#%d",regname[rt],regname[rt],imm&0xFF);
      output_w32(0xe2800000|rd_rn_imm_shift(rt,rs,imm>>8,8));
      output_w32(0xe2800000|rd_rn_imm_shift(rt,rt,imm&0xff,0));
    }
  }
  else if(rs!=rt) emit_mov(rs,rt);
}

static void emit_addimm_and_set_flags(int imm,int rt)
{
  assert(imm>-65536&&imm<65536);
  u_int armval;
  if(genimm(imm,&armval)) {
    assem_debug("adds %s,%s,#%d",regname[rt],regname[rt],imm);
    output_w32(0xe2900000|rd_rn_rm(rt,rt,0)|armval);
  }else if(genimm(-imm,&armval)) {
    assem_debug("subs %s,%s,#%d",regname[rt],regname[rt],imm);
    output_w32(0xe2500000|rd_rn_rm(rt,rt,0)|armval);
  }else if(imm<0) {
    assem_debug("sub %s,%s,#%d",regname[rt],regname[rt],(-imm)&0xFF00);
    assem_debug("subs %s,%s,#%d",regname[rt],regname[rt],(-imm)&0xFF);
    output_w32(0xe2400000|rd_rn_imm_shift(rt,rt,(-imm)>>8,8));
    output_w32(0xe2500000|rd_rn_imm_shift(rt,rt,(-imm)&0xff,0));
  }else{
    assem_debug("add %s,%s,#%d",regname[rt],regname[rt],imm&0xFF00);
    assem_debug("adds %s,%s,#%d",regname[rt],regname[rt],imm&0xFF);
    output_w32(0xe2800000|rd_rn_imm_shift(rt,rt,imm>>8,8));
    output_w32(0xe2900000|rd_rn_imm_shift(rt,rt,imm&0xff,0));
  }
}

#ifndef RAM_OFFSET
static void emit_addimm_no_flags(u_int imm,u_int rt)
{
  emit_addimm(rt,imm,rt);
}
#endif

static void emit_addnop(u_int r)
{
  assert(r<16);
  assem_debug("add %s,%s,#0 (nop)",regname[r],regname[r]);
  output_w32(0xe2800000|rd_rn_rm(r,r,0));
}

static void emit_adcimm(u_int rs,int imm,u_int rt)
{
  u_int armval, ret;
  ret = genimm(imm,&armval);
  assert(ret);
  assem_debug("adc %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0xe2a00000|rd_rn_rm(rt,rs,0)|armval);
}
/*static void emit_sbcimm(int imm,u_int rt)
{
  u_int armval, ret;
  ret = genimm(imm,&armval);
  assert(ret);
  assem_debug("sbc %s,%s,#%d",regname[rt],regname[rt],imm);
  output_w32(0xe2c00000|rd_rn_rm(rt,rt,0)|armval);
}*/

static void emit_rscimm(int rs,int imm,u_int rt)
{
  assert(0);
  u_int armval, ret;
  ret = genimm(imm,&armval);
  assert(ret);
  assem_debug("rsc %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0xe2e00000|rd_rn_rm(rt,rs,0)|armval);
}

static void emit_addimm64_32(int rsh,int rsl,int imm,int rth,int rtl)
{
  // TODO: if(genimm(imm,&armval)) ...
  // else
  emit_movimm(imm,HOST_TEMPREG);
  emit_adds(HOST_TEMPREG,rsl,rtl);
  emit_adcimm(rsh,0,rth);
}
#ifdef INVERTED_CARRY
static void emit_sbb(int rs1,int rs2)
{
  assem_debug("sbb %%%s,%%%s",regname[rs2],regname[rs1]);
  output_byte(0x19);
  output_modrm(3,rs1,rs2);
}
#endif

static void emit_andimm(int rs,int imm,int rt)
{
  u_int armval;
  if(imm==0) {
    emit_zeroreg(rt);
  }else if(genimm(imm,&armval)) {
    assem_debug("and %s,%s,#%d",regname[rt],regname[rs],imm);
    output_w32(0xe2000000|rd_rn_rm(rt,rs,0)|armval);
  }else if(genimm(~imm,&armval)) {
    assem_debug("bic %s,%s,#%d",regname[rt],regname[rs],imm);
    output_w32(0xe3c00000|rd_rn_rm(rt,rs,0)|armval);
  }else if(imm==65535) {
    #ifdef ARMv5_ONLY
    assem_debug("bic %s,%s,#FF000000",regname[rt],regname[rs]);
    output_w32(0xe3c00000|rd_rn_rm(rt,rs,0)|0x4FF);
    assem_debug("bic %s,%s,#00FF0000",regname[rt],regname[rt]);
    output_w32(0xe3c00000|rd_rn_rm(rt,rt,0)|0x8FF);
    #else
    assem_debug("uxth %s,%s",regname[rt],regname[rs]);
    output_w32(0xe6ff0070|rd_rn_rm(rt,0,rs));
    #endif
  }else{
    assert(imm>0&&imm<65535);
    #ifdef ARMv5_ONLY
    assem_debug("mov r14,#%d",imm&0xFF00);
    output_w32(0xe3a00000|rd_rn_imm_shift(HOST_TEMPREG,0,imm>>8,8));
    assem_debug("add r14,r14,#%d",imm&0xFF);
    output_w32(0xe2800000|rd_rn_imm_shift(HOST_TEMPREG,HOST_TEMPREG,imm&0xff,0));
    #else
    emit_movw(imm,HOST_TEMPREG);
    #endif
    assem_debug("and %s,%s,r14",regname[rt],regname[rs]);
    output_w32(0xe0000000|rd_rn_rm(rt,rs,HOST_TEMPREG));
  }
}

static void emit_orimm(int rs,int imm,int rt)
{
  u_int armval;
  if(imm==0) {
    if(rs!=rt) emit_mov(rs,rt);
  }else if(genimm(imm,&armval)) {
    assem_debug("orr %s,%s,#%d",regname[rt],regname[rs],imm);
    output_w32(0xe3800000|rd_rn_rm(rt,rs,0)|armval);
  }else{
    assert(imm>0&&imm<65536);
    assem_debug("orr %s,%s,#%d",regname[rt],regname[rs],imm&0xFF00);
    assem_debug("orr %s,%s,#%d",regname[rt],regname[rs],imm&0xFF);
    output_w32(0xe3800000|rd_rn_imm_shift(rt,rs,imm>>8,8));
    output_w32(0xe3800000|rd_rn_imm_shift(rt,rt,imm&0xff,0));
  }
}

static void emit_xorimm(int rs,int imm,int rt)
{
  u_int armval;
  if(imm==0) {
    if(rs!=rt) emit_mov(rs,rt);
  }else if(genimm(imm,&armval)) {
    assem_debug("eor %s,%s,#%d",regname[rt],regname[rs],imm);
    output_w32(0xe2200000|rd_rn_rm(rt,rs,0)|armval);
  }else{
    assert(imm>0&&imm<65536);
    assem_debug("eor %s,%s,#%d",regname[rt],regname[rs],imm&0xFF00);
    assem_debug("eor %s,%s,#%d",regname[rt],regname[rs],imm&0xFF);
    output_w32(0xe2200000|rd_rn_imm_shift(rt,rs,imm>>8,8));
    output_w32(0xe2200000|rd_rn_imm_shift(rt,rt,imm&0xff,0));
  }
}

static void emit_shlimm(int rs,u_int imm,int rt)
{
  assert(imm>0);
  assert(imm<32);
  //if(imm==1) ...
  assem_debug("lsl %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0xe1a00000|rd_rn_rm(rt,0,rs)|(imm<<7));
}

static void emit_shrimm(int rs,u_int imm,int rt)
{
  assert(imm>0);
  assert(imm<32);
  assem_debug("lsr %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0xe1a00000|rd_rn_rm(rt,0,rs)|0x20|(imm<<7));
}

static void emit_sarimm(int rs,u_int imm,int rt)
{
  assert(imm>0);
  assert(imm<32);
  assem_debug("asr %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0xe1a00000|rd_rn_rm(rt,0,rs)|0x40|(imm<<7));
}

static void emit_rorimm(int rs,u_int imm,int rt)
{
  assert(imm>0);
  assert(imm<32);
  assem_debug("ror %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0xe1a00000|rd_rn_rm(rt,0,rs)|0x60|(imm<<7));
}

static void emit_shldimm(int rs,int rs2,u_int imm,int rt)
{
  assem_debug("shld %%%s,%%%s,%d",regname[rt],regname[rs2],imm);
  assert(imm>0);
  assert(imm<32);
  //if(imm==1) ...
  assem_debug("lsl %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0xe1a00000|rd_rn_rm(rt,0,rs)|(imm<<7));
  assem_debug("orr %s,%s,%s,lsr #%d",regname[rt],regname[rt],regname[rs2],32-imm);
  output_w32(0xe1800020|rd_rn_rm(rt,rt,rs2)|((32-imm)<<7));
}

static void emit_shrdimm(int rs,int rs2,u_int imm,int rt)
{
  assem_debug("shrd %%%s,%%%s,%d",regname[rt],regname[rs2],imm);
  assert(imm>0);
  assert(imm<32);
  //if(imm==1) ...
  assem_debug("lsr %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0xe1a00020|rd_rn_rm(rt,0,rs)|(imm<<7));
  assem_debug("orr %s,%s,%s,lsl #%d",regname[rt],regname[rt],regname[rs2],32-imm);
  output_w32(0xe1800000|rd_rn_rm(rt,rt,rs2)|((32-imm)<<7));
}

static void emit_shl(u_int rs,u_int shift,u_int rt)
{
  assert(rs<16);
  assert(rt<16);
  assert(shift<16);
  //if(imm==1) ...
  assem_debug("lsl %s,%s,%s",regname[rt],regname[rs],regname[shift]);
  output_w32(0xe1a00000|rd_rn_rm(rt,0,rs)|0x10|(shift<<8));
}
static void emit_shr(u_int rs,u_int shift,u_int rt)
{
  assert(rs<16);
  assert(rt<16);
  assert(shift<16);
  assem_debug("lsr %s,%s,%s",regname[rt],regname[rs],regname[shift]);
  output_w32(0xe1a00000|rd_rn_rm(rt,0,rs)|0x30|(shift<<8));
}
static void emit_sar(u_int rs,u_int shift,u_int rt)
{
  assert(rs<16);
  assert(rt<16);
  assert(shift<16);
  assem_debug("asr %s,%s,%s",regname[rt],regname[rs],regname[shift]);
  output_w32(0xe1a00000|rd_rn_rm(rt,0,rs)|0x50|(shift<<8));
}

static void emit_orrshl(u_int rs,u_int shift,u_int rt)
{
  assert(rs<16);
  assert(rt<16);
  assert(shift<16);
  assem_debug("orr %s,%s,%s,lsl %s",regname[rt],regname[rt],regname[rs],regname[shift]);
  output_w32(0xe1800000|rd_rn_rm(rt,rt,rs)|0x10|(shift<<8));
}
static void emit_orrshr(u_int rs,u_int shift,u_int rt)
{
  assert(rs<16);
  assert(rt<16);
  assert(shift<16);
  assem_debug("orr %s,%s,%s,lsr %s",regname[rt],regname[rt],regname[rs],regname[shift]);
  output_w32(0xe1800000|rd_rn_rm(rt,rt,rs)|0x30|(shift<<8));
}

static void emit_cmpimm(int rs,int imm)
{
  u_int armval;
  if(genimm(imm,&armval)) {
    assem_debug("cmp %s,#%d",regname[rs],imm);
    output_w32(0xe3500000|rd_rn_rm(0,rs,0)|armval);
  }else if(genimm(-imm,&armval)) {
    assem_debug("cmn %s,#%d",regname[rs],imm);
    output_w32(0xe3700000|rd_rn_rm(0,rs,0)|armval);
  }else if(imm>0) {
    assert(imm<65536);
    #ifdef ARMv5_ONLY
    emit_movimm(imm,HOST_TEMPREG);
    #else
    emit_movw(imm,HOST_TEMPREG);
    #endif
    assem_debug("cmp %s,r14",regname[rs]);
    output_w32(0xe1500000|rd_rn_rm(0,rs,HOST_TEMPREG));
  }else{
    assert(imm>-65536);
    #ifdef ARMv5_ONLY
    emit_movimm(-imm,HOST_TEMPREG);
    #else
    emit_movw(-imm,HOST_TEMPREG);
    #endif
    assem_debug("cmn %s,r14",regname[rs]);
    output_w32(0xe1700000|rd_rn_rm(0,rs,HOST_TEMPREG));
  }
}

static void emit_cmovne_imm(int imm,int rt)
{
  assem_debug("movne %s,#%d",regname[rt],imm);
  u_int armval, ret;
  ret = genimm(imm,&armval);
  assert(ret);
  output_w32(0x13a00000|rd_rn_rm(rt,0,0)|armval);
}
static void emit_cmovl_imm(int imm,int rt)
{
  assem_debug("movlt %s,#%d",regname[rt],imm);
  u_int armval, ret;
  ret = genimm(imm,&armval);
  assert(ret);
  output_w32(0xb3a00000|rd_rn_rm(rt,0,0)|armval);
}
static void emit_cmovb_imm(int imm,int rt)
{
  assem_debug("movcc %s,#%d",regname[rt],imm);
  u_int armval, ret;
  ret = genimm(imm,&armval);
  assert(ret);
  output_w32(0x33a00000|rd_rn_rm(rt,0,0)|armval);
}
static void emit_cmovs_imm(int imm,int rt)
{
  assem_debug("movmi %s,#%d",regname[rt],imm);
  u_int armval, ret;
  ret = genimm(imm,&armval);
  assert(ret);
  output_w32(0x43a00000|rd_rn_rm(rt,0,0)|armval);
}
static void emit_cmove_reg(int rs,int rt)
{
  assem_debug("moveq %s,%s",regname[rt],regname[rs]);
  output_w32(0x01a00000|rd_rn_rm(rt,0,rs));
}
static void emit_cmovne_reg(int rs,int rt)
{
  assem_debug("movne %s,%s",regname[rt],regname[rs]);
  output_w32(0x11a00000|rd_rn_rm(rt,0,rs));
}
static void emit_cmovl_reg(int rs,int rt)
{
  assem_debug("movlt %s,%s",regname[rt],regname[rs]);
  output_w32(0xb1a00000|rd_rn_rm(rt,0,rs));
}
static void emit_cmovs_reg(int rs,int rt)
{
  assem_debug("movmi %s,%s",regname[rt],regname[rs]);
  output_w32(0x41a00000|rd_rn_rm(rt,0,rs));
}

static void emit_slti32(int rs,int imm,int rt)
{
  if(rs!=rt) emit_zeroreg(rt);
  emit_cmpimm(rs,imm);
  if(rs==rt) emit_movimm(0,rt);
  emit_cmovl_imm(1,rt);
}
static void emit_sltiu32(int rs,int imm,int rt)
{
  if(rs!=rt) emit_zeroreg(rt);
  emit_cmpimm(rs,imm);
  if(rs==rt) emit_movimm(0,rt);
  emit_cmovb_imm(1,rt);
}
static void emit_slti64_32(int rsh,int rsl,int imm,int rt)
{
  assert(rsh!=rt);
  emit_slti32(rsl,imm,rt);
  if(imm>=0)
  {
    emit_test(rsh,rsh);
    emit_cmovne_imm(0,rt);
    emit_cmovs_imm(1,rt);
  }
  else
  {
    emit_cmpimm(rsh,-1);
    emit_cmovne_imm(0,rt);
    emit_cmovl_imm(1,rt);
  }
}
static void emit_sltiu64_32(int rsh,int rsl,int imm,int rt)
{
  assert(rsh!=rt);
  emit_sltiu32(rsl,imm,rt);
  if(imm>=0)
  {
    emit_test(rsh,rsh);
    emit_cmovne_imm(0,rt);
  }
  else
  {
    emit_cmpimm(rsh,-1);
    emit_cmovne_imm(1,rt);
  }
}

static void emit_cmp(int rs,int rt)
{
  assem_debug("cmp %s,%s",regname[rs],regname[rt]);
  output_w32(0xe1500000|rd_rn_rm(0,rs,rt));
}
static void emit_set_gz32(int rs, int rt)
{
  //assem_debug("set_gz32");
  emit_cmpimm(rs,1);
  emit_movimm(1,rt);
  emit_cmovl_imm(0,rt);
}
static void emit_set_nz32(int rs, int rt)
{
  //assem_debug("set_nz32");
  if(rs!=rt) emit_movs(rs,rt);
  else emit_test(rs,rs);
  emit_cmovne_imm(1,rt);
}
static void emit_set_gz64_32(int rsh, int rsl, int rt)
{
  //assem_debug("set_gz64");
  emit_set_gz32(rsl,rt);
  emit_test(rsh,rsh);
  emit_cmovne_imm(1,rt);
  emit_cmovs_imm(0,rt);
}
static void emit_set_nz64_32(int rsh, int rsl, int rt)
{
  //assem_debug("set_nz64");
  emit_or_and_set_flags(rsh,rsl,rt);
  emit_cmovne_imm(1,rt);
}
static void emit_set_if_less32(int rs1, int rs2, int rt)
{
  //assem_debug("set if less (%%%s,%%%s),%%%s",regname[rs1],regname[rs2],regname[rt]);
  if(rs1!=rt&&rs2!=rt) emit_zeroreg(rt);
  emit_cmp(rs1,rs2);
  if(rs1==rt||rs2==rt) emit_movimm(0,rt);
  emit_cmovl_imm(1,rt);
}
static void emit_set_if_carry32(int rs1, int rs2, int rt)
{
  //assem_debug("set if carry (%%%s,%%%s),%%%s",regname[rs1],regname[rs2],regname[rt]);
  if(rs1!=rt&&rs2!=rt) emit_zeroreg(rt);
  emit_cmp(rs1,rs2);
  if(rs1==rt||rs2==rt) emit_movimm(0,rt);
  emit_cmovb_imm(1,rt);
}
static void emit_set_if_less64_32(int u1, int l1, int u2, int l2, int rt)
{
  //assem_debug("set if less64 (%%%s,%%%s,%%%s,%%%s),%%%s",regname[u1],regname[l1],regname[u2],regname[l2],regname[rt]);
  assert(u1!=rt);
  assert(u2!=rt);
  emit_cmp(l1,l2);
  emit_movimm(0,rt);
  emit_sbcs(u1,u2,HOST_TEMPREG);
  emit_cmovl_imm(1,rt);
}
static void emit_set_if_carry64_32(int u1, int l1, int u2, int l2, int rt)
{
  //assem_debug("set if carry64 (%%%s,%%%s,%%%s,%%%s),%%%s",regname[u1],regname[l1],regname[u2],regname[l2],regname[rt]);
  assert(u1!=rt);
  assert(u2!=rt);
  emit_cmp(l1,l2);
  emit_movimm(0,rt);
  emit_sbcs(u1,u2,HOST_TEMPREG);
  emit_cmovb_imm(1,rt);
}

static void emit_call(int a)
{
  assem_debug("bl %x (%x+%x)",a,(int)out,a-(int)out-8);
  u_int offset=genjmp(a);
  output_w32(0xeb000000|offset);
}
static void emit_jmp(int a)
{
  assem_debug("b %x (%x+%x)",a,(int)out,a-(int)out-8);
  u_int offset=genjmp(a);
  output_w32(0xea000000|offset);
}
static void emit_jne(int a)
{
  assem_debug("bne %x",a);
  u_int offset=genjmp(a);
  output_w32(0x1a000000|offset);
}
static void emit_jeq(int a)
{
  assem_debug("beq %x",a);
  u_int offset=genjmp(a);
  output_w32(0x0a000000|offset);
}
static void emit_js(int a)
{
  assem_debug("bmi %x",a);
  u_int offset=genjmp(a);
  output_w32(0x4a000000|offset);
}
static void emit_jns(int a)
{
  assem_debug("bpl %x",a);
  u_int offset=genjmp(a);
  output_w32(0x5a000000|offset);
}
static void emit_jl(int a)
{
  assem_debug("blt %x",a);
  u_int offset=genjmp(a);
  output_w32(0xba000000|offset);
}
static void emit_jge(int a)
{
  assem_debug("bge %x",a);
  u_int offset=genjmp(a);
  output_w32(0xaa000000|offset);
}
static void emit_jno(int a)
{
  assem_debug("bvc %x",a);
  u_int offset=genjmp(a);
  output_w32(0x7a000000|offset);
}

static void emit_jcc(int a)
{
  assem_debug("bcc %x",a);
  u_int offset=genjmp(a);
  output_w32(0x3a000000|offset);
}

static void emit_pushreg(u_int r)
{
  assem_debug("push %%%s",regname[r]);
  assert(0);
}
static void emit_popreg(u_int r)
{
  assem_debug("pop %%%s",regname[r]);
  assert(0);
}
/*
static void emit_callreg(u_int r)
{
  assem_debug("call *%%%s",regname[r]);
  assert(0);
}
static void emit_jmpreg(u_int r)
{
  assem_debug("mov pc,%s",regname[r]);
  output_w32(0xe1a00000|rd_rn_rm(15,0,r));
}
*/
static void emit_readword_indexed(int offset, int rs, int rt)
{
  assert(offset>-4096&&offset<4096);
  assem_debug("ldr %s,%s+%d",regname[rt],regname[rs],offset);
  if(offset>=0) {
    output_w32(0xe5900000|rd_rn_rm(rt,rs,0)|offset);
  }else{
    output_w32(0xe5100000|rd_rn_rm(rt,rs,0)|(-offset));
  }
}
static void emit_readword_dualindexedx4(int rs1, int rs2, int rt)
{
  assem_debug("ldr %s,%s,%s lsl #2",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe7900000|rd_rn_rm(rt,rs1,rs2)|0x100);
}
static void emit_readword_indexed_tlb(int addr, int rs, int map, int rt)
{
  if(map<0) emit_readword_indexed(addr, rs, rt);
  else {
    assert(addr==0);
    emit_readword_dualindexedx4(rs, map, rt);
  }
}
static void emit_readdword_indexed_tlb(int addr, int rs, int map, int rh, int rl)
{
  if(map<0) {
    if(rh>=0) emit_readword_indexed(addr, rs, rh);
    emit_readword_indexed(addr+4, rs, rl);
  }else{
    assert(rh!=rs);
    if(rh>=0) emit_readword_indexed_tlb(addr, rs, map, rh);
    emit_addimm(map,1,HOST_TEMPREG);
    emit_readword_indexed_tlb(addr, rs, HOST_TEMPREG, rl);
  }
}
static void emit_movsbl_indexed(int offset, int rs, int rt)
{
  assert(offset>-256&&offset<256);
  assem_debug("ldrsb %s,%s+%d",regname[rt],regname[rs],offset);
  if(offset>=0) {
    output_w32(0xe1d000d0|rd_rn_rm(rt,rs,0)|((offset<<4)&0xf00)|(offset&0xf));
  }else{
    output_w32(0xe15000d0|rd_rn_rm(rt,rs,0)|(((-offset)<<4)&0xf00)|((-offset)&0xf));
  }
}
static void emit_movsbl_indexed_tlb(int addr, int rs, int map, int rt)
{
  if(map<0) emit_movsbl_indexed(addr, rs, rt);
  else {
    if(addr==0) {
      emit_shlimm(map,2,HOST_TEMPREG);
      assem_debug("ldrsb %s,%s+%s",regname[rt],regname[rs],regname[HOST_TEMPREG]);
      output_w32(0xe19000d0|rd_rn_rm(rt,rs,HOST_TEMPREG));
    }else{
      assert(addr>-256&&addr<256);
      assem_debug("add %s,%s,%s,lsl #2",regname[rt],regname[rs],regname[map]);
      output_w32(0xe0800000|rd_rn_rm(rt,rs,map)|(2<<7));
      emit_movsbl_indexed(addr, rt, rt);
    }
  }
}
static void emit_movswl_indexed(int offset, int rs, int rt)
{
  assert(offset>-256&&offset<256);
  assem_debug("ldrsh %s,%s+%d",regname[rt],regname[rs],offset);
  if(offset>=0) {
    output_w32(0xe1d000f0|rd_rn_rm(rt,rs,0)|((offset<<4)&0xf00)|(offset&0xf));
  }else{
    output_w32(0xe15000f0|rd_rn_rm(rt,rs,0)|(((-offset)<<4)&0xf00)|((-offset)&0xf));
  }
}
static void emit_movzbl_indexed(int offset, int rs, int rt)
{
  assert(offset>-4096&&offset<4096);
  assem_debug("ldrb %s,%s+%d",regname[rt],regname[rs],offset);
  if(offset>=0) {
    output_w32(0xe5d00000|rd_rn_rm(rt,rs,0)|offset);
  }else{
    output_w32(0xe5500000|rd_rn_rm(rt,rs,0)|(-offset));
  }
}
static void emit_movzbl_dualindexedx4(int rs1, int rs2, int rt)
{
  assem_debug("ldrb %s,%s,%s lsl #2",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe7d00000|rd_rn_rm(rt,rs1,rs2)|0x100);
}
static void emit_movzbl_indexed_tlb(int addr, int rs, int map, int rt)
{
  if(map<0) emit_movzbl_indexed(addr, rs, rt);
  else {
    if(addr==0) {
      emit_movzbl_dualindexedx4(rs, map, rt);
    }else{
      emit_addimm(rs,addr,rt);
      emit_movzbl_dualindexedx4(rt, map, rt);
    }
  }
}
static void emit_movzwl_indexed(int offset, int rs, int rt)
{
  assert(offset>-256&&offset<256);
  assem_debug("ldrh %s,%s+%d",regname[rt],regname[rs],offset);
  if(offset>=0) {
    output_w32(0xe1d000b0|rd_rn_rm(rt,rs,0)|((offset<<4)&0xf00)|(offset&0xf));
  }else{
    output_w32(0xe15000b0|rd_rn_rm(rt,rs,0)|(((-offset)<<4)&0xf00)|((-offset)&0xf));
  }
}
static void emit_readword(int addr, int rt)
{
  u_int offset = addr-(u_int)&dynarec_local;
  assert(offset<4096);
  assem_debug("ldr %s,fp+%d",regname[rt],offset);
  output_w32(0xe5900000|rd_rn_rm(rt,FP,0)|offset);
}
static void emit_movsbl(int addr, int rt)
{
  u_int offset = addr-(u_int)&dynarec_local;
  assert(offset<256);
  assem_debug("ldrsb %s,fp+%d",regname[rt],offset);
  output_w32(0xe1d000d0|rd_rn_rm(rt,FP,0)|((offset<<4)&0xf00)|(offset&0xf));
}
static void emit_movswl(int addr, int rt)
{
  u_int offset = addr-(u_int)&dynarec_local;
  assert(offset<256);
  assem_debug("ldrsh %s,fp+%d",regname[rt],offset);
  output_w32(0xe1d000f0|rd_rn_rm(rt,FP,0)|((offset<<4)&0xf00)|(offset&0xf));
}
static void emit_movzbl(int addr, int rt)
{
  u_int offset = addr-(u_int)&dynarec_local;
  assert(offset<4096);
  assem_debug("ldrb %s,fp+%d",regname[rt],offset);
  output_w32(0xe5d00000|rd_rn_rm(rt,FP,0)|offset);
}
static void emit_movzwl(int addr, int rt)
{
  u_int offset = addr-(u_int)&dynarec_local;
  assert(offset<256);
  assem_debug("ldrh %s,fp+%d",regname[rt],offset);
  output_w32(0xe1d000b0|rd_rn_rm(rt,FP,0)|((offset<<4)&0xf00)|(offset&0xf));
}

/*
static void emit_movzwl_reg(int rs, int rt)
{
  assem_debug("movzwl %%%s,%%%s",regname[rs]+1,regname[rt]);
  assert(0);
}
*/

static void emit_writeword_indexed(int rt, int offset, int rs)
{
  assert(offset>-4096&&offset<4096);
  assem_debug("str %s,%s+%d",regname[rt],regname[rs],offset);
  if(offset>=0) {
    output_w32(0xe5800000|rd_rn_rm(rt,rs,0)|offset);
  }else{
    output_w32(0xe5000000|rd_rn_rm(rt,rs,0)|(-offset));
  }
}
static void emit_writeword_dualindexedx4(int rt, int rs1, int rs2)
{
  assem_debug("str %s,%s,%s lsl #2",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe7800000|rd_rn_rm(rt,rs1,rs2)|0x100);
}
static void emit_writeword_indexed_tlb(int rt, int addr, int rs, int map, int temp)
{
  if(map<0) emit_writeword_indexed(rt, addr, rs);
  else {
    assert(addr==0);
    emit_writeword_dualindexedx4(rt, rs, map);
  }
}
static void emit_writedword_indexed_tlb(int rh, int rl, int addr, int rs, int map, int temp)
{
  if(map<0) {
    if(rh>=0) emit_writeword_indexed(rh, addr, rs);
    emit_writeword_indexed(rl, addr+4, rs);
  }else{
    assert(rh>=0);
    if(temp!=rs) emit_addimm(map,1,temp);
    emit_writeword_indexed_tlb(rh, addr, rs, map, temp);
    if(temp!=rs) emit_writeword_indexed_tlb(rl, addr, rs, temp, temp);
    else {
      emit_addimm(rs,4,rs);
      emit_writeword_indexed_tlb(rl, addr, rs, map, temp);
    }
  }
}
static void emit_writehword_indexed(int rt, int offset, int rs)
{
  assert(offset>-256&&offset<256);
  assem_debug("strh %s,%s+%d",regname[rt],regname[rs],offset);
  if(offset>=0) {
    output_w32(0xe1c000b0|rd_rn_rm(rt,rs,0)|((offset<<4)&0xf00)|(offset&0xf));
  }else{
    output_w32(0xe14000b0|rd_rn_rm(rt,rs,0)|(((-offset)<<4)&0xf00)|((-offset)&0xf));
  }
}
static void emit_writebyte_indexed(int rt, int offset, int rs)
{
  assert(offset>-4096&&offset<4096);
  assem_debug("strb %s,%s+%d",regname[rt],regname[rs],offset);
  if(offset>=0) {
    output_w32(0xe5c00000|rd_rn_rm(rt,rs,0)|offset);
  }else{
    output_w32(0xe5400000|rd_rn_rm(rt,rs,0)|(-offset));
  }
}
static void emit_writebyte_dualindexedx4(int rt, int rs1, int rs2)
{
  assem_debug("strb %s,%s,%s lsl #2",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe7c00000|rd_rn_rm(rt,rs1,rs2)|0x100);
}
static void emit_writebyte_indexed_tlb(int rt, int addr, int rs, int map, int temp)
{
  if(map<0) emit_writebyte_indexed(rt, addr, rs);
  else {
    if(addr==0) {
      emit_writebyte_dualindexedx4(rt, rs, map);
    }else{
      emit_addimm(rs,addr,temp);
      emit_writebyte_dualindexedx4(rt, temp, map);
    }
  }
}
static void emit_writeword(int rt, int addr)
{
  u_int offset = addr-(u_int)&dynarec_local;
  assert(offset<4096);
  assem_debug("str %s,fp+%d",regname[rt],offset);
  output_w32(0xe5800000|rd_rn_rm(rt,FP,0)|offset);
}
static void emit_writehword(int rt, int addr)
{
  u_int offset = addr-(u_int)&dynarec_local;
  assert(offset<256);
  assem_debug("strh %s,fp+%d",regname[rt],offset);
  output_w32(0xe1c000b0|rd_rn_rm(rt,FP,0)|((offset<<4)&0xf00)|(offset&0xf));
}
static void emit_writebyte(int rt, int addr)
{
  u_int offset = addr-(u_int)&dynarec_local;
  assert(offset<4096);
  assem_debug("strb %s,fp+%d",regname[rt],offset);
  output_w32(0xe5c00000|rd_rn_rm(rt,FP,0)|offset);
}

/*
static void emit_mul(int rs)
{
  assem_debug("mul %%%s",regname[rs]);
  assert(0);
}
*/

static void emit_umull(u_int rs1, u_int rs2, u_int hi, u_int lo)
{
  assem_debug("umull %s, %s, %s, %s",regname[lo],regname[hi],regname[rs1],regname[rs2]);
  assert(rs1<16);
  assert(rs2<16);
  assert(hi<16);
  assert(lo<16);
  output_w32(0xe0800090|(hi<<16)|(lo<<12)|(rs2<<8)|rs1);
}
static void emit_smull(u_int rs1, u_int rs2, u_int hi, u_int lo)
{
  assem_debug("smull %s, %s, %s, %s",regname[lo],regname[hi],regname[rs1],regname[rs2]);
  assert(rs1<16);
  assert(rs2<16);
  assert(hi<16);
  assert(lo<16);
  output_w32(0xe0c00090|(hi<<16)|(lo<<12)|(rs2<<8)|rs1);
}

static void emit_clz(int rs,int rt)
{
  assem_debug("clz %s,%s",regname[rt],regname[rs]);
  output_w32(0xe16f0f10|rd_rn_rm(rt,0,rs));
}

static void emit_subcs(int rs1,int rs2,int rt)
{
  assem_debug("subcs %s,%s,%s",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0x20400000|rd_rn_rm(rt,rs1,rs2));
}

static void emit_shrcc_imm(int rs,u_int imm,int rt)
{
  assert(imm>0);
  assert(imm<32);
  assem_debug("lsrcc %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0x31a00000|rd_rn_rm(rt,0,rs)|0x20|(imm<<7));
}

static void emit_negmi(int rs, int rt)
{
  assem_debug("rsbmi %s,%s,#0",regname[rt],regname[rs]);
  output_w32(0x42600000|rd_rn_rm(rt,rs,0));
}

static void emit_orreq(u_int rs1,u_int rs2,u_int rt)
{
  assem_debug("orreq %s,%s,%s",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0x01800000|rd_rn_rm(rt,rs1,rs2));
}

static void emit_orrne(u_int rs1,u_int rs2,u_int rt)
{
  assem_debug("orrne %s,%s,%s",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0x11800000|rd_rn_rm(rt,rs1,rs2));
}

static void emit_bic_lsl(u_int rs1,u_int rs2,u_int shift,u_int rt)
{
  assem_debug("bic %s,%s,%s lsl %s",regname[rt],regname[rs1],regname[rs2],regname[shift]);
  output_w32(0xe1C00000|rd_rn_rm(rt,rs1,rs2)|0x10|(shift<<8));
}

static void emit_biceq_lsl(u_int rs1,u_int rs2,u_int shift,u_int rt)
{
  assem_debug("biceq %s,%s,%s lsl %s",regname[rt],regname[rs1],regname[rs2],regname[shift]);
  output_w32(0x01C00000|rd_rn_rm(rt,rs1,rs2)|0x10|(shift<<8));
}

static void emit_bicne_lsl(u_int rs1,u_int rs2,u_int shift,u_int rt)
{
  assem_debug("bicne %s,%s,%s lsl %s",regname[rt],regname[rs1],regname[rs2],regname[shift]);
  output_w32(0x11C00000|rd_rn_rm(rt,rs1,rs2)|0x10|(shift<<8));
}

static void emit_bic_lsr(u_int rs1,u_int rs2,u_int shift,u_int rt)
{
  assem_debug("bic %s,%s,%s lsr %s",regname[rt],regname[rs1],regname[rs2],regname[shift]);
  output_w32(0xe1C00000|rd_rn_rm(rt,rs1,rs2)|0x30|(shift<<8));
}

static void emit_biceq_lsr(u_int rs1,u_int rs2,u_int shift,u_int rt)
{
  assem_debug("biceq %s,%s,%s lsr %s",regname[rt],regname[rs1],regname[rs2],regname[shift]);
  output_w32(0x01C00000|rd_rn_rm(rt,rs1,rs2)|0x30|(shift<<8));
}

static void emit_bicne_lsr(u_int rs1,u_int rs2,u_int shift,u_int rt)
{
  assem_debug("bicne %s,%s,%s lsr %s",regname[rt],regname[rs1],regname[rs2],regname[shift]);
  output_w32(0x11C00000|rd_rn_rm(rt,rs1,rs2)|0x30|(shift<<8));
}

static void emit_teq(int rs, int rt)
{
  assem_debug("teq %s,%s",regname[rs],regname[rt]);
  output_w32(0xe1300000|rd_rn_rm(0,rs,rt));
}

static void emit_rsbimm(int rs, int imm, int rt)
{
  u_int armval, ret;
  ret = genimm(imm,&armval);
  assert(ret);
  assem_debug("rsb %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0xe2600000|rd_rn_rm(rt,rs,0)|armval);
}

// Load 2 immediates optimizing for small code size
static void emit_mov2imm_compact(int imm1,u_int rt1,int imm2,u_int rt2)
{
  emit_movimm(imm1,rt1);
  u_int armval;
  if(genimm(imm2-imm1,&armval)) {
    assem_debug("add %s,%s,#%d",regname[rt2],regname[rt1],imm2-imm1);
    output_w32(0xe2800000|rd_rn_rm(rt2,rt1,0)|armval);
  }else if(genimm(imm1-imm2,&armval)) {
    assem_debug("sub %s,%s,#%d",regname[rt2],regname[rt1],imm1-imm2);
    output_w32(0xe2400000|rd_rn_rm(rt2,rt1,0)|armval);
  }
  else emit_movimm(imm2,rt2);
}

// Conditionally select one of two immediates, optimizing for small code size
// This will only be called if HAVE_CMOV_IMM is defined
static void emit_cmov2imm_e_ne_compact(int imm1,int imm2,u_int rt)
{
  u_int armval;
  if(genimm(imm2-imm1,&armval)) {
    emit_movimm(imm1,rt);
    assem_debug("addne %s,%s,#%d",regname[rt],regname[rt],imm2-imm1);
    output_w32(0x12800000|rd_rn_rm(rt,rt,0)|armval);
  }else if(genimm(imm1-imm2,&armval)) {
    emit_movimm(imm1,rt);
    assem_debug("subne %s,%s,#%d",regname[rt],regname[rt],imm1-imm2);
    output_w32(0x12400000|rd_rn_rm(rt,rt,0)|armval);
  }
  else {
    #ifdef ARMv5_ONLY
    emit_movimm(imm1,rt);
    add_literal((int)out,imm2);
    assem_debug("ldrne %s,pc+? [=%x]",regname[rt],imm2);
    output_w32(0x15900000|rd_rn_rm(rt,15,0));
    #else
    emit_movw(imm1&0x0000FFFF,rt);
    if((imm1&0xFFFF)!=(imm2&0xFFFF)) {
      assem_debug("movwne %s,#%d (0x%x)",regname[rt],imm2&0xFFFF,imm2&0xFFFF);
      output_w32(0x13000000|rd_rn_rm(rt,0,0)|(imm2&0xfff)|((imm2<<4)&0xf0000));
    }
    emit_movt(imm1&0xFFFF0000,rt);
    if((imm1&0xFFFF0000)!=(imm2&0xFFFF0000)) {
      assem_debug("movtne %s,#%d (0x%x)",regname[rt],imm2&0xffff0000,imm2&0xffff0000);
      output_w32(0x13400000|rd_rn_rm(rt,0,0)|((imm2>>16)&0xfff)|((imm2>>12)&0xf0000));
    }
    #endif
  }
}

#if !defined(HOST_IMM8)
// special case for checking invalid_code
static void emit_cmpmem_indexedsr12_imm(int addr,int r,int imm)
{
  assert(0);
}
#endif

// special case for checking invalid_code
static void emit_cmpmem_indexedsr12_reg(int base,int r,int imm)
{
  assert(imm<128&&imm>=0);
  assert(r>=0&&r<16);
  assem_debug("ldrb lr,%s,%s lsr #12",regname[base],regname[r]);
  output_w32(0xe7d00000|rd_rn_rm(HOST_TEMPREG,base,r)|0x620);
  emit_cmpimm(HOST_TEMPREG,imm);
}

// special case for tlb mapping
static void emit_addsr12(int rs1,int rs2,int rt)
{
  assem_debug("add %s,%s,%s lsr #12",regname[rt],regname[rs1],regname[rs2]);
  output_w32(0xe0800620|rd_rn_rm(rt,rs1,rs2));
}

static void emit_callne(int a)
{
  assem_debug("blne %x",a);
  u_int offset=genjmp(a);
  output_w32(0x1b000000|offset);
}

#ifdef IMM_PREFETCH
// Used to preload hash table entries
static void emit_prefetch(void *addr)
{
  assem_debug("prefetch %x",(int)addr);
  output_byte(0x0F);
  output_byte(0x18);
  output_modrm(0,5,1);
  output_w32((int)addr);
}
#endif

#ifdef REG_PREFETCH
static void emit_prefetchreg(int r)
{
  assem_debug("pld %s",regname[r]);
  output_w32(0xf5d0f000|rd_rn_rm(0,r,0));
}
#endif

// Special case for mini_ht
static void emit_ldreq_indexed(int rs, u_int offset, int rt)
{
  assert(offset<4096);
  assem_debug("ldreq %s,[%s, #%d]",regname[rt],regname[rs],offset);
  output_w32(0x05900000|rd_rn_rm(rt,rs,0)|offset);
}

static void emit_flds(int r,int sr)
{
  assem_debug("flds s%d,[%s]",sr,regname[r]);
  output_w32(0xed900a00|((sr&14)<<11)|((sr&1)<<22)|(r<<16));
} 

static void emit_vldr(int r,int vr)
{
  assem_debug("vldr d%d,[%s]",vr,regname[r]);
  output_w32(0xed900b00|(vr<<12)|(r<<16));
} 

static void emit_fsts(int sr,int r)
{
  assem_debug("fsts s%d,[%s]",sr,regname[r]);
  output_w32(0xed800a00|((sr&14)<<11)|((sr&1)<<22)|(r<<16));
} 

static void emit_vstr(int vr,int r)
{
  assem_debug("vstr d%d,[%s]",vr,regname[r]);
  output_w32(0xed800b00|(vr<<12)|(r<<16));
} 

static void emit_ftosizs(int s,int d)
{
  assem_debug("ftosizs s%d,s%d",d,s);
  output_w32(0xeebd0ac0|((d&14)<<11)|((d&1)<<22)|((s&14)>>1)|((s&1)<<5));
} 

static void emit_ftosizd(int s,int d)
{
  assem_debug("ftosizd s%d,d%d",d,s);
  output_w32(0xeebd0bc0|((d&14)<<11)|((d&1)<<22)|(s&7));
} 

static void emit_fsitos(int s,int d)
{
  assem_debug("fsitos s%d,s%d",d,s);
  output_w32(0xeeb80ac0|((d&14)<<11)|((d&1)<<22)|((s&14)>>1)|((s&1)<<5));
} 

static void emit_fsitod(int s,int d)
{
  assem_debug("fsitod d%d,s%d",d,s);
  output_w32(0xeeb80bc0|((d&7)<<12)|((s&14)>>1)|((s&1)<<5));
} 

static void emit_fcvtds(int s,int d)
{
  assem_debug("fcvtds d%d,s%d",d,s);
  output_w32(0xeeb70ac0|((d&7)<<12)|((s&14)>>1)|((s&1)<<5));
} 

static void emit_fcvtsd(int s,int d)
{
  assem_debug("fcvtsd s%d,d%d",d,s);
  output_w32(0xeeb70bc0|((d&14)<<11)|((d&1)<<22)|(s&7));
} 

static void emit_fsqrts(int s,int d)
{
  assem_debug("fsqrts d%d,s%d",d,s);
  output_w32(0xeeb10ac0|((d&14)<<11)|((d&1)<<22)|((s&14)>>1)|((s&1)<<5));
} 

static void emit_fsqrtd(int s,int d)
{
  assem_debug("fsqrtd s%d,d%d",d,s);
  output_w32(0xeeb10bc0|((d&7)<<12)|(s&7));
} 

static void emit_fabss(int s,int d)
{
  assem_debug("fabss d%d,s%d",d,s);
  output_w32(0xeeb00ac0|((d&14)<<11)|((d&1)<<22)|((s&14)>>1)|((s&1)<<5));
} 

static void emit_fabsd(int s,int d)
{
  assem_debug("fabsd s%d,d%d",d,s);
  output_w32(0xeeb00bc0|((d&7)<<12)|(s&7));
} 

static void emit_fnegs(int s,int d)
{
  assem_debug("fnegs d%d,s%d",d,s);
  output_w32(0xeeb10a40|((d&14)<<11)|((d&1)<<22)|((s&14)>>1)|((s&1)<<5));
} 

static void emit_fnegd(int s,int d)
{
  assem_debug("fnegd s%d,d%d",d,s);
  output_w32(0xeeb10b40|((d&7)<<12)|(s&7));
} 

static void emit_fadds(int s1,int s2,int d)
{
  assem_debug("fadds s%d,s%d,s%d",d,s1,s2);
  output_w32(0xee300a00|((d&14)<<11)|((d&1)<<22)|((s1&14)<<15)|((s1&1)<<7)|((s2&14)>>1)|((s2&1)<<5));
} 

static void emit_faddd(int s1,int s2,int d)
{
  assem_debug("faddd d%d,d%d,d%d",d,s1,s2);
  output_w32(0xee300b00|((d&7)<<12)|((s1&7)<<16)|(s2&7));
} 

static void emit_fsubs(int s1,int s2,int d)
{
  assem_debug("fsubs s%d,s%d,s%d",d,s1,s2);
  output_w32(0xee300a40|((d&14)<<11)|((d&1)<<22)|((s1&14)<<15)|((s1&1)<<7)|((s2&14)>>1)|((s2&1)<<5));
} 

static void emit_fsubd(int s1,int s2,int d)
{
  assem_debug("fsubd d%d,d%d,d%d",d,s1,s2);
  output_w32(0xee300b40|((d&7)<<12)|((s1&7)<<16)|(s2&7));
} 

static void emit_fmuls(int s1,int s2,int d)
{
  assem_debug("fmuls s%d,s%d,s%d",d,s1,s2);
  output_w32(0xee200a00|((d&14)<<11)|((d&1)<<22)|((s1&14)<<15)|((s1&1)<<7)|((s2&14)>>1)|((s2&1)<<5));
} 

static void emit_fmuld(int s1,int s2,int d)
{
  assem_debug("fmuld d%d,d%d,d%d",d,s1,s2);
  output_w32(0xee200b00|((d&7)<<12)|((s1&7)<<16)|(s2&7));
} 

static void emit_fdivs(int s1,int s2,int d)
{
  assem_debug("fdivs s%d,s%d,s%d",d,s1,s2);
  output_w32(0xee800a00|((d&14)<<11)|((d&1)<<22)|((s1&14)<<15)|((s1&1)<<7)|((s2&14)>>1)|((s2&1)<<5));
} 

static void emit_fdivd(int s1,int s2,int d)
{
  assem_debug("fdivd d%d,d%d,d%d",d,s1,s2);
  output_w32(0xee800b00|((d&7)<<12)|((s1&7)<<16)|(s2&7));
} 

static void emit_fcmps(int x,int y)
{
  assem_debug("fcmps s14, s15");
  output_w32(0xeeb47a67);
} 

static void emit_fcmpd(int x,int y)
{
  assem_debug("fcmpd d6, d7");
  output_w32(0xeeb46b47);
} 

static void emit_fmstat()
{
  assem_debug("fmstat");
  output_w32(0xeef1fa10);
} 

static void emit_bicne_imm(int rs,int imm,int rt)
{
  u_int armval, ret;
  ret = genimm(imm,&armval);
  assert(ret);
  assem_debug("bicne %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0x13c00000|rd_rn_rm(rt,rs,0)|armval);
}

static void emit_biccs_imm(int rs,int imm,int rt)
{
  u_int armval, ret;
  ret = genimm(imm,&armval);
  assert(ret);
  assem_debug("biccs %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0x23c00000|rd_rn_rm(rt,rs,0)|armval);
}

static void emit_bicvc_imm(int rs,int imm,int rt)
{
  u_int armval, ret;
  ret = genimm(imm,&armval);
  assert(ret);
  assem_debug("bicvc %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0x73c00000|rd_rn_rm(rt,rs,0)|armval);
}

static void emit_bichi_imm(int rs,int imm,int rt)
{
  u_int armval, ret;
  ret = genimm(imm,&armval);
  assert(ret);
  assem_debug("bichi %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0x83c00000|rd_rn_rm(rt,rs,0)|armval);
}

static void emit_orrvs_imm(int rs,int imm,int rt)
{
  u_int armval, ret;
  ret = genimm(imm,&armval);
  assert(ret);
  assem_debug("orrvs %s,%s,#%d",regname[rt],regname[rs],imm);
  output_w32(0x63800000|rd_rn_rm(rt,rs,0)|armval);
}

static void emit_jno_unlikely(int a)
{
  //emit_jno(a);
  assem_debug("addvc pc,pc,#? (%x)",/*a-(int)out-8,*/a);
  output_w32(0x72800000|rd_rn_rm(15,15,0));
}

// Save registers before function call
static void save_regs(u_int reglist)
{
  reglist&=0x100f; // only save the caller-save registers, r0-r3, r12
  if(!reglist) return;
  assem_debug("stmia fp,{");
  if(reglist&1) assem_debug("r0, ");
  if(reglist&2) assem_debug("r1, ");
  if(reglist&4) assem_debug("r2, ");
  if(reglist&8) assem_debug("r3, ");
  if(reglist&0x1000) assem_debug("r12");
  assem_debug("}");
  output_w32(0xe88b0000|reglist);
}
// Restore registers after function call
static void restore_regs(u_int reglist)
{
  reglist&=0x100f; // only restore the caller-save registers, r0-r3, r12
  if(!reglist) return;
  assem_debug("ldmia fp,{");
  if(reglist&1) assem_debug("r0, ");
  if(reglist&2) assem_debug("r1, ");
  if(reglist&4) assem_debug("r2, ");
  if(reglist&8) assem_debug("r3, ");
  if(reglist&0x1000) assem_debug("r12");
  assem_debug("}");
  output_w32(0xe89b0000|reglist);
}

// Write back consts using r14 so we don't disturb the other registers
static void wb_consts(signed char i_regmap[],uint64_t i_is32,u_int i_dirty,int i)
{
  int hr;
  for(hr=0;hr<HOST_REGS;hr++) {
    if(hr!=EXCLUDE_REG&&i_regmap[hr]>=0&&((i_dirty>>hr)&1)) {
      if(((regs[i].isconst>>hr)&1)&&i_regmap[hr]>0) {
        if(i_regmap[hr]<64 || !((i_is32>>(i_regmap[hr]&63))&1) ) {
          int value=constmap[i][hr];
          if(value==0) {
            emit_zeroreg(HOST_TEMPREG);
          }
          else {
            emit_movimm(value,HOST_TEMPREG);
          }
          emit_storereg(i_regmap[hr],HOST_TEMPREG);
          if((i_is32>>i_regmap[hr])&1) {
            if(value!=-1&&value!=0) emit_sarimm(HOST_TEMPREG,31,HOST_TEMPREG);
            emit_storereg(i_regmap[hr]|64,HOST_TEMPREG);
          }
        }
      }
    }
  }
}

/* Stubs/epilogue */

static void literal_pool(int n)
{
  if(!literalcount) return;
  if(n) {
    if((int)out-literals[0][0]<4096-n) return;
  }
  u_int *ptr;
  int i;
  for(i=0;i<literalcount;i++)
  {
    ptr=(u_int *)literals[i][0];
    u_int offset=(u_int)out-(u_int)ptr-8;
    assert(offset<4096);
    assert(!(offset&3));
    *ptr|=offset;
    output_w32(literals[i][1]);
  }
  literalcount=0;
}

static void literal_pool_jumpover(int n)
{
  if(!literalcount) return;
  if(n) {
    if((int)out-literals[0][0]<4096-n) return;
  }
  int jaddr=(int)out;
  emit_jmp(0);
  literal_pool(0);
  set_jump_target(jaddr,(int)out);
}

static void emit_extjump2(int addr, int target, int linker)
{
  u_char *ptr=(u_char *)addr;
  assert((ptr[3]&0x0e)==0xa);
  emit_loadlp(target,0);
  emit_loadlp(addr,1);
  //assert(addr>=0x7000000&&addr<0x7FFFFFF);
  //assert((target>=0x80000000&&target<0x80800000)||(target>0xA4000000&&target<0xA4001000));
//DEBUG >
#ifdef DEBUG_CYCLE_COUNT
  emit_readword((int)&last_count,ECX);
  emit_add(HOST_CCREG,ECX,HOST_CCREG);
  emit_readword((int)&next_interupt,ECX);
  emit_writeword(HOST_CCREG,(int)&Count);
  emit_sub(HOST_CCREG,ECX,HOST_CCREG);
  emit_writeword(ECX,(int)&last_count);
#endif
//DEBUG <
  emit_jmp(linker);
}

static void emit_extjump(int addr, int target)
{
  emit_extjump2(addr, target, (int)dyna_linker);
}
static void emit_extjump_ds(int addr, int target)
{
  emit_extjump2(addr, target, (int)dyna_linker_ds);
}

static void do_readstub(int n)
{
  assem_debug("do_readstub %x",start+stubs[n][3]*4);
  literal_pool(256);
  set_jump_target(stubs[n][1],(int)out);
  int type=stubs[n][0];
  int i=stubs[n][3];
  int rs=stubs[n][4];
  struct regstat *i_regs=(struct regstat *)stubs[n][5];
  u_int reglist=stubs[n][7];
  signed char *i_regmap=i_regs->regmap;
  int addr=get_reg(i_regmap,AGEN1+(i&1));
  int rth,rt;
  int ds;
  if(itype[i]==C1LS||itype[i]==LOADLR) {
    rth=get_reg(i_regmap,FTEMP|64);
    rt=get_reg(i_regmap,FTEMP);
  }else{
    rth=get_reg(i_regmap,rt1[i]|64);
    rt=get_reg(i_regmap,rt1[i]);
  }
  assert(rs>=0);
  if(addr<0) addr=rt;
  if(addr<0&&itype[i]!=C1LS&&itype[i]!=LOADLR) addr=get_reg(i_regmap,-1);
  assert(addr>=0);
  int ftable=0;
  if(type==LOADB_STUB||type==LOADBU_STUB)
    ftable=(int)readmemb;
  if(type==LOADH_STUB||type==LOADHU_STUB)
    ftable=(int)readmemh;
  if(type==LOADW_STUB)
    ftable=(int)readmem;
  if(type==LOADD_STUB)
    ftable=(int)readmemd;
  emit_writeword(rs,(int)&address);
  //emit_pusha();
  save_regs(reglist);
  ds=i_regs!=&regs[i];
  int real_rs=(itype[i]==LOADLR)?-1:get_reg(i_regmap,rs1[i]);
  u_int cmask=ds?-1:(0x100f|~i_regs->wasconst);
  if(!ds) load_all_consts(regs[i].regmap_entry,regs[i].was32,regs[i].wasdirty&~(1<<addr)&(real_rs<0?-1:~(1<<real_rs))&0x100f,i);
  wb_dirtys(i_regs->regmap_entry,i_regs->was32,i_regs->wasdirty&cmask&~(1<<addr)&(real_rs<0?-1:~(1<<real_rs)));
  if(!ds) wb_consts(regs[i].regmap_entry,regs[i].was32,regs[i].wasdirty&~(1<<addr)&(real_rs<0?-1:~(1<<real_rs))&~0x100f,i);
  emit_shrimm(rs,16,1);
  int cc=get_reg(i_regmap,CCREG);
  if(cc<0) {
    emit_loadreg(CCREG,2);
  }
  emit_movimm(ftable,0);
  emit_addimm(cc<0?2:cc,2*stubs[n][6]+2,2);
  emit_movimm(start+stubs[n][3]*4+(((regs[i].was32>>rs1[i])&1)<<1)+ds,3);
  //emit_readword((int)&last_count,temp);
  //emit_add(cc,temp,cc);
  //emit_writeword(cc,(int)&Count);
  //emit_mov(15,14);
  emit_call((int)&indirect_jump_indexed);
  //emit_callreg(rs);
  //emit_readword_dualindexedx4(rs,HOST_TEMPREG,15);
  // We really shouldn't need to update the count here,
  // but not doing so causes random crashes...
  emit_readword((int)&Count,HOST_TEMPREG);
  emit_readword((int)&next_interupt,2);
  emit_addimm(HOST_TEMPREG,-2*stubs[n][6]-2,HOST_TEMPREG);
  emit_writeword(2,(int)&last_count);
  emit_sub(HOST_TEMPREG,2,cc<0?HOST_TEMPREG:cc);
  if(cc<0) {
    emit_storereg(CCREG,HOST_TEMPREG);
  }
  //emit_popa();
  restore_regs(reglist);
  //if((cc=get_reg(regmap,CCREG))>=0) {
  //  emit_loadreg(CCREG,cc);
  //}
  if(rt>=0) {
    if(type==LOADB_STUB)
      emit_movsbl((int)&readmem_dword,rt);
    if(type==LOADBU_STUB)
      emit_movzbl((int)&readmem_dword,rt);
    if(type==LOADH_STUB)
      emit_movswl((int)&readmem_dword,rt);
    if(type==LOADHU_STUB)
      emit_movzwl((int)&readmem_dword,rt);
    if(type==LOADW_STUB)
      emit_readword((int)&readmem_dword,rt);
    if(type==LOADD_STUB) {
      emit_readword((int)&readmem_dword,rt);
      if(rth>=0) emit_readword(((int)&readmem_dword)+4,rth);
    }
  }
  emit_jmp(stubs[n][2]); // return address
}

static void inline_readstub(int type, int i, u_int addr, signed char regmap[], int target, int adj, u_int reglist)
{
  int rs=get_reg(regmap,target);
  int rth=get_reg(regmap,target|64);
  int rt=get_reg(regmap,target);
  if(rs<0) rs=get_reg(regmap,-1);
  assert(rs>=0);
  int ftable=0;
  if(type==LOADB_STUB||type==LOADBU_STUB)
    ftable=(int)readmemb;
  if(type==LOADH_STUB||type==LOADHU_STUB)
    ftable=(int)readmemh;
  if(type==LOADW_STUB)
    ftable=(int)readmem;
  if(type==LOADD_STUB)
    ftable=(int)readmemd;
  emit_writeword(rs,(int)&address);
  //emit_pusha();
  save_regs(reglist);
  if((signed int)addr>=(signed int)0xC0000000) {
    // Theoretically we can have a pagefault here, if the TLB has never
    // been enabled and the address is outside the range 80000000..BFFFFFFF
    // Write out the registers so the pagefault can be handled.  This is
    // a very rare case and likely represents a bug.
    int ds=regmap!=regs[i].regmap;
    if(!ds) load_all_consts(regs[i].regmap_entry,regs[i].was32,regs[i].wasdirty,i);
    if(!ds) wb_dirtys(regs[i].regmap_entry,regs[i].was32,regs[i].wasdirty);
    else wb_dirtys(branch_regs[i-1].regmap_entry,branch_regs[i-1].was32,branch_regs[i-1].wasdirty);
  }
  //emit_shrimm(rs,16,1);
  int cc=get_reg(regmap,CCREG);
  if(cc<0) {
    emit_loadreg(CCREG,2);
  }
  //emit_movimm(ftable,0);
  emit_movimm(((u_int *)ftable)[addr>>16],0);
  //emit_readword((int)&last_count,12);
  emit_addimm(cc<0?2:cc,CLOCK_DIVIDER*(adj+1),2);
  if((signed int)addr>=(signed int)0xC0000000) {
    // Pagefault address
    int ds=regmap!=regs[i].regmap;
    emit_movimm(start+i*4+(((regs[i].was32>>rs1[i])&1)<<1)+ds,3);
  }
  //emit_add(12,2,2);
  //emit_writeword(2,(int)&Count);
  //emit_call(((u_int *)ftable)[addr>>16]);
  emit_call((int)&indirect_jump);
  // We really shouldn't need to update the count here,
  // but not doing so causes random crashes...
  emit_readword((int)&Count,HOST_TEMPREG);
  emit_readword((int)&next_interupt,2);
  emit_addimm(HOST_TEMPREG,-CLOCK_DIVIDER*(adj+1),HOST_TEMPREG);
  emit_writeword(2,(int)&last_count);
  emit_sub(HOST_TEMPREG,2,cc<0?HOST_TEMPREG:cc);
  if(cc<0) {
    emit_storereg(CCREG,HOST_TEMPREG);
  }
  //emit_popa();
  restore_regs(reglist);
  if(rt>=0) {
    if(type==LOADB_STUB)
      emit_movsbl((int)&readmem_dword,rt);
    if(type==LOADBU_STUB)
      emit_movzbl((int)&readmem_dword,rt);
    if(type==LOADH_STUB)
      emit_movswl((int)&readmem_dword,rt);
    if(type==LOADHU_STUB)
      emit_movzwl((int)&readmem_dword,rt);
    if(type==LOADW_STUB)
      emit_readword((int)&readmem_dword,rt);
    if(type==LOADD_STUB) {
      emit_readword((int)&readmem_dword,rt);
      if(rth>=0) emit_readword(((int)&readmem_dword)+4,rth);
    }
  }
}

static void do_writestub(int n)
{
  assem_debug("do_writestub %x",start+stubs[n][3]*4);
  literal_pool(256);
  set_jump_target(stubs[n][1],(int)out);
  int type=stubs[n][0];
  int i=stubs[n][3];
  int rs=stubs[n][4];
  struct regstat *i_regs=(struct regstat *)stubs[n][5];
  u_int reglist=stubs[n][7];
  signed char *i_regmap=i_regs->regmap;
  int addr=get_reg(i_regmap,AGEN1+(i&1));
  int rth,rt,r;
  int ds;
  if(itype[i]==C1LS) {
    rth=get_reg(i_regmap,FTEMP|64);
    rt=get_reg(i_regmap,r=FTEMP);
  }else{
    rth=get_reg(i_regmap,rs2[i]|64);
    rt=get_reg(i_regmap,r=rs2[i]);
  }
  assert(rs>=0);
  assert(rt>=0);
  if(addr<0) addr=get_reg(i_regmap,-1);
  assert(addr>=0);
  int ftable=0;
  if(type==STOREB_STUB)
    ftable=(int)writememb;
  if(type==STOREH_STUB)
    ftable=(int)writememh;
  if(type==STOREW_STUB)
    ftable=(int)writemem;
  if(type==STORED_STUB)
    ftable=(int)writememd;
  emit_writeword(rs,(int)&address);
  //emit_shrimm(rs,16,rs);
  //emit_movmem_indexedx4(ftable,rs,rs);
  if(type==STOREB_STUB)
    emit_writebyte(rt,(int)&cpu_byte);
  if(type==STOREH_STUB)
    emit_writehword(rt,(int)&hword);
  if(type==STOREW_STUB)
    emit_writeword(rt,(int)&word);
  if(type==STORED_STUB) {
    emit_writeword(rt,(int)&dword);
    emit_writeword(r?rth:rt,(int)&dword+4);
  }
  //emit_pusha();
  save_regs(reglist);
  ds=i_regs!=&regs[i];
  int real_rs=get_reg(i_regmap,rs1[i]);
  u_int cmask=ds?-1:(0x100f|~i_regs->wasconst);
  if(!ds) load_all_consts(regs[i].regmap_entry,regs[i].was32,regs[i].wasdirty&~(1<<addr)&(real_rs<0?-1:~(1<<real_rs))&0x100f,i);
  wb_dirtys(i_regs->regmap_entry,i_regs->was32,i_regs->wasdirty&cmask&~(1<<addr)&(real_rs<0?-1:~(1<<real_rs)));
  if(!ds) wb_consts(regs[i].regmap_entry,regs[i].was32,regs[i].wasdirty&~(1<<addr)&(real_rs<0?-1:~(1<<real_rs))&~0x100f,i);
  emit_shrimm(rs,16,1);
  int cc=get_reg(i_regmap,CCREG);
  if(cc<0) {
    emit_loadreg(CCREG,2);
  }
  emit_movimm(ftable,0);
  emit_addimm(cc<0?2:cc,2*stubs[n][6]+2,2);
  emit_movimm(start+stubs[n][3]*4+(((regs[i].was32>>rs1[i])&1)<<1)+ds,3);
  //emit_readword((int)&last_count,temp);
  //emit_addimm(cc,2*stubs[n][5]+2,cc);
  //emit_add(cc,temp,cc);
  //emit_writeword(cc,(int)&Count);
  emit_call((int)&indirect_jump_indexed);
  //emit_callreg(rs);
  emit_readword((int)&Count,HOST_TEMPREG);
  emit_readword((int)&next_interupt,2);
  emit_addimm(HOST_TEMPREG,-2*stubs[n][6]-2,HOST_TEMPREG);
  emit_writeword(2,(int)&last_count);
  emit_sub(HOST_TEMPREG,2,cc<0?HOST_TEMPREG:cc);
  if(cc<0) {
    emit_storereg(CCREG,HOST_TEMPREG);
  }
  //emit_popa();
  restore_regs(reglist);
  //if((cc=get_reg(regmap,CCREG))>=0) {
  //  emit_loadreg(CCREG,cc);
  //}
  emit_jmp(stubs[n][2]); // return address
}

static void inline_writestub(int type, int i, u_int addr, signed char regmap[], int target, int adj, u_int reglist)
{
  int rs=get_reg(regmap,-1);
  int rth=get_reg(regmap,target|64);
  int rt=get_reg(regmap,target);
  assert(rs>=0);
  assert(rt>=0);
  int ftable=0;
  if(type==STOREB_STUB)
    ftable=(int)writememb;
  if(type==STOREH_STUB)
    ftable=(int)writememh;
  if(type==STOREW_STUB)
    ftable=(int)writemem;
  if(type==STORED_STUB)
    ftable=(int)writememd;
  emit_writeword(rs,(int)&address);
  //emit_shrimm(rs,16,rs);
  //emit_movmem_indexedx4(ftable,rs,rs);
  if(type==STOREB_STUB)
    emit_writebyte(rt,(int)&cpu_byte);
  if(type==STOREH_STUB)
    emit_writehword(rt,(int)&hword);
  if(type==STOREW_STUB)
    emit_writeword(rt,(int)&word);
  if(type==STORED_STUB) {
    emit_writeword(rt,(int)&dword);
    emit_writeword(target?rth:rt,(int)&dword+4);
  }
  //emit_pusha();
  save_regs(reglist);
  if((signed int)addr>=(signed int)0xC0000000) {
    // Theoretically we can have a pagefault here, if the TLB has never
    // been enabled and the address is outside the range 80000000..BFFFFFFF
    // Write out the registers so the pagefault can be handled.  This is
    // a very rare case and likely represents a bug.
    int ds=regmap!=regs[i].regmap;
    if(!ds) load_all_consts(regs[i].regmap_entry,regs[i].was32,regs[i].wasdirty,i);
    if(!ds) wb_dirtys(regs[i].regmap_entry,regs[i].was32,regs[i].wasdirty);
    else wb_dirtys(branch_regs[i-1].regmap_entry,branch_regs[i-1].was32,branch_regs[i-1].wasdirty);
  }
  //emit_shrimm(rs,16,1);
  int cc=get_reg(regmap,CCREG);
  if(cc<0) {
    emit_loadreg(CCREG,2);
  }
  //emit_movimm(ftable,0);
  emit_movimm(((u_int *)ftable)[addr>>16],0);
  //emit_readword((int)&last_count,12);
  emit_addimm(cc<0?2:cc,CLOCK_DIVIDER*(adj+1),2);
  if((signed int)addr>=(signed int)0xC0000000) {
    // Pagefault address
    int ds=regmap!=regs[i].regmap;
    emit_movimm(start+i*4+(((regs[i].was32>>rs1[i])&1)<<1)+ds,3);
  }
  //emit_add(12,2,2);
  //emit_writeword(2,(int)&Count);
  //emit_call(((u_int *)ftable)[addr>>16]);
  emit_call((int)&indirect_jump);
  emit_readword((int)&Count,HOST_TEMPREG);
  emit_readword((int)&next_interupt,2);
  emit_addimm(HOST_TEMPREG,-CLOCK_DIVIDER*(adj+1),HOST_TEMPREG);
  emit_writeword(2,(int)&last_count);
  emit_sub(HOST_TEMPREG,2,cc<0?HOST_TEMPREG:cc);
  if(cc<0) {
    emit_storereg(CCREG,HOST_TEMPREG);
  }
  //emit_popa();
  restore_regs(reglist);
}

static void do_unalignedwritestub(int n)
{
  set_jump_target(stubs[n][1],(int)out);
  output_w32(0xef000000);
  emit_jmp(stubs[n][2]); // return address
}

void printregs(int edi,int esi,int ebp,int esp,int b,int d,int c,int a)
{
  DebugMessage(M64MSG_VERBOSE, "regs: %x %x %x %x %x %x %x (%x)",a,b,c,d,ebp,esi,edi,(&edi)[-1]);
}

static void do_invstub(int n)
{
  literal_pool(20);
  u_int reglist=stubs[n][3];
  set_jump_target(stubs[n][1],(int)out);
  save_regs(reglist);
  if(stubs[n][4]!=0) emit_mov(stubs[n][4],0);
  emit_call((int)&invalidate_addr);
  restore_regs(reglist);
  emit_jmp(stubs[n][2]); // return address
}

static int do_dirty_stub(int i)
{
  assem_debug("do_dirty_stub %x",start+i*4);
  // Careful about the code output here, verify_dirty needs to parse it.
  #ifdef ARMv5_ONLY
  emit_loadlp((int)start<(int)0xC0000000?(int)source:(int)start,1);
  emit_loadlp((int)copy,2);
  emit_loadlp(slen*4,3);
  #else
  emit_movw(((int)start<(int)0xC0000000?(u_int)source:(u_int)start)&0x0000FFFF,1);
  emit_movw(((u_int)copy)&0x0000FFFF,2);
  emit_movt(((int)start<(int)0xC0000000?(u_int)source:(u_int)start)&0xFFFF0000,1);
  emit_movt(((u_int)copy)&0xFFFF0000,2);
  emit_movw(slen*4,3);
  #endif
  emit_movimm(start+i*4,0);
  emit_call((int)start<(int)0xC0000000?(int)&verify_code:(int)&verify_code_vm);
  int entry=(int)out;
  load_regs_entry(i);
  if(entry==(int)out) entry=instr_addr[i];
  emit_jmp(instr_addr[i]);
  return entry;
}

static void do_dirty_stub_ds()
{
  // Careful about the code output here, verify_dirty needs to parse it.
  #ifdef ARMv5_ONLY
  emit_loadlp((int)start<(int)0xC0000000?(int)source:(int)start,1);
  emit_loadlp((int)copy,2);
  emit_loadlp(slen*4,3);
  #else
  emit_movw(((int)start<(int)0xC0000000?(u_int)source:(u_int)start)&0x0000FFFF,1);
  emit_movw(((u_int)copy)&0x0000FFFF,2);
  emit_movt(((int)start<(int)0xC0000000?(u_int)source:(u_int)start)&0xFFFF0000,1);
  emit_movt(((u_int)copy)&0xFFFF0000,2);
  emit_movw(slen*4,3);
  #endif
  emit_movimm(start+1,0);
  emit_call((int)&verify_code_ds);
}

static void do_cop1stub(int n)
{
  literal_pool(256);
  assem_debug("do_cop1stub %x",start+stubs[n][3]*4);
  set_jump_target(stubs[n][1],(int)out);
  int i=stubs[n][3];
  int rs=stubs[n][4];
  struct regstat *i_regs=(struct regstat *)stubs[n][5];
  int ds=stubs[n][6];
  if(!ds) {
    load_all_consts(regs[i].regmap_entry,regs[i].was32,regs[i].wasdirty,i);
    //if(i_regs!=&regs[i]) DebugMessage(M64MSG_VERBOSE, "oops: regs[i]=%x i_regs=%x",(int)&regs[i],(int)i_regs);
  }
  //else {DebugMessage(M64MSG_ERROR, "fp exception in delay slot");}
  wb_dirtys(i_regs->regmap_entry,i_regs->was32,i_regs->wasdirty);
  if(regs[i].regmap_entry[HOST_CCREG]!=CCREG) emit_loadreg(CCREG,HOST_CCREG);
  emit_movimm(start+(i-ds)*4,EAX); // Get PC
  emit_addimm(HOST_CCREG,CLOCK_DIVIDER*ccadj[i],HOST_CCREG); // CHECK: is this right?  There should probably be an extra cycle...
  emit_jmp(ds?(int)fp_exception_ds:(int)fp_exception);
}

/* TLB */

static int do_tlb_r(int s,int ar,int map,int cache,int x,int a,int shift,int c,u_int addr)
{
  if(c) {
    if((signed int)addr>=(signed int)0xC0000000) {
      // address_generation already loaded the const
      emit_readword_dualindexedx4(FP,map,map);
    }
    else
      return -1; // No mapping
  }
  else {
    assert(s!=map);
    if(cache>=0) {
      // Use cached offset to memory map
      emit_addsr12(cache,s,map);
    }else{
      emit_movimm(((int)memory_map-(int)&dynarec_local)>>2,map);
      emit_addsr12(map,s,map);
    }
    // Schedule this while we wait on the load
    //if(x) emit_xorimm(s,x,ar);
    if(shift>=0) emit_shlimm(s,3,shift);
    if(~a) emit_andimm(s,a,ar);
    emit_readword_dualindexedx4(FP,map,map);
  }
  return map;
}
static int do_tlb_r_branch(int map, int c, u_int addr, int *jaddr)
{
  if(!c||(signed int)addr>=(signed int)0xC0000000) {
    emit_test(map,map);
    *jaddr=(int)out;
    emit_js(0);
  }
  return map;
}

static void gen_tlb_addr_r(int ar, int map) {
  if(map>=0) {
    assem_debug("add %s,%s,%s lsl #2",regname[ar],regname[ar],regname[map]);
    output_w32(0xe0800100|rd_rn_rm(ar,ar,map));
  }
}

static int do_tlb_w(int s,int ar,int map,int cache,int x,int c,u_int addr)
{
  if(c) {
    if(addr<0x80800000||addr>=0xC0000000) {
      // address_generation already loaded the const
      emit_readword_dualindexedx4(FP,map,map);
    }
    else
      return -1; // No mapping
  }
  else {
    assert(s!=map);
    if(cache>=0) {
      // Use cached offset to memory map
      emit_addsr12(cache,s,map);
    }else{
      emit_movimm(((int)memory_map-(int)&dynarec_local)>>2,map);
      emit_addsr12(map,s,map);
    }
    // Schedule this while we wait on the load
    //if(x) emit_xorimm(s,x,ar);
    emit_readword_dualindexedx4(FP,map,map);
  }
  return map;
}
static void do_tlb_w_branch(int map, int c, u_int addr, int *jaddr)
{
  if(!c||addr<0x80800000||addr>=0xC0000000) {
    emit_testimm(map,0x40000000);
    *jaddr=(int)out;
    emit_jne(0);
  }
}

static void gen_tlb_addr_w(int ar, int map) {
  if(map>=0) {
    assem_debug("add %s,%s,%s lsl #2",regname[ar],regname[ar],regname[map]);
    output_w32(0xe0800100|rd_rn_rm(ar,ar,map));
  }
}

// This reverses the above operation
static void gen_orig_addr_w(int ar, int map) {
  if(map>=0) {
    assem_debug("sub %s,%s,%s lsl #2",regname[ar],regname[ar],regname[map]);
    output_w32(0xe0400100|rd_rn_rm(ar,ar,map));
  }
}

// Generate the address of the memory_map entry, relative to dynarec_local
static void generate_map_const(u_int addr,int reg) {
  //DebugMessage(M64MSG_VERBOSE, "generate_map_const(%x,%s)",addr,regname[reg]);
  emit_movimm((addr>>12)+(((u_int)memory_map-(u_int)&dynarec_local)>>2),reg);
}

/* Special assem */

static void shift_assemble_arm(int i,struct regstat *i_regs)
{
  if(rt1[i]) {
    if(opcode2[i]<=0x07) // SLLV/SRLV/SRAV
    {
      signed char s,t,shift;
      t=get_reg(i_regs->regmap,rt1[i]);
      s=get_reg(i_regs->regmap,rs1[i]);
      shift=get_reg(i_regs->regmap,rs2[i]);
      if(t>=0){
        if(rs1[i]==0)
        {
          emit_zeroreg(t);
        }
        else if(rs2[i]==0)
        {
          assert(s>=0);
          if(s!=t) emit_mov(s,t);
        }
        else
        {
          emit_andimm(shift,31,HOST_TEMPREG);
          if(opcode2[i]==4) // SLLV
          {
            emit_shl(s,HOST_TEMPREG,t);
          }
          if(opcode2[i]==6) // SRLV
          {
            emit_shr(s,HOST_TEMPREG,t);
          }
          if(opcode2[i]==7) // SRAV
          {
            emit_sar(s,HOST_TEMPREG,t);
          }
        }
      }
    } else { // DSLLV/DSRLV/DSRAV
      signed char sh,sl,th,tl,shift;
      th=get_reg(i_regs->regmap,rt1[i]|64);
      tl=get_reg(i_regs->regmap,rt1[i]);
      sh=get_reg(i_regs->regmap,rs1[i]|64);
      sl=get_reg(i_regs->regmap,rs1[i]);
      shift=get_reg(i_regs->regmap,rs2[i]);
      if(tl>=0){
        if(rs1[i]==0)
        {
          emit_zeroreg(tl);
          if(th>=0) emit_zeroreg(th);
        }
        else if(rs2[i]==0)
        {
          assert(sl>=0);
          if(sl!=tl) emit_mov(sl,tl);
          if(th>=0&&sh!=th) emit_mov(sh,th);
        }
        else
        {
          // FIXME: What if shift==tl ?
          assert(shift!=tl);
          int temp=get_reg(i_regs->regmap,-1);
          int real_th=th;
          if(th<0&&opcode2[i]!=0x14) {th=temp;} // DSLLV doesn't need a temporary register
          assert(sl>=0);
          assert(sh>=0);
          emit_andimm(shift,31,HOST_TEMPREG);
          if(opcode2[i]==0x14) // DSLLV
          {
            if(th>=0) emit_shl(sh,HOST_TEMPREG,th);
            emit_rsbimm(HOST_TEMPREG,32,HOST_TEMPREG);
            emit_orrshr(sl,HOST_TEMPREG,th);
            emit_andimm(shift,31,HOST_TEMPREG);
            emit_testimm(shift,32);
            emit_shl(sl,HOST_TEMPREG,tl);
            if(th>=0) emit_cmovne_reg(tl,th);
            emit_cmovne_imm(0,tl);
          }
          if(opcode2[i]==0x16) // DSRLV
          {
            assert(th>=0);
            emit_shr(sl,HOST_TEMPREG,tl);
            emit_rsbimm(HOST_TEMPREG,32,HOST_TEMPREG);
            emit_orrshl(sh,HOST_TEMPREG,tl);
            emit_andimm(shift,31,HOST_TEMPREG);
            emit_testimm(shift,32);
            emit_shr(sh,HOST_TEMPREG,th);
            emit_cmovne_reg(th,tl);
            if(real_th>=0) emit_cmovne_imm(0,th);
          }
          if(opcode2[i]==0x17) // DSRAV
          {
            assert(th>=0);
            emit_shr(sl,HOST_TEMPREG,tl);
            emit_rsbimm(HOST_TEMPREG,32,HOST_TEMPREG);
            if(real_th>=0) {
              assert(temp>=0);
              emit_sarimm(th,31,temp);
            }
            emit_orrshl(sh,HOST_TEMPREG,tl);
            emit_andimm(shift,31,HOST_TEMPREG);
            emit_testimm(shift,32);
            emit_sar(sh,HOST_TEMPREG,th);
            emit_cmovne_reg(th,tl);
            if(real_th>=0) emit_cmovne_reg(temp,th);
          }
        }
      }
    }
  }
}
#define shift_assemble shift_assemble_arm

static void loadlr_assemble_arm(int i,struct regstat *i_regs)
{
  int s,th,tl,temp,temp2,addr,map=-1,cache=-1;
  int offset;
  int jaddr=0;
  int memtarget,c=0;
  u_int hr,reglist=0;
  th=get_reg(i_regs->regmap,rt1[i]|64);
  tl=get_reg(i_regs->regmap,rt1[i]);
  s=get_reg(i_regs->regmap,rs1[i]);
  temp=get_reg(i_regs->regmap,-1);
  temp2=get_reg(i_regs->regmap,FTEMP);
  addr=get_reg(i_regs->regmap,AGEN1+(i&1));
  assert(addr<0);
  offset=imm[i];
  for(hr=0;hr<HOST_REGS;hr++) {
    if(i_regs->regmap[hr]>=0) reglist|=1<<hr;
  }
  reglist|=1<<temp;
  if(offset||s<0||c) addr=temp2;
  else addr=s;
  if(s>=0) {
    c=(i_regs->wasconst>>s)&1;
    memtarget=((signed int)(constmap[i][s]+offset))<(signed int)0x80800000;
    if(using_tlb&&((signed int)(constmap[i][s]+offset))>=(signed int)0xC0000000) memtarget=1;
  }
  if(!using_tlb) {
    if(!c) {
      #ifdef RAM_OFFSET
      map=get_reg(i_regs->regmap,ROREG);
      if(map<0) emit_loadreg(ROREG,map=HOST_TEMPREG);
      #endif
      emit_shlimm(addr,3,temp);
      if (opcode[i]==0x22||opcode[i]==0x26) {
        emit_andimm(addr,0xFFFFFFFC,temp2); // LWL/LWR
      }else{
        emit_andimm(addr,0xFFFFFFF8,temp2); // LDL/LDR
      }
      emit_cmpimm(addr,0x800000);
      jaddr=(int)out;
      emit_jno(0);
    }
    else {
      if (opcode[i]==0x22||opcode[i]==0x26) {
        emit_movimm(((constmap[i][s]+offset)<<3)&24,temp); // LWL/LWR
      }else{
        emit_movimm(((constmap[i][s]+offset)<<3)&56,temp); // LDL/LDR
      }
    }
  }else{ // using tlb
    int a;
    if(c) {
      a=-1;
    }else if (opcode[i]==0x22||opcode[i]==0x26) {
      a=0xFFFFFFFC; // LWL/LWR
    }else{
      a=0xFFFFFFF8; // LDL/LDR
    }
    map=get_reg(i_regs->regmap,TLREG);
    cache=get_reg(i_regs->regmap,MMREG); // Get cached offset to memory_map
    assert(map>=0);
    reglist&=~(1<<map);
    map=do_tlb_r(addr,temp2,map,cache,0,a,c?-1:temp,c,constmap[i][s]+offset);
    if(c) {
      if (opcode[i]==0x22||opcode[i]==0x26) {
        emit_movimm(((constmap[i][s]+offset)<<3)&24,temp); // LWL/LWR
      }else{
        emit_movimm(((constmap[i][s]+offset)<<3)&56,temp); // LDL/LDR
      }
    }
    do_tlb_r_branch(map,c,constmap[i][s]+offset,&jaddr);
  }
  if (opcode[i]==0x22||opcode[i]==0x26) { // LWL/LWR
    if(!c||memtarget) {
      //emit_readword_indexed((int)rdram-0x80000000,temp2,temp2);
      emit_readword_indexed_tlb(0,temp2,map,temp2);
      if(jaddr) add_stub(LOADW_STUB,jaddr,(int)out,i,temp2,(int)i_regs,ccadj[i],reglist);
    }
    else
      inline_readstub(LOADW_STUB,i,(constmap[i][s]+offset)&0xFFFFFFFC,i_regs->regmap,FTEMP,ccadj[i],reglist);
    if(rt1[i]) {
      assert(tl>=0);
      emit_andimm(temp,24,temp);
      if (opcode[i]==0x26) emit_xorimm(temp,24,temp); // LWR
      emit_movimm(-1,HOST_TEMPREG);
      if (opcode[i]==0x26) {
        emit_shr(temp2,temp,temp2);
        emit_bic_lsr(tl,HOST_TEMPREG,temp,tl);
      }else{
        emit_shl(temp2,temp,temp2);
        emit_bic_lsl(tl,HOST_TEMPREG,temp,tl);
      }
      emit_or(temp2,tl,tl);
    }
    //emit_storereg(rt1[i],tl); // DEBUG
  }
  if (opcode[i]==0x1A||opcode[i]==0x1B) { // LDL/LDR
    int temp2h=get_reg(i_regs->regmap,FTEMP|64);
    if(!c||memtarget) {
      //if(th>=0) emit_readword_indexed((int)rdram-0x80000000,temp2,temp2h);
      //emit_readword_indexed((int)rdram-0x7FFFFFFC,temp2,temp2);
      emit_readdword_indexed_tlb(0,temp2,map,temp2h,temp2);
      if(jaddr) add_stub(LOADD_STUB,jaddr,(int)out,i,temp2,(int)i_regs,ccadj[i],reglist);
    }
    else
      inline_readstub(LOADD_STUB,i,(constmap[i][s]+offset)&0xFFFFFFF8,i_regs->regmap,FTEMP,ccadj[i],reglist);
    if(rt1[i]) {
      assert(th>=0);
      assert(tl>=0);
      emit_testimm(temp,32);
      emit_andimm(temp,24,temp);
      if (opcode[i]==0x1A) { // LDL
        emit_rsbimm(temp,32,HOST_TEMPREG);
        emit_shl(temp2h,temp,temp2h);
        emit_orrshr(temp2,HOST_TEMPREG,temp2h);
        emit_movimm(-1,HOST_TEMPREG);
        emit_shl(temp2,temp,temp2);
        emit_cmove_reg(temp2h,th);
        emit_biceq_lsl(tl,HOST_TEMPREG,temp,tl);
        emit_bicne_lsl(th,HOST_TEMPREG,temp,th);
        emit_orreq(temp2,tl,tl);
        emit_orrne(temp2,th,th);
      }
      if (opcode[i]==0x1B) { // LDR
        emit_xorimm(temp,24,temp);
        emit_rsbimm(temp,32,HOST_TEMPREG);
        emit_shr(temp2,temp,temp2);
        emit_orrshl(temp2h,HOST_TEMPREG,temp2);
        emit_movimm(-1,HOST_TEMPREG);
        emit_shr(temp2h,temp,temp2h);
        emit_cmovne_reg(temp2,tl);
        emit_bicne_lsr(th,HOST_TEMPREG,temp,th);
        emit_biceq_lsr(tl,HOST_TEMPREG,temp,tl);
        emit_orrne(temp2h,th,th);
        emit_orreq(temp2h,tl,tl);
      }
    }
  }
}
#define loadlr_assemble loadlr_assemble_arm

static void cop0_assemble(int i,struct regstat *i_regs)
{
  if(opcode2[i]==0) // MFC0
  {
    if(rt1[i]) {
      signed char t=get_reg(i_regs->regmap,rt1[i]);
      char copr=(source[i]>>11)&0x1f;
      if(t>=0) {
        emit_addimm(FP,(int)&fake_pc-(int)&dynarec_local,0);
        emit_movimm((source[i]>>11)&0x1f,1);
        emit_writeword(0,(int)&PC);
        emit_writebyte(1,(int)&(fake_pc.f.r.nrd));
        if(copr==9) {
          emit_readword((int)&last_count,ECX);
          emit_loadreg(CCREG,HOST_CCREG); // TODO: do proper reg alloc
          emit_add(HOST_CCREG,ECX,HOST_CCREG);
          emit_addimm(HOST_CCREG,CLOCK_DIVIDER*ccadj[i],HOST_CCREG);
          emit_writeword(HOST_CCREG,(int)&Count);
        }
        emit_call((int)cached_interpreter_table.MFC0);
        emit_readword((int)&readmem_dword,t);
      }
    }
  }
  else if(opcode2[i]==4) // MTC0
  {
    signed char s=get_reg(i_regs->regmap,rs1[i]);
    char copr=(source[i]>>11)&0x1f;
    assert(s>=0);
    emit_writeword(s,(int)&readmem_dword);
    wb_register(rs1[i],i_regs->regmap,i_regs->dirty,i_regs->is32);
    emit_addimm(FP,(int)&fake_pc-(int)&dynarec_local,0);
    emit_movimm((source[i]>>11)&0x1f,1);
    emit_writeword(0,(int)&PC);
    emit_writebyte(1,(int)&(fake_pc.f.r.nrd));
    if(copr==9||copr==11||copr==12) {
      emit_readword((int)&last_count,ECX);
      emit_loadreg(CCREG,HOST_CCREG); // TODO: do proper reg alloc
      emit_add(HOST_CCREG,ECX,HOST_CCREG);
      emit_addimm(HOST_CCREG,CLOCK_DIVIDER*ccadj[i],HOST_CCREG);
      emit_writeword(HOST_CCREG,(int)&Count);
    }
    // What a mess.  The status register (12) can enable interrupts,
    // so needs a special case to handle a pending interrupt.
    // The interrupt must be taken immediately, because a subsequent
    // instruction might disable interrupts again.
    if(copr==12&&!is_delayslot) {
      emit_movimm(start+i*4+4,0);
      emit_movimm(0,1);
      emit_writeword(0,(int)&pcaddr);
      emit_writeword(1,(int)&pending_exception);
    }
    //else if(copr==12&&is_delayslot) emit_call((int)MTC0_R12);
    //else
    emit_call((int)cached_interpreter_table.MTC0);
    if(copr==9||copr==11||copr==12) {
      emit_readword((int)&Count,HOST_CCREG);
      emit_readword((int)&next_interupt,ECX);
      emit_addimm(HOST_CCREG,-CLOCK_DIVIDER*ccadj[i],HOST_CCREG);
      emit_sub(HOST_CCREG,ECX,HOST_CCREG);
      emit_writeword(ECX,(int)&last_count);
      emit_storereg(CCREG,HOST_CCREG);
    }
    if(copr==12) {
      assert(!is_delayslot);
      emit_readword((int)&pending_exception,14);
    }
    emit_loadreg(rs1[i],s);
    if(get_reg(i_regs->regmap,rs1[i]|64)>=0)
      emit_loadreg(rs1[i]|64,get_reg(i_regs->regmap,rs1[i]|64));
    if(copr==12) {
      emit_test(14,14);
      emit_jne((int)&do_interrupt);
    }
    cop1_usable=0;
  }
  else
  {
    assert(opcode2[i]==0x10);
    if((source[i]&0x3f)==0x01) // TLBR
      emit_call((int)cached_interpreter_table.TLBR);
    if((source[i]&0x3f)==0x02) // TLBWI
      emit_call((int)TLBWI_new);
    if((source[i]&0x3f)==0x06) { // TLBWR
      // The TLB entry written by TLBWR is dependent on the count,
      // so update the cycle count
      emit_readword((int)&last_count,ECX);
      if(i_regs->regmap[HOST_CCREG]!=CCREG) emit_loadreg(CCREG,HOST_CCREG);
      emit_add(HOST_CCREG,ECX,HOST_CCREG);
      emit_addimm(HOST_CCREG,CLOCK_DIVIDER*ccadj[i],HOST_CCREG);
      emit_writeword(HOST_CCREG,(int)&Count);
      emit_call((int)TLBWR_new);
    }
    if((source[i]&0x3f)==0x08) // TLBP
      emit_call((int)cached_interpreter_table.TLBP);
    if((source[i]&0x3f)==0x18) // ERET
    {
      int count=ccadj[i];
      if(i_regs->regmap[HOST_CCREG]!=CCREG) emit_loadreg(CCREG,HOST_CCREG);
      emit_addimm(HOST_CCREG,CLOCK_DIVIDER*count,HOST_CCREG); // TODO: Should there be an extra cycle here?
      emit_jmp((int)jump_eret);
    }
  }
}

static void cop1_assemble(int i,struct regstat *i_regs)
{
  // Check cop1 unusable
  if(!cop1_usable) {
    signed char rs=get_reg(i_regs->regmap,CSREG);
    assert(rs>=0);
    emit_testimm(rs,0x20000000);
    int jaddr=(int)out;
    emit_jeq(0);
    add_stub(FP_STUB,jaddr,(int)out,i,rs,(int)i_regs,is_delayslot,0);
    cop1_usable=1;
  }
  if (opcode2[i]==0) { // MFC1
    signed char tl=get_reg(i_regs->regmap,rt1[i]);
    if(tl>=0) {
      emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],tl);
      emit_readword_indexed(0,tl,tl);
    }
  }
  else if (opcode2[i]==1) { // DMFC1
    signed char tl=get_reg(i_regs->regmap,rt1[i]);
    signed char th=get_reg(i_regs->regmap,rt1[i]|64);
    if(tl>=0) {
      emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],tl);
      if(th>=0) emit_readword_indexed(4,tl,th);
      emit_readword_indexed(0,tl,tl);
    }
  }
  else if (opcode2[i]==4) { // MTC1
    signed char sl=get_reg(i_regs->regmap,rs1[i]);
    signed char temp=get_reg(i_regs->regmap,-1);
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],temp);
    emit_writeword_indexed(sl,0,temp);
  }
  else if (opcode2[i]==5) { // DMTC1
    signed char sl=get_reg(i_regs->regmap,rs1[i]);
    signed char sh=rs1[i]>0?get_reg(i_regs->regmap,rs1[i]|64):sl;
    signed char temp=get_reg(i_regs->regmap,-1);
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],temp);
    emit_writeword_indexed(sh,4,temp);
    emit_writeword_indexed(sl,0,temp);
  }
  else if (opcode2[i]==2) // CFC1
  {
    signed char tl=get_reg(i_regs->regmap,rt1[i]);
    if(tl>=0) {
      u_int copr=(source[i]>>11)&0x1f;
      if(copr==0) emit_readword((int)&FCR0,tl);
      if(copr==31) emit_readword((int)&FCR31,tl);
    }
  }
  else if (opcode2[i]==6) // CTC1
  {
    signed char sl=get_reg(i_regs->regmap,rs1[i]);
    u_int copr=(source[i]>>11)&0x1f;
    assert(sl>=0);
    if(copr==31)
    {
      emit_writeword(sl,(int)&FCR31);
      // Set the rounding mode
      //FIXME
      //char temp=get_reg(i_regs->regmap,-1);
      //emit_andimm(sl,3,temp);
      //emit_fldcw_indexed((int)&rounding_modes,temp);
    }
  }
}

static void fconv_assemble_arm(int i,struct regstat *i_regs)
{
  signed char temp=get_reg(i_regs->regmap,-1);
  assert(temp>=0);
  // Check cop1 unusable
  if(!cop1_usable) {
    signed char rs=get_reg(i_regs->regmap,CSREG);
    assert(rs>=0);
    emit_testimm(rs,0x20000000);
    int jaddr=(int)out;
    emit_jeq(0);
    add_stub(FP_STUB,jaddr,(int)out,i,rs,(int)i_regs,is_delayslot,0);
    cop1_usable=1;
  }
  
  #if (defined(__VFP_FP__) && !defined(__SOFTFP__)) 
  if(opcode2[i]==0x10&&(source[i]&0x3f)==0x0d) { // trunc_w_s
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],temp);
    emit_flds(temp,15);
    emit_ftosizs(15,15); // float->int, truncate
    if(((source[i]>>11)&0x1f)!=((source[i]>>6)&0x1f))
      emit_readword((int)&reg_cop1_simple[(source[i]>>6)&0x1f],temp);
    emit_fsts(15,temp);
    return;
  }
  if(opcode2[i]==0x11&&(source[i]&0x3f)==0x0d) { // trunc_w_d
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],temp);
    emit_vldr(temp,7);
    emit_ftosizd(7,13); // double->int, truncate
    emit_readword((int)&reg_cop1_simple[(source[i]>>6)&0x1f],temp);
    emit_fsts(13,temp);
    return;
  }
  
  if(opcode2[i]==0x14&&(source[i]&0x3f)==0x20) { // cvt_s_w
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],temp);
    emit_flds(temp,13);
    if(((source[i]>>11)&0x1f)!=((source[i]>>6)&0x1f))
      emit_readword((int)&reg_cop1_simple[(source[i]>>6)&0x1f],temp);
    emit_fsitos(13,15);
    emit_fsts(15,temp);
    return;
  }
  if(opcode2[i]==0x14&&(source[i]&0x3f)==0x21) { // cvt_d_w
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],temp);
    emit_flds(temp,13);
    emit_readword((int)&reg_cop1_double[(source[i]>>6)&0x1f],temp);
    emit_fsitod(13,7);
    emit_vstr(7,temp);
    return;
  }
  
  if(opcode2[i]==0x10&&(source[i]&0x3f)==0x21) { // cvt_d_s
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],temp);
    emit_flds(temp,13);
    emit_readword((int)&reg_cop1_double[(source[i]>>6)&0x1f],temp);
    emit_fcvtds(13,7);
    emit_vstr(7,temp);
    return;
  }
  if(opcode2[i]==0x11&&(source[i]&0x3f)==0x20) { // cvt_s_d
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],temp);
    emit_vldr(temp,7);
    emit_readword((int)&reg_cop1_simple[(source[i]>>6)&0x1f],temp);
    emit_fcvtsd(7,13);
    emit_fsts(13,temp);
    return;
  }
  #endif
  
  // C emulation code
  
  u_int hr,reglist=0;
  for(hr=0;hr<HOST_REGS;hr++) {
    if(i_regs->regmap[hr]>=0) reglist|=1<<hr;
  }
  save_regs(reglist);
  
  if(opcode2[i]==0x14&&(source[i]&0x3f)==0x20) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)cvt_s_w);
  }
  if(opcode2[i]==0x14&&(source[i]&0x3f)==0x21) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)cvt_d_w);
  }
  if(opcode2[i]==0x15&&(source[i]&0x3f)==0x20) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)cvt_s_l);
  }
  if(opcode2[i]==0x15&&(source[i]&0x3f)==0x21) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)cvt_d_l);
  }
  
  if(opcode2[i]==0x10&&(source[i]&0x3f)==0x21) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)cvt_d_s);
  }
  if(opcode2[i]==0x10&&(source[i]&0x3f)==0x24) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)cvt_w_s);
  }
  if(opcode2[i]==0x10&&(source[i]&0x3f)==0x25) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)cvt_l_s);
  }
  
  if(opcode2[i]==0x11&&(source[i]&0x3f)==0x20) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)cvt_s_d);
  }
  if(opcode2[i]==0x11&&(source[i]&0x3f)==0x24) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)cvt_w_d);
  }
  if(opcode2[i]==0x11&&(source[i]&0x3f)==0x25) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)cvt_l_d);
  }
  
  if(opcode2[i]==0x10&&(source[i]&0x3f)==0x08) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)round_l_s);
  }
  if(opcode2[i]==0x10&&(source[i]&0x3f)==0x09) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)trunc_l_s);
  }
  if(opcode2[i]==0x10&&(source[i]&0x3f)==0x0a) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)ceil_l_s);
  }
  if(opcode2[i]==0x10&&(source[i]&0x3f)==0x0b) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)floor_l_s);
  }
  if(opcode2[i]==0x10&&(source[i]&0x3f)==0x0c) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)round_w_s);
  }
  if(opcode2[i]==0x10&&(source[i]&0x3f)==0x0d) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)trunc_w_s);
  }
  if(opcode2[i]==0x10&&(source[i]&0x3f)==0x0e) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)ceil_w_s);
  }
  if(opcode2[i]==0x10&&(source[i]&0x3f)==0x0f) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)floor_w_s);
  }
  
  if(opcode2[i]==0x11&&(source[i]&0x3f)==0x08) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)round_l_d);
  }
  if(opcode2[i]==0x11&&(source[i]&0x3f)==0x09) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)trunc_l_d);
  }
  if(opcode2[i]==0x11&&(source[i]&0x3f)==0x0a) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)ceil_l_d);
  }
  if(opcode2[i]==0x11&&(source[i]&0x3f)==0x0b) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)floor_l_d);
  }
  if(opcode2[i]==0x11&&(source[i]&0x3f)==0x0c) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)round_w_d);
  }
  if(opcode2[i]==0x11&&(source[i]&0x3f)==0x0d) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)trunc_w_d);
  }
  if(opcode2[i]==0x11&&(source[i]&0x3f)==0x0e) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)ceil_w_d);
  }
  if(opcode2[i]==0x11&&(source[i]&0x3f)==0x0f) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG2_REG);
    emit_call((int)floor_w_d);
  }
  
  restore_regs(reglist);
}
#define fconv_assemble fconv_assemble_arm

static void fcomp_assemble(int i,struct regstat *i_regs)
{
  signed char fs=get_reg(i_regs->regmap,FSREG);
  signed char temp=get_reg(i_regs->regmap,-1);
  assert(temp>=0);
  // Check cop1 unusable
  if(!cop1_usable) {
    signed char cs=get_reg(i_regs->regmap,CSREG);
    assert(cs>=0);
    emit_testimm(cs,0x20000000);
    int jaddr=(int)out;
    emit_jeq(0);
    add_stub(FP_STUB,jaddr,(int)out,i,cs,(int)i_regs,is_delayslot,0);
    cop1_usable=1;
  }
  
  if((source[i]&0x3f)==0x30) {
    emit_andimm(fs,~0x800000,fs);
    return;
  }
  
  if((source[i]&0x3e)==0x38) {
    // sf/ngle - these should throw exceptions for NaNs
    emit_andimm(fs,~0x800000,fs);
    return;
  }
  
  #if (defined(__VFP_FP__) && !defined(__SOFTFP__)) 
  if(opcode2[i]==0x10) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],temp);
    emit_readword((int)&reg_cop1_simple[(source[i]>>16)&0x1f],HOST_TEMPREG);
    emit_orimm(fs,0x800000,fs);
    emit_flds(temp,14);
    emit_flds(HOST_TEMPREG,15);
    emit_fcmps(14,15);
    emit_fmstat();
    if((source[i]&0x3f)==0x31) emit_bicvc_imm(fs,0x800000,fs); // c_un_s
    if((source[i]&0x3f)==0x32) emit_bicne_imm(fs,0x800000,fs); // c_eq_s
    if((source[i]&0x3f)==0x33) {emit_bicne_imm(fs,0x800000,fs);emit_orrvs_imm(fs,0x800000,fs);} // c_ueq_s
    if((source[i]&0x3f)==0x34) emit_biccs_imm(fs,0x800000,fs); // c_olt_s
    if((source[i]&0x3f)==0x35) {emit_biccs_imm(fs,0x800000,fs);emit_orrvs_imm(fs,0x800000,fs);} // c_ult_s 
    if((source[i]&0x3f)==0x36) emit_bichi_imm(fs,0x800000,fs); // c_ole_s
    if((source[i]&0x3f)==0x37) {emit_bichi_imm(fs,0x800000,fs);emit_orrvs_imm(fs,0x800000,fs);} // c_ule_s
    if((source[i]&0x3f)==0x3a) emit_bicne_imm(fs,0x800000,fs); // c_seq_s
    if((source[i]&0x3f)==0x3b) emit_bicne_imm(fs,0x800000,fs); // c_ngl_s
    if((source[i]&0x3f)==0x3c) emit_biccs_imm(fs,0x800000,fs); // c_lt_s
    if((source[i]&0x3f)==0x3d) emit_biccs_imm(fs,0x800000,fs); // c_nge_s
    if((source[i]&0x3f)==0x3e) emit_bichi_imm(fs,0x800000,fs); // c_le_s
    if((source[i]&0x3f)==0x3f) emit_bichi_imm(fs,0x800000,fs); // c_ngt_s
    return;
  }
  if(opcode2[i]==0x11) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],temp);
    emit_readword((int)&reg_cop1_double[(source[i]>>16)&0x1f],HOST_TEMPREG);
    emit_orimm(fs,0x800000,fs);
    emit_vldr(temp,6);
    emit_vldr(HOST_TEMPREG,7);
    emit_fcmpd(6,7);
    emit_fmstat();
    if((source[i]&0x3f)==0x31) emit_bicvc_imm(fs,0x800000,fs); // c_un_d
    if((source[i]&0x3f)==0x32) emit_bicne_imm(fs,0x800000,fs); // c_eq_d
    if((source[i]&0x3f)==0x33) {emit_bicne_imm(fs,0x800000,fs);emit_orrvs_imm(fs,0x800000,fs);} // c_ueq_d
    if((source[i]&0x3f)==0x34) emit_biccs_imm(fs,0x800000,fs); // c_olt_d
    if((source[i]&0x3f)==0x35) {emit_biccs_imm(fs,0x800000,fs);emit_orrvs_imm(fs,0x800000,fs);} // c_ult_d
    if((source[i]&0x3f)==0x36) emit_bichi_imm(fs,0x800000,fs); // c_ole_d
    if((source[i]&0x3f)==0x37) {emit_bichi_imm(fs,0x800000,fs);emit_orrvs_imm(fs,0x800000,fs);} // c_ule_d
    if((source[i]&0x3f)==0x3a) emit_bicne_imm(fs,0x800000,fs); // c_seq_d
    if((source[i]&0x3f)==0x3b) emit_bicne_imm(fs,0x800000,fs); // c_ngl_d
    if((source[i]&0x3f)==0x3c) emit_biccs_imm(fs,0x800000,fs); // c_lt_d
    if((source[i]&0x3f)==0x3d) emit_biccs_imm(fs,0x800000,fs); // c_nge_d
    if((source[i]&0x3f)==0x3e) emit_bichi_imm(fs,0x800000,fs); // c_le_d
    if((source[i]&0x3f)==0x3f) emit_bichi_imm(fs,0x800000,fs); // c_ngt_d
    return;
  }
  #endif
  
  // C only
  
  u_int hr,reglist=0;
  for(hr=0;hr<HOST_REGS;hr++) {
    if(i_regs->regmap[hr]>=0) reglist|=1<<hr;
  }
  reglist&=~(1<<fs);
  save_regs(reglist);
  if(opcode2[i]==0x10) {
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_simple[(source[i]>>16)&0x1f],ARG2_REG);
    if((source[i]&0x3f)==0x30) emit_call((int)c_f_s);
    if((source[i]&0x3f)==0x31) emit_call((int)c_un_s);
    if((source[i]&0x3f)==0x32) emit_call((int)c_eq_s);
    if((source[i]&0x3f)==0x33) emit_call((int)c_ueq_s);
    if((source[i]&0x3f)==0x34) emit_call((int)c_olt_s);
    if((source[i]&0x3f)==0x35) emit_call((int)c_ult_s);
    if((source[i]&0x3f)==0x36) emit_call((int)c_ole_s);
    if((source[i]&0x3f)==0x37) emit_call((int)c_ule_s);
    if((source[i]&0x3f)==0x38) emit_call((int)c_sf_s);
    if((source[i]&0x3f)==0x39) emit_call((int)c_ngle_s);
    if((source[i]&0x3f)==0x3a) emit_call((int)c_seq_s);
    if((source[i]&0x3f)==0x3b) emit_call((int)c_ngl_s);
    if((source[i]&0x3f)==0x3c) emit_call((int)c_lt_s);
    if((source[i]&0x3f)==0x3d) emit_call((int)c_nge_s);
    if((source[i]&0x3f)==0x3e) emit_call((int)c_le_s);
    if((source[i]&0x3f)==0x3f) emit_call((int)c_ngt_s);
  }
  if(opcode2[i]==0x11) {
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    emit_readword((int)&reg_cop1_double[(source[i]>>16)&0x1f],ARG2_REG);
    if((source[i]&0x3f)==0x30) emit_call((int)c_f_d);
    if((source[i]&0x3f)==0x31) emit_call((int)c_un_d);
    if((source[i]&0x3f)==0x32) emit_call((int)c_eq_d);
    if((source[i]&0x3f)==0x33) emit_call((int)c_ueq_d);
    if((source[i]&0x3f)==0x34) emit_call((int)c_olt_d);
    if((source[i]&0x3f)==0x35) emit_call((int)c_ult_d);
    if((source[i]&0x3f)==0x36) emit_call((int)c_ole_d);
    if((source[i]&0x3f)==0x37) emit_call((int)c_ule_d);
    if((source[i]&0x3f)==0x38) emit_call((int)c_sf_d);
    if((source[i]&0x3f)==0x39) emit_call((int)c_ngle_d);
    if((source[i]&0x3f)==0x3a) emit_call((int)c_seq_d);
    if((source[i]&0x3f)==0x3b) emit_call((int)c_ngl_d);
    if((source[i]&0x3f)==0x3c) emit_call((int)c_lt_d);
    if((source[i]&0x3f)==0x3d) emit_call((int)c_nge_d);
    if((source[i]&0x3f)==0x3e) emit_call((int)c_le_d);
    if((source[i]&0x3f)==0x3f) emit_call((int)c_ngt_d);
  }
  restore_regs(reglist);
  emit_loadreg(FSREG,fs);
}

static void float_assemble(int i,struct regstat *i_regs)
{
  signed char temp=get_reg(i_regs->regmap,-1);
  assert(temp>=0);
  // Check cop1 unusable
  if(!cop1_usable) {
    signed char cs=get_reg(i_regs->regmap,CSREG);
    assert(cs>=0);
    emit_testimm(cs,0x20000000);
    int jaddr=(int)out;
    emit_jeq(0);
    add_stub(FP_STUB,jaddr,(int)out,i,cs,(int)i_regs,is_delayslot,0);
    cop1_usable=1;
  }
  
  #if (defined(__VFP_FP__) && !defined(__SOFTFP__)) 
  if((source[i]&0x3f)==6) // mov
  {
    if(((source[i]>>11)&0x1f)!=((source[i]>>6)&0x1f)) {
      if(opcode2[i]==0x10) {
        emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],temp);
        emit_readword((int)&reg_cop1_simple[(source[i]>>6)&0x1f],HOST_TEMPREG);
        emit_readword_indexed(0,temp,temp);
        emit_writeword_indexed(temp,0,HOST_TEMPREG);
      }
      if(opcode2[i]==0x11) {
        emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],temp);
        emit_readword((int)&reg_cop1_double[(source[i]>>6)&0x1f],HOST_TEMPREG);
        emit_vldr(temp,7);
        emit_vstr(7,HOST_TEMPREG);
      }
    }
    return;
  }
  
  if((source[i]&0x3f)>3)
  {
    if(opcode2[i]==0x10) {
      emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],temp);
      emit_flds(temp,15);
      if(((source[i]>>11)&0x1f)!=((source[i]>>6)&0x1f)) {
        emit_readword((int)&reg_cop1_simple[(source[i]>>6)&0x1f],temp);
      }
      if((source[i]&0x3f)==4) // sqrt
        emit_fsqrts(15,15);
      if((source[i]&0x3f)==5) // abs
        emit_fabss(15,15);
      if((source[i]&0x3f)==7) // neg
        emit_fnegs(15,15);
      emit_fsts(15,temp);
    }
    if(opcode2[i]==0x11) {
      emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],temp);
      emit_vldr(temp,7);
      if(((source[i]>>11)&0x1f)!=((source[i]>>6)&0x1f)) {
        emit_readword((int)&reg_cop1_double[(source[i]>>6)&0x1f],temp);
      }
      if((source[i]&0x3f)==4) // sqrt
        emit_fsqrtd(7,7);
      if((source[i]&0x3f)==5) // abs
        emit_fabsd(7,7);
      if((source[i]&0x3f)==7) // neg
        emit_fnegd(7,7);
      emit_vstr(7,temp);
    }
    return;
  }
  if((source[i]&0x3f)<4)
  {
    if(opcode2[i]==0x10) {
      emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],temp);
    }
    if(opcode2[i]==0x11) {
      emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],temp);
    }
    if(((source[i]>>11)&0x1f)!=((source[i]>>16)&0x1f)) {
      if(opcode2[i]==0x10) {
        emit_readword((int)&reg_cop1_simple[(source[i]>>16)&0x1f],HOST_TEMPREG);
        emit_flds(temp,15);
        emit_flds(HOST_TEMPREG,13);
        if(((source[i]>>11)&0x1f)!=((source[i]>>6)&0x1f)) {
          if(((source[i]>>16)&0x1f)!=((source[i]>>6)&0x1f)) {
            emit_readword((int)&reg_cop1_simple[(source[i]>>6)&0x1f],temp);
          }
        }
        if((source[i]&0x3f)==0) emit_fadds(15,13,15);
        if((source[i]&0x3f)==1) emit_fsubs(15,13,15);
        if((source[i]&0x3f)==2) emit_fmuls(15,13,15);
        if((source[i]&0x3f)==3) emit_fdivs(15,13,15);
        if(((source[i]>>16)&0x1f)==((source[i]>>6)&0x1f)) {
          emit_fsts(15,HOST_TEMPREG);
        }else{
          emit_fsts(15,temp);
        }
      }
      else if(opcode2[i]==0x11) {
        emit_readword((int)&reg_cop1_double[(source[i]>>16)&0x1f],HOST_TEMPREG);
        emit_vldr(temp,7);
        emit_vldr(HOST_TEMPREG,6);
        if(((source[i]>>11)&0x1f)!=((source[i]>>6)&0x1f)) {
          if(((source[i]>>16)&0x1f)!=((source[i]>>6)&0x1f)) {
            emit_readword((int)&reg_cop1_double[(source[i]>>6)&0x1f],temp);
          }
        }
        if((source[i]&0x3f)==0) emit_faddd(7,6,7);
        if((source[i]&0x3f)==1) emit_fsubd(7,6,7);
        if((source[i]&0x3f)==2) emit_fmuld(7,6,7);
        if((source[i]&0x3f)==3) emit_fdivd(7,6,7);
        if(((source[i]>>16)&0x1f)==((source[i]>>6)&0x1f)) {
          emit_vstr(7,HOST_TEMPREG);
        }else{
          emit_vstr(7,temp);
        }
      }
    }
    else {
      if(opcode2[i]==0x10) {
        emit_flds(temp,15);
        if(((source[i]>>11)&0x1f)!=((source[i]>>6)&0x1f)) {
          emit_readword((int)&reg_cop1_simple[(source[i]>>6)&0x1f],temp);
        }
        if((source[i]&0x3f)==0) emit_fadds(15,15,15);
        if((source[i]&0x3f)==1) emit_fsubs(15,15,15);
        if((source[i]&0x3f)==2) emit_fmuls(15,15,15);
        if((source[i]&0x3f)==3) emit_fdivs(15,15,15);
        emit_fsts(15,temp);
      }
      else if(opcode2[i]==0x11) {
        emit_vldr(temp,7);
        if(((source[i]>>11)&0x1f)!=((source[i]>>6)&0x1f)) {
          emit_readword((int)&reg_cop1_double[(source[i]>>6)&0x1f],temp);
        }
        if((source[i]&0x3f)==0) emit_faddd(7,7,7);
        if((source[i]&0x3f)==1) emit_fsubd(7,7,7);
        if((source[i]&0x3f)==2) emit_fmuld(7,7,7);
        if((source[i]&0x3f)==3) emit_fdivd(7,7,7);
        emit_vstr(7,temp);
      }
    }
    return;
  }
  #endif
  
  u_int hr,reglist=0;
  for(hr=0;hr<HOST_REGS;hr++) {
    if(i_regs->regmap[hr]>=0) reglist|=1<<hr;
  }
  if(opcode2[i]==0x10) { // Single precision
    save_regs(reglist);
    emit_readword((int)&reg_cop1_simple[(source[i]>>11)&0x1f],ARG1_REG);
    if((source[i]&0x3f)<4) {
      emit_readword((int)&reg_cop1_simple[(source[i]>>16)&0x1f],ARG2_REG);
      emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG3_REG);
    }else{
      emit_readword((int)&reg_cop1_simple[(source[i]>> 6)&0x1f],ARG2_REG);
    }
    switch(source[i]&0x3f)
    {
      case 0x00: emit_call((int)add_s);break;
      case 0x01: emit_call((int)sub_s);break;
      case 0x02: emit_call((int)mul_s);break;
      case 0x03: emit_call((int)div_s);break;
      case 0x04: emit_call((int)sqrt_s);break;
      case 0x05: emit_call((int)abs_s);break;
      case 0x06: emit_call((int)mov_s);break;
      case 0x07: emit_call((int)neg_s);break;
    }
    restore_regs(reglist);
  }
  if(opcode2[i]==0x11) { // Double precision
    save_regs(reglist);
    emit_readword((int)&reg_cop1_double[(source[i]>>11)&0x1f],ARG1_REG);
    if((source[i]&0x3f)<4) {
      emit_readword((int)&reg_cop1_double[(source[i]>>16)&0x1f],ARG2_REG);
      emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG3_REG);
    }else{
      emit_readword((int)&reg_cop1_double[(source[i]>> 6)&0x1f],ARG2_REG);
    }
    switch(source[i]&0x3f)
    {
      case 0x00: emit_call((int)add_d);break;
      case 0x01: emit_call((int)sub_d);break;
      case 0x02: emit_call((int)mul_d);break;
      case 0x03: emit_call((int)div_d);break;
      case 0x04: emit_call((int)sqrt_d);break;
      case 0x05: emit_call((int)abs_d);break;
      case 0x06: emit_call((int)mov_d);break;
      case 0x07: emit_call((int)neg_d);break;
    }
    restore_regs(reglist);
  }
}

static void multdiv_assemble_arm(int i,struct regstat *i_regs)
{
  //  case 0x18: MULT
  //  case 0x19: MULTU
  //  case 0x1A: DIV
  //  case 0x1B: DIVU
  //  case 0x1C: DMULT
  //  case 0x1D: DMULTU
  //  case 0x1E: DDIV
  //  case 0x1F: DDIVU
  if(rs1[i]&&rs2[i])
  {
    if((opcode2[i]&4)==0) // 32-bit
    {
      if(opcode2[i]==0x18) // MULT
      {
        signed char m1=get_reg(i_regs->regmap,rs1[i]);
        signed char m2=get_reg(i_regs->regmap,rs2[i]);
        signed char hi=get_reg(i_regs->regmap,HIREG);
        signed char lo=get_reg(i_regs->regmap,LOREG);
        assert(m1>=0);
        assert(m2>=0);
        assert(hi>=0);
        assert(lo>=0);
        emit_smull(m1,m2,hi,lo);
      }
      if(opcode2[i]==0x19) // MULTU
      {
        signed char m1=get_reg(i_regs->regmap,rs1[i]);
        signed char m2=get_reg(i_regs->regmap,rs2[i]);
        signed char hi=get_reg(i_regs->regmap,HIREG);
        signed char lo=get_reg(i_regs->regmap,LOREG);
        assert(m1>=0);
        assert(m2>=0);
        assert(hi>=0);
        assert(lo>=0);
        emit_umull(m1,m2,hi,lo);
      }
      if(opcode2[i]==0x1A) // DIV
      {
        signed char d1=get_reg(i_regs->regmap,rs1[i]);
        signed char d2=get_reg(i_regs->regmap,rs2[i]);
        assert(d1>=0);
        assert(d2>=0);
        signed char quotient=get_reg(i_regs->regmap,LOREG);
        signed char remainder=get_reg(i_regs->regmap,HIREG);
        assert(quotient>=0);
        assert(remainder>=0);
        emit_movs(d1,remainder);
        emit_negmi(remainder,remainder);
        emit_movs(d2,HOST_TEMPREG);
        emit_jeq((int)out+52); // Division by zero
        emit_negmi(HOST_TEMPREG,HOST_TEMPREG);
        emit_clz(HOST_TEMPREG,quotient);
        emit_shl(HOST_TEMPREG,quotient,HOST_TEMPREG);
        emit_orimm(quotient,1<<31,quotient);
        emit_shr(quotient,quotient,quotient);
        emit_cmp(remainder,HOST_TEMPREG);
        emit_subcs(remainder,HOST_TEMPREG,remainder);
        emit_adcs(quotient,quotient,quotient);
        emit_shrimm(HOST_TEMPREG,1,HOST_TEMPREG);
        emit_jcc((int)out-16); // -4
        emit_teq(d1,d2);
        emit_negmi(quotient,quotient);
        emit_test(d1,d1);
        emit_negmi(remainder,remainder);
      }
      if(opcode2[i]==0x1B) // DIVU
      {
        signed char d1=get_reg(i_regs->regmap,rs1[i]); // dividend
        signed char d2=get_reg(i_regs->regmap,rs2[i]); // divisor
        assert(d1>=0);
        assert(d2>=0);
        signed char quotient=get_reg(i_regs->regmap,LOREG);
        signed char remainder=get_reg(i_regs->regmap,HIREG);
        assert(quotient>=0);
        assert(remainder>=0);
        emit_test(d2,d2);
        emit_jeq((int)out+44); // Division by zero
        emit_clz(d2,HOST_TEMPREG);
        emit_movimm(1<<31,quotient);
        emit_shl(d2,HOST_TEMPREG,d2);
        emit_mov(d1,remainder);
        emit_shr(quotient,HOST_TEMPREG,quotient);
        emit_cmp(remainder,d2);
        emit_subcs(remainder,d2,remainder);
        emit_adcs(quotient,quotient,quotient);
        emit_shrcc_imm(d2,1,d2);
        emit_jcc((int)out-16); // -4
      }
    }
    else // 64-bit
    {
      if(opcode2[i]==0x1C) // DMULT
      {
        assert(opcode2[i]!=0x1C);
        signed char m1h=get_reg(i_regs->regmap,rs1[i]|64);
        signed char m1l=get_reg(i_regs->regmap,rs1[i]);
        signed char m2h=get_reg(i_regs->regmap,rs2[i]|64);
        signed char m2l=get_reg(i_regs->regmap,rs2[i]);
        assert(m1h>=0);
        assert(m2h>=0);
        assert(m1l>=0);
        assert(m2l>=0);
        emit_pushreg(m2h);
        emit_pushreg(m2l);
        emit_pushreg(m1h);
        emit_pushreg(m1l);
        emit_call((int)&mult64);
        emit_popreg(m1l);
        emit_popreg(m1h);
        emit_popreg(m2l);
        emit_popreg(m2h);
        signed char hih=get_reg(i_regs->regmap,HIREG|64);
        signed char hil=get_reg(i_regs->regmap,HIREG);
        if(hih>=0) emit_loadreg(HIREG|64,hih);
        if(hil>=0) emit_loadreg(HIREG,hil);
        signed char loh=get_reg(i_regs->regmap,LOREG|64);
        signed char lol=get_reg(i_regs->regmap,LOREG);
        if(loh>=0) emit_loadreg(LOREG|64,loh);
        if(lol>=0) emit_loadreg(LOREG,lol);
      }
      if(opcode2[i]==0x1D) // DMULTU
      {
        signed char m1h=get_reg(i_regs->regmap,rs1[i]|64);
        signed char m1l=get_reg(i_regs->regmap,rs1[i]);
        signed char m2h=get_reg(i_regs->regmap,rs2[i]|64);
        signed char m2l=get_reg(i_regs->regmap,rs2[i]);
        assert(m1h>=0);
        assert(m2h>=0);
        assert(m1l>=0);
        assert(m2l>=0);
        save_regs(0x100f);
        if(m1l!=0) emit_mov(m1l,0);
        if(m1h==0) emit_readword((int)&dynarec_local,1);
        else if(m1h>1) emit_mov(m1h,1);
        if(m2l<2) emit_readword((int)&dynarec_local+m2l*4,2);
        else if(m2l>2) emit_mov(m2l,2);
        if(m2h<3) emit_readword((int)&dynarec_local+m2h*4,3);
        else if(m2h>3) emit_mov(m2h,3);
        emit_call((int)&multu64);
        restore_regs(0x100f);
        signed char hih=get_reg(i_regs->regmap,HIREG|64);
        signed char hil=get_reg(i_regs->regmap,HIREG);
        signed char loh=get_reg(i_regs->regmap,LOREG|64);
        signed char lol=get_reg(i_regs->regmap,LOREG);
        /*signed char temp=get_reg(i_regs->regmap,-1);
        signed char rh=get_reg(i_regs->regmap,HIREG|64);
        signed char rl=get_reg(i_regs->regmap,HIREG);
        assert(m1h>=0);
        assert(m2h>=0);
        assert(m1l>=0);
        assert(m2l>=0);
        assert(temp>=0);
        //emit_mov(m1l,EAX);
        //emit_mul(m2l);
        emit_umull(rl,rh,m1l,m2l);
        emit_storereg(LOREG,rl);
        emit_mov(rh,temp);
        //emit_mov(m1h,EAX);
        //emit_mul(m2l);
        emit_umull(rl,rh,m1h,m2l);
        emit_adds(rl,temp,temp);
        emit_adcimm(rh,0,rh);
        emit_storereg(HIREG,rh);
        //emit_mov(m2h,EAX);
        //emit_mul(m1l);
        emit_umull(rl,rh,m1l,m2h);
        emit_adds(rl,temp,temp);
        emit_adcimm(rh,0,rh);
        emit_storereg(LOREG|64,temp);
        emit_mov(rh,temp);
        //emit_mov(m2h,EAX);
        //emit_mul(m1h);
        emit_umull(rl,rh,m1h,m2h);
        emit_adds(rl,temp,rl);
        emit_loadreg(HIREG,temp);
        emit_adcimm(rh,0,rh);
        emit_adds(rl,temp,rl);
        emit_adcimm(rh,0,rh);
        // DEBUG
        /*
        emit_pushreg(m2h);
        emit_pushreg(m2l);
        emit_pushreg(m1h);
        emit_pushreg(m1l);
        emit_call((int)&multu64);
        emit_popreg(m1l);
        emit_popreg(m1h);
        emit_popreg(m2l);
        emit_popreg(m2h);
        signed char hih=get_reg(i_regs->regmap,HIREG|64);
        signed char hil=get_reg(i_regs->regmap,HIREG);
        if(hih>=0) emit_loadreg(HIREG|64,hih);  // DEBUG
        if(hil>=0) emit_loadreg(HIREG,hil);  // DEBUG
        */
        // Shouldn't be necessary
        //char loh=get_reg(i_regs->regmap,LOREG|64);
        //char lol=get_reg(i_regs->regmap,LOREG);
        //if(loh>=0) emit_loadreg(LOREG|64,loh);
        //if(lol>=0) emit_loadreg(LOREG,lol);
      }
      if(opcode2[i]==0x1E) // DDIV
      {
        signed char d1h=get_reg(i_regs->regmap,rs1[i]|64);
        signed char d1l=get_reg(i_regs->regmap,rs1[i]);
        signed char d2h=get_reg(i_regs->regmap,rs2[i]|64);
        signed char d2l=get_reg(i_regs->regmap,rs2[i]);
        assert(d1h>=0);
        assert(d2h>=0);
        assert(d1l>=0);
        assert(d2l>=0);
        save_regs(0x100f);
        if(d1l!=0) emit_mov(d1l,0);
        if(d1h==0) emit_readword((int)&dynarec_local,1);
        else if(d1h>1) emit_mov(d1h,1);
        if(d2l<2) emit_readword((int)&dynarec_local+d2l*4,2);
        else if(d2l>2) emit_mov(d2l,2);
        if(d2h<3) emit_readword((int)&dynarec_local+d2h*4,3);
        else if(d2h>3) emit_mov(d2h,3);
        emit_call((int)&div64);
        restore_regs(0x100f);
        signed char hih=get_reg(i_regs->regmap,HIREG|64);
        signed char hil=get_reg(i_regs->regmap,HIREG);
        signed char loh=get_reg(i_regs->regmap,LOREG|64);
        signed char lol=get_reg(i_regs->regmap,LOREG);
        if(hih>=0) emit_loadreg(HIREG|64,hih);
        if(hil>=0) emit_loadreg(HIREG,hil);
        if(loh>=0) emit_loadreg(LOREG|64,loh);
        if(lol>=0) emit_loadreg(LOREG,lol);
      }
      if(opcode2[i]==0x1F) // DDIVU
      {
      //u_int hr,reglist=0;
      //for(hr=0;hr<HOST_REGS;hr++) {
      //  if(i_regs->regmap[hr]>=0 && (i_regs->regmap[hr]&62)!=HIREG) reglist|=1<<hr;
      //}
        signed char d1h=get_reg(i_regs->regmap,rs1[i]|64);
        signed char d1l=get_reg(i_regs->regmap,rs1[i]);
        signed char d2h=get_reg(i_regs->regmap,rs2[i]|64);
        signed char d2l=get_reg(i_regs->regmap,rs2[i]);
        assert(d1h>=0);
        assert(d2h>=0);
        assert(d1l>=0);
        assert(d2l>=0);
        save_regs(0x100f);
        if(d1l!=0) emit_mov(d1l,0);
        if(d1h==0) emit_readword((int)&dynarec_local,1);
        else if(d1h>1) emit_mov(d1h,1);
        if(d2l<2) emit_readword((int)&dynarec_local+d2l*4,2);
        else if(d2l>2) emit_mov(d2l,2);
        if(d2h<3) emit_readword((int)&dynarec_local+d2h*4,3);
        else if(d2h>3) emit_mov(d2h,3);
        emit_call((int)&divu64);
        restore_regs(0x100f);
        signed char hih=get_reg(i_regs->regmap,HIREG|64);
        signed char hil=get_reg(i_regs->regmap,HIREG);
        signed char loh=get_reg(i_regs->regmap,LOREG|64);
        signed char lol=get_reg(i_regs->regmap,LOREG);
        if(hih>=0) emit_loadreg(HIREG|64,hih);
        if(hil>=0) emit_loadreg(HIREG,hil);
        if(loh>=0) emit_loadreg(LOREG|64,loh);
        if(lol>=0) emit_loadreg(LOREG,lol);
      }
    }
  }
  else
  {
    // Multiply by zero is zero.
    // MIPS does not have a divide by zero exception.
    // The result is undefined, we return zero.
    signed char hr=get_reg(i_regs->regmap,HIREG);
    signed char lr=get_reg(i_regs->regmap,LOREG);
    if(hr>=0) emit_zeroreg(hr);
    if(lr>=0) emit_zeroreg(lr);
  }
}
#define multdiv_assemble multdiv_assemble_arm

static void do_preload_rhash(int r) {
  // Don't need this for ARM.  On x86, this puts the value 0xf8 into the
  // register.  On ARM the hash can be done with a single instruction (below)
}

static void do_preload_rhtbl(int ht) {
  emit_addimm(FP,(int)&mini_ht-(int)&dynarec_local,ht);
}

static void do_rhash(int rs,int rh) {
  emit_andimm(rs,0xf8,rh);
}

static void do_miniht_load(int ht,int rh) {
  assem_debug("ldr %s,[%s,%s]!",regname[rh],regname[ht],regname[rh]);
  output_w32(0xe7b00000|rd_rn_rm(rh,ht,rh));
}

static void do_miniht_jump(int rs,int rh,int ht) {
  emit_cmp(rh,rs);
  emit_ldreq_indexed(ht,4,15);
  #ifdef CORTEX_A8_BRANCH_PREDICTION_HACK
  emit_mov(rs,7);
  emit_jmp(jump_vaddr_reg[7]);
  #else
  emit_jmp(jump_vaddr_reg[rs]);
  #endif
}

static void do_miniht_insert(u_int return_address,int rt,int temp) {
  #ifdef ARMv5_ONLY
  emit_movimm(return_address,rt); // PC into link register
  add_to_linker((int)out,return_address,1);
  emit_pcreladdr(temp);
  emit_writeword(rt,(int)&mini_ht[(return_address&0xFF)>>3][0]);
  emit_writeword(temp,(int)&mini_ht[(return_address&0xFF)>>3][1]);
  #else
  emit_movw(return_address&0x0000FFFF,rt);
  add_to_linker((int)out,return_address,1);
  emit_pcreladdr(temp);
  emit_writeword(temp,(int)&mini_ht[(return_address&0xFF)>>3][1]);
  emit_movt(return_address&0xFFFF0000,rt);
  emit_writeword(rt,(int)&mini_ht[(return_address&0xFF)>>3][0]);
  #endif
}

// Sign-extend to 64 bits and write out upper half of a register
// This is useful where we have a 32-bit value in a register, and want to
// keep it in a 32-bit register, but can't guarantee that it won't be read
// as a 64-bit value later.
static void wb_sx(signed char pre[],signed char entry[],uint64_t dirty,uint64_t is32_pre,uint64_t is32,uint64_t u,uint64_t uu)
{
  if(is32_pre==is32) return;
  int hr,reg;
  for(hr=0;hr<HOST_REGS;hr++) {
    if(hr!=EXCLUDE_REG) {
      //if(pre[hr]==entry[hr]) {
        if((reg=pre[hr])>=0) {
          if((dirty>>hr)&1) {
            if( ((is32_pre&~is32&~uu)>>reg)&1 ) {
              emit_sarimm(hr,31,HOST_TEMPREG);
              emit_storereg(reg|64,HOST_TEMPREG);
            }
          }
        }
      //}
    }
  }
}

static void wb_valid(signed char pre[],signed char entry[],u_int dirty_pre,u_int dirty,uint64_t is32_pre,uint64_t u,uint64_t uu)
{
  //if(dirty_pre==dirty) return;
  int hr,reg,new_hr;
  for(hr=0;hr<HOST_REGS;hr++) {
    if(hr!=EXCLUDE_REG) {
      reg=pre[hr];
      if(((~u)>>(reg&63))&1) {
        if(reg>0) {
          if(((dirty_pre&~dirty)>>hr)&1) {
            if(reg>0&&reg<34) {
              emit_storereg(reg,hr);
              if( ((is32_pre&~uu)>>reg)&1 ) {
                emit_sarimm(hr,31,HOST_TEMPREG);
                emit_storereg(reg|64,HOST_TEMPREG);
              }
            }
            else if(reg>=64) {
              emit_storereg(reg,hr);
            }
          }
        }
      }
    }
  }
}


/* using strd could possibly help but you'd have to allocate registers in pairs
static void wb_invalidate_arm(signed char pre[],signed char entry[],uint64_t dirty,uint64_t is32,uint64_t u,uint64_t uu)
{
  int hr;
  int wrote=-1;
  for(hr=HOST_REGS-1;hr>=0;hr--) {
    if(hr!=EXCLUDE_REG) {
      if(pre[hr]!=entry[hr]) {
        if(pre[hr]>=0) {
          if((dirty>>hr)&1) {
            if(get_reg(entry,pre[hr])<0) {
              if(pre[hr]<64) {
                if(!((u>>pre[hr])&1)) {
                  if(hr<10&&(~hr&1)&&(pre[hr+1]<0||wrote==hr+1)) {
                    if( ((is32>>pre[hr])&1) && !((uu>>pre[hr])&1) ) {
                      emit_sarimm(hr,31,hr+1);
                      emit_strdreg(pre[hr],hr);
                    }
                    else
                      emit_storereg(pre[hr],hr);
                  }else{
                    emit_storereg(pre[hr],hr);
                    if( ((is32>>pre[hr])&1) && !((uu>>pre[hr])&1) ) {
                      emit_sarimm(hr,31,hr);
                      emit_storereg(pre[hr]|64,hr);
                    }
                  }
                }
              }else{
                if(!((uu>>(pre[hr]&63))&1) && !((is32>>(pre[hr]&63))&1)) {
                  emit_storereg(pre[hr],hr);
                }
              }
              wrote=hr;
            }
          }
        }
      }
    }
  }
  for(hr=0;hr<HOST_REGS;hr++) {
    if(hr!=EXCLUDE_REG) {
      if(pre[hr]!=entry[hr]) {
        if(pre[hr]>=0) {
          int nr;
          if((nr=get_reg(entry,pre[hr]))>=0) {
            emit_mov(hr,nr);
          }
        }
      }
    }
  }
}
#define wb_invalidate wb_invalidate_arm
*/

// Clearing the cache is rather slow on ARM Linux, so mark the areas
// that need to be cleared, and then only clear these areas once.
static void do_clear_cache()
{
  int i,j;
  for (i=0;i<(1<<(TARGET_SIZE_2-17));i++)
  {
    u_int bitmap=needs_clear_cache[i];
    if(bitmap) {
      u_int start,end;
      for(j=0;j<32;j++) 
      {
        if(bitmap&(1<<j)) {
          start=BASE_ADDR+i*131072+j*4096;
          end=start+4095;
          j++;
          while(j<32) {
            if(bitmap&(1<<j)) {
              end+=4096;
              j++;
            }else{
              __clear_cache((void *)start,(void *)end);
              //cacheflush((void *)start,(void *)end,0);
              break;
            }
          }
        }
      }
      needs_clear_cache[i]=0;
    }
  }
}

// CPU-architecture-specific initialization
static void arch_init() {
  rounding_modes[0]=0x0<<22; // round
  rounding_modes[1]=0x3<<22; // trunc
  rounding_modes[2]=0x1<<22; // ceil
  rounding_modes[3]=0x2<<22; // floor

  jump_table_symbols[15] = cached_interpreter_table.MFC0;
  jump_table_symbols[16] = cached_interpreter_table.MTC0;
  jump_table_symbols[17] = cached_interpreter_table.TLBR;
  jump_table_symbols[18] = cached_interpreter_table.TLBP;

  #ifdef RAM_OFFSET
  ram_offset=((int)rdram-(int)0x80000000)>>2;
  #endif

  // Trampolines for jumps >32M
  int *ptr,*ptr2;
  ptr=(int *)jump_table_symbols;
  ptr2=(int *)((void *)BASE_ADDR+(1<<TARGET_SIZE_2)-JUMP_TABLE_SIZE);
  while((void *)ptr<(void *)jump_table_symbols+sizeof(jump_table_symbols))
  {
    int offset=*ptr-(int)ptr2-8;
    if(offset>=-33554432&&offset<33554432) {
      *ptr2=0xea000000|((offset>>2)&0xffffff); // direct branch
    }else{
      *ptr2=0xe51ff004; // ldr pc,[pc,#-4]
    }
    ptr2++;
    *ptr2=*ptr;
    ptr++;
    ptr2++;
  }

  // Jumping thru the trampolines created above slows things down by about 1%.
  // If part of the cache is beyond the 32M limit, avoid using this area
  // initially.  It will be used later if the cache gets full.
  if((u_int)dyna_linker-33554432>(u_int)BASE_ADDR) {
    if((u_int)dyna_linker-33554432<(u_int)BASE_ADDR+(1<<(TARGET_SIZE_2-1))) {
      out=(u_char *)(((u_int)dyna_linker-33554432)&~4095);
      expirep=((((int)out-BASE_ADDR)>>(TARGET_SIZE_2-16))+16384)&65535;
    }
  }
}
