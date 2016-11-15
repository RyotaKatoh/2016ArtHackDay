//
//  Flow.cpp
//  2016ArtHackDay
//
//  Created by Ryota Katoh on 2016/11/16.
//
//

#include "Flow.hpp"
#include "ofGraphics.h"
#include "Wrappers.h"

using namespace cv;


Flow::Flow()
:hasFlow(false) {
}

Flow::~Flow(){
}

//call with two images
void Flow::calcOpticalFlow(Mat lastImage, Mat currentImage){
    if(lastImage.channels() == 1 && currentImage.channels() == 1) {
        calcFlow(lastImage, currentImage);
    } else {
        copyGray(lastImage, last);
        copyGray(currentImage, curr);
        calcFlow(last, curr);
    }
    hasFlow = true;
}

//you can add subsequent images this way without having to store
//the previous one yourself
void Flow::calcOpticalFlow(Mat nextImage){
    copyGray(nextImage, curr);
    if(last.size == curr.size){
        calcFlow(last, curr);
        hasFlow = true;
    }
    swap(curr, last);
}

void Flow::draw(){
    if(hasFlow) {
        drawFlow(ofRectangle(0, 0, getWidth(), getHeight()));
    }
}
void Flow::draw(float x, float y){
    if(hasFlow){
        drawFlow(ofRectangle(x, y, getWidth(), getHeight()));
    }
}
void Flow::draw(float x, float y, float width, float height){
    if(hasFlow){
        drawFlow(ofRectangle(x,y,width,height));
    }
}
void Flow::draw(ofRectangle rect){
    if(hasFlow){
        drawFlow(rect);
    }
}
int Flow::getWidth()  {
    return curr.cols;
}
int Flow::getHeight() {
    return curr.rows;
}
void Flow::resetFlow() {
    last = Mat();
    curr = Mat();
    hasFlow = false;
}



FlowFarneback::FlowFarneback()
:pyramidScale(0.5)
,numLevels(4)
,windowSize(8)
,numIterations(2)
,polyN(7)
,polySigma(1.5)
,farnebackGaussian(false)
{
}

FlowFarneback::~FlowFarneback(){
}

void FlowFarneback::setPyramidScale(float scale){
    if(scale < 0.0 || scale >= 1.0){
        ofLogWarning("FlowFarneback::setPyramidScale") << "setting scale to a number outside of 0 - 1";
    }
    this->pyramidScale = scale;
}

void FlowFarneback::setNumLevels(int levels){
    this->numLevels = levels;
}
void FlowFarneback::setWindowSize(int winsize){
    this->windowSize = winsize;
}
void FlowFarneback::setNumIterations(int interations){
    this->numIterations = interations;
}
void FlowFarneback::setPolyN(int polyN){
    this->polyN = polyN;
}
void FlowFarneback::setPolySigma(float polySigma){
    this->polySigma = polySigma;
}
void FlowFarneback::setUseGaussian(bool gaussian){
    this->farnebackGaussian = gaussian;
}

void FlowFarneback::resetFlow(){
    Flow::resetFlow();
    flow.setTo(0);
}

void FlowFarneback::calcFlow(Mat prev, Mat next){
    int flags = 0;
    if(hasFlow){
        flags = OPTFLOW_USE_INITIAL_FLOW;
    }
    if(farnebackGaussian){
        flags |= OPTFLOW_FARNEBACK_GAUSSIAN;
    }

    calcOpticalFlowFarneback(prev,
                             next,
                             flow,
                             pyramidScale,
                             numLevels,
                             windowSize,
                             numIterations,
                             polyN,
                             polySigma,
                             flags);
}
Mat& FlowFarneback::getFlow() {
    if(!hasFlow) {
        flow = Mat::zeros(1, 1, CV_32FC2);
    }
    return flow;
}
ofVec2f FlowFarneback::getFlowOffset(int x, int y){
    if(!hasFlow){
        return ofVec2f(0, 0);
    }
    const Vec2f& vec = flow.at<Vec2f>(y, x);
    return ofVec2f(vec[0], vec[1]);
}
ofVec2f FlowFarneback::getFlowPosition(int x, int y){
    if(!hasFlow){
        return ofVec2f(0, 0);
    }
    const Vec2f& vec = flow.at<Vec2f>(y, x);
    return ofVec2f(x + vec[0], y + vec[1]);
}
ofVec2f FlowFarneback::getTotalFlow(){
    return getTotalFlowInRegion(ofRectangle(0,0,flow.cols, flow.rows));
}
ofVec2f FlowFarneback::getAverageFlow(){
    return getAverageFlowInRegion(ofRectangle(0,0,flow.cols,flow.rows));
}

ofVec2f FlowFarneback::getAverageFlowInRegion(ofRectangle rect){
    return getTotalFlowInRegion(rect)/(rect.width*rect.height);
}

ofVec2f FlowFarneback::getTotalFlowInRegion(ofRectangle region){
    if(!hasFlow){
        return ofVec2f(0, 0);
    }

    const Scalar& sc = sum(flow(toCv(region)));
    return ofVec2f(sc[0], sc[1]);
}

void FlowFarneback::drawFlow(ofRectangle rect){
    if(!hasFlow){
        return;
    }
    ofVec2f offset(rect.x,rect.y);
    ofVec2f scale(rect.width/flow.cols, rect.height/flow.rows);
    int stepSize = 4; //TODO: make class-level parameteric
    for(int y = 0; y < flow.rows; y += stepSize) {
        for(int x = 0; x < flow.cols; x += stepSize) {
            ofVec2f cur = ofVec2f(x, y) * scale + offset;
            ofDrawLine(cur, getFlowPosition(x, y) * scale + offset);
        }
    }
}
