#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    photoMosaic = YJL::PhotoMosaic();
//    photoMosaic.loadImages("/Users/peacedove/Downloads/WeddingPhoto/");
    photoMosaic.loadImagesFromTxt(ofToDataPath("info.txt"));
    photoMosaic.loadBaseImage("base.jpg");
    // photoMosaic.loadImageMap(ofToDataPath("imageMap.txt"));
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    photoMosaic.debugDraw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'r') {
        std::cout << "Processing..." << std::endl;
        ofImage img;
        cv::Mat result = photoMosaic.tileToImages();
        img.allocate(result.rows, result.cols, OF_IMAGE_COLOR);
        ofxCv::toOf(result, img);
        img.save("output.jpg");
        std::cout << "Image Saved" << std::endl;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
