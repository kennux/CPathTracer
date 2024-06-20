#include "ptmath.h"

void p_ray_getPoint(Vec3f* point, Ray* ray, mfloat d)
{
    point->x = ray->origin.x + (d * ray->direction.x);
    point->y = ray->origin.y + (d * ray->direction.y);
    point->z = ray->origin.z + (d * ray->direction.z);
}

Vec3f ray_getPoint(Ray* ray, mfloat d)
{
    Vec3f point;
    p_ray_getPoint(&point, ray, d);
    return point;
}