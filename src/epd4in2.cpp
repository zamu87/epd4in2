#include <node.h>
#include <iostream>
#include <functional>
#include <nan.h>

using namespace std;

#include "epdif.h"

using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Boolean;
using v8::Value;
using v8::Context;
using v8::Array;

// Async worker
class Epd4In2AsyncWorker : public Nan::AsyncWorker {
  public:
    function <void ()> method;

    Epd4In2AsyncWorker(function <void ()> method, Nan::Callback *callback)
      : Nan::AsyncWorker(callback) {
      this->method = method;
    }

    void Execute() {
      this->method();
    }

    void HandleOKCallback() {
      Nan::HandleScope scope;
      v8::Local<v8::Value> argv[] = {
        Nan::Null(), // no error occured
        Nan::Null()
      };
      callback->Call(2, argv);
    }

    void HandleErrorCallback() {
      Nan::HandleScope scope;
      v8::Local<v8::Value> argv[] = {
        Nan::New(this->ErrorMessage()).ToLocalChecked(), // return error message
        Nan::Null()
      };
      callback->Call(2, argv);
    }
};

// Display resolution
#define EPD_WIDTH       400
#define EPD_HEIGHT      300

// EPD4IN2 commands
#define PANEL_SETTING                               0x00
#define POWER_SETTING                               0x01
#define POWER_OFF                                   0x02
#define POWER_OFF_SEQUENCE_SETTING                  0x03
#define POWER_ON                                    0x04
#define POWER_ON_MEASURE                            0x05
#define BOOSTER_SOFT_START                          0x06
#define DEEP_SLEEP                                  0x07
#define DATA_START_TRANSMISSION_1                   0x10
#define DATA_STOP                                   0x11
#define DISPLAY_REFRESH                             0x12
#define DATA_START_TRANSMISSION_2                   0x13
#define LUT_FOR_VCOM                                0x20
#define LUT_WHITE_TO_WHITE                          0x21
#define LUT_BLACK_TO_WHITE                          0x22
#define LUT_WHITE_TO_BLACK                          0x23
#define LUT_BLACK_TO_BLACK                          0x24
#define PLL_CONTROL                                 0x30
#define TEMPERATURE_SENSOR_COMMAND                  0x40
#define TEMPERATURE_SENSOR_SELECTION                0x41
#define TEMPERATURE_SENSOR_WRITE                    0x42
#define TEMPERATURE_SENSOR_READ                     0x43
#define VCOM_AND_DATA_INTERVAL_SETTING              0x50
#define LOW_POWER_DETECTION                         0x51
#define TCON_SETTING                                0x60
#define RESOLUTION_SETTING                          0x61
#define GSST_SETTING                                0x65
#define GET_STATUS                                  0x71
#define AUTO_MEASUREMENT_VCOM                       0x80
#define READ_VCOM_VALUE                             0x81
#define VCM_DC_SETTING                              0x82
#define PARTIAL_WINDOW                              0x90
#define PARTIAL_IN                                  0x91
#define PARTIAL_OUT                                 0x92
#define PROGRAM_MODE                                0xA0
#define ACTIVE_PROGRAMMING                          0xA1
#define READ_OTP                                    0xA2
#define POWER_SAVING                                0xE3

