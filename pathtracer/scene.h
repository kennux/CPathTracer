//
// Created by kennu on 19/06/2024.
//

#pragma once
#include "ptmath.h"
#include "hitinfo.h"

typedef struct Material Material;

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

/**
 * @return The amount of hits
 */
int scene_Raycast(HitInfo* outHitInfo, Scene* scene, Ray* ray, mfloat minDist, mfloat maxDist);