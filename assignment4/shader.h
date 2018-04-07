#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	GLuint ID;
    Shader() { this->ID = -1; }
	// Constructor generates the shader on the fly
    Shader(const GLchar* vertex_shader_path, const GLchar* fragment_shader_path, const GLchar* geometry_shader_path = nullptr) {
        std::string vertex_code;
        std::string fragment_code;
        std::string geometry_code;
        std::ifstream v_shader_file;
        std::ifstream f_shader_file;
        std::ifstream g_shader_file;
        
        v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        g_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        
        try{
            v_shader_file.open(vertex_shader_path);
            f_shader_file.open(fragment_shader_path);
            std::stringstream v_shader_stream, f_shader_stream;
            v_shader_stream << v_shader_file.rdbuf();
            f_shader_stream << f_shader_file.rdbuf();
            v_shader_file.close();
            f_shader_file.close();
            vertex_code = v_shader_stream.str();
            fragment_code = f_shader_stream.str();
            if (geometry_shader_path != nullptr) {
                g_shader_file.open(geometry_shader_path);
                std::stringstream g_shader_stream;
                g_shader_stream << g_shader_file.rdbuf();
                g_shader_file.close();
                geometry_code = g_shader_stream.str();
            }
        }
        catch (const std::exception&){
            std::cout << "Error: Shader not read\n";
        }
        const char* v_shader_code = vertex_code.c_str();
        const char* f_shader_code = fragment_code.c_str();
        
        GLuint vertex, fragement,geometry;
        GLchar info_log[512];
        vertex = glCreateShader(GL_VERTEX_SHADER);
        fragement = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(vertex, 1, &v_shader_code, NULL);
        glShaderSource(fragement, 1, &f_shader_code, NULL);
        glCompileShader(vertex);
        glCompileShader(fragement);
        check_compile_error(vertex, "VERTEX");
        check_compile_error(fragement, "FRAGMENT");
        
        if (geometry_shader_path != nullptr) {
            const char* g_shader_code = geometry_code.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &g_shader_code, NULL);
            glCompileShader(geometry);
            check_compile_error(geometry, "GEOMETRY");
        }
        
        this->ID = glCreateProgram();
        glAttachShader(this->ID, vertex);
        glAttachShader(this->ID, fragement);
        if (geometry_shader_path != nullptr) {
            glAttachShader(this->ID, geometry);
        }
        glLinkProgram(this->ID);
        check_compile_error(this->ID, "PROGRAM");
        glDeleteShader(vertex);
        glDeleteShader(fragement);
        if (geometry_shader_path != nullptr) {
            glDeleteShader(geometry);
        }
    }

	// Uses the current shader
	void Use() { glUseProgram(ID); }

	// utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const
    {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string &name, const glm::vec2 &value) const
    { 
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec2(const std::string &name, float x, float y) const
    { 
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, const glm::vec3 &value) const
    { 
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec3(const std::string &name, float x, float y, float z) const
    { 
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string &name, const glm::vec4 &value) const
    { 
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec4(const std::string &name, float x, float y, float z, float w) const
    { 
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string &name, const glm::mat2 &mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string &name, const glm::mat3 &mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    void check_compile_error(GLuint shader, std::string type) {
        GLint success;
        GLchar info_log[1024];
        if (type == "PROGRAM") {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, info_log);
                std::cout << "| Error:: PROGRAM-LINKING-ERROR of type: " << type << "|\n" << info_log << "\n| -- --------------------------------------------------- -- |\n";
            }
        }
        else if (type == "VERTEX" || type == "FRAGMENT" || type == "GEOMETRY") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, info_log);
                std::cout << "| Error:: SHADER-COMPILATION-ERROR of type: " << type << "|\n" << info_log << "\n| -- --------------------------------------------------- -- |\n";
            }
        }
        else {
            std::cout << "Error: incorrect input type\n";
        }
    }
};

#endif
