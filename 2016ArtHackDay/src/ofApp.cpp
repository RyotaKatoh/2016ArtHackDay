#include "ofApp.h"

#include "cvUtil.h"
#include "cv.h"

int numSamples = 30;
int maxNumMovement = 6 * 30;
int movementWindowSize = 6;
float resizeRate = 8.0;

#define HOST "localhost"
#define PORT 12345

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

    // set osc sender
    sender.setup(HOST, PORT);


    binaryImage.allocate(video.getWidth(), video.getHeight(), OF_IMAGE_GRAYSCALE);



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
    isMoving = false;

    gui.setup();
    gui.add(isProduction.set("production", false));
    gui.add(maxThreshold.setup("white max threshold", 255, 0.0, 255));
    gui.add(leftThreshold.setup("left side ignore area", 0, 0.0, video.getWidth()));
    gui.add(rightThreshold.setup("right side ignore area", 0, 0.0, video.getWidth()));
    gui.add(detectThreshold.setup("human detect threshold", 60000, 10000, 100000));
    gui.add(opticalThreshold.setup("optical flow threshold to send OSC", 300, 0.0, 1000));
    gui.add(maxOpticalThreshold.setup("max optical flow value to visualize", 1000, 1000, 5000));

}

//--------------------------------------------------------------
void ofApp::update(){

    video.update();


    shadowArea = 0;


    if(video.isFrameNew()){
        binaryImage.setFromPixels(video.getPixels());
        for(int y=0;y<video.getHeight();y++) {
            for(int x=0;x<video.getWidth();x++) {

                ofColor col = binaryImage.getColor(x, y);

                if (col.r > maxThreshold || x < leftThreshold || x > video.getWidth() - rightThreshold){
                    binaryImage.setColor(x, y, ofColor(0,0,0));
                } else {
                    binaryImage.setColor(x, y, ofColor(255,255,255));
                    shadowArea++;
                }
            }
        }

        medianFilter(binaryImage);
        binaryImage.update();

        fb.calcOpticalFlow(binaryImage);

        ofVec2f v = fb.getTotalFlow();
        float movementTotal = (abs(v.x) + abs(v.y)) / 1000;

        if(isMoving == false && movementTotal > opticalThreshold) {
            isMoving = true;
        }
        if(isMoving == true && movementTotal < opticalThreshold) {
            isMoving = false;
            if (isProduction)
                sendOSC();
        }

        opticalMovements.push_back(movementTotal);
        if (opticalMovements.size() > maxNumMovement) {
            opticalMovements.erase(opticalMovements.begin());
        }
    }



    ofSetWindowTitle(ofToString(ofGetFrameRate()));


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
        if (drawX + drwImg.getWidth() > video.getWidth()) {
            drawX = 0.0;
            drawY += drwImg.getHeight();
        }

        drwImg.draw(drawX, drawY);

        if(i == recognizedID) {
            ofPushStyle();

            ofNoFill();

            ofSetColor(255, 0, 0);
            ofSetLineWidth(3);
            ofDrawRectangle(drawX+5, drawY+5, drwImg.getWidth()-5, drwImg.getHeight()-5);

            ofPopStyle();
        }
        
        drawX += drwImg.getWidth();
    }

    // draw left and right threshold line
    ofPushStyle();
    ofSetColor(255,87,34);
    ofDrawLine(video.getWidth() + leftThreshold, 0, video.getWidth() + leftThreshold, video.getHeight());
    ofDrawLine(ofGetWidth() - rightThreshold, 0, ofGetWidth() - rightThreshold, video.getHeight());
    ofPopStyle();

    if(debugImage.isAllocated()) {
        debugImage.draw(ofGetWidth()/2. - debugImage.getWidth()/2., ofGetHeight() - debugImage.getHeight());
    }

    // draw opticalFlow
    fb.draw(video.getWidth(), video.getHeight(), 640, 480);

    // draw optical movement
    ofPushStyle();
    float prev = 0.0;
    for(int i=0;i<movementWindowSize;i++){
        prev += opticalMovements[i];
    }
    prev = prev / (float)movementWindowSize;

    for(int i=movementWindowSize;i<opticalMovements.size();i++){
        float now = 0.0;
        for(int j=0;j<movementWindowSize;j++) {
            now += opticalMovements[i-j];
        }
        now = now / (float)movementWindowSize;

        float nowY = ofMap(now, 0.0, maxOpticalThreshold, ofGetHeight(), ofGetHeight() - video.getHeight());
        float prevY = ofMap(prev, 0.0, maxOpticalThreshold, ofGetHeight(), ofGetHeight() - video.getHeight());
        float prevX = ofMap(i-1, movementWindowSize-1, maxNumMovement, video.getWidth(), ofGetWidth());
        float nowX = ofMap(i, movementWindowSize-1, maxNumMovement, video.getWidth(), ofGetWidth());

        ofSetLineWidth(5);

        if (now > opticalThreshold) {
            ofSetColor(236,64,122);
        } else {
            ofSetColor(41,182,246);
        }

        ofDrawLine(prevX, prevY, nowX, nowY);

        prev = now;

    }
    ofSetColor(102,187,106);
    float thresholdLine = ofMap(opticalThreshold, 0.0, maxOpticalThreshold, ofGetHeight(), ofGetHeight() - video.getHeight());
    ofDrawLine(video.getWidth(), thresholdLine, ofGetWidth(), thresholdLine);

    ofPopStyle();


    ofDrawBitmapString(ofToString(opticalMovements[opticalMovements.size()-1]), ofGetWidth()/2., ofGetHeight()/2. + 15);

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
        shadowImage.setImageType(OF_IMAGE_GRAYSCALE);
        shadowImage.resize(binaryImage.getWidth()/resizeRate, binaryImage.getHeight()/resizeRate);

        recognizedID = shapeRecognizer(shadowImage, sampleImages);
    }

    if (key == 't') {

        ofImage shadowImage;
        shadowImage = debugImage;
        shadowImage.setImageType(OF_IMAGE_GRAYSCALE);
        shadowImage.resize(binaryImage.getWidth()/resizeRate, binaryImage.getHeight()/resizeRate);

        recognizedID = shapeRecognizer(shadowImage, sampleImages);
        ofxOscMessage m;
        m.setAddress("/pattern_id");
        m.addIntArg(recognizedID);
        sender.sendMessage(m, false);
    }

    if(key == 'm') {
        ofxOscMessage m;
        m.setAddress("/pattern_id");
        m.addIntArg(recognizedID);
        sender.sendMessage(m, false);

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

void ofApp::recognizeSilhouette() {
    ofImage shadowImage;
    shadowImage = binaryImage;
    shadowImage.setImageType(OF_IMAGE_GRAYSCALE);
    shadowImage.resize(binaryImage.getWidth()/resizeRate, binaryImage.getHeight()/resizeRate);

    recognizedID = shapeRecognizer(shadowImage, sampleImages);

}

void ofApp::sendOSC() {

    recognizeSilhouette();

    ofxOscMessage m;
    m.setAddress("/pattern_id");
    m.addIntArg(recognizedID);
    sender.sendMessage(m, false);

}
