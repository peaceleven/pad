#include "CameraManager.h"

namespace pad {

void errno_exit (string s) {
	cerr << s << " error " << errno << ": " << strerror (errno) << endl;
	exit (EXIT_FAILURE);
}




CameraManager::CameraManager (string dev_name, io_method iom) {
	_dev_name       = dev_name;
	_io_method      = iom;
	_video_fd       = -1;
	_buffers        = NULL;
	_num_buffers    = 0;
	_period         = 5;
}
CameraManager::~CameraManager () {}

void CameraManager::open_and_init_device () {

	/***** open device *****/
	struct stat st;

	if (-1 == stat(_dev_name.c_str(), &st)) {
		cerr << "Cannot identify '" << _dev_name << "': " << errno << ", " << strerror(errno) << endl;
		exit(EXIT_FAILURE);
	}

	if (!S_ISCHR(st.st_mode)) {
		cerr << _dev_name << "is not a device" << endl;
		exit(EXIT_FAILURE);
	}

	_video_fd = open(_dev_name.c_str(), O_RDWR | O_NONBLOCK, 0);

	if (_video_fd == -1) {
		cerr << "Cannot open '" << _dev_name << "': " << errno << ", " << strerror(errno) << endl;
		exit(EXIT_FAILURE);
	}

	/***** init device *****/
	struct v4l2_capability cap;

	if (-1 == ioctl (_video_fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			cerr << _dev_name << "is not a V4L2 device" << endl;
			exit (EXIT_FAILURE);
		} else {
			errno_exit ("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		cerr << _dev_name << " is not a video capturing device" << endl;
		exit (EXIT_FAILURE);
	}

	switch (_io_method) {
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			cerr << _dev_name << " does not support read I/O" << endl;
			exit (EXIT_FAILURE);
		}
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			cerr << _dev_name << " does not support streaming ioctls" << endl;
			exit (EXIT_FAILURE);
		}
		break;
	}

	v4l2_cropcap cropcap;
	MEM_ZERO (cropcap);
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == ioctl (_video_fd, VIDIOC_CROPCAP, &cropcap))
		errno_exit("VIDIOC_CROPCAP");

	MEM_ZERO(_crop_rect);
	_crop_rect.width = cropcap.defrect.width; // 1920;
	cout << "crop.width = " << _crop_rect.width << endl;
	_crop_rect.height = cropcap.defrect.height; //1080;
	cout << "crop.height = " << _crop_rect.height << endl;

	MEM_ZERO(_src_fmt);
	_src_fmt.type                   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	_src_fmt.fmt.pix.width          = _crop_rect.width;
	_src_fmt.fmt.pix.height         = _crop_rect.height;
	_src_fmt.fmt.pix.field          = V4L2_FIELD_INTERLACED;
	_src_fmt.fmt.pix.pixelformat    = V4L2_PIX_FMT_YUYV;

	MEM_ZERO(_dst_fmt);
	_dst_fmt = _src_fmt;
	_dst_fmt.fmt.pix.pixelformat    = V4L2_PIX_FMT_RGB24;

	// VIDIOC_S_FMT may change width and height.
	if (-1 == ioctl (_video_fd, VIDIOC_S_FMT, &_src_fmt))
		errno_exit ("VIDIOC_S_FMT");

	switch (_io_method) {
	case IO_METHOD_READ:
		init_read (_src_fmt.fmt.pix.sizeimage);
		break;

	case IO_METHOD_MMAP:
		init_mmap ();
		break;

	case IO_METHOD_USERPTR:
		init_userp (_src_fmt.fmt.pix.sizeimage);
		break;
	}
}

