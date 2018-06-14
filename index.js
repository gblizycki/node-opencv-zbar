var barcodeReader 	= require('./build/Release/asyncprogressworker');
var _ 				= require('underscore');

var BarcoderReader = function() {

	/**
	 * Reads the barcodes captured in a video file or cam.
	 *
	 * @param 'address' Address to a ipcam or video file or number of a webcam device
	 * @param 'time' Optional value, this is the time interval that the application uses to 
	 * evaluate new barcodes
	 * @param 'callback' Function called when barcode is captured
	 */
	function readData(address, config, callback) {
		if(_.isFunction(config)) {
			callback = config;
			config = {
				time: 0,
				width: 0,
				height: 0
			};
		}
		barcodeReader.readData(address, config.time, config.width, config.height, callback);
	}

	return {
		readData : readData
	}
}
module.exports = BarcoderReader();