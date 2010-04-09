/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "mednafen.h"

#include <string.h>
#include <stdarg.h>
#include <zlib.h>

#include "endian.h"
#include "file.h"
#include "psf.h"
#include "general.h"

static long TimeToMS(const char *str)
{
             int x,c=0;
             int acc=0;
	     char s[100];

	     strncpy(s,str,100);
	     s[99]=0;

             for(x=strlen(s);x>=0;x--)
              if(s[x]=='.' || s[x]==',')
              {
               acc=atoi(s+x+1);
               s[x]=0;
              }
              else if(s[x]==':')
              {
               if(c==0) acc+=atoi(s+x+1)*10;
               else if(c==1) acc+=atoi(s+x+(x?1:0))*10*60;
               c++;
               s[x]=0;
              }
              else if(x==0)
	      {
               if(c==0) acc+=atoi(s+x)*10;
               else if(c==1) acc+=atoi(s+x)*10*60;
               else if(c==2) acc+=atoi(s+x)*10*60*60;
	      }
             acc*=100;  // To milliseconds.
	    return(acc);
}

static int GetKeyVal(char *buf, char **key, char **val)
{
 char *tmp;

 tmp=buf;

 /* First, convert any weirdo ASCII characters to spaces. */
 while(*tmp++) if(*tmp>0 && *tmp<0x20) *tmp=0x20;

 /* Strip off white space off end of string(which should be the "value"). */
 for(tmp=buf+strlen(buf)-1;tmp>=buf;tmp--)
 {
  if(*tmp != 0x20) break;
  *tmp=0;
 }

 /* Now, search for the first non-whitespace character. */
 while(*buf == 0x20) buf++; 

 tmp=buf;
 while((*buf != 0x20) && (*buf != '=')) 
 {
  if(!*buf) return(0);	/* Null character. */
  buf++;
 }

 /* Allocate memory, copy string, and terminate string. */
 if(!(*key=(char *)malloc(buf-tmp+1))) return(0);
 strncpy(*key,tmp,buf-tmp);
 (*key)[(buf-tmp)]=0;

 /* Search for "=" character. */
 while(*buf != '=')
 {
  if(!*buf) return(0);  /* Null character. */
  buf++;
 }

 buf++;	/* Skip over equals character. */

 /* Remove leading whitespace on value. */
 while(*buf == 0x20)
 {
  if(!*buf) return(0);  /* Null character. */
  buf++;
 }

 /* Allocate memory, and copy string over.  Trailing whitespace was eliminated
    earlier.
 */

 if(!(*val=(char *)malloc(strlen(buf)+1))) return(0);
 strcpy(*val,buf);

 //puts(*key);
 //puts(*val);

 return(1);
}

static void FreeTags(PSFTAG *tags)
{
 while(tags)
 {
  PSFTAG *tmp=tags->next;

  free(tags->key);
  free(tags->value);
  free(tags);

  tags=tmp;
 }
}

static void AddKV(PSFTAG **tag, char *key, char *val)
{
 PSFTAG *tmp;

 tmp = (PSFTAG *)malloc(sizeof(PSFTAG));
 memset(tmp,0,sizeof(PSFTAG));

 tmp->key=key;
 tmp->value=val;
 tmp->next=0;

 if(!*tag) *tag=tmp;
 else 
 {
  PSFTAG *rec;
  rec=*tag;
  while(rec->next) rec=rec->next;
  rec->next=tmp;
 }

}

typedef struct {
	int num;
	char *value;
} LIBNCACHE;

static int ccomp(const void *v1, const void *v2)
{
 const LIBNCACHE *a1,*a2;
 a1=(const LIBNCACHE *)v1; a2=(const LIBNCACHE *)v2;

 return(a1->num - a2->num);
}

