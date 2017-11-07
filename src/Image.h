#pragma once
#include "ofxCv.h"
#include <string>

namespace YJL {
    class Image {
    public:
        cv::Mat mat;
        cv::Scalar meanColor;
        std::string filepath;
        int usedCount;
        
        std::string print() {
            std::string result = filepath + "," + to_string(meanColor[0]) + "," + to_string(meanColor[1]) + "," + to_string(meanColor[2]);
            return result;
        }
    };
}
