/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis 'TREMOR' CODEC SOURCE CODE.   *
 *                                                                  *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis 'TREMOR' SOURCE CODE IS (C) COPYRIGHT 1994-2003    *
 * BY THE Xiph.Org FOUNDATION http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: single-block PCM synthesis
 last mod: $Id: synthesis.c,v 1.4 2003/03/29 03:07:21 xiphmont Exp $

 ********************************************************************/

#include <stdio.h>
#include "ogg.h"
#include "ivorbiscodec.h"
#include "codec_internal.h"
#include "registry.h"
#include "misc.h"
#include "block.h"

static int _vorbis_synthesis1(vorbis_block *vb,ogg_packet *op,int decodep){
  vorbis_dsp_state     *vd= vb ? vb->vd : 0;
  private_state        *b= vd ? (private_state *)vd->backend_state: 0;
  vorbis_info          *vi= vd ? vd->vi : 0;
  codec_setup_info     *ci= vi ? (codec_setup_info *)vi->codec_setup : 0;
  oggpack_buffer       *opb=vb ? &vb->opb : 0;
  int                   type,mode,i;
 
  if (!vd || !b || !vi || !ci || !opb) {
    return OV_EBADPACKET;
  }

  /* first things first.  Make sure decode is ready */
  _vorbis_block_ripcord(vb);
  oggpack_readinit(opb,op->packet,op->bytes);

  /* Check the packet type */
  if(oggpack_read(opb,1)!=0){
    /* Oops.  This is not an audio data packet */
    return(OV_ENOTAUDIO);
  }

  /* read our mode and pre/post windowsize */
  mode=oggpack_read(opb,b->modebits);
  if(mode==-1)return(OV_EBADPACKET);
  
  vb->mode=mode;
  if(!ci->mode_param[mode]){
    return(OV_EBADPACKET);
  }

  vb->W=ci->mode_param[mode]->blockflag;
  if(vb->W){
    vb->lW=oggpack_read(opb,1);
    vb->nW=oggpack_read(opb,1);
    if(vb->nW==-1)   return(OV_EBADPACKET);
  }else{
    vb->lW=0;
    vb->nW=0;
  }
  
  /* more setup */
  vb->granulepos=op->granulepos;
  vb->sequence=op->packetno-3; /* first block is third packet */
  vb->eofflag=op->e_o_s;

  if(decodep){
    /* alloc pcm passback storage */
    vb->pcmend=ci->blocksizes[vb->W];
    vb->pcm=(ogg_int32_t **)_vorbis_block_alloc(vb,sizeof(*vb->pcm)*vi->channels);
    for(i=0;i<vi->channels;i++)
      vb->pcm[i]=(ogg_int32_t *)_vorbis_block_alloc(vb,vb->pcmend*sizeof(*vb->pcm[i]));
    
    /* unpack_header enforces range checking */
    type=ci->map_type[ci->mode_param[mode]->mapping];
    
    return(_mapping_P[type]->inverse(vb,b->mode[mode]));
  }else{
    /* no pcm */
    vb->pcmend=0;
    vb->pcm=NULL;
    
    return(0);
  }
}

int vorbis_synthesis(vorbis_block *vb,ogg_packet *op){
  return _vorbis_synthesis1(vb,op,1);
}

/* used to track pcm position without actually performing decode.
   Useful for sequential 'fast forward' */
int vorbis_synthesis_trackonly(vorbis_block *vb,ogg_packet *op){
  return _vorbis_synthesis1(vb,op,0);
}

long vorbis_packet_blocksize(vorbis_info *vi,ogg_packet *op){
  codec_setup_info     *ci=(codec_setup_info *)vi->codec_setup;
  oggpack_buffer       opb;
  int                  mode;
 
  oggpack_readinit(&opb,op->packet,op->bytes);

  /* Check the packet type */
  if(oggpack_read(&opb,1)!=0){
    /* Oops.  This is not an audio data packet */
    return(OV_ENOTAUDIO);
  }

  {
    int modebits=0;
    int v=ci->modes;
    while(v>1){
      modebits++;
      v>>=1;
    }

    /* read our mode and pre/post windowsize */
    mode=oggpack_read(&opb,modebits);
  }
  if(mode==-1)return(OV_EBADPACKET);
  return(ci->blocksizes[ci->mode_param[mode]->blockflag]);
}