// original
const unsigned char lut_vcom0[] = {
0x00, 0x17, 0x00, 0x00, 0x00, 0x02,        
0x00, 0x17, 0x17, 0x00, 0x00, 0x02,        
0x00, 0x0A, 0x01, 0x00, 0x00, 0x01,        
0x00, 0x0E, 0x0E, 0x00, 0x00, 0x02,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_ww[] = {
  0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_bw[] = {
  0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_wb[] = {
  0x80, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x80, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0x50, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_bb[] = {
  0x80, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x80, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0x50, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void SendCommand(UBYTE command)
{
    EpdIf::DigitalWrite(DC_PIN, LOW);
    EpdIf::DigitalWrite(CS_PIN, LOW);
    EpdIf::SpiTransfer(command);
    EpdIf::DigitalWrite(CS_PIN, HIGH);
}

void SendData(UBYTE data)
{
    EpdIf::DigitalWrite(DC_PIN, HIGH);
    EpdIf::DigitalWrite(CS_PIN, LOW);
    EpdIf::SpiTransfer(data);
    EpdIf::DigitalWrite(CS_PIN, HIGH);
}

void WaitUntilIdle(void)
{
    while(EpdIf::DigitalRead(BUSY_PIN) == 0) {      //0: busy, 1: idle
        EpdIf::DelayMs(100);
    }
}

void Reset(void)
{
    EpdIf::DigitalWrite(RST_PIN, HIGH);
    EpdIf::DelayMs(200);
    EpdIf::DigitalWrite(RST_PIN, LOW);                //module reset
    EpdIf::DelayMs(200);
    EpdIf::DigitalWrite(RST_PIN, HIGH);
    EpdIf::DelayMs(200);
}

void TurnOnDisplay(void)
{
    SendCommand(DISPLAY_REFRESH);
    EpdIf::DelayMs(100);
    WaitUntilIdle();
}

void SetLut(void)
{
    unsigned int count;

    SendCommand(LUT_FOR_VCOM);                            //vcom
    for(count = 0; count < 44; count++) {
        SendData(lut_vcom0[count]);
    }

    SendCommand(LUT_WHITE_TO_WHITE);                      //ww --
    for(count = 0; count < 42; count++) {
        SendData(lut_ww[count]);
    }

    SendCommand(LUT_BLACK_TO_WHITE);                      //bw r
    for(count = 0; count < 42; count++) {
        SendData(lut_bw[count]);
    }

    SendCommand(LUT_WHITE_TO_BLACK);                      //wb w
    for(count = 0; count < 42; count++) {
        SendData(lut_wb[count]);
    }

    SendCommand(LUT_BLACK_TO_BLACK);                      //bb b
    for(count = 0; count < 42; count++) {
        SendData(lut_bb[count]);
    }
}

void width(const FunctionCallbackInfo<Value>& args)
{
  Isolate* isolate = args.GetIsolate();
  Local<Number> num = Number::New(isolate, EPD_WIDTH);
  args.GetReturnValue().Set(num);
}

void height(const FunctionCallbackInfo<Value>& args)
{
  Isolate* isolate = args.GetIsolate();
  Local<Number> num = Number::New(isolate, EPD_HEIGHT);
  args.GetReturnValue().Set(num);
}


void init_sync(void)
{
  if (EpdIf::IfInit() != 0) {
    // TODO : throw error
  }	else {
    Reset();

    SendCommand(POWER_SETTING);			//POWER SETTING
    SendData(0x03);
    SendData(0x00);
    SendData(0x2b);
    SendData(0x2b);

    SendCommand(BOOSTER_SOFT_START);         //boost soft start
    SendData(0x17);		//A
    SendData(0x17);		//B
    SendData(0x17);		//C

    SendCommand(POWER_ON);
    WaitUntilIdle();

    SendCommand(PANEL_SETTING);			//panel setting
    SendData(0xbf);		//KW-BF   KWR-AF	BWROTP 0f	BWOTP 1f
    SendData(0x0d);

    SendCommand(PLL_CONTROL);			//PLL setting
    SendData(0x3C);      	// 3A 100HZ   29 150Hz 39 200HZ	31 171HZ

    SendCommand(0x61);			//resolution setting
    SendData(0x01);
    SendData(0x90);       //128
    SendData(0x01);		//
    SendData(0x2c);

    SendCommand(0x82);			//vcom_DC setting
    SendData(0x28);

    SendCommand(0X50);			//VCOM AND DATA INTERVAL SETTING
    SendData(0x97);		//97white border 77black border		VBDF 17|D7 VBDW 97 VBDB 57		VBDF F7 VBDW 77 VBDB 37  VBDR B7
    
    SetLut();
    
    // TODO: return 0;
	}
}

void init(const FunctionCallbackInfo<Value>& args)
{
  Nan::AsyncQueueWorker(new Epd4In2AsyncWorker(
    bind(init_sync),
    new Nan::Callback(args[0].As<v8::Function>())
  ));
}


void display(UBYTE * image)
{
  UWORD Width, Height;
  Width = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
  Height = EPD_HEIGHT;
  
  SendCommand(DATA_START_TRANSMISSION_1);
  for (UWORD j = 0; j < Height; j++) {
      for (UWORD i = 0; i < Width; i++) {
          SendData(0XFF);
      }
  }
  SendCommand(DATA_START_TRANSMISSION_2);
  for (UWORD j = 0; j < Height; j++) {
      for (UWORD i = 0; i < Width; i++) {
          // SendData(image[i + j * Width]);
          UBYTE buffer = 0xff;
          for (UWORD k = 0; k < 8; k++) {
            buffer = (buffer << 1) | (image[ 8 * (i + j * Width) + k ] & 0x01);
          }
          SendData(buffer);
      }
  }
  TurnOnDisplay();
  WaitUntilIdle();
}

void displayFrame(const FunctionCallbackInfo<Value>& args)
{
  UBYTE* imageData = NULL;

  if ( ! args[0]->IsNull()) {
  	v8::Local<v8::Uint8Array> blackView = args[0].As<v8::Uint8Array>();
  	void *blackData = blackView->Buffer()->GetContents().Data();
  	imageData = static_cast<UBYTE*>(blackData);
  }

  Nan::AsyncQueueWorker(new Epd4In2AsyncWorker(
    bind(display, imageData),
    new Nan::Callback(args[1].As<v8::Function>())
  ));
}


void clear_sync(void)
{
    UWORD Width, Height;
    Width = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    Height = EPD_HEIGHT;

    SendCommand(DATA_START_TRANSMISSION_1);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            SendData(0xFF);
        }
    }

    SendCommand(DATA_START_TRANSMISSION_2);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            SendData(0xFF);
        }
    }

    TurnOnDisplay();
    WaitUntilIdle();
}

void clear(const FunctionCallbackInfo<Value>& args)
{
  Nan::AsyncQueueWorker(new Epd4In2AsyncWorker(
    bind(clear_sync),
    new Nan::Callback(args[0].As<v8::Function>())
  ));
}


void sleep_sync(void)
{
  SendCommand(POWER_OFF);
  WaitUntilIdle();
  SendCommand(DEEP_SLEEP);
  SendData(0XA5);
}

void sleep(const FunctionCallbackInfo<Value>& args)
{
  Nan::AsyncQueueWorker(new Epd4In2AsyncWorker(
    bind(sleep_sync),
    new Nan::Callback(args[0].As<v8::Function>())
  ));
}


void InitAll(Local<Object> exports)
{
  NODE_SET_METHOD(exports, "init", init);
  NODE_SET_METHOD(exports, "clear", clear);
  NODE_SET_METHOD(exports, "sleep", sleep);
  NODE_SET_METHOD(exports, "width", width);
  NODE_SET_METHOD(exports, "height", height);
  NODE_SET_METHOD(exports, "displayFrame", displayFrame);
}

NODE_MODULE(epd4in2, InitAll)
