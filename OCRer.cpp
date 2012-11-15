#include "OCRer.h"

namespace pad {

OCRer::OCRer() {
	char tessdata_path[] = "/usr/local/share/";
	setenv("TESSDATA_PREFIX", tessdata_path, 1);

	int n_configs = 1;
	char *tess_configs[n_configs];
	char tess_config0[] = "common";
	tess_configs[0] = tess_config0;

	_tessapi = new TessBaseAPI();
	_tessapi->Init(tessdata_path, "eng", OEM_TESSERACT_ONLY, tess_configs, n_configs, NULL, NULL, false);
	_tessapi->SetPageSegMode(PSM_AUTO_ONLY);
	// _tessapi->SetVariable("save_best_choices", "T");
}
OCRer::~OCRer() {
	_tessapi->End();
	delete _tessapi;
}

bool heuristic(char *s) {
	if (s == NULL)
		return false;

	int l = strlen(s);
	if (l <= 1)
		return false;

	int n_alpha = 0;
	int n_digit = 0;
	for (int i = 0; i < l; i++) {
		if (isalpha(s[i]))
			n_alpha++;
		else if (isdigit(s[i]))
			n_digit++;
	}

	if (n_digit == l)
		return true;

	float p = (float) (n_alpha + n_digit) / (float) l;

	if (l <= 4)
		return (p > .5f);
	else
		return (p > .7f);
}

void OCRer::process_image(const unsigned char *img_data, int bytes_per_pixel, int bytes_per_line, int left, int top, int width, int height) {
	_tessapi->SetImage(img_data, width, height, bytes_per_pixel, bytes_per_line);
	_tessapi->Recognize(0);

	ResultIterator *ri = _tessapi->GetIterator();

	/*
	tesseract::ChoiceIterator* ci;
	if(ri != 0) {
		while((ri->Next(tesseract::RIL_SYMBOL))) {
			const char* symbol = ri->GetUTF8Text(tesseract::RIL_SYMBOL);

			if (symbol != 0) {
				float conf = ri->Confidence(tesseract::RIL_SYMBOL);
				std::cout << "\tnext symbol: " << symbol << "\tconf: " << conf << endl;

				const tesseract::ResultIterator itr = *ri;
				ci = new tesseract::ChoiceIterator(itr);

				do {
					std::cout << "\t\t" << ci->GetUTF8Text() << " conf: " << ci->Confidence() << endl;
				} while(ci->Next());

				delete ci;
			}

			delete[] symbol;
		}
	}
	*/

	if (ri != NULL) {
		// int l, t, r, b;
		while (ri->Next(RIL_WORD)) {
			// ri->BoundingBox(RIL_WORD, &l, &t, &r, &b);
			// cout << "rect = " << l << ", " << r << ", " << t << ", " << b << endl;

			char *ocr_text = ri->GetUTF8Text(RIL_WORD);

			if (heuristic(ocr_text)) {
				if (ri->WordIsFromDictionary()) {
					_dict_words.insert(string(ocr_text));
					// cout << "conf = " << ri->Confidence(RIL_WORD) << endl;

				} else {
					_live_words.insert(string(ocr_text));
				}
			}

			delete[] ocr_text;
		}

		delete ri;
	}
}

}
