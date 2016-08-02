//SHENGYANG WEI   

#ifndef Camera_hpp
#define Camera_hpp
#include "Shader.hpp"

#ifdef __APPLE__
#include<GLUT/glut.h>
#elif __linux__
#include <GL/glut.h>
#endif

#include <eigen3/Eigen/Dense>
#include <memory>


class Window{
    
    enum MODE {ROTATE,SCALE,TRANSLATE};

private:
    Eigen::Matrix4f Model; //Model matrix
    Eigen::Matrix4f View; //View matrix
    Eigen::Matrix4f Projection; //Projection matrix
    Eigen::Matrix4f Trans; //Recorde the translation
    Eigen::Matrix4f Sca;//Record the Scale
    int Height; //Window height
    int Width; //Window width
    float Radius; //Arcball Radius
    MODE mode; //Mouse drag mode
    Eigen::Vector2f mousePosition;//Mouse position
    int current;//Current subdivision level
    int max; // Max subdivision level
    int showEdge; //If display edge
    std::vector<std::unique_ptr<Shader>> Scene; //Models of different subdivision level
    std::vector<std::size_t> Nfaces;//Number of faces at each subdivision level
    std::vector<std::size_t> Nvertices;//Number of vertices at each subdivision level
    
    
public:
    
    static Window* active; //Pointer to active window
    
    Window(int* argc, char** argv,int height,int width);

    //Set view matrix
    void LookAt(Eigen::Vector3f eyePos,Eigen::Vector3f center,Eigen::Vector3f eyeUp);
    
    //Set projection matrix
    void Perspective(float fovy, float aspect, float near, float far);
    
    //Rotate model
    void Rotation(const Eigen::Vector2f& pos);
    
    //Scale model
    void Scale(const Eigen::Vector2f& pos);
    
    //Translate model
    void Translation(const Eigen::Vector2f& pos);
    
    //Window reshape
    void reshape(GLint width,GLint height);
    
    
    Eigen::Vector2f getMouse()
    {
        return mousePosition;
    }
    
    void showedge()
    {
        showEdge = 1-showEdge;
    }
   

    void display();
    
    //Increase subdivision level
    void increase();
    
    //Decrease subdivision level
    void decrease();
    
    //Handle mouse press
    void mousepress(int button,int state,int x,int y);
    
    //Handle mouse drag
    void mousemotion(int x,int y);
    
    //Add model to window
    void add(std::unique_ptr<Shader>& objptr,std::size_t nf,std::size_t nv);
    
    //Enter main loop, waiting for event
    void run();
    
    void displayText();
    
    //Display callback function
    static void show();
    
    //Resize callback function
    static void resize(GLint width, GLint height);

    //Mouse motion callback function
    static void motion(int x,int y);
    
    //Mouse press callback function
    static void mouse(int button,int state, int x, int y);
    
    //Keyboard callback function
    static void keyboard(unsigned char key, int x, int y);
    
    //Special key callback function
    static void special(int key, int x, int y);
};


#endif /* Camera_hpp */
