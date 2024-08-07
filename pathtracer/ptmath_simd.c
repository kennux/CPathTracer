#include "ptmath.h"
#include <immintrin.h>

THREAD_LOCAL SIMD_ALIGN static Vec3f_Pack tmpVec;
THREAD_LOCAL SIMD_ALIGN static AlignedFloatPack tmpFloats;

void sip_v_pack_s(Vec3f_Pack* out, Vec3f* vec)
{
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        out->x[i] = vec->x;
        out->y[i] = vec->y;
        out->z[i] = vec->z;
    }
}

void sip_v_extract_s(Vec3f_Pack* pack, Vec3f* out, size_t idx)
{
    out->x = pack->x[idx];
    out->y = pack->y[idx];
    out->z = pack->z[idx];
}

void si_v_lenSq_p(mfloat* result, mfloat* v0x, mfloat* v0y, mfloat* v0z)
{
    /*
    si_v_dot_pp(result, v0x, v0y, v0z, v0x, v0y, v0z);*/

    // Optimized:
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 m_v0x = _mm_load_ps(v0x);
    __m128 m_v0y = _mm_load_ps(v0y);
    __m128 m_v0z = _mm_load_ps(v0z);

    // Square
    m_v0x = _mm_mul_ps(m_v0x, m_v0x);
    m_v0y = _mm_mul_ps(m_v0y, m_v0y);
    m_v0z = _mm_mul_ps(m_v0z, m_v0z);

    // Add
    __m128 xmm_sum = _mm_add_ps(m_v0x, m_v0y);
    xmm_sum = _mm_add_ps(xmm_sum, m_v0z);

    _mm_store_ps(result, xmm_sum);
#elif SIMD_MATH_WIDTH == 8
    __m256 m_v0x = _mm256_load_ps(v0x);
    __m256 m_v0y = _mm256_load_ps(v0y);
    __m256 m_v0z = _mm256_load_ps(v0z);

    // Square
    m_v0x = _mm256_mul_ps(m_v0x, m_v0x);
    m_v0y = _mm256_mul_ps(m_v0y, m_v0y);
    m_v0z = _mm256_mul_ps(m_v0z, m_v0z);

    // Add
    __m256 xmm_sum = _mm256_add_ps(m_v0x, m_v0y);
    xmm_sum = _mm256_add_ps(xmm_sum, m_v0z);

    _mm256_store_ps(result, xmm_sum);
#endif
#else
    si_v_dot_pp(result, v0x, v0y, v0z, v0x, v0y, v0z);
#endif
}

void sip_v_lenSq_p(mfloat* result, Vec3f_Pack* v0)
{
    si_v_lenSq_p(result, &v0->x, &v0->y, &v0->z);
}

void si_v_dot_pp(mfloat* result, mfloat* v0x, mfloat* v0y, mfloat* v0z, mfloat* v1x, mfloat* v1y, mfloat* v1z)
{
    si_v_mul_pp(&tmpVec.x, &tmpVec.y, &tmpVec.z, v0x, v0y, v0z, v1x, v1y, v1z);
    si_v_sumComps_p(result, &tmpVec.x, &tmpVec.y, &tmpVec.z);
}

void sip_v_dot_pp(mfloat* result, Vec3f_Pack* v0, Vec3f_Pack* v1)
{
    sip_v_mul_pp(&tmpVec, v0, v1);
    sip_v_sumComps_p(result, &tmpVec);
}

