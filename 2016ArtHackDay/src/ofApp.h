#pragma once

#include "ofMain.h"

#include "ofxGui.h"
#include "ofxOsc.h"
#include "Flow.hpp"


class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    void recognizeSilhouette();
    void sendOSC();

    ofVideoGrabber video;
    ofImage binaryImage;

    int shadowArea;

    vector<ofImage> sampleImages;

    int recognizedID;

    vector<float> opticalMovements;
    bool isMoving;
    bool isShoot;
    float lastAverageValue;

    ofxPanel gui;
    ofParameter<bool> isProduction;
    ofxFloatSlider maxThreshold;
    ofxIntSlider detectThreshold;
    ofxFloatSlider opticalThreshold;
    ofxFloatSlider maxOpticalThreshold;
    ofxFloatSlider leftThreshold;
    ofxFloatSlider rightThreshold;

    ofParameter<float> fbPyrScale, fbPolySigma;
    ofParameter<int> fbLevels, fbIterations, fbPolyN, fbWinSize;
    ofParameter<bool> fbUseGaussian;


    ofxOscSender sender;

    FlowFarneback fb;

    ofImage debugImage;
};
