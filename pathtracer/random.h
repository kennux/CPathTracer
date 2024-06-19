//
// Created by kennu on 19/06/2024.
//

#include "ptmath.h"

#ifndef CPATHTRACER_RANDOM_H
#define CPATHTRACER_RANDOM_H

typedef unsigned int random_state;

void xor_shift_32(random_state* state);
mfloat random_01(random_state* state);
void random_in_unit_disk(Vec3f* out, random_state* state);
void random_in_unit_sphere(Vec3f* out, random_state* state);
void random_unit_vector(Vec3f* out, random_state* state);

#endif //CPATHTRACER_RANDOM_H
