#ifndef OBJ_READER
#define OBJ_READER

#include <string>

#include "Scene.h"

class OBJReader {
    public:
    OBJReader(const std::string& filename);
    
    void setMaxObjects(int maxObjects) { maxObjects_ = maxObjects; }
    
    void read(Scene& scene) const;
    
    private:
    std::string filename_;
    int maxObjects_;
};

#endif /* OBJ_READER */