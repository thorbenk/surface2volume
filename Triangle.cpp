#include "Triangle.h"

const float eps = 0.000001;
int triangle_intersection( const Vector3 V1, // Triangle vertices
                           const Vector3 V2,
                           const Vector3 V3,
                           const Vector3 O,  //Ray origin
                           const Vector3 D,  //Ray direction
                           float* out)  
{
    Vector3 e1, e2;  //Edge1, Edge2
    Vector3 P, Q, T;
    float det, inv_det, u, v;
    float t;
    
    //Find vectors for two edges sharing V1
    e1 = V2 - V1;
    e2 = V3 - V1;
    //Begin calculating determinant - also used to calculate u parameter
    P = D ^ e2;
    //if determinant is near zero, ray lies in plane of triangle
    det = e1 * P;
    //NOT CULLING
    if(det > -eps && det < eps) return 0;
    inv_det = 1.f / det;
    
    //calculate distance from V1 to ray origin
    T =  O - V1;
    
    //Calculate u parameter and test bound
    u = (T * P) * inv_det;
    //The intersection lies outside of the triangle
    if(u < 0.f || u > 1.f) return 0;
    
    //Prepare to test v parameter
    Q = T ^ e1;
    
    //Calculate V parameter and test bound
    v = (D * Q) * inv_det;
    //The intersection lies outside of the triangle
    if(v < 0.f || u + v  > 1.f) return 0;
    
    t = (e2 * Q) * inv_det;
    
    if(t > eps) { //ray intersection
        *out = t;
        return 1;
    }
 
    //No hit, no win
    return 0;
}