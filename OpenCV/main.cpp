#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

vector<int> topleftx, toplefty, bottomrightx, bottomrighty;
int pixelsize = 50;

void findRect() {
		for (int j = 0; j < topleftx.size(); j++) {
			if (j < topleftx.size() - 1) {
				if ((topleftx[j] + pixelsize > topleftx[j + 1] && topleftx[j + 1] + pixelsize > topleftx[j] && toplefty[j] == toplefty[j + 1 ]) || (toplefty[j] + pixelsize > toplefty[j + 1] && toplefty[j + 1] + pixelsize > toplefty[j] && topleftx[j] == topleftx[j + 1])) {
					//cout << "j: " << topleftx[j] << endl;
					//cout << "j + 1: " << topleftx[j + 1] << endl;
					//cout << "1. rect-top-left: " << topleftx[j] << ", " << toplefty[j] << " ; ";
					//cout << "1. rect-bottom-right: " << bottomrightx[j] << ", " << bottomrighty[j] << endl;
					bottomrightx[j] = bottomrightx[j + 1];
					bottomrighty[j] = bottomrighty[j + 1];
					topleftx.erase(topleftx.begin() + (j + 1));
					toplefty.erase(toplefty.begin() + (j + 1));
					bottomrightx.erase(bottomrightx.begin() + (j + 1));
					bottomrighty.erase(bottomrighty.begin() + (j + 1));
				}
				else if ((bottomrightx[j] + pixelsize > bottomrightx[j + 1] && bottomrightx[j + 1] + pixelsize > bottomrightx[j] && bottomrighty[j] == bottomrighty[j + 1 ]) || (bottomrighty[j] + pixelsize > bottomrighty[j + 1] && bottomrighty[j + 1] + pixelsize > bottomrighty[j] && bottomrightx[j] == bottomrightx[j + 1])) {
					//cout << "2. rect-top-left: " << topleftx[j] << ", " << toplefty[j] << " ; ";
					//cout << "2. rect-bottom-right: " << bottomrightx[j] << ", " << bottomrighty[j] << endl;
					topleftx[j] = topleftx[j + 1];
					toplefty[j] = toplefty[j + 1];
					topleftx.erase(topleftx.begin() + (j + 1));
					toplefty.erase(toplefty.begin() + (j + 1));
					bottomrightx.erase(bottomrightx.begin() + (j + 1));
					bottomrighty.erase(bottomrighty.begin() + (j + 1));
				}
				else if ((topleftx[j] + pixelsize > bottomrighty[j + 1] && bottomrighty[j + 1] + pixelsize > topleftx[j])) {
					topleftx[j] = bottomrightx[j + 1];
					toplefty[j] = bottomrighty[j + 1];
					topleftx.erase(topleftx.begin() + (j + 1));
					toplefty.erase(toplefty.begin() + (j + 1));
					bottomrightx.erase(bottomrightx.begin() + (j + 1));
					bottomrighty.erase(bottomrighty.begin() + (j + 1));
				}
				else if (toplefty[j] + pixelsize > bottomrightx[j + 1] && bottomrightx[j + 1] + pixelsize > toplefty[j]) {
					bottomrightx[j] = topleftx[j + 1];
					bottomrighty[j] = toplefty[j + 1];
					topleftx.erase(topleftx.begin() + (j + 1));
					toplefty.erase(toplefty.begin() + (j + 1));
					bottomrightx.erase(bottomrightx.begin() + (j + 1));
					bottomrighty.erase(bottomrighty.begin() + (j + 1));
				}
			}
	}

	cout << "vetcor size: " << topleftx.size() << endl;

}

int main(void) {

	cv::VideoCapture capVideo;

	cv::Mat imgFrame1;
	cv::Mat imgFrame2;

	int carCount = 0;

	capVideo.open("Resources/CarsDrivingUnderBridge.mp4");

	if (!capVideo.isOpened()) {
		cout << "error reading video file" << endl << endl;
		return(0);
	}

	capVideo.read(imgFrame1);
	capVideo.read(imgFrame2);


	int frameCount = 2;

	while (capVideo.isOpened()) {

		Mat imgFrame1Copy = imgFrame1.clone();
		Mat imgFrame2Copy = imgFrame2.clone();

		Mat imgDifference;
		Mat imgThresh;

		cvtColor(imgFrame1Copy, imgFrame1Copy, COLOR_BGRA2GRAY);
		cvtColor(imgFrame2Copy, imgFrame2Copy, COLOR_BGRA2GRAY);
		//imshow("imgFrame1", imgFrame1Copy);
		//imshow("imgFrame2", imgFrame2Copy);
		GaussianBlur(imgFrame1Copy, imgFrame1Copy, Size(5, 5), 0);
		GaussianBlur(imgFrame2Copy, imgFrame2Copy, Size(5, 5), 0);

		absdiff(imgFrame1Copy, imgFrame2Copy, imgDifference);

		threshold(imgDifference, imgThresh, 30, 255.0, THRESH_BINARY_INV);

		imshow("imgThresh", imgThresh);

		Mat imgThreshCopy = imgThresh.clone();

		imgFrame2Copy = imgFrame2.clone();

		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		findContours(imgThresh, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
		for (int i = 0; i < contours.size(); i++) {
			//cout << "contours: " << contours[i].size() << endl;
			//cout << "hierarchy: " << hierarchy[i] << endl;
			drawContours(imgFrame2Copy, contours, i, Scalar(255, 0, 255), 2, LINE_8, hierarchy, 0);
			Rect rect = boundingRect(Mat(contours[i]));
			//rectangle(imgFrame2Copy, rect.tl(), rect.br(), Scalar(255, 0, 0), 2, 8, 0);
			/*cout << "rect-top-left: " << rect.tl() << " ; ";
			cout << "rect-bottom-right: " << rect.br() << endl;*/
			topleftx.push_back(rect.tl().x);
			toplefty.push_back(rect.tl().y);
			bottomrightx.push_back(rect.br().x);
			bottomrighty.push_back(rect.br().y);
		}
		cout << "------------------------------------------------------------------------------------------- " << endl << endl << endl;
		cout << "vetcor1 size: " << topleftx.size() << endl;
		int size = topleftx.size();
		for (int i = 0; i < size; i++) {
			findRect();
		}
		
		for (int i = 0; i < topleftx.size(); i++) {
			cout << "2. rect-top-left: " << topleftx[i] << ", " << toplefty[i] << " ; ";
			cout << "2. rect-bottom-right: " << bottomrightx[i] << ", " << bottomrighty[i] << endl;
			rectangle(imgFrame2Copy, Point(topleftx[i], toplefty[i]), Point(bottomrightx[i], bottomrighty[i]), Scalar(255, 0, 0), 2, 8, 0);
		}
		topleftx.clear();
		toplefty.clear();
		bottomrightx.clear();
		bottomrighty.clear();
		cout << "------------------------------------------------------------------------------------------- " << endl << endl << endl;

		imshow("imgFrame2Copy", imgFrame2Copy);
		waitKey(0);

		imgFrame1 = imgFrame2.clone();

		if ((capVideo.get(1) + 1) < capVideo.get(7)) {
			capVideo.read(imgFrame2);
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