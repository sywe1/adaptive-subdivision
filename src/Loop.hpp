//SHENGYANG WEI         

#ifndef Loop_h
#define Loop_h
#include "Base.hpp"
#include <map>
#include <set>
class Loop:public Base<CGAL::Polyhedron_items_3>{
public:
    Loop();
    ~Loop(){}
    
    void setCriteria(const double& t)
    {
        Criteria=t;
    }
    
    
    void enableAdaptive()
    {
        Adaptive = true;
    }
    
    void execution();
    
    void adaptive();
    
    void uniform();
    
    void createBuffers(std::vector<double>& vbuffer, std::vector<double>& nbuffer, std::vector<double>& ebuffer);
private:
    
    double computeBeta(Polyhedron::Vertex_const_handle v, double n)
    {
        return ((0.625-std::pow(0.375+0.25*std::cos(2.0*M_PI/n),2))/n);
    }
    
    Point3 vertexPosition(Polyhedron::Vertex_const_handle v);
    
    Point3 limitPosition(Polyhedron::Vertex_const_handle v);
    
    Point3 innerEdgeVertex(Polyhedron::Halfedge_const_handle h);
    
    Point3 borderEdgeVertex(Polyhedron::Halfedge_const_handle h);
    
    Point3 computeEdgeVertex(Polyhedron::Halfedge_const_handle h);
    
    void storeVertex(Polyhedron::Facet_handle f);
    
    Vector3 limitNormal(Polyhedron::Vertex_const_handle v);
    
    std::pair<Point3,Vector3> limitInfo(Polyhedron::Vertex_const_handle v);
    
    bool Adaptive;//Denote adaptive mode or uniform mode
    std::map<Polyhedron::Vertex_handle, Point3> vertexBuffer; //Vertex buffer
    double Criteria; //Subdivision tolerance
};



#endif /* Loop_h */
