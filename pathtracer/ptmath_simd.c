#include "ptmath.h"
#include <immintrin.h>

__attribute__((aligned(32))) __declspec( thread ) static Vec3f_Pack tmpVec;
__attribute__((aligned(32))) __declspec( thread ) static AlignedFloatPack tmpFloats;

void si_v_pack_s(Vec3f_Pack* out, Vec3f* vec)
{
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        out->x[i] = vec->x;
        out->y[i] = vec->y;
        out->z[i] = vec->z;
    }
}

void si_v_extract_s(Vec3f_Pack* pack, Vec3f* out, size_t idx)
{
    out->x = pack->x[idx];
    out->y = pack->y[idx];
    out->z = pack->z[idx];
}

void si_v_lenSq_p(mfloat* result, Vec3f_Pack* v0)
{
    si_v_dot_pp(result, v0, v0);
}

void si_v_dot_pp(mfloat* result, Vec3f_Pack* v0, Vec3f_Pack* v1)
{
    si_v_mul_pp(&tmpVec, v0, v1);
    si_v_sumComps_p(result, &tmpVec);
    /*for (size_t i = 0; i < 4; i++)
        result[i] = tmpVec.x[i] + tmpVec.y[i] + tmpVec.z[i];*/
}


void si_v_add_sp(Vec3f_Pack* result, Vec3f* v0, Vec3f_Pack* v1)
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
    __m256 xmm1 = _mm256_broadcast_ss(&v0->x);
    __m256 xmm2 = _mm256_loadu_ps(&v1->x);

    __m256 xmm_result = _mm256_add_ps(xmm1, xmm2);

    _mm256_store_ps(&result->x, xmm_result);

    xmm1 = _mm256_broadcast_ss(&v0->y);
    xmm2 = _mm256_loadu_ps(&v1->y);

    xmm_result = _mm256_add_ps(xmm1, xmm2);

    _mm256_store_ps(&result->y, xmm_result);

    xmm1 = _mm256_broadcast_ss(&v0->z);
    xmm2 = _mm256_loadu_ps(&v1->z);

    xmm_result = _mm256_add_ps(xmm1, xmm2);

    _mm256_store_ps(&result->z, xmm_result);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        result->x[i] = v0->x + v1->x[i];
        result->y[i] = v0->y + v1->y[i];
        result->z[i] = v0->z + v1->z[i];
    }
#endif
}
void si_vf_add_sp(Vec3f_Pack* result, Vec3f* v0, mfloat* pack1)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm2 = _mm_loadu_ps(pack1);
    __m128 xmm1 = _mm_broadcast_ss(&v0->x);
    __m128 xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_storeu_ps(&result->x, xmm_result);

    xmm1 = _mm_broadcast_ss(&v0->y);
    xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_storeu_ps(&result->y, xmm_result);

    xmm1 = _mm_broadcast_ss(&v0->z);
    xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_storeu_ps(&result->z, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm2 = _mm256_loadu_ps(pack1);

    __m256 xmm1 = _mm256_broadcast_ss(&v0->x);
    __m256 xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(&result->x, xmm_result);

    xmm1 = _mm256_broadcast_ss(&v0->y);
    xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(&result->y, xmm_result);

    xmm1 = _mm256_broadcast_ss(&v0->z);
    xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(&result->z, xmm_result);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        result->x[i] = v0->x * pack1[i];
        result->y[i] = v0->y * pack1[i];
        result->z[i] = v0->z * pack1[i];
    }
#endif
}

void si_vf_add_pp(Vec3f_Pack* result, Vec3f_Pack* v0, mfloat* pack1)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm2 = _mm_loadu_ps(pack1);
    __m128 xmm1 = _mm_loadu_ps(&v0->x);
    __m128 xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_storeu_ps(&result->x, xmm_result);

    xmm1 = _mm_loadu_ps(&v0->y);
    xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_storeu_ps(&result->y, xmm_result);

    xmm1 = _mm_loadu_ps(&v0->z);
    xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_storeu_ps(&result->z, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm2 = _mm256_loadu_ps(pack1);

    __m256 xmm1 = _mm256_loadu_ps(&v0->x);
    __m256 xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(&result->x, xmm_result);

    xmm1 = _mm256_loadu_ps(&v0->y);
    xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(&result->y, xmm_result);

    xmm1 = _mm256_loadu_ps(&v0->z);
    xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(&result->z, xmm_result);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        result->x[i] = v0->x[i] * pack1[i];
        result->y[i] = v0->y[i] * pack1[i];
        result->z[i] = v0->z[i] * pack1[i];
    }
