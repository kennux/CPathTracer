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
typedef mfloat AlignedFloatPack[8] __attribute__((aligned(32)));
#elif SIMD_MATH_WIDTH == 4
typedef mfloat AlignedFloatPack[8] __attribute__((aligned(16)));
#endif

#if SIMD_MATH_WIDTH == 8
__declspec(align(32)) typedef struct Vec3f_Pack
#elif SIMD_MATH_WIDTH == 4
__declspec(align(16)) typedef struct Vec3f_Pack
#endif
{
    AlignedFloatPack x;
    AlignedFloatPack y;
    AlignedFloatPack z;
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

// si_ = SIMD
// - f_ = Float
// - v_ = Vector
// mul/add/... = Operation
// - s = Single
// - p = Pack

void si_f_mul_pp(mfloat* result, mfloat* pack1, mfloat* pack2);
void si_f_mul_ps(mfloat* result, mfloat* pack1, mfloat val);

void si_v_pack_s(Vec3f_Pack* out, Vec3f* vec);
void si_v_extract_s(Vec3f_Pack* pack, Vec3f* out, size_t idx);
void si_v_sub_pp(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1);
void si_v_sub_sp(Vec3f_Pack* result, Vec3f* v0, Vec3f_Pack* v1);
void si_v_add_pp(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1);
void si_v_add_sp(Vec3f_Pack* result, Vec3f* v0, Vec3f_Pack* v1);
void si_v_mul_pp(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1);
void si_v_mul_sp(Vec3f_Pack* result, Vec3f* v0, Vec3f_Pack* v1);
void si_v_dot_pp(mfloat* result, Vec3f_Pack* v0, Vec3f_Pack* v1);
void si_v_dot_sp(mfloat* result, Vec3f* v0, Vec3f_Pack* v1);
void si_v_lenSq_p(mfloat* result, Vec3f_Pack* v0);
void si_vf_add_pp(Vec3f_Pack* result, Vec3f_Pack* v0, mfloat* pack1);
void si_vf_add_sp(Vec3f_Pack* result, Vec3f* v0, mfloat* pack1);
void si_v_sumComps_p(mfloat *result, Vec3f_Pack *v0);
void si_v_normalizeUnsafe_p(Vec3f_Pack* result, Vec3f_Pack *v0);
void si_v_normalize_p(Vec3f_Pack* result, Vec3f_Pack *v0);