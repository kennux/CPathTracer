#include "ptmath.h"

void p_vec3f(Vec3f* out, mfloat x, mfloat y, mfloat z)
{
    out->x = x;
    out->y = y;
    out->z = z;
}
void p_v3f_add_v3f(Vec3f* out, Vec3f* v0, Vec3f* v1)
{
    out->x = v0->x + v1->x;
    out->y = v0->y + v1->y;
    out->z = v0->z + v1->z;
}
void p_v3f_sub_v3f(Vec3f* out, Vec3f* v0, Vec3f* v1)
{
    out->x = v0->x - v1->x;
    out->y = v0->y - v1->y;
    out->z = v0->z - v1->z;
}
void p_v3f_mul_v3f(Vec3f* out, Vec3f* v0, Vec3f* v1)
{
    out->x = v0->x * v1->x;
    out->y = v0->y * v1->y;
    out->z = v0->z * v1->z;
}
void p_v3f_mul_f(Vec3f* out, Vec3f* v0, mfloat v1)
{
    out->x = v0->x * v1;
    out->y = v0->y * v1;
    out->z = v0->z * v1;
}
void p_v3f_normalize(Vec3f* out, Vec3f* vec)
{
    mfloat len;
    p_v3f_length(&len, vec);

    if (len > 0.00001)
    {
        out->x = vec->x / len;
        out->y = vec->y / len;
        out->z = vec->z / len;
    }
    else
    {
        *out = *vec;
    }
}
void p_v3f_length(mfloat* out, Vec3f* vec)
{
    *out = sqrtf((vec->x * vec->x) + (vec->y * vec->y) + (vec->z * vec->z));
}
void p_v3f_lengthSq(mfloat* out, Vec3f* vec)
{
    *out = (vec->x * vec->x) + (vec->y * vec->y) + (vec->z * vec->z);
}
void p_v3f_cross(Vec3f* out, Vec3f* v0, Vec3f* v1)
{
    mfloat x = v0->y * v1->z - v0->z * v1->y;
    mfloat y = v0->z * v1->x - v0->x * v1->z;
    mfloat z = v0->x * v1->y - v0->y * v1->x;

    out->x = x;
    out->y = y;
    out->z = z;
}

void p_v3f_dot(mfloat* out, Vec3f* v0, Vec3f* v1)
{
    *out = (v0->x * v1->x)
            + (v0->y * v1->y)
            + (v0->z * v1->z);
}

bool p_v3f_refract(Vec3f* out, Vec3f* vec, Vec3f* normal, mfloat refractionIdxRatio)
{
    mfloat dt;
    p_v3f_dot(&dt, vec, normal);
    mfloat discriminant = 1.0f - refractionIdxRatio * refractionIdxRatio * (1 - dt * dt);
    if (discriminant > 0)
    {
        Vec3f tmp;
        // tmp = n * dt
        p_v3f_mul_f(&tmp, normal, dt);

        // out = vec - tmp [n * dt]
        p_v3f_sub_v3f(out, vec, &tmp);

        // out = out * refractionIdxRatio
        p_v3f_mul_f(out, out, refractionIdxRatio);

        // tmp = n * sqrt(discriminant)
        p_v3f_mul_f(&tmp, normal, sqrtf(discriminant));

        // out = out - tmp [n * sqrt(discriminant)]
        p_v3f_sub_v3f(out, out, &tmp);
        return true;
    }
    return false;
}

void p_v3f_reflect(Vec3f* out, Vec3f* vec, Vec3f* normal)
{
    mfloat dot;
    p_v3f_dot(&dot, vec, normal);

    Vec3f dotXNormal;
    p_v3f_mul_f(&dotXNormal, normal, dot);
    p_v3f_mul_f(&dotXNormal, &dotXNormal, 2.0f);
    p_v3f_sub_v3f(out, vec, &dotXNormal);
}
void p_v3f_min(Vec3f *out, Vec3f* v0, Vec3f* v1)
{
    out->x = fminf(v0->x, v1->x);
    out->y = fminf(v0->y, v1->y);
    out->z = fminf(v0->z, v1->z);
}
void p_v3f_max(Vec3f *out, Vec3f* v0, Vec3f* v1)
{
    out->x = fmaxf(v0->x, v1->x);
    out->y = fmaxf(v0->y, v1->y);
    out->z = fmaxf(v0->z, v1->z);
}

void p_v3f_clamp01(Vec3f* out, Vec3f* v0)
{
    out->x = out->x > 1 ? 1 : (out->x < 0 ? 0 : out->x);
    out->y = out->y > 1 ? 1 : (out->y < 0 ? 0 : out->y);
    out->z = out->z > 1 ? 1 : (out->z < 0 ? 0 : out->z);
}

Vec3f v3f_clamp01(Vec3f v0)
{
    Vec3f v;
    p_v3f_clamp01(&v, &v0);
    return v;
}
Vec3f vec3f(mfloat x, mfloat y, mfloat z)
{
    Vec3f v;
    p_vec3f(&v, x, y, z);
    return v;
}

Vec3f v3f_add_v3f(Vec3f v0, Vec3f v1)
{
    Vec3f result;
    p_v3f_add_v3f(&result, &v0, &v1);
    return result;
}

Vec3f v3f_sub_v3f(Vec3f v0, Vec3f v1)
{
    Vec3f result;
    p_v3f_sub_v3f(&result, &v0, &v1);
    return result;
}

Vec3f v3f_mul_v3f(Vec3f v0, Vec3f v1)
{
    Vec3f result;
    p_v3f_mul_v3f(&result, &v0, &v1);
    return result;
}

Vec3f v3f_mul_f(Vec3f v0, mfloat v1)
{
    Vec3f result;
    p_v3f_mul_f(&result, &v0, v1);
    return result;
}

Vec3f v3f_normalize(Vec3f vec)
{
    Vec3f result;
    p_v3f_normalize(&result, &vec);
    return result;
}

mfloat v3f_lengthSq(Vec3f vec)
{
    mfloat result;
    p_v3f_lengthSq(&result, &vec);
    return result;
}

mfloat v3f_length(Vec3f vec)
{
    mfloat result;
    p_v3f_length(&result, &vec);
    return result;
}

Vec3f v3f_cross(Vec3f v0, Vec3f v1)
{
    Vec3f result;
    p_v3f_cross(&result, &v0, &v1);
    return result;
}

mfloat v3f_dot(Vec3f v0, Vec3f v1)
{
    mfloat ret;
    p_v3f_dot(&ret, &v0, &v1);
    return ret;
}
Vec3f v3f_reflect(Vec3f vec, Vec3f normal)
{
    Vec3f result;
    p_v3f_reflect(&result, &vec, &normal);
    return result;
}

Vec3f v3f_min(Vec3f v0, Vec3f v1)
{
    Vec3f v;
    p_v3f_min(&v, &v0, &v1);
    return v;
}

Vec3f v3f_max(Vec3f v0, Vec3f v1)
{
    Vec3f v;
    p_v3f_max(&v, &v0, &v1);
    return v;
}