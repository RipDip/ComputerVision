#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

int main() {
	cv::Mat frame;
	frame = cv::imread("baum.jpg", cv::IMREAD_COLOR);
	cv::imshow("frame", frame);
	cv::waitKey(0);
}