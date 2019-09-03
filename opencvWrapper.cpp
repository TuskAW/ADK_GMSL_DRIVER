#include "opencvWrapper.h"


OpencvWrapper::OpencvWrapper() {
    imgBuffer = nullptr;
    recording = false;
}

OpencvWrapper::~OpencvWrapper() {
    if(imgBuffer) {
        delete imgBuffer;
    }
}

void OpencvWrapper::hello() {
    cv::Mat img(512, 512, CV_8UC3, cv::Scalar(0));

    cv::putText(img,
        "Hello, open CV",
        cv::Point(10, img.rows/2),
        cv::FONT_HERSHEY_DUPLEX,
        1.0,
        CV_RGB(118, 185, 0),
        2);
    cv::imshow("Hello!", img);
    cv::waitKey();
}

void OpencvWrapper::display(uint8_t *data, int width, int height) {
    setImgBuffer(data, width, height);
    cv::imshow("Boson", img);
    cv::waitKey(10);
}

void OpencvWrapper::setImgBuffer(uint8_t *data, int width, int height) {
    if(!imgBuffer) {
        imgBuffer = new uint8_t[width * height];
        img = cv::Mat(height, width, CV_8UC1, reinterpret_cast<void *>(imgBuffer));
    }
    memcpy(imgBuffer, data, width * height * sizeof(uint8_t));
}

void OpencvWrapper::startRecording(int width, int height) {
    recording = true;
    videoWidth = width;
    videoHeight = height;
    videoRecorder = cv::VideoWriter("test.mp4", CV_8UC1, 30, 
        cv::Size(width, height), false);
}

void OpencvWrapper::stopRecording() {
    recording = false;
    videoRecorder.release();
}

void OpencvWrapper::recordFrame(uint8_t *data) {
    if(!recording) {
        return;
    }
    setImgBuffer(data, videoWidth, videoHeight);

    videoRecorder.write(img);
}