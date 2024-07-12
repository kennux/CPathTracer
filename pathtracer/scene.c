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
    bakedBoxes_Free(&scene->boxes);
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
    outHitInfo->distance = MFLOAT_MAX;
    size_t hitCount = 0;

    scene_RaycastSpheres(outHitInfo, &hitCount, scene, ray, minDist, maxDist);
    scene_RaycastBoxes(outHitInfo, &hitCount, scene, ray, minDist, maxDist);

    return hitCount;
}