void si_v_dot_sp(mfloat* result, Vec3f* v0, mfloat* v1x, mfloat* v1y, mfloat* v1z)
{
    /*
    si_v_mul_sp(&tmpVec.x, &tmpVec.y, &tmpVec.z, v0, v1x, v1y, v1z);
    si_v_sumComps_p(result, &tmpVec.x, &tmpVec.y, &tmpVec.z);*/

    // Optimized:
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm1 = _mm_broadcast_ss(&v0->x);
    __m128 xmm2 = _mm_load_ps(v1x);
    __m128 xmm_resultX = _mm_mul_ps(xmm1, xmm2);

    xmm1 = _mm_broadcast_ss(&v0->y);
    xmm2 = _mm_load_ps(v1y);
    __m128 xmm_resultY = _mm_mul_ps(xmm1, xmm2);

    xmm1 = _mm_broadcast_ss(&v0->z);
    xmm2 = _mm_load_ps(v1z);
    __m128 xmm_resultZ = _mm_mul_ps(xmm1, xmm2);

    __m128 xmm_sum1 = _mm_add_ps(xmm_resultX, xmm_resultY);
    xmm_sum1 = _mm_add_ps(xmm_sum1, xmm_resultZ);

    _mm_store_ps(result, xmm_sum1);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_broadcast_ss(&v0->x);
    __m256 xmm2 = _mm256_load_ps(v1x);
    __m256 xmm_resultX = _mm256_mul_ps(xmm1, xmm2);

    xmm1 = _mm256_broadcast_ss(&v0->y);
    xmm2 = _mm256_load_ps(v1y);
    __m256 xmm_resultY = _mm256_mul_ps(xmm1, xmm2);

    xmm1 = _mm256_broadcast_ss(&v0->z);
    xmm2 = _mm256_load_ps(v1z);
    __m256 xmm_resultZ = _mm256_mul_ps(xmm1, xmm2);

    __m256 xmm_sum1 = _mm256_add_ps(xmm_resultX, xmm_resultY);
    xmm_sum1 = _mm256_add_ps(xmm_sum1, xmm_resultZ);

    _mm256_store_ps(result, xmm_sum1);
#endif
#else
    si_v_mul_sp(&tmpVec.x, &tmpVec.y, &tmpVec.z, v0, v1x, v1y, v1z);
    si_v_sumComps_p(result, &tmpVec.x, &tmpVec.y, &tmpVec.z);
#endif
}

void sip_v_dot_sp(mfloat* result, Vec3f* v0, Vec3f_Pack* v1)
{
    sip_v_mul_sp(&tmpVec, v0, v1);
    sip_v_sumComps_p(result, &tmpVec);
}

