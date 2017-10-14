//
//  recognizer.hpp
//  2016ArtHackDay
//
//  Created by Ryota Katoh on 2016/11/24.
//
//

#ifndef recognizer_hpp
#define recognizer_hpp

#include "ofMain.h"
#include "ofxOSC.h"
#include "Utilities.h"
#include "cv.h"
class Recognizer: public ofThread{
public:
    void setup(string host, int port, int numSamples, float resizeRate);
    void recognize(ofImage &src, int realNumber, bool isFake);
    void sendOSC(ofImage &src, bool isFake);

    vector<ofImage> sampleImages;
    int recognizedID;
    ofxOscSender sender;
    float resizeRate;
    int numSamples;
};

/*
#include "ofMain.h"
#include "ofxOSC.h"
#include "Flow.hpp"

class Recognizer: public ofThread {
public:
    //void setup();

    //void recognize(ofImage &src, bool isFake);
    //void sendOSC(ofImage &src, bool isFake);

    vector<ofImage> sampleImages;
    int recognizedID;
    ofxOscSender sender;
};

 */
#endif /* recognizer_hpp */
