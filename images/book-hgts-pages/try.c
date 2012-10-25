// gcc -o try try.c -lm -lopencv_core -lopencv_highgui -lopencv_imgproc -std=c99
// use command "./try" for the first run, and "./try An Y thing" for subsequent runs, to save time.

#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#define N_IMAGES 10


int main (int argc, char **argv) {
	char window_name[] = "try";
	IplImage *averaged;
	CvSize imgSize;
	int jpg_quality = 90;

	if (argc == 1) {

		IplImage* imgReds[N_IMAGES];
		IplImage* imgGreens[N_IMAGES];
		IplImage* imgBlues[N_IMAGES];

		for (int i=0;i<N_IMAGES;i++) {
			IplImage* img;
			char filename[8];
			sprintf(filename, "%d.jpg", (i+1));
			img = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
			imgReds[i] = cvCreateImage(cvGetSize(img), 8, 1);
			imgGreens[i] = cvCreateImage(cvGetSize(img), 8, 1);
			imgBlues[i] = cvCreateImage(cvGetSize(img), 8, 1);
			cvSplit(img, imgReds[i], imgGreens[i], imgBlues[i], NULL);
			cvReleaseImage(&img);
		}

		imgSize = cvGetSize(imgReds[0]);
		averaged = cvCreateImage(imgSize, 8, 3);
		IplImage *imgRedsSum   = cvCreateImage(imgSize, 8, 1);
		IplImage *imgGreensSum = cvCreateImage(imgSize, 8, 1);
		IplImage *imgBluesSum  = cvCreateImage(imgSize, 8, 1);

		for (int y = 0; y < imgSize.height; y++) {
			for (int x = 0; x < imgSize.width; x++) {
				double sumRed   = 0.0f;
				double sumGreen = 0.0f;
				double sumBlue  = 0.0f;

				for (int i = 0; i < N_IMAGES; i++) {
					sumRed   += cvGetReal2D(imgReds[i], y, x);
					sumGreen += cvGetReal2D(imgGreens[i], y, x);
					sumBlue  += cvGetReal2D(imgBlues[i], y, x);
				}
				sumRed   /= N_IMAGES;
				sumGreen /= N_IMAGES;
				sumBlue  /= N_IMAGES;
				cvSetReal2D(imgRedsSum, y, x, sumRed);
				cvSetReal2D(imgGreensSum, y, x, sumGreen);
				cvSetReal2D(imgBluesSum, y, x, sumBlue);
			}
		}
		cvMerge(imgRedsSum, imgGreensSum, imgBluesSum, NULL, averaged);
		cvSaveImage("averaged.jpg", averaged, &jpg_quality);
	}

	else {
		averaged = cvLoadImage("averaged.jpg", CV_LOAD_IMAGE_COLOR);
		imgSize = cvGetSize(averaged);
	}

	cvNamedWindow(window_name, CV_WINDOW_AUTOSIZE);
	cvShowImage(window_name, averaged);
	printf("Showing \"averaged\" image. Going to blur it. Press any key to continue...\n");
	cvWaitKey(0);


	int neighborhood = 3;
	double offset = 3.6;

	cvSmooth(averaged, averaged, CV_GAUSSIAN, 3, 0, 0, 0);
	cvShowImage(window_name, averaged);
	printf("Showing blurred image. Going to do adaptive thresholding. Press any key to continue...\n");
	cvWaitKey(0);

	IplImage *gray = cvCreateImage(imgSize, averaged->depth, 1);
	cvCvtColor(averaged, gray, CV_RGB2GRAY);

	IplImage *thres = cvCreateImage(imgSize, averaged->depth, 1);

	char k;
	while (1) {
		cvAdaptiveThreshold(gray, thres,
		                    255.0f,
		                    CV_ADAPTIVE_THRESH_GAUSSIAN_C,
		                    CV_THRESH_BINARY, neighborhood, offset);
		cvShowImage(window_name, thres);

		printf("Now adjust neighborhood and offset with i, o, -, =.\n");
		k = cvWaitKey(0);
		if (k == '-')
			offset -= 0.1;
		else if (k == '=')
			offset += 0.1;
		else if (k == 'i') {
			if (neighborhood != 3)
				neighborhood -= 2;
		}
		else if (k == 'o')
			neighborhood += 2;
		else if (k == 'q')
			break;

		printf("neighborhood = %d, offset = %G\n", neighborhood, offset);
	}

	cvSaveImage("thres.jpg", thres, &jpg_quality);
	cvReleaseImage(&thres);
	return 0;
}
