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
	double offset = 2.0;

	int count = 12;
	char window_name[] = "adap-thres";

	Mat orig_img = imread(argv[1], 0);
	Mat* thresholded_imgs = new Mat[count];


	for (int i=0; i<count; i++) {
		thresholded_imgs[i] = Mat::zeros( orig_img.size(), orig_img.type() );
	}

	GaussianBlur( orig_img, orig_img, Size(neighborhood, neighborhood), 0, 0 );
//	equalizeHist( orig_img, orig_img );
	Mat dst = Mat::zeros(orig_img.size(), orig_img.type());
//	Laplace(orig_img, dst, 3);


	for (int i=0; i<count; i++) {
		adaptiveThreshold(orig_img,
	                      thresholded_imgs[i],
		                  (double) 255,
		                  ADAPTIVE_THRESH_GAUSSIAN_C,
		                  THRESH_BINARY, neighborhood, offset);
		neighborhood += 2;
	}

	namedWindow(window_name, 1);

	int img_idx = 0;
	char key;

	/*
	 * the following code segment was used to determine which image is the best

	neighborhood -= count * 2;

	do {
		cout << "img number = " << img_idx << ", neighborhood = " << neighborhood + 2*img_idx << endl;
		imshow(window_name, thresholded_imgs[img_idx]);
		key = waitKey(0);

		cout << "key = " << key << endl;

		if (key == 'q')
			break;

		else if (key == 'a') {
			if (img_idx == 0)  img_idx = count-1;
			else               img_idx--;
		}

		else if (key ==  'd') {
			if (img_idx == count-1)  img_idx = 0;
			else                     img_idx++;
		}

	} while (1);
	*/

	Mat best_img = thresholded_imgs[img_idx];

	OCRer *ocrer = new OCRer();
	ocrer->process_image((const unsigned char*) best_img.data,
	                     best_img.step / best_img.cols,
	                     best_img.step,
	                     0, 0, best_img.cols, best_img.rows);

	key = waitKey(0);

	delete ocrer;
	return EXIT_SUCCESS;
}
