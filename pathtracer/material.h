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

typedef struct MaterialSoA
{
    MaterialType* type;

    // Generic
    Vec3f* albedo;

    // Metal
    mfloat* roughness;

    size_t materialCount;
} MaterialSoA;

int material_Scatter(HitInfo* hitInfo, MaterialSoA* materials, size_t matIndex, Vec3f* attenuation, Ray* ray, RandomState* random);

MaterialSoA material_ToSoA(Material* materials, size_t count);
void materialSoA_Free(MaterialSoA* materials);