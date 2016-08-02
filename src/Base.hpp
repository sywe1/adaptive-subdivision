//SHENGYANG WEI       

#ifndef Base_h
#define Base_h

#include "Mathematics.hpp"
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>

template<class Items>
class Base
{
public:
    typedef typename CGAL::Polyhedron_3<Kernel,Items> Polyhedron;

        
    Base(){}
    ~Base(){}
    
    std::size_t size_of_faces()
    {
        return mesh.size_of_facets();
    }
    
    std::size_t size_of_vertices()
    {
        return mesh.size_of_vertices();
    }
    
    void loadMesh()
    {
        
        if(!(std::cin>>mesh)){
            std::cerr<<"Read mesh fail!\n";
            std::exit(1);
        }
        
        if(!mesh.is_pure_triangle()){
            std::cerr<<"Only support triangle mesh\n";
            std::exit(1);
        }
    }
    
    void createBuffers(std::vector<double>& vbuffer, std::vector<double>& nbuffer, std::vector<double>& ebuffer);
    
protected:

    Vector3 FaceNormal(typename Polyhedron::Facet_const_handle f);
    
    Vector3 vertexNormal(typename Polyhedron::Vertex_const_handle v);
    

    double computeDihedral(typename Polyhedron::Facet_const_handle f1,typename Polyhedron::Facet_const_handle f2);
    
    bool checkSubdivide(typename Polyhedron::Facet_const_handle f, const double& Criteria);
    

    Polyhedron mesh;
};

//Create buffers for rendering
template<class Items>
void Base<Items>::createBuffers(std::vector<double>& vbuffer, std::vector<double>& nbuffer, std::vector<double>& ebuffer)
{
    const double BARY[][3]=
    {
        {1.0,0.0,0.0},
        {0.0,1.0,0.0},
        {0.0,0.0,1.0}
    };
    
    for(typename Polyhedron::Facet_const_iterator f=mesh.facets_begin();f!=mesh.facets_end();++f)
    {
        typename Polyhedron::Halfedge_around_facet_const_circulator h=f->facet_begin();
        int i=0;
        do{
            //Buffer three vertices of this face
            typename Polyhedron::Vertex_const_handle v=h->vertex();
            Point3 p = v->point();
            Vector3 n = vertexNormal(v);
            vbuffer.push_back(p.x());
            vbuffer.push_back(p.y());
            vbuffer.push_back(p.z());
            
            nbuffer.push_back(n.x());
            nbuffer.push_back(n.y());
            nbuffer.push_back(n.z());
            
            //The bary center code refer to Andrea's OpenGP/SurfaceMeshRenderFlat
            //https://github.com/ataiya/dgp/blob/master/OpenGP/SurfaceMesh/GL/SurfaceMeshRenderFlat.h
            ebuffer.push_back(BARY[i][0]);
            ebuffer.push_back(BARY[i][1]);
            ebuffer.push_back(BARY[i][2]);
            ++i;
            ++h;
        }while(h!=f->facet_begin());
    }
}

//Compute dihedral between
template<class Items>
double Base<Items>::computeDihedral(typename Polyhedron::Facet_const_handle f1, typename Polyhedron::Facet_const_handle f2)
{
    //Unit normal of the two faces
    Vector3 n1=FaceNormal(f1);
    Vector3 n2=FaceNormal(f2);
    
    //Compute cos(dihedral)
    double r=n1.x()*n2.x()+n1.y()*n2.y()+n1.z()*n2.z();
    
    //Clip if out of boundary
    if(r>1.0){
        r=1.0;
    }
    if(r<-1.0){
        r=-1.0;
    }
    return std::acos(r)/M_PI*180.0;
}

//Check if this face should be subdivided
template<class Items>
bool Base<Items>::checkSubdivide(typename Polyhedron::Facet_const_handle f, const double &Criteria)
{
    typename Polyhedron::Halfedge_around_facet_const_circulator h=f->facet_begin();
    do{
        typename Polyhedron::Halfedge_const_handle oh=h->opposite();
        if(!oh->is_border())
        {
            //Access the neighbour face
            typename Polyhedron::Facet_const_handle fNbor=oh->facet();
            
            double d=computeDihedral(f, fNbor);
            //If the dihedral between either one neighbour is larger than tolerance, this face should be subdivided
            if(d>=Criteria)
                return true;
        }
        ++h;
    }while(h!=f->facet_begin());
    //Otherwise not subdivide this face
    return false;
}

template<class Items>
Vector3 Base<Items>::vertexNormal(typename Polyhedron::Vertex_const_handle v)
{
    //Area weighted vertex normal
    typename Polyhedron::Halfedge_around_vertex_const_circulator h=v->vertex_begin();
    Point3 p0=v->point();
    Vector3 Normal(0.0,0.0,0.0);
    do{
        if(!h->is_border()){
            //Access the incident faces normal, weighted by their area
            Point3 p1=h->next()->vertex()->point();
            Point3 p2=h->opposite()->vertex()->point();
            Vector3 e1(p0,p1);
            Vector3 e2(p0,p2);
            
            Normal = Normal + CGAL::cross_product(e1,e2);
        }
        ++h;
    }while(h!=v->vertex_begin());
    
    
    return normalize(Normal);
}

template<class Items>
Vector3 Base<Items>::FaceNormal(typename Polyhedron::Facet_const_handle f)
{
    typename Polyhedron::Halfedge_const_handle h=f->halfedge();
    Point3 p1=h->vertex()->point();
    Point3 p2=h->next()->vertex()->point();
    Point3 p3=h->opposite()->vertex()->point();
    return unitNormal(p1,p2,p3);
}


#endif /* Base_h */
