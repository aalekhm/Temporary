
#include "vector.h"
#include <tchar.h>
#include <windows.h>

int assert_internal(const char* file, int line, const char* desc, ...)
{
	static WCHAR buffer[2048];
	static char text[512];
	
	va_list args;
	va_start(args, desc);
	vsprintf_s(text, sizeof(text), desc, args);
	swprintf_s(buffer, sizeof(buffer) / 2, L"file : '%S'\nline : %d\nmessage: '%S'\n", file, line, text);
	va_end(args);

	int ret = MessageBox(	NULL,	
							buffer,
							_T("ASSERT FAILED"),
							MB_ABORTRETRYIGNORE | MB_ICONWARNING | MB_APPLMODAL);
       
	if(ret == IDABORT)
	{
		__asm { int 3 }
		return 0;
	}
	if(ret == IDRETRY)
	{
		return 0;
	}
	if(ret == IDIGNORE)
	{
		return 1;
	}
	return 0;
}
