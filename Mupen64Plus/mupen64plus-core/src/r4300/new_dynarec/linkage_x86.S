/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - linkage_x86.s                                           *
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
	.file	"linkage_x86.s"
	.bss
	.align 4

	.section	.rodata
	.text
.globl dyna_linker
	.hidden dyna_linker
	.type	dyna_linker, @function
dyna_linker:
	/* eax = virtual target address */
	/* ebx = instruction to patch */
	mov	%eax, %edi
	mov	%eax, %ecx
	shr	$12, %edi
	cmp	$0xC0000000, %eax
	cmovge	tlb_LUT_r(,%edi,4), %ecx
	test	%ecx, %ecx
	cmovz	%eax, %ecx
	xor	$0x80000000, %ecx
	mov	$2047, %edx
	shr	$12, %ecx
	and	%ecx, %edx
	or	$2048, %edx
	cmp	%edx, %ecx
	cmova	%edx, %ecx
	/* jump_in lookup */
	mov	jump_in(,%ecx,4), %edx
.A1:
	test	%edx, %edx
	je	.A3
	mov	(%edx), %edi
	xor	%eax, %edi
	or	4(%edx), %edi
	je	.A2
	movl	12(%edx), %edx
	jmp	.A1
.A2:
	mov	(%ebx), %edi
	mov	%esi, %ebp
	lea	4(%ebx,%edi,1), %esi
	mov	%eax, %edi
	pusha
	call	add_link
	popa
	mov	8(%edx), %edi
	mov	%ebp, %esi
	lea	-4(%edi), %edx
	subl	%ebx, %edx
	movl	%edx, (%ebx)
	jmp	*%edi
.A3:
	/* hash_table lookup */
	mov	%eax, %edi
	mov	%eax, %edx
	shr	$16, %edi
	shr	$12, %edx
	xor	%eax, %edi
	and	$2047, %edx
	movzwl	%di, %edi
	shl	$4, %edi
	cmp	$2048, %ecx
	cmovc	%edx, %ecx
	cmp	hash_table(%edi), %eax
	jne	.A5
.A4:
	mov	hash_table+4(%edi), %edx
	jmp	*%edx
.A5:
	cmp	hash_table+8(%edi), %eax
	lea	8(%edi), %edi
	je	.A4
	/* jump_dirty lookup */
	mov	jump_dirty(,%ecx,4), %edx
.A6:
	testl	%edx, %edx
	je	.A8
	mov	(%edx), %ecx
	xor	%eax, %ecx
	or	4(%edx), %ecx
	je	.A7
	movl	12(%edx), %edx
	jmp	.A6
.A7:
	mov	8(%edx), %edx
	/* hash_table insert */
	mov	hash_table-8(%edi), %ebx
	mov	hash_table-4(%edi), %ecx
	mov	%eax, hash_table-8(%edi)
	mov	%edx, hash_table-4(%edi)
	mov	%ebx, hash_table(%edi)
	mov	%ecx, hash_table+4(%edi)
	jmp	*%edx
.A8:
	mov	%eax, %edi
	pusha
	call	new_recompile_block
	test	%eax, %eax
	popa
	je	dyna_linker
	/* pagefault */
	mov	%eax, %ebx
	mov	$0x08, %ecx
	.size	dyna_linker, .-dyna_linker

	.type	exec_pagefault, @function
exec_pagefault:
	/* eax = instruction pointer */
	/* ebx = fault address */
	/* ecx = cause */
	mov	reg_cop0+48, %edx
	add	$-12, %esp
	mov	reg_cop0+16, %edi
	or	$2, %edx
	mov	%ebx, reg_cop0+32 /* BadVAddr */
	and	$0xFF80000F, %edi
	mov	%edx, reg_cop0+48 /* Status */
	mov	%ecx, reg_cop0+52 /* Cause */
	mov	%eax, reg_cop0+56 /* EPC */
	mov	%ebx, %ecx
	shr	$9, %ebx
	and	$0xFFFFE000, %ecx
	and	$0x007FFFF0, %ebx
	mov	%ecx, reg_cop0+40 /* EntryHI */
	or	%ebx, %edi
	mov	%edi, reg_cop0+16 /* Context */
	push	$0x80000000
	call	get_addr_ht
	add	$16, %esp
	jmp	*%eax
	.size	exec_pagefault, .-exec_pagefault

