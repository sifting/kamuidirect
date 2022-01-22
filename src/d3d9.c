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
#include "log.h"
#include "local.h"

/*Raw strings are not in the C spec,
but the preprocessor can work around that...*/
#define RAWSTRING(...) #__VA_ARGS__

const char *kamui_vertex = RAWSTRING(
	struct VS_INPUT
	{
		float3 pos : POSITION;
		float2 tc : TEXCOORD0;
		float4 colour : COLOR0;
		float4 offset : COLOR1;
	};
	struct VS_OUTPUT
	{
		float4 pos : POSITION;
		float4 tc : TEXCOORD0;
		float4 colour : COLOR0;
		float4 offset : COLOR1;
	};

	float4x4 tform : register (c0);

	VS_OUTPUT main (in VS_INPUT v)
	{
		VS_OUTPUT o;
		o.pos = mul (tform, float4 (v.pos, 1.0));
		o.tc = float4 (v.tc*o.pos.z, 0, o.pos.z);
		o.colour = v.colour;
		o.offset = v.offset*v.pos.z;
		o.pos.z = 0;
		return o;
	}
);

const char *kamui_frag = RAWSTRING(
	struct PS_INPUT
	{
		float4 tc : TEXCOORD0;
		float4 colour : COLOR0;
		float4 offset : COLOR1;
	};
	struct PS_OUTPUT
	{
		float4 colour : COLOR0;
		float depth : DEPTH;
	};

	sampler2D tex : register (s0);
	float4 trialpha : register (c0);
	float4 mode : register (c1);

	PS_OUTPUT main (in PS_INPUT p)
	{
		PS_OUTPUT o;
		float4 ofs = p.offset/p.tc.w;
		float4 col = p.colour;
		float4 texel = tex2Dproj (tex, p.tc);
	
	
		int use_alpha = int (mode.y + 0.5);
		if (0 == use_alpha) col.a = 1.0;
		
		int val = int (mode.x + 0.5);
		if (val == 0)
		{
			o.colour = texel;
		}
		else if (val == 1)
		{
			o.colour.rgb = col.rgb*texel.rgb;
			o.colour.a = texel.a;
		}
		else if (val == 2)
		{
			o.colour.rgb = lerp (col.rgb, texel.rgb, texel.a);
			o.colour.a = col.a;
		}
		else
		{
			o.colour = col*texel;
		}
		o.colour.rgb += ofs.rgb;
		o.colour *= trialpha;
		o.depth = log2 (1.0 + p.tc.w*100000.0)/34.0;
		return o;
	}
);

static HWND _win;
static HINSTANCE _hinst;
static IDirect3D9 *_d3d9 = NULL;
static IDirect3DDevice9 *_d3dev = NULL;
static IDirect3DVertexBuffer9 *_buf = NULL;
static IDirect3DVertexShader9 *_vs = NULL;
static IDirect3DPixelShader9 *_ps = NULL;
static LPDIRECT3DVERTEXDECLARATION9 _vtxdecl;

