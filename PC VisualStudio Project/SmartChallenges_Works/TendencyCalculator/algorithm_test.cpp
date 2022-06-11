#include "algorithm_test.h"

//#define TESTA
#define TESTB


double chances_of(const pairing8& hist, const double(&Ks)[num_ks])
{
    const auto limfact = [](const double f) -> double { return f > 1.0 ? 1.0 : (f < 0.0 ? 0.0 : f); };
    const auto limpos = [](const double f) -> double { return f > 0.0 ? f : 0.0; };
#ifdef TESTB
    return (
        limfact(limpos(hist.get(1).temp - hist.get(0).temp) * limpos(hist.get(1).umid - hist.get(0).umid) * Ks[0]) +
        limfact(limpos(hist.get(1).temp - hist.get(0).temp) * Ks[1]) +
        limfact(limpos(hist.get(1).umid - hist.get(0).umid) * Ks[1])
        ) / 3.0;
#endif

#ifdef TESTA
    return (
        limfact(limpos(hist.get(1).temp - hist.get(0).temp) * limpos(hist.get(1).umid - hist.get(0).umid) * Ks[0]) +
        limfact(limpos(hist.get(1).temp - hist.get(0).temp) * Ks[1]) +
        limfact(limpos(hist.get(1).umid - hist.get(0).umid) * Ks[1])
    ) / 3.0;
#endif

    //return limfact(pow(0.3 * limfact(hist.get(0).umid > 0.95 ? ((hist.get(0).umid - 0.92) / 0.08) : 0.0) +
    //    0.7 * limfact(
    //        limfact(hist.get(1).temp < hist.get(0).temp ? ((hist.get(0).temp - hist.get(1).temp) * 1.0 / 1.5) : 0.0) * 0.7 +
    //        limfact(hist.get(0).umid > 0.8 ? ((hist.get(0).umid - 0.8) / 0.2) : 0.0) * 0.3) +
    //    0.4 * limfact(
    //        0.4 * (hist.get(0).umid >= hist.get(1).umid) + 0.25 * (hist.get(1).umid >= hist.get(2).umid) + 0.15 * (hist.get(2).umid >= hist.get(3).umid) +
    //        0.2 * (hist.get(0).temp <= hist.get(1).temp) + 0.15 * (hist.get(1).temp <= hist.get(2).temp) + 0.10 * (hist.get(2).temp <= hist.get(3).temp)
    //    ), 0.8));
}