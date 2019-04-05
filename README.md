# epd2in7b

A Node.js package for the 2.7inch e-Paper HAT(B) waveshare display on a Raspberry Pi 2/3/zero.
It was forked from [williwasser's epd7x5 Node.js package](https://github.com/williwasser/epd7x5)
It is rewritten in the Node non-blocking way and implements demo code provided by Waveshare for 2.inch e-Paper HAT(B).

[Link to waveshare wiki](https://www.waveshare.com/wiki/2.7inch_e-Paper_HAT_(B))

## Dependencies
1. WiringPi for GPIO access of Raspberry Pi
2. libgd2 for text output and drawing
3. rpi-gpio-buttons for HAT keys handling

## Installation
Enable the SPI interface on Raspberry Pi: `sudo raspi-config`

WiringPi: follow installation on [wiringpi.com](http://wiringpi.com/download-and-install/)

libgd2: `sudo apt-get install libgd2-dev # libgd`

epd2in7b: `npm install epd2in7b`


## Usage example

```javascript
const epd = require('epd2in7b')
const font = '/home/marco/fonts/Roboto-Regular.ttf';
const fontSize = 12

const img = epd.getImageBuffer('landscape');
const width = epd.height
const height = epd.width
let epdp = epd.init({fastLut: false})

const refreshDisplay = message =>
  epdp = epdp
    // init is required since we set it sleeping at the end of this chain
    .then(() => epd.init({fastLut: false}))
    .then(() => img.then(img => {
      // display a black rectangle
      img.filledRectangle(
        Math.round(width / 8), Math.round(height / 8),
        Math.round(7 * width / 8), Math.round(7 * height / 8),
        epd.colors.black)

      // display a red rectangle
      img.filledRectangle(
        Math.round(width / 4), Math.round(height / 4),
        Math.round(3 * width / 4), Math.round(3 * height / 4),
        epd.colors.red)

      // Retrieve bounding box of displayed string
      let [xll, yll, xlr, ylr, xur, yur, xul, yul] = img.stringFTBBox(epd.colors.white, font, fontSize, 0, 0, 0, message)

      // Center the message
      img.stringFT(epd.colors.white, font, fontSize, 0,
        Math.round(width / 2 - (xur - xul) / 2),
        Math.round(height / 2 + (yll - yul) / 2),
        message)

      return epd.displayImageBuffer(img)
    }))
    .then(() => epd.sleep())

refreshDisplay("Hello world !")

// Handle buttons
epd.buttons.handler.then(handler =>
  handler.on('pressed', function (button) {
    let buttonLabel = 'none'
    switch (button) {
      case epd.buttons.button1:
        buttonLabel = 'first button'
        break
      case epd.buttons.button2:
        buttonLabel = 'second button'
        break
      case epd.buttons.button3:
        buttonLabel = 'third button'
        break
      case epd.buttons.button4:
        buttonLabel = 'fourth button'
        break
      default:
        buttonLabel = 'an unknown button'
    }
    refreshDisplay(`You pressed \n${buttonLabel}`)
  })
)

// Handle exit
function exitHandler (options, err) {
  let promise = null
  if (options.cleanup) {
    promise = img.then(img => img.destroy())
  }

  if (err && err.stack) {
    console.log(err.stack)
  }

  if (options.exit) {
    if (promise !== null) {
      promise.then(() => process.exit())
    } else {
      process.exit()
    }
  }
}

process.on('exit', exitHandler.bind(null, {cleanup: true, exit: true}))
process.on('SIGINT', exitHandler.bind(null, {exit: true}))
process.on('SIGUSR1', exitHandler.bind(null, {exit: true}))
process.on('SIGUSR2', exitHandler.bind(null, {exit: true}))
process.on('uncaughtException', exitHandler.bind(null, {exit: true}))

```

The module exports the following functions and constants:

### Functions:
`epd.init({fastLut: false}))`
`fastLut` parameter is an attempt to speed up refresh process using LUT alternative table on the Web. It doesn't work work well with red color.

`epd.getImageBuffer('landscape')`
 Use `landscape` to get a buffer oriented in landscape mode.

`epd.displayImageBuffer(img)`

`epd.clear()`
 Equivalent to push a white image

`epd.sleep()`
 Put the display in sleep mode. `init` is required to come back to operations.

`epd.buttons.handler`
 The `rpi-gpio-buttons` instance


### Constants:
`epd.buttons.button1`

`epd.buttons.button2`

`epd.buttons.button3`

`epd.buttons.button4`

`epd.colors.white`

`epd.colors.black`

`epd.colors.red`

`epd.width`

`epd.height`

### gd namespace for access of functions on the gd object:
`epd.gd`

example: `epd.gd.createFromFile(path)` to open an image

Documentation of node-gd functions can be found [here](https://y-a-v-a.github.io/node-gd/)

## License

Apache 2.0

## Todos
Handle errors
