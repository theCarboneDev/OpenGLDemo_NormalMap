#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Game.h"
#include "ResourceManager.h"

//settings
int SCR_WIDTH{ 800 };
int SCR_HEIGHT{ 600 };
Game Breakout(SCR_WIDTH, SCR_HEIGHT);

void framebuffer_scall(GLFWwindow* window, int w, int h) {
    glViewport(0, 0, w, h);
    SCR_WIDTH = w;
    SCR_HEIGHT = h;
}

void key_scall(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            Breakout.keys[key] = true;
        else if (action == GLFW_RELEASE)
            Breakout.keys[key] = false;
    }
}

int main()
{
    //init openGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Breakout", NULL, NULL);
    if (window == NULL) {
        std::cout << "ERROR: Failed to load window!\n";
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, &framebuffer_scall);
    glfwSetKeyCallback(window, &key_scall);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "ERROR: Failed to load glad!\n";
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Breakout.Init();

    float deltaTime = 0.f;
    float lastFrame = 0.f;

    //render loop
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;
        glfwPollEvents();

        Breakout.processInput(deltaTime);

        Breakout.update(deltaTime);

        glClearColor(0.f, 0.f, 0.f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Breakout.render();

        glfwSwapBuffers(window);
    }

    ResourceManager::Clear();
    glfwTerminate();
    return 0;
}