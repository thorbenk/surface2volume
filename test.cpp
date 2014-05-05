#include <fastbvh/BVH.h>

#include "Triangle.h"

#include <iostream>

std::ostream& operator<<(std::ostream& o, const Vector3& v) {
    o << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
    return o;
}

int main(int argc, char **argv) {
    std::vector<Object*> objects;
    objects.push_back(new Triangle( Vector3(0,1,5), Vector3(1,0,5), Vector3(1,1,5), 1) );
    
    BVH bvh(&objects);
   
    Vector3 p(0.5, 0.5, 0);
    Ray ray(p, Vector3(0,0,1));
    IntersectionInfo I;
    bool hit = bvh.getIntersection(ray, &I, false);
   
    std::cout << I.hit << std::endl;     
    
    return 0;
}