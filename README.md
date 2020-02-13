## Description
This program implement adaptive Loop and Kobbelt sqrt3 subdivisions. Comparing with uniform subdivision, adaptive subdivision preserve the quality but consumes less memory. 

## Comparison with uniform subdivisiong
### Adaptive Subdivision vs Uniform Subdivision
<img src="https://github.com/shengyangwei/adaptive-subdivision/blob/Adaptive_Subdivision/images/adaptive-s.gif" width="425" alt = "Adpative subdivision" title="Adaptive subdivision"/>   <img src="https://github.com/shengyangwei/adaptive-subdivision/blob/Adaptive_Subdivision/images/uniform-s.gif" width="425" title="Uniform subdivision" alt="Uniform subdivision" />

### With Edge Display
<img src="https://github.com/shengyangwei/adaptive-subdivision/blob/Adaptive_Subdivision/images/adaptive-edge-s.gif" width="425" alt="adaptive with edge display" title="adaptive with edge display"/> <img src="https://github.com/shengyangwei/adaptive-subdivision/blob/Adaptive_Subdivision/images/uniform-edge-s.gif" width="425" alt="uniform with edge display" title="uniform with edge display"/>

## Dependencies
- OS: Linux, Mac OS
- Compiler: g++, clang++ with c++ 14 supporting
- Libraries: CGAL, GLUT, [Eigen3](http://eigen.tuxfamily.org/), OpenGL 3.0(Linux), OpenGL 2.0 (Mac OS)
- Input file format: Object File Format(.off). The input mesh should be a triangle mesh.

## Building Software

```shell
cd ${PROJECT_SOURCE_DIR}
mkdir build && cd build
cmake ..
make 
```

### Run the application
e.g. $./demo -a -l 10 -t 20.0 -m loop <${model}.obj


## Command Line Options
- \-t ${angle}

This option specifies the dihedral tolerance while performing subdivision. If a dihedral of a triangle is larger than ${angle}, then this triangle and its neighbor will be subdivided. E.g. -t 15.2

- \-l ${level}

This option specifies the limitation of iterations of subdivision. It accept integer argument, e.g. -l 10.

- \-f ${faces}

This option specify the limitation of the number of faces after subdivision. If the current number of faces exceed this limitation, further iterations of subdivision will be terminated. This option accept integer argument, e.g. -f 100000

- \-m ${method}

This option specify the subdivision scheme. It accept two arguments, e.g. -m loop, for adaptive Loop subdivision, or -m sqrt3, for adaptive Kobbelt subdivision.

- \-a 

This option will enable adaptive mode. 

### Warning
Without specifying the subdivision level limitation or number of faces limitation, the program will keep subdividing mesh until all dihedral satisfy the given tolerance. Probably, it will consume all the available memory. 

## Keyboard and Mouse Functionality
- Mouse
  * Left drag rotates the model
  * Middle drag scales the model
  * Right drag translates thte model
- Keyboard
  * q Quit the program
  * S Increse the level of subdivision by one
  * s Decrease the level of subdivision by one
  * e Toggle display of mesh edges

## Brief Description of the Implementation

Adaptive Loop Subdivision: In each iteration, for every triangle facet, check the dihedrals between its three neighbor triangles. If a dihedral is larger than the given tolerance, this triangle facet should be subdivided. If all dihedrals are under the specified tolerance, adaptive subdivision is terminated. Otherwise, program will keep subdividing the input mesh until the number of mesh facets or number of subdivision level reaches the limitation.

Adaptive Sqrt(3) Subdivision: In each iteration, for every triangle facet, check the dihedral between its three neighbor triangles, if any dihedral is larger than the dihedral tolerance, then add its three vertices to a set called alive vertices set. Then traverse the vertices in the set, and subdivide the incident triangles of every vertex. If all dihedrals are under the specified tolerance, adaptive subdivision is terminated. Otherwise, program will keep subdividing input mesh until the number of mesh facets or number of subdivision level reaches the limitation. 

To avoid unnecessary memory cost, if the increased amount of faces is less than one percent of current mesh face number after one adaptive subdivision, the tolerance is said to be satisfied. Thus, further iterations of adaptive subdivision are terminated. Actually, in most cases, the number of mesh faces will converge to a certain number. Premature termination will not affect the visual effect.

### Reference
- A. Amresh, G. Farin, A. Razdan, “Adaptive Subdivision Schemes for Triangular Meshes”, Hierarchical and Geometrical Methods in Scientific Visualization, pp. 319-327, 2003

- L. Kobbelt, "√3-subdivision", Proceedings of the 27th annual conference on Computer graphics and interactive techniques - SIGGRAPH '00. pp. 103–112, 2000


