#ifndef OCRER_H
#define OCRER_H

#include <iostream>
#include <set>
#include <vector>
#include <tesseract/baseapi.h>
#include <tesseract/resultiterator.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "CameraDelegate.h"

#define MAX_N_IMGS 10                       // max number of images to average
#define MOTION_THRESHOLD 140.0f

namespace pad {
	using namespace std;
	using namespace tesseract;
	using namespace cv;

	class OCRer : public CameraDelegate {
	public:
		OCRer();
		~OCRer();
		void process_image(unsigned char *img_data,
		                   int bytes_per_pixel, int bytes_per_line,
		                   int left, int top, int width, int height);
		void process_image(Mat img);

	private:
		TessBaseAPI       *_tessapi;
		set<string>        _dict_words;
		set<string>        _live_words;
		Mat       _average_img;
		Mat       _last_img;
		int       _n_imgs;
		bool      _motion;
	};
}
#endif