#endif
}

void si_f_mul_ps(mfloat* result, mfloat* pack1, mfloat val)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm1 = _mm_loadu_ps(pack1);
    __m128 xmm2 = _mm_broadcast_ss(&val);

    __m128 xmm_result = _mm_mul_ps(xmm1, xmm2);

    _mm_store_ps(result, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_loadu_ps(pack1);
    __m256 xmm2 = _mm256_broadcast_ss(&val);

    __m256 xmm_result = _mm256_mul_ps(xmm1, xmm2);

    _mm256_store_ps(result, xmm_result);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        result[i] = pack1[i] * val;
    }
#endif
}

void si_f_mul_pp(mfloat* result, mfloat* pack1, mfloat* pack2)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm1 = _mm_loadu_ps(pack1);
    __m128 xmm2 = _mm_loadu_ps(pack2);

    __m128 xmm_result = _mm_mul_ps(xmm1, xmm2);

    _mm_store_ps(result, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_loadu_ps(pack1);
    __m256 xmm2 = _mm256_loadu_ps(pack2);

    __m256 xmm_result = _mm256_mul_ps(xmm1, xmm2);

    _mm256_store_ps(result, xmm_result);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        result[i] = pack1[i] * pack2[i];
    }
#endif
}

void si_v_normalize_p(Vec3f_Pack* result, Vec3f_Pack *v0)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    sp_packVec_lengthSq(&tmpFloats, v0);
    __m128 xmm_lenSq = _mm_load_ps(&tmpFloats);
    __m128 xmm_len = _mm_sqrt_ps(xmm_lenSq);
    __m256 xmm_lenMin = _mm256_set_ps(0.00001f, 0.00001f, 0.00001f, 0.00001f);
    xmm_len = _mm_max_ps(xmm_len, xmm_lenMin);

    // Calc reciprocal
    __m128 xmm_lenRec = _mm_rcp_ps(xmm_len);

    __m128 xmm_x = _mm_loadu_ps(v0->x);
    __m128 xmm_y = _mm_loadu_ps(v0->y);
    __m128 xmm_z = _mm_loadu_ps(v0->z);

    // Divide by len
    __m128 xmm_resX = _mm_mul_ps(xmm_x, xmm_lenRec);
    __m128 xmm_resY = _mm_mul_ps(xmm_y, xmm_lenRec);
    __m128 xmm_resZ = _mm_mul_ps(xmm_z, xmm_lenRec);

    _mm_storeu_ps(&result->x, xmm_resX);
    _mm_storeu_ps(&result->y, xmm_resY);
    _mm_storeu_ps(&result->z, xmm_resZ);
#elif SIMD_MATH_WIDTH == 8
    si_v_lenSq_p(&tmpFloats, v0);
    __m256 xmm_lenSq = _mm256_load_ps(&tmpFloats);
    __m256 xmm_len = _mm256_sqrt_ps(xmm_lenSq);
    __m256 xmm_lenMin = _mm256_set_ps(0.00001f, 0.00001f, 0.00001f, 0.00001f, 0.00001f, 0.00001f, 0.00001f, 0.00001f);
    xmm_len = _mm256_max_ps(xmm_len, xmm_lenMin);

    __m256 xmm_x = _mm256_loadu_ps(v0->x);
    __m256 xmm_y = _mm256_loadu_ps(v0->y);
    __m256 xmm_z = _mm256_loadu_ps(v0->z);

    // Calc reciprocal
    __m256 xmm_lenRec = _mm256_rcp_ps(xmm_len);

    // Divide by len
    __m256 xmm_resX = _mm256_mul_ps(xmm_x, xmm_lenRec);
    __m256 xmm_resY = _mm256_mul_ps(xmm_y, xmm_lenRec);
    __m256 xmm_resZ = _mm256_mul_ps(xmm_z, xmm_lenRec);

    _mm256_storeu_ps(&result->x, xmm_resX);
    _mm256_storeu_ps(&result->y, xmm_resY);
    _mm256_storeu_ps(&result->z, xmm_resZ);
#endif
#else
    sp_packVec_lengthSq(&tmpFloats, v0);
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        if (tmpFloats[i] == 0)
            tmpFloats[i] = 0.00001f;
        result->x[i] = v0->x[i] / tmpFloats[i];
        result->y[i] = v0->y[i] / tmpFloats[i];
        result->z[i] = v0->z[i] / tmpFloats[i];
    }
#endif
}

