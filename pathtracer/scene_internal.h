#pragma once

#include "scene.h"

void _raycast_ExchangeHit(HitInfo* currentBest, HitInfo* hitInfo, int hitCount);

void bakedSpheres_Free(BakedSpheres* spheres);
void bakedSpheres_Create(BakedSpheres* baked, Sphere* spheres, Material* materials, size_t sphereCount, size_t materialCount);
void _scene_BakeSpheres(Scene* scene, BakedScene* baked);
void scene_RaycastSpheres(HitInfo* hitInfo, size_t* hitCount, const BakedScene* scene, const Ray* ray, const mfloat minDist, const mfloat maxDist);


void bakedBoxes_Free(BakedBoxes* boxes);
void bakedBoxes_Create(BakedBoxes* baked, Box* boxes, Material* materials, size_t boxCount, size_t materialCount);
void _scene_BakeBoxes(Scene* scene, BakedScene* baked);
void scene_RaycastBoxes(HitInfo* hitInfo, size_t* hitCount, BakedScene* scene, Ray* ray, mfloat minDist, mfloat maxDist);