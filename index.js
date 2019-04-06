const epd4in2 = require('./build/Release/epd4in2');
const gd = require('node-gd');

const width = epd4in2.width();
const height = epd4in2.height();

function getImageBuffer(orientation) {
	return new Promise(resolve => {
		let img
		if (orientation === undefined || orientation === 'portrait') {
			img = gd.createSync(height, width);
		} else {
			img = gd.createSync(width, height);
		}

		for (let i=0; i<256; i++) {
			img.colorAllocate(i, i, i); 
		}
		
		return resolve(img);
	})
}

function displayImageBuffer(img) {
	return new Promise(resolve => {
		// let buf = new Buffer(width * height/2);
		// for(let y = 0; y < height; y++) { 
		// 	for(let  x = 0; x<width-1; x+=2){
		// 		let pixel_0 = img.getPixel(x, y);
		// 		let pixel_1 = img.getPixel(x+1, y);
				
		// 		if (pixel_0 < 64)
		// 			if (pixel_1 < 64)
		// 				buf[(y*width+x)/2] = 0x33;
		// 			else if (pixel_1 < 192)
		// 				buf[(y*width+x)/2] = 0x34;
		// 			else
		// 				buf[(y*width+x)/2] = 0x30; 
		// 		else if (pixel_0 < 192) 
		// 			if (pixel_1 < 64)
		// 				buf[(y*width+x)/2] = 0x43;
		// 			else if (pixel_1 < 192)
		// 				buf[(y*width+x)/2] = 0x44;
		// 			else	
		// 				buf[(y*width+x)/2] = 0x40;		
		// 		else
		// 			if (pixel_1 < 64)
		// 				buf[(y*width+x)/2] = 0x03;
		// 			else if (pixel_1 < 192)
		// 				buf[(y*width+x)/2] = 0x04;
		// 			else
		// 				buf[(y*width+x)/2] = 0x00; 
				
		// 	}
		// }

		let buf = new Buffer.alloc(width * height, 0);
		for(let y = 0; y < height; y++) {
			for(let x = 0; x < width; x++) {
				let color = img.height == height
					? img.getPixel(x, y)
					: img.getPixel(img.width - y, x);
				if (color < 128) { // black
					buf[ x + y * width ] = 0x00;
				} else { // white
					buf[ x + y * width ] = 0xff;
				}
			}
		}
		epd4in2.displayFrame(
			buf,
			() => {
				resolve();
			}
		);
	})
}

exports.getImageBuffer = getImageBuffer;

exports.displayImageBuffer = displayImageBuffer;

exports.init = () => new Promise(resolve => {
	epd4in2.init(() => {
		resolve();
	});
})

exports.clear = () => new Promise(resolve => {
	epd4in2.clear(() => {
		resolve();
	});
})

exports.sleep = () => new Promise(resolve => {
	epd4in2.sleep(() => {
		resolve();
	});
})

exports.colors = {
	white: 255,
	black: 0
}

exports.width = width;
exports.height = height;
exports.gd = gd;