void CameraManager::start_capturing () {
	unsigned int i;
	enum v4l2_buf_type type;

	switch (_io_method) {
	case IO_METHOD_READ:
		// Nothing to do.
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < _num_buffers; i++) {
			struct v4l2_buffer buf;

			MEM_ZERO (buf);
			buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory      = V4L2_MEMORY_MMAP;
			buf.index       = i;

			if (-1 == ioctl (_video_fd, VIDIOC_QBUF, &buf))
				errno_exit ("VIDIOC_QBUF");
		}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == ioctl (_video_fd, VIDIOC_STREAMON, &type))
			errno_exit ("VIDIOC_STREAMON");

		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < _num_buffers; i++) {
			struct v4l2_buffer buf;

			MEM_ZERO (buf);
			buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory      = V4L2_MEMORY_USERPTR;
			buf.m.userptr   = (unsigned long) _buffers[i].start;
			buf.length      = _buffers[i].length;

			if (-1 == ioctl (_video_fd, VIDIOC_QBUF, &buf))
				errno_exit ("VIDIOC_QBUF");
		}


		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == ioctl (_video_fd, VIDIOC_STREAMON, &type))
			errno_exit ("VIDIOC_STREAMON");

		break;
	}
}


void CameraManager::mainloop () {
	// read 100 frames for demo.
	unsigned int n_frames = 100;

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(_video_fd, &fds);

	int r;
	unsigned char* raw_img;

	while (n_frames > 0) {

		// wait for I/O readiness forever
		r = select (_video_fd + 1, &fds, NULL, NULL, NULL);

		if (-1 == r && EINTR != errno)
			errno_exit ("select()");

		if (0 == r) {
			cerr << "select() timeout" << endl;
			continue;
			// exit(EXIT_FAILURE);
		}

		raw_img = read_frame();
		n_frames--;

		for (vector<CameraDelegate*>::iterator it = _camera_delegates.begin();
		     it < _camera_delegates.end(); ++it) {
			(*it)->process_image(raw_img,
			                     _src_fmt.fmt.pix.bytesperline/_src_fmt.fmt.pix.width,
			                     _src_fmt.fmt.pix.bytesperline,
			                     _crop_rect.left, _crop_rect.top, _crop_rect.width, _crop_rect.height);
		}
	}
}

void CameraManager::stop_capturing () {
	enum v4l2_buf_type type;

	switch (_io_method) {
	case IO_METHOD_READ:
		// Nothing to do.
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == ioctl (_video_fd, VIDIOC_STREAMOFF, &type))
			errno_exit ("VIDIOC_STREAMOFF");

		break;
	}
}

void CameraManager::uninit_and_close_device () {
	/***** uninit *****/
	unsigned int u;

	switch (_io_method) {
	case IO_METHOD_READ:
		free (_buffers[0].start);
		break;

	case IO_METHOD_MMAP:
		for (u = 0; u < _num_buffers; u++)
			if (-1 == munmap (_buffers[u].start, _buffers[u].length))
				errno_exit ("munmap");
		break;

	case IO_METHOD_USERPTR:
		for (u = 0; u < _num_buffers; u++)
			free (_buffers[u].start);
		break;
	}
	free (_buffers);

	/***** close *****/
	if (-1 == close(_video_fd))
		errno_exit("close");

	_video_fd = -1;
}

void CameraManager::add_camera_delegate(CameraDelegate *cd) {
	_camera_delegates.push_back(cd);
}




