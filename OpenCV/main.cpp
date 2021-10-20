#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

int main(void) {

	VideoCapture capVideo;

	Mat imgFrame1;
	Mat fgMask;

	int carCount = 0;

	capVideo.open("Resources/CarsDrivingUnderBridge.mp4");

	if (!capVideo.isOpened()) {
		cout << "error reading video file" << endl << endl;
		return(0);
	}

	Ptr<BackgroundSubtractor> pBackSub;
	pBackSub = createBackgroundSubtractorMOG2();
	int frameCount = 2;
	vector<vector<Point>> cont;
	vector<Vec4i> hierarchy;
	int area, height, width, x, y;
	Mat roi;
	Rect data;
	while (capVideo.isOpened()) {
		
		capVideo >> imgFrame1;
		roi = imgFrame1(Range(40, 720), Range(0, 1280));
		pBackSub->apply(roi, fgMask);
		threshold(fgMask, fgMask, 254, 255, THRESH_BINARY);
		//finding Contours
		findContours(fgMask, cont, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
		for (int i = 0; i < cont.size(); i++) {
			area = contourArea(cont[i]);
			if (area > 500) {
				//drawContours(roi, cont, i, Scalar(0, 255, 0), 2, LINE_8, hierarchy, 0);
				data = boundingRect(cont[i]);
				rectangle(roi, Point(data.x, data.y), Point(data.x + data.width, data.y + data.height), Scalar(255, 0, 0), 2, 8, 0);
			}
		}

		//show the current frame and the fg masks
		imshow("Frame", imgFrame1);
		imshow("FG Mask", fgMask);
		imshow("ROI", roi);
		frameCount++;
		waitKey(1);
	}

	return(0);
}