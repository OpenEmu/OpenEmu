/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - demo.c                                                  *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Marshallh                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Use F3DEX microcode. */
#define  F3DEX_GBI

/* Include N64 defines. */
#include <ultra64.h>

#include "include/config.h"
#include "include/helpfunc.h"
#include "include/types.h"

#include "graphics.h"
#include "sprite.h"
#include "hardware.h"
#include "control.h"

#define PI 3.141592654
#define DEG90 PI/2
#define DEG180 PI

/* Macros */
#define vecCopy(A,B) B[0]=A[0]; B[1]=A[1]; B[2]=A[2];
#define vecScale(A,B,C) C[0]=A[0]*(B); C[1]=A[1]*(B); C[2]=A[2]*(B);
#define vecAdd(A,B,C) C[0]=A[0]+B[0]; C[1]=A[1]+B[1]; C[2]=A[2]+B[2];
#define vecSet(V,X,Y,Z) V[0] = X; V[1] = Y; V[2] = Z;

#define DEG2RAD(f) (f) / 180 * PI
#define RAD2DEG(f) (f) * 180 / PI

void renderObj(Gfx* displayList, vec3_t* pos, vec3_t* rot, float size, Gfx** listEnd);

/* Declare objects. */
simpleObj camera;
vec3_t camera_dest;
vec3_t camera_pdest;
float camera_lag = 1.0;
float move_lag = 1.0;

simpleObj logo;
simpleObj box;

Dynamic* generate; /* Dynamic info we're generating now. */
int entity = 0; 

/* Graphics variables. */
int firstframe = 1; /* First frame. */
Gfx* gdl_start;     /* Start of this frame's dynamic display list. */
Gfx* gdl_end;       /* Current position in dynamic display list. */
OSTask* gentask;    /* Task we're generating. */
u16 pnorm;          /* Perpsective normal. */

/* Controller variables. */
int analogX = 0;
int analogY = 0;
int button[4];
OSContPad** pad;
int lockCamera = 0;

char tempStr[256];

float f;
int i;
int j;
int k;
int l;
int dist;
int x;
int y;
int lastx = 0;
int lasty = 0;

/* Effect variables. */
int fade_trans = 0;
float loopStart = 0;
float loopEnd = 0;
float loopTime = 0;
float speed = 1;
float sineCount = 0;
float angle1;
float angle2;

/* Custom variables. */
float customVar1 = 0;
float interp;
float temp;
float rotate = 0;
float fov = 50;
float count = 0;
short stage = 0;
short nextStage = 0;
float logoscale = 0.5;
int logotrans = 1;
int logoenv = 190;

LookAt logola;
Mtx logomat;

short buttonPress[255];

void boot(void* arg)
{
    void MainProc(void*);
    register int i;

    /* Init OS. */
    osInitialize();

    /* Screen off. */
    osViBlack(TRUE);

    /* Create main thread. */
    osCreateThread(&mainThread, MAIN_ID, MainProc, arg, (void*)(mainStack+STACKSIZE/8), (OSPri) MAIN_PRIORITY);

    /* Start main thread. */
    osStartThread(&mainThread);
}

void InitGlobalVars(void)
{
    /* Initialize RSP Addresses. */
    rsp_static_len  = _rsp_staticSegmentRomEnd - _rsp_staticSegmentRomStart;
    rsp_static_addr = (char*) _codeSegmentEnd;
    rsp_static_end  = (char*) _codeSegmentEnd + rsp_static_len;

    fifosize = (u8*)fifobuffer + (sizeof(u64) * FIFOSIZE) + 1;
}

