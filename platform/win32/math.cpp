#include <math.h>
#include <stdlib.h>

extern "C" float  MathRound(float x) {
    return roundf(x);
}

extern "C" float  MathCeil(float x) {
    return ceilf(x);
}

extern "C" float  MathFloor(float x) {
    return floorf(x);
}

extern "C" float  MathSin(float x) {
    return sinf(x);
}

extern "C" float  MathCos(float x) {
    return cosf(x);
}

extern "C" float  MathACos(float x) {
    return acosf(x);
}

extern "C" float  MathTan(float x) {
    return tanf(x);
}

extern "C" float  MathSqrt(float x) {
    return sqrtf(x);
}

extern "C" float  MathExp(float x) {
    return expf(x);
}

extern "C" float  MathLog(float x) {
    return logf(x);
}

extern "C" float  MathPow(float x, float y) {
    return powf(x, y);
}

extern "C" double MathLdexp(double x, int exp) {
    return ldexpf(x, exp);
}

extern "C" float  MathRandom() {
    return float(rand()) / float(RAND_MAX);
}

extern "C" float  MathAtan2(float y, float x) {
    return atan2f(y, x);
}