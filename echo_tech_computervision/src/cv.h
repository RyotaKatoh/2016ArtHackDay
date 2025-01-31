//
//  cv.h
//  2016ArtHackDay
//
//  Created by Ryota Katoh on 2016/11/15.
//
//

#ifndef cv_h
#define cv_h

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/shape.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/video.hpp>


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

int shapeRecognizer(ofImage &src, vector< vector<cv::Point> > contours) {

    vector<cv::Point> srcContours = simpleContour(toCv(src.getPixels()));



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

void medianFilter(ofImage &src) {

    cv::medianBlur(toCv(src), toCv(src), 5);

}


#endif /* cv_h */
