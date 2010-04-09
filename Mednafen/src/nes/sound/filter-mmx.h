         // Warning, number of coefficients is hardcoded.
          asm volatile(
		//"emms\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
                "pushl %%edi\n\t"
		"pushl %%ecx\n\t"

                "movl %%ecx,%%ebp\n\t"
                "bigloop_mmx"FILTMMX_SKIP_ADD_STR":\n\t"

		"pxor %%mm1,%%mm1\n\t"
                "movl $16, %%ecx\n\t"

                "frup_mmx"FILTMMX_SKIP_ADD_STR":\n\t"

		/*
		PMADDWD mm1,mm2/m64           ; 0F F5 /r             [PENT,MMX] 

		dst[0-31]   := (dst[0-15] * src[0-15]) 
                               + (dst[16-31] * src[16-31]); 
		dst[32-63]  := (dst[32-47] * src[32-47]) 
	                               + (dst[48-63] * src[48-63]);

		*/
		"movq (%%edi), %%mm0\n\t"
		"pmaddwd (%%esi), %%mm0\n\t"

		"movq 8(%%edi), %%mm2\n\t"
		"pmaddwd 8(%%esi), %%mm2\n\t"

                "movq 16(%%edi), %%mm3\n\t"
		"pmaddwd 16(%%esi), %%mm3\n\t"

                "movq 24(%%edi), %%mm4\n\t"
                "pmaddwd 24(%%esi), %%mm4\n\t"

                "addl $32, %%edi\n\t"
		"addl $32, %%esi\n\t" 
		"paddd %%mm0, %%mm1\n\t"
		"paddd %%mm2, %%mm1\n\t"
		"decl %%ecx\n\t"
		"paddd %%mm3, %%mm1\n\t"
		"paddd %%mm4, %%mm1\n\t"
		"jnz frup_mmx"FILTMMX_SKIP_ADD_STR"\n\t"

                "movd %%mm1, (%%eax)\n\t"
                "subl $512, %%edi\n\t"
                "subl $512, %%esi\n\t"

                "addl $4, %%eax\n\t"
                "addl %%edx, %%edi\n\t"
                "decl %%ebp\n\t"
                "jnz bigloop_mmx"FILTMMX_SKIP_ADD_STR"\n\t"

		"popl %%ecx\n\t"
                "popl %%edi\n\t"
		"popl %%eax\n\t"
		"emms\n\t"

		"convmmx"FILTMMX_SKIP_ADD_STR":\n\t"

		"movl (%%eax), %%ebp\n\t"
		"sarl $15, %%ebp\n\t"
		#ifndef FILTMMX_SKIP_ADD
		"addl $32767, %%ebp\n\t"
		#endif
		"movl %%ebp, (%%eax)\n\t"

		"addl $4, %%eax\n\t"
		"decl %%ecx\n\t"
		"jnz convmmx"FILTMMX_SKIP_ADD_STR"\n\t"
		"popl %%ebp\n\t"
          :
          : "D" (in), "S" (ff->coeffs_i16), "a" (flout), "d" (2*ff->mrratio), "c" (bigcount)
          );
