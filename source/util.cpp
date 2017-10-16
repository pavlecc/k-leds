#include <time.h>
#include <math.h>
#include "util.h"

unsigned long long Util::GetCurrentMilis()
{
   	struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    return (unsigned long long)llround(spec.tv_sec * 1000 + spec.tv_nsec / 1.0e6);
}

const Vec3 Vec3::cZero = Vec3(0.0f, 0.0f, 0.0f);
