/* NVIDIA CORPORATION gave permission to FLIR Systems, Inc to modify this code
  * and distribute it as part of the ADAS GMSL Kit.
  * http://www.flir.com/
  * October-2019
*/
#include <fstream>

#include "opencvWrapper.h"

OpencvWrapper::OpencvWrapper(int width, int height, int bytesPerPixel) :
    width(width),
    height(height),
    bytesPerPixel(bytesPerPixel),
    imgBuffer(nullptr)
{
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

void OpencvWrapper::sendFrame(uint8_t *data) {
    setImgBuffer(data);
}

void OpencvWrapper::getFrame(uint8_t *data) {
    memcpy(data, imgBuffer, width * height * bytesPerPixel * sizeof(uint8_t));
}

void OpencvWrapper::getTelemetry(uint8_t *data) {
    if(!telemetry) {
        return;
    }
    memcpy(data, telemetry, width * bytesPerPixel * sizeof(uint8_t));
}

void OpencvWrapper::display() {
    cv::imshow("Boson", img);
    cv::waitKey(1);
}

void OpencvWrapper::setImgBuffer(uint8_t *data) {
    if(!imgBuffer) {
        imgBuffer = new uint8_t[width * height * bytesPerPixel];
        int pixelType = CV_8UC1;
        if(bytesPerPixel == 2) {
            pixelType = CV_16UC1;
        }
        img = cv::Mat(height, width, pixelType, 
            reinterpret_cast<void *>(imgBuffer));
    }
    memcpy(imgBuffer, data, width * height * bytesPerPixel);

    agc();
}

void OpencvWrapper::startRecording(int fps, std::string filename) {
    // assume that a frame as been captured (img has been initialized) before calling this
    recorder = OpencvRecorder(img, fps, filename);
}

void OpencvWrapper::stopRecording() {
    recorder.stop();
}

void OpencvWrapper::recordFrame() {
    if(!recorder.recording) {
        return;
    }

    recorder.captureFrame();
}

void OpencvWrapper::saveImage(std::string filename) {
    uint8_t *tempBuffer = (uint8_t*)malloc(width * height * bytesPerPixel * sizeof(uint8_t));
    memcpy(tempBuffer, imgBuffer, width * height * bytesPerPixel);
    int pixelType = CV_8UC1;
    if(bytesPerPixel == 2) {
        pixelType = CV_16UC1;
    }
    cv::Mat newImg(height, width, pixelType,
        reinterpret_cast<void *>(tempBuffer));
    cv::imwrite(filename, newImg);
    delete tempBuffer;
}

void OpencvWrapper::sendTelemetry(uint8_t *data, int stride) {
    int serialStart = 2;

    if(!telemetry) {
        telemetry = new uint8_t[stride];
    }
    memcpy(telemetry, data, stride * sizeof(uint8_t));

    serialNumber = 0;
    for (size_t i = 0; i < 4; i++) {
        serialNumber += (uint32_t)(data[i + serialStart] << (24 - (8 * i)));
    }
}

uint32_t OpencvWrapper::getSerialNumber() {
    return serialNumber;
}

void OpencvWrapper::agc() {
    int bytesPerPixel = 1;
    if(img.type() == CV_16UC1) {
        bytesPerPixel = 2;
    }

    cv::normalize(img, img, 0, 1 << (8 * bytesPerPixel) - 1, cv::NORM_MINMAX);
}