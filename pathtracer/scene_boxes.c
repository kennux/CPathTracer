#include <malloc.h>
#include "scene_internal.h"

void bakedBoxes_Free(BakedBoxes* boxes)
{
    free(boxes->matIdx);
    free(boxes->max);
    free(boxes->min);
    free(boxes->center);
    free(boxes->halfSize);
    free(boxes->oMax);
    free(boxes->oMin);
    free(boxes->oCenter);
    free(boxes->oHalfSize);
}

void bakedBoxes_Create(BakedBoxes* baked, Box* boxes, Material* materials, size_t boxCount, size_t materialCount)
{
    baked->max = malloc(sizeof(Vec3f) * boxCount);
    baked->min = malloc(sizeof(Vec3f) * boxCount);
    baked->center = malloc(sizeof(Vec3f) * boxCount);
    baked->halfSize = malloc(sizeof(Vec3f) * boxCount);
    baked->matIdx = malloc(sizeof(size_t) * boxCount);
    baked->boxCount = boxCount;
    baked->oBoxIterationCount = boxCount / SIMD_MATH_WIDTH;
    if (baked->boxCount % SIMD_MATH_WIDTH != 0)
        baked->oBoxIterationCount++;

    baked->oMax = malloc(sizeof(Vec3f_Pack) * baked->oBoxIterationCount);
    baked->oMin = malloc(sizeof(Vec3f_Pack) * baked->oBoxIterationCount);
    baked->oCenter = malloc(sizeof(Vec3f_Pack) * baked->oBoxIterationCount);
    baked->oHalfSize = malloc(sizeof(Vec3f_Pack) * baked->oBoxIterationCount);

    for (size_t i = 0; i < boxCount; i++)
    {
        Vec3f min = boxes[i].center;
        Vec3f max = boxes[i].center;

        p_v3f_sub_v3f(&min, &min, &boxes[i].halfSize);
        p_v3f_add_v3f(&max, &max, &boxes[i].halfSize);

        baked->min[i] = min;
        baked->max[i] = max;
        baked->center[i] = boxes[i].center;
        baked->halfSize[i] = boxes[i].halfSize;

        size_t matIdx = 0;
        for (size_t j = 0; j < materialCount; j++)
        {
            if (&materials[j] == boxes[i].material)
                matIdx = j;
        }
        baked->matIdx[i] = matIdx;
    }

    // Clear prepass
    for (size_t i = 0; i < baked->oBoxIterationCount; i++)
    {
        for (size_t j = 0; j < SIMD_MATH_WIDTH; j++)
        {
            baked->oMax[i].x[j] = 0;
            baked->oMax[i].y[j] = 0;
            baked->oMax[i].z[j] = 0;
            baked->oMin[i].x[j] = 0;
            baked->oMin[i].y[j] = 0;
            baked->oMin[i].z[j] = 0;
            baked->oCenter[i].x[j] = 0;
            baked->oCenter[i].y[j] = 0;
            baked->oCenter[i].z[j] = 0;
            baked->oHalfSize[i].x[j] = 0;
            baked->oHalfSize[i].y[j] = 0;
            baked->oHalfSize[i].z[j] = 0;
        }
    }

    size_t oBoxIdx = 0;
    for (size_t i = 0; i < baked->oBoxIterationCount; i++)
    {
        for (size_t j = 0; j < SIMD_MATH_WIDTH; j++)
        {
            size_t mIdx = oBoxIdx + j;

            if (mIdx < boxCount) {
                baked->oMin[i].x[j] = baked->min[mIdx].x;
                baked->oMin[i].y[j] = baked->min[mIdx].y;
                baked->oMin[i].z[j] = baked->min[mIdx].z;
                baked->oMax[i].x[j] = baked->max[mIdx].x;
                baked->oMax[i].y[j] = baked->max[mIdx].y;
                baked->oMax[i].z[j] = baked->max[mIdx].z;
                baked->oCenter[i].x[j] = baked->center[mIdx].x;
                baked->oCenter[i].y[j] = baked->center[mIdx].y;
                baked->oCenter[i].z[j] = baked->center[mIdx].z;
                baked->oHalfSize[i].x[j] = baked->halfSize[mIdx].x;
                baked->oHalfSize[i].y[j] = baked->halfSize[mIdx].y;
                baked->oHalfSize[i].z[j] = baked->halfSize[mIdx].z;
            }
        }

        oBoxIdx += SIMD_MATH_WIDTH;
    }
}

