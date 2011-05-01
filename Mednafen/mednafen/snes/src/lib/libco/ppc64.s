;*****
;libco.ppc64 (2007-12-05)
;author: Vas Crabb
;license: public domain
;
;cross-platform 64-bit PowerPC implementation of libco
;special thanks to byuu for writing the original version
;
;[ABI compatibility]
;- gcc; mac os x; ppc64
;
;[nonvolatile registers]
;- GPR1, GPR13 - GPR31
;- FPR14 - FPR31
;- V20 - V31
;- VRSAVE, CR2 - CR4
;
;[volatile registers]
;- GPR0, GPR2 - GPR12
;- FPR0 - FPR13
;- V0 - V19
;- LR, CTR, XER, CR0, CR1, CR5 - CR7
;*****


;Declare some target-specific stuff

    .section __TEXT,__text,regular,pure_instructions
    .section __TEXT,__picsymbolstub1,symbol_stubs,pure_instructions,32
    .machine ppc64


;Constants

    .cstring
    .align 3

_sysctl_altivec:
    .ascii "hw.optional.altivec\0"


;Declare space for variables

.lcomm _co_environ,4,2                                      ;bit 0 = initialised, bit 1 = have Altivec/VMX
.lcomm _co_primary_buffer,1024,3                            ;buffer (will be zeroed by loader)

    .data
    .align 3

_co_active_context:
    .quad   _co_primary_buffer


    .text
    .align 2


;Declare exported names

.globl _co_active
.globl _co_create
.globl _co_delete
.globl _co_switch


;*****
;extern "C" cothread_t co_active();
;return = GPR3
;*****

_co_active:
    mflr    r0                                              ;GPR0 = return address
    bcl     20,31,L_co_active$spb
L_co_active$spb:
    mflr    r2                                              ;GPR2 set for position-independance
    addis   r3,r2,ha16(_co_active_context-L_co_active$spb)  ;get value in GPR3
    ld      r3,lo16(_co_active_context-L_co_active$spb)(r3)
    mtlr    r0                                              ;LR = return address
    blr                                                     ;return


;*****
;extern "C" cothread_t co_create(unsigned int heapsize, void (*coentry)());
;GPR3   = heapsize
;GPR4   = coentry
;return = GPR3
;*****

_co_create:
    mflr    r0                                              ;GPR0 = return address
    std     r30,-16(r1)                                     ;save GPR30 and GPR31
    std     r31,-8(r1)
    std     r0,16(r1)                                       ;save return address
    stdu    r1,-(2*8+16+48)(r1)                             ;allocate 16 bytes for locals/parameters

;create heap space (stack + register storage)
    addi    r31,r3,1024-48                                  ;subtract space for linkage
    mr      r30,r4                                          ;GPR30 = coentry
    addi    r3,r3,1024                                      ;allocate extra memory for contextual info
    bl      L_malloc$stub                                   ;GPR3 = malloc(heapsize + 1024)
    add     r4,r3,r31                                       ;GPR4 points to top-of-stack
    rldicr  r5,r4,0,59                                      ;force 16-byte alignment

;store thread entry point + registers, so that first call to co_switch will execute coentry
    std     r30,16(r5)                                      ;store entry point
    addi    r6,0,2+19+18+12*2+1                             ;clear for CR, old GPR1, 19 GPRs, 18 FPRs, 12 VRs, VRSAVE
    addi    r0,0,0
    addi    r7,0,8                                          ;start at 8(GPR5)
    mtctr   r6
L_co_create$clear_loop:
    stdx    r0,r5,r7                                        ;clear a double
    addi    r7,r7,-8                                        ;increment pointer
    bdnz    L_co_create$clear_loop                          ;loop
    stdu    r5,-544(r5)                                     ;store top of stack

