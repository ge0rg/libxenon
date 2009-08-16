#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <xenos/xe.h>
#include <input/input.h>
#include "engine.h"
#include "xee.h"

uint64_t ld(volatile void *addr)
{
	uint64_t l;
	asm volatile ("ld %0, 0(%1)" : "=r" (l) : "r" (addr));
	return l;
}

extern void do_edram_foo(struct XenosDevice *xe, int complete);

int main(void)
{
	printf("Hello, world!\n");

	kmem_init();
	usb_init();
	printf(" * Waiting for USB...\n");
	while (1)
	{
		usb_do_poll();
		mdelay(50);
		struct controller_data_s c;
		if (get_controller_data(&c, 0))
		{
			printf("x= %d\n", c.s1_x);
		}
	}
	
	
	volatile uint64_t *cpuspace = (void*)0x20000000;
	
	struct XenosDevice _xe, *xe;

	xe = &_xe;
		/* initialize the GPU */
	Xe_Init(xe);

		/* create a render target (the framebuffer) */
	struct XenosSurface *fb = Xe_GetFramebufferSurface(xe);
	Xe_SetRenderTarget(xe, fb);

		/* let's define a vertex buffer format */
	static const struct XenosVBFFormat vbf =
	{
		5, {
		  {XE_USAGE_POSITION, 0, XE_TYPE_FLOAT3},
		  {XE_USAGE_NORMAL,   0, XE_TYPE_FLOAT3},
		  {XE_USAGE_TANGENT,  0, XE_TYPE_FLOAT3},
		  {XE_USAGE_COLOR,    0, XE_TYPE_UBYTE4},
		  {XE_USAGE_TEXCOORD, 0, XE_TYPE_FLOAT2},
		}
	};

		/* a cube */
	float cube[] = {
		//       POSITION           |            NORMAL           |        TANGENT              |   COL   |    U        V    |
		-0.5000 , -0.5000 , -0.5000 , +0.0000 , +0.0000 , -1.0000 , +1.0000 , +0.0000 , +0.0000 , +0.0000 , +1.0000 , +1.0000, 
		-0.5000 , +0.5000 , -0.5000 , +0.0000 , +0.0000 , -1.0000 , +1.0000 , +0.0000 , +0.0000 , +0.0000 , +1.0000 , +0.0000, 
		+0.5000 , +0.5000 , -0.5000 , +0.0000 , +0.0000 , -1.0000 , +1.0000 , +0.0000 , +0.0000 , +0.0000 , +2.0000 , +0.0000, 
		+0.5000 , -0.5000 , -0.5000 , +0.0000 , +0.0000 , -1.0000 , +1.0000 , +0.0000 , +0.0000 , +0.0000 , +2.0000 , +1.0000, 
		-0.5000 , -0.5000 , +0.5000 , -1.0000 , +0.0000 , +0.0000 , +0.0000 , +0.0000 , -1.0000 , +0.0000 , +0.0000 , +1.0000, 
		-0.5000 , +0.5000 , +0.5000 , -1.0000 , +0.0000 , +0.0000 , +0.0000 , +0.0000 , -1.0000 , +0.0000 , +0.0000 , +0.0000, 
		-0.5000 , +0.5000 , -0.5000 , -1.0000 , +0.0000 , +0.0000 , +0.0000 , +0.0000 , -1.0000 , +0.0000 , +1.0000 , +0.0000, 
		-0.5000 , -0.5000 , -0.5000 , -1.0000 , +0.0000 , +0.0000 , +0.0000 , +0.0000 , -1.0000 , +0.0000 , +1.0000 , +1.0000, 
		+0.5000 , -0.5000 , +0.5000 , +0.0000 , +0.0000 , +1.0000 , -1.0000 , +0.0000 , +0.0000 , +0.0000 , +3.0000 , +1.0000, 
		+0.5000 , +0.5000 , +0.5000 , +0.0000 , +0.0000 , +1.0000 , -1.0000 , +0.0000 , +0.0000 , +0.0000 , +3.0000 , +0.0000, 
		-0.5000 , +0.5000 , +0.5000 , +0.0000 , +0.0000 , +1.0000 , -1.0000 , +0.0000 , +0.0000 , +0.0000 , +4.0000 , +0.0000, 
		-0.5000 , -0.5000 , +0.5000 , +0.0000 , +0.0000 , +1.0000 , -1.0000 , +0.0000 , +0.0000 , +0.0000 , +4.0000 , +1.0000, 
		+0.5000 , -0.5000 , -0.5000 , +1.0000 , +0.0000 , +0.0000 , +0.0000 , +0.0000 , +1.0000 , +0.0000 , +2.0000 , +1.0000, 
		+0.5000 , +0.5000 , -0.5000 , +1.0000 , +0.0000 , +0.0000 , +0.0000 , +0.0000 , +1.0000 , +0.0000 , +2.0000 , +0.0000, 
		+0.5000 , +0.5000 , +0.5000 , +1.0000 , +0.0000 , +0.0000 , +0.0000 , +0.0000 , +1.0000 , +0.0000 , +3.0000 , +0.0000, 
		+0.5000 , -0.5000 , +0.5000 , +1.0000 , +0.0000 , +0.0000 , +0.0000 , +0.0000 , +1.0000 , +0.0000 , +3.0000 , +1.0000, 
		-0.5000 , +0.5000 , -0.5000 , +0.0000 , +1.0000 , +0.0000 , +1.0000 , +0.0000 , +0.0000 , +0.0000 , +0.0000 , +1.0000, 
		-0.5000 , +0.5000 , +0.5000 , +0.0000 , +1.0000 , +0.0000 , +1.0000 , +0.0000 , +0.0000 , +0.0000 , +0.0000 , +0.0000, 
		+0.5000 , +0.5000 , +0.5000 , +0.0000 , +1.0000 , +0.0000 , +1.0000 , +0.0000 , +0.0000 , +0.0000 , +1.0000 , +0.0000, 
		+0.5000 , +0.5000 , -0.5000 , +0.0000 , +1.0000 , +0.0000 , +1.0000 , +0.0000 , +0.0000 , +0.0000 , +1.0000 , +1.0000, 
		+0.5000 , -0.5000 , -0.5000 , +0.0000 , -1.0000 , +0.0000 , -1.0000 , +0.0000 , +0.0000 , +0.0000 , +0.0000 , +1.0000, 
		+0.5000 , -0.5000 , +0.5000 , +0.0000 , -1.0000 , +0.0000 , -1.0000 , +0.0000 , +0.0000 , +0.0000 , +0.0000 , +0.0000, 
		-0.5000 , -0.5000 , +0.5000 , +0.0000 , -1.0000 , +0.0000 , -1.0000 , +0.0000 , +0.0000 , +0.0000 , +1.0000 , +0.0000, 
		-0.5000 , -0.5000 , -0.5000 , +0.0000 , -1.0000 , +0.0000 , -1.0000 , +0.0000 , +0.0000 , +0.0000 , +1.0000 , +1.0000, 
	};
	unsigned short cube_indices[] = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23};

	printf("loading pixel shader...\n");
		/* load pixel shader */
	struct XenosShader *sh_ps, *sh_vs;
	sh_ps = Xe_LoadShader(xe, "ps.psu");
	Xe_InstantiateShader(xe, sh_ps, 0);

	printf("loading vertex shader...\n");
		/* load vertex shader */
	sh_vs = Xe_LoadShader(xe, "vs.vsu");
	Xe_InstantiateShader(xe, sh_vs, 0);
	Xe_ShaderApplyVFetchPatches(xe, sh_vs, 0, &vbf);

	M_BuildPersp(&g_proj, 45.0 / 180.0 * M_PI, 640.0/480.0, 1, 200.0);

	printf("create vb...\n");
		/* create and fill vertex buffer */
	struct XenosVertexBuffer *vb = Xe_CreateVertexBuffer(xe, sizeof(cube));
	void *v = Xe_VB_Lock(xe, vb, 0, sizeof(cube), XE_LOCK_WRITE);
	memcpy(v, cube, sizeof(cube));
	Xe_VB_Unlock(xe, vb);

	printf("create ib...\n");
		/* create and fill index buffer */
	struct XenosIndexBuffer *ib = Xe_CreateIndexBuffer(xe, sizeof(cube_indices), XE_FMT_INDEX16);
	unsigned short *i = Xe_IB_Lock(xe, ib, 0, sizeof(cube_indices), XE_LOCK_WRITE);
	memcpy(i, cube_indices, sizeof(cube_indices));
	Xe_IB_Unlock(xe, ib);

		/* stats */
	int f = 0;
	int framecount = 0;
	printf("render..\n");
	
	do_edram_foo(xe, 1);

	extern void edram_p4(int *res);

	while (1)
	{
//	do_edram_foo(xe, 1);
		
		f++;
		framecount++;
		
			/* begin a new frame, i.e. reset all renderstates to the default */
		Xe_InvalidateState(xe);
		
			/* load some model-view matrix */
		glLoadIdentity();
		glPushMatrix();
		glTranslate(0, 0, -3);
		glRotate(f / 100.0, .5, .1, 1);
		M_LoadMV(xe, 0); // load model view matrix to VS constant 0
		M_LoadMW(xe, 4); // load (fake) model world matrix to VS constant 4

			/* set the light direction for the pixel shader */
		float lightDirection[] = {0, 0, -1, 0};
		Xe_SetPixelShaderConstantF(xe, 0, lightDirection, 1);

		int max_vertices = sizeof(cube)/(sizeof(*cube)*12);
		int nr_primitives = sizeof(cube_indices)/sizeof(*cube_indices) / 3;

			/* draw cube */
		Xe_SetShader(xe, SHADER_TYPE_PIXEL, sh_ps, 0);
		Xe_SetShader(xe, SHADER_TYPE_VERTEX, sh_vs, 0);
		Xe_SetStreamSource(xe, 0, vb, 0, 12); /* using this vertex buffer */
		Xe_SetIndices(xe, ib); /* ... this index buffer... */
		Xe_SetTexture(xe, 0, fb); /* ... and this texture */
		Xe_DrawIndexedPrimitive(xe, XE_PRIMTYPE_TRIANGLELIST, 0, 0, max_vertices, 0, nr_primitives);

			/* clear to white */
		Xe_SetClearColor(xe, ~0);

			/* resolve (and clear) */
		Xe_Resolve(xe);

			/* wait for render finish */
	 	Xe_Sync(xe);

	 	glPopMatrix();
	 	
	 		/* some stats */
	}
	
	return 0;
}
