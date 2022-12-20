#pragma once
#define WIN32_LEAN_AND_MEAN

#include <cpp-httplib/httplib.h>
#include <time.h>

const std::string CURRDATE = __TIMESTAMP__;

#ifdef _WIN32
#define GMTIM(A,B) localtime_s(B,A)
#define BADGM(X) (X != 0)
#else
#define GMTIM(A,B) localtime_r(A,B)
#define BADGM(X) (!X)
#endif

#define MAKEDAY(onfail) \
tm tm;\
time_t t = time(0);\
if (BADGM(GMTIM(&t, &tm))) {\
	return onfail;\
}

bool __log_full(bool* = nullptr);
bool make_log_full(bool);
bool log_full();