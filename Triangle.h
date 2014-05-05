#ifndef Triangle_h_
#define Triangle_h_

#include <cmath>
#include <iostream>
#include <stdexcept>

#include "fastbvh/Object.h"

int triangle_intersection( const Vector3 V1, // Triangle vertices
                           const Vector3 V2,
                           const Vector3 V3,
                           const Vector3 O,  //Ray origin
                           const Vector3 D,  //Ray direction
                           float* out);

struct Triangle : public Object {
    Vector3 v1, v2, v3;
    uint32_t l;

    Triangle(const Vector3& vec1, const Vector3& vec2, const Vector3& vec3, uint32_t label)
        : v1(vec1), v2(vec2), v3(vec3), l(label) {}

    bool getIntersection(const Ray& ray, IntersectionInfo* I) const {
        float out;
        bool hit = triangle_intersection(v1, v2, v3, ray.o, ray.d, &out);
        if(!hit) { return false; }

        I->object = this;
        I->t = out;
        return true;
    }
 
    Vector3 getNormal(const IntersectionInfo& I) const {
        throw std::runtime_error("Not implemented");
        return normalize((v2-v3) ^ (v2-v1));
    }

    BBox getBBox() const { 
        return BBox(
            min(v1, min(v2,v3)),
            max(v1, max(v2,v3))
        );
    }

    Vector3 getCentroid() const {
        return 1.0f/3.0f * (v1+v2+v3);
    }

};

#endif
