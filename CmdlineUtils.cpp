#include "CmdlineUtils.h"

#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

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