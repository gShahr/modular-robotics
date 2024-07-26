#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    unsigned int ID;

    // Constructor. Pass a vertex and fragment shader filepath and it will create a Shader program.
    Shader(const char* vertexPath, const char* fragmentPath);

    // Call to glUseProgram() and update global modelLoc, transformLoc, etc.
    void use();

    int colorLoc, modelLoc, transformLoc, surfaceNormalLoc, borderAttrsLoc; // Object attribute locations
    int viewLoc, projLoc; // Camera attribute locations
    int timeLoc; // Additional attribute locations

    // Set uniform values in the shader.
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
};

#endif
