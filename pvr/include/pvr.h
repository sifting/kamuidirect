#pragma once

#include <stdint.h>

#define PVR_OK							0
#define PVR_INVALID						-1
#define PVR_BAD_SIZE					-2
#define PVR_NO_IMP						-3
#define PVR_NO_MEM						-4

typedef struct _PVR_surface
{
	void *data;
	uint32_t width, height;
	uint32_t bpp; /*bytes per pixel*/
}PVR_surface;

int
pvr_surface_decode_ptr (
	PVR_surface *dst,
	uint32_t type,
	uint32_t format,
	uint32_t width,
	uint32_t height,
	size_t size,
	uint8_t *src
);
int pvr_surface_decode (PVR_surface *dst, uint8_t *src);
int pvr_surface_free (PVR_surface *surf);
