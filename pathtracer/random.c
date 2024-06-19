//
// Created by kennu on 19/06/2024.
//
#include "random.h"

void xor_shift_32(random_state* state)
{
    *state ^= (*state << 13);
    *state ^= (*state >> 17);
    *state ^= (*state << 15);
}
mfloat random_01(random_state* state)
{
    xor_shift_32(state);
    return (*state & 0xFFFFFF) / 16777216.0f;
}

void random_in_unit_disk(Vec3f* out, random_state* state)
{
    out->x = (random_01(state) * 2.0f) - 1.0f;
    out->y = (random_01(state) * 2.0f) - 1.0f;
    out->z = 0;
    p_v3f_normalize(out, out);
}

void random_in_unit_sphere(Vec3f* out, random_state* state)
{
    out->x = (random_01(state) * 2.0f) - 1.0f;
    out->y = (random_01(state) * 2.0f) - 1.0f;
    out->z = (random_01(state) * 2.0f) - 1.0f;
    p_v3f_normalize(out, out);
}

void random_unit_vector(Vec3f* out, random_state* state)
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