void MainProc(void *arg)
{
    void GameProc(void*);

    InitGlobalVars();

    /* Create game thread. */
    osCreateThread(&gameThread, GAME_ID, GameProc, arg, (void *)(gameStack+STACKSIZE/8), GAME_PRIORITY);

    /* Create video manager. */
    osCreateViManager(OS_PRIORITY_VIMGR);

    /* Set video mode. */
    osViSetMode(&osViModeTable[SCREEN_MODE]);
    osViSetSpecialFeatures(OS_VI_DITHER_FILTER_OFF | OS_VI_GAMMA_OFF);

    /* Create PI manager. */
    osCreatePiManager((OSPri) OS_PRIORITY_PIMGR, &PiMessageQ, PiMessages,           PI_MSGQUEUE_SIZE);

    /* Start game thread. */
    osStartThread(&gameThread);

    /* Relinquish CPU. */
    osSetThreadPri(NULL, 0);

    for(;;);
}

void GameProc(void *arg)
{
    void GameLoop();
    OSIoMesg dmaMb;    
    Gfx* l;

    /* Create DMA message queue. */
    osCreateMesgQueue(&DMAMessageQ, DMAMessages, DMA_MSGQUEUE_SIZE);

    /* Create RSP message queue. */
    osCreateMesgQueue(&RSPDoneMessageQ, RSPDoneMessages, RSPDONE_MSGQUEUE_SIZE);
    osSetEventMesg(OS_EVENT_SP, &RSPDoneMessageQ, dummyMsg);

    /* Create RDP message queue. */
    osCreateMesgQueue(&RDPDoneMessageQ, RDPDoneMessages, RDPDONE_MSGQUEUE_SIZE);
    osSetEventMesg(OS_EVENT_DP, &RDPDoneMessageQ, dummyMsg);

    /* Create VRT message queue. */
    osCreateMesgQueue(&RetraceMessageQ, RetraceMessages, RETRACE_MSGQUEUE_SIZE);
    osViSetEvent(&RetraceMessageQ, dummyMsg, 1);

    /* SI (controllers) */
    osCreateMesgQueue(&SiMessageQ, &SiMessages, 1);
    osSetEventMesg(OS_EVENT_SI, &SiMessageQ, (OSMesg)1);

    initControllers();
    initWorld();

    GameLoop();
}

void initWorld()
{
    vecSet(logo.pos, 0, 0, 0);
    vecSet(box.pos, 0, 0, 0);

    camera.pos[0] = 0;
    camera.pos[1] = 20;
    camera.pos[2] = -300;

    camera_dest[0] = 0;
    camera_dest[1] = 0; 
    camera_dest[2] = 0;
}

void loop1()
{
    loopStart = (int)osGetTime() / 2100;
    /* OS_CYCLES_TO_USEC(osGetTime()); */

    /* Run task we built during the previous frame. */
    if(!firstframe)
        osSpTaskStart(gentask);

     /* Point to new dynamic buffer. */
     generate = &(dynamic[CFB_NextCount]);

     /* Reset entity counter. (matrix and rotation object counter) */
     entity = 0;

     /* Set up pointers to GDL. */
     gdl_start = generate->glist;
     gdl_end = gdl_start;

      /* Init the GDL. */
      N64initDL (&gdl_end, gdl_start, rsp_static_addr, generate, Next_CFB, YES, YES);

      checkCont();
      checkCamera();
}

void checkCont()
{
    /* Read from the joypad controllers. */
    readController();

    button[0]=controllerData[0].button;
    analogX=(int)(controllerData[0].stick_x);
    analogY=(int)(controllerData[0].stick_y);

    /* Deadzone */
    if(fabs(analogX) < 10) {analogX = 0;}
    if(fabs(analogY) < 10) {analogY = 0;}

    if(analogX > 45) analogX = 45;
    if(analogX < -45) analogX = -45;
    if(analogY > 45) analogY = 45;
    if(analogY < -45) analogY = -45;
}

