#include "PhotoMosaic.h"
#include <functional>
#include <queue>
using namespace YJL;

PhotoMosaic::PhotoMosaic() {
    TILE_WIDTH = 8;
    TILE_HEIGHT = 6;
    OUT_RES_W = 1776;
    OUT_RES_H = 1332;
}

void PhotoMosaic::debugDraw() {
    if (resultImage.isAllocated()) {
        resultImage.draw(0, 0, resultImage.getWidth() / 2, resultImage.getHeight() / 2);
    }
    
    if (tileImage.isAllocated()) {
        tileImage.draw(resultImage.getWidth() / 2, 0, tileImage.getWidth(), tileImage.getHeight());
    }
}

void PhotoMosaic::loadImages(std::string dirPath) {
    images.clear();

    imagePathList.clear();
    ofDirectory dir(dirPath);
    dir.allowExt("png");
    dir.allowExt("jpg");
    dir.allowExt("jpeg");
    dir.listDir();


    //
    for (int i = 0; i < dir.size(); i++) {
        std::string imagePath = dir.getPath(i);
        imagePathList.push_back(imagePath);
        ofImage img;
        Image newImage;
        img.load(imagePath);
        newImage.mat = ofxCv::toCv(img).clone();
        newImage.meanColor = cv::mean(newImage.mat);
        newImage.filepath = imagePath;
        newImage.usedCount = 0;
        images.push_back(newImage);
    }
}

void PhotoMosaic::loadBaseImage(std::string filePath) {
    // load base image
    baseImage.load(ofToDataPath(filePath));
    
    
//    ofxCv::imitate(resultImage, baseImage);
    
    baseMat = ofxCv::toCv(baseImage);
    std::cout << baseMat.cols << ":" << baseMat.rows << std::endl;
//    resultMat = ofxCv::toCv(resultImage);
    baseMat.copyTo(resultMat);

    
//    // Draw Debug Line
//    for (int i = 0; i < baseMat.cols; i+= TILE_WIDTH) {
//        cv::line(resultMat, cv::Point(i, 0), cv::Point(i, resultMat.rows), cv::Scalar(100, 100, 100));
//    }
//
//    for (int i = 0; i < baseMat.rows; i+= TILE_HEIGHT) {
//        cv::line(resultMat, cv::Point(0, i), cv::Point(resultMat.cols, i), cv::Scalar(100, 100, 100));
//    }
//
    computeTileAvgRGB(baseMat, tileMat, resultMat);
    
    cv::Mat matchList;
    tileToImages(matchList, tileMat);
    
    
    tileImage.allocate(tileMat.cols, tileMat.rows, baseImage.getImageType());
    ofxCv::toOf(tileMat, tileImage);
    baseImage.update();
//    resultImage.update();
    tileImage.update();

    // cv::imshow("result", resultMat);
}


cv::Mat PhotoMosaic::tileToImages(cv::Mat& matchList, const cv::Mat& tileMat) {
    matchList = cv::Mat(tileMat.rows, tileMat.cols, CV_8UC1);
    int t_Width = OUT_RES_W / tileMat.cols;
    int t_Height = OUT_RES_H / tileMat.rows;
    
    cv::Mat tmpMat(OUT_RES_H, OUT_RES_W, CV_8UC3);
    
    for (int j = 0; j < tileMat.rows; j++) {
        for (int i = 0; i < tileMat.cols; i++) {
            cv::Scalar color;
            color[0] = tileMat.at<cv::Vec3b>(j, i)[0];
            color[1] = tileMat.at<cv::Vec3b>(j, i)[1];
            color[2] = tileMat.at<cv::Vec3b>(j, i)[2];
            
            int index = findNearestImage(color);
            //matchList.at<uchar>(j, i) = index;
            cv::Mat smallMat(t_Height, t_Width, images[index].mat.type());
            cv::resize(images[index].mat, smallMat, cv::Size(t_Width, t_Height), CV_INTER_NN);
            cv::Mat roiMat = tmpMat(cv::Rect(i * t_Width, j * t_Height, t_Width, t_Height));
            smallMat.copyTo(roiMat);
//            cv::imshow("test", smallMat);
        }
    }

//    cv::imshow("test", tmpMat);
    resultImage.allocate(tmpMat.cols, tmpMat.rows, resultImage.getImageType());
    ofxCv::toOf(tmpMat, resultImage);
    resultImage.update();
//    resultImage.update();
    return tmpMat;
}

int PhotoMosaic::findNearestImage(cv::Scalar color) {
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int> >, std::greater<std::pair<double, int> > > que;
    for (int i = 0; i < images.size(); i++) {
        cv::Scalar comparedColor =images[i].meanColor;
        cv::Vec4d d = comparedColor - color;
        double distance = cv::norm(d);
        
        que.push(std::pair<double, int>(distance, i));
    }
    
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int> >, std::greater<std::pair<int, int> > > que2;
    
    for (int i = 0; i < 5; i++) {
        Image img = images[que.top().second];
        que.pop();
        que2.push(std::pair<int, int>(img.usedCount, que.top().second));
    }
    
    images[que2.top().second].usedCount++;
    return que2.top().second;
    
}

void PhotoMosaic::computeTileAvgRGB(const cv::Mat& mat, cv::Mat& tileMat, cv::Mat& debugMat) {
    int colNumber = mat.cols / this->TILE_WIDTH;
    int rowNumber = mat.rows / this->TILE_HEIGHT;
    
    tileMat = cv::Mat(rowNumber, colNumber, CV_8UC3);
    
    
    for (int j = 0; j < rowNumber; j++) {
        for (int i = 0; i < colNumber; i++) {
            cv::Rect roiRect(i * TILE_WIDTH, j * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
            cv::Mat roiMat = mat(roiRect);
            
            cv::Scalar color = cv::mean(roiMat);
            
            tileMat.at<cv::Vec3b>(j, i)[0] = color[0];
            tileMat.at<cv::Vec3b>(j, i)[1] = color[1];
            tileMat.at<cv::Vec3b>(j, i)[2] = color[2];
        }
    }
    
    cv::resize(tileMat, debugMat, cv::Size(mat.cols, mat.rows), 0, 0, CV_INTER_NN);
}
