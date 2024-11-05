#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "stb_image.h"
#include "Model.h"

//settings
int SCR_WIDTH{ 800 };
int SCR_HEIGHT{ 600 };

//screen
Camera camera(glm::vec3(0.f, 0.f, 3.f));
float lastX{ 0 };
float lastY{ 0 };
bool firstMouse{ true };

//delta dogs
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
        firstMouse = false;
        lastX = XPOS;
        lastY = YPOS;
    }

    float xoffset = XPOS - lastX;
    float yoffset = lastY - YPOS; //ypos is negative because yaxis is reversed

    lastX = XPOS;
    lastY = YPOS;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

unsigned int loadTexture(std::string filename, GLenum internal_format = 0) {
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
        GLenum format{};
        switch (nrChannels) {
        case(1): format = GL_RED; break;
        case(3): format = GL_RGB; break;
        case(4): format = GL_RGBA; break;
        default: format = GL_RGB; break;
        }
        internal_format = internal_format == 0 ? format : internal_format;
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture: " << filename << '\n';
    }
    stbi_image_free(data);
    return textureID;
}

bool showColor{ false };
bool isKeyPressed{ false };
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
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && isKeyPressed) {
        showColor = !showColor;
        isKeyPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
        isKeyPressed = true;
    }
}

float lerp(float a, float b, float f) {
    return a + f * (b - a);
}

