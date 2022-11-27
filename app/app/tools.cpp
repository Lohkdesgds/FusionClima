#include "tools.h"

bool __log_full(bool* b)
{
	static bool persis = false;
	if (b) persis = *b;
	return persis;
}

bool make_log_full(bool o)
{
	return __log_full(&o);
}

bool log_full()
{
	return __log_full();
}