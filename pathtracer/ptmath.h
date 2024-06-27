#pragma once

#include <float.h>
#include <math.h>

#define PI 3.14159265359
#define MFLOAT_MIN FLT_MIN
#define MFLOAT_MAX FLT_MAX

#define EXPLICIT_SIMD

#ifdef EXPLICIT_SIMD
    #ifdef __AVX__
    #define SIMD_MATH_WIDTH 8
    #define SIMD 1
    #elifdef __SSE__
    #define SIMD_MATH_WIDTH 4
    #define SIMD 1
    #else
    #define SIMD_MATH_WIDTH 8
    #define SIMD 0
    #endif
#else
    #define SIMD_MATH_WIDTH 8
    #define SIMD 0
#endif

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

#if SIMD_MATH_WIDTH == 8
__declspec(align(32)) typedef struct Vec3f_Pack
#elif SIMD_MATH_WIDTH == 4
__declspec(align(16)) typedef struct Vec3f_Pack
#endif
{
    mfloat x[SIMD_MATH_WIDTH];
    mfloat y[SIMD_MATH_WIDTH];
    mfloat z[SIMD_MATH_WIDTH];
} Vec3f_Pack;

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

void sp_packVec_pack_single(Vec3f_Pack* out, Vec3f* vec);
void sp_packVec_extract(Vec3f_Pack* pack, Vec3f* out, size_t idx);
void sp_packVec_sub(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1);
void sp_packVec_add(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1);
void sp_packVec_mul(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1);
void sp_packVec_dot(mfloat* result, Vec3f_Pack* v0, Vec3f_Pack* v1);
void sp_packVec_lengthSq(mfloat* result, Vec3f_Pack* v0);
void sp_packVec_addF(Vec3f_Pack* v0, mfloat* result);