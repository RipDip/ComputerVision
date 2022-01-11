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
vector<int> datacarcount;
vector<vector<Point>> contoursline;
int datacarhealth = 2;
int maxy = 500;
int countCar = 0;
float imgLineY;
bool condition = true;
Point P1;
Point P2;
Point P3;
Point start, ende;
Rect2i ROIR(0, 0, 0, 0);
bool clicked = false;
bool vertical = false;

VideoCapture capVideo;
VideoCapture strcCap;

Mat imgFrame1;
Mat strucFrame;
Mat strucCopy;
Mat fgMask;

void showRectangle() {
	strucCopy = strucFrame.clone();
	rectangle(strucFrame, ROIR, Scalar(0, 255, 0), 1, 8, 0);
	imshow("Struc", strucFrame);
	strucFrame = strucCopy.clone();
}

void showLine() {
	strucCopy = strucFrame.clone();
	line(strucFrame, start, ende, Scalar(0, 255, 0), 2);
	imshow("CounterLine", strucFrame);
	strucFrame = strucCopy.clone();
}

static void onCounter(int event, int x, int y, int, void*) {
	switch (event) {
		case  EVENT_LBUTTONDOWN:
			P3.x = x;
			P3.y = y;
			cout << "P3:(" << P3.x << ", " << P3.y << ")" << endl;
			cout << "Window:(" << strucFrame.size().width << ", " << strucFrame.size().height << ")" << endl;
			break;

		case EVENT_RBUTTONDOWN:
			vertical = !vertical;

		default:
			break;
	}
	if (vertical) {
		start.x = P3.x;
		ende.y = strucFrame.size().height;
		ende.x = P3.x;
		start.y = 0;
	}
	else {
		start.y = P3.y;
		ende.y = P3.y;
		start.x = 0;
		ende.x = strucFrame.size().width;
	}
	showLine();
}

static void onMouse(int event, int x, int y, int, void*){
	switch (event) {

		case  EVENT_LBUTTONDOWN:
			clicked = true;
			P1.x = x;
			P1.y = y;
			cout << "P1:(" << P1.x << ", " << P1.y << ")" << endl;
			break;

		case  EVENT_LBUTTONUP:
			P2.x = x;
			P2.y = y;
			clicked = false;
			condition = false;
			cout << "P2:(" << P2.x << ", " << P2.y << ")" << endl;
			break;

		case EVENT_MOUSEMOVE:
			if (clicked) {
				P2.x = x;
				P2.y = y;
			}
			break;

		default:
			break;
	}
	if (clicked) {
		if (P1.x > P2.x) {
			ROIR.x = P2.x;
			ROIR.width = P1.x - P2.x;
		}
		else {
			ROIR.x = P1.x;
			ROIR.width = P2.x - P1.x;
		}

		if (P1.y > P2.y) {
			ROIR.y = P2.y;
			ROIR.height = P1.y - P2.y;
		}
		else {
			ROIR.y = P1.y;
			ROIR.height = P2.y - P1.y;
		}

	}
	showRectangle();
}

void countdatacartime() {
	for (int i = 0; i < datacartime.size(); i++) {
		datacartime[i] = datacartime[i] + 1;
		if ((datacar[i].y + datacar[i].height / 2) < P3.y && datacarcount[i] == 0) {
			countCar++;
			datacarcount[i] = 1;
			cout << "Car was counted" << endl;
		}

		if (datacartime[i] >= datacarhealth) {
			cout << "Car was not a real car" << endl;
			datacar.erase(datacar.begin() + i);
			datacartime.erase(datacartime.begin() + i);
			datacarcount.erase(datacarcount.begin() + i);
		}

		/*else if (datacar[i].br().y > ROIR.y + ROIR.height || datacar[i].br().y < ROIR.height) {
			cout << "Outside" << endl;
			datacar.erase(datacar.begin() + i);
			datacartime.erase(datacartime.begin() + i);
			datacarcount.erase(datacarcount.begin() + i);
		}*/
	}
}

bool inRange(int low, int high, int x){
	return (low <= x && x <= high);
}


void newRect(Rect data) {
	int range = 60;
	bool newcar = true;
	//first ckeck if the car is new
	cout << "Count Car: " << countCar << endl;
	for (int i = 0; i < datacar.size(); i++) {
		if ((inRange(datacar[i].x - range, datacar[i].x + range, data.x) && inRange(datacar[i].y - range, datacar[i].y + range, data.y))) {
			cout << "Not a new car" << endl;
			datacar[i] = data;
			datacartime[i] = 0;
			newcar = false;
			break;
		}
		if (data.br().x < datacar[i].br().x && data.br().y < datacar[i].br().y && data.tl().x > datacar[i].tl().x && data.tl().y > datacar[i].tl().y) {
			cout << "Inside a car" << endl;
			newcar = false;
			break;
		}

	}
	//midpoint from car = Point(data.x + data.width/2, data.y + data.height/2)

	//than new car add to datacar
	if (newcar) {
		cout << "New car" << endl;
		datacar.push_back(data);
		datacartime.push_back(0);
		datacarcount.push_back(0);
	}
}