/* Special dynamic linker for the case where a page fault
   may occur in a branch delay slot */
.globl dyna_linker_ds
	.hidden dyna_linker_ds
	.type	dyna_linker_ds, @function
dyna_linker_ds:
	mov	%eax, %edi
	mov	%eax, %ecx
	shr	$12, %edi
	cmp	$0xC0000000, %eax
	cmovge	tlb_LUT_r(,%edi,4), %ecx
	test	%ecx, %ecx
	cmovz	%eax, %ecx
	xor	$0x80000000, %ecx
	mov	$2047, %edx
	shr	$12, %ecx
	and	%ecx, %edx
	or	$2048, %edx
	cmp	%edx, %ecx
	cmova	%edx, %ecx
	/* jump_in lookup */
	mov	jump_in(,%ecx,4), %edx
.B1:
	test	%edx, %edx
	je	.B3
	mov	(%edx), %edi
	xor	%eax, %edi
	or	4(%edx), %edi
	je	.B2
	movl	12(%edx), %edx
	jmp	.B1
.B2:
	mov	(%ebx), %edi
	mov	%esi, %ecx
	lea	4(%ebx,%edi,1), %esi
	mov	%eax, %edi
	pusha
	call	add_link
	popa
	mov	8(%edx), %edi
	mov	%ecx, %esi
	lea	-4(%edi), %edx
	subl	%ebx, %edx
	movl	%edx, (%ebx)
	jmp	*%edi
.B3:
	/* hash_table lookup */
	mov	%eax, %edi
	mov	%eax, %edx
	shr	$16, %edi
	shr	$12, %edx
	xor	%eax, %edi
	and	$2047, %edx
	movzwl	%di, %edi
	shl	$4, %edi
	cmp	$2048, %ecx
	cmovc	%edx, %ecx
	cmp	hash_table(%edi), %eax
	jne	.B5
.B4:
	mov	hash_table+4(%edi), %edx
	jmp	*%edx
.B5:
	cmp	hash_table+8(%edi), %eax
	lea	8(%edi), %edi
	je	.B4
	/* jump_dirty lookup */
	mov	jump_dirty(,%ecx,4), %edx
.B6:
	testl	%edx, %edx
	je	.B8
	mov	(%edx), %ecx
	xor	%eax, %ecx
	or	4(%edx), %ecx
	je	.B7
	movl	12(%edx), %edx
	jmp	.B6
.B7:
	mov	8(%edx), %edx
	/* hash_table insert */
	mov	hash_table-8(%edi), %ebx
	mov	hash_table-4(%edi), %ecx
	mov	%eax, hash_table-8(%edi)
	mov	%edx, hash_table-4(%edi)
	mov	%ebx, hash_table(%edi)
	mov	%ecx, hash_table+4(%edi)
	jmp	*%edx
.B8:
	mov	%eax, %edi
	and	$0xFFFFFFF8, %edi
	inc	%edi
	pusha
	call	new_recompile_block
	test	%eax, %eax
	popa
	je	dyna_linker_ds
	/* pagefault */
	and	$0xFFFFFFF8, %eax
	mov	$0x80000008, %ecx /* High bit set indicates pagefault in delay slot */
	mov	%eax, %ebx
	sub	$4, %eax
	jmp	exec_pagefault
	.size	dyna_linker_ds, .-dyna_linker_ds

.globl jump_vaddr_eax
	.hidden jump_vaddr_eax
	.type	jump_vaddr_eax, @function
jump_vaddr_eax:
	mov	%eax, %edi
	jmp	jump_vaddr_edi
	.size	jump_vaddr_eax, .-jump_vaddr_eax
.globl jump_vaddr_ecx
	.hidden jump_vaddr_ecx
	.type	jump_vaddr_ecx, @function
jump_vaddr_ecx:
	mov	%ecx, %edi
	jmp	jump_vaddr_edi
	.size	jump_vaddr_ecx, .-jump_vaddr_ecx
.globl jump_vaddr_edx
	.hidden jump_vaddr_edx
	.type	jump_vaddr_edx, @function
