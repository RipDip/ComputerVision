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
#include <time.h>

using namespace std;
using namespace cv;

Mat imgFrame1;
Mat strucFrame;
Mat strucCopy;
Mat fgMask;

VideoCapture capVideo;
VideoCapture strcCap;

Point P1;
Point P2;
Point P3;
Point start, ende;
Rect2i ROIR(0, 0, 0, 0);

vector<Rect> datacar;
vector<int> datacartime;
vector<int> datacarcount;
vector<vector<Point>> contoursline;


int countCar = 0;
int indexLine;
float imgLineY = 0;

//Parameter
int datacarhealth = 2;
int range = 60;
int maxLight = 160;
int detectLineParameter[] = { 540, 740, 1200, 260, 460, 600 };
int minArea = 2000;
float detectSize = 2.85;

bool condition = true;
bool clicked = false;
bool vertical = false;

/*
* showRectangle is a function for 
* prewviewing the ROI area while trying
* to set it
*/

void showRectangle() {
	strucCopy = strucFrame.clone();
	rectangle(strucFrame, ROIR, Scalar(0, 255, 0), 1, 8, 0);
	imshow("Struc", strucFrame);
	strucFrame = strucCopy.clone();
}

/*
* showLine is a function for
* prewviewing the Countbar in
* the ROI
*/

void showLine() {
	strucCopy = strucFrame.clone();
	line(strucFrame, start, ende, Scalar(0, 255, 0), 2);
	imshow("CounterLine", strucFrame);
	strucFrame = strucCopy.clone();
}

/*
* Callbackfunction for setting the Countbar
* @param event What event is happening LBUTTONDOWN/RBUTTONDOWN and so on
* @param x Mouse cords of x
* @param y Mouse cords of y
* @param int flags specific buttons like altkey and shiftkey
* @param void* userdata This pointer will be passed to the callback function
*/

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

/*
* Callbackfunction for setting the ROI
* @param event What event is happening LBUTTONDOWN/RBUTTONDOWN and so on
* @param x Mouse cords of x
* @param y Mouse cords of y
* @param int flags specific buttons like altkey and shiftkey
* @param void* userdata This pointer will be passed to the callback function
*/

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

/*
* A function for counting the cars
* and to see if the car is real or not
*/

