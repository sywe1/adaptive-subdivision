//
//  main.cpp
//  Loop
//
//  Created by S WEI on 16/7/20.
//  Copyright © 2016年 S WEI. All rights reserved.
//


#include "Window.hpp"
#include "Loop.hpp"
#include "Sqrt_3.hpp"
#include <unistd.h>
#include <limits>

void usage()
{
    std::cerr<<"Accept option and value: \n"
    <<"[-a ]:Enable adaptive mode\n"
    <<"[-m $(Method)]:Accetp either loop or sqrt3\n"
    <<"[-l $(max_levels)]:Terminate condition in terms of maximum levels,by default, it is max int\n"
    <<"[-f $(max_faces)]:Terminate condition in terms of maximum faces, by default, it is max int\n"
    <<"[-t $(angle)]:Dihedral tolerance\n";
    std::exit(1);
}


double convertDouble(const std::string& arg)
{
    std::size_t n(0);
    
    double v;
    try{
        v = std::stod(arg, &n);
    }catch(...){
        std::cerr<<arg<<" is not a valid value\n";
        std::exit(1);
    }
    
    if(n!=arg.size())
    {
        std::cerr<<arg<<" is not a valid value\n";
        std::exit(1);
    }
    
    return v;
}

int convertInt(const std::string& arg)
{
    std::size_t n(0);
    
    double v;
    try{
        v = std::stoi(arg, &n);
    }catch(...){
        std::cerr<<arg<<" is not a valid value\n";
        std::exit(1);
    }
    
    if(n!=arg.size())
    {
        std::cerr<<arg<<" is not a valid value\n";
        std::exit(1);
    }
    
    return v;
}


//Process function for computation and rendering 
template<class T>
void process(T& mesh, Window& window, const double& angle, bool adaptive, const int& maxLevel, const int& maxFaces)
{
    std::cout<<"Precomputing...\n";
    mesh.loadMesh();
    mesh.setCriteria(angle);
    if(adaptive)
        mesh.enableAdaptive();
    
    std::vector<double> vbuffer;
    std::vector<double> nbuffer;
    std::vector<double> ebuffer;
    
    mesh.createBuffers(vbuffer,nbuffer,ebuffer);
    std::unique_ptr<Shader> shader=std::make_unique<Shader>();
    //Initialize shader
    shader->init();
    //Initialize data buffer
    shader->bindBuffers(vbuffer, nbuffer, ebuffer);
    //Set shader attribute
    shader->setAttribute();
    shader->setSize(vbuffer.size()/3);
    
    int Level = 0;
    //Add to window
    window.add(shader,mesh.size_of_faces(),mesh.size_of_vertices());
    
    //Clear buffer for next iteration
    vbuffer.clear();
    nbuffer.clear();
    ebuffer.clear();
    
    while(mesh.size_of_faces()<maxFaces && Level < maxLevel)
    {
        std::size_t before=mesh.size_of_faces();

        mesh.execution();
        ++Level;
        
        mesh.createBuffers(vbuffer,nbuffer,ebuffer);
        
        shader=std::make_unique<Shader>();
        shader->init();
        shader->bindBuffers(vbuffer, nbuffer, ebuffer);
        shader->setAttribute();
        shader->setSize(vbuffer.size()/3);
        
        window.add(shader,mesh.size_of_faces(),mesh.size_of_vertices());
        
        vbuffer.clear();
        nbuffer.clear();
        ebuffer.clear();
        
        double rise =static_cast<double>(mesh.size_of_faces()-before)/static_cast<double>(mesh.size_of_faces());
       
        if(rise<0.01){
            break;
        }
    }
    std::cout<<"Maximum faces: "<<mesh.size_of_faces()<<"\n";

    
}


int main(int argc,char** argv)
{
    char c;
    
    //Default subdivision mode
    bool enableAdaptive = false;
    
    //Default dihedral tolerance
    double angle=0.0;
    
    //Default subdivision level limitation
    int maxlevel=std::numeric_limits<int>::max();
    
    //Default number of faces limitation
    int maxfaces=std::numeric_limits<int>::max();
    
    std::string method;
    
    while((c=getopt(argc,argv,"at:l:m:f:"))!=EOF)
    {
        switch(c){
                case 'a':
                enableAdaptive=true;
                break;
                
                case 't':
                angle = convertDouble(optarg);
                break;
                
                case 'l':
                maxlevel = convertInt(optarg);
                break;
                
                case 'f':
                maxfaces = convertInt(optarg);
                break;
                
                case 'm':
                method = std::string(optarg);
                if(method != "loop" && method != "sqrt3"){
                    usage();
                }
                break;
                
            default:
                usage();
                break;
                
                
        }
    }
        
    
    if(method == ""){
        usage();
    }
   
    
    Window window(&argc,argv,1024,1024);

    
    if(method == "loop")
    {
        Loop loop;
        process(loop,window,angle,enableAdaptive,maxlevel,maxfaces);
    }else if(method == "sqrt3")
    {
        Sqrt_3 sqrt;
        process(sqrt,window,angle,enableAdaptive,maxlevel,maxfaces);
    }
    
    window.run();
    
    return 0;
}
