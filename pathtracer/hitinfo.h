#pragma once

typedef struct Material Material;

typedef struct HitInfo
{
    Vec3f point;
    Vec3f normal;
    float distance;

    Material *material;
} HitInfo;