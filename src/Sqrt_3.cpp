//SHENGYANG  WEI    

#include "Sqrt_3.hpp"

//Compute the face center vertex position
Point3 Sqrt_3::faceCenter(Polyhedron::Facet_const_handle f){
    Polyhedron::Halfedge_const_handle h=f->halfedge();
    Point3 center= h->vertex()->point()+h->next()->vertex()->point()+h->opposite()->vertex()->point();
    return center/3.0;
}

//Store three vertices position of this triangle
void Sqrt_3::storeVertex(Polyhedron::Facet_handle f)
{
    Polyhedron::Halfedge_around_facet_circulator h=f->facet_begin();
    
    //Traverse vertices around face
    do{
        //Access the vertex v incident to h
        Polyhedron::Vertex_handle v=h->vertex();
        
        Point3 newPos = vertexPosition(v);
        
        vertexBuffer.insert(VPOS(v,newPos));
        
        ++h;
        
    }while(h!=f->facet_begin());
}

//Compute vertex position for both inner and border vertex
Point3 Sqrt_3::vertexPosition(Polyhedron::Vertex_const_handle v)
{
    //Get the valence
    double n=static_cast<double>(v->vertex_degree());
    //Compute beta
    double beta = computeBeta(v,n);
    //Denote if this vertex is on border
    bool onBorder=false;
    
    //In even iteration, border vertex remain unchange
    Point3 P0(v->point());
    
    //Interior vertex position mask;
    Point3 P1((1-n*beta)*v->point());
    
    //Border vertex position mask for odd iteration
    Point3 P2(19.0/27.0*v->point());
    
    
    Polyhedron::Halfedge_around_vertex_const_circulator h = v->vertex_begin();
    
    do{
        //Interior vertex position mask;
        P1 = P1 + beta*h->opposite()->vertex()->point();
        
        //Border vertex position mask for odd iteration
        if(h->is_border_edge()){
            P2 = P2 + 4.0/27.0 * h->opposite()->vertex()->point();
            onBorder = true;
        }
        
        ++h;
    }while(h!=v->vertex_begin());
    
    if(onBorder){
        //In odd iteration, update border vertex
        if(isOdd){
            return P2;
        }else{
            //In even iteration, border vertex remain unchanged
            return P0;
        }
    }else{
        //Return interior vertex position
        return P1;
    }
}


void Sqrt_3::flipEdge(Polyhedron::Halfedge_handle h)
{
    assert(h!=Polyhedron::Halfedge_handle());
    assert(h->facet()!=Polyhedron::Facet_handle());
    assert(h->opposite()->facet()!=Polyhedron::Facet_handle());
    
    //Only flip edge that the two incident faces are in the same odd level
    if((h->facet()->level % 2 != 0) &&(h->facet()->level == h->opposite()->facet()->level))
    {
        mesh.flip_edge(h);
        ++(h->facet()->level);
        ++(h->opposite()->facet()->level);
        assert(h->facet()->level % 2 ==0);
        assert(h->opposite()->facet()->level %2 ==0);
    }
}

//Split face common version
void Sqrt_3::splitFace(Polyhedron::Facet_handle f)
{
    int curlevel = f->level;
    //Access the three previous halfedges
    Polyhedron::Halfedge_handle h = f->halfedge();
    Polyhedron::Halfedge_handle h1=h->next();
    Polyhedron::Halfedge_handle h2=h->prev();
    
    //Face center
    Point3 center=faceCenter(f);
    
    //Create center vertex
    Polyhedron::Halfedge_handle g = mesh.create_center_vertex(h);
    g->vertex()->point() = center;
    
    //If an halfedge incident to a border face, than increase the face level by two, this halfedge will not flip
    //If an halfedge's opposite halfedge is already in flip set, then this one will not insert to flip group, otherwise insert it in. The buffer will only be used in adaptive mode
    if(h->is_border_edge()){
        h->facet()->level = curlevel + 2;
    }else{
        h->facet()->level = curlevel + 1;
         if(Adaptive){
            if(EdgesTobeFlip.find(h->opposite())==EdgesTobeFlip.end()){
                EdgesTobeFlip.insert(h);
            }
         }
    }
    
    if(h1->is_border_edge()){
        h1->facet()->level = curlevel+2;
    }else{
        h1->facet()->level = curlevel +1 ;
         if(Adaptive){
            if(EdgesTobeFlip.find(h1->opposite())==EdgesTobeFlip.end()){
                EdgesTobeFlip.insert(h1);
            }
         }
    }
    
    if(h2->is_border_edge()){
        h2->facet()->level = curlevel+2;
    }else{
        h2->facet()->level=curlevel+1;
         if(Adaptive){
            if(EdgesTobeFlip.find(h2->opposite())==EdgesTobeFlip.end()){
                EdgesTobeFlip.insert(h2);
            }
         }
    }
    
}

