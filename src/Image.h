#pragma once
#include "ofxCv.h"


namespace YJL {
    class Image {
    public:
        cv::Mat mat;
        cv::Scalar meanColor;
        std::string filepath;
        int usedCount;
    };
}
