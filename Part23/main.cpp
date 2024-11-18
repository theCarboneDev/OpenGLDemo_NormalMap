#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//setting
int SCR_WIDTH{ 800 };
int SCR_HEIGHT{ 600 };

//screen
Camera camera(glm::vec3(0.f, 0.f, 3.f));
float lastX{ 0.f };
float lastY{ 0.f };
bool firstMouse{ true };

//deltaTime
float deltaTime{ 0.f };
float lastFrame{ 0.f };

void renderSphere(Shader& shader);
void framebuffer_scall(GLFWwindow* window, int w, int h) {
    glViewport(0, 0, w, h);
    SCR_WIDTH = w;
    SCR_HEIGHT = h;
}

void scroll_scall(GLFWwindow* window, double xpos, double ypos) {
    camera.ProcessMouseScroll(static_cast<float>(ypos));
}

void cursor_scall(GLFWwindow* window, double xpos, double ypos) {
    float XPOS = (float)xpos;
    float YPOS = (float)ypos;

    if (firstMouse) {
        firstMouse = false;
        lastX = XPOS;
        lastY = YPOS;
    }

    float xoffset = XPOS - lastX;
    float yoffset = lastY - YPOS;//ypos is negative because yaxis is reversed

    lastX = XPOS;
    lastY = YPOS;

    camera.ProcessMouseMovement(xoffset, yoffset);
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

unsigned int loadTexture(std::string filename, GLenum internalFormat=0) {
    unsigned int textureID;
    int width, height, nrChannels;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    stbi_set_flip_vertically_on_load(false);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
        std::cout << "Failed to load image: " << filename << '\n';
    }

    stbi_image_free(data);
    return textureID;
}

unsigned int loadHDRTexture(std::string filename) {
    unsigned int textureID;
    int width, height, nrChnanels;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    stbi_set_flip_vertically_on_load(true);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    float* data = stbi_loadf(filename.c_str(), &width, &height, &nrChnanels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load image: " << filename << '\n';
    }

    stbi_image_free(data);
    return textureID;
}

int main() {
    //init openGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "PBR", NULL, NULL);
    if (window == NULL) {
        std::cout << "ERROR: Window failed to load!\n";
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, &framebuffer_scall);
    glfwSetScrollCallback(window, &scroll_scall);
    glfwSetCursorPosCallback(window, &cursor_scall);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "ERROR: Glad failed to load!\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    Shader shader{ "vertexShader.txt", "fragmentShader.txt" };
    Shader hdrShader{ "hdrVertex.vs","hdrFragment.fs" };
    Shader skyboxShader{ "skyVertex.txt", "skyFragment.txt" };
    Shader irradienceShader{ "irrVertex.txt", "irrFragment.txt"};
    Shader prefilterShader{ "preFilter.vs", "preFilter.fs" };
    Shader brdfShader{ "brdf.vs","brdf.fs" };

#define cubeVerticesSize 108
    float* cubeVertices = new float[cubeVerticesSize] {
        -1.f, -1.f, -1.f,
        -1.f,  1.f, -1.f,
         1.f,  1.f, -1.f,
        -1.f, -1.f, -1.f,
         1.f,  1.f, -1.f,
         1.f, -1.f, -1.f,
            
        -1.f, -1.f,  1.f,
        -1.f,  1.f,  1.f,
         1.f,  1.f,  1.f,
        -1.f, -1.f,  1.f,
         1.f,  1.f,  1.f,
         1.f, -1.f,  1.f,

        -1.f, -1.f, -1.f,
        -1.f, -1.f,  1.f,
        -1.f,  1.f,  1.f,
        -1.f, -1.f, -1.f,
        -1.f,  1.f,  1.f,
        -1.f,  1.f, -1.f,
            
         1.f, -1.f, -1.f,
         1.f, -1.f,  1.f,
         1.f,  1.f,  1.f,
         1.f, -1.f, -1.f,
         1.f,  1.f,  1.f,
         1.f,  1.f, -1.f,

        -1.f, -1.f, -1.f,
        -1.f, -1.f,  1.f,
         1.f, -1.f,  1.f,
        -1.f, -1.f, -1.f,
         1.f, -1.f,  1.f,
         1.f, -1.f, -1.f,
            
        -1.f,  1.f, -1.f,
        -1.f,  1.f,  1.f,
         1.f,  1.f,  1.f,
        -1.f,  1.f, -1.f,
         1.f,  1.f,  1.f,
         1.f,  1.f, -1.f,
    };
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cubeVerticesSize, cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);

