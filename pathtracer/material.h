//
// Created by kennu on 19/06/2024.
//

#include "ptmath.h"

#ifndef CPATHTRACER_MATERIAL_H
#define CPATHTRACER_MATERIAL_H

typedef enum MaterialType
{
    Lambert
} MaterialType;

typedef struct Material
{
    MaterialType type;
    Vec3f albedo;
} Material;

#endif //CPATHTRACER_MATERIAL_H
