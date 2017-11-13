#include "PhotoMosaic.h"
#include <functional>
#include <queue>
#include <fstream>
using namespace YJL;

PhotoMosaic::PhotoMosaic() {
    TILE_WIDTH = 20;
    TILE_HEIGHT = 15;
    OUT_RES_W = 1800 * 40;
    OUT_RES_H = 1350 * 40;
    OUT_SUB_X = 9;
    OUT_SUB_Y = 9;
}

void PhotoMosaic::debugDraw() {
//    if (resultImage.isAllocated()) {
//        resultImage.draw(0, 0, resultImage.getWidth() / 2, resultImage.getHeight() / 2);
//    }
//
//    if (tileImage.isAllocated()) {
//        tileImage.draw(resultImage.getWidth() / 2, 0, tileImage.getWidth(), tileImage.getHeight());
//    }
}

void PhotoMosaic::saveImagesInfoToDisk() {
    ofFile file;
    file.open(ofToDataPath("info.txt"), ofFile::WriteOnly, false);
    for (int i = 0; i < images.size(); i++) {
        file << images[i].print() << "\n";
    }
    
    file.close();
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
//        newImage.mat = ofxCv::toCv(img).clone();
//        cv::Mat imgMat = newImage.mat;
        cv::Mat imgMat = ofxCv::toCv(img);
        newImage.meanColor = cv::mean(imgMat);
        newImage.filepath = imagePath;
        newImage.usedCount = 0;
        images.push_back(newImage);
    }
    
    saveImagesInfoToDisk();
}

void PhotoMosaic::loadImagesFromTxt(std::string txtPath) {
    ofBuffer buffer = ofBufferFromFile(txtPath);
    for (auto line : buffer.getLines()) {
        std::vector<string> info = ofSplitString(line, ",");
        Image img;
        if (info[0] == "") {
            continue;
        }
        img.filepath = info[0];
        img.meanColor = cv::Scalar(ofToFloat(info[1]), ofToFloat(info[2]), ofToFloat(info[3]));
        img.usedCount = 0;
        images.push_back(img);
        
    }
}

void PhotoMosaic::loadBaseImage(std::string filePath) {
    // load base image
    baseImage.load(ofToDataPath(filePath));
    baseMat = ofxCv::toCv(baseImage);
    std::cout << baseMat.cols << ":" << baseMat.rows << std::endl;
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
    
    tileImage.allocate(tileMat.cols, tileMat.rows, baseImage.getImageType());
    ofxCv::toOf(tileMat, tileImage);
    baseImage.update();
    tileImage.update();
}

void PhotoMosaic::updateImageMap() {
    imageMap.resize(images.size());
    
    for (int j = 0; j < tileMat.rows; j++) {
        for (int i = 0; i < tileMat.cols; i++) {
            cv::Scalar color;
            color[0] = tileMat.at<cv::Vec3b>(j, i)[0];
            color[1] = tileMat.at<cv::Vec3b>(j, i)[1];
            color[2] = tileMat.at<cv::Vec3b>(j, i)[2];
            
            int index = findNearestImage(color);
            imageMap[index].push_back(cv::Point(i, j));
        }
    }
    saveImageMap();
}

void PhotoMosaic::loadImageMap(std::string filePath) {
    std::ifstream inputFile(filePath);
    imageMap.resize(images.size());
    std::string line;
    while(std::getline(inputFile, line)) {
        if (line == "") {
            continue;
        }
        std::vector<std::string> result1 = ofSplitString(line, ":");
        int index = std::stoi(result1[0]);
        std::vector<std::string> points = ofSplitString(result1[1], ";");

        for (int i = 0; i < points.size(); i++) {
            if (points[i] == "") continue;
            std::vector<std::string> values = ofSplitString(points[i], ",");
            cv::Point2d point(std::stoi(values[0]), std::stoi(values[1]));
            imageMap[index].push_back(point);

        }
    }
    
}