jump_vaddr_edx:
	mov	%edx, %edi
	jmp	jump_vaddr_edi
	.size	jump_vaddr_edx, .-jump_vaddr_edx
.globl jump_vaddr_ebx
	.hidden jump_vaddr_ebx
	.type	jump_vaddr_ebx, @function
jump_vaddr_ebx:
	mov	%ebx, %edi
	jmp	jump_vaddr_edi
	.size	jump_vaddr_ebx, .-jump_vaddr_ebx
.globl jump_vaddr_ebp
	.hidden jump_vaddr_ebp
	.type	jump_vaddr_ebp, @function
jump_vaddr_ebp:
	mov	%ebp, %edi
	.size	jump_vaddr_ebp, .-jump_vaddr_ebp
.globl jump_vaddr_edi
	.hidden jump_vaddr_edi
	.type	jump_vaddr_edi, @function
jump_vaddr_edi:
	mov	%edi, %eax
	.size	jump_vaddr_edi, .-jump_vaddr_edi

	.type	jump_vaddr, @function
jump_vaddr:
  /* Check hash table */
	shr	$16, %eax
	xor	%edi, %eax
	movzwl	%ax, %eax
	shl	$4, %eax
	cmp	hash_table(%eax), %edi
	jne	.C2
.C1:
	mov	hash_table+4(%eax), %edi
	jmp	*%edi
.C2:
	cmp	hash_table+8(%eax), %edi
	lea	8(%eax), %eax
	je	.C1
  /* No hit on hash table, call compiler */
	add	$-12, %esp
	push	%edi
	mov	%esi, cycle_count /* CCREG */
	call	get_addr
	mov	cycle_count, %esi
	add	$16, %esp
	jmp	*%eax
	.size	jump_vaddr, .-jump_vaddr

.globl verify_code_ds
	.hidden verify_code_ds
	.type	verify_code_ds, @function
verify_code_ds:
	mov	%ebp, branch_target
	.size	verify_code_ds, .-verify_code_ds

.globl verify_code_vm
	.hidden verify_code_vm
	.type	verify_code_vm, @function
verify_code_vm:
	/* eax = source (virtual address) */
	/* ebx = target */
	/* ecx = length */
	cmp	$0xC0000000, %eax
	jl	verify_code
	mov	%eax, %edx
	lea	-1(%eax,%ecx,1), %ebp
	shr	$12, %edx
	shr	$12, %ebp
	mov	memory_map(,%edx,4), %edi
	test	%edi, %edi
	js	.D5
	lea	(%eax,%edi,4), %eax
.D1:
	xor	memory_map(,%edx,4), %edi
	shl	$2, %edi
	jne	.D5
	mov	memory_map(,%edx,4), %edi
	inc	%edx
	cmp	%ebp, %edx
	jbe	.D1
	.size	verify_code_vm, .-verify_code_vm

.globl verify_code
	.hidden verify_code
	.type	verify_code, @function
verify_code:
	/* eax = source */
	/* ebx = target */
	/* ecx = length */
	mov	-4(%eax,%ecx,1), %edi
	xor	-4(%ebx,%ecx,1), %edi
	jne	.D5
	mov	%ecx, %edx
	add	$-4, %ecx
	je	.D3
	test	$4, %edx
	cmove	%edx, %ecx
	mov	%esi, cycle_count
.D2:
	mov	-4(%eax,%ecx,1), %edx
	mov	-4(%ebx,%ecx,1), %ebp
	mov	-8(%eax,%ecx,1), %esi
	xor	%edx, %ebp
	mov	-8(%ebx,%ecx,1), %edi
	jne	.D4
	xor	%esi, %edi
	jne	.D4
	add	$-8, %ecx
	jne	.D2
	mov	cycle_count, %esi
	mov	branch_target, %ebp
.D3:
	ret
.D4:
	mov	cycle_count, %esi
.D5:
	mov	branch_target, %ebp
	push	%esi /* for stack alignment, unused */
	push	8(%esp)
	call	get_addr
	add	$16, %esp /* pop stack */
	jmp	*%eax
	.size	verify_code, .-verify_code

.globl cc_interrupt
	.hidden cc_interrupt
	.type	cc_interrupt, @function
