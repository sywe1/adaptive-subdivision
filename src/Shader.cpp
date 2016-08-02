//SHENGYANG WEI     

#include "Shader.hpp"

#ifdef __APPLE__
static const char* src_vshader =R"(
#version 120
uniform mat4 MVP;
uniform mat4 MV;
attribute vec3 vposition;
attribute vec3 vnormal;
attribute vec3 vbaryc;
varying vec3 fnormal;
varying vec3 fbaryc;
void main(){
    gl_Position = MVP* vec4(vposition, 1.0);
    fnormal = mat3(MV)*vnormal;
    fbaryc = vbaryc;
}
)";


static const char* src_fshader = R"(
#version 120
uniform int showedge;
varying vec3 fnormal; ///< normal camera coords
varying vec3 fbaryc;


float edgeFactor(){
    vec3 d = 1.0*fwidth(fbaryc);
    
    vec3 t=clamp((fbaryc-vec3(0.0,0.0,0.0))/(d-vec3(0.0,0.0,0.0)),0.0,1.0);
    
    vec3 a = t*t*(3.0-2.0*t);
    
    return min(min(a.x,a.y),a.z);
}



void main(){
    vec3 ldir = normalize(vec3(0.0,0.0,1.0));
    float albedo= max(dot(normalize(fnormal),ldir),0);
    vec3 basecolor = vec3(0.9,0.9,0.9);
    
    vec4 edgecolor = vec4(0.0,0.0,0.0,1.0);
    vec4 facecolor = vec4(albedo*basecolor,1.0);
    
    gl_FragColor = mix(facecolor,edgecolor,showedge*(1-edgeFactor()));
    
}
)";
#elif __linux__

static const char* src_vshader = R"(
#version 130
uniform mat4 MVP;
uniform mat4 MV;
in vec3 vposition;
in vec3 vnormal;
in vec3 vbaryc;
out vec3 fnormal;
out vec3 fbaryc;

void main(){
    gl_Position = MVP * vec4(vposition,1.0);
    fnormal = mat3(MV) * vnormal;
    fbaryc = vbaryc;
}
)";


static const char* src_fshader = R"(
#version 130
uniform int showedge;
in vec3 fnormal;
in vec3 fbaryc;
out vec4 fragcolor;

float edgeFactor(){
    vec3 d = 1.0 * fwidth(fbaryc);
    vec3 a3 = smoothstep(vec3(0.0),d,fbaryc);
    return min(min(a3.x,a3.y),a3.z);
}

void main(){
    vec3 ldir = normalize(vec3(0.0,0.0,1.0));
    
    float albedo = max(dot(normalize(fnormal),ldir),0);
    
    vec3 basecolor = vec3(0.9,0.9,0.9);
    
    vec4 facecolor = vec4(albedo*basecolor,1);
    
    vec4 edgecolor = vec4(0.0,0.0,0.0,1.0);
    
    fragcolor = mix(facecolor,edgecolor,showedge*(1.0-edgeFactor()));
}

)";
#endif

Shader::Shader()
{
    //Create program
    pid = glCreateProgram();
    
    //Bind program
    glUseProgram(pid);
#ifdef __APPLE__
    //Create Vertex array object
    glGenVertexArraysAPPLE(1, &VAO);
    glBindVertexArrayAPPLE(VAO);
#elif __linux__
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);
#endif
    
    //Create three buffers
    glGenBuffers(1, &VertexBuffer);
    glGenBuffers(1, &NormalBuffer);
    glGenBuffers(1, &BarycBuffer);
}

void Shader::init()
{
    addVertexShader(src_vshader);
    addFragmentShader(src_fshader);
    linkProgram();
}

//Initialize vertex buffer
void Shader::bindVertexBuffer(const std::vector<double>& buffer)
{
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(double)*buffer.size(), buffer.data(), GL_STATIC_DRAW);
}

//Initialize normal buffer
void Shader::bindNormalBuffer(const std::vector<double>& buffer)
{
    glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(double)*buffer.size(), buffer.data(), GL_STATIC_DRAW);
}

//Initialize bary center buffer
void Shader::bindBarycBuffer(const std::vector<double>& buffer)
{
    glBindBuffer(GL_ARRAY_BUFFER, BarycBuffer);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(double)*buffer.size(), buffer.data(), GL_STATIC_DRAW);
}

