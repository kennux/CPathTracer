#include "random.h"

void xor_shift_32(RandomState* state)
{
    *state ^= (*state << 13);
    *state ^= (*state >> 17);
    *state ^= (*state << 15);
}
mfloat random_01(RandomState* state)
{
    xor_shift_32(state);
    return (*state & 0xFFFFFF) / 16777216.0f;
}

void random_packInUnitDisk(Vec3f_Pack* out, RandomState* state)
{
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        out->x[i] = (random_01(state) * 2.0f) - 1.0f;
        out->y[i] = (random_01(state) * 2.0f) - 1.0f;
        out->z[0] = 0;
    }

    sip_v_normalize_p(out, out);
}

void random_inUnitDisk(Vec3f* out, RandomState* state)
{
    out->x = (random_01(state) * 2.0f) - 1.0f;
    out->y = (random_01(state) * 2.0f) - 1.0f;
    out->z = 0;
    p_v3f_normalize(out, out);
}

void random_inUnitSphere(Vec3f* out, RandomState* state)
{
    out->x = (random_01(state) * 2.0f) - 1.0f;
    out->y = (random_01(state) * 2.0f) - 1.0f;
    out->z = (random_01(state) * 2.0f) - 1.0f;
    p_v3f_normalize(out, out);
}

void random_unitVector(Vec3f* out, RandomState* state)
{
    mfloat z = random_01(state) * 2.0f - 1.0f;
    mfloat a = random_01(state) * 2.0f * PI;
    mfloat r = sqrt(1.0f - z * z);
    mfloat x = sin(a);
    mfloat y = cos(a);

    out->x = r * x;
    out->y = r * y;
    out->z = z;
}