#pragma once

#include <float.h>
#include <math.h>

#define PI 3.14159265359
#define MFLOAT_MIN FLT_MIN
#define MFLOAT_MAX FLT_MAX

typedef float mfloat;

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

typedef struct Vec3f_Pack4
{
    mfloat x[4];
    mfloat y[4];
    mfloat z[4];
} Vec3f_Pack4;

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
void p_v3f_min(Vec3f *out, Vec3f* v0, Vec3f* v1);
void p_v3f_max(Vec3f *out, Vec3f* v0, Vec3f* v1);

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
Vec3f v3f_min(Vec3f v0, Vec3f v1);
Vec3f v3f_max(Vec3f v0, Vec3f v1);

void sp_4vec_pack_single(Vec3f_Pack4* out, Vec3f* vec);
void sp_4vec_extract(Vec3f_Pack4* pack, Vec3f* out, size_t idx);
void sp_4vec_sub(Vec3f_Pack4* result, Vec3f_Pack4* v0, Vec3f_Pack4* v1);
void sp_4vec_add(Vec3f_Pack4* result, Vec3f_Pack4* v0, Vec3f_Pack4* v1);
void sp_4vec_mul(Vec3f_Pack4* result, Vec3f_Pack4* v0, Vec3f_Pack4* v1);
void sp_4vec_dot(mfloat* result, Vec3f_Pack4* v0, Vec3f_Pack4* v1);
void sp_4vec_lengthSq(mfloat* result, Vec3f_Pack4* v0);
void sp_4vec_add4(Vec3f_Pack4 v0, mfloat result[4]);