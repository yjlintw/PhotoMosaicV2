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
        int OUT_SUB_X;
        int OUT_SUB_Y;

        // Constructor
        PhotoMosaic();
        // Functions
        void loadImages(std::string dirPath);
        void loadImagesFromTxt(std::string txtPath);
        void loadBaseImage(std::string filePath);
        cv::Mat tileToImages();
        
        void debugDraw();
        void loadImageMap(std::string filePath);

    private:
        ofImage baseImage;
        ofImage tileImage;
        ofImage resultImage;
        cv::Mat baseMat;
        cv::Mat tileMat;
        cv::Mat resultMat;
        cv::Mat matchList;
        std::vector<std::vector<cv::Point2d> > imageMap;
        std::vector<ofImage> outputImages;
        
        void computeTileAvgRGB(const cv::Mat& mat, cv::Mat& tileMat, cv::Mat& debugMat);
        int findNearestImage(cv::Scalar color);
        void saveImagesInfoToDisk();
        void updateImageMap();
        void saveImageMap();
        
        
    };
}
