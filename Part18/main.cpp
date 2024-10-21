#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

//Credit to: https://learnopengl.com/ for these two .h files and brickwall.jpg, brickwall_normal.jpg
#include "Shader.h"
#include "Camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//settings
int SCR_WIDTH{ 800 };
int SCR_HEIGHT{ 600 };

//screen
Camera camera(glm::vec3(0.f, 0.f, 3.f));
float lastX{ SCR_WIDTH / 2.f };
float lastY{ SCR_HEIGHT / 2.f };
bool firstMouse{ true };

//deltaTime
float deltaTime{ 0.f };
float lastFrame{ 0.f };

void framebuffer_scall(GLFWwindow* window, int w, int h) {
    glViewport(0, 0, w, h);
    SCR_WIDTH = w;
    SCR_HEIGHT = h;
}

bool moveScene{ true };
bool doNormalMap{ true };
bool isTPressed{ false };
bool isNPressed{ false };
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
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !isTPressed) {
        moveScene = !moveScene;
        isTPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
        isTPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isNPressed) {
        doNormalMap = !doNormalMap;
        isNPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        isNPressed = false;
    }
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
    float yoffset = lastY - YPOS; //YPOS is negative, because yaxis is reversed

    lastX = XPOS;
    lastY = YPOS;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_scall(GLFWwindow* window, double xpos, double ypos) {
    camera.ProcessMouseScroll(static_cast<float>(ypos));
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
        GLenum format = GL_RGB;
        switch (nrChannels) {
        case(1): format = GL_RED; break;
        case(3): format = GL_RGB; break;
        case(4): format = GL_RGBA; break;
        }
        if (internalFormat == 0) {
            internalFormat = format;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture: " << filename << '\n';
    }

    stbi_image_free(data);
    return textureID;
}

glm::vec3 calcTangentForNormals(float vertices[24]) {
    glm::vec3 pos1(vertices[0], vertices[1], vertices[2]);
    glm::vec3 pos2(vertices[8], vertices[9], vertices[10]);
    glm::vec3 pos3(vertices[16], vertices[17], vertices[18]);

    glm::vec2 uv1(vertices[6], vertices[7]);
    glm::vec2 uv2(vertices[14], vertices[15]);
    glm::vec2 uv3(vertices[22], vertices[23]);

    glm::vec3 edge1 = pos2 - pos1;
    glm::vec3 edge2 = pos3 - pos1;
    glm::vec2 deltaUV1 = uv2 - uv1;
    glm::vec2 deltaUV2 = uv3 - uv1;

    float det = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    float f = (det == 0.0f) ? 0.0f : 1.0f / det;

    glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);

    return glm::normalize(tangent);
}


