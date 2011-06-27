#if _ARCH_PPC64 || __PPC64__ || __ppc64__ || __powerpc64__
	#define LIBCO_PPC32 0
#else
	#define LIBCO_PPC32 1
#endif

#ifdef LIBCO_PPC_NOFP
	#define LIBCO_PPC_FP 0
#else
	#define LIBCO_PPC_FP 1
#endif

#ifdef __ALTIVEC__
	#define LIBCO_PPC_ALTIVEC 1
#else
	#define LIBCO_PPC_ALTIVEC 0
#endif

static void print_libco_opts( void )
{
	#if LIBCO_PPC32
		printf( "PPC32 " );
	#else
		printf( "PPC64 " );
	#endif
	
	#if LIBCO_PPC_FP
		printf( "FP " );
	#endif
	
	#if LIBCO_PPC_ALTIVEC
		printf( "ALTIVEC " );
	#endif
	
	printf( "\n" );
}