static int
d3d_init (HWND hwnd, IDirect3D9 **iface, IDirect3DDevice9 **device)
{
	HRESULT status;
	
	IDirect3D9 *d3d9 = Direct3DCreate9 (D3D_SDK_VERSION);
	if (NULL == d3d9)
	{
		return -1;
	}
	
	D3DPRESENT_PARAMETERS params;
	memset (&params, 0, sizeof (params));
	params.Windowed = TRUE;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.EnableAutoDepthStencil = TRUE;
	params.AutoDepthStencilFormat = D3DFMT_D24S8;
	params.hDeviceWindow = hwnd;
	params.BackBufferWidth = 640;
	params.BackBufferHeight = 480;
	params.BackBufferFormat = D3DFMT_A8R8G8B8;
	params.BackBufferCount = 1;
	params.MultiSampleType = D3DMULTISAMPLE_NONE;
	params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	
	IDirect3DDevice9 *dev;
	status = IDirect3D9_CreateDevice (
		d3d9,
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hwnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING
		|D3DCREATE_MULTITHREADED,
		&params,
		&dev
	);
	if (D3D_OK != status)
	{
		IDirect3D9_Release (dev);
		return -2;
	}

	/*Create vertex buffer and definition*/
	status = IDirect3DDevice9_CreateVertexBuffer (
		dev,
		_cfg.vb_size,
		0,
		0,
		D3DPOOL_MANAGED,
		&_buf,
		NULL
	);
	if (D3D_OK != status)
	{
		return -3;
	}
	
	D3DVERTEXELEMENT9 decl[] = {
		{0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		{0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1},
		{0, 20, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};
	IDirect3DDevice9_CreateVertexDeclaration (dev, decl, &_vtxdecl);
	IDirect3DDevice9_SetVertexDeclaration (dev, _vtxdecl);
	
	/*Create vertex shader*/
	ID3DBlob *vs = NULL;
	ID3DBlob *ps = NULL;
	ID3DBlob *error = NULL;
	status = D3DCompile (
		kamui_vertex,
		strlen (kamui_vertex),
		"Vertex Shader",
		NULL,
		NULL,
		"main",
		"vs_3_0",
		D3DCOMPILE_DEBUG,
		0,
		&vs,
		&error
	);
	if (D3D_OK != status)
	{
		CRITICAL("%s", (char *)error->lpVtbl->GetBufferPointer (error));
		return -4;
	}
	status = IDirect3DDevice9_CreateVertexShader (
		dev, 
		vs->lpVtbl->GetBufferPointer (vs),
		&_vs
	);
	if (D3D_OK != status)
	{
		return -5;
	}
	vs->lpVtbl->Release (vs);
	
	/*Create fragment shader*/
	status = D3DCompile (
		kamui_frag,
		strlen (kamui_frag),
		"Fragment Shader",
		NULL,
		NULL,
		"main",
		"ps_3_0",
		D3DCOMPILE_DEBUG,
		0,
		&ps,
		&error
	);
	if (D3D_OK != status)
	{
		CRITICAL("%s", (char *)error->lpVtbl->GetBufferPointer (error));
		return -6;
	}
	status = IDirect3DDevice9_CreatePixelShader (
		dev,
		ps->lpVtbl->GetBufferPointer (ps),
		&_ps
	);
	if (D3D_OK != status)
	{
		return -7;
	}
	ps->lpVtbl->Release (ps);
	
	*iface = d3d9;
	*device = dev;
	return 0;
}
/*Misc state*/
static int
InitDevice (int device, void *instance, void *window)
{
	static const float proj[] = {
		2.0/640.0, 0, 0, 0,
		0, -2.0/480.0, 0, 0,
		0, 0, 1, 0,
		-1, 1, 0, 1
	};
	
	_hinst = (HINSTANCE)instance;
	_win = (HWND)window;
	
	d3d_init ((HWND)window, &_d3d9, &_d3dev);
	
	IDirect3DDevice9_SetVertexShader (_d3dev, _vs);
	IDirect3DDevice9_SetVertexShaderConstantF (_d3dev, 0, proj, 4);
	IDirect3DDevice9_SetPixelShader (_d3dev, _ps);
	
	D3DVIEWPORT9 vp;
	memset (&vp, 0, sizeof (vp));
	vp.X = 0;
	vp.Y = 0;
	vp.Width = 640;
	vp.Height = 480;
	vp.MinZ = 0;
	vp.MaxZ = 1;
	IDirect3DDevice9_SetViewport (_d3dev, &vp);
	
	return KM_OK;
}
static int
DestroyDevice (void)
{
	TRACE("Destroying D3D9...");
	if (_d3dev) IDirect3DDevice9_Release (_d3dev);
	if (_d3d9) IDirect3D9_Release (_d3d9);
	return KM_OK;
}
static int
SetPixelClipping (int xmin, int ymin, int xmax, int ymax)
{
	D3DVIEWPORT9 vp;
	memset (&vp, 0, sizeof (vp));
	vp.X = xmin;
	vp.Y = ymin;
	vp.Width = xmax;
	vp.Height = ymax;
	vp.MinZ = 0;
	vp.MaxZ = 1;
	IDirect3DDevice9_SetViewport (_d3dev, &vp);
	return KM_OK;
}

static void
apply_context (KM_vertex_ctx *ctx)
{
	static DWORD sblends[] =
	{
		D3DBLEND_INVSRCALPHA,
		D3DBLEND_SRCALPHA,
		D3DBLEND_SRCCOLOR,
		D3DBLEND_SRCALPHA,
		D3DBLEND_INVSRCALPHA,
		D3DBLEND_INVSRCCOLOR,		
		D3DBLEND_INVDESTALPHA,
		D3DBLEND_INVDESTCOLOR,
		D3DBLEND_SRCALPHA,		//this one
		D3DBLEND_DESTALPHA,
		D3DBLEND_ONE,
		D3DBLEND_ZERO
	};
	static DWORD dblends[] =
	{
		D3DBLEND_SRCALPHA,
		D3DBLEND_INVSRCALPHA,
		D3DBLEND_DESTALPHA,
		D3DBLEND_DESTCOLOR,
		D3DBLEND_INVSRCCOLOR,
		D3DBLEND_INVSRCALPHA,
		D3DBLEND_INVSRCALPHA,	//this one
		D3DBLEND_INVDESTALPHA,
		D3DBLEND_SRCALPHA,
		D3DBLEND_SRCCOLOR,
		D3DBLEND_ONE,
		D3DBLEND_ZERO
	};
	
	IDirect3DDevice9_SetTexture (_d3dev, 0, ctx->surface);
	
	float x = 1.0;
	if (ctx->filter_mode > 1)
	{
		x = 0.25*(ctx->mipmap_bias&0x3);
		if (ctx->filter_mode == 2)
			x = 1 - x;
	}
	float alpha[] = {1, 1, 1, x};
	IDirect3DDevice9_SetPixelShaderConstantF (_d3dev, 0, alpha, 1);

	float mode[] = {ctx->texture_mode, ctx->use_alpha, 0, 0};
	IDirect3DDevice9_SetPixelShaderConstantF (_d3dev, 1, mode, 1);
	
	if (KM_IGNORE == ctx->depth_mode)
	{
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_ZENABLE, FALSE);
	}
	else
	{
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_ZENABLE, TRUE);
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_ZFUNC, ctx->depth_mode + 1);
	}
	
	switch (ctx->cull_mode)
	{
	case KM_NOCULLING:
	case KM_CULLSMALL:
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_CULLMODE, D3DCULL_NONE);
		break;
	case KM_CULLCCW:
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_CULLMODE, D3DCULL_CCW);
		break;
	case KM_CULLCW:
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_CULLMODE, D3DCULL_CW);
		break;
	default:
		TRACE("%s : Bad culling param!", __func__);
		break;
	}
	
	switch (ctx->shade_mode)
	{
	case KM_NOTEXTUREFLAT:
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_SHADEMODE, D3DSHADE_FLAT);
		break;
	case KM_NOTEXTUREGOURAUD:
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
		break;
	case KM_TEXTUREFLAT:
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_SHADEMODE, D3DSHADE_FLAT);
		break;
	case KM_TEXTUREGOURAUD:
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
		break;
	default:
		TRACE("%s : Bad shading param!", __func__);
		break;
	}
	
	IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_ZWRITEENABLE, !ctx->depth_write);
	if (KM_DISPLAY_LIST_TRANS == ctx->list_type)
	{
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_ALPHABLENDENABLE, TRUE);
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_SRCBLEND, sblends[ctx->src_blend]);
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_DESTBLEND, dblends[ctx->dst_blend]);
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_SRCBLENDALPHA, sblends[ctx->src_blend]);
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_DESTBLENDALPHA, dblends[ctx->dst_blend]);
	}
	else
	{
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_ALPHABLENDENABLE, FALSE);
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
		IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
	}
	
	switch (ctx->clamp_uv)
	{
	case KM_CLAMP_V:
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		break;
	case KM_CLAMP_U:
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		break;
	case KM_CLAMP_UV:
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		break;
	default:
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		break;
	}
	
	switch (ctx->flip_uv)
	{
	case KM_FLIP_V:
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
		break;
	case KM_FLIP_U:
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
		break;
	case KM_FLIP_UV:
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
		break;
	default:
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		break;
	}
	
	switch (ctx->filter_mode)
	{
	default:
	case KM_POINT_SAMPLE:
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		break;
	case KM_BILINEAR:
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		break;
	case KM_TRILINEAR_A:
	case KM_TRILINEAR_B:
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState (_d3dev, D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		break;
	}
}
static int
Render (void)
{
	IDirect3DDevice9_Clear (
		_d3dev,
		0, NULL,
		D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 0),
		0,
		0
	);
	IDirect3DDevice9_BeginScene (_d3dev);

	IDirect3DVertexBuffer9 *vb = _buf;
	IDirect3DDevice9_SetStreamSource (_d3dev, 0, vb, 0, 36);
	
	void *buffer = NULL;
	IDirect3DVertexBuffer9_Lock (vb, 0, 0, (void **)&buffer, 0);
	memcpy (buffer, km.geobuf, km.geobuf_used);
	IDirect3DVertexBuffer9_Unlock (vb);
	
	for (int i = 0; i < KM_MAX_DISPLAY_LIST; i++)
	{
		KM_command_list *cl = km.cl + i;
		KM_polygon *head = NULL;
		size_t read = 0;
		
		TRACE("CL %i has %i bytes", i, cl->used);
		while (read < cl->used)
		{
			KM_command *cmd = (KM_command *)((char *)cl->buffer + read);
			switch (cmd->code)
			{
			case KMC_NOP:
					break;
			case KMC_POLY: {
					KM_polygon *ply = (KM_polygon *)cmd;
					if (i == 0)
					{
						apply_context (&ply->ctx);
						IDirect3DDevice9_DrawPrimitive (
							_d3dev,
							D3DPT_TRIANGLESTRIP,
							ply->offset,
							ply->nverts - 2
						);
					}
					else
					{
						KM_polygon *n = head;
						KM_polygon *prev = head;
						while (n != NULL)
						{
							if (n->sort <= ply->sort)
							{
								prev = n;
								n = n->next;
								continue;
							}
							ply->next = n->next;
							n->next = ply;
							goto Linked;
						}
						if (prev)
						{
							prev->next = ply;
							ply->next = NULL;
						}
					Linked:	
						if (!head) head = ply;
					}
				} break;
			default:
				TRACE("Unknown render code %i!!", cmd->code);
			}
			read += cmd->size;
		}
		while (head != NULL)
		{
			apply_context (&head->ctx);
			IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_ALPHABLENDENABLE, TRUE);
			IDirect3DDevice9_SetRenderState (_d3dev, D3DRS_ZWRITEENABLE, FALSE);
			IDirect3DDevice9_DrawPrimitive (
				_d3dev,
				D3DPT_TRIANGLESTRIP,
				head->offset,
				head->nverts - 2
			);
			head = head->next;
		}
		
		TRACE("Processed %u bytes of geometry", km.geobuf_used);
		cl->used = 0;
	}
	km.geobuf_used = 0;
	km.geobuf_indices = 0;
	IDirect3DDevice9_EndScene (_d3dev);
	return KM_OK;
}