void checkCamera()
{
    Mtx ma;

    camera.mov[0] = camera.pos[0];
    camera.mov[1] = camera.pos[1];
    camera.mov[2] = camera.pos[2];

    camera_pdest[0] = camera_dest[0];
    camera_pdest[1] = camera_dest[1];
    camera_pdest[2] = camera_dest[2];

    /* Set up projection matrix. */
    guPerspective(&(generate->proj), &pnorm, fov, 1.3333333, 50.0, 5000.0, 1.0);
    gSPPerspNormalize(gdl_end++, pnorm);

    guLookAt(&(generate->view), camera.mov[0], camera.mov[1], camera.mov[2], camera_pdest[0], camera_pdest[1], camera_pdest[2], 0.0, 1.0, 0.0);

    guLookAtReflect(&logomat, &logola, camera.mov[0], camera.mov[1], camera.mov[2], camera_pdest[0], camera_pdest[1], camera_pdest[2], 0.0, 1.0, 0.0);

    gSPMatrix(gdl_end++, OS_K0_TO_PHYSICAL(&(generate->proj)), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl_end++, OS_K0_TO_PHYSICAL(&(generate->view)), G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
}


void drawSprites()
{
    gDPPipeSync(gdl_end++);

    SpriteInit(&gdl_end);


    /* Fade to black/ white. */
    if(fade_trans != 0)
        {
        if(fade_trans > 0)
            drawSprite(&gdl_end, &fade_white, 0, 0, 32, 32, 10, 7.5, fade_trans);
        else
            drawSprite(&gdl_end, &fade_black, 0, 0, 32, 32, 10, 7.5, -fade_trans);
        }

    SpriteFinish(&gdl_end);
}

void loop2()
{
    drawSprites();

    _gdlend:

    /* End the GDL. */
    N64terminateDL(&gdl_end);

    /* Init task. */
    gentask = &(task[CFB_NextCount]);
    N64initTask(gentask, gdl_end, gdl_start);

    /* Flush cache. */
    osWritebackDCacheAll();

    /* Wait for previous task to finish. */
    N64waitRSP(firstframe);
    N64waitRDP(firstframe);

    _alldone:

    /* Swap buffers. */
    N64swapCFB(firstframe);

    loopEnd = (int)osGetTime() / 2100 ;
    /* OS_CYCLES_TO_USEC(osGetTime()); */

    loopTime = loopEnd - loopStart;

    speed = (float)loopTime / 100.0;

    /* Wait for vertical retrace. */
    N64waitVRT();

    firstframe = 0;
}

void GameLoop()
{
    float a_mag;

    while (1)
        {
        loop1();

        count++;

        if(stage == 3)
            {
            logoenv = 190;
            fov = 90;
            vecSet(camera.pos, 0, -200, -170);
            vecSet(camera_dest, 0, 0, 0);

            if(count > 1) {logotrans = 255;}

            camera.pos[0] = -(count*0.5-160);
            camera.pos[1] = (-count*0.1+30);
            camera.pos[2] = -100;

            camera_dest[0] = -(count*0.5-200)+100;
            camera_dest[1] = 0;
            camera_dest[2] = count/3;

            if(count > 568)
                logotrans = (int)(32-(count-568))*8;

            if(count >= 600 || nextStage == 1)
                {
                count = 0;
                stage = 0;
                }
            }

        if(stage == 2)
            {
            logoenv = 0;
            fov = 30;
            vecSet(camera.pos, 0, -200, -170);
            vecSet(camera_dest, 0, 0, 0);

            if(count > 1) {logotrans = 255;}

            if(count > 550)
                fov = (count-550)*0.1+30;

            camera.pos[0] = -(count*0.5-200);
            camera_dest[0] = (count*0.5-200);

            if(count > 568)
                logotrans = (int)(32-(count-568))*8;

            if(count >= 600 || nextStage == 1)
                {
                count = 0;
                stage = 3;
                }
            }

        if(stage == 1)
            {
            logoenv = 190;
            vecSet(camera.pos, 0, 20, -300);
            vecSet(camera_dest, 0, 0, 0);

            if(count > 1) {logotrans = 255;}
            if(count < 40)
                {
                fov = count/3+10;
                logotrans = count*6.375;
                }

            if(count > 300 && count < 350)
                fov += 0.2;

            if(count > 426)
                logotrans = (int)(64-(count-426))*4;

            camera_dest[0] = (count/1.2-220);

            if(count >= 490 || nextStage == 1)
                {
                count = 0;
                stage = 2;
                logotrans = 1;
                }
            }

        if(stage == 0)
            {
            logoenv = 0;
            logo.rot[1] = 0;
            vecSet(camera.pos, 0, 20, -300);
            vecSet(camera_dest, 0, 0, 0);

            camera.pos[0] = (count-300)/4;
            camera.pos[1] = (count-300)/8;

            if(count > 1) {logotrans = 255;}

            if(count <= 40) 
                {
                logoscale = (count/1600)+0.15;
                logotrans = count*6.375;
                }

            if(count > 560)
                logotrans = (int)(64-(count-560))*4;

            if(count >= 624 || nextStage == 1)
                {
                count = 0;
                stage = 1;
                logotrans = 1;
                }
            }

        nextStage = 0;

        guPosition( &(logomat), 90+logo.rot[0], logo.rot[1], logo.rot[2]-180, logoscale, logo.pos[0], logo.pos[1], logo.pos[2]);

        /* Render Objects. */
        renderObj(Wtx_box_box_dl, box.pos, box.rot, 0.5, &gdl_end);
        gDPPipeSync(gdl_end++);
        gDPSetRenderMode(gdl_end++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);

        if(logotrans == 255)
            renderObj(Wtx_logo_logo_dl, logo.pos, logo.rot, logoscale, &gdl_end);

        /* Chnage render mode to transparent solids. */
        gDPPipeSync(gdl_end++); /* Always call PipeSync before changing rendermodes. */

        gDPSetCombineMode(gdl_end++,G_CC_MODULATERGB_PRIM, G_CC_MODULATERGB_PRIM);
        gDPSetRenderMode(gdl_end++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
        gDPSetBlendColor(gdl_end++,0xff, 0xff, 0xff, 0x01);

        gDPSetPrimColor(gdl_end++,0, 0, 255, 255, 255, logotrans);

        if(logotrans < 255)
            renderObj(Wtx_logo_logo_dl, logo.pos, logo.rot, logoscale, &gdl_end);

        /* Environmental Mapping. */
        gDPPipeSync(gdl_end++);
        gDPSetRenderMode( gdl_end++, G_RM_ZB_XLU_SURF,G_RM_ZB_XLU_SURF2);
        gDPSetCombineMode(gdl_end++, G_CC_MODULATERGB_PRIM, G_CC_MODULATERGB_PRIM);
        gSPSetGeometryMode(gdl_end++, G_LIGHTING | G_TEXTURE_GEN);
        gDPSetPrimColor( gdl_end++, 0, 0, 255, 255, 255, (float)logoenv*((float)logotrans/255.0));
        gSPTexture(gdl_end++, 31<<6, 31<<6, 0, G_TX_RENDERTILE, G_ON);

        /* Position and render environment map model. */
        gSPMatrix(gdl_end++, &(logomat), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
        gSPDisplayList(gdl_end++, Wtx_logo_env_logo_env_dl);

        /* Back to normal. */
        gSPClearGeometryMode(gdl_end++, G_LIGHTING | G_TEXTURE_GEN);

        loop2();
        }
}

void renderObj(Gfx* displayList, vec3_t* pos, vec3_t* rot, float size, Gfx** DL_Pos)
{
    /* Grab the DL. */
    Gfx* glistp;
    glistp = *DL_Pos;

    /* Translate and rotate the object. */
    guPosition(&(generate->xform[entity]), 90+(*rot)[0], (*rot)[1], (*rot)[2]-90+270, size, (*pos)[0], (*pos)[1], (*pos)[2]);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(generate->xform[entity])), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

    gSPDisplayList(glistp++, displayList);

    /* Give the DL back. */
    *DL_Pos = glistp;

    entity++;
}

