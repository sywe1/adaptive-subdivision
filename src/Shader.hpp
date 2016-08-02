//SHENGYANG     WEI     

#ifndef Shader_h
#define Shader_h
#ifdef __APPLE__
#include <OpenGL/gl.h>
#elif __linux__
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include<vector>
#include<eigen3/Eigen/Dense>
#include <iostream>

class Shader{
private:
    GLuint pid; //Program ID;
    GLuint VAO;//Vertex Array Object
    GLuint VertexBuffer;//vertex buffer
    GLuint NormalBuffer;//normals buffer
    GLuint BarycBuffer;//bary center buffer, used to draw edges
    int Size;//Total vertices
public:
    Shader();
    ~Shader()
    {
        glDeleteBuffers(1,&VertexBuffer);
        glDeleteBuffers(1,&NormalBuffer);
        glDeleteBuffers(1,&BarycBuffer);
#ifdef __APPLE__
        glDeleteVertexArraysAPPLE(1, &VAO);
#elif __linux__
        glDeleteVertexArrays(1,&VAO);
#endif
        glDeleteProgram(pid);
        Size=0;
    }
    
    void setSize(int n)
    {
        Size = n;
    }
    
    int size()
    {
        return Size;
    }
    
    void init();
    
    void bindProgram()
    {
        glUseProgram(pid);
    }
    
    GLuint programID()
    {
        return pid;
    }
#ifdef __APPLE__
    void bindVAO()
    {
        //Bind vertex array object
        glBindVertexArrayAPPLE(VAO);
    }
#elif __linux__
    void bindVAO()
    {
        glBindVertexArray(VAO);
    }
#endif
    
    void addVertexShader(const char* code);
    void addFragmentShader(const char* code);
    void linkProgram();
    
    void bindVertexBuffer(const std::vector<double>& vertexBuffer);
    void bindNormalBuffer(const std::vector<double>& normalBuffer);
    void bindBarycBuffer(const std::vector<double>& barycBuffer);
    
    void bindBuffers(const std::vector<double>& vbuffer,const std::vector<double>& nbuffer,
                     const std::vector<double>& ebuffer);
    
    
    void setPosition();
    void setNormal();
    void setBarycenter();
    
    void setAttribute();
    
    void setMatrix(const char* name, const Eigen::Matrix4f& mat);
    
    void setUniform(const char* name, int value);
};

#endif /* Shader_h */
