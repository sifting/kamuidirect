#pragma once

typedef struct _Log
{
	FILE *fp;
	int level;
}Log;

int log_create (Log *ls, const char *name);
int log_destroy (Log *ls);
int log_write (Log *ls, int level, const char *fmt, ...);

#define TRACE(...) log_write (&_log, 2, __VA_ARGS__)
#define CRITICAL(...) log_write (&_log, 1, "CRITICAL: " __VA_ARGS__)

