#pragma once
#include <vector>
#include "ofxCv.h"
#include "Image.h"


namespace YJL {
    class PhotoMosaic {
    public:
        // Variables
        std::vector<std::string> imagePathList;
        std::vector<YJL::Image> images;
        int TILE_WIDTH;
        int TILE_HEIGHT;
        int OUT_RES_W;
        int OUT_RES_H;

        // Constructor
        PhotoMosaic();
        // Functions
        void loadImages(std::string dirPath);
        void loadImagesFromTxt(std::string txtPath);
        void loadBaseImage(std::string filePath);
        cv::Mat tileToImages();
        
        void debugDraw();
        

    private:
        ofImage baseImage;
        ofImage tileImage;
        ofImage resultImage;
        cv::Mat baseMat;
        cv::Mat tileMat;
        cv::Mat resultMat;
        cv::Mat matchList;
        
        void computeTileAvgRGB(const cv::Mat& mat, cv::Mat& tileMat, cv::Mat& debugMat);
        int findNearestImage(cv::Scalar color);
        void saveImagesInfoToDisk();
        void getNearestImageSet(std::vector<std::vector<cv::Point2d> >& imageMap);
        
    };
}
