//
//  cvUtil.h
//  2016ArtHackDay
//
//  Created by Ryota Katoh on 2016/11/14.
//
//

// This file is just copy and paste from
// http://qiita.com/TatsuyaOGth/items/d4493446ba3e5e292753

#ifndef cvUtil_h
#define cvUtil_h
#include "ofMain.h"
#include <opencv2/core.hpp>

template <typename T>
static cv::Mat toCv(ofPixels_<T>& pix) {
    int depth;
    switch(pix.getBytesPerChannel()) {
        case 4: depth = CV_32F;
        case 2: depth = CV_16U;
        case 1: default: depth = CV_8U;
    }
    return cv::Mat(pix.getHeight(), pix.getWidth(), CV_MAKETYPE(depth, pix.getNumChannels()), pix.getData(), 0);
}



cv::Point2f toCv(ofVec2f vec)
{
    return cv::Point2f(vec.x, vec.y);
}

cv::Point3f toCv(ofVec3f vec)
{
    return cv::Point3f(vec.x, vec.y, vec.z);
}

cv::Rect toCv(ofRectangle rect)
{
    return cv::Rect(rect.x, rect.y, rect.width, rect.height);
}

cv::Mat toCv(ofMesh& mesh)
{
    vector<ofVec3f>& vertices = mesh.getVertices();
    return cv::Mat(1, vertices.size(), CV_32FC3, &vertices[0]);
}

vector<cv::Point2f> toCv(const ofPolyline& polyline)
{
    // if polyline.getVertices() were const, this could wrap toCv(vec<vec2f>)
    vector<cv::Point2f> contour(polyline.size());
    for(int i = 0; i < polyline.size(); i++) {
        contour[i].x = polyline[i].x;
        contour[i].y = polyline[i].y;
    }
    return contour;
}

vector<cv::Point2f> toCv(const vector<ofVec2f>& points)
{
    vector<cv::Point2f> out(points.size());
    for(int i = 0; i < points.size(); i++) {
        out[i].x = points[i].x;
        out[i].y = points[i].y;
    }
    return out;
}

vector<cv::Point3f> toCv(const vector<ofVec3f>& points)
{
    vector<cv::Point3f> out(points.size());
    for(int i = 0; i < points.size(); i++) {
        out[i].x = points[i].x;
        out[i].y = points[i].y;
        out[i].z = points[i].z;
    }
    return out;
}

cv::Scalar toCv(ofColor color)
{
    return cv::Scalar(color.r, color.g, color.b, color.a);
}




ofVec2f toOf(cv::Point2f point)
{
    return ofVec2f(point.x, point.y);
}

ofVec3f toOf(cv::Point3f point)
{
    return ofVec3f(point.x, point.y, point.z);
}

ofRectangle toOf(cv::Rect rect)
{
    return ofRectangle(rect.x, rect.y, rect.width, rect.height);
}


#endif /* cvUtil_h */
