#pragma once

#include <stdint.h>
#include "ptmath.h"
#include "random.h"
#include "hitinfo.h"

typedef struct BakedScene BakedScene;

typedef enum MaterialType
{
    MaterialType_Lambert,
    MaterialType_Metal,
    MaterialType_Emissive,
    MaterialType_Dielectric
} MaterialType;

typedef struct Material
{
    MaterialType type;
    Vec3f albedo;
    Vec3f emissive;

    // Metal
    mfloat roughness;

    // Dielectric
    mfloat ri;
} Material;

typedef struct BakedMaterials
{
    MaterialType* type;

    // Generic
    Vec3f* albedo;
    Vec3f* emissive;

    // Metal
    mfloat* roughness;

    // Dielectric
    mfloat* ri;

    size_t materialCount;
} BakedMaterials;

int material_Scatter(const HitInfo* hitInfo, const BakedScene* scene, const BakedMaterials* materials, const size_t matIndex, Vec3f* attenuation, Vec3f* light, Vec3f* emissive, Ray* ray, uint64_t* rayCount, RandomState* random);

BakedMaterials material_Bake(Material* materials, size_t count);
void bakedMaterials_Free(BakedMaterials* materials);