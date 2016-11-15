#include "ofApp.h"

#include "cvUtil.h"
#include "cv.h"

int numSamples = 30;
float resizeRate = 5.0;


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



    binaryImage.allocate(video.getWidth(), video.getHeight(), OF_IMAGE_GRAYSCALE);
    prevBinaryImage.allocate(binaryImage.getWidth(), binaryImage.getHeight(), OF_IMAGE_GRAYSCALE);
    opticalFlowImage.allocate(binaryImage.getWidth(), binaryImage.getHeight(), OF_IMAGE_GRAYSCALE);



    // load sample images
    ofFile file;
    for(int i=0;i<numSamples;i++){
        ofImage img;
        if (file.doesFileExist(ofToDataPath(ofToString(i) + ".jpg"))){
            img.load(ofToString(i) + ".jpg");
            img.resize(img.getWidth()/resizeRate, img.getHeight()/resizeRate);
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
    prevBinaryImage = binaryImage;


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

    fb.calcOpticalFlow(binaryImage);


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



    float drawX =0.0, drawY = video.getHeight() + 15.0;
    for(int i=0;i<sampleImages.size();i++){
        ofImage drwImg = sampleImages[i];
        drwImg.resize(drwImg.getWidth()/2., drwImg.getHeight()/2.);
        if (drawX + drwImg.getWidth() > ofGetWidth()) {
            drawX = 0.0;
            drawY += drwImg.getHeight();
        }

        drwImg.draw(drawX, drawY);

        if(i == recognizedID) {
            ofPushStyle();

            ofNoFill();

            ofSetColor(255, 0, 0);
            ofSetLineWidth(5);
            ofDrawRectangle(drawX+5, drawY+5, drwImg.getWidth()-5, drwImg.getHeight()-5);

            ofPopStyle();
        }
        
        drawX += drwImg.getWidth();
    }

    if(debugImage.isAllocated()) {
        debugImage.draw(ofGetWidth()/2. - debugImage.getWidth()/2., ofGetHeight() - debugImage.getHeight());
    }

    fb.draw(video.getWidth(), video.getHeight(), 640, 480);


    gui.draw();

//    ofVec2f v = fb.getTotalFlow();
//    ofDrawBitmapString("vector (x, y) = (" + ofToString(v.x) + ", " + ofToString(v.y) + ")", 10, video.getHeight() + 10);


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
        shadowImage.resize(binaryImage.getWidth()/resizeRate, binaryImage.getHeight()/resizeRate);

        recognizedID = shapeRecognizer(shadowImage, sampleImages);
    }

    if (key == 't') {

        ofImage shadowImage;
        shadowImage = debugImage;
        shadowImage.resize(binaryImage.getWidth()/resizeRate, binaryImage.getHeight()/resizeRate);

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
    if(dragInfo.files.size() > 0) {
        debugImage.load(dragInfo.files[0]);
        debugImage.resize(debugImage.getWidth()/resizeRate, debugImage.getHeight()/resizeRate);
        debugImage.setImageType(OF_IMAGE_GRAYSCALE);
    }
    
}
