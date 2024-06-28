#include <malloc.h>
#include "material.h"

void bakedMaterials_Free(BakedMaterials* materials)
{
    free(materials->roughness);
    free(materials->albedo);
    free(materials->type);
}

BakedMaterials material_Bake(Material* materials, size_t count)
{
    BakedMaterials mats;
    mats.type = malloc(sizeof(MaterialType) * count);
    mats.albedo = malloc(sizeof(Vec3f) * count);
    mats.roughness = malloc(sizeof(mfloat) * count);
    mats.materialCount = count;

    for (size_t i = 0; i < count; i++)
    {
        mats.type[i] = materials[i].type;
        mats.albedo[i] = materials[i].albedo;
        mats.roughness[i] = materials[i].roughness;
    }
    return mats;
}

int material_Scatter(HitInfo* hitInfo, BakedMaterials* materials, size_t matIndex, Vec3f* attenuation, Ray* ray, RandomState* random)
{
    switch (materials->type[matIndex])
    {
        case MaterialType_Lambert: {
            Vec3f target, randomUnitV;
            random_unitVector(&randomUnitV, random);

            p_v3f_add_v3f(&target, &hitInfo->point, &hitInfo->normal);
            p_v3f_add_v3f(&target, &target, &randomUnitV);

            ray->origin = hitInfo->point;

            p_v3f_sub_v3f(&ray->direction, &target, &hitInfo->point);
            p_v3f_normalize(&ray->direction, &ray->direction);

            *attenuation = materials->albedo[matIndex];
            return 1;
        }
        case MaterialType_Metal: {
            Vec3f reflected;
            p_v3f_reflect(&reflected, &ray->direction, &hitInfo->normal);
            ray->origin = hitInfo->point;

            Vec3f randomVec;
            random_inUnitSphere(&randomVec, random);
            p_v3f_mul_f(&randomVec, &randomVec, materials->roughness[matIndex]);
            p_v3f_add_v3f(&ray->direction, &reflected, &randomVec);
            p_v3f_normalize(&ray->direction, &ray->direction);

            *attenuation = materials->albedo[matIndex];

            mfloat dot;
            p_v3f_dot(&dot, &ray->direction, &hitInfo->normal);
            if (dot > 0)
                return 1;
            break;
        }
    }

    return 0;
}