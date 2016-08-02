//SHENGYANG WEI     

#include "Loop.hpp"

Loop::Loop():Base(),Adaptive(false),Criteria(0)
{
}

//Store three vertices of this triangle into vertex buffer
void Loop::storeVertex(Polyhedron::Facet_handle f)
{
    Polyhedron::Halfedge_around_facet_circulator h=f->facet_begin();
    do{
        Polyhedron::Vertex_handle v=h->vertex();
        
        //Buffer new vertex position
        Point3 newPos=vertexPosition(v);
        vertexBuffer.insert(std::pair<Polyhedron::Vertex_handle,Point3>(v,newPos));

        ++h;
    }while(h!=f->facet_begin());
}


//Compute interior edge vertex
Point3 Loop::innerEdgeVertex(Polyhedron::Halfedge_const_handle h)
{
    return 3.0/8.0*h->vertex()->point()+
    3.0/8.0*h->opposite()->vertex()->point()+
    1.0/8.0*h->next()->vertex()->point()+
    1.0/8.0*h->opposite()->next()->vertex()->point();
}

//Compute border edge vertex position
Point3 Loop::borderEdgeVertex(Polyhedron::Halfedge_const_handle h)
{
    return 0.5*h->vertex()->point()+0.5*h->opposite()->vertex()->point();
}

//Wrap the two functions
Point3 Loop::computeEdgeVertex(Polyhedron::Halfedge_const_handle h)
{
    if(h->is_border_edge())
        return borderEdgeVertex(h);
    
    return innerEdgeVertex(h);
}


//Compute vertex position for both border vertex and inner vertex
Point3 Loop::vertexPosition(Polyhedron::Vertex_const_handle v)
{
    //Get the valence
    double n=static_cast<double>(v->vertex_degree());
    
    //Compute coefficient beta
    double beta = computeBeta(v, n);
    
    //Denote if this vertex is on border
    bool onBorder=false;
    
    //Interior vertex mask
    Point3 P1((1-n*beta)*v->point());
    
    //Border vertex mask
    Point3 P2(3.0/4.0*v->point());
    
    Polyhedron::Halfedge_around_vertex_const_circulator h=v->vertex_begin();

    do{
        //Interior vertex mask
        P1 = P1 + beta*h->opposite()->vertex()->point();
        
        //Border vertex mask, set onBorder true
        if(h->is_border_edge()){
            P2 = P2 + (1.0/8.0)*h->opposite()->vertex()->point();
            onBorder=true;
        }
        
        ++h;
    }while(h!=v->vertex_begin());
    
    //If it is boder vertex, return P2, otherwise return P1
    if(onBorder){
        return P2;
    }else{
        return P1;
    }
}


//Adaptive subdivisioin
void Loop::adaptive()
{
    std::size_t nv=mesh.size_of_vertices();
    std::size_t nf=mesh.size_of_facets();
    std::size_t ne=mesh.size_of_halfedges();
    
    //Reserve memory
    mesh.reserve(nv+ne, 2*ne+6*nf, 4*nf);
    
    std::vector<std::pair<Polyhedron::Halfedge_handle,Point3>> newEdgeVertices; //Edge vertex buffer
    std::vector<Polyhedron::Halfedge_handle> newHalfedges;//Halfedge buffer, used to split faces

    
    //For each edge...
    for(Polyhedron::Edge_iterator edgeIter=mesh.edges_begin();
        edgeIter!=mesh.edges_end();++edgeIter)
    {
        //Denote if two incident faces should be subdivided
        bool subdivideF1=false;
        bool subdivideF2=false;
        
        //Get two incident faces
        Polyhedron::Facet_handle f1=edgeIter->facet();
        Polyhedron::Facet_handle f2=edgeIter->opposite()->facet();
        
        
        if(f1!=Polyhedron::Facet_handle())
        {
            //If incident face should be subdivided, then buffer its three vertices
            subdivideF1=checkSubdivide(f1, Criteria);
            if(subdivideF1)
                storeVertex(f1);
            
        }
        if(f2!=Polyhedron::Facet_handle())
        {
            //If incident face should be subdivided, then buffer its three vertices
            subdivideF2=checkSubdivide(f2, Criteria);
            if(subdivideF2)
                storeVertex(f2);
        }
        
        //If either one or two incident faces should be divided, this edge should be split
        if(subdivideF1 || subdivideF2 )
        {
            //Compute edge position
            Point3 edgeV=computeEdgeVertex(edgeIter);
            //Add this edge and vertex position to buffer
            newEdgeVertices.emplace_back(edgeIter,edgeV);
        }
    }
    
    
    //For each edge which will be split
    for(auto ListIter=newEdgeVertices.begin();ListIter!=newEdgeVertices.end();++ListIter)
    {
        
        Polyhedron::Halfedge_handle h=ListIter->first;
        //Split halfedge
        mesh.split_edge(h);
        
        //Assign edge vertex position
        h->opposite()->vertex()->point()=ListIter->second;
        
        //Now we have four halfedges, add the two which are not point to the new edge vertex to buffer which will be used to split face
        if(!h->is_border())
            newHalfedges.push_back(h);
        if(!h->prev()->opposite()->is_border())
            newHalfedges.push_back(h->prev()->opposite());
    }

    for(auto ListIter=newHalfedges.begin();ListIter!=newHalfedges.end();++ListIter)
    {
        //split face
        Polyhedron::Halfedge_handle h=*ListIter;
        assert(h!=Polyhedron::Halfedge_handle());
        mesh.split_facet(h->prev(), h->next());
    }
    
    for(auto VerIter=vertexBuffer.begin();VerIter!=vertexBuffer.end();++VerIter)
    {
        //Update vertex position
        Polyhedron::Vertex_handle v=VerIter->first;
        assert(v!=Polyhedron::Vertex_handle());
        v->point()=VerIter->second;
    }
    
   // std::cout<<mesh.size_of_facets()<<"\n";
    
    vertexBuffer.clear();
}

