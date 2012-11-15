#ifndef OCRER_H
#define OCRER_H

#include <iostream>
#include <set>
#include <tesseract/baseapi.h>
#include <tesseract/resultiterator.h>
#include "CameraDelegate.h"

namespace pad {
	using namespace std;
	using namespace tesseract;

	class OCRer : public CameraDelegate {
	public:
		OCRer();
		~OCRer();
		void process_image(const unsigned char *img_data,
		                   int bytes_per_pixel, int bytes_per_line,
		                   int left, int top, int width, int height);

	private:
		TessBaseAPI       *_tessapi;
		set<string>        _dict_words;
		set<string>        _live_words;
	};
}
#endif
