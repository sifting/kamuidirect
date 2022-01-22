#include <windows.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <d3d9.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <pvr.h>
#include "local.h"

static HWND _win;
static HINSTANCE _hinst;
static HANDLE _timer;
static Backend *_backends[] =
{
	&backend_d3d9,
	NULL
};
static Backend *_backend = NULL;

KM_global_state km;
Log _log;
Config _cfg;

static void
parse_cfg (const char *file, Config *cfg)
{
#define MAX_TOKEN 128
	Script script;
	
	memset (cfg, 0, sizeof (*cfg));
	cfg->vb_size = 8*1024*1024;
	
	if (script_from_file (&script, file))
	{
		TRACE("No configuration file found");
		return;
	}
	
	while (1)
	{
		char token[MAX_TOKEN];
		int type = 0;
		int code = 0;
		
		code = script_next (&script, &type, token, MAX_TOKEN);
		if (SCRIPT_OK != code)
		{
			TRACE("SCRIPT ERROR");
		}
		
		TRACE("%s", token);
		if (SCRIPT_EOF == type)
		{
			break;
		}
		
		if (!strcmp ("loglevel", token))
		{
			script_next (&script, &type, token, MAX_TOKEN);
			cfg->loglevel = (uint32_t)atoi (token);
			continue;
		}
		if (!strcmp ("framelimit", token))
		{
			script_next (&script, &type, token, MAX_TOKEN);
			cfg->framelimit = (uint32_t)atoi (token);
			continue;
		}
		if (!strcmp ("vb_size", token))
		{
			script_next (&script, &type, token, MAX_TOKEN);
			cfg->vb_size = (size_t)atoi (token)*1024*1024;
			continue;
		}
		if (!strcmp ("backend", token))
		{
			script_next (&script, &type, token, MAX_TOKEN);
			snprintf (cfg->backend, 16, "%16s", token);
			continue;
		}

		TRACE("Unrecognised token \'%s\'", token);
	}
	script_destroy (&script);
}

static int
video_init (HINSTANCE hinst, HWND *window)
{
	static const char *NAME = "Kamui Video-out";
	HWND hwnd;
	WNDCLASS wc;
	
	memset (&wc, 0, sizeof (wc));
	wc.lpfnWndProc = DefWindowProcA;
	wc.hInstance = hinst;
	wc.lpszClassName = NAME;
	if (0 == RegisterClass (&wc))
	{
		return -1;
	}
	
	hwnd = CreateWindowEx (
		0,
		NAME,
		NAME,
		WS_OVERLAPPED|WS_CAPTION|WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		640, 480,
		NULL,
		NULL,
		hinst,
		NULL
	);
	if (NULL == hwnd)
	{
		return -2;
	}
	
	/*All good*/
	ShowWindow (hwnd, SW_NORMAL);
	*window = hwnd;
	return 0;
}

BOOL WINAPI
DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch( fdwReason ) 
    { 
	case DLL_PROCESS_ATTACH:
		log_create (&_log, "kamui_log.txt");
		TRACE("====================");
		TRACE("KAMUI DIRECT!");
		TRACE("Compiled %s @ %s", __DATE__, __TIME__);
		TRACE("Dedicated to all the punks and cowboys out there in c-space");
		TRACE("====================");
		
		_timer = CreateWaitableTimer (NULL, TRUE, NULL);

		parse_cfg ("kamui.cfg", &_cfg);
		_log.level = _cfg.loglevel;
		_hinst = hinstDLL;
		break;
	case DLL_PROCESS_DETACH:
		_backend->destroy_device ();
		
		TRACE("Destroying video-out window...");
		DestroyWindow (_win);	
		
		TRACE("Keep the dream alive.");
		log_destroy (&_log);
		break;
    }
    return TRUE;
}