void sip_v_add_sp(Vec3f_Pack* result, Vec3f* v0, Vec3f_Pack* v1)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm1 = _mm_broadcast_ss(&v0->x);
    __m128 xmm2 = _mm_load_ps(&v1->x);
    __m128 xmm_result = _mm_add_ps(xmm1, xmm2);
    _mm_store_ps(&result->x, xmm_result);

    xmm1 = _mm_broadcast_ss(&v0->y);
    xmm2 = _mm_load_ps(&v1->y);
    xmm_result = _mm_add_ps(xmm1, xmm2);
    _mm_store_ps(&result->y, xmm_result);

    xmm1 = _mm_broadcast_ss(&v0->z);
    xmm2 = _mm_load_ps(&v1->z);
    xmm_result = _mm_add_ps(xmm1, xmm2);
    _mm_store_ps(&result->z, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_broadcast_ss(&v0->x);
    __m256 xmm2 = _mm256_load_ps(&v1->x);

    __m256 xmm_result = _mm256_add_ps(xmm1, xmm2);

    _mm256_store_ps(&result->x, xmm_result);

    xmm1 = _mm256_broadcast_ss(&v0->y);
    xmm2 = _mm256_load_ps(&v1->y);

    xmm_result = _mm256_add_ps(xmm1, xmm2);

    _mm256_store_ps(&result->y, xmm_result);

    xmm1 = _mm256_broadcast_ss(&v0->z);
    xmm2 = _mm256_load_ps(&v1->z);

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
void sip_vf_add_sp(Vec3f_Pack* result, Vec3f* v0, mfloat* pack1)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm2 = _mm_load_ps(pack1);
    __m128 xmm1 = _mm_broadcast_ss(&v0->x);
    __m128 xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_store_ps(&result->x, xmm_result);

    xmm1 = _mm_broadcast_ss(&v0->y);
    xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_store_ps(&result->y, xmm_result);

    xmm1 = _mm_broadcast_ss(&v0->z);
    xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_store_ps(&result->z, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm2 = _mm256_load_ps(pack1);

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

void sip_vf_add_pp(Vec3f_Pack* result, Vec3f_Pack* v0, mfloat* pack1)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm2 = _mm_load_ps(pack1);
    __m128 xmm1 = _mm_load_ps(&v0->x);
    __m128 xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_store_ps(&result->x, xmm_result);

    xmm1 = _mm_load_ps(&v0->y);
    xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_store_ps(&result->y, xmm_result);

    xmm1 = _mm_load_ps(&v0->z);
    xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_store_ps(&result->z, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm2 = _mm256_load_ps(pack1);

    __m256 xmm1 = _mm256_load_ps(&v0->x);
    __m256 xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(&result->x, xmm_result);

    xmm1 = _mm256_load_ps(&v0->y);
    xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(&result->y, xmm_result);

    xmm1 = _mm256_load_ps(&v0->z);
    xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(&result->z, xmm_result);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        result->x[i] = v0->x[i] + pack1[i];
        result->y[i] = v0->y[i] + pack1[i];
        result->z[i] = v0->z[i] + pack1[i];
    }
#endif
}

void sip_f_mul_ps(mfloat* result, mfloat* pack1, mfloat val)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm1 = _mm_load_ps(pack1);
    __m128 xmm2 = _mm_broadcast_ss(&val);

    __m128 xmm_result = _mm_mul_ps(xmm1, xmm2);

    _mm_store_ps(result, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_load_ps(pack1);
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

void sip_f_mul_pp(mfloat* result, mfloat* pack1, mfloat* pack2)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm1 = _mm_load_ps(pack1);
    __m128 xmm2 = _mm_load_ps(pack2);

    __m128 xmm_result = _mm_mul_ps(xmm1, xmm2);

    _mm_store_ps(result, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_load_ps(pack1);
    __m256 xmm2 = _mm256_load_ps(pack2);

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

void sip_v_normalize_p(Vec3f_Pack* result, Vec3f_Pack *v0)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    sip_v_lenSq_p(&tmpFloats, v0);
    __m128 xmm_lenSq = _mm_load_ps(&tmpFloats);
    __m128 xmm_len = _mm_sqrt_ps(xmm_lenSq);
    __m128 xmm_lenMin = _mm_set_ps(0.00001f, 0.00001f, 0.00001f, 0.00001f);
    xmm_len = _mm_max_ps(xmm_len, xmm_lenMin);

    __m128 xmm_x = _mm_load_ps(v0->x);
    __m128 xmm_y = _mm_load_ps(v0->y);
    __m128 xmm_z = _mm_load_ps(v0->z);

    // Divide by len
    __m128 xmm_resX = _mm_div_ps(xmm_x, xmm_len);
    __m128 xmm_resY = _mm_div_ps(xmm_y, xmm_len);
    __m128 xmm_resZ = _mm_div_ps(xmm_z, xmm_len);

    _mm_store_ps(&result->x, xmm_resX);
    _mm_store_ps(&result->y, xmm_resY);
    _mm_store_ps(&result->z, xmm_resZ);
#elif SIMD_MATH_WIDTH == 8
    sip_v_lenSq_p(&tmpFloats, v0);
    __m256 xmm_lenSq = _mm256_load_ps(&tmpFloats);
    __m256 xmm_len = _mm256_sqrt_ps(xmm_lenSq);
    __m256 xmm_lenMin = _mm256_set_ps(0.00001f, 0.00001f, 0.00001f, 0.00001f, 0.00001f, 0.00001f, 0.00001f, 0.00001f);
    xmm_len = _mm256_max_ps(xmm_len, xmm_lenMin);

    __m256 xmm_x = _mm256_load_ps(v0->x);
    __m256 xmm_y = _mm256_load_ps(v0->y);
    __m256 xmm_z = _mm256_load_ps(v0->z);

    // Divide by len
    __m256 xmm_resX = _mm256_div_ps(xmm_x, xmm_len);
    __m256 xmm_resY = _mm256_div_ps(xmm_y, xmm_len);
    __m256 xmm_resZ = _mm256_div_ps(xmm_z, xmm_len);

    _mm256_store_ps(&result->x, xmm_resX);
    _mm256_store_ps(&result->y, xmm_resY);
    _mm256_store_ps(&result->z, xmm_resZ);
#endif
#else
    sip_v_lenSq_p(&tmpFloats, v0);
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        mfloat sqrtLen = sqrt(tmpFloats[i]);

        if (sqrtLen == 0)
            sqrtLen = 0.00001f;

        result->x[i] = v0->x[i] / sqrtLen;
        result->y[i] = v0->y[i] / sqrtLen;
        result->z[i] = v0->z[i] / sqrtLen;
    }
#endif
}

void sip_v_normalizeUnsafe_p(Vec3f_Pack* result, Vec3f_Pack *v0)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    sip_v_lenSq_p(&tmpFloats, v0);
    __m128 xmm_lenSq = _mm_load_ps(&tmpFloats);
    __m128 xmm_rsqrt = _mm_rsqrt_ps(xmm_lenSq);

    __m128 xmm_x = _mm_load_ps(v0->x);
    __m128 xmm_y = _mm_load_ps(v0->y);
    __m128 xmm_z = _mm_load_ps(v0->z);

    // Divide by len
    __m128 xmm_resX = _mm_mul_ps(xmm_x, xmm_rsqrt);
    __m128 xmm_resY = _mm_mul_ps(xmm_y, xmm_rsqrt);
    __m128 xmm_resZ = _mm_mul_ps(xmm_z, xmm_rsqrt);

    _mm_store_ps(&result->x, xmm_resX);
    _mm_store_ps(&result->y, xmm_resY);
    _mm_store_ps(&result->z, xmm_resZ);
#elif SIMD_MATH_WIDTH == 8
    sip_v_lenSq_p(&tmpFloats, v0);
    __m256 xmm_lenSq = _mm256_load_ps(&tmpFloats);
    __m256 xmm_rsqrt = _mm256_rsqrt_ps(xmm_lenSq);

    __m256 xmm_x = _mm256_load_ps(v0->x);
    __m256 xmm_y = _mm256_load_ps(v0->y);
    __m256 xmm_z = _mm256_load_ps(v0->z);

    // Divide by len
    __m256 xmm_resX = _mm256_mul_ps(xmm_x, xmm_rsqrt);
    __m256 xmm_resY = _mm256_mul_ps(xmm_y, xmm_rsqrt);
    __m256 xmm_resZ = _mm256_mul_ps(xmm_z, xmm_rsqrt);

    _mm256_store_ps(&result->x, xmm_resX);
    _mm256_store_ps(&result->y, xmm_resY);
    _mm256_store_ps(&result->z, xmm_resZ);
#endif
#else
    sip_v_lenSq_p(&tmpFloats, v0);
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        mfloat sqrtLen = sqrt(tmpFloats[i]);

        result->x[i] = v0->x[i] / sqrtLen;
        result->y[i] = v0->y[i] / sqrtLen;
        result->z[i] = v0->z[i] / sqrtLen;
    }
