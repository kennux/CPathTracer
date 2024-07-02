#pragma once
#include "ptmath.h"
#include "hitinfo.h"
#include "material.h"

typedef struct BakedSpheres
{
    Vec3f* center;
    mfloat* radius;
    mfloat* radiusSq;
    mfloat* radiusReciprocal;

    Vec3f* boxMin;
    Vec3f* boxMax;

    // Optimized memory access copies
    Vec3f_Pack* oCenter;

    size_t* matIdx;

    size_t sphereCount;
    size_t oSphereIterationCount;
} BakedSpheres;

typedef struct BakedBoxes
{
    Vec3f* min;
    Vec3f* max;
    Vec3f* center;
    Vec3f* halfSize;

    Vec3f_Pack* oMin;
    Vec3f_Pack* oMax;
    Vec3f_Pack* oCenter;
    Vec3f_Pack* oHalfSize;

    size_t* matIdx;

    size_t boxCount;
    size_t oBoxIterationCount;
} BakedBoxes;

typedef struct Box
{
    Vec3f center;
    Vec3f halfSize;

    Material *material;
} Box;

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
    Box* boxes;
    size_t boxCount;
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
    BakedSpheres spheres;
    size_t* emissiveSpheres;
    size_t emissiveSphereCount;

    BakedBoxes boxes;
    size_t* emissiveBoxes;
    size_t emissiveBoxCount;

    // Lighting data
    Vec3f ambientLight;

    // Material data
    BakedMaterials materials;
} BakedScene;

void scene_Free(Scene* scene);
void bakedScene_Free(BakedScene* scene);

void scene_Bake(Scene* scene, BakedScene* baked);

/**
 * @return The amount of hits
 */
int scene_Raycast(HitInfo* outHitInfo, BakedScene* scene, Ray* ray, mfloat minDist, mfloat maxDist);