//SHENGYANG WEI     

#include "Window.hpp"

Window* Window::active = nullptr;



Window::Window(int* argc, char** argv, int height, int width):
Height(height),Width(width),current(0),max(-1),showEdge(0)
{
    active=this;
    //Initialize to identity matrix
    Trans.setIdentity();
    
    Sca.setIdentity();
    
    Model.setIdentity();

    glutInit(argc, argv);
    
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    
    glutInitWindowSize(width,height);
    
    glutCreateWindow("Adaptive Subdivision");
    
    //Register functions
    glutDisplayFunc(show);
    
    glutReshapeFunc(resize);
    
    glutKeyboardFunc(keyboard);
    
    glutSpecialFunc(special);
    
    glutMouseFunc(mouse);
    
    glutMotionFunc(motion);
    
    glEnable(GL_DEPTH_TEST);
}


void Window::LookAt(Eigen::Vector3f eyePos, Eigen::Vector3f center, Eigen::Vector3f eyeUp)
{
    //Follow OpenGL instruction to build view matrix
    Eigen::Vector3f f=(center-eyePos).normalized();
    Eigen::Vector3f u=eyeUp.normalized();
    Eigen::Vector3f s=f.cross(u);
    
    View(0,0)=s.x();
    View(0,1)=s.y();
    View(0,2)=s.z();
    View(0,3)=-eyePos.x();
    View(1,0)=u.x();
    View(1,1)=u.y();
    View(1,2)=u.z();
    View(1,3)=-eyePos.y();
    View(2,0)=-f.x();
    View(2,1)=-f.y();
    View(2,2)=-f.z();
    View(2,3)=-eyePos.z();
    View(3,0)=0.0;
    View(3,1)=0.0;
    View(3,2)=0.0;
    View(3,3)=1.0;
}


void Window::Perspective(float fovy, float aspect, float near, float far)
{
    //Follow OpenGL instructions to build projection matrix
    float radf=M_PI*fovy/180.0;
    float f=1.0/std::tan(radf/2.0);
    Projection(0,0) = f/aspect;
    Projection(0,1) = 0;
    Projection(0,2) = 0;
    Projection(0,3) = 0;
    Projection(1,0) = 0;
    Projection(1,1) = f;
    Projection(1,2) = 0;
    Projection(1,3) = 0;
    Projection(2,0) = 0;
    Projection(2,1) = 0;
    Projection(2,2) = (far+near)/(near-far);
    Projection(2,3) = 2.0*far*near/(near-far);
    Projection(3,0) = 0;
    Projection(3,1) = 0;
    Projection(3,2) = -1.0;
    Projection(3,3) = 0;
    
    //Multiply by translation and scale to make sure model has right behavior when resizing the window
    Projection = Trans*Sca*Projection;
    
}

void Window::add(std::unique_ptr<Shader>& objptr,std::size_t nf, std::size_t nv)
{
    //Add models which will be displayed to window
    Scene.emplace_back(std::move(objptr));
    objptr = nullptr;
    Nfaces.push_back(nf);
    Nvertices.push_back(nv);
    ++max;
    assert(max == (Scene.size()-1));
}


void Window::run()
{
    glClearColor(0.4,0.4,0.4,1.0);

    glutMainLoop();
}

void Window::reshape(int width, int height)
{
    Width = width;
    Height = height;
    //Compute arcball radius
    Radius = std::sqrt(width*width + height*height);
    
    GLfloat aspect = static_cast<GLfloat>(width) / ((height) ? height : 1.0);
    
    glViewport(0, 0, width, height);
    
    //Set projection matrix
    Perspective(70.0, aspect, 0.1, 100.0);
    
    //Set view matrix
    LookAt(Eigen::Vector3f(0.0,0.0,5.0), Eigen::Vector3f(0.0,0.0,-1.0), Eigen::Vector3f(0.0,1.0,0.0));

}

//Arcball rotation
void Window::Rotation(const Eigen::Vector2f& pos)
{
    float x1=pos.x()-0.5*Width;
    float x2=mousePosition.x()-0.5*Width;
    float y1=pos.y()-0.5*Height;
    float y2=mousePosition.y()-0.5*Height;
    
    
    if((Radius*Radius-x1*x1-y1*y1)<0||(Radius*Radius-x2*x2-y2*y2)<0 || (x1==x2 && y1==y2)){
        return;
    }
    
    //Compute z coordinates
    float z1=std::sqrt(Radius*Radius - x1*x1 - y1*y1);
    float z2=std::sqrt(Radius*Radius - x2*x2 - y2*y2);
    
    Eigen::Vector3f v1(x1,y1,z1);
    Eigen::Vector3f v2(x2,y2,z2);
    
    //Compute rotation angle
    float r = v1.dot(v2)/(v1.norm()*v2.norm());
    
    //Clip value
    if(r>1)
        r=1;
    if(r<-1)
        r=-1;
    //Rotation angle
    float theta = std::acos(r)*1.5;
    
    //Compute rotation axis
    Eigen::Vector3f Nor((v2.cross(v1)).normalized());
    
    //Compute rotation matrix
    Eigen::Matrix4f Rot = Eigen::Affine3f(Eigen::AngleAxisf(theta,Nor)).matrix();
    
    //Modify model matrix
    Model = Rot*Model;
    
    glutPostRedisplay();
}

