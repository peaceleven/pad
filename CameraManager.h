// Abstraction layer  between OCRer (which uses  Tesseract), and video
// libraries, which could vary among platforms.

// This  class is  written for  Linux,  using v4l2  (video for  linux,
// version 2) library.

#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <string>
#include <vector>
#include <iostream>

#include <cstdlib>
#include <cerrno>
#include <cassert>
#include <cstring>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <linux/videodev2.h>
#include <libv4lconvert.h>

#include "types.h"
#include "CameraDelegate.h"

#define MEM_ZERO(x) memset(&(x), 0, sizeof(x))

namespace pad {
	using namespace std;

	class CameraManager {
	public:
		CameraManager(string dev_name, io_method iom);
		~CameraManager();

		void open_and_init_device();		// open(); set cropping; negotiate formats.
		void start_capturing();				// turn the stream on.
		void mainloop();					// read-wait-process dead loop.
		void stop_capturing();				// turn the stream off.
		void uninit_and_close_device();		// free/unmap buffers; close().

		void add_camera_delegate(CameraDelegate *cd);

	private:
		int                         _video_fd;
		string                      _dev_name;
		io_method                   _io_method;
		struct v4l2_rect            _crop_rect;
		struct buffer              *_buffers;
		unsigned int                _num_buffers;
		int                         _period;
		struct v4lconvert_data     *_v4lconvert_data;
		struct v4l2_format          _src_fmt;
		struct v4l2_format          _dest_fmt;
		unsigned char              *_converted_img_buffer;
		vector<CameraDelegate*>     _camera_delegates;

		int  read_frame();
		void init_read(unsigned int buffer_size);
		void init_mmap();
		void init_userp(unsigned int buffer_size);
	};
}
#endif
