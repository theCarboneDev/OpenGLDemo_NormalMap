#ifndef GL_DEBUG_H
#define GL_DEBUG_H

#include <glad/glad.h>
#include <iostream>

namespace Debug {
    void glCheckError_(const char* file, int line) {
        while (true) {
            GLenum error = glGetError();
            if (error == 0) { break; }
            switch (error) {
            case(GL_INVALID_ENUM):                  std::cerr << "ERROR: INVALID_ENUM"; break;
            case(GL_INVALID_VALUE):                 std::cerr << "ERROR: INVALID_VALUE"; break;
            case(GL_INVALID_OPERATION):             std::cerr << "ERROR: INVALID_OPERATION"; break;
            case(1283):                             std::cerr << "ERROR: STACK_OVERFLOW"; break;
            case(1284):                             std::cerr << "ERROR: STACK_UNDERFLOW"; break;
            case(GL_OUT_OF_MEMORY):                 std::cerr << "ERROR: OUT_OF_MEMORY"; break;
            case(GL_INVALID_FRAMEBUFFER_OPERATION): std::cerr << "ERROR: INVALID_FRAMEBUFFER_OPERATION"; break;
            default:                                std::cerr << "ERROR: GL_UNKNOWN (" << error << ")"; break;
            }
            std::cout << " | Called from " << file << " at line " << line << '\n';
        }
    }
#define glCheckError() glCheckError_(__FILE__, __LINE__);
}
#endif
