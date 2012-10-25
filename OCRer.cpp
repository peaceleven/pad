#include "OCRer.h"

namespace pad {

OCRer::OCRer() {
	char tessdata_path[] = "/usr/local/share/tessdata/";

	int n_configs = 1;
	char tess_config0[] = "common";
	char *tess_configs[n_configs];
	tess_configs[0] = tess_config0;

	_tessapi = new TessBaseAPI();
	_tessapi->Init(tessdata_path, "eng", OEM_TESSERACT_ONLY, tess_configs, n_configs, NULL, NULL, false);
	_tessapi->SetPageSegMode(PSM_AUTO_ONLY);
	setenv("TESSDATA_PREFIX", tessdata_path, 1);
}
OCRer::~OCRer() {
	_tessapi->End();
	delete _tessapi;
}

void OCRer::process_image(const unsigned char *img_data, int bytes_per_pixel, int bytes_per_line, int left, int top, int width, int height) {
	_tessapi->SetImage(img_data, width, height, bytes_per_pixel, bytes_per_line);
	_tessapi->Recognize(0);

	ResultIterator *ri = _tessapi->GetIterator();
	if (ri != NULL) {
		ri->Begin();
		int left, top, right, bottom;
		while (ri->Next(RIL_WORD)) {
			ri->BoundingBox(RIL_WORD, &left, &top, &right, &bottom);
			cout << "left = " << left << " right = " << right << " top = " << top << " bottom = " << bottom << endl;

			char *ocr_text = ri->GetUTF8Text(RIL_WORD);
			cout << "t = " << ocr_text << endl;
			delete ocr_text;
		}
		delete ri;
	}
}

}
