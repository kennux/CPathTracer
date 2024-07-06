#include <malloc.h>
#include "ptmath.h"

void p_vec3f_soa(Vec3f_SoA* result, size_t size)
{
    result->x = malloc(sizeof(mfloat) * size);
    result->y = malloc(sizeof(mfloat) * size);
    result->z = malloc(sizeof(mfloat) * size);
}
void p_vec3f_soa_fill(Vec3f_SoA* soa, Vec3f* source, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        soa->x[i] = source[i].x;
        soa->y[i] = source[i].y;
        soa->z[i] = source[i].z;
    }
}
void p_vec3f_soa_destroy(Vec3f_SoA* soa)
{
    free(soa->x);
    free(soa->y);
    free(soa->z);
}