static inline void
uv_from_f16 (DWORD uv, float *u, float *v)
{
	union
	{
		DWORD dw;
		float fl;
	}val;

	val.dw = uv<<16;
	*u = val.fl;

	val.dw = uv&0xffff0000;
	*v = val.fl;
}
static DWORD
vtx0_copy (void *dst, size_t length, void *src)
{
	struct
	{
		float x, y, z;
		DWORD diff;
		DWORD spec;
		float u, v, s, t;
	}*output = dst;
	struct
	{
		int type;
		float x;
		float y;
		float z;
		int resv0;
		int resv1;
		int argb;
		int resv2;
	}*vtx = src;
	while (length--)
	{
		output->x = vtx->x;
		output->y = vtx->y;
		output->z = vtx->z;
		output->u = 0;
		output->v = 0;
		output->s = 1;
		output->t = 1;
		
		output->diff = vtx->argb;
		output->spec = D3DCOLOR_ARGB(0, 0, 0, 0);
		output++;
		vtx++;
	}
	return 0;
}
static DWORD
vtx1_copy (void *dst, size_t length, void *src)
{
	struct
	{
		float x, y, z;
		DWORD diff;
		DWORD spec;
		float u, v, s, t;
	}*output = dst;
	struct
	{
		int type;
		float x, y, z;
		float a, r, g, b;
	}*vtx = src;
	while (length--)
	{
		output->x = vtx->x;
		output->y = vtx->y;
		output->z = vtx->z;
		output->u = 0;
		output->v = 0;
		output->s = 1;
		output->t = 1;
		
		DWORD a = 255*vtx->a;
		DWORD r = 255*vtx->r;
		DWORD g = 255*vtx->g;
		DWORD b = 255*vtx->b;
		output->diff = D3DCOLOR_ARGB(a, r, g, b);
		output->spec = D3DCOLOR_ARGB(0, 0, 0, 0);
		output++;
		vtx++;
	}
	return 0;
}
static DWORD
vtx2_copy (void *dst, size_t length, void *src)
{
	struct
	{
		float x, y, z;
		DWORD diff;
		DWORD spec;
		float u, v, s, t;
	}*output = dst;
	struct
	{
		int type;
		float x;
		float y;
		float z;
		int resv0;
		int resv1;
		float intensity;
		int resv2;
	}*vtx = src;
	while (length--)
	{
		output->x = vtx->x;
		output->y = vtx->y;
		output->z = vtx->z;
		output->u = 0;
		output->v = 0;
		output->s = 0;
		output->t = 1;
		
		int white = 255*vtx->intensity;
		output->diff = D3DCOLOR_ARGB (255, white, white, white);
		output->spec = D3DCOLOR_ARGB(0, 0, 0, 0);
		output++;
		vtx++;
	}
	return 0;
}
static DWORD
vtx3_copy (void *dst, size_t length, void *src)
{
	struct
	{
		float x, y, z;
		DWORD diff;
		DWORD spec;
		float u, v, s, t;
	}*output = dst;
	struct
	{
		int type;
		float x;
		float y;
		float z;
		float u, v;
		DWORD diff;
		DWORD spec;
	}*vtx = src;
	while (length--)
	{
		output->x = vtx->x;
		output->y = vtx->y;
		output->z = vtx->z;
		
		output->u = vtx->u;
		output->v = vtx->v;
		output->s = vtx->z;
		output->t = vtx->z;
		
		output->diff = vtx->diff;
		output->spec = vtx->spec;
		output++;
		vtx++;
		
		
	}
	return 1;
}
static DWORD
vtx4_copy (void *dst, size_t length, void *src)
{
	struct
	{
		float x, y, z;
		DWORD diff;
		DWORD spec;
		float u, v, s, t;
	}*output = dst;
	struct
	{
		int type;
		float x;
		float y;
		float z;
		DWORD uv;
		DWORD resv;
		DWORD diff;
		DWORD spec;
	}*vtx = src;
	while (length--)
	{
		output->x = vtx->x;
		output->y = vtx->y;
		output->z = vtx->z;
		
		uv_from_f16 (vtx->uv, &output->u, &output->v);

		output->diff = vtx->diff;
		output->spec = vtx->spec;
		output++;
		vtx++;
	}
	return 1;
}
static DWORD
vtx5_copy (void *dst, size_t length, void *src)
{
	struct
	{
		float x, y, z;
		DWORD diff;
		DWORD spec;
		float u, v, s, t;
	}*output = dst;
	struct
	{
		int type;
		float x;
		float y;
		float z;
		DWORD uv;
		DWORD resv0;
		DWORD resv1;
		DWORD resv2;
		float diff[4];
		float spec[4];
	}*vtx = src;
	while (length--)
	{
		output->x = vtx->x;
		output->y = vtx->y;
		output->z = vtx->z;	
		
		uv_from_f16 (vtx->uv, &output->u, &output->v);
		
		DWORD a0 = 255*vtx->diff[0];
		DWORD r0 = 255*vtx->diff[1];
		DWORD g0 = 255*vtx->diff[2];
		DWORD b0 = 255*vtx->diff[3];
		output->diff = D3DCOLOR_ARGB(a0, r0, g0, b0);
		
		DWORD a1 = 255*vtx->spec[0];
		DWORD r1 = 255*vtx->spec[1];
		DWORD g1 = 255*vtx->spec[2];
		DWORD b1 = 255*vtx->spec[3];		
		output->spec = D3DCOLOR_ARGB(a1, r1, g1, b1);
		output++;
		vtx++;
	}
	return 1;
}
static DWORD
vtx6_copy (void *dst, size_t length, void *src)
{
	struct
	{
		float x, y, z;
		DWORD diff;
		DWORD spec;
		float u, v, s, t;
	}*output = dst;
	struct
	{
		int type;
		float x;
		float y;
		float z;
		DWORD uv;
		DWORD resv0;
		DWORD resv1;
		DWORD resv2;
		float diff[4];
		float spec[4];
	}*vtx = src;
	while (length--)
	{
		output->x = vtx->x;
		output->y = vtx->y;
		output->z = vtx->z;
		
		uv_from_f16 (vtx->uv, &output->u, &output->v);
		
		DWORD a0 = 255*vtx->diff[0];
		DWORD r0 = 255*vtx->diff[1];
		DWORD g0 = 255*vtx->diff[2];
		DWORD b0 = 255*vtx->diff[3];
		output->diff = D3DCOLOR_ARGB(a0, r0, g0, b0);
		
		DWORD a1 = 255*vtx->spec[0];
		DWORD r1 = 255*vtx->spec[1];
		DWORD g1 = 255*vtx->spec[2];
		DWORD b1 = 255*vtx->spec[3];		
		output->spec = D3DCOLOR_ARGB(a1, r1, g1, b1);
		
		output++;
		vtx++;
	}
	return 1;
}
static DWORD
vtx7_copy (void *dst, size_t length, void *src)
{
	struct
	{
		float x, y, z;
		DWORD diff;
		DWORD spec;
		float u, v, s, t;
	}*output = dst;
	struct
	{
		int type;
		float x;
		float y;
		float z;
		float u;
		float v;
		float diff;
		float spec;
	}*vtx = src;
	while (length--)
	{
		output->x = vtx->x;
		output->y = vtx->y;
		output->z = vtx->z;
		
		output->u = vtx->u;
		output->v = vtx->v;
		output->s = vtx->z;
		output->t = vtx->z;
		
		DWORD a0 = 255*km.ctx.a;
		DWORD r0 = 255*km.ctx.r;
		DWORD g0 = 255*km.ctx.g;
		DWORD b0 = 255*km.ctx.b;
		output->diff = D3DCOLOR_ARGB(a0, r0, g0, b0);
		
		DWORD a1 = vtx->spec*255*km.ctx.sa;
		DWORD r1 = vtx->spec*255*km.ctx.sr;
		DWORD g1 = vtx->spec*255*km.ctx.sg;
		DWORD b1 = vtx->spec*255*km.ctx.sb;		
		output->spec = D3DCOLOR_ARGB(a1, r1, g1, b1);
		output++;
		vtx++;
	}
	return 1;
}
static DWORD
vtx8_copy (void *dst, size_t length, void *src)
{
	struct
	{
		float x, y, z;
		DWORD diff;
		DWORD spec;
		float u, v, s, t;
	}*output = dst;
	struct
	{
		int type;
		float x;
		float y;
		float z;
		DWORD uv;
		float diff;
		float spec;
	}*vtx = src;
	while (length--)
	{
		output->x = vtx->x;
		output->y = vtx->y;
		output->z = vtx->z;
		
		uv_from_f16 (vtx->uv, &output->u, &output->v);
		
		int diff = 255*vtx->diff;
		int spec = 255*vtx->spec;
		output->diff = D3DCOLOR_ARGB (255, diff, diff, diff);
		output->spec = D3DCOLOR_ARGB (255, spec, spec, spec);
		output++;
		vtx++;
	}
	return 1;
}

