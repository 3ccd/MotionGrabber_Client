#pragma once

//#define DEBUG

#if defined(DEBUG)
#pragma comment (lib, "opencv_world420d.lib")
#else
#pragma comment (lib, "opencv_world420.lib")
#endif

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>


#include <iostream>


class Camera {

private:

	int id, deviceId;
	int resLevel;
	cv::Size captureSize;
	cv::Size frameSize;
	bool isPS4EYE;
	double framerate;

	bool isOpen;
	bool isCalibrated;

	cv::Mat matrix;
	cv::Mat distotion;


public:
	const static int RESOLUTION_H = 0;
	const static int RESOLUTION_M = 1;

	Camera(const int id, const int deviceId, int resLevel, bool isPS4EYE, double framerate);

	int open(void);

	int close(void);

	bool isOpened();

	bool isCaptured();

	void getRawFrame(cv::Mat3b& dst);

	void getFrame(cv::Mat3b& dst);

	void getUndistortedFrame(cv::Mat3b& dst);

	cv::Size getCaptureSize();

	cv::Size getFrameSize();

	void setCalibrateParameter(cv::Mat mtx, cv::Mat dist);

	cv::Mat getCalibrateMatrix();

	cv::Mat getCalibrateDistotion();

	bool isCalibrate();

	char* getDeviceName();

	~Camera(void);

};