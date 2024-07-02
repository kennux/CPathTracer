#include <malloc.h>
#include <stdlib.h>
#include <minmax.h>
#include "material_internal.h"
#include "scene.h"

void bakedMaterials_Free(BakedMaterials* materials)
{
    free(materials->roughness);
    free(materials->albedo);
    free(materials->type);
    free(materials->emissive);
    free(materials->ri);
}

BakedMaterials material_Bake(Material* materials, size_t count)
{
    BakedMaterials mats;
    mats.type = malloc(sizeof(MaterialType) * count);
    mats.albedo = malloc(sizeof(Vec3f) * count);
    mats.roughness = malloc(sizeof(mfloat) * count);
    mats.ri = malloc(sizeof(mfloat) * count);
    mats.emissive = malloc(sizeof(Vec3f) * count);
    mats.materialCount = count;

    for (size_t i = 0; i < count; i++)
    {
        mats.type[i] = materials[i].type;
        mats.albedo[i] = materials[i].albedo;
        mats.roughness[i] = materials[i].roughness;
        mats.emissive[i] = materials[i].emissive;
        mats.ri[i] = materials[i].ri;
    }
    return mats;
}

int material_Scatter(HitInfo* hitInfo, BakedScene* scene, BakedMaterials* materials, size_t matIndex, Vec3f* attenuation, Vec3f* light, Vec3f* emissive, Ray* ray, uint64_t* rayCount, RandomState* random)
{
    switch (materials->type[matIndex])
    {
        case MaterialType_Lambert: {
            Vec3f target, randomUnitV;
            random_unitVector(&randomUnitV, random);

            Vec3f _light = vec3f(0,0,0);
            _material_Lighting(ray, rayCount, hitInfo, scene, materials, &_light, random);

            p_v3f_add_v3f(&target, &hitInfo->point, &hitInfo->normal);
            p_v3f_add_v3f(&target, &target, &randomUnitV);

            ray->origin = hitInfo->point;

            p_v3f_sub_v3f(&ray->direction, &target, &hitInfo->point);
            p_v3f_normalize(&ray->direction, &ray->direction);

            *attenuation = materials->albedo[matIndex];
            *light = _light;
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
        case MaterialType_Dielectric: {
            Vec3f outwardNorm, refracted, reflected;
            mfloat refrIdxRatio;
            mfloat ri = materials->ri[hitInfo->matIdx];

            p_v3f_reflect(&reflected, &ray->direction, &hitInfo->normal);

            *attenuation = vec3f(1,1,1);

            mfloat dt;
            p_v3f_dot(&dt, &ray->direction, &hitInfo->normal);
            if (dt > 0)
            {
                p_v3f_mul_f(&outwardNorm, &hitInfo->normal, -1);
                refrIdxRatio = ri;
            }
            else
            {
                outwardNorm = hitInfo->normal;
                refrIdxRatio = 1.0f / ri;
            }

            mfloat reflProb = 1;
            if (p_v3f_refract(&refracted, &ray->direction, &outwardNorm, refrIdxRatio))
            {
                if (dt > 0)
                {
                    mfloat discriminant = 1-ri * ri * (1 - dt * dt);

                    if (discriminant > 0.f)
                    {
                        float r0 = pow((1.f - ri) / (1.f + ri), 2);
                        reflProb = r0 + (1.f - r0) * pow(1.f - sqrtf(discriminant), 5);
                    }
                }
                else
                {
                    float r0 = pow((1.f - ri) / (1.f + ri), 2);
                    reflProb = r0 + (1.f - r0) * pow(1.f + dt, 5);
                }
            }

            ray->origin = hitInfo->point;
            if (random_01(random) < reflProb)
                p_v3f_normalize(&ray->direction, &reflected);
            else
                p_v3f_normalize(&ray->direction, &refracted);

            return 1;
        }
        case MaterialType_Emissive: {
            *emissive = materials->emissive[matIndex];
            return 0;
        }
    }

    return 0;
}