;initialize context memory heap and return
    addis   r9,0,0x8000                                     ;GPR13 not set (system TLS)
    std     r5,0(r3)                                        ;*cothread_t = stack heap pointer (GPR1)
    stw     r9,8(r3)                                        ;this is a flag word
    ld      r1,0(r1)                                        ;deallocate stack frame
    ld      r8,16(r1)                                       ;fetch return address
    ld      r30,-16(r1)                                     ;restore GPR30 and GPR31
    ld      r31,-8(r1)
    mtlr    r8                                              ;return address in LR
    blr                                                     ;return


;*****
;extern "C" void co_delete(cothread_t cothread);
;GPR3 = cothread
;*****

_co_delete:
    b      L_free$stub                                      ;free(GPR3)


;*****
;extern "C" void co_switch(cothread_t cothread);
;GPR3 = cothread
;*****
;
;Frame looks like:
;
;Old        New         Value
;  16(r1)    560(r1)    Saved LR
;   8(r1)    552(r1)    Saved CR
;   0(r1)    544(r1)    Old GPR1
;  -8(r1)    536(r1)    Saved GPR31
; -16(r1)    528(r1)    Saved GPR30
;...        ...         ...
;-144(r1)    400(r1)    Saved GPR14
;-152(r1)    392(r1)    Saved GPR13
;-160(r1)    384(r1)    Saved FPR31
;-168(r1)    376(r1)    Saved FPR30
;...        ...         ...
;-288(r1)    256(r1)    Saved FPR15
;-296(r1)    248(r1)    Saved FPR14
;-304(r1)    240(r1)    Saved VRSAVE
;-312(r1)    232(r1)    +++             value
;-320(r1)    224(r1)    Saved VR31      len
;-328(r1)    216(r1)    +++
;-336(r1)    208(r1)    Saved VR30
;...        ...         ...
;-456(r1)     88(r1)    +++
;-464(r1)     80(r1)    Saved VR22      Param 5 (GPR7)
;-472(r1)     72(r1)    +++             Param 4 (GPR6)
;-480(r1)     64(r1)    Saved VR21      Param 3 (GPR5)
;-488(r1)     56(r1)    +++             Param 2 (GPR4)
;-496(r1)     48(r1)    Saved VR20      Param 1 (GPR3)
;-504(r1)     40(r1)    -               Reserved
;-512(r1)     32(r1)    -               Reserved
;-520(r1)     24(r1)    -               Reserved
;-528(r1)     16(r1)    -               New LR
;-536(r1)      8(r1)    -               New CR
;-544(r1)      0(r1)    Saved GPR1


_co_switch:
    std     r13,-152(r1)                                    ;save preserved GPRs
    std     r14,-144(r1)
    std     r15,-136(r1)
    std     r16,-128(r1)
    std     r17,-120(r1)
    std     r18,-112(r1)
    std     r19,-104(r1)
    std     r20,-96(r1)
    std     r21,-88(r1)
    std     r22,-80(r1)
    std     r23,-72(r1)
    std     r24,-64(r1)
    std     r25,-56(r1)
    std     r26,-48(r1)
    std     r27,-40(r1)
    std     r28,-32(r1)
    std     r29,-24(r1)
    std     r30,-16(r1)
    std     r31,-8(r1)
    mflr    r0                                              ;save return address
    std     r0,16(r1)
    mfcr    r2                                              ;save condition codes
    stw     r2,8(r1)
    stdu    r1,-544(r1)                                     ;create stack frame (save 19 GPRs, 18 FRPs, 12 VRs, VRSAVE)
    stfd    f14,248(r1)                                     ;save preserved FPRs
    stfd    f15,256(r1)
    stfd    f16,264(r1)
    stfd    f17,272(r1)
    stfd    f18,280(r1)
    stfd    f19,288(r1)
    stfd    f20,296(r1)
    stfd    f21,304(r1)
    stfd    f22,312(r1)
    stfd    f23,320(r1)
    stfd    f24,328(r1)
    stfd    f25,336(r1)
    stfd    f26,344(r1)
    stfd    f27,352(r1)
    stfd    f28,360(r1)
    stfd    f29,368(r1)
    stfd    f30,376(r1)
    stfd    f31,384(r1)

    mr      r30,r3                                          ;save new context pointer
    bcl     20,31,L_co_switch$spb                           ;get address of co_active_context
