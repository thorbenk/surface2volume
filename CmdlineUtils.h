#ifndef CMDLINEUTILS_H
#define CMDLINEUTILS_H

#include <vigra/multi_shape.hxx>

#include "fastbvh/Vector3.h"

class FloatBBox {
    public:
    FloatBBox() {}
    FloatBBox(const Vector3& s, const Vector3& t) : start(s), stop(t) {}
    Vector3 start;
    Vector3 stop;
};

std::istream& operator >>(std::istream& source, FloatBBox& target);

namespace vigra {
    std::istream& operator >>(std::istream& source, vigra::Shape3& target);
}

#endif /* CMDLINEUTILS_H */