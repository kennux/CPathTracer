#include "GLFW/glfw3.h"
#include "camera.h"
#include "tracer.h"
#include "bmpwriter.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

// Callback function to handle window resizing

void checkOpenGLError() {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL error: %u\n", err);
    }
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void progressCallback(float progress)
{
    printf("%.2f %% Done...\n", progress * 100.0f);
}

Scene createScene()
{
    Scene scene;

    // Create materials
    scene.materials = malloc(sizeof(Material) * 5);
    scene.materialCount = 5;
    Material* materialLambert1 = &scene.materials[0];
    materialLambert1->albedo = vec3f(0.8f, 0.4f, 0.4f);
    materialLambert1->emissive = vec3f(0,0,0);
    materialLambert1->type = MaterialType_Lambert;
    Material* materialLambert2 = &scene.materials[1];
    materialLambert2->albedo = vec3f(0.8f, 0.8f, 0.8f);
    materialLambert2->emissive = vec3f(0,0,0);
    materialLambert2->type = MaterialType_Lambert;
    Material* materialMetal = &scene.materials[2];
    materialMetal->albedo = vec3f(0.75f, 0.75f, 0.75f);
    materialMetal->emissive = vec3f(0,0,0);
    materialMetal->roughness = 0.025f;
    materialMetal->type = MaterialType_Metal;
    Material* materialEmissive = &scene.materials[3];
    materialEmissive->albedo = vec3f(1, 1, 1);
    materialEmissive->emissive = vec3f(4, 4, 4);
    materialEmissive->type = MaterialType_Emissive;
    Material* materialDielectric = &scene.materials[4];
    materialDielectric->albedo = vec3f(1, 1, 1);
    materialDielectric->ri = 1.5;
    materialDielectric->type = MaterialType_Dielectric;

    // Create spheres
    scene.spheres = malloc(sizeof(Sphere) * 59);
    scene.sphereCount = 9;
    Sphere* editSphere = &scene.spheres[0];
    editSphere->radius = 100.0f;
    editSphere->center = vec3f(0, -100.5f, -1);
    editSphere->material = materialLambert2;
    editSphere = &scene.spheres[1];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(2, 0, -1);
    editSphere->material = materialLambert1;
    editSphere = &scene.spheres[2];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(0, 0, -1);
    editSphere->material = materialLambert1;
    editSphere = &scene.spheres[3];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(-2, 0, -1);
    editSphere->material = materialMetal;
    editSphere = &scene.spheres[4];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(2, 0, 1);
    editSphere->material = materialLambert1;
    editSphere = &scene.spheres[5];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(0, 0, 1);
    editSphere->material = materialLambert1;
    editSphere = &scene.spheres[6];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(-2, 0, 1);
    editSphere->material = materialLambert1;
    editSphere = &scene.spheres[7];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(0, 2.25f, 0.5f);
    editSphere->material = materialEmissive;
    editSphere = &scene.spheres[8];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(0.25f, 1.f, 0);
    editSphere->material = materialDielectric;

    for (size_t i = 0; i < 50; i++)
    {
        int row = i % 10;
        int col = floor((mfloat)i / 10.0f);
        mfloat x = -4.0f + (row);
        mfloat z = -2.0f - (col);

        editSphere = &scene.spheres[i+9];
        editSphere->radius = 0.5f;
        editSphere->center = vec3f(x, 0, z);
        editSphere->material = i % 2 == 0 ? materialLambert1 : materialMetal;
    }
    /*editSphere->radius = 100;
    editSphere->center = vec3f(0, -100.5f, -1);
    editSphere->materialLambert1 = materialLambert1;*/

    // Setup lighting
    scene.ambientLight = vec3f(0.75f, 0.75f, 0.75f);
    scene.ambientLight = vec3f(0.f, 0.f, 0.f);
    scene.ambientLight = vec3f(0.01f, 0.01f, 0.01f);

    return scene;
}

void transformBackbufferToTexData(mfloat* backbufferData, char* textureData, size_t textureWidth, size_t textureHeight)
{
    for (int x = 0; x < textureWidth; x++)
    {
        for (int y = 0; y < textureHeight; y++)
        {
            int index = (y * textureWidth + x) * 4;

            // Linear -> gamma approximation with sqrt
            char r = (char)(255.99f * sqrt(backbufferData[index + 0]));
            char g = (char)(255.99f * sqrt(backbufferData[index + 1]));
            char b = (char)(255.99f * sqrt(backbufferData[index + 2]));
            char a = (char)(255.0f * backbufferData[index + 3]);

            textureData[index + 0] = r; // R
            textureData[index + 1] = g; // G
            textureData[index + 2] = b; // B
            textureData[index + 3] = a; // A
        }
    }
}

void transformBackbufferToBmpData(mfloat* backbufferData, char* bmpData, size_t textureWidth, size_t textureHeight)
{
    for (int x = 0; x < textureWidth; x++)
    {
        for (int y = 0; y < textureHeight; y++)
        {
            int index = (y * textureWidth + x) * 4;
            int bmpIndex = (y * textureWidth + x) * 3;

            // Linear -> gamma approximation with sqrt
            char r = (char)(255.99f * sqrt(backbufferData[index + 0]));
            char g = (char)(255.99f * sqrt(backbufferData[index + 1]));
            char b = (char)(255.99f * sqrt(backbufferData[index + 2]));
            char a = (char)(255.0f * backbufferData[index + 3]);

            // Bmp format is more special than me
            // It's actually BGR...
            bmpData[bmpIndex + 2] = r; // R
            bmpData[bmpIndex + 1] = g; // G
            bmpData[bmpIndex + 0] = b; // B
        }
    }
}

