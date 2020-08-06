
#pragma comment (lib, "ws2_32.lib")

#include "camera.h"
#include "SocketSender.h"
#include <iostream>
#include <string>


int displayMenu(void) {

	int input = 0;

	std::cout << "\n----------MAIN MENU----------\nPlease enter a number.\n" << std::endl;
	std::cout << "1. Camera Caribration" << std::endl;
	std::cout << "2. Camera Setup" << std::endl;
	std::cout << "3. Start Capture" << std::endl;
	std::cout << "4. Exit\n" << std::endl;

	while (true) {
		std::cout << ">>" << std::flush;
		std::cin >> input;

		if (input >= 1 && input <= 3 && std::cin.good()) {
			std::cout << "You choose : " << input << std::endl;
			break;
		}
		else {
			std::cin.clear();
			std::cin.ignore();
			std::cout << "Invalid input. please retry." << std::endl;
		}
	}
	return input;
}

int caribration(Camera& cam) {
	if (!cam.isOpened()) {
		int ret = cam.open();
		if (ret != 0) {
			std::cout << "[ERROR] Caribration Failed. camera can't open." << std::endl;
			return -1;
		}
	}
	cv::Mat3b                   grabImg = cv::Mat_<cv::Vec3b>(cam.getCaptureSize().height, cam.getCaptureSize().width);
	cv::Mat                     gray;

	cv::Size                    chessboardPattern(10, 7);
	std::vector<cv::Point2f>    chessboardCenters;

	std::vector<std::vector<cv::Point3f>> objectPoints;
	std::vector<std::vector<cv::Point2f>> imgPoints;
	std::vector<cv::Point3f> obj;
	for (int j = 0; j < chessboardPattern.width * chessboardPattern.height; j++) {
		obj.push_back(cv::Point3f(j / chessboardPattern.width, j % chessboardPattern.width, 0.0f));
	}

	for (;;) {
		bool endFlag = false;
		if (cam.isCaptured()) {
			cam.getFrame(grabImg);

			cv::imshow(cam.getDeviceName(), grabImg);

			bool chessboardIsFound = false;
			switch (cv::waitKey(1)) {
			case 0x1b:

				endFlag = true;
				break;

			case 0x20:

				//referenceImgs.push_back(grabImg);

				cv::cvtColor(grabImg, gray, cv::COLOR_BGR2GRAY);
				chessboardIsFound = cv::findChessboardCorners(gray, chessboardPattern, chessboardCenters, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);

				if (chessboardIsFound) {
					cv::cornerSubPix(gray, chessboardCenters, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));
					objectPoints.push_back(obj);
					imgPoints.push_back(chessboardCenters);

					cv::drawChessboardCorners(grabImg, chessboardPattern, chessboardCenters, true);
					cv::imshow("detected chessboard", grabImg);

					fprintf(stdout, "Recorded. Count:%d\n", imgPoints.size());
				}
				else {
					fprintf(stdout, "Chessboard not found.Please Retry. Count:%d\n", imgPoints.size());
				}

				break;

			case 0x72:

				if (imgPoints.size() == 0)break;

				objectPoints.pop_back();
				imgPoints.pop_back();
				fprintf(stdout, "Deleted. Count:%d\n", imgPoints.size());

				break;
			}

			if (endFlag)break;
		}
	}

	std::cout << "Start Calculation." << std::endl;

	std::vector<cv::Mat> rvecs, tvecs;
	cv::Mat mtx(3, 3, CV_64F);
	cv::Mat dist(8, 1, CV_64F);
	cv::calibrateCamera(objectPoints, imgPoints, grabImg.size(), mtx, dist, rvecs, tvecs);

	cam.setCalibrateParameter(mtx, dist);

	cam.close();

}

int main(int argc, char* argv[])
{
	std::cout << "-----Motion Grabber v0.1-----\n\n" << std::endl;

	int mode = 0;
	if (argc == 1) {
		std::cout << "[INFO] Normal Startup.\n" << std::endl;
		mode = displayMenu();
	}

	Camera eye1(0, 0, Camera::RESOLUTION_M, false, 30);

	if (mode == 1) {
		caribration(eye1);
	}

	SocketSender sockSender(50007, "127.0.0.1");
	sockSender.SS_startup();
	sockSender.SS_connect();


	int ret = eye1.open();
	if (ret != 0) {
		fprintf(stderr, "[ERROR] EWC_Open Failed. Code[ %d ]\n", ret);
		std::exit(1);
	}
	char* deviceName = eye1.getDeviceName();
	if (deviceName != nullptr) {
		std::cout << "DeviceName : " << eye1.getDeviceName() << std::endl;
	}


	cv::Size eye1FrameSize = eye1.getFrameSize();
	cv::Mat_<cv::Vec3b> grabImg = cv::Mat_<cv::Vec3b>(eye1.getCaptureSize().height, eye1.getCaptureSize().width);

	const cv::aruco::PREDEFINED_DICTIONARY_NAME dictionary_name = cv::aruco::DICT_4X4_50;
	cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(dictionary_name);

	std::vector<int> markerIds;
	std::vector<std::vector<cv::Point2f>> markerCorners;
	cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();

	float markerSize = 10.0f;
	int cam = 1;

	sockSender.SS_setBlockingMode(false);

	for (;;) {

		int ret = sockSender.SS_receive();
		if (ret < 1) {
			std::cout << sockSender.SS_getBuffer() << std::endl;
		}

		if (eye1.isCaptured()) {
			eye1.getUndistortedFrame(grabImg);

			cv::aruco::detectMarkers(grabImg, dictionary, markerCorners, markerIds, parameters);
			cv::aruco::drawDetectedMarkers(grabImg, markerCorners, markerIds);

			
			std::string data = "";

			data += std::to_string(cam) + ',';
			data += std::to_string(markerIds.size());

			for (int i = 0; i < markerIds.size(); i++) {
				data += ',';

				data += std::to_string(markerIds[i]);

				for (int j = 0; j < 4; j++) {
					data += ',';
					data += std::to_string(markerCorners[i][j].x) + ':' + std::to_string(markerCorners[i][j].y);
				}

			}

			std::cout << data << std::endl;
			sockSender.SS_send(data);

			//cv::Mat rotImg(cv::Size(grabImg.size().height, grabImg.size().width), grabImg.type(), cv::Scalar(0, 0, 0));
			//cv::transpose(grabImg, rotImg);

			cv::imshow("CameraImage", grabImg);
		}
		if (cv::waitKey(1) == 0x1b) { // ESC
			break;
		}
	}
	eye1.close();
	sockSender.SS_clean();

	return 0;
}