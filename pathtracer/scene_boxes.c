#include <malloc.h>
#include "scene_internal.h"

void bakedBoxes_Free(BakedBoxes* boxes)
{
    free(boxes->matIdx);
    free(boxes->max);
    free(boxes->min);
    free(boxes->center);
    free(boxes->halfSize);
    free(boxes->pMax);
    free(boxes->pMin);
    free(boxes->pCenter);
    free(boxes->pHalfSize);
}

void bakedBoxes_Create(BakedBoxes* baked, Box* boxes, Material* materials, size_t boxCount, size_t materialCount)
{
    baked->max = malloc(sizeof(Vec3f) * boxCount);
    baked->min = malloc(sizeof(Vec3f) * boxCount);
    baked->center = malloc(sizeof(Vec3f) * boxCount);
    baked->halfSize = malloc(sizeof(Vec3f) * boxCount);
    baked->matIdx = malloc(sizeof(size_t) * boxCount);
    baked->boxCount = boxCount;
    baked->pBoxIterationCount = boxCount / SIMD_MATH_WIDTH;
    if (baked->boxCount % SIMD_MATH_WIDTH != 0)
        baked->pBoxIterationCount++;

    baked->pMax = malloc(sizeof(Vec3f_Pack) * baked->pBoxIterationCount);
    baked->pMin = malloc(sizeof(Vec3f_Pack) * baked->pBoxIterationCount);
    baked->pCenter = malloc(sizeof(Vec3f_Pack) * baked->pBoxIterationCount);
    baked->pHalfSize = malloc(sizeof(Vec3f_Pack) * baked->pBoxIterationCount);

    size_t* order = malloc(sizeof(size_t) * boxCount);
    size_t orderPtr = 0;

    // Fill emissives
    for (size_t i = 0; i < boxCount; i++)
    {
        Vec3f emissive = boxes[i].material->emissive;
        if (emissive.x > 0.0001f || emissive.y > 0.0001f || emissive.z > 0.0001f)
        {
            order[orderPtr++] = i;
        }
    }
    baked->emissiveBoxCount = orderPtr;

    // Fill rest
    for (size_t i = 0; i < boxCount; i++)
    {
        Vec3f emissive = boxes[i].material->emissive;
        if (emissive.x <= 0.0001f && emissive.y <= 0.0001f && emissive.z <= 0.0001f)
        {
            order[orderPtr++] = i;
        }
    }

    for (size_t j = 0; j < boxCount; j++)
    {
        size_t i = order[j];
        Vec3f min = boxes[i].center;
        Vec3f max = boxes[i].center;

        p_v3f_sub_v3f(&min, &min, &boxes[i].halfSize);
        p_v3f_add_v3f(&max, &max, &boxes[i].halfSize);

        baked->min[j] = min;
        baked->max[j] = max;
        baked->center[j] = boxes[i].center;
        baked->halfSize[j] = boxes[i].halfSize;

        size_t matIdx = 0;
        for (size_t k = 0; k < materialCount; k++)
        {
            if (&materials[k] == boxes[i].material)
                matIdx = k;
        }
        baked->matIdx[j] = matIdx;
    }
    free(order);

    // Clear prepass
    for (size_t i = 0; i < baked->pBoxIterationCount; i++)
    {
        for (size_t j = 0; j < SIMD_MATH_WIDTH; j++)
        {
            baked->pMax[i].x[j] = 0;
            baked->pMax[i].y[j] = 0;
            baked->pMax[i].z[j] = 0;
            baked->pMin[i].x[j] = 0;
            baked->pMin[i].y[j] = 0;
            baked->pMin[i].z[j] = 0;
            baked->pCenter[i].x[j] = 0;
            baked->pCenter[i].y[j] = 0;
            baked->pCenter[i].z[j] = 0;
            baked->pHalfSize[i].x[j] = 0;
            baked->pHalfSize[i].y[j] = 0;
            baked->pHalfSize[i].z[j] = 0;
        }
    }

    size_t oBoxIdx = 0;
    for (size_t i = 0; i < baked->pBoxIterationCount; i++)
    {
        for (size_t j = 0; j < SIMD_MATH_WIDTH; j++)
        {
            size_t mIdx = oBoxIdx + j;

            if (mIdx < boxCount) {
                baked->pMin[i].x[j] = baked->min[mIdx].x;
                baked->pMin[i].y[j] = baked->min[mIdx].y;
                baked->pMin[i].z[j] = baked->min[mIdx].z;
                baked->pMax[i].x[j] = baked->max[mIdx].x;
                baked->pMax[i].y[j] = baked->max[mIdx].y;
                baked->pMax[i].z[j] = baked->max[mIdx].z;
                baked->pCenter[i].x[j] = baked->center[mIdx].x;
                baked->pCenter[i].y[j] = baked->center[mIdx].y;
                baked->pCenter[i].z[j] = baked->center[mIdx].z;
                baked->pHalfSize[i].x[j] = baked->halfSize[mIdx].x;
                baked->pHalfSize[i].y[j] = baked->halfSize[mIdx].y;
                baked->pHalfSize[i].z[j] = baked->halfSize[mIdx].z;
            }
        }

        oBoxIdx += SIMD_MATH_WIDTH;
    }
}

void _scene_BakeBoxes(Scene* scene, BakedScene* baked)
{
    bakedBoxes_Create(&baked->boxes, scene->boxes, scene->materials, scene->boxCount, scene->materialCount);
}

void scene_RaycastBoxes(HitInfo* hitInfo, size_t* hitCount, BakedScene* scene, Ray* ray, mfloat minDist, mfloat maxDist)
{
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

        if (*hitCount == 0 || hitInfo->distance > tmin) {
            // Calculate hit point
            Vec3f hitPoint = vec3f(
                ray->origin.x + tmin * ray->direction.x,
                ray->origin.y + tmin * ray->direction.y,
                ray->origin.z + tmin * ray->direction.z
            );

            // Calculate normal
            Vec3f normal = {0.0f, 0.0f, 0.0f};
            if (hitPoint.x >= boxMax->x - MFLOAT_EPSILON) {
                normal.x = 1.0f;
            } else if (hitPoint.x <= boxMin->x + MFLOAT_EPSILON) {
                normal.x = -1.0f;
            } else if (hitPoint.y >= boxMax->y - MFLOAT_EPSILON) {
                normal.y = 1.0f;
            } else if (hitPoint.y <= boxMin->y + MFLOAT_EPSILON) {
                normal.y = -1.0f;
            } else if (hitPoint.z >= boxMax->z - MFLOAT_EPSILON) {
                normal.z = 1.0f;
            } else if (hitPoint.z <= boxMin->z + MFLOAT_EPSILON) {
                normal.z = -1.0f;
            }

            hitInfo->distance = tmin;
            hitInfo->point = hitPoint;
            hitInfo->normal = normal;
            hitInfo->hitObjectPtr = &scene->boxes.center[i];
            hitInfo->matIdx = scene->boxes.matIdx[i];
        }

        ++*hitCount;
    }
}