unsigned char* CameraManager::read_frame () {
	struct v4l2_buffer buf;
	unsigned char *raw_img;

	int img_size;
	unsigned int u;

	switch (_io_method) {
	case IO_METHOD_READ:

		img_size = read (_video_fd, _buffers[0].start, _buffers[0].length);
		if (img_size == -1) {
			switch (errno) {
			case EAGAIN:
				return 0;
			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */
			default:
				errno_exit ("read()");
			}
		}
		raw_img = (unsigned char *) _buffers[0].start;
		break;

	case IO_METHOD_MMAP:

		MEM_ZERO (buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

		if (-1 == ioctl (_video_fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;
			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */
			default:
				errno_exit ("VIDIOC_DQBUF");
			}
		}

		assert (buf.index < _num_buffers);
		raw_img = (unsigned char *) _buffers[buf.index].start;

		if (-1 == ioctl (_video_fd, VIDIOC_QBUF, &buf))
			errno_exit ("VIDIOC_QBUF");

		break;

	case IO_METHOD_USERPTR:

		MEM_ZERO (buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;

		if (-1 == ioctl (_video_fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;
			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */
			default:
				errno_exit ("VIDIOC_DQBUF");
			}
		}

		for (u = 0; u < _num_buffers; u++)
			if (buf.m.userptr == (unsigned long) _buffers[u].start
				&& buf.length == _buffers[u].length)
				break;

		assert (u < _num_buffers);
		raw_img = (unsigned char *) buf.m.userptr;

		if (-1 == ioctl (_video_fd, VIDIOC_QBUF, &buf))
			errno_exit ("VIDIOC_QBUF");

		break;
	}

	/*
	if (v4lconvert_convert(_v4lconvert_data,
	                       &_src_fmt,
	                       &_dst_fmt,
	                       (unsigned char*) _buffers[buf.index].start, buf.bytesused,
	                       _converted_img_buffer, _dst_fmt.fmt.pix.sizeimage) < 0) {
		if (errno != EAGAIN)
			errno_exit("v4lconvert_convert()");
	}
	*/

	return raw_img;
}

void CameraManager::init_read (unsigned int buffer_size) {
	_buffers = (struct buffer*) calloc (1, sizeof (*_buffers));

	if (!_buffers) {
		cerr << "Error allocating memory on host" << endl;
		exit (EXIT_FAILURE);
	}

	_buffers[0].length = buffer_size;
	_buffers[0].start = malloc (buffer_size);

	if (!_buffers[0].start) {
		cerr << "Error allocating memory on host" << endl;
		exit (EXIT_FAILURE);
	}
}

void CameraManager::init_mmap () {
	struct v4l2_requestbuffers req;

	MEM_ZERO (req);

	// req.count contains the # of frames to request in the device buffer
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.count  = 4;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == ioctl (_video_fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			cerr << _dev_name << " does not support memory mapping" << endl;
			exit (EXIT_FAILURE);
		} else {
			errno_exit ("VIDIOC_REQBUFS");
		}
	}

	// req.count contains the # of frames ready in the device buffer
	// this is just probing
	if (req.count < 2) {
		// requesting 4 frames, but not even getting 2 frames to read
		cerr << "Insufficient buffer memory on " << _dev_name << endl;
		exit (EXIT_FAILURE);
	}


	_buffers = (struct buffer*) calloc(req.count, sizeof(*_buffers));
	if (!_buffers) {
		cerr << "Error allocating memory on host" << endl;
		exit (EXIT_FAILURE);
	}

	for (_num_buffers = 0; _num_buffers < req.count; _num_buffers++) {
		struct v4l2_buffer buf;

		MEM_ZERO (buf);

		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		buf.index	= _num_buffers;

		if (-1 == ioctl (_video_fd, VIDIOC_QUERYBUF, &buf))
			errno_exit ("VIDIOC_QUERYBUF");

		_buffers[_num_buffers].length = buf.length;
		_buffers[_num_buffers].start =
			mmap(NULL,						// start anywhere, for portability
				 buf.length,
				 PROT_READ | PROT_WRITE,	// required
				 MAP_SHARED,				// recommended
				 _video_fd, buf.m.offset);

		if (MAP_FAILED == _buffers[_num_buffers].start)
			errno_exit ("mmap()");
	}
}

void CameraManager::init_userp (unsigned int buffer_size) {
	struct v4l2_requestbuffers req;

	MEM_ZERO(req);
	req.count		= 4;
	req.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory		= V4L2_MEMORY_USERPTR;

	if (-1 == ioctl (_video_fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			cerr << _dev_name << " does not support user pointer I/O" << endl;
			exit (EXIT_FAILURE);
		} else {
			errno_exit ("VIDIOC_REQBUFS");
		}
	}

	_buffers = (struct buffer*) calloc (4, sizeof (*_buffers));

	if (!_buffers) {
		cerr << "Error allocating memory on host" << endl;
		exit (EXIT_FAILURE);
	}

	for (_num_buffers = 0; _num_buffers < req.count; _num_buffers++) {
		_buffers[_num_buffers].length = buffer_size;
		_buffers[_num_buffers].start = malloc (buffer_size);

		if (!_buffers[_num_buffers].start) {
			cerr << "Error allocating memory on host" << endl;
			exit (EXIT_FAILURE);
		}
	}
}

}
