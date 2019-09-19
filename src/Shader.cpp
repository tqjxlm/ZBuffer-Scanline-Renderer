#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#define SHADERFAILED(path)                                       \
    std::cout << "Shader failed to load: " << path << std::endl; \
    return false;

Shader::Shader()
{}

Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath)
{
    this->init(vertexPath, fragmentPath, geometryPath);
}

bool Shader::init(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath)
{
    // 1. Retrieve the vertex/fragment source code from filePath
    std::string   vertexCode;
    std::string   fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::string   geometryCode;
    std::ifstream gShaderFile;

    // ensures ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::badbit);
    gShaderFile.exceptions(std::ifstream::badbit);

    if (geometryPath)
    {}

    try
    {
        // Open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        // Read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        // close file handlers
        vShaderFile.close();
        fShaderFile.close();

        // Convert stream into GLchar array
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        if (geometryPath)
        {
            gShaderFile.open(geometryPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        SHADERFAILED(vertexPath)
    }
    const GLchar* vShaderCode = vertexCode.c_str();
    const GLchar* fShaderCode = fragmentCode.c_str();
    const GLchar* gShaderCode = geometryCode.c_str();

    // 2. Compile shaders
    GLuint vertex, fragment, geometry;
    GLint  success;
    GLchar infoLog[512];

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);

    // Print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        SHADERFAILED(vertexPath)
    }

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);

    // Print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        SHADERFAILED(fragmentPath)
    }

    if (geometryPath)
    {
        // Geometry Shader
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, nullptr);
        glCompileShader(geometry);

        // Print compile errors if any
        glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(geometry, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
            SHADERFAILED(geometryPath)
        }
    }

    // Shader Program
    Program = glCreateProgram();
    glAttachShader(this->Program,   vertex);
    glAttachShader(this->Program, fragment);

    if (geometryPath)
    {
        glAttachShader(this->Program, geometry);
    }

    // Link program
    glLinkProgram(this->Program);
    glGetProgramiv(this->Program, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(this->Program, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        SHADERFAILED(vertexPath)
    }

    // Delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (geometryPath)
    {
        glDeleteShader(geometry);
    }

    return true;
}