cv::Mat PhotoMosaic::tileToImages() {
    int outputImagesNumber = OUT_SUB_X * OUT_SUB_Y;
    // outputImages.clear();
    
    int SUB_IMAGE_WIDTH = OUT_RES_W / OUT_SUB_X;
    int SUB_IMAGE_HEIGHT = OUT_RES_H / OUT_SUB_Y;
    std::vector<cv::Mat> outputMats;
    for (int i = 0; i < outputImagesNumber; i++) {
        // ofImage img;
        // img.allocate(SUB_IMAGE_WIDTH, SUB_IMAGE_HEIGHT, OF_IMAGE_COLOR);
        // cv::Mat mat = ofxCv::toCv(img);
        cv::Mat mat(SUB_IMAGE_HEIGHT, SUB_IMAGE_WIDTH, CV_8UC3);
        outputMats.push_back(mat);
        // outputImages.push_back(img);
    }
    
    
    int t_Width = OUT_RES_W / tileMat.cols;
    int t_Height = OUT_RES_H / tileMat.rows;

    
    cv::Mat tmpMat(OUT_RES_H, OUT_RES_W, CV_8UC3);
    
    updateImageMap();
    // loadImageMap(ofToDataPath("imageMap.txt"));
    int SUB_IMAGE_CELL_X = tileMat.cols / OUT_SUB_X;
    int SUB_IMAGE_CELL_Y = tileMat.rows / OUT_SUB_Y;
    for (int p = 0; p < imageMap.size(); p++) {
        ofImage img;
        img.load(images[p].filepath);
        cv::Mat imgMat = ofxCv::toCv(img);
        cv::resize(imgMat, imgMat, cv::Size(t_Width, t_Height), CV_INTER_NN);
        for (int q = 0; q < imageMap[p].size(); q++) {
            int i = imageMap[p][q].x;
            int j = imageMap[p][q].y;
            int subimg_i = i / SUB_IMAGE_CELL_X;
            int subimg_j = j / SUB_IMAGE_CELL_Y;
            int subimg_index = subimg_i + subimg_j * OUT_SUB_X;
            cv::Mat targetMat = outputMats[subimg_index];
            cv::Mat roiMat  = targetMat(cv::Rect(
                (i - subimg_i * SUB_IMAGE_CELL_X) * t_Width,
                (j - subimg_j * SUB_IMAGE_CELL_Y) * t_Height,
                t_Width, t_Height
            ));
            // cv::Mat roiMat = tmpMat(cv::Rect(i * t_Width, j * t_Height, t_Width, t_Height));
            imgMat.copyTo(roiMat);
        }
        
    }

    for (int i = 0; i < outputMats.size(); i++) {
        cv::Mat mat = outputMats[i];
//        cv::imwrite(ofToDataPath("output_" + to_string(i % OUT_SUB_X) + "_" + to_string(i / OUT_SUB_X) + ".jpg"), mat);
        ofImage img;
        img.allocate(mat.cols, mat.rows, OF_IMAGE_COLOR);
        ofxCv::toOf(mat, img);
        // ofImage img = outputImages[i];
        // img.update();

         img.save("output_" + to_string(i % OUT_SUB_X) + "_" + to_string(i / OUT_SUB_X) + ".jpg");
    }

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
    
    for (int i = 0; i < 20; i++) {
        Image img = images[que.top().second];
        que2.push(std::pair<int, int>(img.usedCount, que.top().second));
        que.pop();
        
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

void PhotoMosaic::saveImageMap() {
    ofstream stream;
    stream.open(ofToDataPath("imageMap.txt"));
    for (int i = 0; i < imageMap.size(); i++) {
        stream << i << ":";
        for (int j = 0; j < imageMap[i].size(); j++) {
            stream << imageMap[i][j].x << "," << imageMap[i][j].y << ";";
        }
        stream << std::endl;
    }
    stream.close();

}