/*Framebuffers*/
static int
FlipFrameBuffer (void)
{
	IDirect3DDevice9_Present (_d3dev, NULL, NULL, _win, NULL);
	return KM_OK;
}

/*Textures*/
static int
CreateTextureSurface (
	KM_surface_desc *surf,
	int width, int height,
	int type
) {
	D3DFORMAT d3df;
	d3df = D3DFMT_A8R8G8B8;
	DWORD mipmaps = 1;
	DWORD usage = 0;

	KM_shim *shim = (KM_shim *)surf->surface;
	IDirect3DDevice9_CreateTexture (
		_d3dev,
		surf->width, surf->height,
		mipmaps, usage,
		d3df,
		D3DPOOL_MANAGED,
		(IDirect3DTexture9 **)(&(shim->surface)),
		NULL
	);
	return KM_OK;
}
static int
LoadTexture (KM_surface_desc *surf, uint32_t width, uint32_t height, uint32_t bpp, void *data)
{
	D3DLOCKED_RECT rect;
	KM_shim *shim = (KM_shim *)surf->surface;
	IDirect3DTexture9_LockRect ((IDirect3DTexture9 *)shim->surface, 0, &rect, 0, 0);
	uint8_t *pix = (uint8_t *)rect.pBits;
	uint8_t *buf = (uint8_t *)data;
	uint32_t pitch = rect.Pitch>>2;
	if (4 == bpp)
	{
		for (uint32_t i = 0; i < height; i++)
		{
			for (uint32_t j = 0; j < width; j++)
			{
				uint32_t dst = 4*i*pitch + 4*j;
				uint32_t src = bpp*i*width + bpp*j;
				pix[dst + 0] = buf[src + 2];
				pix[dst + 1] = buf[src + 1];
				pix[dst + 2] = buf[src + 0];
				pix[dst + 3] = buf[src + 3];
			}
		}
	}
	else
	{
		for (uint32_t i = 0; i < height; i++)
		{
			for (uint32_t j = 0; j < width; j++)
			{
				uint32_t dst = 4*i*pitch + 4*j;
				uint32_t src = bpp*i*width + bpp*j;
				pix[dst + 0] = buf[src + 2];
				pix[dst + 1] = buf[src + 1];
				pix[dst + 2] = buf[src + 0];
				pix[dst + 3] = 0xff;
			}
		}
	}
	IDirect3DTexture9_UnlockRect ((IDirect3DTexture9 *)shim->surface, 0);
	return KM_OK;
}
static int
FreeTexture (KM_surface_desc *surf)
{
	if (surf->surface)
	{
		KM_shim *shim = surf->surface;
		IDirect3DTexture9_Release ((IDirect3DTexture9 *)shim->surface);
	}
	return KM_OK;
}

Backend backend_d3d9 =
{
	.tag = "d3d9",
	.init_device = InitDevice, 
	.destroy_device = DestroyDevice,
	
	.framebuffer_flip = FlipFrameBuffer,
	
	.pixel_clipping = SetPixelClipping,
	.render = Render,
	
	.texture_create = CreateTextureSurface,
	.texture_load = LoadTexture,
	.texture_free = FreeTexture
};
