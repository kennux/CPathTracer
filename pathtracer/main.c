#include "GLFW/glfw3.h"
#include "camera.h"
#include "tracer.h"
#include <stdlib.h>
#include <stdio.h>

// Callback function to handle window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main(void) {
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // Set the framebuffer size callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glClearColor(0, 0, 0, 0.0f);

    // Backbuffer
    int textureWidth = 1280;
    int textureHeight = 720;
    mfloat* backbufferData = malloc(textureWidth*textureHeight*4*sizeof(mfloat));

    Camera cam = camera_Construct(vec3f(0,2,3), vec3f(0,0,0), vec3f(0,1,0), 70, (float)textureWidth / (float)textureHeight, 0.025f, 3.0f);

    // Create scene
    Scene scene;

    // Create materials
    scene.materials = malloc(sizeof(Material) * 1);
    scene.materialCount = 1;
    Material* material_lambert = &scene.materials[0];
    material_lambert->albedo = vec3f(0.8f, 0.4f, 0.4f);
    material_lambert->type = MaterialType_Lambert;

    // Create spheres
    scene.spheres = malloc(sizeof(Sphere) * 8);
    scene.sphereCount = 8;
    Sphere* editSphere = &scene.spheres[0];
    editSphere->radius = 100.0f;
    editSphere->center = vec3f(0, -100.5f, -1);
    editSphere->material = material_lambert;
    editSphere = &scene.spheres[1];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(2, 0, -1);
    editSphere->material = material_lambert;
    editSphere = &scene.spheres[2];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(0, 0, -1);
    editSphere->material = material_lambert;
    editSphere = &scene.spheres[3];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(-2, 0, -1);
    editSphere->material = material_lambert;
    editSphere = &scene.spheres[4];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(2, 0, 1);
    editSphere->material = material_lambert;
    editSphere = &scene.spheres[5];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(0, 0, 1);
    editSphere->material = material_lambert;
    editSphere = &scene.spheres[6];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(-2, 0, 1);
    editSphere->material = material_lambert;
    editSphere = &scene.spheres[7];
    editSphere->radius = 0.5f;
    editSphere->center = vec3f(0.5f, 1.25f, 0.5f);
    editSphere->material = material_lambert;
    /*editSphere->radius = 100;
    editSphere->center = vec3f(0, -100.5f, -1);
    editSphere->material_lambert = material_lambert;*/

    // Setup lighting
    scene.ambientLight = vec3f(0.75f, 0.75f, 0.75f);

    // Prepare tracing
    TraceParameters params;
    params.backbufferWidth = textureWidth;
    params.backbufferHeight = textureHeight;
    params.scene = &scene;
    params.samplesPerPixel = 128;
    params.camera = &cam;
    params.maxBounces = 8;
    params.maxDepth = 10000;

    trace(params, backbufferData);

    // Load texture data
    char* textureData = malloc(textureWidth*textureHeight*4);
    for (int x = 0; x < textureWidth; x++)
    {
        for (int y = 0; y < textureHeight; y++)
        {
            int index = (y * textureWidth + x) * 4;
            textureData[index + 0] = (char)(backbufferData[index + 0] * 255.0); // R
            textureData[index + 1] = (char)(backbufferData[index + 1] * 255.0); // G
            textureData[index + 2] = (char)(backbufferData[index + 2] * 255.0); // B
            textureData[index + 3] = (char)(backbufferData[index + 3] * 255.0); // A
        }
    }

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
    // free(textureData);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Render the texture
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);
        glEnd();

        glDisable(GL_TEXTURE_2D);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    // Clean up
    glDeleteTextures(1, &texture);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
