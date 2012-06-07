/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis 'TREMOR' CODEC SOURCE CODE.   *
 *                                                                  *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis 'TREMOR' SOURCE CODE IS (C) COPYRIGHT 1994-2009    *
 * BY THE Xiph.Org FOUNDATION http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: illustrate seeking, and test it too
 last mod: $Id: iseeking_example.c 16037 2009-05-26 21:10:58Z xiphmont $

 ********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "ivorbiscodec.h"
#include "ivorbisfile.h"

#ifdef _WIN32 /* We need the following two to set stdin/stdout to binary */
# include <io.h>
# include <fcntl.h>
#endif

void _verify(OggVorbis_File *ov,
             ogg_int64_t val,
             ogg_int64_t pcmval,
             ogg_int64_t timeval,
             ogg_int64_t pcmlength,
             char *bigassbuffer){
  int j;
  long bread;
  char buffer[4096];
  int dummy;
  ogg_int64_t pos;

  /* verify the raw position, the pcm position and position decode */
  if(val!=-1 && ov_raw_tell(ov)<val){
    fprintf(stderr,"raw position out of tolerance: requested %ld, got %ld\n",
           (long)val,(long)ov_raw_tell(ov));
    exit(1);
  }
  if(pcmval!=-1 && ov_pcm_tell(ov)>pcmval){
    fprintf(stderr,"pcm position out of tolerance: requested %ld, got %ld\n",
           (long)pcmval,(long)ov_pcm_tell(ov));
    exit(1);
  }
  if(timeval!=-1 && ov_time_tell(ov)>timeval){
    fprintf(stderr,"time position out of tolerance: requested %ld, got %ld\n",
            (long)timeval,(long)ov_time_tell(ov));
    exit(1);
  }
  pos=ov_pcm_tell(ov);
  if(pos<0 || pos>pcmlength){
    fprintf(stderr,"pcm position out of bounds: got %ld\n",(long)pos);
    exit(1);
  }
  bread=ov_read(ov,buffer,4096,&dummy);
  if(bigassbuffer){
    for(j=0;j<bread;j++){
      if(buffer[j]!=bigassbuffer[j+pos*4]){
	fprintf(stderr,"data position after seek doesn't match pcm position\n");
	
	{
	  FILE *f=fopen("a.m","w");
	  for(j=0;j<bread;j++)fprintf(f,"%d\n",(int)buffer[j]);
	  fclose(f);
	  f=fopen("b.m","w");
	  for(j=0;j<bread;j++)fprintf(f,"%d\n",(int)bigassbuffer[j+pos*2]);
	  fclose(f);
	}
	
	exit(1);
      }
    }
  }
}

