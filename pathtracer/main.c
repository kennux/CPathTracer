#include "GLFW/glfw3.h"
#include "camera.h"
#include "tracer.h"
#include "bmpwriter.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

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
    int textureWidth = 1280;
    int textureHeight = 720;
    mfloat* backbufferData = malloc(textureWidth*textureHeight*4*sizeof(mfloat));

    Camera cam = camera_Construct(vec3f(0,2,3), vec3f(0,0,0), vec3f(0,1,0), 70, (float)textureWidth / (float)textureHeight, 0.025f, 3.0f);

    // Create scene
    Scene scene;

    // Create materials
    scene.materials = malloc(sizeof(Material) * 3);
    scene.materialCount = 3;
    Material* materialLambert1 = &scene.materials[0];
    materialLambert1->albedo = vec3f(0.8f, 0.4f, 0.4f);
    materialLambert1->type = MaterialType_Lambert;
    Material* materialLambert2 = &scene.materials[1];
    materialLambert2->albedo = vec3f(0.8f, 0.8f, 0.8f);
    materialLambert2->type = MaterialType_Lambert;
    Material* materialMetal = &scene.materials[2];
    materialMetal->albedo = vec3f(0.75f, 0.75f, 0.75f);
    materialMetal->roughness = 0.025f;
    materialMetal->type = MaterialType_Metal;

    // Create spheres
    scene.spheres = malloc(sizeof(Sphere) * 8);
    scene.sphereCount = 8;
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
    editSphere->center = vec3f(0.5f, 1.25f, 0.5f);
    editSphere->material = materialLambert1;
    /*editSphere->radius = 100;
    editSphere->center = vec3f(0, -100.5f, -1);
    editSphere->materialLambert1 = materialLambert1;*/

    // Setup lighting
    scene.ambientLight = vec3f(0.75f, 0.75f, 0.75f);

    clock_t start = clock();
    BakedScene bakedScene;
    scene_Bake(&scene, &bakedScene);
    clock_t end = clock();
    float seconds = (float)(end - start) / (float)CLOCKS_PER_SEC;
    printf("Baking scene took %.6f seconds!\n",  seconds);

    // Prepare tracing
    TraceParameters params;
    params.backbufferWidth = textureWidth;
    params.backbufferHeight = textureHeight;
    params.scene = &bakedScene;
    params.samplesPerPixel = 16;
    params.camera = &cam;
    params.maxBounces = 6;
    params.maxDepth = 10000;

    long int rayCount = 0;
    start = clock();
    trace(params, backbufferData, &rayCount);
    end = clock();

    seconds = (float)(end - start) / (float)CLOCKS_PER_SEC;
    double megaRays = rayCount / 1000000.0;
    float megaRaysPerSecond = megaRays / seconds;
    printf("%.6f MRays processed in %.6f seconds | %.6f MRays/s!\n\n", megaRays, seconds, megaRaysPerSecond);

    // Load texture data
    char* textureData = malloc(textureWidth*textureHeight*4);
    char* bmpData = malloc(textureWidth*textureHeight*3);
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

            textureData[index + 0] = r; // R
            textureData[index + 1] = g; // G
            textureData[index + 2] = b; // B
            textureData[index + 3] = a; // A

            // Bmp format is more special than me
            // It's actually BGR...
            bmpData[bmpIndex + 2] = r; // R
            bmpData[bmpIndex + 1] = g; // G
            bmpData[bmpIndex + 0] = b; // B
        }
    }

    free(backbufferData);
    scene_Free(&scene);
    bakedScene_Free(&bakedScene);

    char tmpPath[512];
    sprintf(&tmpPath, "%i.bmp", time(NULL));
    //saveBMP(&tmpPath, textureWidth, textureHeight, bmpData);
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

    // Free the texture data if dynamically allocated
    free(textureData);

    printf("Tracing done! Entering main loop...\n");
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
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
    }
    printf("Window was closed!\n");

    // Clean up
    glDeleteTextures(1, &texture);

    glfwDestroyWindow(window);
    glfwTerminate();
    printf("Proper termination :)\n");
    return 0;
}