cc_interrupt:
	add	last_count, %esi
	add	$-28, %esp /* Align stack */
	mov	%esi, reg_cop0+36 /* Count */
	shr	$19, %esi
	movl	$0, pending_exception
	and	$0x7f, %esi
	cmpl	$0, restore_candidate(,%esi,4)
	jne	.E4
.E1:
	call	gen_interupt
	mov	reg_cop0+36, %esi
	mov	next_interupt, %eax
	mov	pending_exception, %ebx
	mov	stop, %ecx
	add	$28, %esp
	mov	%eax, last_count
	sub	%eax, %esi
	test	%ecx, %ecx
	jne	.E3
	test	%ebx, %ebx
	jne	.E2
	ret
.E2:
	add	$-8, %esp
	mov	pcaddr, %edi
	mov	%esi, cycle_count /* CCREG */
	push	%edi
	call	get_addr_ht
	mov	cycle_count, %esi
	add	$16, %esp
	jmp	*%eax
.E3:
	add	$16, %esp /* pop stack */
	pop	%edi /* restore edi */
	pop	%esi /* restore esi */
	pop	%ebx /* restore ebx */
	pop	%ebp /* restore ebp */
	ret	     /* exit dynarec */
.E4:
	/* Move 'dirty' blocks to the 'clean' list */
	mov	restore_candidate(,%esi,4), %ebx
	mov	%esi, %ebp
	movl	$0, restore_candidate(,%esi,4)
	shl	$5, %ebp
.E5:
	shr	$1, %ebx
	jnc	.E6
	mov	%ebp, (%esp)
	call	clean_blocks
.E6:
	inc	%ebp
	test	$31, %ebp
	jne	.E5
	jmp	.E1
	.size	cc_interrupt, .-cc_interrupt

.globl do_interrupt
	.hidden do_interrupt
	.type	do_interrupt, @function
do_interrupt:
	mov	pcaddr, %edi
	add	$-12, %esp
	push	%edi
	call	get_addr_ht
	add	$16, %esp
	mov	reg_cop0+36, %esi
	mov	next_interupt, %ebx
	mov	%ebx, last_count
	sub	%ebx, %esi
	add	$2, %esi
	jmp	*%eax
	.size	do_interrupt, .-do_interrupt

.globl fp_exception
	.hidden fp_exception
	.type	fp_exception, @function
fp_exception:
	mov	$0x1000002c, %edx
.E7:
	mov	reg_cop0+48, %ebx
	add	$-12, %esp
	or	$2, %ebx
	mov	%ebx, reg_cop0+48 /* Status */
	mov	%edx, reg_cop0+52 /* Cause */
	mov	%eax, reg_cop0+56 /* EPC */
	push    $0x80000180
	call	get_addr_ht
	add	$16, %esp
	jmp	*%eax
	.size	fp_exception, .-fp_exception

.globl fp_exception_ds
	.hidden fp_exception_ds
	.type	fp_exception_ds, @function
fp_exception_ds:
	mov	$0x9000002c, %edx /* Set high bit if delay slot */
	jmp	.E7
	.size	fp_exception_ds, .-fp_exception_ds

.globl jump_syscall
	.hidden jump_syscall
	.type	jump_syscall, @function
jump_syscall:
	mov	$0x20, %edx
	mov	reg_cop0+48, %ebx
	add	$-12, %esp
	or	$2, %ebx
	mov	%ebx, reg_cop0+48 /* Status */
	mov	%edx, reg_cop0+52 /* Cause */
	mov	%eax, reg_cop0+56 /* EPC */
	push    $0x80000180
	call	get_addr_ht
	mov	$16, %esp
	jmp	*%eax
	.size	jump_syscall, .-jump_syscall

.globl jump_eret
	.hidden jump_eret
	.type	jump_eret, @function
jump_eret:
	mov	reg_cop0+48, %ebx /* Status */
	add	last_count, %esi
	and	$0xFFFFFFFD, %ebx
	mov	%esi, reg_cop0+36 /* Count */
	mov	%ebx, reg_cop0+48 /* Status */
	call	check_interupt
	mov	next_interupt, %eax
	mov	reg_cop0+36, %esi
	mov	%eax, last_count
	sub	%eax, %esi
	mov	reg_cop0+56, %eax /* EPC */
	jns	.E11
