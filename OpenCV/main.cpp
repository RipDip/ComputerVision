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

vector<Rect> datacar;
vector<int> datacartime;
int datacarhealth = 3;
int maxy = 500;
int countCar = 0;
float imgLineY;

void countdatacartime() {
	for (int i = 0; i < datacartime.size(); i++) {
		datacartime[i] = datacartime[i] + 1;
		if (datacartime[i] >= datacarhealth) {
			datacar.erase(datacar.begin() + i);
			datacartime.erase(datacartime.begin() + i);
		}

		else if (datacar[i].tl().y < imgLineY) {
			cout << "Outside" << endl;
			datacar.erase(datacar.begin() + i);
			datacartime.erase(datacartime.begin() + i);
		}
	}
}

bool inRange(int low, int high, int x){
	return (low <= x && x <= high);
}

void newRect(Rect data) {
	int range = 50;
	bool newcar = true;
	//first ckeck if the car is new
	//cout << "Data x: " << data.x << endl;
	//cout << "Data y: " << data.y << endl;
	cout << "Count Car: " << countCar << endl;
	for (int i = 0; i < datacar.size(); i++) {
		//cout << "Datacar x: " << datacar[i].x << " von Zahl: " << i << endl;
		//cout << "Datacar y: " << datacar[i].y << " von Zahl: " << i << endl;
		
		if ((inRange(datacar[i].x - range, datacar[i].x + range, data.x) && inRange(datacar[i].y - range, datacar[i].y + range, data.y)) ) {
			cout << "Not a new car" << endl;
			datacar[i] = data;
			datacartime[i] = 0;
			newcar = false;
			break;
		}
		//if (datacar[i].br().x > data.br().x && datacar[i].br().y > data.br().y && datacar[i].tl().x < data.tl().x && datacar[i].tl().y < data.tl().y) {
		if (data.br().x < datacar[i].br().x && data.br().y < datacar[i].br().y && data.tl().x > datacar[i].tl().x && data.tl().y > datacar[i].tl().y) {
			cout << "Inside a car" << endl;
			newcar = false;
			break;
		}
		if (datacar[i].tl().y < maxy) {
			cout << "Outside" << endl;
			datacar.erase(datacar.begin() + i);
			datacartime.erase(datacartime.begin() + i);
			break;
		}

	}

	//than new car add to datacar
	if (data.x < 570 && data.y > 450 && data.y < 720 && newcar) {
		cout << "New car" << endl;
		countCar++;
		datacar.push_back(data);
		datacartime.push_back(0);
	}
	
}

void newRect2(Rect data) {
	int range = 50;
	bool newcar = true;
	//first ckeck if the car is new
	//cout << "Data x: " << data.x << endl;
	//cout << "Data y: " << data.y << endl;
	cout << "Count Car: " << countCar << endl;
	for (int i = 0; i < datacar.size(); i++) {
		//cout << "Datacar x: " << datacar[i].x << " von Zahl: " << i << endl;
		//cout << "Datacar y: " << datacar[i].y << " von Zahl: " << i << endl;

		if ((inRange(datacar[i].x - range, datacar[i].x + range, data.x) && inRange(datacar[i].y - range, datacar[i].y + range, data.y))) {
			cout << "Not a new car" << endl;
			datacar[i] = data;
			datacartime[i] = 0;
			newcar = false;
			break;
		}
		//if (datacar[i].br().x > data.br().x && datacar[i].br().y > data.br().y && datacar[i].tl().x < data.tl().x && datacar[i].tl().y < data.tl().y) {
		if (data.br().x < datacar[i].br().x && data.br().y < datacar[i].br().y && data.tl().x > datacar[i].tl().x && data.tl().y > datacar[i].tl().y) {
			cout << "Inside a car" << endl;
			newcar = false;
			break;
		}

	}

	//than new car add to datacar
	if (data.tl().y > imgLineY && newcar) {
		cout << "New car" << endl;
		countCar++;
		datacar.push_back(data);
		datacartime.push_back(0);
	}

}

