#pragma once

/*Calling convention*/
#define KMAPI __stdcall

#define PVR_TWIDDLED    				0x1
#define PVR_TWIDDLED_MIPMAP 			0x2
#define PVR_TWIDDLED_MIPMAP_DMA			0x12 /*Has padding at the end*/
#define PVR_VQ 							0x3
#define PVR_VQ_MIPMAP 					0x4
#define PVR_RECTANGLE           		0x9
#define PVR_RECTANGLE_MIPMAP			0xA
#define PVR_RECTANGLE_TWIDDLED			0xD
#define PVR_SMALL_VQ 					0x10
#define PVR_SMALL_VQ_MIPMAP				0x11
#define PVR_STRIDE 						0xB
#define PVR_STRIDE_TWIDDLED 			0xC
#define PVR_BITMAP						0xE
#define PVR_BITMAP_MIPMAP				0xF
#define PVR_PAL4 						0x5
#define PVR_PAL4_MIPMAP 				0x6
#define PVR_PAL8 						0x7
#define PVR_PAL8_MIPMAP					0x8

/*Kamui constants*/
#define KM_OK							0
#define KM_BAD_VIDEO					2
#define KM_NO_MEMORY					3
#define KM_NO_HW						8
#define KM_BAD_PARAM					0x0ddddL
#define KM_ERROR						0x0eeeeL

#define KM_DREAMCAST					0x000000
#define KM_NAOMI						0x010000

#define KM_NTSC							0x00
#define KM_VGA							0x01
#define KM_PAL 							0x02
#define KM_INTERLACE					0x04
#define KM_PSEUDONONINTERLACE			0x08
#define KM_WIDTH_640					0x10
#define KM_HEIGHT_480					0x20

#define KM_RGB565						0
#define KM_RGB555						1
#define KM_ARGB1555						3
#define KM_RGB888						4
#define KM_ARGB8888						5

#define KM_TEXTURE_ARGB1555             0x00
#define KM_TEXTURE_RGB565               0x01
#define KM_TEXTURE_ARGB4444             0x02
#define KM_TEXTURE_YUV422               0x03
#define KM_TEXTURE_BUMP                 0x04
#define KM_TEXTURE_RGB555               0x05 	/* for PCX compatible only.	*/
#define KM_TEXTURE_YUV420               0x06 	/* for YUV converter */

#define KM_PIXELFORMAT_ARGB1555			0x00000000
#define KM_PIXELFORMAT_RGB565			0x08000000
#define KM_PIXELFORMAT_ARGB4444			0x10000000
#define KM_PIXELFORMAT_YUV422			0x18000000
#define KM_PIXELFORMAT_BUMP				0x20000000
#define KM_PIXELFORMAT_PALETTIZED_4BPP	0x28000000
#define KM_PIXELFORMAT_PALETTIZED_8BPP	0x30000000

#define KM_SURFACEFLAGS_MIPMAPED		0x80000000
#define KM_SURFACEFLAGS_VQ				0x40000000
#define KM_SURFACEFLAGS_NOTWIDDLED		0x04000000
#define KM_SURFACEFLAGS_TWIDDLED		0x00000000
#define KM_SURFACEFLAGS_STRIDE			0x02000000
#define KM_SURFACEFLAGS_PALETTIZED		0x00008000

#define KM_SURFACEFLAGS_VSIZE8			0x00000000
#define KM_SURFACEFLAGS_VSIZE16			0x00000001
#define KM_SURFACEFLAGS_VSIZE32			0x00000002
#define KM_SURFACEFLAGS_VSIZE64			0x00000003
#define KM_SURFACEFLAGS_VSIZE128		0x00000004
#define KM_SURFACEFLAGS_VSIZE256		0x00000005
#define KM_SURFACEFLAGS_VSIZE512		0x00000006
#define KM_SURFACEFLAGS_VSIZE1024		0x00000007

