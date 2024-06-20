//
// Created by kennu on 19/06/2024.
//

#include "material.h"


int material_Scatter(HitInfo* hitInfo, Material* material, Vec3f* attenuation, Ray* ray, RandomState* random)
{
    switch (material->type)
    {
        case MaterialType_Lambert: {
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
        case MaterialType_Metal: {
            Vec3f reflected;
            p_v3f_reflect(&reflected, &ray->direction, &hitInfo->normal);
            ray->origin = hitInfo->point;

            Vec3f randomVec;
            random_in_unit_sphere(&randomVec, random);
            p_v3f_mul_f(&randomVec, &randomVec, material->roughness);
            p_v3f_add_v3f(&ray->direction, &reflected, &randomVec);
            p_v3f_normalize(&ray->direction, &ray->direction);

            *attenuation = material->albedo;

            mfloat dot;
            p_v3f_dot(&dot, &ray->direction, &hitInfo->normal);
            if (dot > 0)
                return 1;
            break;
        }
    }

    return 0;
}