typedef DWORD (*KM_vtx_copy) (void *, size_t, void *);
static KM_vtx_copy _vfuncs[] =
{
	vtx0_copy,
	vtx1_copy,
	vtx2_copy,
	vtx3_copy,
	vtx4_copy,
	vtx5_copy,
	vtx6_copy,
	vtx7_copy,
	vtx8_copy
};

static KM_command *
km_command_alloc (int code, size_t size)
{
	KM_command_list *cl = km.cl + km.curr_list;
	KM_command *cmd = (KM_command *)((char *)cl->buffer + cl->used);
	cmd->code = code;
	cmd->size = size;
	cl->used += size;
	return cmd;
}

/*Misc state*/
KMAPI int
kmInitDevice (int device)
{
	TRACE("%s : device (%i)", __func__, device);
	
	memset (&km, 0, sizeof (km));
	km.opaque = malloc (16*1024*1024);
	if (NULL == km.opaque)
	{
		return KM_NO_MEMORY;
	}
	km.sorted = malloc (16*1024*1024);
	if (NULL == km.sorted)
	{
		return KM_NO_MEMORY;
	}
	km.geobuf = malloc (16*1024*1024);
	km.device = device;
	
	for (int i = 0; i < KM_MAX_DISPLAY_LIST; i++)
	{
		KM_command_list *cl = km.cl + i;
		cl->buffer = malloc (16*1024*1024);
		cl->size = 16*1024*1024;
		cl->used = 0;
	}
	
	_backend = _backends[0];
	for (int i = 0; _backends[i] != NULL; i++)
	{
		if (strcmp (_backends[i]->tag, _cfg.backend) != 0)
		{
			continue;
		}
		_backend = _backends[i];
	}
	VERIFY(_backend != NULL, "No valid backends?");
	TRACE("Selected %s as the backend", _backend->tag);
			
	TRACE("Creating video-out window...");
	switch (video_init (_hinst, &_win))
	{
	case -1:
		TRACE("  Failed to register classname");
		return FALSE;
	case -2:
		TRACE("  Failed to create window");
		return FALSE;
	default:
		TRACE("  Okay!");
	}

	TRACE("Initialising backend...");
	switch (_backend->init_device (device, _hinst, _win))
	{
	case -1:
		TRACE("  Failed to create d3d9 interface!");
		return FALSE;
	case -2:
		TRACE("  Failed to create d3d9 device!");
		return FALSE;
	case -3:
		TRACE("  Failed to create d3d9 buffers!");
		return FALSE;
	default:
		TRACE("  Okay!");	
	}	
	return KM_OK;
}

