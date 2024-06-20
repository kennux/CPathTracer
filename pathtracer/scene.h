//
// Created by kennu on 19/06/2024.
//

#include "ptmath.h"
#include "material.h"

#ifndef CPATHTRACER_SCENE_H
#define CPATHTRACER_SCENE_H

typedef struct Sphere
{
    Vec3f center;
    mfloat radius;

    Material* material;
} Sphere;

typedef struct Scene
{
    // Scene data
    Sphere* spheres;
    size_t sphereCount;

    // Lighting data
    Vec3f ambientLight;

    // Material data
    Material* materials;
    size_t materialCount;
} Scene;

typedef struct HitInfo
{
    Vec3f point;
    Vec3f normal;
    float distance;

    Material* material;
} HitInfo;

/**
 * @return The amount of hits
 */
int scene_Raycast(HitInfo* outHitInfo, Scene* scene, Ray* ray, mfloat minDist, mfloat maxDist);

#endif //CPATHTRACER_SCENE_H
