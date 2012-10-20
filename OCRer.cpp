#include "OCRer.h"

namespace pad {

OCRer::OCRer() {
	char tessdata_path[] = "/usr/local/share/tessdata/";

	int n_configs = 1;
	char tess_config0[] = "common";
	char *tess_configs[n_configs];
	tess_configs[0] = tess_config0;

	_tess = new tesseract::TessBaseAPI();
	_tess->Init(tessdata_path, "eng", tesseract::OEM_TESSERACT_ONLY, tess_configs, n_configs, NULL, NULL, false);
	setenv("TESSDATA_PREFIX", tessdata_path, 1);
}
OCRer::~OCRer() {
	_tess->End();
	delete _tess;
}

void OCRer::process_image(const unsigned char *img_data, int bytes_per_pixel, int bytes_per_line, int left, int top, int width, int height) {
	char *s = _tess->TesseractRect(img_data, bytes_per_pixel, bytes_per_line, left, top, width, height);
	cout << s;
	free(s);
}

}