KMAPI int
kmSetDisplayMode (int mode, int bpp, int dither, int aa)
{
	TRACE("%s : mode (%i) bpp (%i) dither (%i) aa (%i)", __func__, mode, bpp, dither, aa);
	return KM_OK;
}

KMAPI int
kmSetWaitVsyncCount (int count)
{
	TRACE("%s : count (%i)", __func__, count);
	return KM_OK;
}
KMAPI int
kmSetCullingRegister (float value)
{
	TRACE("%s : value (%f)", __func__, value);
	return KM_OK;
}
KMAPI int
kmSetPixelClipping (int xmin, int ymin, int xmax, int ymax)
{
	TRACE("%s : xmin (%i) ymin (%i) xmax (%i) ymax (%i)", __func__, xmin, ymin, xmax, ymax);
	int x0 = xmin;
	int y0 = ymin;
	int x1 = xmax;
	int y1 = ymax;
	if (KM_RGB888 == km.display)
	{
		x0 += (xmin&1);
		y0 += (ymin&1);
		x1 -= (xmax&1);
		y1 -= (ymax&1);		
	}
	if (km.width <= x0 - x1)
	{
		return KM_BAD_PARAM;
	}
	if (km.height <= y0 - y1)
	{
		return KM_BAD_PARAM;
	}
	return _backend->pixel_clipping (x0, y0, x1, y1);
}
KMAPI int
kmRender (void)
{
	TRACE("%s", __func__);
	
	/*Activate the timer for the frame if using the limiter.
	If we're running slower than the requested rate then this
	will just expire partway through the frame and skip wait
	at the end*/
	if (_cfg.framelimit != 0)
	{
		float tick = 1000.0/_cfg.framelimit;
		LARGE_INTEGER duration = {.QuadPart = -tick*1000LL};
		SetWaitableTimer (_timer, &duration, 0, NULL, NULL, 0);
	}

	_backend->render ();
							
	if (km.eor.func != NULL)
	{
		km.eor.func (km.eor.args);
	}
	
	/*Wait out residual time if using frame limiter*/
	if (_cfg.framelimit != 0)
	{
		WaitForSingleObject (_timer, INFINITE);
	}
	
	return KM_OK;
}

