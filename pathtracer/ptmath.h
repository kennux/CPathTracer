#pragma once

#include <stdbool.h>
#include <float.h>
#include <math.h>

#define PI 3.14159265359
#define MFLOAT_MIN FLT_MIN
#define MFLOAT_MAX FLT_MAX
#define MFLOAT_EPSILON FLT_EPSILON

#define EXPLICIT_SIMD

#if defined(__clang__) || defined(__GNUC__)
#define THREAD_LOCAL __thread
#define SIMD_ALIGN __attribute__((aligned(32)))
#elif defined(_MSC_VER)
#define THREAD_LOCAL __declspec(thread)
#define SIMD_ALIGN __declspec(align(32))
#else
#error "Unsupported compiler"
#endif

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

SIMD_ALIGN typedef float mfloat;

SIMD_ALIGN typedef struct Vec3f
{
    mfloat x;
    mfloat y;
    mfloat z;
} Vec3f;

SIMD_ALIGN typedef struct Ray
{
    Vec3f origin;
    Vec3f direction;
} Ray;

#if SIMD_MATH_WIDTH == 8
SIMD_ALIGN typedef float AlignedFloatPack[8];
#elif SIMD_MATH_WIDTH == 4
SIMD_ALIGN typedef float AlignedFloatPack[4];
#endif

SIMD_ALIGN typedef struct Vec3f_Pack
{
    AlignedFloatPack x;
    AlignedFloatPack y;
    AlignedFloatPack z;
} Vec3f_Pack;

SIMD_ALIGN typedef struct Vec3f_SoA
{
    mfloat* x;
    mfloat* y;
    mfloat* z;
} Vec3f_SoA;

SIMD_ALIGN typedef int SimdCompareMask;

// SoA Data handling
void p_vec3f_soa(Vec3f_SoA* result, size_t size);
void p_vec3f_soa_fill(Vec3f_SoA* soa, Vec3f* source, size_t size);
void p_vec3f_soa_destroy(Vec3f_SoA* soa);

// Floats
mfloat lerp(mfloat a, mfloat b, mfloat t);

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
bool p_v3f_refract(Vec3f* out, Vec3f* vec, Vec3f* normal, mfloat refractionIdxRatio);
void p_v3f_min(Vec3f *out, Vec3f* v0, Vec3f* v1);
void p_v3f_max(Vec3f *out, Vec3f* v0, Vec3f* v1);
void p_v3f_clamp01(Vec3f* out, Vec3f* v0);

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
Vec3f v3f_clamp01(Vec3f v0);

// si_ = SIMD
// sip_ = SIMD Packed
// - f_ = Float
// - v_ = Vector
// mul/add/... = Operation
// - s = Single
// - p = Pack

void sip_f_mul_pp(mfloat* result, mfloat* pack1, mfloat* pack2);
void sip_f_mul_ps(mfloat* result, mfloat* pack1, mfloat val);

void si_ff_sub_p(mfloat* result, mfloat* minuend, mfloat* subtrahend);
void si_ff_mul_p(mfloat* result, mfloat* factor0, mfloat* factor1);

SimdCompareMask si_f_any_gte(mfloat* pack, mfloat greaterThanOrEqualTo);
SimdCompareMask si_f_any_gt(mfloat* pack, mfloat greaterThan);
SimdCompareMask si_f_any_lte(mfloat* pack, mfloat lessThanOrEqual);

void si_v_sub_sp(mfloat* resultX, mfloat* resultY, mfloat* resultZ, Vec3f* v0, mfloat* v1x, mfloat* v1y, mfloat* v1z);
void si_v_lenSq_p(mfloat* result, mfloat* v0x, mfloat* v0y, mfloat* v0z);
void si_v_dot_sp(mfloat* result, Vec3f* v0, mfloat* v1x, mfloat* v1y, mfloat* v1z);
void si_v_dot_pp(mfloat* result, mfloat* v0x, mfloat* v0y, mfloat* v0z, mfloat* v1x, mfloat* v1y, mfloat* v1z);
void si_v_mul_pp(mfloat* resultX, mfloat* resultY, mfloat* resultZ, mfloat* v0x, mfloat* v0y, mfloat* v0z, mfloat* v1x, mfloat* v1y, mfloat* v1z);
void si_v_mul_sp(mfloat* resultX, mfloat* resultY, mfloat* resultZ, Vec3f* v0, mfloat* v1x, mfloat* v1y, mfloat* v1z);
void si_v_sumComps_p(mfloat *result, mfloat* v0x, mfloat* v0y, mfloat* v0z);

void sip_v_pack_s(Vec3f_Pack* out, Vec3f* vec);
void sip_v_extract_s(Vec3f_Pack* pack, Vec3f* out, size_t idx);
void sip_v_sub_pp(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1);
void sip_v_sub_sp(Vec3f_Pack* result, Vec3f* v0, Vec3f_Pack* v1);
void sip_v_add_pp(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1);
void sip_v_add_sp(Vec3f_Pack* result, Vec3f* v0, Vec3f_Pack* v1);
void sip_v_mul_pp(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1);
void sip_v_mul_sp(Vec3f_Pack* result, Vec3f* v0, Vec3f_Pack* v1);
void sip_v_dot_pp(mfloat* result, Vec3f_Pack* v0, Vec3f_Pack* v1);
void sip_v_dot_sp(mfloat* result, Vec3f* v0, Vec3f_Pack* v1);
void sip_v_lenSq_p(mfloat* result, Vec3f_Pack* v0);
void sip_vf_add_pp(Vec3f_Pack* result, Vec3f_Pack* v0, mfloat* pack1);
void sip_vf_add_sp(Vec3f_Pack* result, Vec3f* v0, mfloat* pack1);
void sip_v_sumComps_p(mfloat *result, Vec3f_Pack *v0);
void sip_v_normalizeUnsafe_p(Vec3f_Pack* result, Vec3f_Pack *v0);
void sip_v_normalize_p(Vec3f_Pack* result, Vec3f_Pack *v0);