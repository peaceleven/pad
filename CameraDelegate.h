#ifndef CAMERA_DELEGATE_H
#define CAMERA_DELEGATE_H

#include <iostream>

namespace pad {
	class CameraDelegate {
	public:
		virtual ~CameraDelegate() {};
		virtual void process_image (unsigned char *img_data, int bytes_per_pixel, int bytes_per_line, int left, int top, int width, int height) = 0;
	};
}
#endif
