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
    
    auto to_voxel_coor = [shape, start, stop](const Vector3& p) -> std::array<int, 3> {
        std::array<int, 3> out;
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

    vigra::MultiArray<3, uint16_t> vol(vigra::Shape3(shape[2], shape[1], shape[0]));
    
    cout << "*** tracing objects" << endl;
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
    
        for(int x=0; x<shape[0]; ++x) {
        for(int y=0; y<shape[1]; ++y) {
            cout << "  " << nRays << "/" << nRaysTotal << "                   \r" << std::flush;
            ++nRays;
           
            if(N == 1) {
                const float ox = x+0.5;
                const float oy = y+0.5;
                Vector3 rayStart = to_scene_coor(ox,oy,-10);
                Ray ray(rayStart, Vector3(0,0,1));
                IntersectionInfo I;
                bool hit = bvh.getIntersection(ray, &I, false);
                bool inside = false;
                std::array<int, 3> prevVoxelCoor = {x,y,-10};
                while(hit) {
                    std::array<int, 3> currVoxelCoor = to_voxel_coor(I.hit);
                    if(inside) {
                        for(int z=prevVoxelCoor[2]+1; z<=currVoxelCoor[2]; ++z) {
                            if(z >= 0 && z < shape[2]) {
                                vol(z,y,x) = currentLabel + 1;
                            }
                        }
                    }
                    prevVoxelCoor = currVoxelCoor;
                    rayStart = I.hit + 10*std::numeric_limits<float>::epsilon() * ray.d;
                    inside = !inside;
                    ray = Ray(rayStart, Vector3(0,0,1));
                    hit = bvh.getIntersection(ray, &I, false);
                }
                
            }
            else {
                vigra::MultiArray<2, uint8_t> columns(vigra::Shape2(N*N, shape[2]));
                size_t currColumn = 0;
                for(int w=0; w<N; ++w) {
                for(int h=0; h<N; ++h) {
                    const float ox = x+(w+1)/((float)(N+1));
                    const float oy = y+(h+1)/((float)(N+1));
                    Vector3 rayStart = to_scene_coor(ox,oy,-10);
                    Ray ray(rayStart, Vector3(0,0,1));
                    IntersectionInfo I;
                    bool hit = bvh.getIntersection(ray, &I, false);
                    bool inside = false;
                    std::array<int, 3> prevVoxelCoor = {x,y,-10};
                    while(hit) {
                        std::array<int, 3> currVoxelCoor = to_voxel_coor(I.hit);
                        if(inside) {
                            for(int z=prevVoxelCoor[2]+1; z<=currVoxelCoor[2]; ++z) {
                                if(z >= 0 && z < shape[2]) {
                                    columns(currColumn, z) = currentLabel + 1;
                                }
                            }
                        }
                        prevVoxelCoor = currVoxelCoor;
                        rayStart = I.hit + 10*std::numeric_limits< float >::epsilon() * ray.d;
                        inside = !inside;
                        ray = Ray(rayStart, Vector3(0,0,1));
                        hit = bvh.getIntersection(ray, &I, false);
                    }
                    
                ++currColumn;
                }
                }
                std::vector<uint8_t> vals(N*N);
                const size_t mid = N*N/2;
                for(int z=0; z<shape[2]; ++z) {
                    if(vol(z,y,x) != 0) { continue; } 
                    const auto r = columns.bind<1>(z);
                    std::copy(r.begin(), r.end(), vals.begin());
                    std::sort(vals.begin(), vals.end());
                    vol(z,y,x) = vals[mid];
                }
            }
        }
        
    }
    }
    cout << "  ... done tracing" << endl << endl;
    
    cout << "writing file ... " << std::flush;
    vigra::HDF5File file(outFile, vigra::HDF5File::New);
    file.write("labels", vol, 64, 1); 
    file.close();
    cout << " done" << endl;

    return 0;
}