#define quadVerticesSize 30
    float* quadVertices = new float[quadVerticesSize] {
        -1.f, -1.f, 0.f,    0.f, 0.f,
        -1.f,  1.f, 0.f,    0.f, 1.f,
         1.f,  1.f, 0.f,    1.f, 1.f,
        -1.f, -1.f, 0.f,    0.f, 0.f,
         1.f,  1.f, 0.f,    1.f, 1.f,
         1.f, -1.f, 0.f,    0.f, 1.f,
    };
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * quadVerticesSize, quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));

    glBindVertexArray(0);

    //textures
    unsigned int albedoTexture[5];
    unsigned int metallicTexture[5];
    unsigned int normalTexture[5];
    unsigned int roughnessTexture[5];
    unsigned int aoTexture[5];

    std::string filenameTextures[5] = { "rustediron", "brick-wall", "plastic", "grass_meadow", "gold"};
    for (int i{ 0 }; i < 5; i++) {
        albedoTexture[i] = loadTexture(filenameTextures[i] + "/basecolor.png", GL_SRGB_ALPHA);
        metallicTexture[i] = loadTexture(filenameTextures[i] + "/metallic.png");
        normalTexture[i] = loadTexture(filenameTextures[i] + "/normal.png");
        roughnessTexture[i] = loadTexture(filenameTextures[i] + "/roughness.png");
        aoTexture[i] = loadTexture(filenameTextures[i] + "/ao.png");
    }

    unsigned int hdrTexture = loadHDRTexture("newport_loft.hdr");

    shader.use();
    shader.setInt("albedoTex", 0);
    shader.setInt("metallicTex", 1);
    shader.setInt("normalTex", 2);
    shader.setInt("roughnessTex", 3);
    shader.setInt("aoTex", 4);
    shader.setInt("irrMap", 5);

    skyboxShader.use();
    skyboxShader.setInt("envMap", 0);

    //fbo
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    unsigned int envCubeMap;
    glGenTextures(1, &envCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (int i{ 0 }; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, NULL);
    }

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
    glm::mat4 captureView[6]{
        glm::lookAt(glm::vec3(0.f,0.f,0.f),glm::vec3( 1.f,0.f,0.f),glm::vec3(0.f,-1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f,0.f,0.f),glm::vec3(-1.f,0.f,0.f),glm::vec3(0.f,-1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f,0.f,0.f),glm::vec3(0.f, 1.f,0.f),glm::vec3(0.f,0.f, 1.f)),
        glm::lookAt(glm::vec3(0.f,0.f,0.f),glm::vec3(0.f,-1.f,0.f),glm::vec3(0.f,0.f,-1.f)),
        glm::lookAt(glm::vec3(0.f,0.f,0.f),glm::vec3(0.f,0.f, 1.f),glm::vec3(0.f,-1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f,0.f,0.f),glm::vec3(0.f,0.f,-1.f),glm::vec3(0.f,-1.f, 0.f)),
    };
    hdrShader.use();
    hdrShader.setInt("equirectangleMap", 0);
    hdrShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 512, 512);
    glBindVertexArray(cubeVAO);
    glDisable(GL_CULL_FACE);
    for (int i{ 0 }; i < 6; i++) {
        hdrShader.setMat4("view", captureView[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubeMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int irradienceMap;
    glGenTextures(1, &irradienceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradienceMap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (int i{ 0 }; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, NULL);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    irradienceShader.use();
    irradienceShader.setInt("envMap", 0);
    irradienceShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);

    glViewport(0, 0, 32, 32);
    glBindVertexArray(cubeVAO);
    for (int i{ 0 }; i < 6; i++) {
        irradienceShader.setMat4("view", captureView[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradienceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    unsigned int prefilterMap;
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for (int i{ 0 }; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, NULL);
    }

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    prefilterShader.use();
    prefilterShader.setInt("envMap", 0);
    prefilterShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
    for (int i{ 0 }; i < 5; i++) {
        unsigned int mipWidth = 128 * std::pow(0.5, i);
        unsigned int mipHeight = 128 * std::pow(0.5, i);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)i / (float)4;
        prefilterShader.setFloat("roughness", roughness);
        glBindVertexArray(cubeVAO);
        for (int j{ 0 }; j < 6; j++) {
            prefilterShader.setMat4("view", captureView[j]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, prefilterMap, i);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    unsigned int brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);

    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, 512, 512, 0, GL_RG, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    brdfShader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    //light settings
    glm::vec3 lightPositions[4]{
        glm::vec3( 20.f, 3.f, 20.f),
        glm::vec3(-20.f, 3.f, 20.f),
        glm::vec3(20.f, 3.f, -20.f),
        glm::vec3(-20.f, 3.f, -20.f),
    };

    glm::vec3 lightColors[4]{
        glm::vec3(300.f,300.f,300.f),
        glm::vec3(300.f,300.f,300.f),
        glm::vec3(300.f,300.f,300.f),
        glm::vec3(300.f,300.f,300.f),
    };

    //render loop
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        glClearColor(0.2f, 0.5f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window);

        shader.use();
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradienceMap);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);
        shader.setMat4("projection", projection);

        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);

        glm::mat4 model;
        shader.setVec3("viewPos", camera.Position);
        shader.setInt("isLight", -1);
        for (int x{ -2 }; x < 3; x++) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, albedoTexture[x + 2]);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, metallicTexture[x + 2]);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, normalTexture[x + 2]);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, roughnessTexture[x + 2]);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, aoTexture[x + 2]);
                model = glm::mat4(1.0);
                model = glm::translate(model, glm::vec3(2.5f * x, 0.f, 0.f));

                shader.setMat4("model", model);
                shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
                renderSphere(shader);
        }


        for (int i{ 0 }; i < 4; ++i)
        {
            shader.setInt("isLight", i);
            shader.setVec3("lightPos[" + std::to_string(i) + "]", lightPositions[i]);
            shader.setVec3("lightColor[" + std::to_string(i) + "]", lightColors[i]);

            model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions[i]);
            shader.setMat4("model", model);
            shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            renderSphere(shader);
        }

        skyboxShader.use();
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setMat4("view", view);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);

        glBindVertexArray(cubeVAO);
        glDisable(GL_CULL_FACE);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glEnable(GL_CULL_FACE);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    delete[] cubeVertices;
    delete[] quadVertices;
    return 0;
}

unsigned int sphereVAO{ 0 };
unsigned int indexCount;
void renderSphere(Shader& shader) {
    if (sphereVAO == 0) {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (int x{ 0 }; x <= X_SEGMENTS; x++) {
            for (int y{ 0 }; y <= Y_SEGMENTS; y++) {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xpos = std::cos(2 * xSegment * PI) * std::sin(ySegment * PI);
                float ypos = std::cos(ySegment * PI);
                float zpos = std::sin(2 * xSegment * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xpos, ypos, zpos));
                uvs.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xpos, ypos, zpos));
            }
        }

        bool oddRow = false;
        for (int y{ 0 }; y < Y_SEGMENTS; y++) {
            if (!oddRow) {
                for (int x{ 0 }; x <= X_SEGMENTS; x++) {
                    indices.push_back(y     * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else {
                for (int x{ X_SEGMENTS }; x >= 0; x--) {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y     * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = static_cast<unsigned int>(indices.size());

        std::vector<float> data;
        for (int i{ 0 }; i < positions.size(); i++) {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0) {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (uvs.size() > 0) {
                data.push_back(uvs[i].x);
                data.push_back(uvs[i].y);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);

        GLsizei stride = (3 + 3 + 2) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    }
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}
