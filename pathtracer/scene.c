#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "ptmath.h"
#include "scene.h"
#include "material.h"

void bakedSpheres_Free(BakedSpheres* spheres)
{
    free(spheres->matIdx);
    free(spheres->center);
    free(spheres->radius);
    free(spheres->radiusReciprocal);
    free(spheres->radiusSq);
    free(spheres->boxMax);
    free(spheres->boxMin);
}

void bakedScene_Free(BakedScene* scene)
{
    bakedSpheres_Free(&scene->spheres);
    bakedMaterials_Free(&scene->materials);
}

void scene_Free(Scene* scene)
{
    free(scene->materials);
    free(scene->spheres);
}

void scene_Bake(Scene* scene, BakedScene* baked)
{
    // Bake spheres to SoA
    baked->spheres.radiusSq = malloc(sizeof(mfloat) * scene->sphereCount);
    baked->spheres.radiusReciprocal = malloc(sizeof(mfloat) * scene->sphereCount);
    baked->spheres.radius = malloc(sizeof(mfloat) * scene->sphereCount);
    baked->spheres.center = malloc(sizeof(Vec3f) * scene->sphereCount);
    baked->spheres.boxMax = malloc(sizeof(Vec3f) * scene->sphereCount);
    baked->spheres.boxMin = malloc(sizeof(Vec3f) * scene->sphereCount);
    baked->spheres.matIdx = malloc(sizeof(size_t) * scene->sphereCount);
    baked->spheres.sphereCount = scene->sphereCount;
    baked->spheres.oSphereIterationCount = scene->sphereCount / 4;
    if (baked->spheres.sphereCount % 4 != 0)
        baked->spheres.oSphereIterationCount++;

    baked->spheres.oCenter = malloc(sizeof(Vec3f_Pack4) * baked->spheres.oSphereIterationCount);

    baked->sceneBoundsMin = vec3f(MFLOAT_MAX, MFLOAT_MAX, MFLOAT_MAX);
    baked->sceneBoundsMax = vec3f(MFLOAT_MIN, MFLOAT_MIN, MFLOAT_MIN);
    for (size_t i = 0; i < scene->sphereCount; i++)
    {
        baked->spheres.radius[i] = scene->spheres[i].radius;
        baked->spheres.radiusSq[i] = scene->spheres[i].radius * scene->spheres[i].radius;
        baked->spheres.radiusReciprocal[i] = 1.0f / scene->spheres[i].radius;
        baked->spheres.center[i] = scene->spheres[i].center;
        baked->spheres.boxMax[i] = v3f_add_v3f(scene->spheres[i].center, vec3f(scene->spheres[i].radius, scene->spheres[i].radius, scene->spheres[i].radius));
        baked->spheres.boxMin[i] = v3f_sub_v3f(scene->spheres[i].center, vec3f(scene->spheres[i].radius, scene->spheres[i].radius, scene->spheres[i].radius));

        size_t matIdx = 0;
        for (size_t j = 0; j < scene->materialCount; j++)
        {
            if (&scene->materials[j] == scene->spheres[i].material)
                matIdx = j;
        }
        baked->spheres.matIdx[i] = matIdx;

        p_v3f_min(&baked->sceneBoundsMin, &baked->sceneBoundsMin, &baked->spheres.boxMin[i]);
        p_v3f_max(&baked->sceneBoundsMax, &baked->sceneBoundsMax, &baked->spheres.boxMax[i]);
    }

    // Clear prepass
    for (size_t i = 0; i < baked->spheres.oSphereIterationCount; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            baked->spheres.oCenter[i].x[j] = 0;
            baked->spheres.oCenter[i].y[j] = 0;
            baked->spheres.oCenter[i].z[j] = 0;
        }
    }

    size_t oSphereIdx = 0;
    for (size_t i = 0; i < baked->spheres.oSphereIterationCount; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            size_t mIdx = oSphereIdx + j;

            if (mIdx < scene->sphereCount) {
                baked->spheres.oCenter[i].x[j] = baked->spheres.center[mIdx].x;
                baked->spheres.oCenter[i].y[j] = baked->spheres.center[mIdx].y;
                baked->spheres.oCenter[i].z[j] = baked->spheres.center[mIdx].z;
            }
        }

        oSphereIdx += 4;
    }

    // Prep mats
    baked->materials = material_Bake(scene->materials, scene->materialCount);
    baked->ambientLight = scene->ambientLight;
}