/*Framebuffers*/
KMAPI int
kmCreateFrameBufferSurface (
	KM_surface_desc *surf,
	int width, int height,
	int stripbuffer,
	int bufferclear
) {
	TRACE(\
	"%s : surf (%p) width (%i) height (%i), stripbuffer (%i) bufferclear (%i)",\
	__func__, surf, width, height, stripbuffer, bufferclear);
	memset (surf, 0, sizeof (*surf));
	surf->type = 1;
	surf->depth = 1;
	surf->width = width;
	surf->height = height;
	surf->size = width*height*4;
	return KM_OK;
}
KMAPI int
kmActivateFrameBuffer (
	KM_surface_desc *front,
	KM_surface_desc *back,
	int stripbuffer,
	int vsync
) {
	TRACE("%s : front (%p) back (%p) stripbuffer (%i) vsync (%i)",\
		__func__, front, back, stripbuffer, vsync);
	return KM_OK;
}
KMAPI int
kmFlipFrameBuffer (void)
{
	TRACE("%s", __func__);
	return _backend->framebuffer_flip ();
}

/*Callbacks*/
KMAPI int
kmSetVSyncCallback (KM_callback func, void *args)
{
	TRACE("%s", __func__);
	km.vsync.func = func;
	km.vsync.args = args;
	return KM_OK;
}
KMAPI int
kmSetEORCallback (KM_callback func, void *args)
{
	TRACE("%s", __func__);
	km.eor.func = func;
	km.eor.args = args;
	return KM_OK;
}
KMAPI int
kmSetEndOfVertexCallback (KM_callback func, void *args)
{
	TRACE("%s", __func__);
	km.eov.func = func;
	km.eov.args = args;
	return KM_OK;
}

