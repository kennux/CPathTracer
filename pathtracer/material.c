#include <malloc.h>
#include <stdlib.h>
#include <minmax.h>
#include "material.h"
#include "scene.h"

void bakedMaterials_Free(BakedMaterials* materials)
{
    free(materials->roughness);
    free(materials->albedo);
    free(materials->type);
    free(materials->emissive);
}

BakedMaterials material_Bake(Material* materials, size_t count)
{
    BakedMaterials mats;
    mats.type = malloc(sizeof(MaterialType) * count);
    mats.albedo = malloc(sizeof(Vec3f) * count);
    mats.roughness = malloc(sizeof(mfloat) * count);
    mats.emissive = malloc(sizeof(Vec3f) * count);
    mats.materialCount = count;

    for (size_t i = 0; i < count; i++)
    {
        mats.type[i] = materials[i].type;
        mats.albedo[i] = materials[i].albedo;
        mats.roughness[i] = materials[i].roughness;
        mats.emissive[i] = materials[i].emissive;
    }
    return mats;
}

void _material_Lighting(Ray* rayIn, HitInfo* hit, BakedScene* scene, BakedMaterials* materials, Vec3f* outLight, RandomState* random)
{
    size_t* emissiveSpheres = &scene->emissiveSpheres;

    Vec3f sw, su, sv, sphereCenterToHit;
    Ray lightRay;
    HitInfo lightHit;

    *outLight = vec3f(0,0,0);

    Vec3f nl, lightLocal;
    for (size_t j = 0; j < scene->emissiveSphereCount; j++)
    {
        size_t i = scene->emissiveSpheres[j];

        // sw = sphereCenter - hitPoint
        p_v3f_sub_v3f(&sw, &scene->spheres.center[i], &hit->point);
        // sw = normalize(sw)
        p_v3f_normalize(&sw, &sw);

        // su = abs(sw.x) > 0.01 ? vec3f(0,1,0) : vec3f(1,0,0)
        su = fabs(sw.x) > 0.01 ? vec3f(0,1,0) : vec3f(1,0,0);
        // su = cross(su, sw)
        p_v3f_cross(&su, &su, &sw);
        // su = normalize(su)
        p_v3f_normalize(&su, &su);

        // sv = cross(sw, su)
        p_v3f_cross(&sv, &sw, &su);

        p_v3f_sub_v3f(&sphereCenterToHit, &hit->point, &scene->spheres.center[i]);
        mfloat hitToSphereDistSq = 0;
        p_v3f_lengthSq(&hitToSphereDistSq, &sphereCenterToHit);

        mfloat cosAMax = sqrtf(1.0f - scene->spheres.radius[i]*scene->spheres.radius[i] / hitToSphereDistSq);
        mfloat eps1 = random_01(random), eps2 = random_01(random);
        mfloat cosA = 1.0f - eps1 + eps1 * cosAMax;
        mfloat sinA = sqrtf(1.0f - cosA*cosA);
        mfloat phi = 2 * PI * eps2;

        // l = su * (cosf(phi) * sinA) + sv * (sinf(phi) * sinA) + sw * cosA;
        p_v3f_mul_f(&su, &su, cosf(phi) * sinA);
        p_v3f_mul_f(&sv, &sv, sinf(phi) * sinA);
        p_v3f_mul_f(&sw, &sw, cosA);
        p_v3f_add_v3f(&lightRay.direction, &su, &sw);
        p_v3f_add_v3f(&lightRay.direction, &lightRay.direction, &sv);

        lightRay.origin = hit->point;
        int hits = scene_Raycast(&lightHit, scene, &lightRay, 0.00001f, 9999999);
        if (hits > 0 && hit->hitObjectPtr == &scene->spheres.center[i])
        {
            mfloat omega = 2 * PI * (1 - cosAMax);

            mfloat nDotL;
            p_v3f_dot(&nDotL, &hit->normal, &rayIn->direction);

            nl = hit->normal;
            if (nDotL > 0)
                p_v3f_mul_f(&nl, &nl, -1);

            p_v3f_mul_v3f(&lightLocal, &materials->albedo[hit->matIdx], &materials->emissive[lightHit.matIdx]);

            p_v3f_dot(&nDotL, &lightRay.direction, &nl);
            mfloat lightAmt = max(0, nDotL) * omega / PI;

            p_v3f_mul_f(&lightLocal, &lightLocal, lightAmt);
            p_v3f_add_v3f(outLight, outLight, &lightLocal);
        }
    }
}

int material_Scatter(HitInfo* hitInfo, BakedScene* scene, BakedMaterials* materials, size_t matIndex, Vec3f* attenuation, Ray* ray, uint64_t* rayCount, RandomState* random)
{
    switch (materials->type[matIndex])
    {
        case MaterialType_Lambert: {
            Vec3f target, randomUnitV;
            random_unitVector(&randomUnitV, random);

            Vec3f light = vec3f(0,0,0);
            _material_Lighting(ray, hitInfo, scene, materials, &light, random);

            p_v3f_add_v3f(&target, &hitInfo->point, &hitInfo->normal);
            p_v3f_add_v3f(&target, &target, &randomUnitV);

            ray->origin = hitInfo->point;

            p_v3f_sub_v3f(&ray->direction, &target, &hitInfo->point);
            p_v3f_normalize(&ray->direction, &ray->direction);

            p_v3f_add_v3f(attenuation, &materials->albedo[matIndex], &light);
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
        case MaterialType_Emissive: {
            *attenuation = materials->emissive[matIndex];
            return 0;
        }
    }

    return 0;
}