#endif
}

void si_v_sumComps_p(mfloat *result, mfloat* v0x, mfloat* v0y, mfloat* v0z)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm_x = _mm_load_ps(v0x);
    __m128 xmm_y = _mm_load_ps(v0y);
    __m128 xmm_z = _mm_load_ps(v0z);

    __m128 xmm_sum1 = _mm_add_ps(xmm_x, xmm_y);
    xmm_sum1 = _mm_add_ps(xmm_sum1, xmm_z);
    _mm_store_ps(result, xmm_sum1);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm_x = _mm256_load_ps(v0x);
    __m256 xmm_y = _mm256_load_ps(v0y);
    __m256 xmm_z = _mm256_load_ps(v0z);

    __m256 xmm_sum1 = _mm256_add_ps(xmm_x, xmm_y);
    xmm_sum1 = _mm256_add_ps(xmm_sum1, xmm_z);

    _mm256_store_ps(result, xmm_sum1);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
        result[i] = v0x[i] + v0y[i] + v0z[i];
#endif
}

void sip_v_sumComps_p(mfloat *result, Vec3f_Pack *v0)
{
    si_v_sumComps_p(result, &v0->x, &v0->y, &v0->z);
}

void sip_v_add_pp(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm1 = _mm_load_ps(&v0->x);
    __m128 xmm2 = _mm_load_ps(&v1->x);
    __m128 xmm_result = _mm_add_ps(xmm1, xmm2);
    _mm_store_ps(&result->x, xmm_result);

    xmm1 = _mm_load_ps(&v0->y);
    xmm2 = _mm_load_ps(&v1->y);
    xmm_result = _mm_add_ps(xmm1, xmm2);
    _mm_store_ps(&result->y, xmm_result);

    xmm1 = _mm_load_ps(&v0->z);
    xmm2 = _mm_load_ps(&v1->z);
    xmm_result = _mm_add_ps(xmm1, xmm2);
    _mm_store_ps(&result->z, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_load_ps(&v0->x);
    __m256 xmm2 = _mm256_load_ps(&v1->x);

    __m256 xmm_result = _mm256_add_ps(xmm1, xmm2);

    _mm256_store_ps(&result->x, xmm_result);

    xmm1 = _mm256_load_ps(&v0->y);
    xmm2 = _mm256_load_ps(&v1->y);

    xmm_result = _mm256_add_ps(xmm1, xmm2);

    _mm256_store_ps(&result->y, xmm_result);

    xmm1 = _mm256_load_ps(&v0->z);
    xmm2 = _mm256_load_ps(&v1->z);

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

void si_v_mul_sp(mfloat* resultX, mfloat* resultY, mfloat* resultZ, Vec3f* v0, mfloat* v1x, mfloat* v1y, mfloat* v1z)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm1 = _mm_broadcast_ss(&v0->x);
    __m128 xmm2 = _mm_load_ps(v1x);
    __m128 xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_store_ps(resultX, xmm_result);

    xmm1 = _mm_broadcast_ss(&v0->y);
    xmm2 = _mm_load_ps(v1y);
    xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_store_ps(resultY, xmm_result);

    xmm1 = _mm_broadcast_ss(&v0->z);
    xmm2 = _mm_load_ps(v1z);
    xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_store_ps(resultZ, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_broadcast_ss(&v0->x);
    __m256 xmm2 = _mm256_load_ps(v1x);
    __m256 xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(resultX, xmm_result);

    xmm1 = _mm256_broadcast_ss(&v0->y);
    xmm2 = _mm256_load_ps(v1y);
    xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(resultY, xmm_result);

    xmm1 = _mm256_broadcast_ss(&v0->z);
    xmm2 = _mm256_load_ps(v1z);
    xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(resultZ, xmm_result);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        resultX[i] = v0->x * v1x[i];
        resultY[i] = v0->y * v1y[i];
        resultZ[i] = v0->z * v1z[i];
    }
#endif
}

void sip_v_mul_sp(Vec3f_Pack* result, Vec3f* v0, Vec3f_Pack* v1)
{
    si_v_mul_sp(&result->x, &result->y, &result->z, v0, &v1->x, &v1->y, &v1->z);
}

void si_ff_mul_p(mfloat* result, mfloat* factor0, mfloat* factor1)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm1 = _mm_load_ps(factor0);
    __m128 xmm2 = _mm_load_ps(factor1);
    __m128 xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_store_ps(result, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_load_ps(factor0);
    __m256 xmm2 = _mm256_load_ps(factor1);
    __m256 xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(result, xmm_result);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        result[i] = factor0[i] * factor1[i];
    }
#endif
}

