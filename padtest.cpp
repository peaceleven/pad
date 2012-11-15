/*
 * this is an automated test driver;
 * it is like "pad", except that it takes images from files (specified on the cmd line)
 * rather than from the camera.
 */


#include "OCRer.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace std;
using namespace cv;
using namespace pad;

int main (int argc, char *argv[]) {

	Mat orig_img = imread(argv[1], 0);
	Mat thresholded_img = Mat::zeros(orig_img.size(), orig_img.type());

	GaussianBlur(orig_img, orig_img, Size(3, 3), 0, 0);
	adaptiveThreshold(orig_img, thresholded_img, 255.f,
	                  ADAPTIVE_THRESH_GAUSSIAN_C,
	                  THRESH_BINARY, 5, 4.1f);

	OCRer *ocrer = new OCRer();
	ocrer->process_image((const unsigned char*) thresholded_img.data,
	                     thresholded_img.channels(),
	                     thresholded_img.step,
	                     0, 0, thresholded_img.cols, thresholded_img.rows);

	delete ocrer;
	return EXIT_SUCCESS;
}
