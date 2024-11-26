#ifndef G_TEXT_H
#define G_TEXT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Text {
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec4 aVertex;\n"
        "out vec2 TexCoord;\n"
        "uniform mat4 projection;\n"
        "void main(){"
        "gl_Position = projection * vec4(aVertex.xy, 0.0, 1.0);\n"
        "TexCoord = aVertex.zw;\n"
        "}\0";

    const char* fragmentShaderSource = "#version 330 core\n"
        "in vec2 TexCoord;\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D text;\n"
        "uniform vec3 textColor;\n"
        "void main(){\n"
        "vec4 sampled = vec4(1.0,1.0,1.0,texture(text,TexCoord).r);\n"
        "FragColor = vec4(textColor, 1.0) * sampled\n;"
        "}\0";

    struct Character {
        unsigned int textureID;
        glm::ivec2 size;
        glm::ivec2 bearing;
        unsigned int advance;
    };
    std::map<char, Character> characters;
    unsigned int shaderProgram;
    glm::mat4 projection;
    unsigned int VAO, VBO;

    float scrWidth;
    float scrHeight;

    FT_Library ft;
    FT_Face face;
    void setup(std::string fontFilePath, float SCR_WIDTH, float SCR_HEIGHT) {
        if (FT_Init_FreeType(&ft)) {
            std::cerr << "ERROR: TEXT_H: Could not init FreeType library!\n";
            return;
        }

        if (FT_New_Face(ft, fontFilePath.c_str(), 0, &face)) {
            std::cerr << "ERROR: TEXT_H: Failed to load font!\n";
            return;
        }

        FT_Set_Pixel_Sizes(face, 0, 96);

        if (FT_Load_Char(face, 'X', FT_LOAD_RENDER)) {
            std::cerr << "ERROR: TEXT_H: Failed to load glyph!\n";
            return;
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cerr << "ERROR: TEXT_H: Failed to load " << c << "!\n";
                return;
            }

            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            Character sChar = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                face->glyph->advance.x
            };

            characters.insert(std::pair<char, Character>(c, sChar));
        }
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        //setup shader
        int success;
        char infoLog[512];

        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "ERROR: TEXT_H: Failed to compile vertexShader!\n" << infoLog;
        }

        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "ERROR: TEXT_H: Failed to compile fragmentShader!\n" << infoLog;
        }

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cerr << "ERROR: TEXT_H: Failed to link shaderProgram!\n" << infoLog;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        scrWidth = SCR_WIDTH;
        scrHeight = SCR_HEIGHT;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void renderText(std::string text, float x, float y, float textScale, glm::vec3 color, unsigned int shader=shaderProgram) {
        projection = glm::ortho(0.f, scrWidth, 0.f, scrHeight);
        float scale = textScale * scrWidth / 1600.f;

        glUseProgram(shader);
        glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(glGetUniformLocation(shader, "text"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        //iterates through all characters
        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++) {
            Character ch = characters[*c];

            float xpos = (x + ch.bearing.x * scale);
            float ypos = (y - (ch.size.y - ch.bearing.y) * scale); //allows chars to be rendered below line, such as 'p' or 'q'

            float w = ch.size.x * scale;
            float h = ch.size.y * scale;

            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
            };

            glBindTexture(GL_TEXTURE_2D, ch.textureID);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            x += (ch.advance >> 6) * scale;

            if (glGetError() != GL_NO_ERROR) {
                std::cerr << "OpenGL Error after setting up character: " << *c << std::endl;
            }
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void updateScreenSize(float width, float height) {
        scrWidth = width;
        scrHeight = height;
    }
}

#endif
