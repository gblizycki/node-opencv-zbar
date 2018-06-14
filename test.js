var barcodeReader = require('./index');
console.log(barcodeReader);

var callback = function(data) {
	console.log('callback');
};

var readData = function(data) {	
	console.log(data);
};

// barcodeReader.readData('rtsp://admin:admin@192.168.1.113:554/', readData);
barcodeReader.readData(0, {time: 1, width: 100, height: 100}, readData);