//This function only apply to border faces in odd iteration
void Sqrt_3::splitFaceOdd(Polyhedron::Facet_handle f)
{
    assert(f!=Polyhedron::Facet_handle());
    
    
    int level = f->level;
    
    Polyhedron::Halfedge_handle h=f->halfedge();
    
    //Find the hafledge whose opposite is border halfedge
    while(!h->opposite()->is_border()){
        h=h->next();
    }
    
    /*
                    /\
             h3--->/  \<-----h0
             _____/____\_____
            p1   p2    p3    p4
     
     The figure above show the location of p1,p2,p3,p4 which are used to compute border edge vertex
     */
    Point3 p1=h->opposite()->next()->vertex()->point();
    Point3 p4=h->opposite()->prev()->opposite()->vertex()->point();
    Point3 p2=h->opposite()->vertex()->point();
    Point3 p3=h->vertex()->point();
    
    //h0 and h3 is shown in the figure above
    Polyhedron::Halfedge_handle h0=h->next();
    Polyhedron::Halfedge_handle h3=h->prev();
    
    //The two new edge vertex
    Point3 edgeV0= 1.0/27.0 * p1 + 16.0/27.0 * p2 + 10.0/27.0 * p3;
    Point3 edgeV1= 10.0/27.0 * p2 + 16.0/27.0 * p3 + 1.0/27.0 * p4;
    
    //Split halfedge h and insert one edge vertex and split this face, face split rule is defined by CGAL
    Polyhedron::Halfedge_handle h1=mesh.split_edge(h);
    h1->vertex()->point()=edgeV0;
    mesh.split_facet(h1,h0);
    
    //Split halfedge h again since it is still point to vertex p3, and then split the face again
    Polyhedron::Halfedge_handle h2=mesh.split_edge(h);
    h2->vertex()->point()=edgeV1;
    mesh.split_facet(h2,h0);
    
    
    assert(h2->opposite()->facet()==Polyhedron::Facet_handle());
    //Due to sqrt_3 face split rule on boundary faces, the face incident to h,h1 will trigger edge flip, and face incident to h2 will not.
    //So increase the face level incident to h,h1 by one, and increase face level incident to h2 by two
    h->facet()->level = level +1;
    h1->facet()->level = level + 1;
    h2->facet()->level = level + 2;
    
    if(Adaptive){ //In uniform mode, buffer will not be used
    //Insert h0 and h3 to edge flip set
        if(EdgesTobeFlip.find(h0->opposite()) == EdgesTobeFlip.end()){
            EdgesTobeFlip.insert(h0);
        }
        if(EdgesTobeFlip.find(h3->opposite()) == EdgesTobeFlip.end()){
            EdgesTobeFlip.insert(h3);
        }
    }

}

//Wrap the face split function
void Sqrt_3::split(Polyhedron::Facet_handle f)
{
    int i=0; //Denote how many triangle edges are on border
    bool onBorder=false;
    Polyhedron::Halfedge_around_facet_circulator h=f->facet_begin();
    do{
        if(h->is_border_edge()){
            ++i;
            onBorder = true;
        }
        ++h;
    }while(h!=f->facet_begin());
    
    if(onBorder)
    {
        if(isOdd){
            if(i>1){//If more than one edges on border, only split it by common version, this condition only happens in adaptive mode
                splitFace(f);
            }else{
                splitFaceOdd(f);
            }
        }else{//Split border face in even iteration
            splitFace(f);
        }
    }else{
        splitFace(f);
    }
}



void Sqrt_3::splitOddNeighbor(Polyhedron::Facet_handle f)
{
    Polyhedron::Halfedge_handle Flip;
    Polyhedron::Halfedge_around_facet_circulator h = f->facet_begin();
    
    do{
        if(!h->is_border_edge()){
            //Access the neighbour face fop of facet f
            Polyhedron::Facet_handle fop = h->opposite()->facet();
            //Only process neighbour faces whose subdivision level is less than this face f
            if( fop->level < f->level){
                if(fop->level == f->level-2){
                    
                    //Erase odd index faces if it is in the buffer
                    if(OddFaces.find(fop)!=OddFaces.end()){
                        OddFaces.erase(fop);
                    }
                    
                    //if the level difference is two, then apply this function to fop recursively
                    splitOddNeighbor(fop);
                    
                    //After the above function return, "fop" has even level, so split it, and flip edge
                    
                    fop = h->opposite()->facet();

                    split(fop);
                    
                    Flip = h;
                }else{
                    //The level difference is one, means fop has even level, split fop
                    
                    //It maybe already in faces split set, remove it
                    if(FacesTobeSplit.find(fop)!=FacesTobeSplit.end()){
                        FacesTobeSplit.erase(fop);
                    }
                    
                    
                    if(fop->level !=(f->level -1))
                        std::cerr<<f->level <<"   "<<fop->level<<"\n";
                    
                    //Store vertices that would be updated
                    storeVertex(fop);
                    
                    //Split neighbour
                    split(fop);
                    
                    assert(f->level == h->opposite()->face()->level);
                    assert(h->opposite()->facet()->level % 2 !=0);
                    
                    Flip = h;
                }
            }
        }
        ++h;
    }while(h!=f->facet_begin());
    
    
    if(Flip != Polyhedron::Halfedge_handle()){
        if(EdgesTobeFlip.find(Flip->opposite())!=EdgesTobeFlip.end()){
            EdgesTobeFlip.erase(Flip->opposite());
        }
        flipEdge(Flip);
    }
}

