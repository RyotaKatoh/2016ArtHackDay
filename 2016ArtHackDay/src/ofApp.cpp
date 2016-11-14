#include "ofApp.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/shape.hpp>
#include "cvUtil.h"

int numSamples = 30;


static std::vector<cv::Point> simpleContour(const cv::Mat& src)
{
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Point> contour_points;
    cv::findContours(src, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    size_t contours_size = contours.size();

    for (size_t border = 0; border < contours_size; border++)
    {
        size_t contour_size = contours[border].size();
        for (size_t p = 0; p < contour_size; p++)
        {
            contour_points.push_back(contours[border][p]);
        }
    }
    return contour_points;
}

int shapeRecognizer(ofImage &src, vector<ofImage> images) {

    vector<cv::Point> srcContours = simpleContour(toCv(src.getPixels()));

    vector< vector<cv::Point> > contours;
    for(int i=0;i<images.size();i++){
        vector<cv::Point> contour = simpleContour(toCv(images[i].getPixels()));
        contours.push_back(contour);
    }

    cv::Ptr<cv::ShapeContextDistanceExtractor> mysc = cv::createShapeContextDistanceExtractor();

    int minIdx = 0;
    float minDis = INT_MAX;
    for(int i=0;i<contours.size();i++) {
        float d = mysc->computeDistance(srcContours, contours[i]);
        cout<<i<<" : "<<d<<endl;
        if (d < minDis) {
            minDis = d;
            minIdx = i;
        }
    }

    return minIdx;
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0, 0, 0);


    // Set up Camera
    vector<ofVideoDevice> devices;
    devices = video.listDevices();
    int deviceID = 0;
    for(int i=0;i<devices.size();i++){
        cout<<devices[i].deviceName[0]<<endl;
        if (devices[i].deviceName[0] == 'M'){
            deviceID = i;
        }
    }
    video.setDeviceID(deviceID);
    video.initGrabber(640, 480);


    //
    binaryImage.allocate(video.getWidth(), video.getHeight(), OF_IMAGE_GRAYSCALE);


    // load sample images
    ofFile file;
    for(int i=0;i<numSamples;i++){
        ofImage img;
        if (file.doesFileExist(ofToString(i) + "jpg")){
            img.load(ofToString(i) + ".jpg");
            img.resize(img.getWidth()/5., img.getHeight()/5.);
            img.setImageType(OF_IMAGE_GRAYSCALE);
            sampleImages.push_back(img);
        }
    }


    recognizedID = -1;

    gui.setup();
    gui.add(maxThreshold.setup("white max threshold", 255, 0.0, 255));
    gui.add(detectThreshold.setup("human detect threshold", 60000, 10000, 100000));

}

//--------------------------------------------------------------
void ofApp::update(){

    video.update();


    shadowArea = 0;

    binaryImage.setFromPixels(video.getPixels());
    for(int y=0;y<video.getHeight();y++) {
        for(int x=0;x<video.getWidth();x++) {

            ofColor col = binaryImage.getColor(x, y);

            if (col.r > maxThreshold){
                binaryImage.setColor(x, y, ofColor(0,0,0));
            } else {
                binaryImage.setColor(x, y, ofColor(255,255,255));
                shadowArea++;
            }
        }
    }
    binaryImage.update();


}

//--------------------------------------------------------------
void ofApp::draw(){

    video.draw(0, 0);


    binaryImage.draw(video.getWidth(), 0);

    if (shadowArea > detectThreshold) {
        ofPushStyle();
        ofSetColor(255, 0, 0);
        ofDrawCircle(ofGetWidth()/2. - 2, 10, 5);
        ofPopStyle();
    }

    gui.draw();


}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' ') {
        string fileName = "pose_sample_" + ofGetTimestampString() + ".jpg";
        binaryImage.save(fileName);
    }

    if (key == 'r') {
        ofImage shadowImage;
        shadowImage = binaryImage;
        shadowImage.resize(binaryImage.getWidth()/5., binaryImage.getHeight()/5.);

        recognizedID = shapeRecognizer(shadowImage, sampleImages);
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
