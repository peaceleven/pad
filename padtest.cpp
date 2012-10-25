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

	int neighborhood = 3;
	int neighborhood0 = 3;
	double offset = 4.f;

	char window_name[] = "adap-thres";
	namedWindow(window_name, 1);

	Mat orig_img = imread(argv[1], 0);
	Mat thresholded_img = Mat::zeros(orig_img.size(), orig_img.type());

	GaussianBlur( orig_img, orig_img, Size(neighborhood, neighborhood), 0, 0 );

	char key;
	do {
		cout << "neighborhood = " << neighborhood << ", offset = " << offset << endl;
		adaptiveThreshold(orig_img,
		                  thresholded_img,
		                  255.f,
		                  ADAPTIVE_THRESH_GAUSSIAN_C,
		                  THRESH_BINARY, neighborhood, offset);


		imshow(window_name, thresholded_img);
		key = waitKey(0);

		cout << "key = " << key << endl;

		if (key == 'q')
			break;

		else if (key == '-') {
			offset -= .1f;
		}
		else if (key == '=') {
			offset += .1f;
		}
		else if (key == 'i') {
			if (neighborhood != neighborhood0)
				neighborhood -= 2;
		}
		else if (key == 'o') {
			neighborhood += 2;
		}

	} while (1);


	OCRer *ocrer = new OCRer();
	ocrer->process_image((const unsigned char*) thresholded_img.data,
	                     thresholded_img.channels(),
	                     thresholded_img.step,
	                     0, 0, thresholded_img.cols, thresholded_img.rows);

	key = waitKey(0);

	delete ocrer;
	return EXIT_SUCCESS;
}