void si_v_mul_pp(mfloat* resultX, mfloat* resultY, mfloat* resultZ, mfloat* v0x, mfloat* v0y, mfloat* v0z, mfloat* v1x, mfloat* v1y, mfloat* v1z)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm1 = _mm_load_ps(v0x);
    __m128 xmm2 = _mm_load_ps(v1x);
    __m128 xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_store_ps(resultX, xmm_result);

    xmm1 = _mm_load_ps(v0y);
    xmm2 = _mm_load_ps(v1y);
    xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_store_ps(resultY, xmm_result);

    xmm1 = _mm_load_ps(v0z);
    xmm2 = _mm_load_ps(v1z);
    xmm_result = _mm_mul_ps(xmm1, xmm2);
    _mm_store_ps(resultZ, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_load_ps(v0x);
    __m256 xmm2 = _mm256_load_ps(v1x);
    __m256 xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(resultX, xmm_result);

    xmm1 = _mm256_load_ps(v0y);
    xmm2 = _mm256_load_ps(v1y);
    xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(resultY, xmm_result);

    xmm1 = _mm256_load_ps(v0z);
    xmm2 = _mm256_load_ps(v1z);
    xmm_result = _mm256_mul_ps(xmm1, xmm2);
    _mm256_store_ps(resultZ, xmm_result);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        resultX[i] = v0x[i] * v1x[i];
        resultY[i] = v0y[i] * v1y[i];
        resultZ[i] = v0z[i] * v1z[i];
    }
