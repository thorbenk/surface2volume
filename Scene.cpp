#include "Scene.h"

BBox Scene::bbox() const
{
    BBox b = BBox();
    for(const Mesh& m : meshes) {
        b.expandToInclude(m.bbox());
    }
    return b;
}

void Scene::appendTriangles(std::vector<Object*>& objects) const
{
    for(const auto& m : meshes) {
        m.appendTriangles(objects, -1.0);
    }
}