inline double Det(double a, double b, double c, double d){
	return a * d - b * c;
}

bool LineLineIntersect(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, double& ixOut, double& iyOut) {
	double detL1 = Det(x1, y1, x2, y2);
	double detL2 = Det(x3, y3, x4, y4);
	double x1mx2 = x1 - x2;
	double x3mx4 = x3 - x4;
	double y1my2 = y1 - y2;
	double y3my4 = y3 - y4;

	double denom = Det(x1mx2, y1my2, x3mx4, y3my4);
	if (denom == 0.0){
		ixOut = NAN;
		iyOut = NAN;
		return false;
	}

	double xnom = Det(detL1, x1mx2, detL2, x3mx4);
	double ynom = Det(detL1, y1my2, detL2, y3my4);
	ixOut = xnom / denom;
	iyOut = ynom / denom;
	if (!isfinite(ixOut) || !isfinite(iyOut))
		return false;

	return true;
}

Mat detectStreet(Mat roi) {
	Mat static mask1;
	Mat static street;
	Mat matlines = roi.clone();
	vector<Vec4i> static lines;
	vector<Vec4i> static tmplines;
	vector<Vec4i> static tmplines2;
	
	vector<Point> tmpcontours;
	bool static firstframe = false;
	double x, y;

	if (!firstframe) {
		cvtColor(matlines, mask1, COLOR_RGB2GRAY);
		Canny(mask1, mask1, 50, 150);

		HoughLinesP(mask1, lines, 1, CV_PI / 180, 300, 100, 250);
		firstframe = true;
		for (size_t i = 0; i < lines.size(); i++) {
			Vec4i l = lines[i];
			double x, y;
			LineLineIntersect(l[0], l[1], l[2], l[3], 0, imgLineY, roi.size().width, imgLineY, x, y);
			if ((l[0] > 540 && l[0] < 740) || (l[2] > 1200 && l[3] > 260 && l[3] > 460) || (l[1] > 600)) {
				//line(roi, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(125, 0, 125), 3, LINE_AA);
				if (l[1] < imgLineY) {
					tmplines.push_back(l);
				}
				else {
					tmplines2.push_back(l);
				}
			}
		}
	}
	if (tmplines.size() > 0) {
		LineLineIntersect(tmplines[0][0], tmplines[0][1], tmplines[0][2], tmplines[0][3], 0, imgLineY, roi.size().width, imgLineY, x, y);
		line(roi, Point(x, imgLineY), Point(tmplines[0][2], tmplines[0][3]), Scalar(0, 255, 0), 3, LINE_AA);
		int tmpx = x;
		float sizeline = 0;
		float tmpsizeline = 0;
		int index;
		for (int i = 0; i < tmplines2.size(); i++) {
			tmpsizeline = tmplines2[i][1] - tmplines2[i][3];
			if (tmpsizeline > sizeline && tmplines2[i][0] != tmplines2[i][2]) {
				sizeline = tmpsizeline;
				index = i;
			}
		}
		LineLineIntersect(tmplines2[index][0], tmplines2[index][1], tmplines2[index][2], tmplines2[index][3], 0, imgLineY, roi.size().width, imgLineY, x, y);
		line(roi, Point(tmplines2[index][0], tmplines2[index][1]), Point(x, imgLineY), Scalar(255, 255, 0), 3, LINE_AA);

		//street = roi(Rect(tmplines2[0][0], imgLineY, 1280 - tmplines2[0][0], roi.size().height - imgLineY)); //x,y, width 1280 x, height 720 y
		tmpcontours.push_back(Point(tmplines2[0][0], tmplines2[0][1]));
		tmpcontours.push_back(Point(x - 200, imgLineY));

		tmpcontours.push_back(Point(tmpx + 200, imgLineY));
		tmpcontours.push_back(Point(tmplines[0][2], tmplines[0][3]));
		contoursline.push_back(tmpcontours);
	}

	
	/*imshow("Mask1", mask1);
	imshow("roi2", roi);*/
	cout << "-------------------------------------" << endl;
	return roi;
}

