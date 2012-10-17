// gcc -o try try.c -lm -lopencv_core -lopencv_highgui -lopencv_imgproc -std=c99

#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#define N_IMAGES 10

int main() {
	IplImage* imgRed[N_IMAGES];
	IplImage* imgGreen[N_IMAGES];
	IplImage* imgBlue[N_IMAGES];

	for(int i=0;i<N_IMAGES;i++) {
		IplImage* img;
		char filename[8];
		sprintf(filename, "%d.jpg", (i+1));
		img = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
		imgRed[i] = cvCreateImage(cvGetSize(img), 8, 1);
		imgGreen[i] = cvCreateImage(cvGetSize(img), 8, 1);
		imgBlue[i] = cvCreateImage(cvGetSize(img), 8, 1);
		cvSplit(img, imgRed[i], imgGreen[i], imgBlue[i], NULL);
		cvReleaseImage(&img);
	}


	CvSize imgSize = cvGetSize(imgRed[0]);
	IplImage *imgResultRed = cvCreateImage(imgSize, 8, 1);
	IplImage *imgResultGreen = cvCreateImage(imgSize, 8, 1);
	IplImage *imgResultBlue = cvCreateImage(imgSize, 8, 1);

	IplImage *imgResult = cvCreateImage(imgSize, 8, 3);

	for(int y=0;y<imgSize.height;y++) {
		for(int x=0;x<imgSize.width;x++) {
			int theSumRed=0;
			int theSumGreen=0;
			int theSumBlue=0;
			for(int i=0;i<N_IMAGES;i++) {
				theSumRed+=cvGetReal2D(imgRed[i], y, x);
				theSumGreen+=cvGetReal2D(imgGreen[i], y, x);
				theSumBlue+=cvGetReal2D(imgBlue[i], y, x);
			}
			theSumRed = (float)theSumRed / N_IMAGES;
			theSumGreen = (float)theSumGreen / N_IMAGES;
			theSumBlue = (float)theSumBlue / N_IMAGES;
			cvSetReal2D(imgResultRed, y, x, theSumRed);
			cvSetReal2D(imgResultGreen, y, x, theSumGreen);
			cvSetReal2D(imgResultBlue, y, x, theSumBlue);
		}
	}

	char window_name[] = "averaged";
	cvMerge(imgResultRed, imgResultGreen, imgResultBlue, NULL, imgResult);
	cvNamedWindow(window_name, CV_WINDOW_AUTOSIZE);
	cvShowImage(window_name, imgResult);

	cvWaitKey(0);

	int neighborhood = 3;
	double offset = 3.6;

	// for (int i=0; i<N_IMAGES; i++) {
		cvSmooth( imgResult, imgResult, CV_GAUSSIAN, 3, 0, 0, 0 );
		cvShowImage(window_name, imgResult);
		cvWaitKey(0);

		IplImage *a = cvCreateImage(imgSize, imgResult->depth, 1);
		cvCvtColor(imgResult, a, CV_RGB2GRAY);

		IplImage *b = cvCreateImage(imgSize, imgResult->depth, 1);

		cvAdaptiveThreshold(a,
		                    b,
		                    (double) 255,
		                    CV_ADAPTIVE_THRESH_GAUSSIAN_C,
		                    CV_THRESH_BINARY, neighborhood, offset);
		cvShowImage(window_name, b);
		cvWaitKey(0);
	// 	neighborhood += 2;
		int quality = 90;
		cvSaveImage("thres.jpg", b, &quality);
		cvReleaseImage(&b);
	// }


	char k;
	while (1) {
		k = cvWaitKey(0);
		if (k == 'q')
			break;
	}

	return 0;
}
