//
// Created by kennu on 19/06/2024.
//

#include <time.h>
#include <malloc.h>
#include "tracer.h"

void trace(TraceParameters params, mfloat* backbuffer)
{
    RandomState rand = time(NULL);

    mfloat invWidth = 1.0f / params.backbufferWidth;
    mfloat invHeight = 1.0f / params.backbufferHeight;
    mfloat invSamplesPerPixel = 1.0f / (mfloat)params.samplesPerPixel;

    // RGB
    Vec3f color;
    Vec3f* colors = malloc(sizeof(Vec3f) * params.samplesPerPixel);

    for (int x = 0; x < params.backbufferWidth; x++)
    {
        for (int y = 0; y < params.backbufferHeight; y++)
        {
            color = params.scene->ambientLight;
            mfloat u = x * invWidth, v = y * invHeight;
            int colorIndex = (y * params.backbufferWidth + x) * 4;

            for (int r = 0; r < params.samplesPerPixel; r++)
            {
                // Get ray
                Ray ray;
                camera_GetRay(&ray, params.camera, u, v, &rand);

                Vec3f localColor = params.scene->ambientLight;
                size_t bounceCount = 0;
                while (bounceCount < params.maxBounces)
                {
                    // Trace ray
                    HitInfo hitInfo;
                    int hits = scene_Raycast(&hitInfo, params.scene, &ray, 0.01, params.maxDepth);
                    if (hits > 0)
                    {
                        Vec3f attenuation;
                        int scatter = material_Scatter(&hitInfo, hitInfo.material, &attenuation, &ray, &rand);
                        p_v3f_mul_v3f(&localColor, &attenuation, &localColor);

                        if (scatter == 0)
                            break;
                    }
                    else
                        break;

                    bounceCount++;
                }

                colors[r] = localColor;
            }

            color = vec3f(0,0,0);
            for (size_t i = 0; i < params.samplesPerPixel; i++)
            {
                p_v3f_add_v3f(&color, &colors[i], &color);
            }
            p_v3f_mul_f(&color, &color, invSamplesPerPixel);
            backbuffer[colorIndex + 0] = color.x;
            backbuffer[colorIndex + 1] = color.y;
            backbuffer[colorIndex + 2] = color.z;
            backbuffer[colorIndex + 3] = 1;
        }
    }

    free(colors);
}