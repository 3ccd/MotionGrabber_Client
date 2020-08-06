//#define DEBUG

#if defined(DEBUG)
#pragma comment (lib, "opencv_world420d.lib")
#else
#pragma comment (lib, "opencv_world420.lib")
#endif

//#include <opencv2/opencv.hpp>
//#include <opencv2/aruco.hpp>
#include <ewclib.h>
#include <camera.h>

#include <iostream>

#define EWCLIB_CAMERA_MAX 10


Camera::Camera(const int id, const int deviceId, int resLevel, bool isPS4EYE, double framerate) {

	this->id = id;
	this->deviceId = deviceId;
	this->resLevel = resLevel;
	this->isPS4EYE = isPS4EYE;
	this->framerate = framerate;

	this->isOpen = false;
	this->isCalibrated = false;

	if (isPS4EYE) {
		switch (resLevel) {
		case RESOLUTION_H:
			captureSize = cv::Size(3448, 808);
			frameSize = cv::Size(1280, 800);
			break;
		case RESOLUTION_M:
			captureSize = cv::Size(1748, 408);
			frameSize = cv::Size(640, 400);
		}
	}
	else {
		switch (resLevel) {
		case RESOLUTION_H:
			captureSize = cv::Size(1280, 840);
			frameSize = cv::Size(1280, 840);
			break;
		case RESOLUTION_M:
			captureSize = cv::Size(640, 480);
			frameSize = cv::Size(640, 480);
		}
	}

}

int Camera::open(void) {
	if (id < 0)return -1;
	isOpen = true;

	int ret = EWC_Open(id, captureSize.width, captureSize.height, framerate, deviceId, MEDIASUBTYPE_RGB24);

	return ret;
}

int Camera::close(void) {
	isOpen = false;
	int ret = EWC_Close(id);
	return ret;
}

bool Camera::isOpened() {
	return this->isOpen;
}

bool Camera::isCaptured() {
	return EWC_IsCaptured(this->id);
}

void Camera::getRawFrame(cv::Mat3b& dst) {
	EWC_GetImage(this->id, dst.data);
}

void Camera::getFrame(cv::Mat3b& dst) {

	if (this->isPS4EYE) {
		cv::Mat3b frame = cv::Mat3b(this->captureSize.height, this->captureSize.width);
		getRawFrame(frame);
		dst = frame(cv::Rect(48, 0, this->captureSize.width, this->captureSize.height));
		return;
	}
	else {
		getRawFrame(dst);
		return;
	}
}

void Camera::getUndistortedFrame(cv::Mat3b& dst) {
	if (!this->isCalibrated) {
		getFrame(dst);
		return;
	}
	cv::Mat3b frame = cv::Mat3b(this->frameSize.height, this->frameSize.width);
	getFrame(frame);
	cv::undistort(frame, dst, this->matrix, this->distotion);
	return;
}

cv::Size Camera::getCaptureSize() {
	return captureSize;
}

cv::Size Camera::getFrameSize() {
	return frameSize;
}

void Camera::setCalibrateParameter(cv::Mat mtx, cv::Mat dist) {
	matrix = mtx;
	distotion = dist;
	isCalibrated = true;
}

cv::Mat Camera::getCalibrateMatrix() {
	return this->matrix;
}

cv::Mat Camera::getCalibrateDistotion() {
	return this->distotion;
}

bool Camera::isCalibrate() {
	return this->isCalibrated;
}

char* Camera::getDeviceName() {
	return EWC_GetDeviceName(this->id);
}

Camera::~Camera(void) {
	close();
}

