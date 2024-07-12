#include <malloc.h>
#include <stdlib.h>
#include <minmax.h>
#include <immintrin.h>
#include "scene_internal.h"

void bakedSpheres_Free(BakedSpheres* spheres)
{
    free(spheres->matIdx);
    free(spheres->center);
    free(spheres->radius);
    free(spheres->radiusReciprocal);
    free(spheres->radiusSq);
    free(spheres->boxMax);
    free(spheres->boxMin);
    free(spheres->pRadiusSq);
    p_vec3f_soa_destroy(&spheres->soaCenter);
}

void bakedSpheres_Create(BakedSpheres* baked, Sphere* spheres, Material* materials, size_t sphereCount, size_t materialCount)
{
    // Bake spheres to SoA
    baked->radiusSq = malloc(sizeof(mfloat) * sphereCount);
    baked->radiusReciprocal = malloc(sizeof(mfloat) * sphereCount);
    baked->radius = malloc(sizeof(mfloat) * sphereCount);
    baked->center = malloc(sizeof(Vec3f) * sphereCount);
    baked->boxMax = malloc(sizeof(Vec3f) * sphereCount);
    baked->boxMin = malloc(sizeof(Vec3f) * sphereCount);
    baked->matIdx = malloc(sizeof(size_t) * sphereCount);
    baked->sphereCount = sphereCount;
    baked->pSphereIterationCount = sphereCount / SIMD_MATH_WIDTH;
    if (baked->sphereCount % SIMD_MATH_WIDTH != 0)
        baked->pSphereIterationCount++;

    baked->pCenter = malloc(sizeof(Vec3f_Pack) * baked->pSphereIterationCount);
    baked->pRadiusSq = malloc(sizeof(AlignedFloatPack) * baked->pSphereIterationCount);
    p_vec3f_soa(&baked->soaCenter, baked->pSphereIterationCount * SIMD_MATH_WIDTH);

    size_t* order = malloc(sizeof(size_t) * sphereCount);
    size_t orderPtr = 0;

    // Fill emissives
    for (size_t i = 0; i < sphereCount; i++)
    {
        Vec3f emissive = spheres[i].material->emissive;
        if (emissive.x > 0.0001f || emissive.y > 0.0001f || emissive.z > 0.0001f)
        {
            order[orderPtr++] = i;
        }
    }
    baked->emissiveSphereCount = orderPtr;

    // Fill rest
    for (size_t i = 0; i < sphereCount; i++)
    {
        Vec3f emissive = spheres[i].material->emissive;
        if (emissive.x <= 0.0001f && emissive.y <= 0.0001f && emissive.z <= 0.0001f)
        {
            order[orderPtr++] = i;
        }
    }

    for (size_t j = 0; j < sphereCount; j++)
    {
        size_t i = order[j];
        baked->radius[j] = spheres[i].radius;
        baked->radiusSq[j] = spheres[i].radius * spheres[i].radius;
        baked->radiusReciprocal[j] = 1.0f / spheres[i].radius;
        baked->center[j] = spheres[i].center;
        baked->boxMax[j] = v3f_add_v3f(spheres[i].center, vec3f(spheres[i].radius, spheres[i].radius, spheres[i].radius));
        baked->boxMin[j] = v3f_sub_v3f(spheres[i].center, vec3f(spheres[i].radius, spheres[i].radius, spheres[i].radius));

        size_t matIdx = 0;
        for (size_t k = 0; k < materialCount; k++)
        {
            if (&materials[k] == spheres[i].material)
                matIdx = k;
        }
        baked->matIdx[j] = matIdx;
    }
    p_vec3f_soa_fill(&baked->soaCenter, baked->center, baked->sphereCount);
    free(order);

    // Clear prepass
    for (size_t i = 0; i < baked->pSphereIterationCount; i++)
    {
        for (size_t j = 0; j < SIMD_MATH_WIDTH; j++)
        {
            baked->pCenter[i].x[j] = 0;
            baked->pCenter[i].y[j] = 0;
            baked->pCenter[i].z[j] = 0;
            baked->pRadiusSq[i][j] = 0;
        }
    }

    size_t oSphereIdx = 0;
    for (size_t i = 0; i < baked->pSphereIterationCount; i++)
    {
        for (size_t j = 0; j < SIMD_MATH_WIDTH; j++)
        {
            size_t mIdx = oSphereIdx + j;

            if (mIdx < sphereCount) {
                baked->pCenter[i].x[j] = baked->center[mIdx].x;
                baked->pCenter[i].y[j] = baked->center[mIdx].y;
                baked->pCenter[i].z[j] = baked->center[mIdx].z;
                baked->pRadiusSq[i][j] = baked->radiusSq[mIdx];
            }
        }

        oSphereIdx += SIMD_MATH_WIDTH;
    }
}