#endif
}

void sip_v_mul_pp(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1)
{
    si_v_mul_pp(&result->x, &result->y, &result->z, &v0->x, &v0->y, &v0->z, &v1->x, &v1->y, &v1->z);
}

void si_ff_sub_p(mfloat* result, mfloat* minuend, mfloat* subtrahend)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm1 = _mm_load_ps(minuend);
    __m128 xmm2 = _mm_load_ps(subtrahend);
    __m128 xmm_result = _mm_sub_ps(xmm1, xmm2);
    _mm_store_ps(result, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_load_ps(minuend);
    __m256 xmm2 = _mm256_load_ps(subtrahend);
    __m256 xmm_result = _mm256_sub_ps(xmm1, xmm2);
    _mm256_store_ps(result, xmm_result);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        result[i] = minuend[i] - subtrahend[i];
    }
#endif
}

SimdCompareMask si_f_any_gte(mfloat* pack, mfloat greaterThanOrEqualTo)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 data = _mm_load_ps(pack);
    __m128 comparisonVal = _mm_broadcast_ss(&greaterThanOrEqualTo);
    __m128 cmp = _mm_cmp_ps(data, comparisonVal, _CMP_GE_OQ);
    return _mm_movemask_ps(cmp);
#elif SIMD_MATH_WIDTH == 8
    __m256 data = _mm256_load_ps(pack);
    __m256 comparisonVal = _mm256_broadcast_ss(&greaterThanOrEqualTo);
    __m256 cmp = _mm256_cmp_ps(data, comparisonVal, _CMP_GE_OQ);
    return _mm256_movemask_ps(cmp);
#endif
#else
    SimdCompareMask mask = 0;
    for (int i = 0; i < SIMD_MATH_WIDTH; i++) {
        if (pack[i] >= greaterThanOrEqualTo) {
            mask |= (1 << i);
        }
    }
    return mask;
#endif
}

SimdCompareMask si_f_any_lte(mfloat* pack, mfloat lessThanOrEqual)
{
#if SIMD == 1
    #if SIMD_MATH_WIDTH == 4
    __m128 data = _mm_load_ps(pack);
    __m128 comparisonVal = _mm_broadcast_ss(&lessThanOrEqual);
    __m128 cmp = _mm_cmp_ps(data, comparisonVal, _CMP_LE_OQ);
    return _mm_movemask_ps(cmp);
#elif SIMD_MATH_WIDTH == 8
    __m256 data = _mm256_load_ps(pack);
    __m256 comparisonVal = _mm256_broadcast_ss(&lessThanOrEqual);
    __m256 cmp = _mm256_cmp_ps(data, comparisonVal, _CMP_LE_OQ);
    return _mm256_movemask_ps(cmp);
#endif
#else
    SimdCompareMask mask = 0;
    for (int i = 0; i < SIMD_MATH_WIDTH; i++) {
        if (pack[i] <= lessThanOrEqual) {
            mask |= (1 << i);
        }
    }
    return mask;
#endif
}

SimdCompareMask si_f_any_gt(mfloat* pack, mfloat greaterThan)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 data = _mm_load_ps(pack);
    __m128 comparisonVal = _mm_broadcast_ss(&greaterThan);
    __m128 cmp = _mm_cmp_ps(data, comparisonVal, _CMP_GT_OQ);
    return _mm_movemask_ps(cmp);
#elif SIMD_MATH_WIDTH == 8
    __m256 data = _mm256_load_ps(pack);
    __m256 comparisonVal = _mm256_broadcast_ss(&greaterThan);
    __m256 cmp = _mm256_cmp_ps(data, comparisonVal, _CMP_GT_OQ);
    return _mm256_movemask_ps(cmp);
