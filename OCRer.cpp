#include "OCRer.h"

namespace pad {

OCRer::OCRer() {
	const char* tessdata_path = "/usr/local/share/tessdata/";
	_tess = new tesseract::TessBaseAPI();
	_tess->Init(tessdata_path, "eng");
	setenv("TESSDATA_PREFIX", tessdata_path, 1);
}
OCRer::~OCRer() {
	_tess->End();
	delete _tess;
}

void OCRer::process_image(const unsigned char *img_data, int bytes_per_pixel, int bytes_per_line, int left, int top, int width, int height) {
	char *s = _tess->TesseractRect(img_data, bytes_per_pixel, bytes_per_line, left, top, width, height);
	cout << "OCR text:" << s;
	free(s);
}

}