#define KM_SURFACEFLAGS_USIZE8			0x00000000
#define KM_SURFACEFLAGS_USIZE16			0x00000008
#define KM_SURFACEFLAGS_USIZE32			0x00000010
#define KM_SURFACEFLAGS_USIZE64			0x00000018
#define KM_SURFACEFLAGS_USIZE128		0x00000020
#define KM_SURFACEFLAGS_USIZE256		0x00000028
#define KM_SURFACEFLAGS_USIZE512		0x00000030
#define KM_SURFACEFLAGS_USIZE1024		0x00000038

#define KM_NORMAL						0xE0000000
#define KM_ENDOFSTRIP					0xF0000000

#define KM_MIPMAP_D_ADJUST_0_25			0x00000001
#define KM_MIPMAP_D_ADJUST_0_50			0x00000002
#define KM_MIPMAP_D_ADJUST_0_75			0x00000003
#define KM_MIPMAP_D_ADJUST_1_00			0x00000004
#define KM_MIPMAP_D_ADJUST_1_25			0x00000005
#define KM_MIPMAP_D_ADJUST_1_50			0x00000006
#define KM_MIPMAP_D_ADJUST_1_75			0x00000007
#define KM_MIPMAP_D_ADJUST_2_00			0x00000008
#define KM_MIPMAP_D_ADJUST_2_25			0x00000009
#define KM_MIPMAP_D_ADJUST_2_50			0x0000000A
#define KM_MIPMAP_D_ADJUST_2_75			0x0000000B
#define KM_MIPMAP_D_ADJUST_3_00			0x0000000C
#define KM_MIPMAP_D_ADJUST_3_25			0x0000000D
#define KM_MIPMAP_D_ADJUST_3_50			0x0000000E
#define KM_MIPMAP_D_ADJUST_3_75			0x0000000F

#define KM_PARAM_POLYGON   				0x04
#define KM_PARAM_MODIFIERVOLUME			0x04
#define KM_PARAM_SPRITE					0x05

#define	PARAMTYPE						0x00100000
#define	KM_LISTTYPE						0x00200000
#define	KM_STRIPLENGTH					0x08000000
#define	KM_USERCLIPMODE					0x10000000
#define	KM_COLORTYPE					0x00400000
#define	KM_UVFORMAT						0x00800000
#define	KM_DEPTHMODE					0x00000001
#define	KM_CULLINGMODE					0x00000002
#define	KM_SCREENCOORDINATION			0x00000004
#define	KM_SHADINGMODE					0x00000008
#define	KM_MODIFIER						0x00000010
#define	KM_ZWRITEDISABLE				0x00000020
#define	KM_SRCBLENDINGMODE				0x00000040
#define	KM_DSTBLENDINGMODE				0x00000080
#define	KM_SRCSELECT					0x01000000
#define	KM_DSTSELECT					0x02000000
#define	KM_FOGMODE						0x00000100
#define	KM_USESPECULAR					0x00000200
#define	KM_USEALPHA						0x00000400
#define	KM_IGNORETEXTUREALPHA			0x00000800
#define	KM_CLAMPUV						0x00001000
#define	KM_FLIPUV						0x00002000
#define	KM_FILTERMODE					0x00004000
#define	KM_SUPERSAMPLE					0x00008000
#define	KM_MIPMAPDADJUST				0x00010000
#define	KM_TEXTURESHADINGMODE			0x00020000
#define	KM_COLORCLAMP					0x00040000
#define	KM_PALETTEBANK					0x00080000
#define	KM_DCALCEXACT					0x04000000 


#define	KM_MODIFIER_NORMAL_POLY			0
#define	KM_MODIFIER_INCLUDE_FIRST_POLY	KM_MODIFIER_NORMAL_POLY
#define	KM_MODIFIER_EXCLUDE_FIRST_POLY	KM_MODIFIER_NORMAL_POLY
#define	KM_MODIFIER_INCLUDE_LAST_POLY	(1 << 29)
#define	KM_MODIFIER_EXCLUDE_LAST_POLY	(2 << 29)

