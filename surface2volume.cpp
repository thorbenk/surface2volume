#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <map>
#include <sstream>
#include <cmath>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include <fastbvh/BVH.h>

#include <vigra/multi_array.hxx>
#include <vigra/hdf5impex.hxx>

#include "Triangle.h"
#include "Mesh.h"
#include "OBJReader.h"

std::ostream& operator<<(std::ostream& o, const Vector3& v) {
    o << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
    return o;
}

namespace po = boost::program_options;

class FloatBBox {
    public:
    FloatBBox() {}
    FloatBBox(const Vector3& s, const Vector3& t) : start(s), stop(t) {}
    Vector3 start;
    Vector3 stop;
};

std::istream& operator >>(std::istream& source, FloatBBox& target) {
    std::string in;
    source >> in;
    const std::string f = "([-+]?[0-9]*\\.?[0-9]+)";
    static const boost::regex e("\\("+f+","+f+","+f+"\\)\\("+f+","+f+","+f+"\\)");
    boost::match_results<std::string::const_iterator> matches; 
    
    float num[6];
    try {
        boost::regex_match(in, matches, e);
        for(size_t i=1; i<matches.size(); ++i) {
            num[i-1] = boost::lexical_cast<float>(std::string(matches[i].first, matches[i].second));
        }
        target = FloatBBox(Vector3(num[0], num[1], num[2]), Vector3(num[3], num[4], num[5]));
    }
    catch(const std::exception& e) {
        std::stringstream err;
        err << "Could not parse '" << in << "' as a float bounding box" << std::endl;
        throw std::runtime_error(err.str());
    }
    return source;
}

namespace vigra {
std::istream& operator >>(std::istream& source, vigra::Shape3& target) {
    std::string in;
    source >> in;
    const std::string i = "([0-9]+)";
    static const boost::regex e("\\("+i+","+i+","+i+"\\)");
    boost::match_results<std::string::const_iterator> matches; 
    
    int num[3];
    try {
        boost::regex_match(in, matches, e);
        for(size_t i=1; i<matches.size(); ++i) {
            num[i-1] = boost::lexical_cast<int>(std::string(matches[i].first, matches[i].second));
        }
        target = vigra::Shape3(num[0], num[1], num[2]);
    }
    catch(const std::exception& e) {
        std::stringstream err;
        err << "Could not parse '" << in << "' as an integer bounding box" << std::endl;
        throw std::runtime_error(err.str());
    }
    return source;
}
} /* namespace vigra */

