#include "../game_q3/q_shared.h"
#include "../game_q3/botlib.h"
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
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

    botimport.Print(PRT_ERROR, "%s", text);
}
void Q_strncpyz( char *dest, const char *src, int destsize ) {
	strncpy( dest, src, destsize-1 );
    dest[destsize-1] = 0;
}
void Q_strcat( char *dest, int size, const char *src ) {
	int		l1;

	l1 = strlen( dest );
	if ( l1 >= size ) {
		Com_Error( ERR_FATAL, "Q_strcat: already overflowed" );
		return;	/* this Com_Error returns */
	}
	Q_strncpyz( dest + l1, src, size - l1 );
}
void QDECL Com_Printf (const char *msg, ...)
{
    va_list		argptr;
    char		text[1024];

    va_start (argptr, msg);
    Q_vsnprintf (text, sizeof(text), msg, argptr);
    va_end (argptr);

    botimport.Print(PRT_MESSAGE, "%s", text);
}
int COM_Compress( char *data_p ) {
	return strlen(data_p);
}