void _scene_BakeBoxes(Scene* scene, BakedScene* baked)
{
    size_t emissiveBoxCount = 0;
    for (size_t i = 0; i < scene->boxCount; i++)
    {
        Vec3f emissive = scene->boxes[i].material->emissive;
        if (emissive.x > 0.0001f || emissive.y > 0.0001f || emissive.z > 0.0001f)
            emissiveBoxCount++;
    }

    size_t emissiveBoxIdx = 0;
    size_t* emissiveBoxes = malloc(sizeof(size_t) * emissiveBoxCount);
    for (size_t i = 0; i < scene->boxCount; i++)
    {
        Vec3f emissive = scene->boxes[i].material->emissive;
        if (emissive.x > 0.0001f || emissive.y > 0.0001f || emissive.z > 0.0001f)
        {
            emissiveBoxes[emissiveBoxIdx] = i;
            emissiveBoxIdx++;
        }
    }

    baked->emissiveBoxes = emissiveBoxes;
    baked->emissiveBoxCount = emissiveBoxCount;

    bakedBoxes_Create(&baked->boxes, scene->boxes, scene->materials, scene->boxCount, scene->materialCount);
}

int scene_RaycastBoxes(HitInfo* outHitInfo, BakedScene* scene, Ray* ray, mfloat minDist, mfloat maxDist)
{
    int hitCount = 0;
    HitInfo localHitInfo;
    Vec3f* boxMin = scene->boxes.min;
    Vec3f* boxMax = scene->boxes.max;

    for (size_t i = 0; i < scene->boxes.boxCount; i++)
    {
        float tmin = (boxMin[i].x - ray->origin.x) / ray->direction.x;
        float tmax = (boxMax[i].x - ray->origin.x) / ray->direction.x;

        if (tmin > tmax) {
            float temp = tmin;
            tmin = tmax;
            tmax = temp;
        }

        float tymin = (boxMin[i].y - ray->origin.y) / ray->direction.y;
        float tymax = (boxMax[i].y - ray->origin.y) / ray->direction.y;

        if (tymin > tymax) {
            float temp = tymin;
            tymin = tymax;
            tymax = temp;
        }

        if ((tmin > tymax) || (tymin > tmax))
            continue;

        if (tymin > tmin) {
            tmin = tymin;
        }

        if (tymax < tmax) {
            tmax = tymax;
        }

        float tzmin = (boxMin[i].z - ray->origin.z) / ray->direction.z;
        float tzmax = (boxMax[i].z - ray->origin.z) / ray->direction.z;

        if (tzmin > tzmax) {
            float temp = tzmin;
            tzmin = tzmax;
            tzmax = temp;
        }

        if ((tmin > tzmax) || (tzmin > tmax))
            continue;

        if (tzmin > tmin) {
            tmin = tzmin;
        }

        if (tzmax < tmax) {
            tmax = tzmax;
        }

        if (tmin < 0) {
            tmin = tmax;
            if (tmin < 0)
                continue;
        }

        // Calculate hit point
        Vec3f hitPoint = vec3f(
                ray->origin.x + tmin * ray->direction.x,
                ray->origin.y + tmin * ray->direction.y,
                ray->origin.z + tmin * ray->direction.z
        );

        // Calculate normal
        Vec3f normal = {0.0f, 0.0f, 0.0f};
        if (hitPoint.x >= boxMax->x - FLT_EPSILON) {
            normal.x = 1.0f;
        } else if (hitPoint.x <= boxMin->x + FLT_EPSILON) {
            normal.x = -1.0f;
        } else if (hitPoint.y >= boxMax->y - FLT_EPSILON) {
            normal.y = 1.0f;
        } else if (hitPoint.y <= boxMin->y + FLT_EPSILON) {
            normal.y = -1.0f;
        } else if (hitPoint.z >= boxMax->z - FLT_EPSILON) {
            normal.z = 1.0f;
        } else if (hitPoint.z <= boxMin->z + FLT_EPSILON) {
            normal.z = -1.0f;
        }

        localHitInfo.distance = tmin;
        localHitInfo.point = hitPoint;
        localHitInfo.normal = normal;
        localHitInfo.hitObjectPtr = &scene->boxes.center[i];
        localHitInfo.matIdx = scene->boxes.matIdx[i];

        _raycast_ExchangeHit(outHitInfo, &localHitInfo, hitCount);
        hitCount++;
    }

    return hitCount;
}