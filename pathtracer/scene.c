#include <stdbool.h>
#include <malloc.h>
#include <string.h>
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
    }

    // Prep mats
    baked->materials = material_Bake(scene->materials, scene->materialCount);
    baked->ambientLight = scene->ambientLight;
}

int scene_Raycast(HitInfo* outHitInfo, BakedScene* scene, Ray* ray, mfloat minDist, mfloat maxDist)
{
    int hitCount = 0;
    HitInfo localHitInfo;

    Vec3f invRayDir;
    invRayDir.x = 1.0f / ray->direction.x;
    invRayDir.y = 1.0f / ray->direction.y;
    invRayDir.z = 1.0f / ray->direction.z;

    // Spheres
    BakedSpheres spheres = scene->spheres;
    for (size_t i = 0; i < spheres.sphereCount; i++)
    {
        /*Vec3f tMin, tMax;
        p_v3f_sub_v3f(&tMin, &scene->spheres.boxMin[i], &ray->origin);
        p_v3f_mul_v3f(&tMin, &tMin, &invRayDir);
        p_v3f_sub_v3f(&tMax, &scene->spheres.boxMax[i], &ray->origin);
        p_v3f_mul_v3f(&tMax, &tMax, &invRayDir);

        if (invRayDir.x < 0.0f) { float temp = tMin.x; tMin.x = tMax.x; tMax.x = temp; }
        if (invRayDir.y < 0.0f) { float temp = tMin.y; tMin.y = tMax.y; tMax.y = temp; }
        if (invRayDir.z < 0.0f) { float temp = tMin.z; tMin.z = tMax.z; tMax.z = temp; }

        float t0 = fmaxf(fmaxf(tMin.x, tMin.y), tMin.z);
        float t1 = fminf(fminf(tMax.x, tMax.y), tMax.z);*/

        bool hasHit = false;

        // Box was hit
        //if (t0 <= t1 && t1 >= 0.0f)
        {
            Vec3f oc;
            // origin - center
            p_v3f_sub_v3f(&oc, &ray->origin, &spheres.center[i]);

            // oc.direction
            mfloat b;
            p_v3f_dot(&b, &oc, &ray->direction);

            if (b > 0)
                continue;

            // length sq of oc
            mfloat c;
            p_v3f_lengthSq(&c, &oc);

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
        }

        if (hasHit)
        {
            if (hitCount == 0)
                *outHitInfo = localHitInfo;
            else
            {
                if (outHitInfo->distance > localHitInfo.distance)
                    *outHitInfo = localHitInfo; // Better hit - exchange!
            }

            hitCount++;
        }
    }

    return hitCount;
}