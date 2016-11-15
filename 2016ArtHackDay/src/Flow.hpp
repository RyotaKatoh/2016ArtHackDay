//
//  Flow.hpp
//  2016ArtHackDay
//
//  Created by Ryota Katoh on 2016/11/16.
//
//

#ifndef Flow_hpp
#define Flow_hpp

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/shape.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/video.hpp>
#include "Utilities.h"

class Flow {
public:
    // should constructor be protected?
    Flow();
    virtual ~Flow();

    //call these functions to calculate flow on sequential images.
    //After this call the flow field will be populated and
    //subsequent calls to getFlow() will be updated

    //call with two contiguous images
    template <class T>
    void calcOpticalFlow(T& lastImage, T& currentImage) {
        calcOpticalFlow(toCv(lastImage), toCv(currentImage));
    }
    void calcOpticalFlow(cv::Mat lastImage, cv::Mat currentImage);

    //call with subsequent images to do running optical flow.
    //the Flow class internally stores the last image for convenience
    template <class T>
    void calcOpticalFlow(T& currentImage) {
        calcOpticalFlow(toCv(currentImage));
    }
    void calcOpticalFlow(cv::Mat nextImage);

    void draw();
    void draw(float x, float y);
    void draw(float x, float y, float width, float height);
    void draw(ofRectangle rect);
    int getWidth();
    int getHeight();

    virtual void resetFlow();

private:
    cv::Mat last, curr;

protected:
    bool hasFlow;

    //specific flow implementation
    virtual void calcFlow(cv::Mat prev, cv::Mat next) = 0;
    //specific drawing implementation
    virtual void drawFlow(ofRectangle r) = 0;
};

class FlowFarneback: public Flow {
public:

    FlowFarneback();
    virtual ~FlowFarneback();

    //see http://opencv.willowgarage.com/documentation/cpp/motion_analysis_and_object_tracking.html
    //for a description of these parameters

    void setPyramidScale(float scale);
    void setNumLevels(int levels);
    void setWindowSize(int winsize);
    void setNumIterations(int interations);
    void setPolyN(int polyN);
    void setPolySigma(float polySigma);
    void setUseGaussian(bool gaussian);

    cv::Mat& getFlow();
    ofVec2f getTotalFlow();
    ofVec2f getAverageFlow();
    ofVec2f getFlowOffset(int x, int y);
    ofVec2f getFlowPosition(int x, int y);
    ofVec2f getTotalFlowInRegion(ofRectangle region);
    ofVec2f getAverageFlowInRegion(ofRectangle region);

    //call this if you switch to a new video file to reset internal caches
    void resetFlow();

protected:
    cv::Mat flow;

    void drawFlow(ofRectangle rect);
    void calcFlow(cv::Mat prev, cv::Mat next);

    float pyramidScale;
    int numLevels;
    int windowSize;
    int numIterations;
    int polyN;
    float polySigma;
    bool farnebackGaussian;
};

#endif /* Flow_hpp */