#define KM_MAXVTF						18

#define KM_DISPLAY_LIST_OPAQUE 			0
#define KM_DISPLAY_LIST_OPAQUE_MOD		1
#define KM_DISPLAY_LIST_TRANS			2
#define KM_DISPLAY_LIST_TRANS_MOD		3
#define KM_DISPLAY_LIST_RESV			4
#define KM_MAX_DISPLAY_LIST				5

#define KM_POLYGON						0
#define KM_MODIFIERVOLUME				1
#define KM_SPRITE						2

#define KM_NORMAL_POLYGON				0
#define KM_CHEAPSHADOW_POLYGON			1

#define KM_PACKEDCOLOR					0
#define KM_FLOATINGCOLOR				1
#define KM_INTENSITY					2
#define KM_INTENSITY_PREV_FACE_COL		3

#define KM_32BITUV						0
#define KM_16BITUV						1

#define KM_NOMODIFIER					0
#define KM_MODIFIER_A					1

#define KM_IGNORE						0
#define KM_LESS							1
#define KM_EQUAL						2
#define KM_LESSEQUAL					3
#define KM_GREATER						4
#define KM_NOTEQUAL						5
#define KM_GREATEREQUAL					6
#define KM_ALWAYS						7

#define KM_NOCULLING					0
#define KM_CULLSMALL					1
#define KM_CULLCCW						2
#define KM_CULLCW						3

#define KM_NOTEXTUREFLAT				0
#define KM_NOTEXTUREGOURAUD				1
#define KM_TEXTUREFLAT					2
#define KM_TEXTUREGOURAUD				3

#define KM_FOGTABLE						0
#define KM_FOGVERTEX					1
#define KM_NOFOG						2
#define KM_FOGTABLE_2					3

#define KM_BOTHINVSRCALPHA				0
#define KM_BOTHSRCALPHA					1
#define KM_DESTALPHA					2
#define KM_DESTCOLOR					3
#define KM_INVDESTALPHA					4
#define KM_INVDESTCOLOR					5
#define KM_INVSRCALPHA					6
#define KM_INVSRCCOLOR					7
#define KM_SRCALPHA						8
#define KM_SRCCOLOR						9
#define KM_ONE							10
#define KM_ZERO							11

#define KM_NOCLAMP						0
#define KM_CLAMP_V						1
#define KM_CLAMP_U						2
#define KM_CLAMP_UV						3

#define KM_NOFLIP						0
#define KM_FLIP_V						1
#define KM_FLIP_U						2
#define KM_FLIP_UV						3

#define KM_POINT_SAMPLE					0
#define KM_BILINEAR						1
#define KM_TRILINEAR_A					2
#define KM_TRILINEAR_B					3

#define KM_DECAL						0
#define KM_MODULATE						1
#define KM_DECAL_ALPHA					2
#define KM_MODULATE_ALPHA				3

typedef void (*KM_callback) (void *args);
typedef struct _KM_vertex_buffers
{
	int lists[KM_MAX_DISPLAY_LIST];
}KM_vertex_buffers;

typedef struct _KM_pass_info
{
	int flags;
	int *vertex_buffer;
	int vertex_size;
	int txlist;
	float buffer_size[KM_MAX_DISPLAY_LIST];
	int opb_size[KM_MAX_DISPLAY_LIST];
}KM_pass_info;

typedef struct _KM_list_state
{
	int type;
	int size;
	int vertex_type;
	int vertex_bank;
	int curr_pass;
	int max_pass;
	int flags;
}KM_list_state;

typedef struct _KM_vertex_desc
{
	int **curr;
	int *param;
	KM_list_state *curr_list;
	KM_vertex_buffers **buffer;
	int size;
	KM_pass_info *passinfo;
	int resv;
}KM_vertex_desc;