int main(){
  OggVorbis_File ov;
  int i,ret;
  ogg_int64_t pcmlength;
  ogg_int64_t timelength;
  char *bigassbuffer;
  int dummy;

#ifdef _WIN32 /* We need to set stdin/stdout to binary mode. Damn windows. */
  _setmode( _fileno( stdin ), _O_BINARY );
#endif


  /* open the file/pipe on stdin */
  if(ov_open(stdin, &ov, NULL, 0) < 0) {
    fprintf(stderr,"Could not open input as an OggVorbis file.\n\n");
    exit(1);
  }

  if(ov_seekable(&ov)){

    /* to simplify our own lives, we want to assume the whole file is
       stereo.  Verify this to avoid potentially mystifying users
       (pissing them off is OK, just don't confuse them) */
    for(i=0;i<ov.links;i++){
      vorbis_info *vi=ov_info(&ov,i);
      if(vi->channels!=2){
        fprintf(stderr,"Sorry; right now seeking_test can only use Vorbis files\n"
               "that are entirely stereo.\n\n");
        exit(1);
      }
    }
    
    /* because we want to do sample-level verification that the seek
       does what it claimed, decode the entire file into memory */
    pcmlength=ov_pcm_total(&ov,-1);
    timelength=ov_time_total(&ov,-1);
    bigassbuffer=malloc(pcmlength*4); /* w00t */
    if(bigassbuffer){
      i=0;
      while(i<pcmlength*4){
	int ret=ov_read(&ov,bigassbuffer+i,pcmlength*4-i,&dummy);
	if(ret<0)continue;
	if(ret){
	  i+=ret;
	}else{
	  pcmlength=i/4;
	}
	fprintf(stderr,"\rloading.... [%ld left]              ",
		(long)(pcmlength*4-i));
      }
    }else{
      fprintf(stderr,"\rfile too large to load into memory for read tests;\n\tonly verifying seek positioning...\n");
    }
    
    {
      ogg_int64_t length=ov.end;
      fprintf(stderr,"\rtesting raw seeking to random places in %ld bytes....\n",
             (long)length);
    
      for(i=0;i<1000;i++){
        ogg_int64_t val=rand()*length/RAND_MAX;
        fprintf(stderr,"\r\t%d [raw position %ld]...     ",i,(long)val);
        ret=ov_raw_seek(&ov,val);
        if(ret<0){
          fprintf(stderr,"seek failed: %d\n",ret);
          exit(1);
        }

        _verify(&ov,val,-1,-1.,pcmlength,bigassbuffer);

      }
    }

    fprintf(stderr,"\r");
    {
      fprintf(stderr,"testing pcm page seeking to random places in %ld samples....\n",
             (long)pcmlength);
    
      for(i=0;i<1000;i++){
        ogg_int64_t val=(double)rand()*pcmlength/RAND_MAX;
        fprintf(stderr,"\r\t%d [pcm position %ld]...     ",i,(long)val);
        ret=ov_pcm_seek_page(&ov,val);
        if(ret<0){
          fprintf(stderr,"seek failed: %d\n",ret);
          exit(1);
        }

        _verify(&ov,-1,val,-1.,pcmlength,bigassbuffer);

      }
    }
    
    fprintf(stderr,"\r");
    {
      fprintf(stderr,"testing pcm exact seeking to random places in %ld samples....\n",
             (long)pcmlength);
    
      for(i=0;i<1000;i++){
        ogg_int64_t val=(double)rand()*pcmlength/RAND_MAX;
        fprintf(stderr,"\r\t%d [pcm position %ld]...     ",i,(long)val);
        ret=ov_pcm_seek(&ov,val);
        if(ret<0){
          fprintf(stderr,"seek failed: %d\n",ret);
          exit(1);
        }
        if(ov_pcm_tell(&ov)!=val){
          fprintf(stderr,"Declared position didn't perfectly match request: %ld != %ld\n",
                 (long)val,(long)ov_pcm_tell(&ov));
          exit(1);
        }

        _verify(&ov,-1,val,-1.,pcmlength,bigassbuffer);

      }
    }

    fprintf(stderr,"\r");
    {
      fprintf(stderr,"testing time page seeking to random places in %ld milliseconds....\n",
              (long)timelength);
    
      for(i=0;i<1000;i++){
        ogg_int64_t val=(double)rand()*timelength/RAND_MAX;
        fprintf(stderr,"\r\t%d [time position %ld]...     ",i,(long)val);
        ret=ov_time_seek_page(&ov,val);
        if(ret<0){
          fprintf(stderr,"seek failed: %d\n",ret);
          exit(1);
        }

        _verify(&ov,-1,-1,val,pcmlength,bigassbuffer);

      }
    }

    fprintf(stderr,"\r");
    {
      fprintf(stderr,"testing time exact seeking to random places in %ld milliseconds....\n",
              (long)timelength);
    
      for(i=0;i<1000;i++){
        ogg_int64_t val=(double)rand()*timelength/RAND_MAX;
        fprintf(stderr,"\r\t%d [time position %ld]...     ",i,(long)val);
        ret=ov_time_seek(&ov,val);
        if(ret<0){
          fprintf(stderr,"seek failed: %d\n",ret);
          exit(1);
        }
        if(ov_time_tell(&ov)<val-1 || ov_time_tell(&ov)>val+1){
          fprintf(stderr,"Declared position didn't perfectly match request: %ld != %ld\n",
                  (long)val,(long)ov_time_tell(&ov));
          exit(1);
        }

        _verify(&ov,-1,-1,val,pcmlength,bigassbuffer);

      }
    }
    
    fprintf(stderr,"\r                                           \nOK.\n\n");


  }else{
    fprintf(stderr,"Standard input was not seekable.\n");
  }

  ov_clear(&ov);
  return 0;
}













