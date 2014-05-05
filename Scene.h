#ifndef SCENE_H
#define SCENE_H

#include "Mesh.h"
#include "fastbvh/BBox.h"

class Scene {
    public:
    std::vector<Mesh> meshes;
    
    BBox bbox() const;
    
    void appendTriangles(std::vector<Object*>& objects) const;
};

#endif /* SCENE_H */