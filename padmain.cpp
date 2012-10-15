#include "OCRer.h"
#include "CameraManager.h"
#include "types.h"
#include <sstream>
#include <getopt.h>

int main (int argc, char **argv) {
	using namespace std;

	/***** get the cmdline options *****/
	// set default values.
	string      dev_name      = "/dev/video0";
	io_method   iom           = IO_METHOD_MMAP;

	string usage1 = "Usage: ";
	string usage2 = " [options]\n\n"
		"Options:\n"
		"-h | --help			Print this message\n"
		"-d | --device name		Video device name (default /dev/video0)\n"
		"-m | --mmap			Use memory mapped buffers (default)\n"
		"-r | --read			Use read() calls\n"
		"-u | --userp			Use application allocated buffers\n";

	const char short_options[] = "d:hmru";
	const struct option long_options [] = {
		{ "device",     required_argument,      NULL,       'd' },
		{ "help",       no_argument,            NULL,       'h' },
		{ "mmap",       no_argument,            NULL,       'm' },
		{ "read",       no_argument,            NULL,       'r' },
		{ "userp",      no_argument,            NULL,       'u' },
		{}
	};

	while(1) {
		int index;
		int v;
		ostringstream oss;

		v = getopt_long (argc, argv,
		                 short_options, long_options,
		                 &index);
		if (v == -1)
			break;

		switch (v) {
		case 'd':
			if (isdigit (optarg[0])) {
				oss << "/dev/video" << atoi(optarg);
				dev_name = oss.str();
			} else {
			 	dev_name = optarg;
				cout << "dev_name = " << dev_name << endl;
			}
			break;

		case 'h':
			cout << usage1 << argv[0] << usage2;
			exit (EXIT_SUCCESS);

		case 'm':
			break;

		case 'r':
			iom = IO_METHOD_READ;
			break;

		case 'u':
			iom = IO_METHOD_USERPTR;
			break;

		default:
			cerr << usage1 << argv[0] << usage2;
			exit (EXIT_FAILURE);
		}
	}


	/***** alloc-init'ng objects based on cmdline options *****/
	using namespace pad;
	CameraManager *camera_manager = new CameraManager(dev_name, iom);
	OCRer *ocrer = new OCRer();
	camera_manager->add_camera_delegate(ocrer);

	camera_manager->open_and_init_device();
	camera_manager->start_capturing();
	camera_manager->mainloop();
	camera_manager->stop_capturing();
	camera_manager->uninit_and_close_device();

	delete camera_manager;
	delete ocrer;

	return EXIT_SUCCESS;
}
