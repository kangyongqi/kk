#ifndef FFFACEDETECTOR_H
#define FFFACEDETECTOR_H

#include<opencv2/opencv.hpp>
#include<atomic>
#include"ffvideoadapter.h"

class FFFaceDetector
{
public:
    FFFaceDetector();
    ~FFFaceDetector();

    void init();
    AVFrame* detectFace(AVFrame* frame);

    void setSmoothValue(int value);
    void setWhiteValue(int value);

private:
    cv::CascadeClassifier* faceDetector = nullptr;
    cv::CascadeClassifier* profileDetector = nullptr;
    FFVideoAdapter* adapter = nullptr;

    std::vector<cv::Rect>lastFaces;
    size_t noFaceCount = 0;

    std::atomic<int> smoothValue;
    std::atomic<int> whiteValue ;


};

#endif // FFFACEDETECTOR_H
