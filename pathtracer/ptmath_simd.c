#include "ptmath.h"
#include <immintrin.h>

__attribute__((aligned(32))) __declspec( thread ) static Vec3f_Pack tmp;

void sp_packVec_pack_single(Vec3f_Pack* out, Vec3f* vec)
{
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        out->x[i] = vec->x;
        out->y[i] = vec->y;
        out->z[i] = vec->z;
    }
}

void sp_packVec_extract(Vec3f_Pack* pack, Vec3f* out, size_t idx)
{
    out->x = pack->x[idx];
    out->y = pack->y[idx];
    out->z = pack->z[idx];
}

void sp_packVec_lengthSq(mfloat* result, Vec3f_Pack* v0)
{
    sp_packVec_dot(result, v0, v0);
}

void sp_packVec_dot(mfloat* result, Vec3f_Pack* v0, Vec3f_Pack* v1)
{
    sp_packVec_mul(&tmp, v0, v1);
    sp_packVec_addF(&tmp, result);
    /*for (size_t i = 0; i < 4; i++)
        result[i] = tmp.x[i] + tmp.y[i] + tmp.z[i];*/
}

void sp_packVec_addF(Vec3f_Pack* v0, mfloat* result)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm_x = _mm_loadu_ps(v0->x); // Load tmp.x[0], tmp.x[1], tmp.x[2], tmp.x[3] into xmm_x
    __m128 xmm_y = _mm_loadu_ps(v0->y); // Load tmp.y[0], tmp.y[1], tmp.y[2], tmp.y[3] into xmm_y
    __m128 xmm_z = _mm_loadu_ps(v0->z); // Load tmp.z[0], tmp.z[1], tmp.z[2], tmp.z[3] into xmm_z

    // Perform SIMD addition
    __m128 xmm_sum1 = _mm_add_ps(xmm_x, xmm_y); // Add tmp.x and tmp.y
    xmm_sum1 = _mm_add_ps(xmm_sum1, xmm_z);     // Add tmp.z

    // Store the result back to result array
    _mm_storeu_ps(result, xmm_sum1); // Store xmm_sum1 into result[0], result[1], result[2], result[3]
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm_x = _mm256_loadu_ps(v0->x);
    __m256 xmm_y = _mm256_loadu_ps(v0->y);
    __m256 xmm_z = _mm256_loadu_ps(v0->z);

    // Perform SIMD addition
    __m256 xmm_sum1 = _mm256_add_ps(xmm_x, xmm_y); // Add tmp.x and tmp.y
    xmm_sum1 = _mm256_add_ps(xmm_sum1, xmm_z);     // Add tmp.z

    // Store the result back to result array
    _mm256_storeu_ps(result, xmm_sum1); // Store xmm_sum1 into result[0], result[1], result[2], result[3]
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
        result[i] = v0->x[i] + v0->y[i] + v0->z[i];
#endif
}

void sp_packVec_add(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    // Load vectors into SIMD registers
    __m128 xmm1 = _mm_loadu_ps(&v0->x);
    __m128 xmm2 = _mm_loadu_ps(&v1->x);

    // Perform subtraction
    __m128 xmm_result = _mm_add_ps(xmm1, xmm2);

    // Store result back to memory
    _mm_storeu_ps(&result->x, xmm_result);

    // Load vectors into SIMD registers
    xmm1 = _mm_loadu_ps(&v0->y);
    xmm2 = _mm_loadu_ps(&v1->y);

    // Perform subtraction
    xmm_result = _mm_add_ps(xmm1, xmm2);

    // Store result back to memory
    _mm_storeu_ps(&result->y, xmm_result);

    // Load vectors into SIMD registers
    xmm1 = _mm_loadu_ps(&v0->z);
    xmm2 = _mm_loadu_ps(&v1->z);

    // Perform subtraction
    xmm_result = _mm_add_ps(xmm1, xmm2);

    // Store result back to memory
    _mm_storeu_ps(&result->z, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_loadu_ps(&v0->x);
    __m256 xmm2 = _mm256_loadu_ps(&v1->x);

    __m256 xmm_result = _mm256_add_ps(xmm1, xmm2);

    _mm256_store_ps(&result->x, xmm_result);

    xmm1 = _mm256_loadu_ps(&v0->y);
    xmm2 = _mm256_loadu_ps(&v1->y);

    xmm_result = _mm256_add_ps(xmm1, xmm2);

    _mm256_store_ps(&result->y, xmm_result);

    xmm1 = _mm256_loadu_ps(&v0->z);
    xmm2 = _mm256_loadu_ps(&v1->z);

    xmm_result = _mm256_add_ps(xmm1, xmm2);

    _mm256_store_ps(&result->z, xmm_result);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        result->x[i] = v0->x[i] + v1->x[i];
        result->y[i] = v0->y[i] + v1->y[i];
        result->z[i] = v0->z[i] + v1->z[i];
    }
#endif
}