int main() {
    //init OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif // __APPLE__

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SSAO", NULL, NULL);
    if (window == NULL) {
        std::cout << "ERROR: Failed to load window!\n";
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, &framebuffer_scall);
    glfwSetScrollCallback(window, &scroll_scall);
    glfwSetCursorPosCallback(window, &cursor_scall);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "ERROR: Failed to load GLAD!\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    Shader depthShader{ "depthVertex.txt", "depthFragment.txt" };
    Shader ssaoShader{ "ssaoVertex.txt","ssaoFragment.txt" };
    Shader shader{ "vertexShader.txt", "fragmentShader.txt" };
    Shader blurShader{ "blurVertex.vs","blurFragment.fs" };

#define cubeVerticesSize 240
    float* cubeVertices = new float[cubeVerticesSize] {
        -0.5f, -0.5f, -0.5f,    0.f, 0.f, -1.f,     0.f, 0.f,
         0.5f,  0.5f, -0.5f,    0.f, 0.f, -1.f,     1.f, 1.f,
        -0.5f,  0.5f, -0.5f,    0.f, 0.f, -1.f,     0.f, 1.f,
        -0.5f, -0.5f, -0.5f,    0.f, 0.f, -1.f,     0.f, 0.f,
         0.5f, -0.5f, -0.5f,    0.f, 0.f, -1.f,     1.f, 0.f,
         0.5f,  0.5f, -0.5f,    0.f, 0.f, -1.f,     1.f, 1.f,
            
        -0.5f, -0.5f,  0.5f,    0.f, 0.f,  1.f,     0.f, 0.f,
         0.5f,  0.5f,  0.5f,    0.f, 0.f,  1.f,     1.f, 1.f,
        -0.5f,  0.5f,  0.5f,    0.f, 0.f,  1.f,     0.f, 1.f,
        -0.5f, -0.5f,  0.5f,    0.f, 0.f,  1.f,     0.f, 0.f,
         0.5f, -0.5f,  0.5f,    0.f, 0.f,  1.f,     1.f, 0.f,
         0.5f,  0.5f,  0.5f,    0.f, 0.f,  1.f,     1.f, 1.f,
            
        -0.5f, -0.5f, -0.5f,    -1.f, 0.f, 0.f,     0.f, 0.f,
        -0.5f,  0.5f,  0.5f,    -1.f, 0.f, 0.f,     1.f, 1.f,
        -0.5f, -0.5f,  0.5f,    -1.f, 0.f, 0.f,     0.f, 1.f,
        -0.5f, -0.5f, -0.5f,    -1.f, 0.f, 0.f,     0.f, 0.f,
        -0.5f,  0.5f, -0.5f,    -1.f, 0.f, 0.f,     1.f, 0.f,
        -0.5f,  0.5f,  0.5f,    -1.f, 0.f, 0.f,     1.f, 1.f,
         
         0.5f, -0.5f, -0.5f,     1.f, 0.f, 0.f,     0.f, 0.f,
         0.5f,  0.5f,  0.5f,     1.f, 0.f, 0.f,     1.f, 1.f,
         0.5f, -0.5f,  0.5f,     1.f, 0.f, 0.f,     0.f, 1.f,
         0.5f, -0.5f, -0.5f,     1.f, 0.f, 0.f,     0.f, 0.f,
         0.5f,  0.5f, -0.5f,     1.f, 0.f, 0.f,     1.f, 0.f,
         0.5f,  0.5f,  0.5f,     1.f, 0.f, 0.f,     1.f, 1.f,
         
            
        -0.5f, -0.5f, -0.5f,    0.f, -1.f, 0.f,     0.f, 0.f,
         0.5f, -0.5f,  0.5f,    0.f, -1.f, 0.f,     1.f, 1.f,
        -0.5f, -0.5f,  0.5f,    0.f, -1.f, 0.f,     0.f, 1.f,
        -0.5f, -0.5f, -0.5f,    0.f, -1.f, 0.f,     0.f, 0.f,
         0.5f, -0.5f, -0.5f,    0.f, -1.f, 0.f,     1.f, 0.f,
         0.5f, -0.5f,  0.5f,    0.f, -1.f, 0.f,     1.f, 1.f,
         /*
        -0.5f,  0.5f, -0.5f,    0.f,  1.f, 0.f,     0.f, 0.f,
         0.5f,  0.5f,  0.5f,    0.f,  1.f, 0.f,     1.f, 1.f,
        -0.5f,  0.5f,  0.5f,    0.f,  1.f, 0.f,     0.f, 1.f,
        -0.5f,  0.5f, -0.5f,    0.f,  1.f, 0.f,     0.f, 0.f,
         0.5f,  0.5f, -0.5f,    0.f,  1.f, 0.f,     1.f, 0.f,
         0.5f,  0.5f,  0.5f,    0.f,  1.f, 0.f,     1.f, 1.f,
         */
    };
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, cubeVerticesSize * sizeof(float), cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

#define quadVerticesSize 48
    float* quadVertices = new float[quadVerticesSize] {
        -1.f, -1.f, 0.f,    0.f, 0.f, 0.f,  0.f, 0.f,
         1.f, -1.f, 0.f,    0.f, 0.f, 0.f,  1.f, 0.f,
         1.f,  1.f, 0.f,    0.f, 0.f, 0.f,  1.f, 1.f,
        -1.f, -1.f, 0.f,    0.f, 0.f, 0.f,  0.f, 0.f,
         1.f,  1.f, 0.f,    0.f, 0.f, 0.f,  1.f, 1.f,
        -1.f,  1.f, 0.f,    0.f, 0.f, 0.f,  0.f, 1.f,
    };
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, quadVerticesSize * sizeof(float), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    Model rockModel{ "rock/rock.gltf" };

    //framebuffers
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    unsigned int gPosition;
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    unsigned int gNormal;
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    unsigned int gColor;
    glGenTextures(1, &gColor);
    glBindTexture(GL_TEXTURE_2D, gColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColor, 0);

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //ssao Framebuffer
    unsigned int ssaoFBO;
    glGenFramebuffers(1, &ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    unsigned int ssaoTexture;
    glGenTextures(1, &ssaoTexture);
    glBindTexture(GL_TEXTURE_2D, ssaoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //blur framebuffer
    unsigned int blurFBO;
    glGenFramebuffers(1, &blurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);

    unsigned int blurTexture;
    glGenTextures(1, &blurTexture);
    glBindTexture(GL_TEXTURE_2D, blurTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //kernal
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernal;
    for (int i{ 0 }; i < 64; ++i) {
        glm::vec3 sample{
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator)
        };
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = (float)i / 64.0;

        scale = lerp(0.1f, 1.f, scale * scale);
        sample *= scale;
        ssaoKernal.push_back(sample);
    }

    std::vector<glm::vec3> ssaoNoise;
    for (int i{ 0 }; i < 16; i++){
        glm::vec3 noise{
             randomFloats(generator) * 2.0 - 1.0,
             randomFloats(generator) * 2.0 - 1.0,
             0.f
        };
        ssaoNoise.push_back(noise);
    }

    unsigned int noiseTexture;
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGBA, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //textures
    unsigned int wood = loadTexture("wood_floor.png", GL_SRGB_ALPHA);
    unsigned int rock = loadTexture("rock/rock.png", GL_SRGB_ALPHA);

    shader.use();
    shader.setInt("gPosition", 0);
    shader.setInt("gNormal", 1);
    shader.setInt("gColor", 2);
    shader.setInt("ssao", 3);

    ssaoShader.use();
    ssaoShader.setInt("gPosition", 0);
    ssaoShader.setInt("gNormal", 1);
    ssaoShader.setInt("texNoise", 2);

    blurShader.use();
    blurShader.setInt("texture1", 0);

    //render loop
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        glClearColor(0.2f, 0.5f, 1.f, 1.f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wood);

        //gBuffer
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        depthShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);
        depthShader.setMat4("projection", projection);

        glm::mat4 view = camera.GetViewMatrix();
        depthShader.setMat4("view", view);
        glBindVertexArray(cubeVAO);
        glm::mat4 model = glm::mat4(1.f);
        model = glm::scale(model, glm::vec3(10.f));
        depthShader.setBool("isWall", true);

        depthShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        depthShader.setBool("isWall", false);
        glBindTexture(GL_TEXTURE_2D, rock);
        for (int i{ 0 }; i < 6; ++i) {
            model = glm::mat4(1.f);
            model = glm::translate(model, glm::vec3(glm::sin(i*45.f)*4.9, glm::cos(i * 45.f)*4.9, glm::sin(i * 45.f)*4.9));
            model = glm::scale(model, glm::vec3(0.6f));

            depthShader.setMat4("model", model);
            rockModel.Draw(depthShader);
        }

        //ssao
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);

        ssaoShader.use();
        for (unsigned int i = 0; i < 64; ++i)
            ssaoShader.setVec3("samples[" + std::to_string(i) + "]", ssaoKernal[i]);
        ssaoShader.setMat4("projection", projection);
        ssaoShader.setFloat("SCR_WIDTH", (float)SCR_WIDTH);
        ssaoShader.setFloat("SCR_HEIGHT", (float)SCR_HEIGHT);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //blur
        glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoTexture);

        blurShader.use();
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //final lighting pass
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gColor);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, blurTexture);

        shader.use();
        shader.setBool("showColor", showColor);
        shader.setVec3("lightDir", 0.1f, 1.f, 0.2f);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        processInput(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete[] cubeVertices;
    glfwTerminate();
    return 0;
}
