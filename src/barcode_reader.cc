#include <nan.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <zbar.h>
#include <iomanip>
#include <string>
#include <time.h>

using namespace Nan;
using namespace std;
using namespace cv;
using namespace zbar;

class BarcodeReader : public AsyncProgressWorker {
	public:

	BarcodeReader(	Callback *callback,
					Callback *progress,
					char *cam_address,
					int device_number,
					int time_interval,
					int camera_width,
					int camera_height
				):
					AsyncProgressWorker(callback),
					progress(progress),
					cam_address(cam_address),
					device_number(device_number),
					camera_width(camera_width),
					camera_height(camera_height),
					time_interval(time_interval) {}

	~BarcodeReader() {}

	void Execute(const AsyncProgressWorker::ExecutionProgress &progress) {
		VideoCapture cap;
		time_t first_decoded;
		time_t last_decoded;
		string last_barcode_decoded("");

		// Check if we need to open a device by address or id
		if(device_number == -1)
			cap.open(cam_address);
		else
			cap.open(device_number);

		if (!cap.isOpened()) {
			cout << "Could not open camera." << endl;
			exit(EXIT_FAILURE);
		}
		// cout << "Cam opened" << endl;
		if(camera_width!=0)
			cap.set(CV_CAP_PROP_FRAME_WIDTH, camera_width);
		if(camera_height!=0)
			cap.set(CV_CAP_PROP_FRAME_HEIGHT, camera_height);

		// Create a zbar reader
		ImageScanner scanner;

		// Configure the reader
		scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

		for (;;) {

			// Capture an OpenCV frame
			cv::Mat frame, frame_grayscale;
			cap >> frame;

			// Convert to grayscale
			cvtColor(frame, frame_grayscale, CV_BGR2GRAY);

			// Obtain image data
			int width = frame_grayscale.cols;
			int height = frame_grayscale.rows;
			uchar *raw = (uchar *)(frame_grayscale.data);

			// Wrap image data
			Image image(width, height, "Y800", raw, width * height);

			// Scan the image for barcodes
			scanner.scan(image);

			if(image.symbol_begin() != image.symbol_end()) {
				time(&last_decoded);
				double diff = difftime(last_decoded, first_decoded);
				time(&first_decoded);

				// Extract results
				for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {

					const char *data = symbol->get_data().c_str();
					string new_barcode(data);

					if(diff >= time_interval) {
						progress.Send(symbol->get_data().c_str(), symbol->get_data().length()+1);
						last_barcode_decoded = new_barcode;
					}
				}
			}
		}
	}

	void HandleProgressCallback(const char *data, size_t size) {
		HandleScope scope;

		v8::Local<v8::Value> argv[] = {
			New<v8::String>(data).ToLocalChecked()
		};

		progress->Call(1, argv);
	}

	private:
		Callback *progress;
		char *cam_address;
		int device_number;
		int time_interval;
		int camera_width;
		int camera_height;
};

NAN_METHOD(ReadData) {
	Callback *progress = new Callback(info[4].As<v8::Function>());
	Callback *callback = new Callback(info[5].As<v8::Function>());
	char *address = new char[50];
	int device_number = -1;

	if(!info[0]->IsNumber()) {
		v8::String::Utf8Value path(info[0]->ToString());
		strcpy(address, *path);
	} else {
		device_number = To<uint32_t>(info[0]).FromJust();
	}

	AsyncQueueWorker(new BarcodeReader(
		callback,
		progress,
		address,
		device_number,
		To<uint32_t>(info[1]).FromJust(),
		To<uint32_t>(info[2]).FromJust(),
		To<uint32_t>(info[3]).FromJust()
		));
}

NAN_MODULE_INIT(Init) {
	Set(target,
		New<v8::String>("readData").ToLocalChecked(),
		New<v8::FunctionTemplate>(ReadData)->GetFunction());
}

NODE_MODULE(asyncprogressworker, Init)
