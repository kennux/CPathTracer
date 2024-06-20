#pragma once
#include "ptmath.h"

typedef unsigned int RandomState;

void xor_shift_32(RandomState* state);
mfloat random_01(RandomState* state);
void random_in_unit_disk(Vec3f* out, RandomState* state);
void random_in_unit_sphere(Vec3f* out, RandomState* state);
void random_unit_vector(Vec3f* out, RandomState* state);