typedef struct _KM_vertex_ctx
{
	int state;	
	int param_type;
	int list_type;
	int color_type;
	int uv_type;
	int depth_mode;
	int cull_mode;
	int resv0;
	int shade_mode;
	int modifier;
	int depth_write;
	int src_blend;
	int dst_blend;
	int src_select;
	int dst_select;
	int fog_mode;
	int use_specular;
	int use_alpha;
	int no_attribute;
	int clamp_uv;
	int flip_uv;
	int filter_mode;
	int super_sample;
	int mipmap_bias;
	int texture_mode;
	int clamp_color;
	int palette;
	void *surface;
	float a, r, g, b;
	float sa, sr, sg, sb;
	int param;
	int isp;
	int tsp;
	int tparam;
	int modifer_inst;
	float r0, r1, r2, r3;
	int exact;
	int resv1;
	int user_clip;
}KM_vertex_ctx;

typedef struct _KM_surface_desc
{
	int type;
	int depth;
	int format;
	int width, height;
	int size;
	int flags;
	void *surface;
}KM_surface_desc;

/*Misc state*/
KMAPI int kmInitDevice (int device);
KMAPI int kmSetDisplayMode (int mode, int bpp, int dither, int aa);
KMAPI int kmSetWaitVsyncCount (int count);
KMAPI int kmSetCullingRegister (float value);
KMAPI int kmSetPixelClipping (int xmin, int ymin, int xmax, int ymax);
KMAPI int kmRender (void);

/*Framebuffers*/
KMAPI int kmCreateFrameBufferSurface (
	KM_surface_desc *surf,
	int width, int height,
	int stripbuffer,
	int bufferclear
);
KMAPI int kmActivateFrameBuffer (
	KM_surface_desc *front,
	KM_surface_desc *back,
	int stripbuffer,
	int vsync
);
KMAPI int kmFlipFrameBuffer (void);

/*Callbacks*/
KMAPI int kmSetVSyncCallback (KM_callback func, void *args);
KMAPI int kmSetEORCallback (KM_callback func, void *args);
KMAPI int kmSetEndOfVertexCallback (KM_callback func, void *args);

/*Vertices*/
KMAPI int kmCreateVertexBuffer (
	KM_vertex_desc *desc,
	int opaque_buffer,
	int opaque_modifier,
	int trans_buffer,
	int trans_modifier
);
KMAPI int kmSetVertexRenderState (KM_vertex_ctx *ctx);
KMAPI int kmProcessVertexRenderState (KM_vertex_ctx *ctx);
KMAPI int kmDiscardVertexBuffer (KM_vertex_desc *desc);
KMAPI int kmStartVertexStrip (KM_vertex_desc *desc);
KMAPI int kmSetVertex (KM_vertex_desc *desc, void *v, int type, int size);

/*Textures*/
KMAPI int kmCreateTextureSurface (
	KM_surface_desc *surf,
	int width, int height,
	int type
);
KMAPI int kmLoadTexture (KM_surface_desc *surf, void *data, int unk0, int unk1);
KMAPI int kmFreeTexture (KM_surface_desc *surf);
KMAPI int kmReLoadMipmap (KM_surface_desc *surf, void *texture, int count);
KMAPI int kmGetFreeTextureMem (int *size, int *block);
KMAPI int kmSetFramebufferTexture (KM_surface_desc *surface);

/*Modifier Volumes*/
KMAPI int kmSetModifierRenderState (KM_vertex_desc *desc, KM_vertex_ctx *ctx);
KMAPI int kmUseAnotherModifier (int modifier);

/*Background Planes*/
KMAPI int kmSetBackGroundPlane (void *v[3], int type, int size);
KMAPI int kmSetBackGroundRenderState (KM_vertex_ctx *ctx);

KMAPI int kmSetFogTable (void *);
KMAPI int kmSetFogDensity (void *);
KMAPI int kmSetFogTableColor (void *);
