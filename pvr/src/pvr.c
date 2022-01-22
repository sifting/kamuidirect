#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include "pvr.h"

#define PVR_GBIX 						(('X'<<24)|('I'<<16)|('B'<<8)|'G')
#define PVR_PVRT 						(('T'<<24)|('R'<<16)|('V'<<8)|'P')
#define PVR_CODEBOOK					0x1000
#define PVR_MAX_WIDTH					0x1000
#define PVR_MAX_HEIGHT					0x1000

#define PVR_ARGB1555					0x0
#define PVR_RGB565						0x1
#define PVR_ARGB4444					0x2
#define PVR_YUV422						0x3
#define PVR_BUMP						0x4
#define PVR_RGB555						0x5
#define PVR_YUV420						0x6

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

typedef struct _PVR_header
{
	uint32_t magick;
	uint32_t size;
	uint8_t format;
	uint8_t type;
	uint16_t unk;
	uint16_t width;
	uint16_t height;
}PVR_header;

static uint32_t
morton (uint32_t x, uint32_t y)
{
	x = (x|(x<<8))&0x00ff00ff;
	y = (y|(y<<8))&0x00ff00ff;
	x = (x|(x<<4))&0x0f0f0f0f;
	y = (y|(y<<4))&0x0f0f0f0f;
	x = (x|(x<<2))&0x33333333;
	y = (y|(y<<2))&0x33333333;
	x = (x|(x<<1))&0x55555555;
	y = (y|(y<<1))&0x55555555;
	return x|(y<<1);
}

static uint32_t
unpack1555 (uint32_t colour)
{
	uint32_t a = (uint32_t)(255*((colour>>15)&31));
	uint32_t r = (uint32_t)(255*((colour>>10)&31)/31.0);
	uint32_t g = (uint32_t)(255*((colour>> 5)&31)/31.0);
	uint32_t b = (uint32_t)(255*((colour    )&31)/31.0);
	return (r<<24)|(g<<16)|(b<<8)|a;
}

static uint32_t
unpack4444 (uint32_t colour)
{
	uint32_t a = (uint32_t)(255*((colour>>12)&15)/15.0);
	uint32_t r = (uint32_t)(255*((colour>> 8)&15)/15.0);
	uint32_t g = (uint32_t)(255*((colour>> 4)&15)/15.0);
	uint32_t b = (uint32_t)(255*((colour    )&15)/15.0);
	return (r<<24)|(g<<16)|(b<<8)|a;
}
static uint32_t
unpack565 (uint32_t colour)
{
	uint32_t r = (uint32_t)(255*((colour>>11)&31)/31.0);
	uint32_t g = (uint32_t)(255*((colour>> 5)&63)/63.0);
	uint32_t b = (uint32_t)(255*((colour    )&31)/31.0);
	return (r<<24)|(g<<16)|(b<<8)|0xff;
}

static void
write_24 (uint8_t **buf, uint32_t px)
{
	uint8_t *pix = *buf;
	pix[0] = (px>>24)&0xff;
	pix[1] = (px>>16)&0xff;
	pix[2] = (px>> 8)&0xff;
	pix += 3;
	*buf = pix; 
}
static void
write_32 (uint8_t **buf, uint32_t px)
{
	uint8_t *pix = *buf;
	pix[0] = (px>>24)&0xff;
	pix[1] = (px>>16)&0xff;
	pix[2] = (px>> 8)&0xff;
	pix[3] = (px    )&0xff; 
	pix += 4;
	*buf = pix;
}

