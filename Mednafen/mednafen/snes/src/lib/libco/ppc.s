;*****
;libco.ppc (2007-11-29)
;author: Vas Crabb
;license: public domain
;
;cross-platform PowerPC implementation of libco
;special thanks to byuu for writing the original version
;
;[ABI compatibility]
;- gcc; mac os x; ppc
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
    .machine ppc


;Constants

    .cstring
    .align 2

_sysctl_altivec:
    .ascii "hw.optional.altivec\0"


;Declare space for variables

.lcomm _co_environ,4,2                                      ;bit 0 = initialised, bit 1 = have Altivec/VMX
.lcomm _co_primary_buffer,1024,2                            ;buffer (will be zeroed by loader)

    .data
    .align 2

_co_active_context:
    .long   _co_primary_buffer


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
    lwz     r3,lo16(_co_active_context-L_co_active$spb)(r3)
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
    stmw    r30,-8(r1)                                      ;save GPR30 and GPR31
    stw     r0,8(r1)                                        ;save return address
    stwu    r1,-(2*4+16+24)(r1)                             ;allocate 16 bytes for locals/parameters

;create heap space (stack + register storage)
    addi    r31,r3,1024-24                                  ;subtract space for linkage
    mr      r30,r4                                          ;GPR30 = coentry
    addi    r3,r3,1024                                      ;allocate extra memory for contextual info
    bl      L_malloc$stub                                   ;GPR3 = malloc(heapsize + 1024)
    add     r4,r3,r31                                       ;GPR4 points to top-of-stack
    rlwinm  r5,r4,0,0,27                                    ;force 16-byte alignment

;store thread entry point + registers, so that first call to co_switch will execute coentry
    stw     r30,8(r5)                                       ;store entry point
    addi    r6,0,2+19+18*2+12*4+1                           ;clear for CR, old GPR1, 19 GPRs, 18 FPRs, 12 VRs, VRSAVE
    addi    r0,0,0
    addi    r7,0,4                                          ;start at 4(GPR5)
    mtctr   r6
L_co_create$clear_loop:
    stwx    r0,r5,r7                                        ;clear a word
    addi    r7,r7,-4                                        ;increment pointer
    bdnz    L_co_create$clear_loop                          ;loop
    stwu    r5,-448(r5)                                     ;store top of stack

;initialize context memory heap and return
    stw     r5,0(r3)                                        ;*cothread_t = stack heap pointer (GPR1)
    lwz     r1,0(r1)                                        ;deallocate stack frame
    lwz     r8,8(r1)                                        ;fetch return address
    lmw     r30,-8(r1)                                      ;restore GPR30 and GPR31
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
;   8(r1)    456(r1)    Saved LR
;   4(r1)    452(r1)    Saved CR
;   0(r1)    448(r1)    Old GPR1
;  -4(r1)    444(r1)    Saved GPR31
;  -8(r1)    440(r1)    Saved GPR30
;...        ...         ...
; -72(r1)    376(r1)    Saved GPR14
; -76(r1)    372(r1)    Saved GPR13
; -80(r1)    368(r1)    Saved VRSAVE
; -84(r1)    364(r1)    +++
; -88(r1)    360(r1)    Saved FPR31
; -92(r1)    356(r1)    +++
; -96(r1)    352(r1)    Saved FPR30
;...        ...         ...
;-212(r1)    236(r1)    +++
;-216(r1)    232(r1)    Saved FPR15
;-220(r1)    228(r1)    +++
;-224(r1)    224(r1)    Saved FPR14
;-228(r1)    220(r1)    +++             value
;-232(r1)    216(r1)    +++             len
;-236(r1)    212(r1)    +++
;-240(r1)    208(r1)    Saved VR31
;-244(r1)    204(r1)    +++
;-248(r1)    200(r1)    +++
;-252(r1)    196(r1)    +++
;-256(r1)    192(r1)    Saved VR30
;...        ...         ...
;-388(r1)     60(r1)    +++
;-392(r1)     56(r1)    +++
;-396(r1)     52(r1)    +++
;-400(r1)     48(r1)    Saved VR21
;-404(r1)     44(r1)    +++
;-408(r1)     40(r1)    +++             Param 5 (GPR7)
;-412(r1)     36(r1)    +++             Param 4 (GPR6)
;-416(r1)     32(r1)    Saved VR20      Param 3 (GPR5)
;-420(r1)     28(r1)    -               Param 2 (GPR4)
;-424(r1)     24(r1)    -               Param 1 (GPR3)
;-428(r1)     20(r1)    -               Reserved
;-432(r1)     16(r1)    -               Reserved
;-436(r1)     12(r1)    -               Reserved
;-440(r1)      8(r1)    -               New LR
;-444(r1)      4(r1)    -               New CR
;-448(r1)      0(r1)    Saved GPR1