void countdatacartime() {
	for (int i = 0; i < datacartime.size(); i++) {
		datacartime[i] = datacartime[i] + 1;
		if ((datacar[i].y + datacar[i].height / 2) < P3.y  && (datacar[i].y + datacar[i].height / 2) > (P3.y - range) && datacarcount[i] == 0) {
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
	}
}

/*
* A function that returns true or false
* if the element is in range of specific area
* @param low start point
* @param hight end point
* @param x should be a point between low and high
* @return true or false
*/

bool inRange(int low, int high, int x){
	return (low <= x && x <= high);
}

/*
* A function that checks if the
* rectangle is a new rectangle or if a rectangle
* shows inside of an other rectangle or if its the same rectangle
* @param data the Rectangle of the detected car
*/

void newRect(Rect data) {
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

	//than new car add to datacar
	if (newcar) {
		cout << "New car" << endl;
		datacar.push_back(data);
		datacartime.push_back(0);
		datacarcount.push_back(0);
	}
}

/**
 * Calculate the determinant of the two lines
 * 
 * @param x1 x value from the first line
 * @param y1 y value from the first line
 * @param x2 x value from the second line
 * @param y2 y value from the second line
 * @return The result of the new Point
 */
double Det(double x1, double y1, double x2, double y2){
	return x1 * y2 - y1 * x2;
}

/**
 * Calculate the Point where two lines are meet
 * 
 * @param x1 First x of the first line
 * @param y1 First y of the first line
 * @param x2 Second x of the first line
 * @param y2 Second y of the first line
 * @param x3 First x of the second line
 * @param y3 First y of the second line
 * @param x4 Second x of the second line
 * @param y4 Second y of the second line
 * @param xOut The result for the new x value
 * @param yOut the result for the new y value
 * @return if the new Point is calculated
 */

bool lineIntersect(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, double& xOut, double& yOut) {
	double det1 = Det(x1, y1, x2, y2);
	double det2 = Det(x3, y3, x4, y4);
	double x1x2 = x1 - x2;
	double x3x4 = x3 - x4;
	double y1y2 = y1 - y2;
	double y3y4 = y3 - y4;

	double det = Det(x1x2, y1y2, x3x4, y3y4);
	if (det == 0.0){
		return false;
	}

	double xnom = Det(det1, x1x2, det2, x3x4);
	double ynom = Det(det1, y1y2, det2, y3y4);
	xOut = xnom / det;
	yOut = ynom / det;
	if (!isfinite(xOut) || !isfinite(yOut)){
		return false;
	}
		
	return true;
}

/**
 * Calculates the two lines from the street in the first frame and then
 * 
 * @param roi The image in which the road should be recognized. The image should be the original image and no filter has already been applied 
 * @return The image of the street with the two new lines.
 */

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
	float sizeline = 0;
	float tmpsizeline = 0;

	if (!firstframe) {
		cvtColor(matlines, mask1, COLOR_RGB2GRAY);
		Canny(mask1, mask1, 50, 150);

		HoughLinesP(mask1, lines, 1, CV_PI / 180, 300, 100, 250);
		firstframe = true;
		for (size_t i = 0; i < lines.size(); i++) {
			Vec4i l = lines[i];
			double x, y;
			lineIntersect(l[0], l[1], l[2], l[3], 0, imgLineY, roi.size().width, imgLineY, x, y);
			if ((l[0] > detectLineParameter[0] && l[0] < detectLineParameter[1]) || (l[2] > detectLineParameter[2] && l[3] > detectLineParameter[3] && l[3] > detectLineParameter[4] || (l[1] > detectLineParameter[5]))) {
				if (l[1] < imgLineY) {
					tmplines.push_back(l);
				}
				else {
					tmplines2.push_back(l);
				}
			}
		}
		for (int i = 0; i < tmplines2.size(); i++) {
			tmpsizeline = tmplines2[i][1] - tmplines2[i][3];
			if (tmpsizeline > sizeline && tmplines2[i][0] != tmplines2[i][2]) {
				sizeline = tmpsizeline;
				indexLine = i;
			}
		}
	}
	if (tmplines.size() > 0) {
		lineIntersect(tmplines[0][0], tmplines[0][1], tmplines[0][2], tmplines[0][3], 0, imgLineY, roi.size().width, imgLineY, x, y);
		line(roi, Point(x, imgLineY), Point(tmplines[0][2], tmplines[0][3]), Scalar(0, 255, 0), 3, LINE_AA);
		
		lineIntersect(tmplines2[indexLine][0], tmplines2[indexLine][1], tmplines2[indexLine][2], tmplines2[indexLine][3], 0, imgLineY, roi.size().width, imgLineY, x, y);
		line(roi, Point(tmplines2[indexLine][0], tmplines2[indexLine][1]), Point(x, imgLineY), Scalar(255, 255, 0), 3, LINE_AA);
	}

	cout << "-------------------------------------" << endl;
	return roi;
}

/**
* A function for selecting a video
* in the resource folder
* @return path string for the video file
 */

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

/**
* A function for turning the streetdectection on/off
* @return activeline true or false for the streetdectection
 */

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

/**
* function for turning the histogram on/off
* @return activehistogram true or false for the histogram
*/

bool askActiveHistogram() {
	int choise = 0;
	string tmp;
	bool endwhile = false;
	bool activehistogram;
	while (!endwhile) {
		cout << "Do you want histogram equalize?" << endl;
		cout << "You should enable it for night videos" << endl;
		cout << "You have the choise between 0 and 1" << endl;
		cout << "0 for histogram equalize off and 1 for histogram equalize on" << endl;
		cin >> tmp;
		istringstream(tmp) >> choise;
		if (choise < 0 || choise > 1) {
			cout << "Error" << endl;
		}
		else {
			endwhile = true;
			if (choise == 0) {
				activehistogram = false;
			}
			else {
				activehistogram = true;
			}
		}
	}
	return activehistogram;
}

/**
 * Tries to darken the pixels that are too bright. This images must be a gray images.
 * 
 * @param imgFrame1 The image that should be darkened
 * @return the new images 
 */
Mat equalize(Mat& imgFrame1) {
	int flatimg[256] = { 0 };
	int cumsum[256] = { 0 };
	int memory = 0;
	int normalizeimg[256] = { 0 };
	Mat result(imgFrame1.rows, imgFrame1.cols, CV_8U);

	for (int i = 0; i < imgFrame1.rows; i++) {
		for (int j = 0; j < imgFrame1.cols; j++) {
			int index;
			index = static_cast<int>(imgFrame1.at<uchar>(i, j));
			flatimg[index]++;
		}
	}

	for (int i = 0; i < maxLight; i++) {
		memory += flatimg[i];
		cumsum[i] = memory;
	}

	for (int i = 0; i < maxLight; i++) {
		normalizeimg[i] = ((cumsum[i] - cumsum[0]) * 255) / (imgFrame1.rows * imgFrame1.cols - cumsum[0]);
		normalizeimg[i] = static_cast<int>(normalizeimg[i]);
	}
	

	Mat_<uchar>::iterator tmpresult = result.begin<uchar>();
	Mat_<uchar>::iterator begin = imgFrame1.begin<uchar>();
	Mat_<uchar>::iterator end = imgFrame1.end<uchar>();

	while (begin != end) {
		int intensityvalue = static_cast<int>(*begin);
		*tmpresult = normalizeimg[intensityvalue];
		tmpresult++;
		begin++;
	}

	return result;
}

