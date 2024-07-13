#include <malloc.h>
#include <stdlib.h>
#include <minmax.h>
#include "scene.h"
#include "material_internal.h"

void _material_computeLightRay_Sphere(Ray* rayIn, Ray* lightRay, HitInfo* hit, BakedScene* scene, size_t sphereIdx, RandomState* random, mfloat* lightIntensity)
{
    Vec3f sw, su, sv, sphereCenterToHit;

    // sw = sphereCenter - hitPoint
    p_v3f_sub_v3f(&sw, &scene->spheres.center[sphereIdx], &hit->point);
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

    p_v3f_sub_v3f(&sphereCenterToHit, &hit->point, &scene->spheres.center[sphereIdx]);
    mfloat hitToSphereDistSq = 0;
    p_v3f_lengthSq(&hitToSphereDistSq, &sphereCenterToHit);

    // Calculate random angle which will hit the light sphere
    mfloat cosAMax = sqrtf(1.0f - scene->spheres.radiusSq[sphereIdx] / hitToSphereDistSq);
    mfloat eps1 = random_01(random), eps2 = random_01(random);
    mfloat cosA = 1.0f - eps1 + eps1 * cosAMax;
    mfloat sinA = sqrtf(1.0f - cosA*cosA);
    mfloat phi = 2 * PI * eps2;

    // l = su * (cosf(phi) * sinA) + sv * (sinf(phi) * sinA) + sw * cosA;
    p_v3f_mul_f(&su, &su, cosf(phi) * sinA);
    p_v3f_mul_f(&sv, &sv, sinf(phi) * sinA);
    p_v3f_mul_f(&sw, &sw, cosA);
    p_v3f_add_v3f(&lightRay->direction, &su, &sw);
    p_v3f_add_v3f(&lightRay->direction, &lightRay->direction, &sv);

    lightRay->origin = hit->point;
    *lightIntensity = 2 * PI * (1 - cosAMax);
    *lightIntensity = *lightIntensity / PI;
}

void _material_computeLightRay_Box(Ray* rayIn, Ray* lightRay, HitInfo* hit, BakedScene* scene, size_t boxIdx, RandomState* random, mfloat* lightIntensity)
{
    Vec3f boxCenter, boxSizeHalf, randomPointOnBox, direction;

    // Calculate the box center and half-sizes
    boxCenter = scene->boxes.center[boxIdx];
    boxSizeHalf = scene->boxes.halfSize[boxIdx];

    // Calculate the min and max corners of the box
    Vec3f boxMin, boxMax;
    p_v3f_sub_v3f(&boxMin, &boxCenter, &boxSizeHalf);
    p_v3f_add_v3f(&boxMax, &boxCenter, &boxSizeHalf);

    // Generate a random point on the box surface
    mfloat face = random_01(random) * 6.0f; // There are 6 faces on a box
    mfloat u = random_01(random);
    mfloat v = random_01(random);
    mfloat faceArea, cosTheta;

    if (face < 1.0f) { // Front face
        randomPointOnBox.x = boxMin.x + u * (boxMax.x - boxMin.x);
        randomPointOnBox.y = boxMin.y + v * (boxMax.y - boxMin.y);
        randomPointOnBox.z = boxMax.z;
        faceArea = (boxMax.x - boxMin.x) * (boxMax.y - boxMin.y);
    } else if (face < 2.0f) { // Back face
        randomPointOnBox.x = boxMin.x + u * (boxMax.x - boxMin.x);
        randomPointOnBox.y = boxMin.y + v * (boxMax.y - boxMin.y);
        randomPointOnBox.z = boxMin.z;
        faceArea = (boxMax.x - boxMin.x) * (boxMax.y - boxMin.y);
    } else if (face < 3.0f) { // Left face
        randomPointOnBox.x = boxMin.x;
        randomPointOnBox.y = boxMin.y + u * (boxMax.y - boxMin.y);
        randomPointOnBox.z = boxMin.z + v * (boxMax.z - boxMin.z);
        faceArea = (boxMax.z - boxMin.z) * (boxMax.y - boxMin.y);
    } else if (face < 4.0f) { // Right face
        randomPointOnBox.x = boxMax.x;
        randomPointOnBox.y = boxMin.y + u * (boxMax.y - boxMin.y);
        randomPointOnBox.z = boxMin.z + v * (boxMax.z - boxMin.z);
        faceArea = (boxMax.z - boxMin.z) * (boxMax.y - boxMin.y);
    } else if (face < 5.0f) { // Top face
        randomPointOnBox.x = boxMin.x + u * (boxMax.x - boxMin.x);
        randomPointOnBox.y = boxMax.y;
        randomPointOnBox.z = boxMin.z + v * (boxMax.z - boxMin.z);
        faceArea = (boxMax.x - boxMin.x) * (boxMax.z - boxMin.z);
    } else { // Bottom face
        randomPointOnBox.x = boxMin.x + u * (boxMax.x - boxMin.x);
        randomPointOnBox.y = boxMin.y;
        randomPointOnBox.z = boxMin.z + v * (boxMax.z - boxMin.z);
        faceArea = (boxMax.x - boxMin.x) * (boxMax.z - boxMin.z);
    }

    // Calculate the direction from hit point to the random point on the box
    p_v3f_sub_v3f(&direction, &randomPointOnBox, &hit->point);
    p_v3f_normalize(&direction, &direction);

    // Set the light ray's direction and origin
    lightRay->direction = direction;
    lightRay->origin = hit->point;

    if (face < 1.0f) { // Front face
        cosTheta = fabs(direction.z);
    } else if (face < 2.0f) { // Back face
        cosTheta = fabs(direction.z);
    } else if (face < 3.0f) { // Left face
        cosTheta = fabs(direction.x);
    } else if (face < 4.0f) { // Right face
        cosTheta = fabs(direction.x);
    } else if (face < 5.0f) { // Top face
        cosTheta = fabs(direction.y);
    } else { // Bottom face
        cosTheta = fabs(direction.y);
    }

    *lightIntensity = (faceArea * cosTheta) / PI;
}

