#ifndef SHADER_H
#define SHADER_H

#include <string>

#include <GL/glew.h> // Include glew to get all the required OpenGL headers

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:

    // The program ID
    GLuint Program;

    // Constructor reads and builds the shader
    Shader();

    Shader(const GLchar* vertexPath,
           const GLchar* fragmentPath,
           const GLchar* geometryPath = 0);

    // Use the program
    void use() const
    {
        glUseProgram(Program);
    }

    bool init(const GLchar* vertexPath,
              const GLchar* fragmentPath,
              const GLchar* geometryPath = 0);

    void setUniform(const std::string& key, GLfloat value) const
    {
        glUniform1f(glGetUniformLocation(Program, key.c_str()), value);
    }

    void setUniform(const std::string& key, GLint value) const
    {
        glUniform1i(glGetUniformLocation(Program, key.c_str()), value);
    }

    void setUniform(const std::string& key, GLuint value) const
    {
        glUniform1i(glGetUniformLocation(Program, key.c_str()), value);
    }

    void setUniform(const std::string& key, const glm::vec3& vec3) const
    {
        glUniform3f(glGetUniformLocation(Program, key.c_str()), vec3.x, vec3.y, vec3.z);
    }

    void setUniform(const std::string& key, const glm::vec4& vec4) const
    {
        glUniform4f(glGetUniformLocation(Program, key.c_str()), vec4.x, vec4.y, vec4.z, vec4.w);
    }

    void setUniform(const std::string& key, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(Program, key.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
};

#endif // ifndef SHADER_H