static PSFINFO *LoadPSF(void (*datafunc)(void *, uint32), MDFNFILE *fp, int level, int type) // Type==1 for just info load.
{
        char *in,*out=0;
        uint32 reserved;
        uint32 complen;
        uint32 crc32; 
        uLongf outlen;
	PSFINFO *psfi;
	PSFINFO *tmpi;

	psfi=(PSFINFO *)malloc(sizeof(PSFINFO));
	memset(psfi,0,sizeof(PSFINFO));
        psfi->stop=~0;
        psfi->fade=0; 

        MDFN_fread(&reserved,1,4,fp);
        MDFN_fread(&complen,1,4,fp);

	#ifdef MSB_FIRST
	FlipByteOrder((uint8*)&complen, sizeof(complen));
	#endif

        MDFN_fread(&crc32,1,4,fp);
        #ifdef MSB_FIRST
        FlipByteOrder((uint8*)&crc32, sizeof(crc32));
        #endif

        MDFN_fseek(fp,reserved,SEEK_CUR);

        if(type)
	 MDFN_fseek(fp,complen,SEEK_CUR);
        else
        {
         in=(char *)malloc(complen);
	 out = (char *)malloc(1024 * 1024 * 32 + 12);
         //out=(char *)malloc(1024*1024*2+0x800);
         MDFN_fread(in,1,complen,fp);
         outlen=1024 * 1024 * 32 + 12; //1024*1024*2;
	 int t = uncompress((Bytef *)out,&outlen,(const Bytef*)in,complen);
         free(in);

	 if(t) exit(1); //return(0);

 	 if(level)
 	 {
	  datafunc(out, outlen);
          free(out);
	 }
        }

        {
         uint8 tagdata[5];
         if(MDFN_fread(tagdata,1,5,fp)==5)
         {
          if(!memcmp(tagdata,"[TAG]",5))
          {
           char linebuf[1024];

           while(MDFN_fgets(linebuf,1024,fp)>0)
           {
            int x;
	    char *key=0,*value=0;

	    if(!GetKeyVal(linebuf,&key,&value))
	    { 
	     if(key) free(key);
	     if(value) free(value);
	     continue;
            }

	    AddKV(&psfi->tags,key,value);

	    if(!level)
	    {
       	     static const char *yoinks[8]={"title","artist","game","year","genre",
                                  "copyright","psfby","comment"};
	     char **yoinks2[8]={&psfi->title,&psfi->artist,&psfi->game,&psfi->year,&psfi->genre,
                                    &psfi->copyright,&psfi->psfby,&psfi->comment};
	     for(x=0;x<8;x++)
	      if(!strcasecmp(key,yoinks[x]))
		*yoinks2[x]=value;
	     if(!strcasecmp(key,"length"))
	      psfi->stop=TimeToMS(value);
	     else if(!strcasecmp(key,"fade"))
	      psfi->fade=TimeToMS(value);
	    }

	    if(!strcasecmp(key,"_lib") && !type)
	    {
	     MDFNFILE *tmpfp;
	     /* Load file name "value" from the directory specified in
		the full path(directory + file name) "path"
	     */
	     tmpfp = MDFN_fopen(MDFN_MakeFName(MDFNMKF_AUX, 0, value).c_str(), NULL, "rb", NULL);
	     if(!tmpfp)
	     {
	      FreeTags(psfi->tags);
	      free(psfi);
	      return(0);
	     }
	     MDFN_fseek(tmpfp,4,SEEK_SET);
	     if(!(tmpi=LoadPSF(datafunc, tmpfp,level+1,0))) 
	     {
	      free(key);
	      free(value);
 	      if(!level) free(out);
	      MDFN_fclose(tmpfp);
	      FreeTags(psfi->tags);
	      free(psfi);
	      return(0);
	     }
	     MDFN_fclose(tmpfp);
	     FreeTags(tmpi->tags);
	     free(tmpi);
	    }
           }
          }
         }
        }  

	/* Now, if we're at level 0(main PSF), load the main executable, and any libN stuff */
        if(!level && !type)
        {
	 datafunc(out, outlen);
	 free(out);
        }

	if(!type)	/* Load libN */
	{
	 LIBNCACHE *cache;
	 PSFTAG *tag;
	 unsigned int libncount=0;
	 unsigned int cur=0;

	 tag=psfi->tags;
	 while(tag)
	 {
	  if(!strncasecmp(tag->key,"_lib",4) && tag->key[4])
	   libncount++;
	  tag=tag->next;
	 }

	 if(libncount)
	 {
	  cache = (LIBNCACHE *)malloc(sizeof(LIBNCACHE)*libncount);

	  tag=psfi->tags;
	  while(tag)
	  {
	   if(!strncasecmp(tag->key,"_lib",4) && tag->key[4])
	   {
	    cache[cur].num=atoi(&tag->key[4]);
	    cache[cur].value=tag->value;
	    cur++;
	   }
	   tag=tag->next;
	  }
	  qsort(cache, libncount, sizeof(LIBNCACHE), ccomp);
	  for(cur=0;cur<libncount;cur++)
	  {
 	   if(cache[cur].num < 2) continue;

	   MDFNFILE *tmpfp;
           tmpfp = MDFN_fopen(MDFN_MakeFName(MDFNMKF_AUX, 0, cache[cur].value).c_str(), NULL, "rb", NULL);

           if(!(tmpi=LoadPSF(datafunc, tmpfp,level+1,0)))
           {
            //free(key);
            //free(value);
            //free(tmpfn);
            //fclose(fp);
            //return(0);
           }
	   FreeTags(tmpi->tags);
	   free(tmpi);
	  }
	  free(cache);

	 }	// if(libncount)

	}	// if(!type)

	return(psfi);
}

void MDFNPSF_Free(PSFINFO *info)
{
 assert(info);

 FreeTags(info->tags);
 free(info);
}

int MDFNPSF_Load(int desired_version, MDFNFILE *fp, PSFINFO **info, void (*datafunc)(void *data, uint32 len))
{
	PSFINFO *pi;
	int version;

	assert(fp && info && datafunc);

        if(memcmp(fp->data,"PSF",3)) return(-1);
	version = fp->data[3];
	if(desired_version != version) return(-1);

	MDFN_fseek(fp, 4, SEEK_SET);

	if(!(pi=LoadPSF(datafunc, fp,0,0)))
	 return(0);

	if(pi->stop==~0U) pi->fade=0; // Infinity+anything is still infinity...or is it?
	pi->length=pi->stop+pi->fade;
	*info = pi;

	return(1);
}
