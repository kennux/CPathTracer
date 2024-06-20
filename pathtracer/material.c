//
// Created by kennu on 19/06/2024.
//

#include "material.h"


int material_Scatter(HitInfo* hitInfo, Material* material, Vec3f* attenuation, Ray* ray, RandomState* random)
{
    switch (material->type)
    {
        case MaterialType_Lambert:
            Vec3f target, randomUnitV;
            random_unit_vector(&randomUnitV, random);

            p_v3f_add_v3f(&target, &hitInfo->point, &hitInfo->normal);
            p_v3f_add_v3f(&target, &target, &randomUnitV);

            ray->origin = hitInfo->point;

            p_v3f_sub_v3f(&ray->direction, &target, &hitInfo->point);
            p_v3f_normalize(&ray->direction, &ray->direction);

            *attenuation = material->albedo;
            return 1;
    }

    return 0;
}