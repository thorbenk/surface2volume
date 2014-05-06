#include "OBJReader.h"

#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <iostream>

OBJReader::OBJReader(const std::string& filename)
    : filename_(filename), maxObjects_(-1) {}

void OBJReader::read(Scene& scene) const
{
    uint32_t vertexOffset = 1;
    
    std::ifstream f(filename_);
    std::string line;
    
    uint32_t currentLabel = 0;
   
    std::vector<std::string> toks;
    Mesh* m = 0;
    size_t lineNo = 0;
    while (std::getline(f, line)) {
        ++lineNo;
        if(line.size() == 0) continue;
        if(line[1] == ' ') {
            char c = line[0];
            if(c == '#') {
            }
            else if(c == 'o') {
                //if(scene.meshes.size() > 0) {
                //    break;
                //}
                line = line.substr(2,line.size());
                //if(objs.find(line) != objs.end()) {
                //    throw std::runtime_error("duplicate object");
                //}
               
                if(maxObjects_ >= 0 && currentLabel == maxObjects_) {
                    break;
                }
                
                std::cout << "  " << currentLabel+1 << " : " << line << std::endl;
                
                if(m) {
                    vertexOffset += m->vertices.size();
                }
                scene.meshes.push_back( Mesh() );
                m = &scene.meshes.back();
                m->setName(line);
                m->setLabel(++currentLabel);
            }
            else if(c == 'v') {
                if( m == 0 ) { throw std::runtime_error("m == 0"); }
                
                toks.clear();
                line = line.substr(2, line.size());
                boost::split(toks, line, boost::is_any_of(" "));
                if(toks.size() != 3) {
                    std::stringstream ss;
                    ss << "vertices: unexpected number of tokens" << std::endl;
                    ss << "line: " << line << std::endl;
                    throw std::runtime_error(ss.str());
                }
                Vector3 f;
                std::transform(toks.begin(), toks.end(), &f[0], [](const std::string& s) { return std::stof(s); });
                m->vertices.push_back(f);
            }
            else if(c == 'f') {
                if( m == 0 ) { throw std::runtime_error("m == 0"); }
                toks.clear();
                line = line.substr(2, line.size());
                boost::split(toks, line, boost::is_any_of(" "));
                
                if(toks.size() == 4) {
                    std::array<uint32_t, 4> f;
                    std::transform(toks.begin(), toks.end(), f.begin(), [vertexOffset](const std::string& s) { return std::stoi(s)-vertexOffset; });
                    for(int i=0; i<4; ++i) {
                        if(f[i] >= m->vertices.size()) {
                            std::stringstream ss;
                            ss << "object '" << m->name() << "' has " << m->vertices.size() << " vertices, but read face = ";
                            std::copy(f.begin(), f.end(), std::ostream_iterator<uint32_t>(ss, " "));
                            ss << std::endl;
                            throw std::runtime_error(ss.str());
                        }
                    }
                    m->faces.push_back({f[0], f[1], f[3]});
                    m->faces.push_back({f[3], f[1], f[2]});
                }
                else if(toks.size() == 3) {
                    std::array<uint32_t, 3> f;
                    std::transform(toks.begin(), toks.end(), f.begin(), [vertexOffset](const std::string& s) { return std::stoi(s)-vertexOffset; });
                    for(int i=0; i<3; ++i) {
                        if(f[i] >= m->vertices.size()) {
                            std::stringstream ss;
                            ss << "object '" << m->name() << "' has " << m->vertices.size() << " vertices, but read face = ";
                            std::copy(f.begin(), f.end(), std::ostream_iterator<uint32_t>(ss, " "));
                            ss << std::endl;
                            throw std::runtime_error(ss.str());
                        }
                    }
                    m->faces.push_back(f);
                }
                else {
                    std::stringstream ss;
                    ss << "WARNING: line " << lineNo << ": faces: unexpected number of tokens: " << toks.size();
                    std::cerr << ss.str() << std::endl;
                }
            }
            else if(c == 's') {
            }
            else if(c == 'l') {
            }
            else {
                std::cout << line << std::endl;
            }
        }
        else if(line.substr(0,6) == std::string("usemtl")) {
        }
        else if(line.substr(0,6) == std::string("mtllib")) {
        }
        else {
            std::stringstream ss;
            ss << "unexpected line: " << line << std::endl;
            throw std::runtime_error(ss.str());
        }
    }
}