void sp_packVec_mul(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    // Load vectors into SIMD registers
    __m128 xmm1 = _mm_loadu_ps(&v0->x);
    __m128 xmm2 = _mm_loadu_ps(&v1->x);

    // Perform subtraction
    __m128 xmm_result = _mm_mul_ps(xmm1, xmm2);

    // Store result back to memory
    _mm_storeu_ps(&result->x, xmm_result);

    // Load vectors into SIMD registers
    xmm1 = _mm_loadu_ps(&v0->y);
    xmm2 = _mm_loadu_ps(&v1->y);

    // Perform subtraction
    xmm_result = _mm_mul_ps(xmm1, xmm2);

    // Store result back to memory
    _mm_storeu_ps(&result->y, xmm_result);


    // Load vectors into SIMD registers
    xmm1 = _mm_loadu_ps(&v0->z);
    xmm2 = _mm_loadu_ps(&v1->z);

    // Perform subtraction
    xmm_result = _mm_mul_ps(xmm1, xmm2);

    // Store result back to memory
    _mm_storeu_ps(&result->z, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_loadu_ps(&v0->x);
    __m256 xmm2 = _mm256_loadu_ps(&v1->x);

    __m256 xmm_result = _mm256_mul_ps(xmm1, xmm2);

    _mm256_store_ps(&result->x, xmm_result);

    xmm1 = _mm256_loadu_ps(&v0->y);
    xmm2 = _mm256_loadu_ps(&v1->y);

    xmm_result = _mm256_mul_ps(xmm1, xmm2);

    _mm256_store_ps(&result->y, xmm_result);

    xmm1 = _mm256_loadu_ps(&v0->z);
    xmm2 = _mm256_loadu_ps(&v1->z);

    xmm_result = _mm256_mul_ps(xmm1, xmm2);

    _mm256_store_ps(&result->z, xmm_result);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        result->x[i] = v0->x[i] * v1->x[i];
        result->y[i] = v0->y[i] * v1->y[i];
        result->z[i] = v0->z[i] * v1->z[i];
    }
#endif
}

void sp_packVec_sub(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    // Load vectors into SIMD registers
    __m128 xmm1 = _mm_loadu_ps(&v0->x);
    __m128 xmm2 = _mm_loadu_ps(&v1->x);

    // Perform subtraction
    __m128 xmm_result = _mm_sub_ps(xmm1, xmm2);

    // Store result back to memory
    _mm_storeu_ps(&result->x, xmm_result);

    // Load vectors into SIMD registers
    xmm1 = _mm_loadu_ps(&v0->y);
    xmm2 = _mm_loadu_ps(&v1->y);

    // Perform subtraction
    xmm_result = _mm_sub_ps(xmm1, xmm2);

    // Store result back to memory
    _mm_storeu_ps(&result->y, xmm_result);


    // Load vectors into SIMD registers
    xmm1 = _mm_loadu_ps(&v0->z);
    xmm2 = _mm_loadu_ps(&v1->z);

    // Perform subtraction
    xmm_result = _mm_sub_ps(xmm1, xmm2);

    // Store result back to memory
    _mm_storeu_ps(&result->z, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_loadu_ps(&v0->x);
    __m256 xmm2 = _mm256_loadu_ps(&v1->x);

    __m256 xmm_result = _mm256_sub_ps(xmm1, xmm2);

    _mm256_store_ps(&result->x, xmm_result);

    xmm1 = _mm256_loadu_ps(&v0->y);
    xmm2 = _mm256_loadu_ps(&v1->y);

    xmm_result = _mm256_sub_ps(xmm1, xmm2);

    _mm256_store_ps(&result->y, xmm_result);

    xmm1 = _mm256_loadu_ps(&v0->z);
    xmm2 = _mm256_loadu_ps(&v1->z);

    xmm_result = _mm256_sub_ps(xmm1, xmm2);

    _mm256_store_ps(&result->z, xmm_result);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        result->x[i] = v0->x[i] - v1->x[i];
        result->y[i] = v0->y[i] - v1->y[i];
        result->z[i] = v0->z[i] - v1->z[i];
    }
#endif
}