.E8:
	mov	$248, %ebx
	xor	%edi, %edi
.E9:
	mov	reg(%ebx), %ecx
	mov	reg+4(%ebx), %edx
	sar	$31, %ecx
	xor	%ecx, %edx
	neg	%edx
	adc	%edi, %edi
	sub	$8, %ebx
	jne	.E9
	mov	hi(%ebx), %ecx
	mov	hi+4(%ebx), %edx
	sar	$31, %ecx
	xor	%ecx, %edx
	jne	.E10
	mov	lo(%ebx), %ecx
	mov	lo+4(%ebx), %edx
	sar	$31, %ecx
	xor	%ecx, %edx
.E10:
	neg	%edx
	adc	%edi, %edi
	add	$-8, %esp
	push	%edi
	push	%eax
	mov	%esi, cycle_count
	call	get_addr_32
	mov	cycle_count, %esi
	add	$16, %esp
	jmp	*%eax
.E11:
	mov	%eax, pcaddr
	call	cc_interrupt
	mov	pcaddr, %eax
	jmp	.E8
	.size	jump_eret, .-jump_eret

.globl new_dyna_start
	.hidden new_dyna_start
	.type	new_dyna_start, @function
new_dyna_start:
	push	%ebp
	push	%ebx
	push	%esi
	push	%edi
	add	$-8, %esp /* align stack */
	push	$0xa4000040
	call	new_recompile_block
	movl	next_interupt, %edi
	movl	reg_cop0+36, %esi
	movl	%edi, last_count
	subl	%edi, %esi
	jmp	*base_addr
	.size	new_dyna_start, .-new_dyna_start

/* Note: Assumes %ebx, %ebp, %esi, %edi are callee-saved */
.globl invalidate_block_eax
	.hidden invalidate_block_eax
	.type	invalidate_block_eax, @function
invalidate_block_eax:
	push	%eax
	push	%ecx
	push	%edx
	push	%eax
	jmp	invalidate_block_call
	.size	invalidate_block_eax, .-invalidate_block_eax
.globl invalidate_block_ecx
	.hidden invalidate_block_ecx
	.type	invalidate_block_ecx, @function
invalidate_block_ecx:
	push	%eax
	push	%ecx
	push	%edx
	push	%ecx
	jmp	invalidate_block_call
	.size	invalidate_block_ecx, .-invalidate_block_ecx
.globl invalidate_block_edx
	.hidden invalidate_block_edx
	.type	invalidate_block_edx, @function
invalidate_block_edx:
	push	%eax
	push	%ecx
	push	%edx
	push	%edx
	jmp	invalidate_block_call
	.size	invalidate_block_edx, .-invalidate_block_edx
.globl invalidate_block_ebx
	.hidden invalidate_block_ebx
	.type	invalidate_block_ebx, @function
invalidate_block_ebx:
	push	%eax
	push	%ecx
	push	%edx
	push	%ebx
	jmp	invalidate_block_call
	.size	invalidate_block_ebx, .-invalidate_block_ebx
.globl invalidate_block_ebp
	.hidden invalidate_block_ebp
	.type	invalidate_block_ebp, @function
invalidate_block_ebp:
	push	%eax
	push	%ecx
	push	%edx
	push	%ebp
	jmp	invalidate_block_call
	.size	invalidate_block_ebp, .-invalidate_block_ebp
.globl invalidate_block_esi
	.hidden invalidate_block_esi
	.type	invalidate_block_esi, @function
invalidate_block_esi:
	push	%eax
	push	%ecx
	push	%edx
	push	%esi
	jmp	invalidate_block_call
	.size	invalidate_block_esi, .-invalidate_block_esi
.globl invalidate_block_edi
	.hidden invalidate_block_edi
	.type	invalidate_block_edi, @function
invalidate_block_edi:
	push	%eax
	push	%ecx
	push	%edx
	push	%edi
	.size	invalidate_block_edi, .-invalidate_block_edi

	.type	invalidate_block_call, @function
invalidate_block_call:
	call	invalidate_block
	pop	%eax /* Throw away */
	pop	%edx
	pop	%ecx
	pop	%eax
	ret
	.size	invalidate_block_call, .-invalidate_block_call