int main(int argc, char **argv) {
    using std::cout;
    using std::endl;
    
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("file", po::value<std::string>(),
         "input .obj file")
        ("scene", po::value<FloatBBox>(),
         "bounding box of scene. Example: '(0,0,-5)(5,5,0)'")
        ("shape", po::value<vigra::Shape3>(),
         "output shape.          Example: '(999,999,898)'"  )
        ("max", po::value<int>(),
         "maximal number of objects read in")
        ("out", po::value<std::string>(),
         "output file.           Example: 'volume.h5'"      )
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    
    
    std::string objFile;
    std::string outFile; 
    FloatBBox sceneBBox;
    vigra::Shape3 shape;
    int maxObjects = -1;

    if (vm.count("help")) {
        cout << desc << endl;
        return 1;
    }
    if (vm.count("file")) {
        objFile = vm["file"].as<std::string>();
    } 
    else {
        cout << "No .obj file specified." << endl;
        cout << desc << endl;
        return 1;
    }
    if (vm.count("scene")) {
        sceneBBox = vm["scene"].as<FloatBBox>();
    }
    else {
        cout << "No scene bounding box specified" << endl;
        cout << desc << endl;
        return 1;
    }
    if (vm.count("shape")) {
        shape = vm["shape"].as<vigra::Shape3>();
    }
    else {
        cout << "No output shape specified" << endl;
        cout << desc << endl;
        return 1;
    }
    if( vm.count("out")) {
        outFile = vm["out"].as<std::string>();
    }
    else {
        cout << "No output file specified" << endl;
        cout << desc << endl;
        return 1;
    }
    if (vm.count("max")) {
        maxObjects = vm["max"].as<int>();
    }
    
    Vector3 start = sceneBBox.start;
    Vector3 stop  = sceneBBox.stop;
    cout << "input file:         " << objFile << endl;
    cout << "scene bounding box: " << start << ", " << stop << endl;
    cout << "output shape:       " << shape[0] << ", " << shape[1] << ", " << shape[2] << endl;
    if(maxObjects > 0) {
    cout << "reading in only     " << maxObjects << " objects" << endl;
    }
    cout << endl;
   
    //swap, vigra order has z,y,x
    shape = vigra::Shape3(shape[2], shape[1], shape[0]);
    
    Scene scn;
    OBJReader r(objFile);
    if(maxObjects > 0) {
        r.setMaxObjects(maxObjects);
    }
    
    // Allows to set a maximum allowed edge length for triangles considered.
    // Disabled for now.
    const float edgeLengthThreshold = -1.0; 
    
    cout << "*** reading all objects" << endl;
    r.read(scn);
    cout << endl;
   
    cout << "*** building BVHs" << endl;
    for(auto& m : scn.meshes) {
        m.buildBVH(edgeLengthThreshold);
    }
    cout << endl;
    
    auto to_voxel_coor = [shape, start, stop](const Vector3& p) -> std::array<long int, 3> {
        std::array<long int, 3> out;
        for(int i=0; i<3; ++i) {
            out[i] = std::round( (p[i]-start[i])/((float)(stop[i]-start[i]))*shape[i] );
        }
        return out;
    };
    auto to_scene_coor = [shape, start, stop](float x, float y, float z) -> Vector3 {
        Vector3 out(x,y,z);
        for(int i=0; i<3; ++i) {
            out[i] = out[i]/((float)shape[i]) * (stop[i]-start[i]) + start[i];
        }
        return out;
    };

    //vigra: axis order is z,y,x
    typedef vigra::MultiArray<3, uint16_t> V;
    V vol[3] = {V(shape), V(shape), V(shape)};
   
    for(int rayAxis = 0; rayAxis<3; ++rayAxis) {
        int otherAxes[2];
        {
            int j = 0;
            for(int i=0; i<3; ++i) { 
                if(i!=rayAxis) {
                    otherAxes[j] = i;
                    ++j;
                }
            }
        }
        
    cout << "*** tracing objects (ray axis = " << rayAxis << ")" << endl;
    for(uint32_t currentLabel = 0; currentLabel < scn.meshes.size(); ++currentLabel) {
        const Mesh& m = scn.meshes[currentLabel];
        if(!m.bvh()) {
            continue;
        }
        
        const BVH& bvh = *m.bvh();
        cout << "  tracing " << currentLabel << "/" << scn.meshes.size()
                  << " '" << scn.meshes[currentLabel].name() << "'"
                  << endl;
    
        size_t nRaysTotal = shape[0]*shape[1];
        size_t nRays = 0;
        const int N = 1;
    
        
        vigra::TinyVector<vigra::MultiArrayIndex, 3> coord;
        for(coord[otherAxes[0]]=0; coord[otherAxes[0]] < shape[otherAxes[0]]; ++coord[otherAxes[0]]) {
        for(coord[otherAxes[1]]=0; coord[otherAxes[1]] < shape[otherAxes[1]]; ++coord[otherAxes[1]]) {
            cout << "  " << nRays << "/" << nRaysTotal << "                   \r" << std::flush;
            ++nRays;
           
            float c[3] = {coord[0]+0.5f, coord[1]+0.5f, coord[2]+0.5f};
            c[rayAxis] = -10.0f;
            
            const Vector3 normal(1 ? rayAxis==0 : 0, 1 ? rayAxis==1 : 0, 1 ? rayAxis==2 : 0);
            
            Vector3 rayStart = to_scene_coor(c[0], c[1], c[2]);
            Ray ray(rayStart, normal);
            IntersectionInfo I;
            bool hit = bvh.getIntersection(ray, &I, false);
            bool inside = false;
            
            std::array<long int, 3> prevVoxelCoor = {coord[0], coord[1], coord[2]};
            prevVoxelCoor[rayAxis] = -10.0f;
            
            while(hit) {
                std::array<long int, 3> currVoxelCoor = to_voxel_coor(I.hit);
                if(inside) {
                    vigra::MultiArrayIndex& t = coord[rayAxis];
                    for(t=prevVoxelCoor[rayAxis]+1;
                        t<=currVoxelCoor[rayAxis]; ++t)
                    {
                        if(t >= 0 && t < shape[2-rayAxis]) {
                            vol[rayAxis](coord[2], coord[1], coord[0]) = currentLabel + 1;
                        }
                    }
                }
                prevVoxelCoor = currVoxelCoor;
                rayStart = I.hit + 10*std::numeric_limits<float>::epsilon() * ray.d;
                inside = !inside;
                ray = Ray(rayStart, normal);
                hit = bvh.getIntersection(ray, &I, false);
            }
        }
        }
    }
    cout << "  ... done tracing" << endl << endl;
    
    } /* ray axis iteration */
    
    for(int i=0; i<vol[0].size(); ++i) {
        const uint16_t a = vol[0][i];
        const uint16_t b = vol[1][i];
        const uint16_t c = vol[2][i];
        if     ( a == b ) { vol[0][i] = a; }
        else if( a == c ) { vol[0][i] = a; }
        else if( b == c ) { vol[0][i] = b; }
        else              { vol[0][i] = 0; }
    }
    
    cout << "writing file ... " << std::flush;
    vigra::HDF5File file(outFile, vigra::HDF5File::New);
    file.write("labels", vol[0], 64, 1); 
    file.close();
    cout << " done" << endl;

    return 0;
}