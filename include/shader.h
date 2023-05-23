#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);

    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use() const
    {
        glUseProgram(ID);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec2(const std::string& name, float x, float y) const
    {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec3(const std::string& name, float x, float y, float z) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec4(const std::string& name, float x, float y, float z, float w) const
    {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string& name, const glm::mat2& mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string& name, const glm::mat3& mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setDirLight(const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular)
    {
        setVec3("dirLight.direction", direction);
        setVec3("dirLight.ambient", ambient);
        setVec3("dirLight.diffuse", diffuse);
        setVec3("dirLight.specular", specular);
    }
    // ------------------------------------------------------------------------
    void setDirLight(const int index, const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular)
    {
        setVec3("dirLight[" + std::to_string(index) + "].direction", direction);
        setVec3("dirLight[" + std::to_string(index) + "].ambient", ambient);
        setVec3("dirLight[" + std::to_string(index) + "].diffuse", diffuse);
        setVec3("dirLight[" + std::to_string(index) + "].specular", specular);
    }
    // ------------------------------------------------------------------------
    void setPointLight(const glm::vec3& position, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const float constant, const float linear, const float quadratic)
    {
        setVec3("pointLights.position", position);
        setVec3("pointLights.ambient", ambient);
        setVec3("pointLights.diffuse", diffuse);
        setVec3("pointLights.specular", specular);
        setFloat("pointLights.constant", constant);
        setFloat("pointLights.linear", linear);
        setFloat("pointLights.quadratic", quadratic);
    }
    // ------------------------------------------------------------------------
    void setPointLight(const int index, const glm::vec3& position, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const float constant, const float linear, const float quadratic)
    {
        setVec3("pointLights[" + std::to_string(index) + "].position", position);
        setVec3("pointLights[" + std::to_string(index) + "].ambient", ambient);
        setVec3("pointLights[" + std::to_string(index) + "].diffuse", diffuse);
        setVec3("pointLights[" + std::to_string(index) + "].specular", specular);
        setFloat("pointLights[" + std::to_string(index) + "].constant", constant);
        setFloat("pointLights[" + std::to_string(index) + "].linear", linear);
        setFloat("pointLights[" + std::to_string(index) + "].quadratic", quadratic);
    }
    // ------------------------------------------------------------------------
    void setSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const float constant, const float linear, const float quadratic, const float cutOff, const float outerCutOff)
    {
        setVec3("spotLight.position", position);
        setVec3("spotLight.direction", direction);
        setVec3("spotLight.ambient", ambient);
        setVec3("spotLight.diffuse", diffuse);
        setVec3("spotLight.specular", specular);
        setFloat("spotLight.constant", constant);
        setFloat("spotLight.linear", linear);
        setFloat("spotLight.quadratic", quadratic);
        setFloat("spotLight.cutOff", cutOff);
        setFloat("spotLight.outerCutOff", outerCutOff);
    }
    // ------------------------------------------------------------------------
    void setSpotLight(const int index, const glm::vec3& position, const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const float constant, const float linear, const float quadratic, const float cutOff, const float outerCutOff)
    {
        setVec3("spotLight[" + std::to_string(index) + "].position", position);
        setVec3("spotLight[" + std::to_string(index) + "].direction", direction);
        setVec3("spotLight[" + std::to_string(index) + "].ambient", ambient);
        setVec3("spotLight[" + std::to_string(index) + "].diffuse", diffuse);
        setVec3("spotLight[" + std::to_string(index) + "].specular", specular);
        setFloat("spotLight[" + std::to_string(index) + "].constant", constant);
        setFloat("spotLight[" + std::to_string(index) + "].linear", linear);
        setFloat("spotLight[" + std::to_string(index) + "].quadratic", quadratic);
        setFloat("spotLight[" + std::to_string(index) + "].cutOff", cutOff);
        setFloat("spotLight[" + std::to_string(index) + "].outerCutOff", outerCutOff);
    }
private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(GLuint shader, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
#endif