void si_v_normalizeUnsafe_p(Vec3f_Pack* result, Vec3f_Pack *v0)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    sp_packVec_lengthSq(&tmpFloats, v0);
    __m128 xmm_lenSq = _mm_load_ps(&tmpFloats);
    __m128 xmm_len = _mm_sqrt_ps(xmm_lenSq);

    // Calc reciprocal
    __m128 xmm_lenRec = _mm_rcp_ps(xmm_len);

    __m128 xmm_x = _mm_loadu_ps(v0->x);
    __m128 xmm_y = _mm_loadu_ps(v0->y);
    __m128 xmm_z = _mm_loadu_ps(v0->z);

    // Divide by len
    __m128 xmm_resX = _mm_mul_ps(xmm_x, xmm_lenRec);
    __m128 xmm_resY = _mm_mul_ps(xmm_y, xmm_lenRec);
    __m128 xmm_resZ = _mm_mul_ps(xmm_z, xmm_lenRec);

    _mm_storeu_ps(&result->x, xmm_resX);
    _mm_storeu_ps(&result->y, xmm_resY);
    _mm_storeu_ps(&result->z, xmm_resZ);
#elif SIMD_MATH_WIDTH == 8
    si_v_lenSq_p(&tmpFloats, v0);
    __m256 xmm_lenSq = _mm256_load_ps(&tmpFloats);
    __m256 xmm_len = _mm256_sqrt_ps(xmm_lenSq);

    __m256 xmm_x = _mm256_loadu_ps(v0->x);
    __m256 xmm_y = _mm256_loadu_ps(v0->y);
    __m256 xmm_z = _mm256_loadu_ps(v0->z);

    // Calc reciprocal
    __m256 xmm_lenRec = _mm256_rcp_ps(xmm_len);

    // Divide by len
    __m256 xmm_resX = _mm256_div_ps(xmm_x, xmm_lenRec);
    __m256 xmm_resY = _mm256_div_ps(xmm_y, xmm_lenRec);
    __m256 xmm_resZ = _mm256_div_ps(xmm_z, xmm_lenRec);

    _mm256_storeu_ps(&result->x, xmm_resX);
    _mm256_storeu_ps(&result->y, xmm_resY);
    _mm256_storeu_ps(&result->z, xmm_resZ);
#endif
#else
    sp_packVec_lengthSq(&tmpFloats, v0);
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        result->x[i] = v0->x[i] / tmpFloats[i];
        result->y[i] = v0->y[i] / tmpFloats[i];
        result->z[i] = v0->z[i] / tmpFloats[i];
    }
#endif
}

void si_v_sumComps_p(mfloat *result, Vec3f_Pack *v0)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm_x = _mm_loadu_ps(v0->x); // Load tmpVec.x[0], tmpVec.x[1], tmpVec.x[2], tmpVec.x[3] into xmm_x
    __m128 xmm_y = _mm_loadu_ps(v0->y); // Load tmpVec.y[0], tmpVec.y[1], tmpVec.y[2], tmpVec.y[3] into xmm_y
    __m128 xmm_z = _mm_loadu_ps(v0->z); // Load tmpVec.z[0], tmpVec.z[1], tmpVec.z[2], tmpVec.z[3] into xmm_z

    // Perform SIMD addition
    __m128 xmm_sum1 = _mm_add_ps(xmm_x, xmm_y); // Add tmpVec.x and tmpVec.y
    xmm_sum1 = _mm_add_ps(xmm_sum1, xmm_z);     // Add tmpVec.z

    // Store the result back to result array
    _mm_storeu_ps(result, xmm_sum1); // Store xmm_sum1 into result[0], result[1], result[2], result[3]
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm_x = _mm256_loadu_ps(v0->x);
    __m256 xmm_y = _mm256_loadu_ps(v0->y);
    __m256 xmm_z = _mm256_loadu_ps(v0->z);

    // Perform SIMD addition
    __m256 xmm_sum1 = _mm256_add_ps(xmm_x, xmm_y); // Add tmpVec.x and tmpVec.y
    xmm_sum1 = _mm256_add_ps(xmm_sum1, xmm_z);     // Add tmpVec.z

    // Store the result back to result array
    _mm256_storeu_ps(result, xmm_sum1); // Store xmm_sum1 into result[0], result[1], result[2], result[3]
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
        result[i] = v0->x[i] + v0->y[i] + v0->z[i];
#endif
}

void si_v_add_pp(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1)
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

void si_v_mul_pp(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1)
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

void si_v_sub_pp(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1)
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