//Loop subdivision
void Loop::uniform()
{
    std::size_t nv=mesh.size_of_vertices();
    std::size_t nf=mesh.size_of_facets();
    std::size_t ne=mesh.size_of_halfedges();
    
    //Reserve memory
    mesh.reserve(nv+ne, 2*ne+6*nf, 4*nf);
    
    std::vector<std::pair<Polyhedron::Halfedge_handle,Point3>> newEdgeVertices; //Edge vertex buffer
    
    //Vertex buffer
    std::vector<std::pair<Polyhedron::Vertex_handle,Point3>> vBuffer;
    
    std::vector<Polyhedron::Halfedge_handle> newHalfedges;//Halfedge buffer, used to split faces

    //Buffer all vertices
    for(Polyhedron::Vertex_iterator v=mesh.vertices_begin(); v!=mesh.vertices_end();++v)
    {
        Point3 p = vertexPosition(v);
        vBuffer.emplace_back(v,p);
    }
    
    //Buffer all edge vertices
    for(Polyhedron::Edge_iterator e = mesh.edges_begin();e!= mesh.edges_end();++e)
    {
        Point3 p = computeEdgeVertex(e);
        newEdgeVertices.emplace_back(e,p);
    }
    
    //Split edges
    for(auto edgeIter = newEdgeVertices.begin();edgeIter!=newEdgeVertices.end();++edgeIter)
    {
        Polyhedron::Halfedge_handle h=edgeIter->first;
        mesh.split_edge(h);
        h->opposite()->vertex()->point() = edgeIter->second;
        
        
        if(!h->is_border())
            newHalfedges.push_back(h);
        if(!h->prev()->opposite()->is_border())
            newHalfedges.push_back(h->prev()->opposite());
    }
    
    //Split face
    for(auto ListIter=newHalfedges.begin();ListIter!=newHalfedges.end();++ListIter)
    {
        Polyhedron::Halfedge_handle h=*ListIter;
        assert(h!=Polyhedron::Halfedge_handle());
        mesh.split_facet(h->prev(), h->next());
    }
    
    //Update vertices
    for(auto VerIter = vBuffer.begin();VerIter != vBuffer.end();++VerIter)
    {
        VerIter->first->point() = VerIter->second;
    }
    
    // std::cout<<mesh.size_of_facets()<<"\n";

    
    newHalfedges.clear();
}


void Loop::execution()
{
    if(Adaptive){
        adaptive();
    }else{
        uniform();
    }
}

//Compute the tangent normal at limited position
Vector3 Loop::limitNormal(Polyhedron::Vertex_const_handle v)
{
    
    Polyhedron::Halfedge_around_vertex_const_circulator h=v->vertex_begin();
    Point3 p0=v->point();
    //Normal for border vertex
    Vector3 N2(0.0,0.0,0.0);
    
    //Coefficient for inner vertex normal computation mask
    double coef = 2.0*M_PI/static_cast<double>(v->vertex_degree());
    
    //Two tangent mask
    Vector3 t0(CGAL::ORIGIN,CGAL::ORIGIN);
    Vector3 t1(t0);
    
    double k=0.0;
    bool onBorder=false;
    
    do{
        if(!h->is_border()){
            //For border vertex average the incident face normal
            Point3 p1=h->next()->vertex()->point();
            Point3 p2=h->opposite()->vertex()->point();
            Vector3 e1(p0,p1);
            Vector3 e2(p0,p2);
            N2 = N2 + CGAL::cross_product(e1,e2);
            
            //For inner vertex using tangent mask
            Point3 p3 = h->opposite()->vertex()->point();
            
            t0=t0+std::cos(coef*k)*Vector3(CGAL::ORIGIN,p3);
            t1=t1+std::cos(coef*(k-1.0))*Vector3(CGAL::ORIGIN,p3);
        }else{
            //If h is border, than vertex is border vertex
            onBorder = true;
        }
        --h;
        ++k;
    }while(h!=v->vertex_begin());
    
    //If on boundary, return N2, otherwise return result computed by inner vertex computation mask
    if(onBorder)
        return normalize(N2);
    else
        return normalize(CGAL::cross_product(t0,t1));
}