int
pvr_surface_decode_ptr (
	PVR_surface *dst,
	uint32_t type,
	uint32_t format,
	uint32_t width,
	uint32_t height,
	size_t size,
	uint8_t *src
){
	/*Set up I/O*/
	uint32_t (*decoder) (uint32_t);
	void (*writer) (uint8_t **, uint32_t);
	uint32_t scalar = 0;
	switch (format)
	{
	case PVR_ARGB1555:
		decoder = unpack1555;
		writer = write_32;
		scalar = 4;
		break;
	case PVR_ARGB4444:
		decoder = unpack4444;
		writer = write_32;
		scalar = 4;
		break;
	case PVR_RGB565:
		decoder = unpack565;
		writer = write_24;
		scalar = 3;
		break;
	default:
		return PVR_NO_IMP;
	}

	/*Fill out the surface*/
	dst->data = malloc (scalar*width*height);
	if (NULL == dst->data)
	{
		return PVR_NO_MEM;
	}
	dst->width = width;
	dst->height = height;
	dst->bpp = scalar;

	/*Decode the image data...*/
	switch (type)
	{
	case PVR_VQ:
	case PVR_VQ_MIPMAP:
	case PVR_SMALL_VQ:
	case PVR_SMALL_VQ_MIPMAP: {
			uint32_t hw = width/2;
			uint32_t hh = height/2;
			uint32_t pitch = scalar*width;

			/*Extract VQ codebook*/
			uint16_t codebook[PVR_CODEBOOK];
			memcpy (codebook, src, sizeof (codebook));

			/*Offset to highest resolution mipmap*/
			uint8_t *pixels = src + size - hw*hh;
			
			/*Data is morton coded indices into the 2x2 16bit pixel codebook*/
			uint8_t *row0 = (uint8_t *)dst->data;
			uint8_t *row1 = (uint8_t *)dst->data + pitch;
			for (uint32_t i = 0; i < hh; i++)
			{
				for (uint32_t j = 0; j < hw; j++)
				{
					uint32_t entry = 4*pixels[morton (i, j)];
					writer (&row0, decoder (codebook[entry + 0]));
					writer (&row1, decoder (codebook[entry + 1]));
					writer (&row0, decoder (codebook[entry + 2]));
					writer (&row1, decoder (codebook[entry + 3]));
				}
				row0 += pitch;
				row1 += pitch;
			}
		} break;
	case PVR_TWIDDLED:
	case PVR_TWIDDLED_MIPMAP:
	case PVR_TWIDDLED_MIPMAP_DMA:
	case PVR_RECTANGLE_TWIDDLED: {
			/*Offset to highest resolution mipmap*/
			uint16_t *pixels = (uint16_t *)(src + size - 2*width*height);
			uint8_t *row = (uint8_t *)dst->data;
			for (uint32_t i = 0; i < height; i++)
			{
				for (uint32_t j = 0; j < width; j++)
				{
					uint32_t colour = decoder (pixels[morton (i, j)]);
					writer (&row, colour);
				}
			}
		} break;
	case PVR_RECTANGLE:
	case PVR_RECTANGLE_MIPMAP: {
			/*Offset to highest resolution mipmap*/
			uint16_t *pixels = (uint16_t *)(src + size - 2*width*height);
			uint8_t *row = (uint8_t *)dst->data;
			for (uint32_t i = 0; i < height; i++)
			{
				for (uint32_t j = 0; j < width; j++)
				{
					uint32_t index = i*width + j;
					uint32_t colour = decoder (pixels[index]);
					writer (&row, colour);
				}
			}
		} break;
	default:
		return PVR_NO_IMP;
	}
	return PVR_OK;
}

int
pvr_surface_decode (PVR_surface *dst, uint8_t *src)
{
	assert (sizeof (PVR_header) == 16 && "PVR_header is not 16 bytes!");
	uint8_t *p = src;

	/*Ensure data is (likely) a PVR texture*/
	uint32_t magick = *(uint32_t *)p;
	if (PVR_GBIX != magick && PVR_PVRT != magick)
	{
		return PVR_INVALID;
	}

	/*Skip gbix header*/
	if (PVR_GBIX == magick)
	{
		p += 12;
	}

	/*Valid PVR header*/
	PVR_header h = *(PVR_header *)p;
	if (PVR_PVRT != h.magick)
	{
		return PVR_INVALID;
	}
	if (PVR_MAX_WIDTH < h.width)
	{
		return PVR_BAD_SIZE;
	}
	if (PVR_MAX_HEIGHT < h.height)
	{
		return PVR_BAD_SIZE;
	}
	p += sizeof (h);
	
	/*Decode the data proper*/
	return pvr_surface_decode_ptr (
		dst,
		h.type, h.format,
		h.width, h.height,
		h.size - 8, p
	);
}
int
pvr_surface_free (PVR_surface *surf)
{
	assert (surf != NULL && "surf is NULL!");
	if (surf->data)
	{
		free (surf->data);
		surf->data = NULL;
	}
	return PVR_OK;
}
