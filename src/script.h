#pragma once

#define SCRIPT_EOF			0
#define SCRIPT_STRING		1
#define SCRIPT_SYMBOL		2

#define SCRIPT_OK			0
#define SCRIPT_NOSOURCE		-1
#define SCRIPT_TRUNCATED	-2

typedef struct _Script
{
	char *source;
	char *pos;
}Script;

int script_next (Script *script, int *type, char *token, size_t len);
int script_from_file (Script *script, const char *file);
int script_destroy (Script *script);
