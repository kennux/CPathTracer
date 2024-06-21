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

typedef struct SphereSoA
{
    Vec3f* center;
    mfloat* radius;
    mfloat* radiusSq;
    mfloat* radiusReciprocal;

    Material** material;

    size_t sphereCount;
} SphereSoA;

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

typedef struct BakedScene
{
    // Scene data
    SphereSoA spheres;

    // Lighting data
    Vec3f ambientLight;

    // Material data
    Material* materials;
    size_t materialCount;
} BakedScene;

void scene_Free(Scene* scene);
void sphereSoA_Free(SphereSoA* spheres);
void bakedScene_Free(BakedScene* scene);

void scene_Bake(Scene* scene, BakedScene* baked);

/**
 * @return The amount of hits
 */
int scene_Raycast(HitInfo* outHitInfo, BakedScene* scene, Ray* ray, mfloat minDist, mfloat maxDist);