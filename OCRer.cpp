#include "OCRer.h"

namespace pad {

OCRer::OCRer() {
	_n_imgs = 0;
	_motion = false;

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

void calcImageDisplacement(Mat img1, Mat img2, vector<uchar> *status, vector<float> *err) {

	vector<Point2f> corners1, corners2;

	int maxCorners = 200;
	double qualityLevel = .2f;
	double minDistance = 4.f;
	goodFeaturesToTrack(img1, corners1, maxCorners, qualityLevel, minDistance);

	int nIterations = 30;
	double epislon = .01f;
	TermCriteria tc (CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, nIterations, epislon);
	Size winSize = Size(3, 3);
	Size zeroZone = Size(-1, -1);
	cornerSubPix(img1, corners1, winSize, zeroZone, tc);

	int maxLevel = 3;
	calcOpticalFlowPyrLK(img1, img2, corners1, corners2,
	                     (*status), (*err), winSize, maxLevel);
}

void OCRer::process_image(const unsigned char *img_data, int bytes_per_pixel, int bytes_per_line, int left, int top, int width, int height) {
}

void OCRer::process_image(Mat img) {
	if (_motion) {
		_n_imgs = 0;
		_motion = false;
	}


	if (_n_imgs == 0) {
		_average_img = img;

	} else {
		// detect motion by comparing with _average_img (or _last_img?)
		vector<uchar> status;
		vector<float> err;
		calcImageDisplacement(_average_img, img, &status, &err);

		float avg_error = 0;
		int n = 0;
		for (unsigned int i = 0; i < status.size(); i++) {
			bool matched = (bool) status[i];
			if (matched) {
				avg_error += err[i];
				n++;
			}
		}

		avg_error /= (float) n;
		if (avg_error > MOTION_THRESHOLD)
			_motion = true;

		else if (_n_imgs < MAX_N_IMGS) {
		}
	}
	_n_imgs++;


	_tessapi->SetImage((const unsigned char*) _average_img.data,
	                   _average_img.cols, _average_img.rows,
	                   _average_img.channels(), _average_img.step);
	_tessapi->Recognize(NULL);

	ResultIterator *ri = _tessapi->GetIterator();

	/*
	ChoiceIterator* ci;
	if (ri != NULL) {
		while ((ri->Next(RIL_SYMBOL))) {
			const char* symbol = ri->GetUTF8Text(RIL_SYMBOL);

			if (symbol != 0) {
				float conf = ri->Confidence(RIL_SYMBOL);
				std::cout << "\tnext symbol: " << symbol << "\tconf: " << conf << endl;

				const ResultIterator itr = *ri;
				ci = new ChoiceIterator(itr);

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
