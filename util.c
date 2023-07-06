#include "util.h"


int my_toLower(int x)
{
	if (x >= 'A' && x <= 'Z')
	{
		return x + 32;
	}

	return x;
}

int my_wcsicmp(wchar_t const* s1, wchar_t const* s2)
{
	int i = 0;
	while (s1[i] != L'\0' && s2[i] != L'\0')
	{
		if (my_toLower(s1[i]) != my_toLower(s2[i]))
		{
			return my_toLower(s1[i]) - my_toLower(s2[i]);
		}
		i++;
	}

	return s1[i] - s2[i];
}

size_t my_strlen(const char* str)
{
	size_t len = 0;
	while (str[len] != '\0')
	{
		len++;
	}

	return len;
}