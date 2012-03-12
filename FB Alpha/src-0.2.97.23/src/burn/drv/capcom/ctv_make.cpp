#include <stdio.h>

// Create the ctv.h header file
// which includes all combinations of the cps tile drawing functions

int main()
{
  int nCuMask=0;
  int nCuBpp=0;
  int nCuSize=0;
  int nCuRows=0;
  int nCuCare=0;
  int nCuFlipX=0;

  for (nCuMask=0; nCuMask<=2; nCuMask++)
  {
    printf ("#define CU_MASK  (%d)\n\n",nCuMask);
    for (nCuBpp=2; nCuBpp<=4; nCuBpp++)
	{
      printf ("#define CU_BPP   (%d)\n\n",nCuBpp);
      for (nCuSize=8; nCuSize<=32; nCuSize<<=1)
	  {
        printf ("#define CU_SIZE  (%d)\n\n",nCuSize);
        for (nCuRows=0; nCuRows<2; nCuRows++)
		{
          printf ("#define CU_ROWS  (%d)\n\n",nCuRows);
          for (nCuCare=0; nCuCare<2; nCuCare++)
		  {
            printf ("#define CU_CARE  (%d)\n",nCuCare);
            for (nCuFlipX=0; nCuFlipX<2; nCuFlipX++)
			{
              printf ("#define CU_FLIPX (%d)\n",nCuFlipX);

			  if (((nCuRows && (nCuSize != 16)) || (nCuRows && nCuMask))) {
				  printf("// Invalid combination of capabilities.\n");
			  }	else {
				printf ("static INT32 ");
				printf ("CtvDo");
				printf ("%d",nCuBpp);
				printf ("%.2d",nCuSize);
					 if (nCuRows)  printf ("r"); else printf ("_");
					 if (nCuCare)  printf ("c"); else printf ("_");
					 if (nCuFlipX) printf ("f"); else printf ("_");
					 if (nCuMask==1)  printf ("m()\n#include \"ctv_do.h\"\n");
			    else if (nCuMask==2)  printf ("b()\n#include \"ctv_do.h\"\n");
			    else                  printf ("_()\n#include \"ctv_do.h\"\n");
			  }



              printf ("#undef  CU_FLIPX\n");
			}
            printf ("#undef  CU_CARE\n\n");
		  }
          printf ("#undef  CU_ROWS\n\n");
		}
        printf ("#undef  CU_SIZE\n\n");
	  }
      printf ("#undef  CU_BPP\n\n");
	}
	printf ("#undef  CU_MASK\n\n");
  }

  printf ("\n\n");

  printf ("// Filler function\n");
  printf ("static INT32 CtvDo_______() { return 0; }\n\n\n\n");

  for (nCuMask=0; nCuMask<=2; nCuMask++)
  {
    for (nCuBpp=2; nCuBpp<=4; nCuBpp++)
	{
      int i=0;

      if (nCuMask==1)
      {
	     printf ("// Lookup table for %d bpp with Sprite Masking\n",nCuBpp);
	     printf ("static CtvDoFn CtvDo%dm[0x20]={\n",nCuBpp);
      }
      else if (nCuMask==2)
      {
	     printf ("// Lookup table for %d bpp with BgHi\n",nCuBpp);
         printf ("static CtvDoFn CtvDo%db[0x20]={\n",nCuBpp);
      }
      else
      {
	     printf ("// Lookup table for %d bpp\n",nCuBpp);
         printf ("static CtvDoFn CtvDo%d[0x20]={\n",nCuBpp);
      }

      for (i=0;i<0x20;i++)
	  {
        int s;
        printf ("CtvDo");

        s=(i&24)+8;
        if (s!=8 && s!=16 && s!=32) { printf ("_______"); goto End; }
		if ((i&4) && (s!=16)) { printf ("_______"); goto End; }
		if ((i&4) && nCuMask) { printf ("_______"); goto End; }
        printf ("%d",nCuBpp);
        printf ("%.2d",s);
        if (i&4)     printf ("r"); else printf ("_");
        if (i&2)     printf ("c"); else printf ("_");
        if (i&1)     printf ("f"); else printf ("_");
		if (nCuMask==1) printf ("m");
   else if (nCuMask==2) printf ("b");
   else                 printf ("_");

      End:
        printf (",");
        if (((i+1)&3)==0) printf("\n");
	  }
      printf ("};\n");
	}
  }

  printf ("\n\n");
  printf ("// Current BPP:\n");
  printf ("CtvDoFn CtvDoX[0x20];\n");
  printf ("CtvDoFn CtvDoXM[0x20];\n");
  printf ("CtvDoFn CtvDoXB[0x20];\n");
  printf ("\n\n");

  return 0;
}