L_co_switch$spb:
    mflr    r31

    addis   r29,r31,ha16(_co_environ-L_co_switch$spb)       ;get environment flags
    lwz     r8,lo16(_co_environ-L_co_switch$spb)(r29)
    andis.  r9,r8,0x8000                                    ;is it initialised?
    bne+    L_co_switch$initialised

    addi    r0,0,4                                          ;len = sizeof(int)
    std     r0,224(r1)
    addis   r3,r31,ha16(_sysctl_altivec-L_co_switch$spb)    ;GPR3 = "hw.optional.altivec"
    addi    r3,r3,lo16(_sysctl_altivec-L_co_switch$spb)
    addi    r4,r1,232                                       ;GPR4 = &value
    addi    r5,r1,224                                       ;GPR5 = &len
    addi    r6,0,0                                          ;newp = 0
    addi    r7,0,0                                          ;newlen = 0
    bl      L_sysctlbyname$stub                             ;call sysctlbyname
    lwz     r2,232(r1)                                      ;fetch result
    addis   r8,0,0x8000                                     ;set initialised bit
    cmpdi   cr5,r3,0                                        ;assume error means not present
    cmpwi   cr6,r2,0                                        ;test result
    blt-    cr5,L_co_switch$store_environ
    beq     cr6,L_co_switch$store_environ
    oris    r8,r8,0x4000                                    ;set the flag to say we have it!
L_co_switch$store_environ:
    stw     r8,lo16(_co_environ-L_co_switch$spb)(r29)       ;store environment flags
L_co_switch$initialised:

    andis.  r10,r8,0x4000                                   ;do we have Altivec/VMX?
    beq     L_co_switch$save_no_vmx
    mfspr   r11,256                                         ;save VRSAVE
    andi.   r0,r11,0x0FFF                                   ;short-circuit if it's zero
    stw     r11,240(r1)
    beq     L_co_switch$save_no_vmx
    andi.   r0,r11,0x0800                                   ;check bit 20
    addi    r2,0,48                                         ;starting index
    beq     L_co_switch$save_skip_vr20
    stvx    v20,r1,r2                                       ;save VR20
L_co_switch$save_skip_vr20:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0400                                   ;check bit 21
    beq     L_co_switch$save_skip_vr21
    stvx    v21,r1,r2                                       ;save VR21
L_co_switch$save_skip_vr21:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0200                                   ;check bit 22
    beq     L_co_switch$save_skip_vr22
    stvx    v22,r1,r2                                       ;save VR22
L_co_switch$save_skip_vr22:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0100                                   ;check bit 23
    beq     L_co_switch$save_skip_vr23
    stvx    v23,r1,r2                                       ;save VR23
L_co_switch$save_skip_vr23:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0080                                   ;check bit 24
    beq     L_co_switch$save_skip_vr24
    stvx    v24,r1,r2                                       ;save VR24
L_co_switch$save_skip_vr24:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0040                                   ;check bit 25
    beq     L_co_switch$save_skip_vr25
    stvx    v25,r1,r2                                       ;save VR25
L_co_switch$save_skip_vr25:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0020                                   ;check bit 26
    beq     L_co_switch$save_skip_vr26
    stvx    v26,r1,r2                                       ;save VR26
L_co_switch$save_skip_vr26:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0010                                   ;check bit 27
    beq     L_co_switch$save_skip_vr27
    stvx    v27,r1,r2                                       ;save VR27
L_co_switch$save_skip_vr27:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0008                                   ;check bit 28
    beq     L_co_switch$save_skip_vr28
    stvx    v28,r1,r2                                       ;save VR28
L_co_switch$save_skip_vr28:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0004                                   ;check bit 29
    beq     L_co_switch$save_skip_vr29
    stvx    v29,r1,r2                                       ;save VR29