_co_switch:
    stmw    r13,-76(r1)                                     ;save preserved GPRs
    stfd    f14,-224(r1)                                    ;save preserved FPRs
    stfd    f15,-216(r1)
    stfd    f16,-208(r1)
    stfd    f17,-200(r1)
    stfd    f18,-192(r1)
    stfd    f19,-184(r1)
    stfd    f20,-176(r1)
    stfd    f21,-168(r1)
    stfd    f22,-160(r1)
    stfd    f23,-152(r1)
    stfd    f24,-144(r1)
    stfd    f25,-136(r1)
    stfd    f26,-128(r1)
    stfd    f27,-120(r1)
    stfd    f28,-112(r1)
    stfd    f29,-104(r1)
    stfd    f30,-96(r1)
    stfd    f31,-88(r1)
    mflr    r0                                              ;save return address
    stw     r0,8(r1)
    mfcr    r2                                              ;save condition codes
    stw     r2,4(r1)
    stwu    r1,-448(r1)                                     ;create stack frame (save 19 GPRs, 18 FRPs, 12 VRs, VRSAVE)

    mr      r30,r3                                          ;save new context pointer
    bcl     20,31,L_co_switch$spb                           ;get address of co_active_context
L_co_switch$spb:
    mflr    r31

    addis   r29,r31,ha16(_co_environ-L_co_switch$spb)       ;get environment flags
    lwz     r8,lo16(_co_environ-L_co_switch$spb)(r29)
    andis.  r9,r8,0x8000                                    ;is it initialised?
    bne+    L_co_switch$initialised

    addi    r0,0,4                                          ;len = sizeof(int)
    stw     r0,216(r1)
    addis   r3,r31,ha16(_sysctl_altivec-L_co_switch$spb)    ;GPR3 = "hw.optional.altivec"
    addi    r3,r3,lo16(_sysctl_altivec-L_co_switch$spb)
    addi    r4,r1,220                                       ;GPR4 = &value
    addi    r5,r1,216                                       ;GPR5 = &len
    addi    r6,0,0                                          ;newp = 0
    addi    r7,0,0                                          ;newlen = 0
    bl      L_sysctlbyname$stub                             ;call sysctlbyname
    lwz     r2,220(r1)                                      ;fetch result
    addis   r8,0,0x8000                                     ;set initialised bit
    cmpwi   cr5,r3,0                                        ;assume error means not present
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
    stw     r11,368(r1)
    beq     L_co_switch$save_no_vmx
    andi.   r0,r11,0x0800                                   ;check bit 20
    addi    r2,0,32                                         ;starting index
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
    lwz     r5,lo16(_co_active_context-L_co_switch$spb)(r4)
    stw     r30,lo16(_co_active_context-L_co_switch$spb)(r4);set new context
    stw     r1,0(r5)                                        ;save current stack pointer
    lwz     r1,0(r30)                                       ;get new stack pointer

    andis.  r10,r8,0x4000                                   ;do we have Altivec/VMX?
    beq     L_co_switch$restore_no_vmx
    lwz     r11,368(r1)                                     ;restore VRSAVE
    andi.   r0,r11,0x0FFF                                   ;short-circuit if it's zero
    mtspr   256,r11
    beq     L_co_switch$restore_no_vmx
    andi.   r0,r11,0x0800                                   ;check bit 20
    addi    r2,0,32                                         ;starting index
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

    lwz     r1,0(r1)                                        ;deallocate stack frame
    lwz     r6,8(r1)                                        ;return address in GPR6
    lwz     r7,4(r1)                                        ;condition codes in GPR7
    addi    r0,0,0                                          ;make thread main crash if it returns
    lmw     r13,-76(r1)                                     ;restore preserved GPRs
    lfd     f14,-224(r1)                                    ;restore preserved FPRs
    lfd     f15,-216(r1)
    lfd     f16,-208(r1)
    lfd     f17,-200(r1)
    lfd     f18,-192(r1)
    lfd     f19,-184(r1)
    lfd     f20,-176(r1)
    lfd     f21,-168(r1)
    lfd     f22,-160(r1)
    lfd     f23,-152(r1)
    lfd     f24,-144(r1)
    lfd     f25,-136(r1)
    lfd     f26,-128(r1)
    lfd     f27,-120(r1)
    lfd     f28,-112(r1)
    lfd     f29,-104(r1)
    lfd     f30,-96(r1)
    lfd     f31,-88(r1)
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
    lwzu    r12,lo16(L_malloc$lazy_ptr-L_malloc$spb)(r11)
    mtctr   r12
    bctr
    .lazy_symbol_pointer
L_malloc$lazy_ptr:
    .indirect_symbol _malloc
    .long   dyld_stub_binding_helper


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
    lwzu    r12,lo16(L_free$lazy_ptr-L_free$spb)(r11)
    mtctr   r12
    bctr
    .lazy_symbol_pointer
L_free$lazy_ptr:
    .indirect_symbol _free
    .long   dyld_stub_binding_helper


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
    lwzu    r12,lo16(L_sysctlbyname$lazy_ptr-L_sysctlbyname$spb)(r11)
    mtctr   r12
    bctr
    .lazy_symbol_pointer
L_sysctlbyname$lazy_ptr:
    .indirect_symbol _sysctlbyname
    .long   dyld_stub_binding_helper


;This needs to be here!

    .subsections_via_symbols

