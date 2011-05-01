
	 uint32 vdc2_pixel, vdc1_pixel;

	 vdc2_pixel = vdc1_pixel = vce.color_table_cache[0];

	 if(pb & 1)
	  vdc1_pixel = lb0[x];

	 if(pb & 2)
          vdc2_pixel = lb1[x];

	 /* Dai MakaiMura uses setting 1, and expects VDC #2 sprites in front of VDC #1 background, but
	    behind VDC #1's sprites.
	  */
	 switch(pb >> 2)
	 {    
	  case 1:
                //if((vdc2_pixel & (amask << 2)) && !(vdc1_pixel & (amask << 2)))
                //        vdc1_pixel |= amask;
		vdc1_pixel |= (((vdc2_pixel ^ vdc1_pixel) & vdc2_pixel) >> 2) & amask;
                break;

	  case 2:
                //if((vdc1_pixel & (amask << 2)) && !(vdc2_pixel & (amask << 2)) && !(vdc2_pixel & amask))
                //        vdc1_pixel |= amask;
		puts("MOO");
		// TODO: Verify that this is correct logic.
		{
		 const uint32 intermediate = ((vdc1_pixel ^ vdc2_pixel) & vdc1_pixel) >> 2;
		 vdc1_pixel |= (intermediate ^ vdc2_pixel) & intermediate & amask;
		}
                break;
	 }
	 target[x] = (vdc1_pixel & amask) ? vdc2_pixel : vdc1_pixel;

