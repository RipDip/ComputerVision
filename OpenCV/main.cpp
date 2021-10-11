#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

int main() {
	Mat frame;
	frame = imread("baum.jpg", IMREAD_COLOR);
	imshow("frame", frame);
	waitKey(0);
}