L_co_switch$save_skip_vr29:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0002                                   ;check bit 30
    beq     L_co_switch$save_skip_vr30
    stvx    v30,r1,r2                                       ;save VR30
L_co_switch$save_skip_vr30:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0001                                   ;check bit 31
    beq     L_co_switch$save_skip_vr31
    stvx    v31,r1,r2                                       ;save VR31
L_co_switch$save_skip_vr31:
L_co_switch$save_no_vmx:

    addis   r4,r31,ha16(_co_active_context-L_co_switch$spb) ;save current context
    ld      r5,lo16(_co_active_context-L_co_switch$spb)(r4)
    std     r30,lo16(_co_active_context-L_co_switch$spb)(r4);set new context
    std     r1,0(r5)                                        ;save current stack pointer
    ld      r1,0(r30)                                       ;get new stack pointer
    lwz     r12,8(r30)                                      ;have we already set GPR13 (system TLS)?
    andis.  r0,r12,0x8000
    beq+    L_co_switch$gpr13_set
    std     r13,392(r1)
    xoris   r12,r12,0x8000
    stw     r12,8(r30)
L_co_switch$gpr13_set:

    andis.  r10,r8,0x4000                                   ;do we have Altivec/VMX?
    beq     L_co_switch$restore_no_vmx
    lwz     r11,240(r1)                                     ;restore VRSAVE
    andi.   r0,r11,0x0FFF                                   ;short-circuit if it's zero
    mtspr   256,r11
    beq     L_co_switch$restore_no_vmx
    andi.   r0,r11,0x0800                                   ;check bit 20
    addi    r2,0,48                                         ;starting index
    beq     L_co_switch$restore_skip_vr20
    lvx     v20,r1,r2                                       ;restore VR20
L_co_switch$restore_skip_vr20:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0400                                   ;check bit 21
    beq     L_co_switch$restore_skip_vr21
    lvx     v21,r1,r2                                       ;restore VR21
L_co_switch$restore_skip_vr21:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0200                                   ;check bit 22
    beq     L_co_switch$restore_skip_vr22
    lvx     v22,r1,r2                                       ;restore VR22
L_co_switch$restore_skip_vr22:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0100                                   ;check bit 23
    beq     L_co_switch$restore_skip_vr23
    lvx     v23,r1,r2                                       ;restore VR23
L_co_switch$restore_skip_vr23:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0080                                   ;check bit 24
    beq     L_co_switch$restore_skip_vr24
    lvx     v24,r1,r2                                       ;restore VR24
L_co_switch$restore_skip_vr24:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0040                                   ;check bit 25
    beq     L_co_switch$restore_skip_vr25
    lvx     v25,r1,r2                                       ;restore VR25
L_co_switch$restore_skip_vr25:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0020                                   ;check bit 26
    beq     L_co_switch$restore_skip_vr26
    lvx     v26,r1,r2                                       ;restore VR26
L_co_switch$restore_skip_vr26:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0010                                   ;check bit 27
    beq     L_co_switch$restore_skip_vr27
    lvx     v27,r1,r2                                       ;restore VR27
L_co_switch$restore_skip_vr27:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0008                                   ;check bit 28
    beq     L_co_switch$restore_skip_vr28
    lvx     v28,r1,r2                                       ;restore VR28
L_co_switch$restore_skip_vr28:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0004                                   ;check bit 29
    beq     L_co_switch$restore_skip_vr29
    lvx     v29,r1,r2                                       ;restore VR29
L_co_switch$restore_skip_vr29:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0002                                   ;check bit 30
    beq     L_co_switch$restore_skip_vr30
    lvx     v30,r1,r2                                       ;restore VR30
L_co_switch$restore_skip_vr30:
    addi    r2,r2,16                                        ;stride
    andi.   r0,r11,0x0001                                   ;check bit 31
    beq     L_co_switch$restore_skip_vr31
    lvx     v31,r1,r2                                       ;restore VR31
