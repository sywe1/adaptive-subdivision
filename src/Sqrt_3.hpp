//SHENGYANG WEI     

#ifndef Sqrt_3_hpp
#define Sqrt_3_hpp

#include "Base.hpp"
#include <set>
#include <map>
template<class Refs,class Traits>
struct MyFace:public CGAL::HalfedgeDS_face_base<Refs,Traits>
{
    int level;
    MyFace():level(0){}
};


struct MyItems:public CGAL::Polyhedron_items_3
{
    template<class Refs,class Traits>
    struct Face_wrapper
    {
        typedef MyFace<Refs,CGAL::Tag_true> Face;
    };
};

class Sqrt_3:public Base<MyItems>
{
    typedef std::pair<Polyhedron::Vertex_handle,Point3> VPOS;
private:
    std::set<Polyhedron::Halfedge_handle> EdgesTobeFlip;
    std::set<Polyhedron::Facet_handle> FacesTobeSplit;
    std::set<Polyhedron::Facet_handle> OddFaces;

    std::map<Polyhedron::Vertex_handle,Point3> vertexBuffer;
    double Criteria;
    bool isOdd;
    bool Adaptive;
    
public:
    Sqrt_3():Criteria(0),isOdd(false),Adaptive(false){}
    
    ~Sqrt_3(){}
    
    void setCriteria(double t)
    {
        Criteria=t;
    }
    
    void enableAdaptive()
    {
        Adaptive = true;
    }
    
    double computeBeta(Polyhedron::Vertex_const_handle v,double n)
    {
        return (4.0-2.0*std::cos(2.0*M_PI/n))/(9.0*n);
    }
    
    void storeVertex(Polyhedron::Facet_handle f);
    
    Point3 vertexPosition(Polyhedron::Vertex_const_handle v);
    
    Point3 faceCenter(Polyhedron::Facet_const_handle f);
    
    void flipEdge(Polyhedron::Halfedge_handle h);
    
    void splitFaceOdd(Polyhedron::Facet_handle f);
    
    void splitFace(Polyhedron::Facet_handle f);
    
    void splitOddNeighbor(Polyhedron::Facet_handle f);
    
    void split(Polyhedron::Facet_handle f);
    
    void adaptive();
    
    void uniform();
    
    void execution();
};

#endif /* Sqrt_3_hpp */
