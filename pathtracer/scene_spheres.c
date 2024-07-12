#include <malloc.h>
#include <stdlib.h>
#include <minmax.h>
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
        // origin-center
        si_v_sub_sp(&packOc.x, &packOc.y, &packOc.z, &ray->origin,
                    &spheres->soaCenter.x[packIdx * SIMD_MATH_WIDTH], &spheres->soaCenter.y[packIdx * SIMD_MATH_WIDTH], &spheres->soaCenter.z[packIdx * SIMD_MATH_WIDTH]);

        // oc.direction
        // p_v3f_dot(&b, &oc, &ray->direction);
        si_v_dot_sp(&packDot, &ray->direction, &packOc.x, &packOc.y, &packOc.z);

        SimdCompareMask mask = si_f_any_lte(&packDot, 0);

        if (mask != 0) {
            // p_v3f_lengthSq(&c, &oc);
            si_v_lenSq_p(&packLen, &packOc.x, &packOc.y, &packOc.z);

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