//Bind buffers
void Shader::bindBuffers(const std::vector<double> &vbuffer, const std::vector<double> &nbuffer, const std::vector<double> &ebuffer)
{
    bindVertexBuffer(vbuffer);
    bindNormalBuffer(nbuffer);
    bindBarycBuffer(ebuffer);
}

//Set vertex position
void Shader::setPosition()
{
    GLint location = glGetAttribLocation(pid, "vposition");
        
    glEnableVertexAttribArray(location);
    
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    
    glVertexAttribPointer(location, 3, GL_DOUBLE, GL_FALSE, 0, 0);
}

//Set vertex normal
void Shader::setNormal()
{
    
    GLint location = glGetAttribLocation(pid, "vnormal");
    
    glEnableVertexAttribArray(location);
    
    glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
    
    glVertexAttribPointer(location, 3, GL_DOUBLE, GL_FALSE, 0, 0);
}

void Shader::setBarycenter()
{
 //Set bary center center used to display edges
    GLint location = glGetAttribLocation(pid, "vbaryc");
    
    glEnableVertexAttribArray(location);
    
    glBindBuffer(GL_ARRAY_BUFFER, BarycBuffer);
    
    glVertexAttribPointer(location, 3, GL_DOUBLE, GL_FALSE, 0, 0);
}

//Set attributes
void Shader::setAttribute()
{
    glUseProgram(pid);
    bindVAO();
    
    setPosition();
    setNormal();
    setBarycenter();
}

//Set matrix of shader
void Shader::setMatrix(const char* name, const Eigen::Matrix4f& mat)
{
    glUseProgram(pid);
    
    GLint location = glGetUniformLocation(pid, name);
    
    glUniformMatrix4fv(location,1,GL_FALSE,mat.data());
}

//Set uniform
void Shader::setUniform(const char* name, int value)
{
    glUseProgram(pid);
    
    GLint location = glGetUniformLocation(pid, name);
    
    glUniform1i(location,value);
}

void Shader::addVertexShader(const char* code)
{
    //Create vertex shader
    GLuint vshaderID = glCreateShader(GL_VERTEX_SHADER);
    
    //Compile fragment shader
    glShaderSource(vshaderID, 1, &code, NULL);
    glCompileShader(vshaderID);
    
    //Check if compiled
    GLint Success = GL_FALSE;
    glGetShaderiv(vshaderID, GL_COMPILE_STATUS, &Success);
    if (!Success) {
        int InfoLogLength;
        glGetShaderiv(vshaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        std::vector<char> VertexShaderErrorMessage(InfoLogLength);
        glGetShaderInfoLog(vshaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        std::cerr << std::string(&VertexShaderErrorMessage[0])<<"\n";
        std::exit(1);
    } else {
        glAttachShader(pid, vshaderID);
    }
}

void Shader::addFragmentShader(const char* code)
{
    // Create the Fragment Shader
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
        
    // Compile Fragment Shader
    glShaderSource(FragmentShaderID, 1, &code, NULL);
    glCompileShader(FragmentShaderID);
        
    // Check Fragment Shader
    GLint Success = GL_FALSE;
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Success);
    if (!Success) {
        int InfoLogLength;
        glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        std::cerr << std::string(&FragmentShaderErrorMessage[0]);
        std::exit(1);
    } else {
        glAttachShader(pid, FragmentShaderID);
    }
}

void Shader::linkProgram()
{
    //Link program
    glLinkProgram(pid);
    
    //Check status
    //Codes below refer to Andrea's OpenGP/Shader.cpp
    //https://github.com/ataiya/dgp/blob/master/OpenGP/GL/Shader.cpp
    GLint Success = GL_FALSE;
    glGetProgramiv(pid, GL_LINK_STATUS, &Success);
    if (!Success) {
        int InfoLogLength;
        glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &InfoLogLength);
        std::vector<char> ProgramErrorMessage( std::max(InfoLogLength, int(1)) );
        glGetProgramInfoLog(pid, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        std::cerr << "Failed: " << &ProgramErrorMessage[0];
        std::exit(1);
    }
}



