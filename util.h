#include <stdio.h>
#define NO_INLINE  __attribute__ ((noinline))

int my_toLower(int x);
size_t my_strlen(const char* str);
int my_wcsicmp(wchar_t const* s1, wchar_t const* s2);
unsigned char HChar(unsigned char C );
wchar_t HWChar(wchar_t C );
