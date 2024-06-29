#include "ptmath.h"

mfloat lerp(mfloat a, mfloat b, mfloat t) {
    return a + t * (b - a);
}