void Sqrt_3::adaptive()
{
    const std::size_t nv = mesh.size_of_vertices();
    const std::size_t nf = mesh.size_of_facets();
    const std::size_t ne = mesh.size_of_halfedges()/2;
    

    //Reserve memory
    mesh.reserve(nv+nf, ne*2+nf*8, 3*nf);
    
    
    //Alive vertices which has at least one incident face should be subdivided
    std::set<Polyhedron::Vertex_handle> AliveVertices;
    
    
    //Pick out the Alive vertices
    for(Polyhedron::Facet_iterator f=mesh.facets_begin();f!=mesh.facets_end();++f)
    {
        if(checkSubdivide(f, Criteria))
        {
            Polyhedron::Halfedge_around_facet_circulator h=f->facet_begin();
            
            do{
                Polyhedron::Vertex_handle v=h->vertex();
                
                AliveVertices.insert(v);
                ++h;
            }while(h!=f->facet_begin());
        }
    }
    
    //Subdivide all faces incident to Alive vertices
    for(auto VerIter = AliveVertices.begin();VerIter!=AliveVertices.end();++VerIter)
    {
        Polyhedron::Vertex_handle v=*VerIter;
        Polyhedron::Halfedge_around_vertex_circulator h = v->vertex_begin();
        do{
            if(!h->is_border()){
                Polyhedron::Facet_handle f=h->facet();
                storeVertex(f);
                if(f->level % 2!=0){
                    OddFaces.insert(f);
                }else{
                    FacesTobeSplit.insert(f);
                }
            }
            ++h;
        }while(h!=v->vertex_begin());
    }
    
    
    //For odd index faces, keep splitting its neighbour
    for(auto FaceIter = OddFaces.begin();FaceIter!=OddFaces.end();++FaceIter )
    {
        Polyhedron::Facet_handle f=*FaceIter;
        if(f->level % 2!=0){
            splitOddNeighbor(f);
        }
    }
    
    //For even index faces, first split its odd index neighbour
    for(auto FaceIter = FacesTobeSplit.begin();FaceIter!=FacesTobeSplit.end();++FaceIter)
    {
        Polyhedron::Facet_handle f=*FaceIter;
        if(f->level % 2==0)
        {
            Polyhedron::Halfedge_around_facet_circulator h=f->facet_begin();
            do{
                if(!h->is_border_edge())
                {
                    //split its odd index neighbour
                    Polyhedron::Facet_handle fop=h->opposite()->facet();
                    //Find odd index neighbour which level is less than f
                    if((fop->level % 2!=0) && (fop->level < f->level))
                    {
                        splitOddNeighbor(fop);
                    }
                }
            }while(h!=f->facet_begin());
           
            //Then split itself.
            split(f);
        }
    }
    
    
    //Flip edges
    for(auto edgeIter = EdgesTobeFlip.begin();edgeIter != EdgesTobeFlip.end();++edgeIter)
    {
        flipEdge(*edgeIter);
    }
    //Update vertices
    for(auto VerIter = vertexBuffer.begin();VerIter != vertexBuffer.end();++VerIter)
    {
        Polyhedron::Vertex_handle v=VerIter->first;
        v->point() = VerIter->second;
    }
    
    
    isOdd = !isOdd;
    
    std::cout<<mesh.size_of_facets()<<"\n";
    
    //Clear buffer for next iteration
    vertexBuffer.clear();
    EdgesTobeFlip.clear();
    FacesTobeSplit.clear();
    OddFaces.clear();
    
}



void Sqrt_3::uniform()
{
    const std::size_t nv = mesh.size_of_vertices();
    const std::size_t nf = mesh.size_of_facets();
    const std::size_t ne = mesh.size_of_halfedges()/2;
    
    
    //Reserve memory
    mesh.reserve(nv+nf, ne*2+nf*8, 3*nf);
    
    //Vertex position buffer
    std::vector<VPOS> vBuffer;
    
    //Compute vertex position
    for(Polyhedron::Vertex_iterator v=mesh.vertices_begin();v!=mesh.vertices_end();++v)
    {
        Point3 p = vertexPosition(v);
        vBuffer.emplace_back(v,p);
    }
    
    //Split faces
    Polyhedron::Facet_iterator f=mesh.facets_begin();
    for(int i=0;i<nf;++f,++i)
    {
        split(f);
    }
    
    //Flip old edges
    Polyhedron::Edge_iterator e=mesh.edges_begin();
    for(int i=0; i<ne; ++e,++i)
    {
        if(!e->is_border_edge()){
            flipEdge(e);
        }
    }
    
    //Update vertex position
    for(auto VerIter = vBuffer.begin();VerIter != vBuffer.end();++VerIter)
    {
        VerIter->first->point() = VerIter->second;
    }
    
    isOdd = !isOdd;
    
    std::cout<<mesh.size_of_facets()<<"\n";
    
}



void Sqrt_3::execution()
{
    if(Adaptive)
        adaptive();
    else
        uniform();
}