L_co_switch$restore_skip_vr31:
L_co_switch$restore_no_vmx:

    lfd     f14,248(r1)                                     ;restore preserved FPRs
    lfd     f15,256(r1)
    lfd     f16,264(r1)
    lfd     f17,272(r1)
    lfd     f18,280(r1)
    lfd     f19,288(r1)
    lfd     f20,296(r1)
    lfd     f21,304(r1)
    lfd     f22,312(r1)
    lfd     f23,320(r1)
    lfd     f24,328(r1)
    lfd     f25,336(r1)
    lfd     f26,344(r1)
    lfd     f27,352(r1)
    lfd     f28,360(r1)
    lfd     f29,368(r1)
    lfd     f30,376(r1)
    lfd     f31,384(r1)
    addi    r0,0,0                                          ;make thread main crash if it returns
    ld      r1,0(r1)                                        ;deallocate stack frame
    ld      r6,16(r1)                                       ;return address in GPR6
    lwz     r7,8(r1)                                        ;condition codes in GPR7
    ld      r13,-152(r1)                                    ;restore preserved GPRs
    ld      r14,-144(r1)
    ld      r15,-136(r1)
    ld      r16,-128(r1)
    ld      r17,-120(r1)
    ld      r18,-112(r1)
    ld      r19,-104(r1)
    ld      r20,-96(r1)
    ld      r21,-88(r1)
    ld      r22,-80(r1)
    ld      r23,-72(r1)
    ld      r24,-64(r1)
    ld      r25,-56(r1)
    ld      r26,-48(r1)
    ld      r27,-40(r1)
    ld      r28,-32(r1)
    ld      r29,-24(r1)
    ld      r30,-16(r1)
    ld      r31,-8(r1)
    mtlr    r0
    mtctr   r6                                              ;restore return address
    mtcrf   32,r7                                           ;restore preserved condition codes
    mtcrf   16,r7
    mtcrf   8,r7
    bctr                                                    ;return



;Import external functions

    .section __TEXT,__picsymbolstub1,symbol_stubs,pure_instructions,32
    .align 5
L_malloc$stub:
    .indirect_symbol _malloc
    mflr    r0
    bcl     20,31,L_malloc$spb
L_malloc$spb:
    mflr    r11
    addis   r11,r11,ha16(L_malloc$lazy_ptr-L_malloc$spb)
    mtlr    r0
    ldu     r12,lo16(L_malloc$lazy_ptr-L_malloc$spb)(r11)
    mtctr   r12
    bctr
    .lazy_symbol_pointer
L_malloc$lazy_ptr:
    .indirect_symbol _malloc
    .quad   dyld_stub_binding_helper


    .section __TEXT,__picsymbolstub1,symbol_stubs,pure_instructions,32
    .align 5
L_free$stub:
    .indirect_symbol _free
    mflr    r0
    bcl     20,31,L_free$spb
L_free$spb:
    mflr    r11
    addis   r11,r11,ha16(L_free$lazy_ptr-L_free$spb)
    mtlr    r0
    ldu     r12,lo16(L_free$lazy_ptr-L_free$spb)(r11)
    mtctr   r12
    bctr
    .lazy_symbol_pointer
L_free$lazy_ptr:
    .indirect_symbol _free
    .quad   dyld_stub_binding_helper


    .section __TEXT,__picsymbolstub1,symbol_stubs,pure_instructions,32
    .align 5
L_sysctlbyname$stub:
    .indirect_symbol _sysctlbyname
    mflr    r0
    bcl     20,31,L_sysctlbyname$spb
L_sysctlbyname$spb:
    mflr    r11
    addis   r11,r11,ha16(L_sysctlbyname$lazy_ptr-L_sysctlbyname$spb)
    mtlr    r0
    ldu     r12,lo16(L_sysctlbyname$lazy_ptr-L_sysctlbyname$spb)(r11)
    mtctr   r12
    bctr
    .lazy_symbol_pointer
L_sysctlbyname$lazy_ptr:
    .indirect_symbol _sysctlbyname
    .quad   dyld_stub_binding_helper


;This needs to be here!

    .subsections_via_symbols

