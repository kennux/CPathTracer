//
// Created by kennu on 19/06/2024.
//

#pragma once

#include "ptmath.h"
#include "random.h"
#include "hitinfo.h"

typedef enum MaterialType
{
    MaterialType_Lambert
} MaterialType;

typedef struct Material
{
    MaterialType type;
    Vec3f albedo;
} Material;

int material_Scatter(HitInfo* hitInfo, Material* material, Vec3f* attenuation, Ray* ray, RandomState* random);
