#include "ofApp.h"

#include "cv.h"

int numSamples = 100;
int maxNumMovement = 6 * 30;
int movementWindowSize = 3;
float resizeRate = 10.0;

//#define HOST "169.254.113.254"
#define HOST "localhost"
#define PORT 5005

void Recognizer::setup(string host, int port, int numSamples, float resizeRate){
    
    sender.setup(host, port);
    recognizedID = -1;
    
}

void Recognizer::recognize(ofImage &src, vector<vector<cv::Point> > &contours){
    
    ofImage shadowImage;
    shadowImage = src;
    shadowImage.setImageType(OF_IMAGE_GRAYSCALE);
    shadowImage.resize(src.getWidth()/resizeRate, src.getHeight()/resizeRate);
    
    recognizedID = shapeRecognizer(shadowImage, contours);
    
}

void Recognizer::sendOSC(ofImage &src, vector< vector<cv::Point> > &contours){
    
    recognize(src, contours);
    
    ofxOscMessage m;
    m.setAddress("/pattern");
    m.addIntArg(recognizedID);
    sender.sendMessage(m, false);
    cout<<"send ID: "<<recognizedID<<endl;
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

    // set osc sender
    sender.setup(HOST, PORT);
    
    //recognizer.setup(HOST, PORT, numSamples, resizeRate);
    //recognizer.startThread();

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
    
    for(int i=0;i<sampleImages.size();i++){
        vector<cv::Point> contour = simpleContour(toCv(sampleImages[i].getPixels()));
        sampleContours.push_back(contour);
    }


    recognizedID = -1;
    isMoving = false;
    isShoot = false;
    lastAverageValue = 0.0;
    lastShootTime = 0.0;
    
    gui.setup();
    gui.add(isProduction.set("production", false));
    gui.add(maxThreshold.setup("white max threshold", 255, 0.0, 255));
    gui.add(leftThreshold.setup("left threshold", 0, 0.0, video.getWidth()));
    gui.add(rightThreshold.setup("right threshold", 0, 0.0, video.getWidth()));
    gui.add(topThreshold.setup("up threshold", 0, 0.0, video.getHeight()));
    gui.add(bottomThreshold.setup("bottom threshold", 0, 0.0, video.getHeight()));
    gui.add(detectThreshold.setup("human detect threshold", 60000, 10000, 100000));
    gui.add(opticalThreshold.setup("optical flow threshold", 300, 0.0, 1000));
    //gui.add(maxOpticalThreshold.setup("max optical flow value to visualize", 1000, 1000, 5000));
    maxOpticalThreshold = 1000;
    gui.add(stableWindowSize.setup("stable window size", 15, 1, 300));
    gui.add(shootInterval.setup("shoot interval[sec]", 0.7, 0.5, 5.0));

    gui.add(fbPyrScale.set("fbPyrScale", .5, 0, .99));
    gui.add(fbLevels.set("fbLevels", 4, 1, 8));
    gui.add(fbIterations.set("fbIterations", 2, 1, 8));
    gui.add(fbPolyN.set("fbPolyN", 7, 5, 10));
    gui.add(fbPolySigma.set("fbPolySigma", 1.5, 1.1, 2));
    gui.add(fbUseGaussian.set("fbUseGaussian", false));
    gui.add(fbWinSize.set("winSize", 32, 4, 64));
    gui.add(isFake.set("fake mode", false));
    
    gui.loadFromFile("settings.xml");


}

