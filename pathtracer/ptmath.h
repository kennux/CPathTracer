#pragma once

#include <math.h>

#define PI 3.14159265359

typedef float mfloat;
typedef int mint;

typedef struct Vec2i
{
    mint x;
    mint y;
} Vec2i;

typedef struct Vec3f
{
    mfloat x;
    mfloat y;
    mfloat z;
} Vec3f;

typedef struct Ray
{
    Vec3f origin;
    Vec3f direction;
} Ray;

// Ray
void p_ray_getPoint(Vec3f* point, Ray* ray, mfloat d);
Vec3f ray_getPoint(Ray* ray, mfloat d);

// Vec3f
void p_vec3f(Vec3f* out, mfloat x, mfloat y, mfloat z);
void p_v3f_add_v3f(Vec3f* out, Vec3f* v0, Vec3f* v1);
void p_v3f_sub_v3f(Vec3f* out, Vec3f* v0, Vec3f* v1);
void p_v3f_mul_v3f(Vec3f* out, Vec3f* v0, Vec3f* v1);
void p_v3f_mul_f(Vec3f* out, Vec3f* v0, mfloat v1);
void p_v3f_normalize(Vec3f* out, Vec3f* vec);
void p_v3f_length(mfloat* out, Vec3f* vec);
void p_v3f_lengthSq(mfloat* out, Vec3f* vec);
void p_v3f_cross(Vec3f* out, Vec3f* v0, Vec3f* v1);
void p_v3f_dot(mfloat* out, Vec3f* v0, Vec3f* v1);
void p_v3f_reflect(Vec3f* out, Vec3f* vec, Vec3f* normal);

Vec3f vec3f(mfloat x, mfloat y, mfloat z);
Vec3f v3f_add_v3f(Vec3f v0, Vec3f v1);
Vec3f v3f_sub_v3f(Vec3f v0, Vec3f v1);
Vec3f v3f_mul_v3f(Vec3f v0, Vec3f v1);
Vec3f v3f_mul_f(Vec3f v0, mfloat v1);
Vec3f v3f_normalize(Vec3f vec);
mfloat v3f_length(Vec3f vec);
mfloat v3f_lengthSq(Vec3f vec);
Vec3f v3f_cross(Vec3f v0, Vec3f v1);
mfloat v3f_dot(Vec3f v0, Vec3f v1);
Vec3f v3f_reflect(Vec3f vec, Vec3f normal);