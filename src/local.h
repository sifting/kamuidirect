#pragma once

#include <stdint.h>
#include "script.h"
#include "log.h"
#include "km.h"

#define VERIFY(x, y) assert ((x) && (y))

typedef struct _Backend
{
	const char *tag;
	int (*init_device) (int, void *, void *);
	int (*destroy_device) (void);
		
	int (*pixel_clipping) (int, int, int, int);
	int (*render) (void);	
	
	int (*framebuffer_flip) (void);
	
	int (*texture_create) (KM_surface_desc *, int, int, int);
	int (*texture_load) (KM_surface_desc *, uint32_t, uint32_t, uint32_t, void *);
	int (*texture_free) (KM_surface_desc *);

}Backend;

extern Backend backend_d3d9;

#define KMC_NOP		0
#define KMC_CTX		1
#define KMC_POLY	2

typedef struct _KM_command
{
	int code;
	size_t size;
}KM_command;
typedef struct _KM_context
{
	KM_command s;
	KM_vertex_ctx ctx;
}KM_context;
typedef struct _KM_polygon
{
	KM_command s;
	struct _KM_polygon *next;
	KM_vertex_ctx ctx;
	size_t offset;
	float sort;
	int nverts;
	int index;
}KM_polygon;

typedef struct _KM_shim
{
	uint32_t type;
	uint32_t format;
	uint32_t size;
	void *surface;
}KM_shim;

typedef struct _Config
{
	uint32_t loglevel;
	uint32_t framelimit;
	size_t vb_size;
	char backend[16];
}Config;

typedef struct _KM_command_list
{
	struct _KM_polygon *poly;
	void *buffer;
	size_t size;
	size_t used;
}KM_command_list;

typedef struct _KM_global_state
{
	int device; /*dreamcast or naomi mode*/
	int display, bpp, dither, aa;
	int width, height;
	int viewport[4];
	int rate;

	struct _Callbacks
	{
		KM_callback func;
		void *args;
	}vsync, eor, eov;

	float cull_value;

	size_t curr_list;
	void *base[KM_MAX_DISPLAY_LIST];
	void *lists[KM_MAX_DISPLAY_LIST];
	
	KM_command_list cl[KM_MAX_DISPLAY_LIST];
	void *geobuf;
	size_t geobuf_used;
	size_t geobuf_indices;

	KM_vertex_ctx ctx;
	void *texture;
	
	void *opaque;
	void *sorted;
	int opaque_w;
	int opaque_c;
	int opaque_used;
}KM_global_state;

extern KM_global_state km;
extern Log _log;
extern Config _cfg;