int main() {
    //init opengl
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Normal Map", NULL, NULL);
    if (window == NULL) {
        std::cout << "ERROR: Window failed!\n";
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, &framebuffer_scall);
    glfwSetCursorPosCallback(window, &cursor_scall);
    glfwSetScrollCallback(window, &scroll_scall);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "ERORR: GLAD!";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    Shader shader{ "vertexShader.txt", "fragmentShader.txt" };
    Shader lightShader{ "vertexShader.txt", "fragmentShaderLight.txt" };

#define cubeVerticesSize 288
    float *cubeVertices = new float[cubeVerticesSize]{
            -0.5f, -0.5f, -0.5f,    0.f, 0.f, -1.f,     0.f, 0.f, 
            -0.5f,  0.5f, -0.5f,    0.f, 0.f, -1.f,     0.f, 1.f,
             0.5f,  0.5f, -0.5f,    0.f, 0.f, -1.f,     1.f, 1.f,
            -0.5f, -0.5f, -0.5f,    0.f, 0.f, -1.f,     0.f, 0.f,
             0.5f,  0.5f, -0.5f,    0.f, 0.f, -1.f,     1.f, 1.f,
             0.5f, -0.5f, -0.5f,    0.f, 0.f, -1.f,     1.f, 0.f,
                
            -0.5f, -0.5f,  0.5f,    0.f, 0.f,  1.f,     0.f, 0.f,
             0.5f,  0.5f,  0.5f,    0.f, 0.f,  1.f,     1.f, 1.f,
            -0.5f,  0.5f,  0.5f,    0.f, 0.f,  1.f,     0.f, 1.f,
            -0.5f, -0.5f,  0.5f,    0.f, 0.f,  1.f,     0.f, 0.f,
             0.5f, -0.5f,  0.5f,    0.f, 0.f,  1.f,     1.f, 0.f,
             0.5f,  0.5f,  0.5f,    0.f, 0.f,  1.f,     1.f, 1.f,
                
            -0.5f, -0.5f, -0.5f,    -1.f, 0.f, 0.f,     0.f, 0.f,
            -0.5f, -0.5f,  0.5f,    -1.f, 0.f, 0.f,     0.f, 1.f,
            -0.5f,  0.5f,  0.5f,    -1.f, 0.f, 0.f,     1.f, 1.f,
            -0.5f, -0.5f, -0.5f,    -1.f, 0.f, 0.f,     0.f, 0.f,
            -0.5f,  0.5f,  0.5f,    -1.f, 0.f, 0.f,     1.f, 1.f,
            -0.5f,  0.5f, -0.5f,    -1.f, 0.f, 0.f,     1.f, 0.f,
                
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
                
            -0.5f,  0.5f, -0.5f,    0.f,  1.f, 0.f,     0.f, 0.f,
            -0.5f,  0.5f,  0.5f,    0.f,  1.f, 0.f,     0.f, 1.f,
             0.5f,  0.5f,  0.5f,    0.f,  1.f, 0.f,     1.f, 1.f,
            -0.5f,  0.5f, -0.5f,    0.f,  1.f, 0.f,     0.f, 0.f,
             0.5f,  0.5f,  0.5f,    0.f,  1.f, 0.f,     1.f, 1.f,
             0.5f,  0.5f, -0.5f,    0.f,  1.f, 0.f,     1.f, 0.f,
        };
    //cube vao
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    //tangents
    float tangents[108];
    for (int i = 0; i < 36; i += 3) {
        glm::vec3 tangentVec = calcTangentForNormals(&cubeVertices[8 * i]);

        for (int j = 0; j < 3; ++j) {
            tangents[(i + j) * 3 + 0] = tangentVec.x;
            tangents[(i + j) * 3 + 1] = tangentVec.y;
            tangents[(i + j) * 3 + 2] = tangentVec.z;
        }
    }

    unsigned int tangentVBO;
    glGenBuffers(1, &tangentVBO);
    glBindBuffer(GL_ARRAY_BUFFER, tangentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tangents), tangents, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cubeVerticesSize, cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, tangentVBO);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);


    //textures
    unsigned int brick = loadTexture("brickwall.jpg");
    unsigned int brickNormal = loadTexture("brickwall_normal.jpg");

    shader.use();
    shader.setInt("texture1", 0);
    shader.setInt("normal_texture", 1);

    //render loop
    float moveTime = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, brick);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, brickNormal);

        if (moveScene) {
            moveTime = currentTime;
        }
        glm::vec3 lightPos = glm::vec3(glm::sin(moveTime * 2) * 1.5, glm::sin(moveTime) * 0.2 - 0.5f, glm::cos(moveTime * 2) * 1.5);

        shader.use();
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("lightPos", lightPos);
        shader.setFloat("specialEffect", doNormalMap);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);
        shader.setMat4("projection", projection);

        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);

        glBindVertexArray(cubeVAO);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, moveTime /1.5f, glm::vec3(0.f,1.f,0.f));
        model = glm::translate(model, glm::vec3(0.f, -0.4f, 0.f));

        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));

        lightShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}
