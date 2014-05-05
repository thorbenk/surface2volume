#include "Mesh.h"

#include "Triangle.h"

BBox Mesh::bbox() const
{
    if(vertices.size() == 0) {
        return BBox();
    }
    else if(vertices.size() == 1) {
        return BBox(vertices[0], vertices[0]);
    }
    else {
        auto it = vertices.begin();
        BBox b(*it, *(++it));
        while(it != vertices.end()) {
            b.expandToInclude(*it);
            ++it;
        }
        return b;
    }
}

//Mesh::~Mesh()
//{
//    for(Object* o : objects_) {
//        delete o;
//    }
//}

bool Mesh::buildBVH(float edgeLengthThreshold)
{
    size_t tris = appendTriangles(objects_, edgeLengthThreshold);
    if(tris > 0) { 
        std::cout << "building BVH with " << tris << " / " << faces.size() << " tris after filtering" << std::endl;
        bvh_ = std::unique_ptr<BVH>(new BVH(&objects_));
    }
}

size_t Mesh::appendTriangles(std::vector<Object*>& objects, float edgeLengthTreshold=-1.0) const
{
    size_t i = 0;
    for(const auto& f : faces) {
        Vector3 v1(vertices[f[0]][0], vertices[f[0]][1], vertices[f[0]][2]);
        Vector3 v2(vertices[f[1]][0], vertices[f[1]][1], vertices[f[1]][2]);
        Vector3 v3(vertices[f[2]][0], vertices[f[2]][1], vertices[f[2]][2]);
        
        const float a = length(v2-v1);
        const float b = length(v2-v3);
        const float c = length(v1-v3);
        const float l = std::max(a, std::max(b,c));
        
        if(edgeLengthTreshold > 0 && l > edgeLengthTreshold) {
            continue;
        }
        
        objects.push_back(new Triangle(v1, v2, v3, label_) );
        ++i;
    }
    return i;
}

bool Mesh::contains(const Vector3& p, const BVH& bvh) const
{
    bool hit = false;
    int nHits = 0;
    Vector3 rayStart = p;
    do {
        Ray ray(rayStart, Vector3(0,0,1));
        IntersectionInfo I;
        hit = bvh.getIntersection(ray, &I, false);
        ++nHits;
        rayStart = I.hit + 0.001*ray.d; 
    }
    while(hit);
    return (nHits % 2) == 1;
}

/*
bool contains(Vec3F pt) {
    Vec3F dir(1,0,0);
    int intersect = 0;
    for(const auto& f: faces) {
        const auto v0 = vertices[f[0]];
        const auto v1 = vertices[f[1]];
        const auto v2 = vertices[f[2]];
        
        intersect += triangle_intersection(v0, v1, v2, pt, dir);
    }
    return (intersect % 2) == 1;
}
*/