string openVideo() {
	string path;
	string videos[6] = { "CarsDrivingUnderBridge.mp4", "HighwayTraffic2.mp4", "HighwayTraffic3.mp4" , "nighthighway.mp4" , "nightvideo.mp4" , "rainvideo.mp4"};
	int choise = 0;
	int counter = 1;
	string tmp;
	bool endwhile = false;
	while (!endwhile) {
		cout << "Which Video do you want to use?" << endl;
		cout << "You have the choise between 1 and 6" << endl;
		for (string &video : videos) {
			cout <<  counter <<". " << video << endl;
			counter++;
		}
		counter = 0;
		cin >> tmp;
		istringstream(tmp) >> choise;
		if (choise < 1 || choise > 6) {
			cout << "Error" << endl;
		}
		else {
			endwhile = true;
		}
	}
	choise--;
	videos[choise] = "Resources/" + videos[choise];
	cout << "Path: " << videos[choise] << endl;
	path = videos[choise];

	return path;
}

bool askActiveLinie() {
	int choise = 0;
	string tmp;
	bool endwhile = false;
	bool activeline;
	while (!endwhile) {
		cout << "Do you want lines?" << endl;
		cout << "You have the choise between 0 and 1" << endl;
		cout << "0 for detect line off and 1 for detect line on" << endl;
		cin >> tmp;
		istringstream(tmp) >> choise;
		if (choise < 0 || choise > 1) {
			cout << "Error" << endl;
		}
		else {
			endwhile = true;
			if (choise == 0) {
				activeline = false;
			}
			else {
				activeline = true;
			}
		}
	}
	return activeline;
}

Mat histogramm(Mat imgFrame1) {
	/*vector<Mat> bgr_planes;
	split(imgFrame1, bgr_planes);
	int histSize = 256;
	float range[] = { 0, 256 };
	const float* histRange[] = { range };
	bool uniform = true, accumulate = false;
	Mat b_hist, g_hist, r_hist;
	calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, histRange, uniform, accumulate);
	calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, histRange, uniform, accumulate);
	calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, histRange, uniform, accumulate);
	int hist_w = 512, hist_h = 400;
	int bin_w = cvRound((double)hist_w / histSize);
	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
	normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	for (int i = 1; i < histSize; i++)
	{
		line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
			Point(bin_w * (i), hist_h - cvRound(b_hist.at<float>(i))),
			Scalar(255, 0, 0), 2, 8, 0);
		line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(g_hist.at<float>(i - 1))),
			Point(bin_w * (i), hist_h - cvRound(g_hist.at<float>(i))),
			Scalar(0, 255, 0), 2, 8, 0);
		line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
			Point(bin_w * (i), hist_h - cvRound(r_hist.at<float>(i))),
			Scalar(0, 0, 255), 2, 8, 0);
		b_hist.at<float>(i);
		cout << "Blue.x1: " << bin_w * (i - 1) << " Blue.y1: " << hist_h - cvRound(b_hist.at<float>(i - 1)) << " Blue.x2: " << bin_w * (i) << " Blue.y2: " << hist_h - cvRound(b_hist.at<float>(i)) << endl;
	}
	imshow("calcHist Demo", histImage);*/
	// calculate histogram of the image
	Mat img, hist, weighted_hist;
	int hsize = 256;
	float range[] = { 0, 256 };
	const float* histRange[] = { range };
	Mat tmpFrame1;
	cvtColor(imgFrame1, tmpFrame1, COLOR_BGRA2GRAY);
	calcHist(&tmpFrame1, 1, 0, Mat(), hist, 1, &hsize, histRange, true, false);

	// calculate weighted histogram
	weighted_hist = hist / sum(hist);

	// calculate cumulative histogram
	Mat acc_hist = Mat::zeros(weighted_hist.size(), weighted_hist.type());
	acc_hist.at<float>(0) = weighted_hist.at<float>(0);
	for (int i = 1; i < 256; i++)
	{
		acc_hist.at<float>(i) = weighted_hist.at<float>(i) + acc_hist.at<float>(i - 1);
	}
	acc_hist = acc_hist * 255;

	// Mapping
	Mat imgClone = Mat::zeros(tmpFrame1.size(), CV_32FC1);
	tmpFrame1.convertTo(imgClone, CV_32FC1);
	Mat output = Mat::zeros(tmpFrame1.size(), CV_32FC1);
	for (int m = 0; m < tmpFrame1.rows; m++)
	{
		for (int n = 0; n < tmpFrame1.cols; n++)
		{
			output.at<float>(m, n) = acc_hist.at<float>(imgClone.at<float>(m, n));
		}
	}

	// quantize output
	output.convertTo(output, CV_8UC1);
	namedWindow("output", WINDOW_AUTOSIZE);
	imshow("output", output);
	return imgFrame1;
}

