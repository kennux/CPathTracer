#pragma once
#include "ptmath.h"

typedef unsigned int RandomState;

void xor_shift_32(RandomState* state);
mfloat random_01(RandomState* state);

void random_packInUnitDisk(Vec3f_Pack* out, RandomState* state);

void random_inUnitDisk(Vec3f* out, RandomState* state);
void random_inUnitSphere(Vec3f* out, RandomState* state);
void random_unitVector(Vec3f* out, RandomState* state);