Point3 Loop::limitPosition(Polyhedron::Vertex_const_handle v)
{
    //Get the valence
    double n=static_cast<double>(v->vertex_degree());
    
    //Compute coefficient beta
    double beta = computeBeta(v, n);
    
    //Compute coefficient alpha
    double alpha = 1.0/(3.0/(8.0*beta)+n);
    
    Point3 P1((1-n*alpha)*v->point());
    
    Point3 P2(2.0/3.0*v->point());
    
    bool onBorder = false;
    
    Polyhedron::Halfedge_around_vertex_const_circulator h=v->vertex_begin();
    
    do{
        P1 = P1 + alpha* h->opposite()->vertex()->point();
        if(h->is_border_edge())
        {
            P2 = P2 + (1.0/6.0)* h->opposite()->vertex()->point();
            onBorder = true;
        }
        ++h;
    }while(h!=v->vertex_begin());
    
    if(onBorder){
        return P2;
    }else{
        return P1;
    }
}

//Compute vertex limit position and tangent normal at limit position
std::pair<Point3,Vector3> Loop::limitInfo(Polyhedron::Vertex_const_handle v)
{
    Point3 P0=v->point();
    
    //Get the valence
    double n=static_cast<double>(v->vertex_degree());
    
    //Coeffcient used to compute limit position
    double beta = computeBeta(v, n);
    double alpha = 1.0/(3.0/(8.0*beta)+n);
    
    //Coefficient used to compute tangent normal
    double coef = 2.0*M_PI/n;
    double k=0.0;
    
    //Two tangent mask for interior vertex
    Vector3 t0(CGAL::ORIGIN,CGAL::ORIGIN);
    Vector3 t1(t0);
    
    //Tangent normal for border vertex
    Vector3 N2(0.0,0.0,0.0);
    
    //Limit position of interior vertex
    Point3 P1((1-n*alpha)*P0);
    
    //Limit position of border vertex
    Point3 P2(2.0/3.0*P0);
    
    //Denote if on border
    bool onBorder = false;
    
    Polyhedron::Halfedge_around_vertex_const_circulator h=v->vertex_begin();
    
    do{
        if(!h->is_border_edge())
        {
            //Compute two tangent mask for interior vertex
            Point3 op=h->opposite()->vertex()->point();
            t0=t0+std::cos(coef*k)*Vector3(CGAL::ORIGIN,op);
            t1=t1+std::cos(coef*(k-1.0))*Vector3(CGAL::ORIGIN,op);
            
            //For border vertex, average its incident faces unit normal
            //Compute the two edge vector of a incident face
            Vector3 e1(P0,h->next()->vertex()->point());
            Vector3 e2(P0,h->opposite()->vertex()->point());
            //Use the two edge vector to compute unit normal and sum up
            N2 = N2 + normalize(CGAL::cross_product(e1, e2));
            
            
            //Compute interior vertex limit position
            P1 = P1 + alpha* h->opposite()->vertex()->point();
            
        }else{
            if(!h->is_border()){
                //For non-border halfedge, it also has incident face, so compute the normal
                Vector3 e1(P0,h->next()->vertex()->point());
                Vector3 e2(P0,h->opposite()->vertex()->point());
                N2 = N2 + normalize(CGAL::cross_product(e1, e2));
            }
            //Compute border vertex limit position
            P2 = P2 + 1.0/6.0*h->opposite()->vertex()->point();
            onBorder = true;
        }
        ++k;
        --h;
    }while(h!=v->vertex_begin());
    
    if(onBorder){
        N2 = normalize(N2/(n-1));
        return std::pair<Point3,Vector3>(P2,N2);
    }else{
        return std::pair<Point3,Vector3>(P1,normalize(CGAL::cross_product(t0, t1)));
    }
}



void Loop::createBuffers(std::vector<double>& vbuffer, std::vector<double>& nbuffer, std::vector<double>& ebuffer)
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
            typename Polyhedron::Vertex_const_handle v=h->vertex();
            std::pair<Point3,Vector3> PN = limitInfo(v);
            Point3 p = PN.first;
            Vector3 n = PN.second;
            vbuffer.push_back(p.x());
            vbuffer.push_back(p.y());
            vbuffer.push_back(p.z());
            
            nbuffer.push_back(n.x());
            nbuffer.push_back(n.y());
            nbuffer.push_back(n.z());
            
            ebuffer.push_back(BARY[i][0]);
            ebuffer.push_back(BARY[i][1]);
            ebuffer.push_back(BARY[i][2]);
            ++i;
            ++h;
        }while(h!=f->facet_begin());
    }
}