/**
 * This algorithm only works for night videos and convert.
 * Convert the images to a gray Images and equalize this images.
 * Create a histogramm images from the gray image
 * 
 * @param imgFrame1 The images that shut be improve
 * @return The gray image that has been darkened 
 */
Mat histogramm(Mat imgFrame1) {
	cvtColor(imgFrame1, imgFrame1, COLOR_BGRA2GRAY);

	Mat img_out;
	img_out = equalize(imgFrame1);

	imshow("New Images", img_out);

	vector<Mat> bgr_planes;
	split(imgFrame1, bgr_planes);
	int histSize = 256;
	float range[] = { 0, 256 };
	const float* histRange[] = { range };
	
	bool uniform = true, accumulate = false;
	Mat b_hist, g_hist, r_hist;
	calcHist(&img_out, 1, 0, Mat(), b_hist, 1, &histSize, histRange, uniform, accumulate);
	int hist_w = 520, hist_h = 420;
	int bin_w = cvRound((double)hist_w / histSize);
	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
	normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	for (int i = 1; i < histSize; i++){
		line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))), Point(bin_w * (i), hist_h - cvRound(b_hist.at<float>(i))), Scalar(255, 0, 0), 2, 8, 0);
		b_hist.at<float>(i);
	}
	imshow("calcHist Demo", histImage);

	return img_out;
}

/**
 * The main Programm
 * 
 * @return 1
 */
int main(void) {
	int frameCount = 3;
	vector<vector<Point>> cont;
	vector<Vec4i> hierarchy;
	int area;
	int fps = 1;
	Mat roi, tmproi, element, dilation_dst;
	Rect data;
	clock_t startTime, endTime;
	bool activeline = askActiveLinie();
	string path = openVideo();
	bool activehistogram = askActiveHistogram();
	
	capVideo.open(path);

	strcCap = capVideo;

	if (!capVideo.isOpened()) {
		cout << "error reading video file" << endl << endl;
		return(0);
	}

	Ptr<BackgroundSubtractor> pBackSub;
	pBackSub = createBackgroundSubtractorMOG2();
	
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
		startTime = clock();
		capVideo >> imgFrame1;
		
		rectangle(imgFrame1, ROIR, Scalar(0, 255, 0), 1, 8, 0);
		line(imgFrame1(ROIR), start, ende, Scalar(0, 255, 0), 2);
		roi = imgFrame1(ROIR);
		Mat newFrame;
		if (activehistogram) {
			newFrame = histogramm(imgFrame1(ROIR));
		}
		else {
			newFrame = imgFrame1(ROIR);
		}
			
		tmproi = imgFrame1.clone();
		
		if (activeline) {
			imgLineY = tmproi.size().height / detectSize;
			imgFrame1 = detectStreet(imgFrame1);
			
		}
		
		pBackSub->apply(newFrame, fgMask);
		
		blur(fgMask, fgMask, Size(13, 13), Point(-1, -1));
		threshold(fgMask, fgMask, 127, 255, THRESH_BINARY);

		element = getStructuringElement(MORPH_RECT, Size(9, 9));
		dilate(fgMask, dilation_dst, element);
		findContours(dilation_dst, cont, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

		for (unsigned int i = 0; i < cont.size(); i++) {
			area = contourArea(cont[i]);
			if (area > minArea) {
				//drawContours(roi, cont, i, Scalar(0, 255, 0), 2, LINE_8, hierarchy, 0);
				data = boundingRect(cont[i]);
				newRect(data);
			}
		}
		countdatacartime();
		for (int i = 0; i < datacar.size(); i++) {
			rectangle(roi, Point(datacar[i].x, datacar[i].y), Point(datacar[i].x + datacar[i].width, datacar[i].y + datacar[i].height), Scalar(255, 0, 0), 2, 8, 0);
		}

		cout << "-------------------------------------" << endl;
		
		endTime = clock();
		double seconds = (double(endTime) - double(startTime))/double(CLOCKS_PER_SEC);
		cout << "Time taken : " << seconds << " seconds" << endl;
		cout << "FPS: " << 1/seconds << endl;
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