//SHENGYANG  WEI        

#ifndef MATHEMATICS
#define MATHEMATICS

#include <CGAL/Cartesian.h>

typedef CGAL::Cartesian<double> Kernel;
typedef Kernel::Point_3 Point3;
typedef Kernel::Vector_3 Vector3;

//Overload operator + for Point3
inline Point3 operator+(const Point3& p,const Point3& q)
{
	return Point3(p.x()+q.x(),p.y()+q.y(),p.z()+q.z());
}
//Overload operator * for Point3
inline Point3 operator*(const Point3& p, double s)
{
	return Point3(p.x()*s,p.y()*s,p.z()*s);
}
//Overload operator * for Point3
inline Point3 operator*(double s,const Point3& p)
{
	return p*s;
}
//Overload operator / for Point3
inline Point3 operator/(const Point3& p, double s)
{
	return Point3(p.x()/s,p.y()/s,p.z()/s);
}

//Normalize vector
inline Vector3 normalize(const Vector3& v)
{
	double norm=std::sqrt(v.squared_length());
	if(norm!=0)
    {
        return v/norm;
    }
    else{
    
        return Vector3(0.0,0.0,0.0);
    }
}

//Compute the unit normal of the plane composed of three point
inline Vector3 unitNormal(const Point3& p,const Point3& q,const Point3& r)
{
	Vector3 v1(p,q);
	Vector3 v2(p,r);
	return normalize(CGAL::cross_product(v1,v2));
}

#endif
