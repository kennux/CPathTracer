#include <xmmintrin.h>
#include "ptmath.h"

#if _MSC_VER
__declspec( thread ) static Vec3f_Pack4 tmp;
#else
thread_local static Vec3f_Pack4 tmp;
#endif

void sp_4vec_pack_single(Vec3f_Pack4* out, Vec3f* vec)
{
    out->x[0] = vec->x;
    out->x[1] = vec->x;
    out->x[2] = vec->x;
    out->x[3] = vec->x;

    out->y[0] = vec->y;
    out->y[1] = vec->y;
    out->y[2] = vec->y;
    out->y[3] = vec->y;

    out->z[0] = vec->z;
    out->z[1] = vec->z;
    out->z[2] = vec->z;
    out->z[3] = vec->z;
}

void sp_4vec_extract(Vec3f_Pack4* pack, Vec3f* out, size_t idx)
{
    out->x = pack->x[idx];
    out->y = pack->y[idx];
    out->z = pack->z[idx];
}

void sp_4vec_lengthSq(mfloat* result, Vec3f_Pack4* v0)
{
    sp_4vec_dot(result, v0, v0);
}

void sp_4vec_dot(mfloat* result, Vec3f_Pack4* v0, Vec3f_Pack4* v1)
{
    sp_4vec_mul(&tmp, v0, v1);
    sp_4vec_add4(tmp, result);
    /*for (size_t i = 0; i < 4; i++)
        result[i] = tmp.x[i] + tmp.y[i] + tmp.z[i];*/
}

void sp_4vec_add4(Vec3f_Pack4 v0, mfloat result[4])
{
    __m128 xmm_x = _mm_loadu_ps(v0.x); // Load tmp.x[0], tmp.x[1], tmp.x[2], tmp.x[3] into xmm_x
    __m128 xmm_y = _mm_loadu_ps(v0.y); // Load tmp.y[0], tmp.y[1], tmp.y[2], tmp.y[3] into xmm_y
    __m128 xmm_z = _mm_loadu_ps(v0.z); // Load tmp.z[0], tmp.z[1], tmp.z[2], tmp.z[3] into xmm_z

    // Perform SIMD addition
    __m128 xmm_sum1 = _mm_add_ps(xmm_x, xmm_y); // Add tmp.x and tmp.y
    xmm_sum1 = _mm_add_ps(xmm_sum1, xmm_z);     // Add tmp.z

    // Store the result back to result array
    _mm_storeu_ps(result, xmm_sum1); // Store xmm_sum1 into result[0], result[1], result[2], result[3]
}

void sp_4vec_add(Vec3f_Pack4* result, Vec3f_Pack4* v0, Vec3f_Pack4* v1)
{
    /*for (size_t i = 0; i < 4; i++) {
        result->x[i] = v0->x[i] + v1->x[i];
        result->y[i] = v0->y[i] + v1->y[i];
        result->z[i] = v0->z[i] + v1->z[i];
    }*/

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
}

void sp_4vec_mul(Vec3f_Pack4* result, Vec3f_Pack4* v0, Vec3f_Pack4* v1)
{
    /*for (size_t i = 0; i < 4; i++) {
        result->x[i] = v0->x[i] * v1->x[i];
        result->y[i] = v0->y[i] * v1->y[i];
        result->z[i] = v0->z[i] * v1->z[i];
    }*/

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
}

void sp_4vec_sub(Vec3f_Pack4* result, Vec3f_Pack4* v0, Vec3f_Pack4* v1)
{
    /*
    for (size_t i = 0; i < 4; i++)
    {
        result->x[i] = v0->x[i] - v1->x[i];
        result->y[i] = v0->y[i] - v1->y[i];
        result->z[i] = v0->z[i] - v1->z[i];
    }
     */

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
}