//Model Translation
void Window::Translation(const Eigen::Vector2f& pos)
{
    float x1=pos.x()-0.5*Width;
    float x2=mousePosition.x()-0.5*Width;
    float y1=pos.y()-0.5*Height;
    float y2=mousePosition.y()-0.5*Height;
    //Compute the translation matrix
    Eigen::Vector3f shift((x1-x2)/Width,(y1-y2)/Height,0);
    
    Eigen::Matrix4f T = Eigen::Affine3f(Eigen::Translation3f(shift)).matrix();
    
    //Change projection matrix
    Projection = T*Projection;
    
    //Record the translation operation
    Trans = T*Trans;
    
    glutPostRedisplay();
    
}

//Scale model function
void Window::Scale(const Eigen::Vector2f& pos){
    //Compute the scale magnitude
    float r= (pos-mousePosition).dot(Eigen::Vector2f(0.0,1.0))/100.0+1;
    
    Eigen::Matrix4f mat;
    mat.setIdentity();
    
    mat(1,1) *=r;
    mat(0,0) *=r;
    
    //Record the scale matrix
    Sca = mat*Sca;
    
    //Modify the projection matrix
    Projection = mat*Projection;
    
    glutPostRedisplay();
}

//Display text on screen
void Window::displayText()
{
    
    glWindowPos2f(3.0,Height-20);
    
    std::string str="FACES: "+std::to_string(Nfaces[current]);
    for(int i=0;i<str.size();++i)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
    
    glWindowPos2f(3.0, Height-40);
    str="VERTICES: "+std::to_string(Nvertices[current]);
    for(int i=0;i<str.size();++i)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
    
    glWindowPos2f(3.0, Height-60);
    str="LEVEL: "+std::to_string(current);
    for(int i=0;i<str.size();++i)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
}

void Window::display(){
    
    
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    
    Eigen::Matrix4f MV = Model;
    Eigen::Matrix4f MVP = Projection*View*Model;
    
    //Access the model that should be displayed
    std::unique_ptr<Shader>& object = Scene[current];
    
    //Use the corresponding shader program
    object->bindProgram();
    
    //Set modelview matrix and modelview projection matrix
    object->setMatrix("MV", MV);
    
    object->setMatrix("MVP", MVP);
    
    //If display edges
    object->setUniform("showedge", showEdge);
    
    //Bind corresponding vertex array object
    object->bindVAO();
    
    //Draw triangles
    glDrawArrays(GL_TRIANGLES, 0, object->size());
    
    displayText();
    
    glutSwapBuffers();
}

//Handle mousepress
void Window::mousepress(int button,int state,int x,int y)
{
    Eigen::Vector2f mousePos((float)x,(float)(Height-1-y));
    switch(button){
            //Rotate model
        case GLUT_LEFT_BUTTON:
            if(state==GLUT_DOWN){
                mode=MODE::ROTATE;
                mousePosition = mousePos;
            }else if(state == GLUT_UP){
                Rotation(mousePos);
            }
            break;
            //Translate model
        case GLUT_RIGHT_BUTTON:
            if(state == GLUT_DOWN){
                mode = MODE::TRANSLATE;
                mousePosition = mousePos;
            }else if(state == GLUT_UP){
                Translation(mousePos);
            }
            break;
            
            //Scale model
        case GLUT_MIDDLE_BUTTON:
            if(state == GLUT_DOWN){
                mode = MODE::SCALE;
                mousePosition = mousePos;
            }else if(state == GLUT_UP){
                Scale(mousePos);
            }
            break;
    }
    
}

//Handle mouse drag
void Window::mousemotion(int x, int y)
{
    Eigen::Vector2f mousePos((float)x,(float)(Height-1-y));
    switch(mode){
            //Rotation
        case MODE::ROTATE:
            Rotation(mousePos);
            mousePosition = mousePos;
            break;
            //Translation
        case MODE::TRANSLATE:
            Translation(mousePos);
            mousePosition = mousePos;
            break;
            //Scale
        case MODE::SCALE:
            Scale(mousePos);
            mousePosition = mousePos;
            break;
    }
}


//Resize callback function
void Window::resize(GLint width, GLint height){
    active->reshape(width,height);
}

//Display callback function
void Window::show(){
    active->display();
}

//Mouse press callback function
void Window::mouse(int button, int state, int x, int y)
{
    active->mousepress(button, state, x, y);
}

//Mouse motion callback function
void Window::motion(int x, int y)
{
    active->mousemotion(x, y);
}

//Increase subdivision level
void Window::increase()
{
    if(current>=max){
        return;
    }
    
    ++current;

}

//Decrease subdivision level
void Window::decrease()
{
    if(current<=0){
        return ;
    }
    
    --current;

}


//Keyboard function
void Window::keyboard(unsigned char key, int x, int y)
{
    switch(key){
            //Increase subdivision level by one
            case 'S':
            active->increase();
            glutPostRedisplay();
            break;
            //Decrease subdivision level by one
            case 's':
            active->decrease();
            glutPostRedisplay();
            break;
            //Toggle edge display;
            case 'e':
            active->showedge();
            glutPostRedisplay();
            break;
            //Quit program
            case 'q':
            std::exit(0);
            break;
    }
}

//Using Up and Down key to scale model
void Window::special(int key, int x, int y)
{
    switch(key){
        case GLUT_KEY_UP:
            active->Scale(active->getMouse()+Eigen::Vector2f(0.0,5.0));
            break;
        case GLUT_KEY_DOWN:
            active->Scale(active->getMouse()+Eigen::Vector2f(0.0,-5.0));
            break;
    }
}