//--------------------------------------------------------------
void ofApp::update(){

    video.update();


    shadowArea = 0;


    if(video.isFrameNew()){
        
        // update binary image
        binaryImage.setFromPixels(video.getPixels());
        for(int y=0;y<video.getHeight();y++) {
            for(int x=0;x<video.getWidth();x++) {

                ofColor col = binaryImage.getColor(x, y);

                if (col.r > maxThreshold || x < leftThreshold || x > video.getWidth() - rightThreshold || y < topThreshold || y > video.getHeight() - bottomThreshold){
                    binaryImage.setColor(x, y, ofColor(0,0,0));
                } else {
                    binaryImage.setColor(x, y, ofColor(255,255,255));
                    shadowArea++;
                }
            }
        }

        medianFilter(binaryImage);
        binaryImage.update();
        

        // update opticalflow
        fb.setPyramidScale(fbPyrScale);
        fb.setNumLevels(fbLevels);
        fb.setWindowSize(fbWinSize);
        fb.setNumIterations(fbIterations);
        fb.setPolyN(fbPolyN);
        fb.setPolySigma(fbPolySigma);
        fb.setUseGaussian(fbUseGaussian);

        fb.calcOpticalFlow(binaryImage);

        ofVec2f v = fb.getTotalFlow();
        float movementTotal = (abs(v.x) + abs(v.y)) / 1000;


        opticalMovements.push_back(movementTotal);
        if (opticalMovements.size() > maxNumMovement) {
            opticalMovements.erase(opticalMovements.begin());
        }
        
        // update average movement
        averageOpticalMovements.clear();
        if(opticalMovements.size() > movementWindowSize) {
            
            
            for(int i=movementWindowSize;i<opticalMovements.size();i++){
                float now = 0.0;
                for(int j=0;j<movementWindowSize;j++) {
                    now += opticalMovements[i-j];
                }
                now = now / (float)movementWindowSize;
                
                averageOpticalMovements.push_back(now);
                if (averageOpticalMovements.size() > maxNumMovement) {
                    averageOpticalMovements.erase(averageOpticalMovements.begin());
                }
                
            }
            
        }


        maximumPoint.clear();
        maximumPoint.assign(maxNumMovement, false);
        bool prevIncrease = true;
        for(int i=0;i<averageOpticalMovements.size();i++){
            if(i+1 < averageOpticalMovements.size()){
                bool increase = true;
                if(averageOpticalMovements[i+1] - averageOpticalMovements[i] < 0){
                    increase = false;
                }
                if(averageOpticalMovements[i] > opticalThreshold && prevIncrease && !increase) {
                    maximumPoint[i] = true;
 
                }
                prevIncrease = increase;
            }
        }
        
        if(averageOpticalMovements.size() >= 2 && maximumPoint[averageOpticalMovements.size() - 2]){
            isShoot = true;
        } else {
            isShoot = false;
        }
        
        
        float lastMovement = 0.0;
        if (opticalMovements.size() > 0)
            lastMovement = opticalMovements[opticalMovements.size()-1];
        
        if(shadowArea > detectThreshold && lastMovement < opticalThreshold && ofGetElapsedTimef() - lastShootTime > shootInterval){
            cout<<"Shoot!!!"<<endl;
            lastShootTime = ofGetElapsedTimef();
            isShoot = false;
            //isMoving = false;
            
            if(isProduction ) {
                sendOSC();
                //recognizer.sendOSC(binaryImage, sampleContours);
            }
        }
        
        if(shadowArea > detectThreshold && lastMovement > opticalThreshold) {
            ofxOscMessage m;
            m.setAddress("/random");
            
            sender.sendMessage(m, false);
            
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

    // draw top, bottom, left and right threshold line
    ofPushStyle();
    ofSetColor(255,87,34);
    ofDrawLine(video.getWidth() + leftThreshold, 0, video.getWidth() + leftThreshold, video.getHeight());
    ofDrawLine(ofGetWidth() - rightThreshold, 0, ofGetWidth() - rightThreshold, video.getHeight());
    ofDrawLine(video.getWidth(), topThreshold, ofGetWidth(), topThreshold);
    ofDrawLine(video.getWidth(), video.getHeight() - bottomThreshold, ofGetWidth(), video.getHeight() - bottomThreshold);
    ofPopStyle();

    if(debugImage.isAllocated()) {
        debugImage.draw(ofGetWidth()/2. - debugImage.getWidth()/2., ofGetHeight() - debugImage.getHeight());
    }

    // draw opticalFlow
    fb.draw(video.getWidth(), video.getHeight(), 640, 480);

    // draw optical movement

    if(averageOpticalMovements.size() > movementWindowSize) {
        ofPushStyle();
        for(int i=1;i<averageOpticalMovements.size();i++){
            float now = averageOpticalMovements[i];
            float prev = averageOpticalMovements[i-1];
            float nowY = ofMap(now, 0.0, maxOpticalThreshold, ofGetHeight(), ofGetHeight() - video.getHeight());
            float prevY = ofMap(prev, 0.0, maxOpticalThreshold, ofGetHeight(), ofGetHeight() - video.getHeight());
            float prevX = ofMap(i-1, movementWindowSize-1, maxNumMovement, video.getWidth(), ofGetWidth());
            float nowX = ofMap(i, movementWindowSize-1, maxNumMovement, video.getWidth(), ofGetWidth());
            
            if (now > opticalThreshold) {
                ofSetColor(236,64,122);
            } else {
                ofSetColor(41,182,246);
            }
            
            ofSetLineWidth(4);
            ofDrawLine(prevX, prevY, nowX, nowY);
            
            if(maximumPoint[i]){
                ofSetColor(0, 255, 0);
                ofDrawCircle(nowX, nowY, 10);
            }
        }
    
        
        ofSetColor(102,187,106);
        float thresholdLine = ofMap(opticalThreshold, 0.0, maxOpticalThreshold, ofGetHeight(), ofGetHeight() - video.getHeight());
        ofDrawLine(video.getWidth(), thresholdLine, ofGetWidth(), thresholdLine);
        
        ofPopStyle();
        ofDrawBitmapString(ofToString(opticalMovements[opticalMovements.size()-1]), ofGetWidth()/2., ofGetHeight()/2. + 15);
    }


    gui.draw();





}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' ') {
        string fileName = "pose_sample_" + ofGetTimestampString() + ".jpg";
        binaryImage.save(fileName);
    }
    
    if (key == 't') {
        ofImage shadowImage;
        shadowImage = debugImage;
        shadowImage.setImageType(OF_IMAGE_GRAYSCALE);
        shadowImage.resize(binaryImage.getWidth()/resizeRate, binaryImage.getHeight()/resizeRate);
        
        
        recognizedID = shapeRecognizer(shadowImage, sampleContours);
    }

    if (key == 'r') {
        sendOSC();
        //recognizer.sendOSC(binaryImage, sampleContours);
    }

    if(key == 'm') {
        ofxOscMessage m;
        m.setAddress("/pattern");
        m.addIntArg(0);

        sender.sendMessage(m, false);

    }
    
    if(key == '0') {
        ofxOscMessage m;
        m.setAddress("/pattern");
        m.addIntArg(0);
        
        sender.sendMessage(m, false);
        
    }
    if(key == '1') {
        ofxOscMessage m;
        m.setAddress("/pattern");
        m.addIntArg(1);
        
        sender.sendMessage(m, false);
        
    }
    
    if(key == '2') {
        ofxOscMessage m;
        m.setAddress("/pattern");
        m.addIntArg(2);
        
        sender.sendMessage(m, false);
        
    }

    if(key == 'p') {
        ofxOscMessage m;
        m.setAddress("/ping");
        
        sender.sendMessage(m, false);
        
    }

    if(key == 'n') {
        ofxOscMessage m;
        m.setAddress("/set");
        m.addIntArg(0);
        m.addIntArg(0);
        m.addIntArg(48);

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
    
    
    recognizedID = shapeRecognizer(shadowImage, sampleContours);

}

void ofApp::sendOSC() {

    recognizeSilhouette();

    ofxOscMessage m;
    m.setAddress("/pattern");
    m.addIntArg(recognizedID);
    sender.sendMessage(m, false);
    cout<<"send ID: "<<recognizedID<<endl;

}