#define INITIAL_SAMPLES 8
#define SAMPLES_PER_ITERATION 8

int main(void) {
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }

    /* Create a windowed mode window and its OpenGL context */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    window = glfwCreateWindow(640, 480, "CPathTracer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glClearColor(0, 0, 0, 0.0f);

    // Backbuffer
    int textureWidth = 2560;
    int textureHeight = 1440;
    mfloat* backbufferData = malloc(textureWidth*textureHeight*4*sizeof(mfloat));

    Camera cam = camera_Construct(vec3f(0,2,3), vec3f(0,0,0), vec3f(0,1,0), 70, (float)textureWidth / (float)textureHeight, 0.025f, 3.0f);
    Scene scene = createScene();

    clock_t start = clock();
    BakedScene bakedScene;
    scene_Bake(&scene, &bakedScene);
    clock_t end = clock();
    float seconds = (float)(end - start) / (float)CLOCKS_PER_SEC;
    printf("Baking scene took %.6f seconds!\n",  seconds);

    // Prepare tracing
    RandomState rndState = time(NULL);
    TraceParameters params;
    params.backbufferWidth = textureWidth;
    params.backbufferHeight = textureHeight;
    params.scene = &bakedScene;
    params.samplesPerPixel = INITIAL_SAMPLES;
    params.camera = &cam;
    params.maxBounces = 6;
    params.maxDepth = 1000000;

    TraceTileParameters fullTileParams = singleTileTraceParams(params);

    int tileSize = 24;
    size_t parallelTileCount = parallelTileTraceParams_TileCount(params, tileSize, tileSize);
    TraceTileParameters* parallelTileParams = malloc(sizeof(TraceTileParameters) * parallelTileCount);
    parallelTileTraceParams(params, tileSize, tileSize, parallelTileParams);

    uint64_t rayCount = 0;
    start = clock();
    traceParallel(parallelTileParams, parallelTileCount, backbufferData, &rayCount, 32, progressCallback, 0, &rndState);
    //traceTile(fullTileParams, backbufferData, &rayCount, 0, &rndState);
    end = clock();

    seconds = (float)(end - start) / (float)CLOCKS_PER_SEC;
    double megaRays = rayCount / 1000000.0;
    float megaRaysPerSecond = megaRays / seconds;
    printf("%.6f MRays processed in %.6f seconds | %.6f MRays/s!\n\n", megaRays, seconds, megaRaysPerSecond);

    // Load texture data
    char* textureData = malloc(textureWidth*textureHeight*4);
    transformBackbufferToTexData(backbufferData, textureData, textureWidth, textureHeight);

    int fileTimeI = time(NULL);

    char* bmpData = malloc(textureWidth*textureHeight*3);
    char tmpPath[512];
    sprintf((void*)&tmpPath, "%i.bmp", fileTimeI);
    transformBackbufferToBmpData(backbufferData, bmpData, textureWidth, textureHeight);
    saveBMP(&tmpPath, textureWidth, textureHeight, bmpData);
    free(bmpData);


    // Generate a texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload the texture data to the GPU
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);

    printf("Tracing done! Entering main loop...\n");
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    size_t sampleCount = INITIAL_SAMPLES;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);
        glEnd();

        glfwSwapBuffers(window);

        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_S))
        {
            char* bmpData = malloc(textureWidth*textureHeight*3);
            char tmpPath[512];
            sprintf((void*)&tmpPath, "%i_%i.bmp", fileTimeI, sampleCount);
            transformBackbufferToBmpData(backbufferData, bmpData, textureWidth, textureHeight);
            saveBMP(&tmpPath, textureWidth, textureHeight, bmpData);
            free(bmpData);
        }

        // next iteration
        fullTileParams.traceParams.samplesPerPixel = SAMPLES_PER_ITERATION;
        params.samplesPerPixel = SAMPLES_PER_ITERATION;
        parallelTileTraceParams(params, tileSize, tileSize, parallelTileParams);

        start = clock();
        // MT
        traceParallel(parallelTileParams, parallelTileCount, backbufferData, &rayCount, 32, progressCallback, sampleCount, &rndState);

        // ST
        //traceTile(fullTileParams, backbufferData, &rayCount, sampleCount, &rndState);
        end = clock();

        seconds = (float)(end - start) / (float)CLOCKS_PER_SEC;
        double megaRays = rayCount / 1000000.0;
        float megaRaysPerSecond = megaRays / seconds;
        printf("Samples: %i | %.6f MRays processed in %.6f seconds | %.6f MRays/s!\n", sampleCount, megaRays, seconds, megaRaysPerSecond);
        sampleCount += SAMPLES_PER_ITERATION;

        // Update Tex
        transformBackbufferToTexData(backbufferData, textureData, textureWidth, textureHeight);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    }

    // Clean up
    glDeleteTextures(1, &texture);

    free(backbufferData);
    free(parallelTileParams);
    scene_Free(&scene);
    bakedScene_Free(&bakedScene);
    free(textureData);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
