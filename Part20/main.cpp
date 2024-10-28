#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "stb_image.h"
#include "Model.h"

//screen
int SCR_WIDTH{ 800 };
int SCR_HEIGHT{ 600 };

//settings
Camera camera(glm::vec3(0.f, 0.f, 3.f));
float lastX{ 0.f };
float lastY{ 0.f };
bool firstMouse{ true };

//deltaTime
float deltaTime{ 0.f };
float lastFrame{ 0.f };

void framebuffer_scall(GLFWwindow* window, int w, int h) {
    glViewport(0, 0, w, h);
    SCR_WIDTH = w;
    SCR_HEIGHT = h;
}

void scroll_scall(GLFWwindow* window, double xpos, double ypos) {
    camera.ProcessMouseScroll(static_cast<float>(ypos));
}

void cursor_scall(GLFWwindow* window, double xpos, double ypos) {
    float XPOS = static_cast<float>(xpos);
    float YPOS = static_cast<float>(ypos);

    if (firstMouse) {
        lastX = XPOS;
        lastY = YPOS;
        firstMouse = false;
    }

    float xoffset = XPOS - lastX;
    float yoffset = lastY - YPOS; //ypos is negative because yaxis is reversed

    lastX = XPOS;
    lastY = YPOS;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

unsigned int loadTexture(std::string filename, GLenum internalFormat = 0) {
    unsigned int textureID;
    int width, height, nrChannels;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format;
        switch (nrChannels) {
        case(1): format = GL_RED; break;
        case(3): format = GL_RGB; break;
        case(4): format = GL_RGBA; break;
        default: format = GL_RGB; break;
        }
        internalFormat = internalFormat == 0 ? format : internalFormat;
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture: " << filename << '\n';
    }
    stbi_image_free(data);
    return textureID;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

int main() {
    //init OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Defered Rendering", NULL, NULL);
    if (window == NULL) {
        std::cout << "ERROR: Window Failed!\n";
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, &framebuffer_scall);
    glfwSetCursorPosCallback(window, &cursor_scall);
    glfwSetScrollCallback(window, &scroll_scall);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "ERROR: Window Failed!\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_FRAMEBUFFER_SRGB);
    Shader shader{ "vertexShader.txt", "fragmentShader.txt" };
    Shader lightShader{ "vertexShader.txt", "fragmentShaderLight.txt" };
    Shader quadShader{ "depthVertex.txt","depthFragment.txt" };

#define cubeVerticesSize 288
    float* cubeVertices = new float[cubeVerticesSize] {
        -0.5f, -0.5f, -0.5f,    0.f, 0.f, -1.f,     0.f, 0.f,
         0.5f,  0.5f, -0.5f,    0.f, 0.f, -1.f,     1.f, 1.f,
         0.5f, -0.5f, -0.5f,    0.f, 0.f, -1.f,     1.f, 0.f,
        -0.5f, -0.5f, -0.5f,    0.f, 0.f, -1.f,     0.f, 0.f,
        -0.5f,  0.5f, -0.5f,    0.f, 0.f, -1.f,     0.f, 1.f,
         0.5f,  0.5f, -0.5f,    0.f, 0.f, -1.f,     1.f, 1.f,
            
        -0.5f, -0.5f,  0.5f,    0.f, 0.f,  1.f,     0.f, 0.f,
         0.5f, -0.5f,  0.5f,    0.f, 0.f,  1.f,     1.f, 0.f,
         0.5f,  0.5f,  0.5f,    0.f, 0.f,  1.f,     1.f, 1.f,
        -0.5f, -0.5f,  0.5f,    0.f, 0.f,  1.f,     0.f, 0.f,
         0.5f,  0.5f,  0.5f,    0.f, 0.f,  1.f,     1.f, 1.f,
        -0.5f,  0.5f,  0.5f,    0.f, 0.f,  1.f,     0.f, 1.f,
            
        -0.5f, -0.5f, -0.5f,    -1.f, 0.f, 0.f,     0.f, 0.f,
        -0.5f,  0.5f,  0.5f,    -1.f, 0.f, 0.f,     1.f, 1.f,
        -0.5f,  0.5f, -0.5f,    -1.f, 0.f, 0.f,     1.f, 0.f,
        -0.5f, -0.5f, -0.5f,    -1.f, 0.f, 0.f,     0.f, 0.f,
        -0.5f, -0.5f,  0.5f,    -1.f, 0.f, 0.f,     0.f, 1.f,
        -0.5f,  0.5f,  0.5f,    -1.f, 0.f, 0.f,     1.f, 1.f,
            
         0.5f, -0.5f, -0.5f,     1.f, 0.f, 0.f,     0.f, 0.f,
         0.5f,  0.5f, -0.5f,     1.f, 0.f, 0.f,     1.f, 0.f,
         0.5f,  0.5f,  0.5f,     1.f, 0.f, 0.f,     1.f, 1.f,
         0.5f, -0.5f, -0.5f,     1.f, 0.f, 0.f,     0.f, 0.f,
         0.5f,  0.5f,  0.5f,     1.f, 0.f, 0.f,     1.f, 1.f,
         0.5f, -0.5f,  0.5f,     1.f, 0.f, 0.f,     0.f, 1.f,
            
        -0.5f, -0.5f, -0.5f,    0.f, -1.f, 0.f,     0.f, 0.f,
         0.5f, -0.5f, -0.5f,    0.f, -1.f, 0.f,     1.f, 0.f,
         0.5f, -0.5f,  0.5f,    0.f, -1.f, 0.f,     1.f, 1.f,
        -0.5f, -0.5f, -0.5f,    0.f, -1.f, 0.f,     0.f, 0.f,
         0.5f, -0.5f,  0.5f,    0.f, -1.f, 0.f,     1.f, 1.f,
        -0.5f, -0.5f,  0.5f,    0.f, -1.f, 0.f,     0.f, 1.f,
            
        -0.5f,  0.5f, -0.5f,    0.f,  1.f, 0.f,     0.f, 0.f,
         0.5f,  0.5f,  0.5f,    0.f,  1.f, 0.f,     1.f, 1.f,
         0.5f,  0.5f, -0.5f,    0.f,  1.f, 0.f,     1.f, 0.f,
        -0.5f,  0.5f, -0.5f,    0.f,  1.f, 0.f,     0.f, 0.f,
        -0.5f,  0.5f,  0.5f,    0.f,  1.f, 0.f,     0.f, 1.f,
         0.5f,  0.5f,  0.5f,    0.f,  1.f, 0.f,     1.f, 1.f,
    };
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVAO);
    glBufferData(GL_ARRAY_BUFFER, cubeVerticesSize * sizeof(float), cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    float quadVertices[30]{
        -1.f, -1.f, 0.f,    0.f, 0.f,
         1.f,  1.f, 0.f,    1.f, 1.f,
        -1.f,  1.f, 0.f,    0.f, 1.f,
        -1.f, -1.f, 0.f,    0.f, 0.f,
         1.f, -1.f, 0.f,    1.f, 0.f,
         1.f,  1.f, 0.f,    1.f, 1.f,
    };
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    Model rock{ "rock/rock.gltf" };

    glm::vec3 lightColors[15]{
        glm::vec3(1.f, 1.f, 1.f),
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec3(0.f, 0.f, 1.f),
        glm::vec3(1.f, 1.f, 0.f),
        glm::vec3(1.f, 0.f, 1.f),
        glm::vec3(0.f, 1.f, 1.f),
        glm::vec3(0.5f, 0.8f, 0.8f),
        glm::vec3(0.8f, 0.5f, 0.8f),
        glm::vec3(0.8f, 0.8f, 0.5f),
        glm::vec3(0.f, 0.5f, 1.f),
        glm::vec3(1.f, 0.f, 0.5f),
        glm::vec3(0.5f, 1.f, 0.f),
        glm::vec3(0.5f, 0.f, 1.f),
        glm::vec3(0.f, 0.5f, 1.f),
    };

    //gBuffer
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPosition, gNormal, gColor;

    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    glGenTextures(1, &gColor);
    glBindTexture(GL_TEXTURE_2D, gColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColor, 0);

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    unsigned int RBO;
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //textures
    unsigned int rockTexture = loadTexture("rock/rock.png", GL_SRGB_ALPHA);

    shader.use();
    shader.setInt("texture1", 0);

    quadShader.use();
    quadShader.setInt("gPosition", 0);
    quadShader.setInt("gNormal", 1);
    quadShader.setInt("gColor", 2);

    for (int i{ 0 }; i < 15; i++) {
        float row = (i % 5);
        float col = (i / 5);
        glm::vec3 pos{ row * 6.f,0.f,col * 6.f };

        float constant = 1.0;
        float linear = 0.7;
        float quadratic = 1.2;
        float lightMax = std::fmaxf(std::fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);
        float radius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax))) / (2 * quadratic);

        quadShader.setVec3("lights[" + std::to_string(i) + "].position", pos);
        quadShader.setVec3("lights[" + std::to_string(i) + "].color", lightColors[i]);
        quadShader.setFloat("lights[" + std::to_string(i) + "].radius", radius);
    }

    //render loop
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rockTexture);

        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);
        shader.setMat4("projection", projection);

        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);

        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);

        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        for (int i{ 0 }; i < 15; ++i) {
            float row = (i % 5);
            float col = (i / 5);
            glm::mat4 model = glm::mat4(1.0);
            model = glm::translate(model, glm::vec3{row*6.f,0.f,col*6.f});

            shader.setMat4("model", model);
            rock.Draw(shader);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearColor(0.75f, 0.52f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        quadShader.use();
        quadShader.setVec3("viewPos", camera.Position);
        glBindVertexArray(quadVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gColor);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        lightShader.use();
        for (int i{ 0 }; i < 15; i++) {
            float row = (i % 5);
            float col = (i / 5);
            glm::mat4 model = glm::mat4(1.0);
            glm::vec3 pos{ row * 6.f,0.f,col * 6.f };
            model = glm::translate(model, pos);

            model = glm::translate(model, glm::vec3{ 0.5f, 2.f, 0.f });
            model = glm::scale(model, glm::vec3{ 0.2f });
            glBindVertexArray(cubeVAO);

            lightShader.setMat4("model", model);
            lightShader.setVec3("lightColor", lightColors[i]);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        processInput(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete[] cubeVertices;
    glfwTerminate();
    return 0;
}
