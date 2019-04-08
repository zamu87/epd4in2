# epd4in2

A Node.js package for the 4.2inch e-Paper waveshare display on a Raspberry Pi 2/3/zero, forked from [williwasser's epd7x5 Node.js package](https://github.com/williwasser/epd7x5)

[Link to waveshare wiki](https://www.waveshare.com/wiki/4.2inch_e-Paper_Module)

## Dependencies
1. WiringPi for GPIO access of Raspberry Pi
2. libgd2 for text output and drawing

## Installation
Enable the SPI interface on Raspberry Pi: `sudo raspi-config`

WiringPi: follow installation on [wiringpi.com](http://wiringpi.com/download-and-install/)

libgd2: `sudo apt-get install libgd2-dev # libgd`

epd4in2: `npm install epd4in2`


## Usage example

```javascript
const epd = require('epd4in2')

const font = '/home/pi/bauhaus.ttf';
const fontSize = 26

async function refreshDisplay(message) {
  await epd.init();
  let img = await epd.getImageBuffer('landscape');

  img.filledRectangle(
    Math.round(10), Math.round(10),
    Math.round(80), Math.round(20),
    epd.colors.white)

  let [xll, yll, xlr, ylr, xur, yur, xul, yul] = img.stringFTBBox(epd.colors.white, font, fontSize, 0, 0, 0, message)
 
  // Center the message
  img.stringFT(epd.colors.white, font, fontSize, 0,
    Math.round(img.width / 2 - (xur - xul) / 2),
    Math.round(img.height / 2 + (yll - yul) / 2),
    message)

  img.negate()
 
  await epd.displayImageBuffer(img);
  await epd.sleep();
}
 
 
refreshDisplay("Hello World!")

```

The module exports the following functions and constants:

### Functions:
`epd.init()`

`epd.getImageBuffer('landscape')`
 Use `landscape` to get a buffer oriented in landscape mode.

`epd.displayImageBuffer(img)`

`epd.clear()`
 Equivalent to push a white image

`epd.sleep()`
 Put the display in sleep mode. `init` is required to come back to operations.


### Constants:
`epd.colors.white`

`epd.colors.black`

`epd.width`

`epd.height`

### gd namespace for access of functions on the gd object:
`epd.gd`

example: `epd.gd.createFromFile(path)` to open an image

Documentation of node-gd functions can be found [here](https://y-a-v-a.github.io/node-gd/)

## License

Apache 2.0
