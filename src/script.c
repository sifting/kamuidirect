#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "script.h"

int
script_next (Script *script, int *type, char *token, size_t len)
{
	char *p = script->pos;
	char *d = token;
	int code = SCRIPT_NOSOURCE;
	len--;
	while (p != NULL)
	{	/*Skip leading white space*/
		while (*p <= ' ')
		{
			if (*p == '\0')
			{
				*type = SCRIPT_EOF;
				code = SCRIPT_OK;
				goto Exit;
			}
			p++;
		}
		/*Single line comments*/
		if ('#' == *p)
		{
			while (*p != '\n')
			{
				if (*p == '\0') break;
				p++;
			}
			continue;
		}
		/*Strings*/
		if ('\"' == *p)
		{
			while (*++p != '\"')
			{
				if (len != 0)
				{
					*d++ = *p;
					*d = '\0';
					len--;
				}				
			}
			p++;
			*type = SCRIPT_STRING;
			code = (len != 0) ? SCRIPT_OK : SCRIPT_TRUNCATED;
			goto Exit;
		}
		/*Symbols*/
		while (*p > ' ')
		{
			if (len != 0)
			{
				int c = *p;
				*d++ = c|((c != '_')<<5);
				*d = '\0';
				len--;
			}
			p++;
		}
		*type = SCRIPT_SYMBOL;
		code = (len != 0) ? SCRIPT_OK : SCRIPT_TRUNCATED;
		goto Exit;
	}
Exit:
	script->pos = p;
	return code;
}
int
script_from_file (Script *script, const char *file)
{
	char *text = NULL;
	size_t len = 0;
	
	FILE *fp = fopen (file, "rb");
	if (NULL == fp)
	{
		return -1;
	}
	
	fseek (fp, 0, SEEK_END);
	len = ftell (fp);
	fseek (fp, 0, SEEK_SET);
	
	text = malloc (len + 1);
	if (NULL == text)
	{
		fclose (fp);
		return -2;
	}
	fread (text, len, 1, fp);
	text[len] = '\0';
	
	fclose (fp);
	
	memset (script, 0, sizeof (*script));
	script->source = text;
	script->pos = text;
	
	return 0;
}
int
script_destroy (Script *script)
{
	if (script->source)
	{
		free (script->source);
		script->source = NULL;
		script->pos = NULL;
	}
	return 0;
}

