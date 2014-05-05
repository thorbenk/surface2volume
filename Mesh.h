#ifndef MESH_H
#define MESH_H

#include <array>
#include <vector>
#include <memory>

#include "fastbvh/BBox.h"
#include "fastbvh/BVH.h"
#include "fastbvh/Object.h"
#include "fastbvh/Vector3.h"

class Mesh {
    public:
    typedef std::array<uint32_t, 3> Tri;
    
    //~Mesh();
    
    std::vector<Vector3> vertices;
    std::vector<Tri> faces;
    
    BBox bbox() const;
    
    void setName(std::string name) { name_ = name; }
    std::string name() const { return name_; }
    
    void setLabel(uint32_t label) { label_ = label; }
    uint32_t label() const { return label_; }
   
    size_t appendTriangles(std::vector<Object*>& objects, float edgeLengthThreshold) const;
    
    bool contains(const Vector3& p, const BVH& bvh) const;
    
    bool buildBVH(float edgeLengthThreshold);
    
    const BVH* bvh() const { return bvh_.get(); }
    
    private:
    std::unique_ptr<BVH> bvh_;
    std::vector<Object*> objects_;
    
    std::string name_;
    uint32_t label_;
};

#endif /* MESH_H */
