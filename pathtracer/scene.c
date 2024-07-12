#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <minmax.h>
#include "ptmath.h"
#include "scene_internal.h"
#include "material.h"

void bakedScene_Free(BakedScene* scene)
{
    bakedSpheres_Free(&scene->spheres);
    bakedMaterials_Free(&scene->materials);
}

void scene_Free(Scene* scene)
{
    free(scene->materials);
    free(scene->spheres);
    free(scene->boxes);
}

void scene_Bake(Scene* scene, BakedScene* baked)
{
    _scene_BakeSpheres(scene, baked);
    _scene_BakeBoxes(scene, baked);

    // Prep mats
    baked->materials = material_Bake(scene->materials, scene->materialCount);
    baked->ambientLight = scene->ambientLight;
}

int scene_Raycast(HitInfo* outHitInfo, BakedScene* scene, Ray* ray, mfloat minDist, mfloat maxDist)
{
    outHitInfo->distance = -1;
    HitInfo localHit;
    int totalHitCount = 0;

    int hitCountSpheres = scene_RaycastSpheres(&localHit, scene, ray, minDist, maxDist);
    totalHitCount += hitCountSpheres;
    if (hitCountSpheres > 0)
        *outHitInfo = localHit;

    int hitCountBoxes = scene_RaycastBoxes(&localHit, scene, ray, minDist, maxDist);
    totalHitCount += hitCountBoxes;
    if (hitCountBoxes > 0) {
        if (outHitInfo->distance == -1)
            *outHitInfo = localHit;
        else if (outHitInfo->distance != -1)
            _raycast_ExchangeHit(outHitInfo, &localHit, hitCountBoxes + hitCountSpheres);
    }

    return totalHitCount;
}