/*Vertices*/
KMAPI int
kmCreateVertexBuffer (
	KM_vertex_desc *desc,
	int opaque_buffer,
	int opaque_modifier,
	int trans_buffer,
	int trans_modifier
) {
	TRACE("%s : desc (%p) opaque_buffer (%i) opaque_modifier (%i) trans_buffer (%i) trans_modifier (%i)",\
	__func__,\
	desc,\
	opaque_buffer, opaque_modifier,\
	trans_buffer, trans_modifier);

	/*Verify and setup state*/
	VERIFY(0 <= opaque_buffer, "opaque_buffer is < 0");
	VERIFY(0 <= opaque_modifier, "opaque_modifer is < 0");
	VERIFY(0 <= trans_buffer, "trans_buffer is < 0");
	VERIFY(0 <= trans_modifier, "trans_modifier is < 0");
	void *ptrs[KM_MAX_DISPLAY_LIST] = {
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
	};
	int sizes[KM_MAX_DISPLAY_LIST] = {
		opaque_buffer,
		opaque_modifier,
		trans_buffer,
		trans_modifier,
		0
	};

	/*Allocate buffers as needed*/
	for (int i = 0; i < KM_MAX_DISPLAY_LIST; i++)
	{
		int size = sizes[i];
		if (size <= 0)
		{
			continue;
		}
		void *ptr = malloc (size);
		if (NULL == ptr)
		{
			for (int j = 0; j < i; j++)
			{
				if (ptrs[j]) free (ptrs[j]);
				ptrs[j] = NULL;
			}
			return KM_BAD_PARAM;
		}
		ptrs[i] = ptr;
	}

	/*Store buffers into global state*/
	memcpy (km.base, ptrs, sizeof (km.base));
	for (int i = 0; i < KM_MAX_DISPLAY_LIST; i++)
	{
		km.lists[i] = km.base[i];
	}
	return KM_OK;
}
KMAPI int
kmSetVertexRenderState (KM_vertex_ctx *ctx)
{
	TRACE("%s : ctx (%p)", __func__, ctx);

	int state = ctx->state;
	
	km.curr_list = ctx->list_type;
	km.ctx.list_type = ctx->list_type;
	if (ctx->surface)
	{
		KM_surface_desc *desc = (KM_surface_desc *)ctx->surface;
		if (desc)
		{
			KM_shim *shim = desc->surface;
			km.ctx.surface = shim->surface;
		}
	}

	if (state&KM_DEPTHMODE)
	{
		km.ctx.depth_mode = ctx->depth_mode;
	}
	if (state&KM_CULLINGMODE)
	{
		km.ctx.cull_mode = ctx->cull_mode;
	}
	if (state&KM_SHADINGMODE)
	{
		km.ctx.shade_mode = ctx->shade_mode;
	}
	if (state&KM_ZWRITEDISABLE)
	{
		km.ctx.depth_write = ctx->depth_write;
	}
	if (state&KM_SRCBLENDINGMODE)
	{
		km.ctx.src_blend = ctx->src_blend;
	}
	if (state&KM_DSTBLENDINGMODE)
	{
		km.ctx.dst_blend = ctx->dst_blend;
	}
	if (state&KM_CLAMPUV)
	{
		km.ctx.clamp_uv = ctx->clamp_uv;
	}
	if (state&KM_FLIPUV)
	{
		km.ctx.flip_uv = ctx->flip_uv;
	}
	if (state&KM_FILTERMODE)
	{
		km.ctx.filter_mode = ctx->filter_mode;
	}
	if (state&KM_TEXTURESHADINGMODE)
	{
		km.ctx.texture_mode = ctx->texture_mode;
	}
	
	if (state&KM_USEALPHA)
	{
		km.ctx.use_alpha = ctx->use_alpha;
	}
	
	km.ctx.a = ctx->a;
	km.ctx.r = ctx->r;
	km.ctx.g = ctx->g;
	km.ctx.b = ctx->b;

	km.ctx.sa = ctx->sa;
	km.ctx.sr = ctx->sr;
	km.ctx.sg = ctx->sg;
	km.ctx.sb = ctx->sb;
	return KM_OK;
}
KMAPI int
kmProcessVertexRenderState (KM_vertex_ctx *ctx)
{
	TRACE("%s : ctx (%p)", __func__, ctx);
	return KM_OK;
}
KMAPI int
kmDiscardVertexBuffer (KM_vertex_desc *desc)
{
	TRACE("%s : desc (%p)", __func__, desc);
	return KM_OK;
}
KMAPI int
kmStartVertexStrip (KM_vertex_desc *desc)
{
	TRACE("%s : desc (%p)", __func__, desc);
	return KM_OK;
}
KMAPI int
kmSetVertex (KM_vertex_desc *desc, void *v, int type, int size)
{
	TRACE("%s : desc (%p) v (%p) type (%i) size (%i)", __func__, desc, v, type, size);
	
	/*No modifier volumes for now*/
	if (type >= 8)
	{
		return KM_OK;
	}
	
	unsigned char *base = (unsigned char *)km.base[km.curr_list];
	unsigned char **list = (unsigned char **)&km.lists[km.curr_list];
	memcpy (*list, v, size);
	*list += size;
	
	int code = *(int *)v;
	if (KM_ENDOFSTRIP == code)
	{
		size_t length = (size_t)((ptrdiff_t)(*list - base))/size;
		size_t verts = length*36;
		
		/*Store vertex data in the polygon object*/		
		KM_polygon *ply = (KM_polygon *)km_command_alloc (KMC_POLY, sizeof (*ply));		
		memcpy (&(ply->ctx), &km.ctx, sizeof (ply->ctx));
		ply->next = NULL;
		ply->offset = km.geobuf_indices;
		ply->sort = ((float *)base)[2];
		ply->nverts = length;
		ply->index = 0;
		
		_vfuncs[type] ((char *)km.geobuf + km.geobuf_used, length, base);
		km.geobuf_used += verts;
		km.geobuf_indices += length;
		
#if 0
		/*Compute centroid*/
		float centroid[3] = {0, 0, 0};
		float centroid2[3] = {0,0,0};
		float *point = (float *)((char *)km.geobuf + 36*ply->offset);
		float de = 0;
		for (int i = 0; i < ply->nverts; i++)
		{
			for (int j = 0; j < 3; j++) centroid[j] += point[j];
			point += 36/4;
			de += 1.0;
		}
		float rp = 1.0/de;
		for (int i = 0; i < 3; i++) centroid2[i] = rp*centroid[i];
		ply->sort = sqrtf (centroid[0]*centroid[0] + centroid[1]*centroid[1] + centroid[2]*centroid2[2]);
		ply->sort = log2 (1.0 + centroid[2]*100000.0)/34.0;
#else
		float *point = (float *)((char *)km.geobuf + 36*ply->offset);
		ply->sort = log2 (1.0 + point[2]*100000.0)/34.0;
#endif
		
		/*Reset the list head*/
		*list = base;
		
		if (km.eov.func)
		{
			km.eov.func (km.eov.args);
		}
	}
	return KM_OK;
}
/*Textures*/
KMAPI int
kmCreateTextureSurface (
	KM_surface_desc *surf,
	int width, int height,
	int type
) {
	TRACE("%s : surf (%p) width (%i) height (%i) type (%i)", __func__, surf, width, height, type);
	int flags = 0;
	int fmt = (type&0xff00)>>8;
	int pix = type&0x00ff;

	memset (surf, 0, sizeof (*surf));
	surf->type = 2;
	surf->depth = 1;
	
	switch (pix)
	{
	case KM_TEXTURE_ARGB1555:
		surf->format = KM_PIXELFORMAT_ARGB1555;
		break;
	case KM_TEXTURE_RGB565:
		surf->format = KM_PIXELFORMAT_RGB565;
		break;
	case KM_TEXTURE_ARGB4444:
		surf->format = KM_PIXELFORMAT_ARGB4444;
		break;
	case KM_TEXTURE_YUV422: surf->format = KM_PIXELFORMAT_YUV422; break;
	case KM_TEXTURE_BUMP: surf->format = KM_PIXELFORMAT_BUMP; break;
	default: break;
	}
	
	switch (fmt)
	{
	case PVR_TWIDDLED:
	case PVR_RECTANGLE_TWIDDLED:
		flags = 0;
		break;
	case PVR_TWIDDLED_MIPMAP:
	case PVR_TWIDDLED_MIPMAP_DMA:
		flags = KM_SURFACEFLAGS_MIPMAPED;
		break;
	case PVR_VQ:
	case PVR_SMALL_VQ:
		flags = KM_SURFACEFLAGS_VQ;
		break;
	case PVR_VQ_MIPMAP:
	case PVR_SMALL_VQ_MIPMAP:
		flags = KM_SURFACEFLAGS_VQ|KM_SURFACEFLAGS_MIPMAPED;
		break;
	case PVR_RECTANGLE:
		flags = KM_SURFACEFLAGS_NOTWIDDLED;
		break;
	case PVR_RECTANGLE_MIPMAP:
		flags = KM_SURFACEFLAGS_MIPMAPED|KM_SURFACEFLAGS_NOTWIDDLED;
		break;
	case PVR_STRIDE:
		flags = KM_SURFACEFLAGS_STRIDE|KM_SURFACEFLAGS_NOTWIDDLED;
		break;
	case PVR_STRIDE_TWIDDLED:
		flags = KM_SURFACEFLAGS_STRIDE;
		break;
	case PVR_BITMAP: break;
	case PVR_BITMAP_MIPMAP: break;
	case PVR_PAL8:
	case PVR_PAL4: 
		flags = KM_SURFACEFLAGS_PALETTIZED;
		break;
	case PVR_PAL8_MIPMAP:
	case PVR_PAL4_MIPMAP:
		flags = KM_SURFACEFLAGS_PALETTIZED|KM_SURFACEFLAGS_MIPMAPED;
		break;
	}
	switch (width)
	{
	case 8: flags |= KM_SURFACEFLAGS_USIZE8; break;
	case 16: flags |= KM_SURFACEFLAGS_USIZE16; break;
	case 32: flags |= KM_SURFACEFLAGS_USIZE32; break;
	case 64: flags |= KM_SURFACEFLAGS_USIZE64; break;
	case 128: flags |= KM_SURFACEFLAGS_USIZE128; break;
	case 256: flags |= KM_SURFACEFLAGS_USIZE256; break;
	case 512: flags |= KM_SURFACEFLAGS_USIZE512; break;
	case 1024: flags |= KM_SURFACEFLAGS_USIZE1024; break;
	default: break;
	}
	switch (height)
	{
	case 8: flags |= KM_SURFACEFLAGS_VSIZE8; break;
	case 16: flags |= KM_SURFACEFLAGS_VSIZE16; break;
	case 32: flags |= KM_SURFACEFLAGS_VSIZE32; break;
	case 64: flags |= KM_SURFACEFLAGS_VSIZE64; break;
	case 128: flags |= KM_SURFACEFLAGS_VSIZE128; break;
	case 256: flags |= KM_SURFACEFLAGS_VSIZE256; break;
	case 512: flags |= KM_SURFACEFLAGS_VSIZE512; break;
	case 1024: flags |= KM_SURFACEFLAGS_VSIZE1024; break;
	default: break;
	}
	//flags = 0;
	
	{/*Compute total size, including mipmaps and codebooks and such*/
		uint32_t size = 0;
		if (flags&KM_SURFACEFLAGS_MIPMAPED)
		{
			uint32_t w = width;
			uint32_t h = height;
			uint32_t lo = w < h ? w : h;
			if (flags&KM_SURFACEFLAGS_VQ)
			{
				size += 2048;
				size += 1;
				w >>= 1;
				h >>= 1;
			}
			while (lo != 0)
			{
				size += w*h;
				lo >>= 1;
				w >>= 1;
				h >>= 1;
			}
		}
		else size = width*height;
		if (!(flags&(KM_SURFACEFLAGS_PALETTIZED|KM_SURFACEFLAGS_VQ)))
		{
			size <<= 1;
		}
		surf->size = size + 2;
	}
	surf->width = width;
	surf->height = height;
	surf->flags = flags;
	
	/*This is needed to convert PVR textures properly
	It's not possible to losslessly recover the format flags
	with the standard fields, so we box them up inside the shim
	and store it on the ->surface field, then store the actual
	texture object inside the shim with everything else we need.
	
	Not great, but not the worst either*/
	KM_shim *shim = malloc (sizeof (*shim));
	memset (shim, 0, sizeof (*shim));
	shim->type = pix;
	shim->format = fmt;
	surf->surface = shim;
	return _backend->texture_create (surf, width, height, flags);
}