void _material_computeLightRayHit(Ray* rayIn, Ray* lightRay, HitInfo* hit, HitInfo* lightHit, mfloat lightAmtFactor, Vec3f* outLight, BakedMaterials* materials, RandomState* random)
{
    Vec3f nl, lightLocal;
    mfloat nDotL;

    p_v3f_dot(&nDotL, &hit->normal, &rayIn->direction);

    nl = hit->normal;
    if (nDotL > 0)
        p_v3f_mul_f(&nl, &nl, -1);

    p_v3f_mul_v3f(&lightLocal, &materials->albedo[hit->matIdx], &materials->emissive[lightHit->matIdx]);

    p_v3f_dot(&nDotL, &lightRay->direction, &nl);
    mfloat lightAmt = max(0, nDotL) * lightAmtFactor;

    p_v3f_mul_f(&lightLocal, &lightLocal, lightAmt);
    p_v3f_add_v3f(outLight, outLight, &lightLocal);
}

void _material_Lighting(Ray* rayIn, uint64_t* rayCount, HitInfo* hit, const BakedScene* scene, const BakedMaterials* materials, Vec3f* outLight, RandomState* random)
{
    Ray lightRay;
    HitInfo lightHit;
    mfloat lightIntensity;

    *outLight = vec3f(0,0,0);

    size_t emissiveSphereCount = scene->spheres.emissiveSphereCount;
    for (size_t i = 0; i < emissiveSphereCount; i++)
    {
        if (hit->hitObjectPtr == &scene->spheres.center[i])
            continue; // Skip self

        _material_computeLightRay_Sphere(rayIn, &lightRay, hit, scene, i, random, &lightIntensity);
        ++*rayCount;
        int hits = scene_Raycast(&lightHit, scene, &lightRay, 0.00001f, 9999999);
        if (hits > 0 && lightHit.hitObjectPtr == &scene->spheres.center[i])
        {
            _material_computeLightRayHit(rayIn, &lightRay, hit, &lightHit, lightIntensity, outLight, materials, random);
        }
    }

    size_t emissiveBoxCount = scene->boxes.emissiveBoxCount;
    for (size_t i = 0; i < emissiveBoxCount; i++)
    {
        if (hit->hitObjectPtr == &scene->boxes.center[i])
            continue; // Skip self

        _material_computeLightRay_Box(rayIn, &lightRay, hit, scene, i, random, &lightIntensity);
        ++*rayCount;
        int hits = scene_Raycast(&lightHit, scene, &lightRay, 0.00001f, 9999999);
        if (hits > 0 && lightHit.hitObjectPtr == &scene->boxes.center[i])
        {
            _material_computeLightRayHit(rayIn, &lightRay, hit, &lightHit, lightIntensity, outLight, materials, random);
        }
    }
}