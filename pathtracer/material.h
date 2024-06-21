#pragma once

#include "ptmath.h"
#include "random.h"
#include "hitinfo.h"

typedef enum MaterialType
{
    MaterialType_Lambert,
    MaterialType_Metal
} MaterialType;

typedef struct Material
{
    MaterialType type;
    Vec3f albedo;

    // Metal
    mfloat roughness;
} Material;

typedef struct BakedMaterials
{
    MaterialType* type;

    // Generic
    Vec3f* albedo;

    // Metal
    mfloat* roughness;

    size_t materialCount;
} BakedMaterials;

int material_Scatter(HitInfo* hitInfo, BakedMaterials* materials, size_t matIndex, Vec3f* attenuation, Ray* ray, RandomState* random);

BakedMaterials material_Bake(Material* materials, size_t count);
void bakedMaterials_Free(BakedMaterials* materials);