.globl write_rdram_new
	.hidden write_rdram_new
	.type	write_rdram_new, @function
write_rdram_new:
	mov	address, %edi
	mov	word, %ecx
	mov	%ecx, rdram-0x80000000(%edi)
	jmp	.E12
	.size	write_rdram_new, .-write_rdram_new

.globl write_rdramb_new
	.hidden write_rdramb_new
	.type	write_rdramb_new, @function
write_rdramb_new:
	mov	address, %edi
	xor	$3, %edi
	movb	cpu_byte, %cl
	movb	%cl, rdram-0x80000000(%edi)
	jmp	.E12
	.size	write_rdramb_new, .-write_rdramb_new

.globl write_rdramh_new
	.hidden write_rdramh_new
	.type	write_rdramh_new, @function
write_rdramh_new:
	mov	address, %edi
	xor	$2, %edi
	movw	hword, %cx
	movw	%cx, rdram-0x80000000(%edi)
	jmp	.E12
	.size	write_rdramh_new, .-write_rdramh_new

.globl write_rdramd_new
	.hidden write_rdramd_new
	.type	write_rdramd_new, @function
write_rdramd_new:
	mov	address, %edi
	mov	dword+4, %ecx
	mov	dword, %edx
	mov	%ecx, rdram-0x80000000(%edi)
	mov	%edx, rdram-0x80000000+4(%edi)
	jmp	.E12
	.size	write_rdramd_new, .-write_rdramd_new

	.type	do_invalidate, @function
do_invalidate:
	mov	address, %edi
	mov	%edi, %ebx /* Return ebx to caller */
.E12:
	shr	$12, %edi
	cmpb	$1, invalid_code(%edi)
	je	.E13
	push	%edi
	call	invalidate_block
	pop	%edi
.E13:
	ret
	.size	do_invalidate, .-do_invalidate

.globl read_nomem_new
	.hidden read_nomem_new
	.type	read_nomem_new, @function
read_nomem_new:
	mov	address, %edi
	mov	%edi, %ebx
	shr	$12, %edi
	mov	memory_map(,%edi,4),%edi
	mov	$0x8, %eax
	test	%edi, %edi
	js	tlb_exception
	mov	(%ebx,%edi,4), %ecx
	mov	%ecx, readmem_dword
	ret
	.size	read_nomem_new, .-read_nomem_new

.globl read_nomemb_new
	.hidden read_nomemb_new
	.type	read_nomemb_new, @function
read_nomemb_new:
	mov	address, %edi
	mov	%edi, %ebx
	shr	$12, %edi
	mov	memory_map(,%edi,4),%edi
	mov	$0x8, %eax
	test	%edi, %edi
	js	tlb_exception
	xor	$3, %ebx
	movzbl	(%ebx,%edi,4), %ecx
	mov	%ecx, readmem_dword
	ret
	.size	read_nomemb_new, .-read_nomemb_new

.globl read_nomemh_new
	.hidden read_nomemh_new
	.type	read_nomemh_new, @function
read_nomemh_new:
	mov	address, %edi
	mov	%edi, %ebx
	shr	$12, %edi
	mov	memory_map(,%edi,4),%edi
	mov	$0x8, %eax
	test	%edi, %edi
	js	tlb_exception
	xor	$2, %ebx
	movzwl	(%ebx,%edi,4), %ecx
	mov	%ecx, readmem_dword
	ret
	.size	read_nomemh_new, .-read_nomemh_new

.globl read_nomemd_new
	.hidden read_nomemd_new
	.type	read_nomemd_new, @function
read_nomemd_new:
	mov	address, %edi
	mov	%edi, %ebx
	shr	$12, %edi
	mov	memory_map(,%edi,4),%edi
	mov	$0x8, %eax
	test	%edi, %edi
	js	tlb_exception
	mov	4(%ebx,%edi,4), %ecx
	mov	(%ebx,%edi,4), %edx
	mov	%ecx, readmem_dword
	mov	%edx, readmem_dword+4
	ret
	.size	read_nomemd_new, .-read_nomemd_new

.globl write_nomem_new
	.hidden write_nomem_new
	.type	write_nomem_new, @function
