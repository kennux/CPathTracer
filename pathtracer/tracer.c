//
// Created by kennu on 19/06/2024.
//

#include <time.h>
#include "tracer.h"

void trace(TraceParameters params, mfloat* backbuffer)
{
    random_state state = time(NULL);

    mfloat invWidth = 1.0f / params.width;
    mfloat invHeight = 1.0f / params.height;
    mfloat invSamplesPerPixel = 1.0f / (mfloat)params.samplesPerPixel;

    // RGB
    float colors[3];
    float localColors[params.samplesPerPixel * 3];

    for (int x = 0; x < params.width; x++)
    {
        for (int y = 0; y < params.height; y++)
        {
            float u = x * invWidth, v = y * invHeight;

            for (int r = 0; r < params.samplesPerPixel; r++)
            {
                // Get ray
                Ray ray;
                camera_GetRay(&ray, params.camera, u, v, &state);

                // Trace ray

            }
        }
    }
}