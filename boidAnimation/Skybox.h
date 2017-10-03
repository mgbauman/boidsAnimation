#ifndef LAB_LINUX
#include <glad/glad.h>
#else
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLFW/glfw3.h>
#include <iostream>

#include "stb/stb_image.h"
class Skybox {
    std::string face_names[6];

    bool initSkyboxVAO();

public:
    GLuint vao, vbo, tex_id;
    Skybox(std::string files[6]);
    bool CheckGLErrorsSB(std::string location);
};
