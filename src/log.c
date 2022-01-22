#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "log.h"

int
log_create (Log *ls, const char *name)
{
	memset (ls, 0, sizeof (*ls));
	ls->level = 10;
	ls->fp = fopen (name, "w");
	if (NULL == ls->fp)
	{
		return -1;
	}
	setbuf (ls->fp, NULL);
	return 0;
}
int
log_destroy (Log *ls)
{
	if (ls->fp)
	{
		fclose (ls->fp);
		ls->fp = NULL;
	}
	return 0;
}
int
log_write (Log *ls, int level, const char *fmt, ...)
{
	if (ls->level < level)
	{
		return -1;
	}
	FILE *fp = ls->fp ? ls->fp : stdout;
	va_list args;
	
	va_start (args, fmt);
	vfprintf (fp, fmt, args);
	fprintf (fp, "\r\n");
	va_end (args);
	return 0;
}
