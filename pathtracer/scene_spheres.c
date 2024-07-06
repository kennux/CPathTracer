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
    p_vec3f_soa(&baked->soaCenter, baked->pSphereIterationCount * SIMD_MATH_WIDTH);

    for (size_t i = 0; i < sphereCount; i++)
    {
        baked->radius[i] = spheres[i].radius;
        baked->radiusSq[i] = spheres[i].radius * spheres[i].radius;
        baked->radiusReciprocal[i] = 1.0f / spheres[i].radius;
        baked->center[i] = spheres[i].center;
        baked->boxMax[i] = v3f_add_v3f(spheres[i].center, vec3f(spheres[i].radius, spheres[i].radius, spheres[i].radius));
        baked->boxMin[i] = v3f_sub_v3f(spheres[i].center, vec3f(spheres[i].radius, spheres[i].radius, spheres[i].radius));

        size_t matIdx = 0;
        for (size_t j = 0; j < materialCount; j++)
        {
            if (&materials[j] == spheres[i].material)
                matIdx = j;
        }
        baked->matIdx[i] = matIdx;
    }
    p_vec3f_soa_fill(&baked->soaCenter, baked->center, baked->sphereCount);

    // Clear prepass
    for (size_t i = 0; i < baked->pSphereIterationCount; i++)
    {
        for (size_t j = 0; j < SIMD_MATH_WIDTH; j++)
        {
            baked->pCenter[i].x[j] = 0;
            baked->pCenter[i].y[j] = 0;
            baked->pCenter[i].z[j] = 0;
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
            }
        }

        oSphereIdx += SIMD_MATH_WIDTH;
    }
}

void _scene_BakeSpheres(Scene* scene, BakedScene* baked)
{
    size_t emissiveSphereCount = 0;
    for (size_t i = 0; i < scene->sphereCount; i++)
    {
        Vec3f emissive = scene->spheres[i].material->emissive;
        if (emissive.x > 0.0001f || emissive.y > 0.0001f || emissive.z > 0.0001f)
            emissiveSphereCount++;
    }

    size_t emissiveSphereIdx = 0;
    size_t* emissiveSpheres = malloc(sizeof(size_t) * emissiveSphereCount);
    for (size_t i = 0; i < scene->sphereCount; i++)
    {
        Vec3f emissive = scene->spheres[i].material->emissive;
        if (emissive.x > 0.0001f || emissive.y > 0.0001f || emissive.z > 0.0001f)
        {
            emissiveSpheres[emissiveSphereIdx] = i;
            emissiveSphereIdx++;
        }
    }

    baked->emissiveSpheres = emissiveSpheres;
    baked->emissiveSphereCount = emissiveSphereCount;

    bakedSpheres_Create(&baked->spheres, scene->spheres, scene->materials, scene->sphereCount, scene->materialCount);
}

int scene_RaycastSpheres(HitInfo* outHitInfo, BakedScene* scene, Ray* ray, mfloat minDist, mfloat maxDist)
{
    int hitCount = 0;
    HitInfo localHitInfo;

    // Prep packs
    SIMD_ALIGN Vec3f_Pack packOc;
    SIMD_ALIGN AlignedFloatPack packDot;
    SIMD_ALIGN AlignedFloatPack packLen;
    SIMD_ALIGN mfloat b;

    // Spheres
    BakedSpheres spheres = scene->spheres;
    for (size_t packIdx = 0; packIdx < spheres.pSphereIterationCount; packIdx++)
    {
        // origin-center
        si_v_sub_sp(&packOc.x, &packOc.y, &packOc.z, &ray->origin,
                    &spheres.soaCenter.x[packIdx * SIMD_MATH_WIDTH], &spheres.soaCenter.y[packIdx * SIMD_MATH_WIDTH], &spheres.soaCenter.z[packIdx * SIMD_MATH_WIDTH]);

        // oc.direction
        // p_v3f_dot(&b, &oc, &ray->direction);
        si_v_dot_sp(&packDot, &ray->direction, &packOc.x, &packOc.y, &packOc.z);

        // p_v3f_lengthSq(&c, &oc);
        si_v_lenSq_p(&packLen, &packOc.x, &packOc.y, &packOc.z);

        size_t itStart = packIdx * SIMD_MATH_WIDTH;
        size_t itEnd = min(itStart+SIMD_MATH_WIDTH, spheres.sphereCount);
        for (size_t instIdx = 0; instIdx < SIMD_MATH_WIDTH; instIdx++)
        {
            size_t i = itStart + instIdx;
            if (i >= itEnd)
                break;

            b = packDot[instIdx];
            if (b > 0)
                continue;

            // length sq of oc
            mfloat c = packLen[instIdx];

            bool hasHit = false;
            mfloat discriminantSqr = b * b - (c - (spheres.radiusSq[i]));
            if (discriminantSqr > 0) {
                mfloat discriminant = sqrt(discriminantSqr);
                localHitInfo.distance = (-b - discriminant);
                if (localHitInfo.distance < maxDist && localHitInfo.distance > minDist) {
                    // Calculate hit point
                    p_ray_getPoint(&localHitInfo.point, ray, localHitInfo.distance);

                    // Calculate normal
                    p_v3f_sub_v3f(&localHitInfo.normal, &localHitInfo.point, &spheres.center[i]);
                    p_v3f_mul_f(&localHitInfo.normal, &localHitInfo.normal, spheres.radiusReciprocal[i]);

                    // Set material
                    localHitInfo.matIdx = spheres.matIdx[i];
                    localHitInfo.hitObjectPtr = &spheres.center[i];

                    hasHit = true;
                } else {
                    localHitInfo.distance = (-b + discriminant);
                    if (localHitInfo.distance < maxDist && localHitInfo.distance > minDist) {
                        // Calculate hit point
                        p_ray_getPoint(&localHitInfo.point, ray, localHitInfo.distance);

                        // Calculate normal
                        p_v3f_sub_v3f(&localHitInfo.normal, &localHitInfo.point, &spheres.center[i]);
                        p_v3f_mul_f(&localHitInfo.normal, &localHitInfo.normal, spheres.radiusReciprocal[i]);

                        // Set material
                        localHitInfo.matIdx = spheres.matIdx[i];
                        localHitInfo.hitObjectPtr = &spheres.center[i];

                        hasHit = true;
                    }
                }
            }

            if (hasHit) {
                _raycast_ExchangeHit(outHitInfo, &localHitInfo, hitCount);
                hitCount++;
            }
        }
    }

    return hitCount;
}