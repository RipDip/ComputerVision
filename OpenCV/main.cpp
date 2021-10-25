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
	Mat roi, element, element2, erosion_dst, dilation_dst;
	Rect data;
	while (capVideo.isOpened()) {
		
		capVideo >> imgFrame1;
		roi = imgFrame1(Range(40, 720), Range(0, 1280));
		pBackSub->apply(roi, fgMask);
		threshold(fgMask, fgMask, 254, 255, THRESH_BINARY);
		element = getStructuringElement(MORPH_RECT, Size(2, 2));
		element2 = getStructuringElement(MORPH_RECT, Size(4, 4));
		erode(fgMask, erosion_dst, element);
		dilate(erosion_dst, dilation_dst, element2);
		//finding Contours
		findContours(dilation_dst, cont, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
		for (int i = 0; i < cont.size(); i++) {
			area = contourArea(cont[i]);
			if (area > 500) {
				//drawContours(roi, cont, i, Scalar(0, 255, 0), 2, LINE_8, hierarchy, 0);
				data = boundingRect(cont[i]);
				rectangle(roi, Point(data.x, data.y), Point(data.x + data.width, data.y + data.height), Scalar(255, 0, 0), 2, 8, 0);
			}
		}

		//show the current frame and the fg masks
		imshow("Frame", dilation_dst);
		imshow("FG Mask", fgMask);
		imshow("ROI", roi);
		if ((capVideo.get(1) + 1) < capVideo.get(7)) {
			capVideo.read(imgFrame1);
		}
		else {
			cout << "end of video\n";
			break;
		}
		frameCount++;
		waitKey(1);
	}

	return(0);
}