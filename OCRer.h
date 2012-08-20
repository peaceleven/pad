#ifndef OCRER_H
#define OCRER_H

#include <iostream>
#include <tesseract/baseapi.h>
#include "CameraDelegate.h"

namespace pad {
	using namespace std;

	class OCRer : public CameraDelegate {
	public:
		OCRer();
		~OCRer();
		void process_image(const unsigned char *img_data,
		                   int bytes_per_pixel, int bytes_per_line,
		                   int left, int top, int width, int height);

	private:
		tesseract::TessBaseAPI     *_tess;
	};
}
#endif
