
                        section .text

;----------------------------------------------------------------------------
; blur (void *src, int width, int height, int pitch)
;----------------------------------------------------------------------------
                        global  _blur
                        align   4
_blur:
                        pushad
                        mov     esi, [esp + 36]
                        mov     ebp, [esp + 44]
        .line
                        mov     ecx, [esp + 40]
                        xor     ebx, ebx
        .column
                        mov     ax, word [esi]
                        mov     dx, ax
                        shl     eax, 16
                        mov     ax, dx
                        and     eax, 0x07E0F81F

                        mov     edx, eax
                        add     eax, ebx
                        mov     ebx, edx
                        shr     eax, 1

                        and     eax, 0x07E0F81F
                        mov     dx, ax
                        shr     eax, 16
                        or      ax, dx

                        mov     word [esi], ax

                        add     esi, byte 2
                        dec     ecx
                        jnz     .column

                        add     esi, dword [esp + 48]

                        dec     ebp
                        jnz     .line

                        popad
                        ret
