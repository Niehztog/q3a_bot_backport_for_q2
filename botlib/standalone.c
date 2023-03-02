#include "../qcommon_q3/q_shared.h"
#include "botlib.h"
extern botlib_import_t botimport;
void Com_Memset (void* dest, const int val, const size_t count)
{
	memset(dest, val, count);
}
void Com_Memcpy (void* dest, const void* src, const size_t count)
{
	memcpy(dest, src, count);
}
void QDECL Com_Error ( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

    botimport.Print(PRT_ERROR, "%s", text);
}
void Q_strncpyz( char *dest, const char *src, int destsize ) {
	strncpy( dest, src, destsize-1 );
    dest[destsize-1] = 0;
}
void QDECL Com_Printf (const char *msg, ...)
{
    va_list		argptr;
    char		text[1024];

    va_start (argptr, msg);
    vsprintf (text, msg, argptr);
    va_end (argptr);

    botimport.Print(PRT_MESSAGE, "%s", text);
}
int COM_Compress( char *data_p ) {
	return strlen(data_p);
}