KMAPI int
kmLoadTexture (KM_surface_desc *surf, void *data, int unk0, int unk1)
{
	TRACE("%s : surf (%p) data (%p) unk0 (%i) unk1 (%i)", __func__, surf, data, unk0, unk1);
	
	PVR_surface tex;
	KM_shim *shim = surf->surface;
	if (!(surf->flags&KM_SURFACEFLAGS_MIPMAPED)) data = (char *)data - 2;
	if (pvr_surface_decode_ptr (&tex, shim->format, shim->type, surf->width, surf->height, surf->size, data))
	{
		TRACE("Failed to decode");
		return KM_OK;
	}
	
	return _backend->texture_load (surf, tex.width, tex.height, tex.bpp, tex.data);
}
KMAPI int
kmFreeTexture (KM_surface_desc *surf)
{
	TRACE("%s : surf (%p)", __func__, surf);
	return _backend->texture_free (surf);
}
KMAPI int
kmReLoadMipmap (KM_surface_desc *surf, void *texture, int count)
{
	TRACE("%s : surf (%p), texture (%p) count (%i)", __func__, surf, texture, count);
	return KM_OK;
}
KMAPI int
kmGetFreeTextureMem (int *size, int *block)
{
	TRACE("%s", __func__);
	*size = 32*1024*1024;
	*block = 1*1024*1024;
	return KM_OK;
}
KMAPI int
kmSetFramebufferTexture (KM_surface_desc *surface)
{
	TRACE("%s : surf (%p)", __func__, surface);
	return KM_OK;
}

/*Modifier Volumes*/
KMAPI int
kmSetModifierRenderState (KM_vertex_desc *desc, KM_vertex_ctx *ctx)
{
	TRACE("%s", __func__);
	return KM_OK;
}
KMAPI int
kmUseAnotherModifier (int modifier)
{
	TRACE("%s", __func__);
	return KM_OK;
}

/*Background Planes*/
KMAPI int
kmSetBackGroundPlane (void *v[3], int type, int size)
{
	TRACE("%s", __func__);
	return KM_OK;
}
KMAPI int
kmSetBackGroundRenderState (KM_vertex_ctx *ctx)
{
	TRACE("%s", __func__);
	return KM_OK;
}

KMAPI int
kmSetFogTable (void *user)
{
	return KM_OK;
}
KMAPI int
kmSetFogDensity (void *user)
{
	return KM_OK;
}
KMAPI int
kmSetFogTableColor (void *user)
{
	return KM_OK;
}


