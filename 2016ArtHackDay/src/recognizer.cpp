//
//  recognizer.cpp
//  2016ArtHackDay
//
//  Created by Ryota Katoh on 2016/11/24.
//
//

#include "recognizer.hpp"


void Recognizer::setup(string host, int port, int _numSamples, float _resizeRate){

    // set osc sender
    sender.setup(host, port);

    numSamples = _numSamples;
    resizeRate = _resizeRate;

    
    recognizedID = -1;

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
}



void Recognizer::recognize(ofImage &src, int realNumber, bool isFake){

    ofImage shadowImage;
    shadowImage = src;
    shadowImage.setImageType(OF_IMAGE_GRAYSCALE);
    shadowImage.resize(src.getWidth()/resizeRate, src.getHeight()/resizeRate);


    vector<ofImage> samples;
    if(isFake) {
        samples = sampleImages;
    } else {
        for(int i=0;i<realNumber;i++){
            samples.push_back(sampleImages[i]);
        }
    }

    //recognizedID = shapeRecognizer(shadowImage, samples);
}

/*
void Recognizer::sendOSC(ofImage &src, bool isFake){

    recognize(src, isFake);

    ofxOscMessage m;
    m.setAddress("/pattern");
    m.addIntArg(recognizedID);
    sender.sendMessage(m, false);
    cout<<"send ID: "<<recognizedID<<endl;
}
*/
