#pragma once

#include "material.h"

void _material_Lighting(Ray* rayIn, uint64_t* rayCount, HitInfo* hit, const BakedScene* scene, const BakedMaterials* materials, Vec3f* outLight, RandomState* random);