int scene_Raycast(HitInfo* outHitInfo, BakedScene* scene, Ray* ray, mfloat minDist, mfloat maxDist)
{
    int hitCount = 0;
    HitInfo localHitInfo;

    /* SCENE BOUNDS CHECK:
    Vec3f invRayDir;
    invRayDir.x = 1.0f / ray->direction.x;
    invRayDir.y = 1.0f / ray->direction.y;
    invRayDir.z = 1.0f / ray->direction.z;

    Vec3f tMin, tMax;
    p_v3f_sub_v3f(&tMin, &scene->sceneBoundsMin, &ray->origin);
    p_v3f_mul_v3f(&tMin, &tMin, &invRayDir);
    p_v3f_sub_v3f(&tMax, &scene->sceneBoundsMax, &ray->origin);
    p_v3f_mul_v3f(&tMax, &tMax, &invRayDir);

    if (invRayDir.x < 0.0f) { float temp = tMin.x; tMin.x = tMax.x; tMax.x = temp; }
    if (invRayDir.y < 0.0f) { float temp = tMin.y; tMin.y = tMax.y; tMax.y = temp; }
    if (invRayDir.z < 0.0f) { float temp = tMin.z; tMin.z = tMax.z; tMax.z = temp; }

    float t0 = fmaxf(fmaxf(tMin.x, tMin.y), tMin.z);
    float t1 = fminf(fminf(tMax.x, tMax.y), tMax.z);
    if (t0 > t1 || t1 < 0.0f)
        return 0; // Outside of scene bounds
    */

    // Pack ray
    Vec3f_Pack4 packRayOrigin;
    Vec3f_Pack4 packRayDir;
    sp_4vec_pack_single(&packRayOrigin, &ray->origin);
    sp_4vec_pack_single(&packRayDir, &ray->direction);

    // Prep packs
    Vec3f_Pack4 packOc;
    mfloat packB[4];
    mfloat packC[4];
    Vec3f oc;
    mfloat b;

    // Spheres
    BakedSpheres spheres = scene->spheres;
    for (size_t packIdx = 0; packIdx < spheres.oSphereIterationCount; packIdx++)
    {
        // origin-center
        sp_4vec_sub(&packOc, &packRayOrigin, &spheres.oCenter[packIdx]);

        // oc.direction
        // p_v3f_dot(&b, &oc, &ray->direction);
        sp_4vec_dot(&packB, &packOc, &packRayDir);

        // p_v3f_lengthSq(&c, &oc);
        sp_4vec_lengthSq(&packC, &packOc);

        size_t itStart = packIdx * 4;
        size_t itEnd = min(itStart+4, spheres.sphereCount);
        for (size_t instIdx = 0; instIdx < 4; instIdx++)
        {
            size_t i = itStart + instIdx;
            if (i >= itEnd)
                break;

            b = packB[instIdx];
            if (b > 0)
                continue;

            // length sq of oc
            mfloat c = packC[instIdx];

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

                        hasHit = true;
                    }
                }
            }

            if (hasHit) {
                if (hitCount == 0)
                    *outHitInfo = localHitInfo;
                else {
                    if (outHitInfo->distance > localHitInfo.distance)
                        *outHitInfo = localHitInfo; // Better hit - exchange!
                }

                hitCount++;
            }
        }
    }

    return hitCount;
}