void _scene_BakeSpheres(Scene* scene, BakedScene* baked)
{
    bakedSpheres_Create(&baked->spheres, scene->spheres, scene->materials, scene->sphereCount, scene->materialCount);
}

void scene_RaycastSpheres(HitInfo* hitInfo, size_t* hitCount, BakedScene* scene, Ray* ray, mfloat minDist, mfloat maxDist)
{
    // Prep packs
    SIMD_ALIGN Vec3f_Pack packOc;
    SIMD_ALIGN AlignedFloatPack packDot;
    SIMD_ALIGN AlignedFloatPack packLen;

    // Spheres
    BakedSpheres* spheres = &scene->spheres;
    size_t iterationCount = spheres->pSphereIterationCount;
    for (size_t packIdx = 0; packIdx < iterationCount; packIdx++)
    {
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
        __m128 rayOriginX = _mm_broadcast_ss(&ray->origin.x);
        __m128 rayOriginY = _mm_broadcast_ss(&ray->origin.y);
        __m128 rayOriginZ = _mm_broadcast_ss(&ray->origin.z);

        __m128 centerX = _mm_load_ps(&spheres->soaCenter.x[packIdx * SIMD_MATH_WIDTH]);
        __m128 centerY = _mm_load_ps(&spheres->soaCenter.y[packIdx * SIMD_MATH_WIDTH]);
        __m128 centerZ = _mm_load_ps(&spheres->soaCenter.z[packIdx * SIMD_MATH_WIDTH]);

        __m128 originToCenterX = _mm_sub_ps(rayOriginX, centerX);
        __m128 originToCenterY = _mm_sub_ps(rayOriginY, centerY);
        __m128 originToCenterZ = _mm_sub_ps(rayOriginZ, centerZ);

        __m128 rayDirX = _mm_broadcast_ss(&ray->direction.x);
        __m128 rayDirY = _mm_broadcast_ss(&ray->direction.y);
        __m128 rayDirZ = _mm_broadcast_ss(&ray->direction.z);

        __m128 dotX = _mm_mul_ps(rayDirX, originToCenterX);
        __m128 dotY = _mm_mul_ps(rayDirY, originToCenterY);
        __m128 dotZ = _mm_mul_ps(rayDirZ, originToCenterZ);

        __m128 dot = _mm_add_ps(dotX, dotY);
        dot = _mm_add_ps(dot, dotZ);

        _mm_store_ps(&packDot, dot);
        _mm_store_ps(&packOc.x, originToCenterX);
        _mm_store_ps(&packOc.y, originToCenterY);
        _mm_store_ps(&packOc.z, originToCenterZ);

        // si_f_any_lte(&packDot, 0);
        __m128 comparisonVal = _mm_set1_ps(0);
        __m128 cmp = _mm_cmp_ps(dot, comparisonVal, _CMP_LE_OQ);
        SimdCompareMask mask = _mm_movemask_ps(cmp);
#elif SIMD_MATH_WIDTH == 8
        __m256 rayOriginX = _mm256_broadcast_ss(&ray->origin.x);
        __m256 rayOriginY = _mm256_broadcast_ss(&ray->origin.y);
        __m256 rayOriginZ = _mm256_broadcast_ss(&ray->origin.z);

        __m256 centerX = _mm256_load_ps(&spheres->soaCenter.x[packIdx * SIMD_MATH_WIDTH]);
        __m256 centerY = _mm256_load_ps(&spheres->soaCenter.y[packIdx * SIMD_MATH_WIDTH]);
        __m256 centerZ = _mm256_load_ps(&spheres->soaCenter.z[packIdx * SIMD_MATH_WIDTH]);

        __m256 originToCenterX = _mm256_sub_ps(rayOriginX, centerX);
        __m256 originToCenterY = _mm256_sub_ps(rayOriginY, centerY);
        __m256 originToCenterZ = _mm256_sub_ps(rayOriginZ, centerZ);

        __m256 rayDirX = _mm256_broadcast_ss(&ray->direction.x);
        __m256 rayDirY = _mm256_broadcast_ss(&ray->direction.y);
        __m256 rayDirZ = _mm256_broadcast_ss(&ray->direction.z);

        __m256 dotX = _mm256_mul_ps(rayDirX, originToCenterX);
        __m256 dotY = _mm256_mul_ps(rayDirY, originToCenterY);
        __m256 dotZ = _mm256_mul_ps(rayDirZ, originToCenterZ);

        __m256 dot = _mm256_add_ps(dotX, dotY);
        dot = _mm256_add_ps(dot, dotZ);

        _mm256_store_ps(&packDot, dot);
        _mm256_store_ps(&packOc.x, originToCenterX);
        _mm256_store_ps(&packOc.y, originToCenterY);
        _mm256_store_ps(&packOc.z, originToCenterZ);

        // si_f_any_lte(&packDot, 0);
        __m256 comparisonVal = _mm256_set1_ps(0);
        __m256 cmp = _mm256_cmp_ps(dot, comparisonVal, _CMP_LE_OQ);
        SimdCompareMask mask = _mm256_movemask_ps(cmp);
#endif
#else
        // origin-center
        si_v_sub_sp(&packOc.x, &packOc.y, &packOc.z, &ray->origin,
                    &spheres->soaCenter.x[packIdx * SIMD_MATH_WIDTH], &spheres->soaCenter.y[packIdx * SIMD_MATH_WIDTH], &spheres->soaCenter.z[packIdx * SIMD_MATH_WIDTH]);

        // oc.direction
        // p_v3f_dot(&b, &oc, &ray->direction);
        si_v_dot_sp(&packDot, &ray->direction, &packOc.x, &packOc.y, &packOc.z);

        SimdCompareMask mask = si_f_any_lte(&packDot, 0);
#endif

        if (mask != 0) {
            // p_v3f_lengthSq(&c, &oc);
#if SIMD == 1
#if SIMD_MATH_WIDTH == 4
            // Squared
            __m128 lenSqX = _mm_mul_ps(originToCenterX, originToCenterX);
            __m128 lenSqY = _mm_mul_ps(originToCenterY, originToCenterY);
            __m128 lenSqZ = _mm_mul_ps(originToCenterZ, originToCenterZ);

            // Add
            __m128 lenSq = _mm_add_ps(lenSqX, lenSqY);
            lenSq = _mm_add_ps(lenSq, lenSqZ);

            _mm_store_ps(&packLen, lenSq);
#elif SIMD_MATH_WIDTH == 8
            // Squared
            __m256 lenSqX = _mm256_mul_ps(originToCenterX, originToCenterX);
            __m256 lenSqY = _mm256_mul_ps(originToCenterY, originToCenterY);
            __m256 lenSqZ = _mm256_mul_ps(originToCenterZ, originToCenterZ);

            // Add
            __m256 lenSq = _mm256_add_ps(lenSqX, lenSqY);
            lenSq = _mm256_add_ps(lenSq, lenSqZ);

            _mm256_store_ps(&packLen, lenSq);
#endif
#else
            si_v_lenSq_p(&packLen, &packOc.x, &packOc.y, &packOc.z);
#endif

            size_t itStart = packIdx * SIMD_MATH_WIDTH;
            size_t itEnd = min(itStart + SIMD_MATH_WIDTH, spheres->sphereCount);
            for (size_t instIdx = 0; instIdx < SIMD_MATH_WIDTH; instIdx++) {

                if ((mask & (1 << instIdx)) == 0)
                    continue;

                size_t i = itStart + instIdx;
                if (i >= itEnd)
                    break;

                mfloat c = packLen[instIdx];
                mfloat b = packDot[instIdx];

                mfloat discriminantSqr = b * b - (c - (spheres->radiusSq[i]));
                // mfloat discriminantSqr = packDiscriminantSqr[instIdx]; // b * b - (c - spheres.radiusSq[i]);
                if (discriminantSqr > 0) {
                    mfloat discriminant = sqrt(discriminantSqr);
                    // Process both solutions of the quadratic equation
                    mfloat t1 = -b - discriminant;
                    mfloat t2 = -b + discriminant;

                    if ((t1 > minDist && t1 < maxDist) || (t2 > minDist && t2 < maxDist)) {
                        mfloat distance = (t1 > minDist && t1 < maxDist) ? t1 : t2;

                        if (*hitCount == 0 || hitInfo->distance > distance) {
                            // Calculate hit point
                            p_ray_getPoint(&hitInfo->point, ray, distance);

                            // Calculate normal
                            p_v3f_sub_v3f(&hitInfo->normal, &hitInfo->point, &spheres->center[i]);
                            p_v3f_mul_f(&hitInfo->normal, &hitInfo->normal, spheres->radiusReciprocal[i]);

                            // Set material
                            hitInfo->matIdx = spheres->matIdx[i];
                            hitInfo->hitObjectPtr = &spheres->center[i];
                            hitInfo->distance = distance;
                        }

                        ++*hitCount;
                    }
                }
            }
        }
    }
}