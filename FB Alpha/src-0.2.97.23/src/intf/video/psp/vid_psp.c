#include "vid_psp.h"

#define SLICE_SIZE 32 // change this to experiment with different page-cache sizes

static unsigned int __attribute__((aligned(16))) list[262144];

unsigned short __attribute__((aligned(16))) g_BlitBuff[512*512*2];

unsigned short *g_pBlitBuff = NULL;

struct Vertex
{
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
};

void vidgu_init()
{

	g_pBlitBuff = (unsigned short *)(0x40000000 | (unsigned int)&g_BlitBuff);

	sceGuInit();

	// setup
	sceGuStart(GU_DIRECT,list);
	sceGuDrawBuffer(GU_PSM_5650,(void*)0,512);
	sceGuDispBuffer(480,272,(void*)0x88000,512);
	sceGuDepthBuffer((void*)0x110000,512);
	sceGuOffset(2048 - (480/2),2048 - (272/2));
	sceGuViewport(2048,2048,480,272);
	sceGuDepthRange(0xc350,0x2710);
	sceGuScissor(0,0,480,272);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFrontFace(GU_CW);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(1);

}

void vidgu_exit()
{
	sceGuTerm();
}



void vidgu_render_nostretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh)
{
	int start, end;

	sceGuStart(GU_DIRECT,list);
	sceGuTexMode(GU_PSM_5650,0,0,0); // 16-bit RGBA
	sceGuTexImage(0,512,512,512,g_pBlitBuff); // setup texture as a 256x256 texture
	//sceKernelDcacheWritebackAll();
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA); // don't get influenced by any vertex colors
	sceGuTexFilter(GU_NEAREST,GU_NEAREST); // point-filtered sampling
	for (start = sx, end = sx+sw; start < end; start += SLICE_SIZE, dx += SLICE_SIZE)
	{
		struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
		int width = (start + SLICE_SIZE) < end ? SLICE_SIZE : end-start;

		vertices[0].u = start; vertices[0].v = sy;
		vertices[0].color = 0;
		vertices[0].x = dx; vertices[0].y = dy; vertices[0].z = 0;

		vertices[1].u = start + width; vertices[1].v = sy + sh;
		vertices[1].color = 0;
		vertices[1].x = dx + width; vertices[1].y = dy + sh; vertices[1].z = 0;

		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5650|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
	}
	sceGuFinish();
	sceGuSync(0,0);
	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
	//sceGuSwapBuffers();
}


void vidgu_render(int sx, int sy, int sw,int sh,int dx, int dy, int dw,int dh)
{
	unsigned int j,cx,cy;
	struct Vertex* vertices;

	cx=(480-dw)/2;
	cy=(272-dh)/2;

	sceGuStart(GU_DIRECT,list);
	sceGuTexMode(GU_PSM_5650,0,0,0); // 16-bit RGBA
	sceGuTexImage(0,512,512,512,g_pBlitBuff); // setup texture as a 256x256 texture
	//sceKernelDcacheWritebackAll();
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA); // don't get influenced by any vertex colors
	sceGuTexFilter(GU_LINEAR,GU_LINEAR); // point-filtered sampling

	int		start, end;
	float	ustart = (float)sx;
	float	ustep = (float)sw / (float)(dw / SLICE_SIZE);

	// blit maximizing the use of the texture-cache

	for (start = sx, end = sx+dw; start < end; start += SLICE_SIZE, dx += SLICE_SIZE)
	{
		struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
		int width = (start + SLICE_SIZE) < end ? SLICE_SIZE : end-start;

		vertices[0].u = ustart; 
		vertices[0].v = (float)sy;
		vertices[0].color = 0;
		vertices[0].x = dx; 
		vertices[0].y = dy; 
		vertices[0].z = 0;

		vertices[1].u = ustart + ustep; 
		vertices[1].v = (float)(sy + sh);
		vertices[1].color = 0;
		vertices[1].x = dx + width; 
		vertices[1].y = dy + dh; 
		vertices[1].z = 0;

		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5650|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);

		ustart += ustep;
	}


	sceGuFinish();
	sceGuSync(0,0);
	sceDisplayWaitVblankStart();
	sceGuSwapBuffers(); 
}