int main(void) {

	VideoCapture capVideo;

	Mat imgFrame1;
	Mat fgMask, fgMask2, fgMask3;
	int option = 4;

	//int carCount = 0;

	//capVideo.open("Resources/CarsDrivingUnderBridge.mp4");
	//capVideo.open("Resources/trafficCrossing.mp4");
	//capVideo.open("Resources/HighwayTraffic.mp4");
	capVideo.open("Resources/HighwayTraffic2.mp4");
	//capVideo.open("Resources/HighwayTraffic3.mp4");

	if (!capVideo.isOpened()) {
		cout << "error reading video file" << endl << endl;
		return(0);
	}

	Ptr<BackgroundSubtractor> pBackSub;
	pBackSub = createBackgroundSubtractorMOG2(500, 16, false);
	int frameCount = 3;
	vector<vector<Point>> cont, cont2;
	vector<Vec4i> hierarchy, hierarchy2;
	vector<Mat> result_planes, result_norm_planes;
	int area, height, width, x, y;
	Mat roi, tmproi, element, element2, erosion_dst, dilation_dst, show;
	Rect data;

	while (capVideo.isOpened()) {
		capVideo >> imgFrame1;
		//roi = imgFrame1(Range(40, 720), Range(0, 1280));
		roi = imgFrame1(Range(0, 720), Range(0, 1280));
		imgLineY = roi.size().height / 2.85;
		show = roi(Rect(0, 450, 570, 270)); //x,y, width 1280 x, height 720 y
		//roi = imgFrame1(Range(200, 720), Range(0, 600));
		fgMask2 = roi.clone();
		fgMask3 = roi.clone();
		
		
		Mat rgbchannel[3], result, result_norm;

		split(fgMask3, rgbchannel);
		element = getStructuringElement(MORPH_RECT, Size(7, 7));
		for (Mat rgb : rgbchannel) {
			dilate(rgb, fgMask3, element);
			medianBlur(fgMask3, fgMask3, 25);
			Mat diff_img, norm_img;
			absdiff(rgb, fgMask3, diff_img);
			diff_img = 255 - diff_img;
			normalize(diff_img, norm_img, 0, 255, NORM_MINMAX, CV_8UC1);
			result_planes.push_back(diff_img);
			result_norm_planes.push_back(norm_img);
		}
		merge(result_planes, result);
		merge(result_norm_planes, result_norm);
		result_planes.clear();
		result_norm_planes.clear();

		imshow("result", result);
		imshow("result_norm", result_norm);
		pBackSub->apply(result_norm, fgMask);
		threshold(fgMask, fgMask, 254, 255, THRESH_BINARY);
		element = getStructuringElement(MORPH_RECT, Size(2, 2));
		element2 = getStructuringElement(MORPH_RECT, Size(9, 9));
		erode(fgMask, erosion_dst, element);
		dilate(erosion_dst, dilation_dst, element2);
		erode(dilation_dst, dilation_dst, element);
		//medianBlur(dilation_dst, dilation_dst, 3);
		//finding Contours
		findContours(dilation_dst, cont, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

		if (option == 0) {
			for (int i = 0; i < cont.size(); i++) {
				area = contourArea(cont[i]);
				if (area > 700 && area < roi.size().width * roi.size().height / 10) {
					//drawContours(roi, cont, i, Scalar(0, 255, 0), 2, LINE_8, hierarchy, 0);
					data = boundingRect(cont[i]);

					//cout << "First Data x: " << data.x << endl;
					//cout << "First Data y: " << data.y << endl;
					//rectangle(roi, Point(data.x, data.y), Point(data.x + data.width, data.y + data.height), Scalar(255, 0, 255), 2, 8, 0);
					newRect(data);
					if (data.x < 570 && data.y > 350 && data.y < 720) {

						//cout << "Data x: " << data.x << endl;
						//cout << "Data y: " << data.y << endl;
						//rectangle(roi, Point(data.x, data.y), Point(data.x + data.width, data.y + data.height), Scalar(255, 0, 0), 2, 8, 0);
					}

				}
			}
			countdatacartime();
			for (int i = 0; i < datacar.size(); i++) {
				rectangle(roi, Point(datacar[i].x, datacar[i].y), Point(datacar[i].x + datacar[i].width, datacar[i].y + datacar[i].height), Scalar(255, 0, 0), 2, 8, 0);
			}
			cout << "-------------------------------------" << endl;
		}
		else if (option == 1) {
			for (unsigned int i = 0; i < cont.size(); i++) {
				area = contourArea(cont[i]);
				if (area > 500) {
					//drawContours(roi, cont, i, Scalar(0, 255, 0), 2, LINE_8, hierarchy, 0);
					data = boundingRect(cont[i]);
					rectangle(roi, Point(data.x, data.y), Point(data.x + data.width, data.y + data.height), Scalar(255, 0, 0), 2, 8, 0);
				}
			}
		}
		//approach to detect line
		else if (option == 3) {
			Mat gray, gray2;
			cvtColor(fgMask2, gray, COLOR_BGR2GRAY);
			threshold(gray, gray, 127, 255, 0);
			gray2 = gray.clone();
			pBackSub->apply(gray, fgMask);

			threshold(fgMask, fgMask, 254, 255, THRESH_BINARY_INV);
			element = getStructuringElement(MORPH_RECT, Size(2, 2));
			element2 = getStructuringElement(MORPH_RECT, Size(9, 9));
			erode(fgMask, erosion_dst, element);
			dilate(erosion_dst, dilation_dst, element2);
			findContours(dilation_dst, cont2, hierarchy2, RETR_TREE, CHAIN_APPROX_SIMPLE);
			for (int i = 0; i < cont2.size(); i++) {
				area = contourArea(cont2[i]);
				if (area > 150 && area < 500) {
					data = boundingRect(cont2[i]);
					rectangle(roi, Point(data.x, data.y), Point(data.x + data.width, data.y + data.height), Scalar(255, 0, 0), 2, 8, 0);
				}
			}
		}
		else if (option == 4) {
			
			for (unsigned int i = 0; i < cont.size(); i++) {
				area = contourArea(cont[i]);
				if (area > 2000) {
					//drawContours(roi, cont, i, Scalar(0, 255, 0), 2, LINE_8, hierarchy, 0);
					data = boundingRect(cont[i]);
					newRect2(data);
					//rectangle(roi, Point(data.x, data.y), Point(data.x + data.width, data.y + data.height), Scalar(255, 0, 0), 2, 8, 0);
				}
			}
			countdatacartime();
			line(roi, Point(0, imgLineY), Point(roi.size().width, imgLineY), 1, 8, 0);
			for (int i = 0; i < datacar.size(); i++) {
				rectangle(roi, Point(datacar[i].x, datacar[i].y), Point(datacar[i].x + datacar[i].width, datacar[i].y + datacar[i].height), Scalar(255, 0, 0), 2, 8, 0);
			}
			cout << "-------------------------------------" << endl;
		}

		
		//show the current frame and the fg masks
		//imshow("show", show);
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