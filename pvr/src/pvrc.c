#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <png.h>
#include "pvr.h"

static jmp_buf jmpbuf;

static void
error_handler (png_structp png_ptr, png_const_charp msg)
{
    longjmp (jmpbuf, 1);
}

int
main (int argc, char **argv)
{
	if (argc < 2)
	{
		printf ("prvc <image>\n");
		return -1;
	}

	FILE *fp = fopen (argv[1], "rb");
	if (NULL == fp)
	{
		printf ("failed to open '%s'\n", argv[1]);
		return -1;
	}
	fseek (fp, 0, SEEK_END);
	int size = ftell (fp);
	uint8_t *buf = malloc (size);
	fseek (fp, 0, SEEK_SET);
	fread (buf, 1, size, fp);
	fclose (fp);

	PVR_surface srf;
	switch (pvr_surface_decode (&srf, buf))
	{
	case PVR_OK: {
			printf ("saving...\n");
			FILE *outfile2 = fopen ("out.raw", "wb");
			fwrite (srf.data, 1, srf.width*srf.height*srf.bpp, outfile2);
			fclose (outfile2);

			/*Create output stream*/
			FILE *outfile = fopen ("out.png", "wb");
			if (NULL == outfile)
			{
				pvr_surface_free (&srf);
				return -1;
			}

			/*Initialise libpng*/
		    png_structp png_ptr = png_create_write_struct (
		    	PNG_LIBPNG_VER_STRING,
		    	NULL,
        		error_handler,
        		NULL
        	);
        	if (NULL == png_ptr)
        	{
        		pvr_surface_free (&srf);
        		fclose (fp);
        		return -1;
        	}
        	png_infop info_ptr = png_create_info_struct (png_ptr);
		    if (!info_ptr)
		    {
		       png_destroy_write_struct (&png_ptr, NULL);
		       pvr_surface_free (&srf);
		       fclose (fp);
		       return -1;
		    }

		    /*Abort! Abort! Abort!*/
			if (setjmp (png_jmpbuf (png_ptr)))
		    {
				png_destroy_write_struct (&png_ptr, &info_ptr);
				pvr_surface_free (&srf);
				fclose (fp);
				return -1;
		    }

		    png_init_io (png_ptr, fp);
			png_set_IHDR (
				png_ptr, info_ptr,
				srf.width, srf.height, 8,
      			(srf.bpp == 4) ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
      			PNG_INTERLACE_NONE,
    			PNG_COMPRESSION_TYPE_DEFAULT,
    			PNG_FILTER_TYPE_DEFAULT
    		);

			png_write_info (png_ptr, info_ptr);
			png_set_packing (png_ptr);

			uint8_t *buf = srf.data;
			for (uint32_t i = 0; i < srf.height; i++)
			{
				png_write_row (png_ptr, buf);
				buf += srf.bpp*srf.width;
			}
			png_write_end (png_ptr, NULL);
			
			png_destroy_write_struct (&png_ptr, &info_ptr);
			pvr_surface_free (&srf);
			fclose (fp);
		} break;
	case PVR_BAD_SIZE:
		printf ("width/height must be <= 1024\n");
		break;
	case PVR_NO_IMP:
		printf ("unsupported PVR format\n");
		break;
	case PVR_NO_MEM:
		printf ("no memory left to allocate surface\n");
		break;
	case PVR_INVALID:
		printf ("invalid image format\n");
	default:
		break;
	}
	return 0;
}