write_nomem_new:
	call	do_invalidate
	mov	memory_map(,%edi,4),%edi
	mov	word, %ecx
	mov	$0xc, %eax
	shl	$2, %edi
	jc	tlb_exception
	mov	%ecx, (%ebx,%edi)
	ret
	.size	write_nomem_new, .-write_nomem_new

.globl write_nomemb_new
	.hidden write_nomemb_new
	.type	write_nomemb_new, @function
write_nomemb_new:
	call	do_invalidate
	mov	memory_map(,%edi,4),%edi
	movb	cpu_byte, %cl
	mov	$0xc, %eax
	shl	$2, %edi
	jc	tlb_exception
	xor	$3, %ebx
	movb	%cl, (%ebx,%edi)
	ret
	.size	write_nomemb_new, .-write_nomemb_new

.globl write_nomemh_new
	.hidden write_nomemh_new
	.type	write_nomemh_new, @function
write_nomemh_new:
	call	do_invalidate
	mov	memory_map(,%edi,4),%edi
	movw	hword, %cx
	mov	$0xc, %eax
	shl	$2, %edi
	jc	tlb_exception
	xor	$2, %ebx
	movw	%cx, (%ebx,%edi)
	ret
	.size	write_nomemh_new, .-write_nomemh_new

.globl write_nomemd_new
	.hidden write_nomemd_new
	.type	write_nomemd_new, @function
write_nomemd_new:
	call	do_invalidate
	mov	memory_map(,%edi,4),%edi
	mov	dword+4, %edx
	mov	dword, %ecx
	mov	$0xc, %eax
	shl	$2, %edi
	jc	tlb_exception
	mov	%edx, (%ebx,%edi)
	mov	%ecx, 4(%ebx,%edi)
	ret
	.size	write_nomemd_new, .-write_nomemd_new

	.type	tlb_exception, @function
tlb_exception:
	/* eax = cause */
	/* ebx = address */
	/* ebp = instr addr + flags */
	mov	0x24(%esp), %ebp
/* Debug: 
	push	%ebp
	push	%ebx
	push	%eax
	call	tlb_debug
	pop	%eax
	pop	%ebx
	pop	%ebp
/* end debug */
	mov	reg_cop0+48, %esi
	mov	%ebp, %ecx
	mov	%ebp, %edx
	mov	%ebp, %edi
	shl	$31, %ebp
	shr	$12, %ecx
	or	%ebp, %eax
	sar	$29, %ebp
	and	$0xFFFFFFFC, %edx
	mov	memory_map(,%ecx,4), %ecx
	or	$2, %esi
	mov	(%edx, %ecx, 4), %ecx
	add	%ebp, %edx
	mov	%esi, reg_cop0+48 /* Status */
	mov	%eax, reg_cop0+52 /* Cause */
	mov	%edx, reg_cop0+56 /* EPC */
	add	$0x24, %esp
	mov	$0x6000022, %edx
	mov	%ecx, %ebp
	movswl	%cx, %eax
	shr	$26, %ecx
	shr	$21, %ebp
	sub	%eax, %ebx
	and	$0x1f, %ebp
	ror	%cl, %edx
	mov	reg_cop0+16, %esi
	cmovc	reg(,%ebp,8), %ebx
	and	$0xFF80000F, %esi
	mov	%ebx, reg(,%ebp,8)
	add	%ebx, %eax
	sar	$31, %ebx
	mov	%eax, reg_cop0+32 /* BadVAddr */
	shr	$9, %eax
	test	$2, %edi
	cmove	reg+4(,%ebp,8), %ebx
	add	$-12, %esp
	and	$0x007FFFF0, %eax
	mov	%ebx, reg+4(,%ebp,8)
	push	$0x80000180
	or	%eax, %esi
	mov	%esi, reg_cop0+16 /* Context */
	call	get_addr_ht
	add	$16, %esp
	movl	next_interupt, %edi
	movl	reg_cop0+36, %esi /* Count */
	movl	%edi, last_count
	subl	%edi, %esi
	jmp	*%eax
	.size	tlb_exception, .-tlb_exception

.globl breakpoint
	.hidden breakpoint
	.type	breakpoint, @function
breakpoint:
	.size	breakpoint, .-breakpoint