int main(void) {
	bool activeline = askActiveLinie();
	string path = openVideo();
	//int carCount = 0;
	
	capVideo.open(path);
	//capVideo.open("Resources/CarsDrivingUnderBridge.mp4");
	//capVideo.open("Resources/HighwayTraffic.mp4");
	//capVideo.open("Resources/HighwayTraffic2.mp4");
	//capVideo.open("Resources/HighwayTraffic3.mp4");
	strcCap = capVideo;

	if (!capVideo.isOpened()) {
		cout << "error reading video file" << endl << endl;
		return(0);
	}

	Ptr<BackgroundSubtractor> pBackSub;
	pBackSub = createBackgroundSubtractorMOG2();
	
	int frameCount = 3;
	vector<vector<Point>> cont, cont2;
	vector<Vec4i> hierarchy, hierarchy2;
	vector<Mat> result_planes, result_norm_planes;
	int area;
	int fps = 0;
	Mat roi, tmproi, element, element2, erosion_dst, dilation_dst, show, matlines;
	Rect data;

	strcCap >> strucFrame;
	strcCap >> strucFrame;
	namedWindow("Struc", 1);
	setMouseCallback("Struc", onMouse, NULL);
	putText(strucFrame, "Press Space after ROI is set", Point(strucFrame.cols / 2 - 220, 80), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 255, 0), 2);
	imshow("Struc", strucFrame);
	while (condition) {
		cout << "Press Space after ROI is selected" << endl;
		char ch = waitKey();
		if (ch == ' ') {
			condition = false;
		}
		cout << "P1:(" << P1.x << ", " << P1.y << ")" << endl;
		cout << "P2:(" << P2.x << ", " << P2.y << ")" << endl;
		cout << "ROI:(" << ROIR.x << ", " << ROIR.y <<  ", " << ROIR.width << ", " << ROIR.height << ")" << endl;
	}
	
	destroyWindow("Struc");
	namedWindow("CounterLine", 1);
	setMouseCallback("CounterLine", onCounter, NULL);
	strucFrame = strucFrame(ROIR);
	putText(strucFrame, "Press Space after Countbar is set", Point(strucFrame.cols / 4, 80), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 255, 0), 2);
	imshow("CounterLine", strucFrame);
	condition = true;
	while (condition) {
		cout << "Press Space after Countbar is set" << endl;
		char c = waitKey();
		if (c == ' ') {
			condition = false;
		}
	}

	destroyWindow("CounterLine");
	
	while (capVideo.isOpened()) {
		capVideo >> imgFrame1;
		imgFrame1 = histogramm(imgFrame1);
		rectangle(imgFrame1, ROIR, Scalar(0, 255, 0), 1, 8, 0);
		line(imgFrame1(ROIR), start, ende, Scalar(0, 255, 0), 2);
		roi = imgFrame1(ROIR);
		tmproi = imgFrame1.clone();
		imgLineY = tmproi.size().height / 2.85;
		if (activeline) {
			imgFrame1 = detectStreet(imgFrame1);
		}
		
		pBackSub->apply(roi, fgMask);
		
		blur(fgMask, fgMask, Size(13, 13), Point(-1, -1));
		threshold(fgMask, fgMask, 127, 255, THRESH_BINARY);

		element = getStructuringElement(MORPH_RECT, Size(2, 2));
		element2 = getStructuringElement(MORPH_RECT, Size(9, 9));
		dilate(fgMask, dilation_dst, element2);
		findContours(dilation_dst, cont, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

		for (unsigned int i = 0; i < cont.size(); i++) {
			area = contourArea(cont[i]);
			if (area > 1000) {
				//drawContours(roi, cont, i, Scalar(0, 255, 0), 2, LINE_8, hierarchy, 0);
				data = boundingRect(cont[i]);
				newRect(data);
				//rectangle(roi, Point(data.x, data.y), Point(data.x + data.width, data.y + data.height), Scalar(255, 0, 0), 2, 8, 0);
			}
		}
		countdatacartime();
		//line(roi, Point(0, imgLineY), Point(roi.size().width, imgLineY), 1, 8, 0);
		for (int i = 0; i < datacar.size(); i++) {
			rectangle(roi, Point(datacar[i].x, datacar[i].y), Point(datacar[i].x + datacar[i].width, datacar[i].y + datacar[i].height), Scalar(255, 0, 0), 2, 8, 0);
		}
		cout << "-------------------------------------" << endl;
		
		
		//show the current frame and the fg masks
		imshow("FG Mask", fgMask);
		imshow("ROI", roi);
		imshow("Source", imgFrame1);
		if ((capVideo.get(1) + 1) < capVideo.get(7)) {
			capVideo.read(imgFrame1);
		}
		else {
			cout << "end of video\n";
			break;
		}
		frameCount++;

 		char b = waitKey(fps);
		if (b == '+') {
			fps++;
		}
		else if (b == '-') {
			fps--;
		}
	}

	return(1);
}