#endif
#else
    SimdCompareMask mask = 0;
    for (int i = 0; i < SIMD_MATH_WIDTH; i++) {
        if (pack[i] > greaterThan) {
            mask |= (1 << i);
        }
    }
    return mask;
#endif
}

void si_v_sub_sp(mfloat* resultX, mfloat* resultY, mfloat* resultZ, Vec3f* v0, mfloat* v1x, mfloat* v1y, mfloat* v1z)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm1 = _mm_broadcast_ss(&v0->x);
    __m128 xmm2 = _mm_load_ps(v1x);
    __m128 xmm_result = _mm_sub_ps(xmm1, xmm2);
    _mm_store_ps(resultX, xmm_result);

    xmm1 = _mm_broadcast_ss(&v0->y);
    xmm2 = _mm_load_ps(v1y);
    xmm_result = _mm_sub_ps(xmm1, xmm2);
    _mm_store_ps(resultY, xmm_result);

    xmm1 = _mm_broadcast_ss(&v0->z);
    xmm2 = _mm_load_ps(v1z);
    xmm_result = _mm_sub_ps(xmm1, xmm2);
    _mm_store_ps(resultZ, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_broadcast_ss(&v0->x);
    __m256 xmm2 = _mm256_load_ps(v1x);

    __m256 xmm_result = _mm256_sub_ps(xmm1, xmm2);

    _mm256_store_ps(resultX, xmm_result);

    xmm1 = _mm256_broadcast_ss(&v0->y);
    xmm2 = _mm256_load_ps(v1y);

    xmm_result = _mm256_sub_ps(xmm1, xmm2);

    _mm256_store_ps(resultY, xmm_result);

    xmm1 = _mm256_broadcast_ss(&v0->z);
    xmm2 = _mm256_load_ps(v1z);

    xmm_result = _mm256_sub_ps(xmm1, xmm2);

    _mm256_store_ps(resultZ, xmm_result);
#endif
#else
    for (size_t i = 0; i < SIMD_MATH_WIDTH; i++)
    {
        resultX[i] = v0->x - v1x[i];
        resultY[i] = v0->y - v1y[i];
        resultZ[i] = v0->z - v1z[i];
    }
#endif
}

void sip_v_sub_sp(Vec3f_Pack* result, Vec3f* v0, Vec3f_Pack* v1)
{
    si_v_sub_sp(&result->x, &result->y, &result->z, v0, &v1->x, &v1->y, &v1->z);
}

void sip_v_sub_pp(Vec3f_Pack* result, Vec3f_Pack* v0, Vec3f_Pack* v1)
{
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
    __m128 xmm1 = _mm_load_ps(&v0->x);
    __m128 xmm2 = _mm_load_ps(&v1->x);
    __m128 xmm_result = _mm_sub_ps(xmm1, xmm2);
    _mm_store_ps(&result->x, xmm_result);

    xmm1 = _mm_load_ps(&v0->y);
    xmm2 = _mm_load_ps(&v1->y);
    xmm_result = _mm_sub_ps(xmm1, xmm2);
    _mm_store_ps(&result->y, xmm_result);

    xmm1 = _mm_load_ps(&v0->z);
    xmm2 = _mm_load_ps(&v1->z);
    xmm_result = _mm_sub_ps(xmm1, xmm2);
    _mm_store_ps(&result->z, xmm_result);
#elif SIMD_MATH_WIDTH == 8
    __m256 xmm1 = _mm256_load_ps(&v0->x);
    __m256 xmm2 = _mm256_load_ps(&v1->x);

    __m256 xmm_result = _mm256_sub_ps(xmm1, xmm2);

    _mm256_store_ps(&result->x, xmm_result);

    xmm1 = _mm256_load_ps(&v0->y);
    xmm2 = _mm256_load_ps(&v1->y);

    xmm_result = _mm256_sub_ps(xmm1, xmm2);

    _mm256_store_ps(&result->y, xmm_result);

    xmm1 = _mm256_load_ps(&v0->z);
    xmm2 = _mm256_load_ps(&v1->z);

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