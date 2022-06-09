#include "benchmark.h"

void do_benchmark()
{
    mprint("= = = = = = = = = = = = = = = = = = = = Tasking @ %d...\n", xPortGetCoreID());
    char buf[256];
    for(size_t p = 0; p < 3000; ++p) {
        size_t cpy = p;
        for(int a = 255; a != 1; --a) {
            buf[a] = (cpy % 15) + '0';
            cpy /= 3;
            buf[a-1] = powf(cpy, 2.543f) * 1.03f - 4.f;
            buf[rand()%256] = buf[rand()%256];
        }
    }
    mprint("= = = = = = = = = = = = = = = = = = = = End task @ %d\n", xPortGetCoreID());
}