
# ESP8266 based 3D-Printer Status Lamp

Simple Sketch to display the printer state connected to Klipper/Moonraker on a Neopixel (WS2812b or similar)

This Sketch is a fork of [FrYakaTKoP/simple-octo-ws2812](https://github.com/FrYakaTKoP/simple-octo-ws2812) made to be compatible with Klipper/Moonraker instead of Octoprint.

## Description:
The color of the lamp will show you the status as follows:

-flash white: booting / connecting  
-fade rainbow: Printer is ready  
-fade yellow: Printer is printing  
-fade blue: Print is paused  
-fade red: error occurred  
-red/blue: HTTP Code Unauthorized  
-fade green: HTTP Code Conflict  

It will also output small informations on the serial line.

# Hardware


## Electronics

BOM:
* 1x Wemos d1 mini (or any esp8266 with 4Mb flash)
* 1x WS2812b RGB Led
* 1x Wires (3 cores, about 3-4cm)
* 1x Micro USB Cable and 5V Power supply
![schematic](https://raw.githubusercontent.com/FrYakaTKoP/simple-octo-ws2812/master/doc/electronics/sch/illumination-cat-electronic-wiring.png)


## Printed Parts

For best results using this Sketch in combination with the almighty **Illumination Cat** found on thingiverse is recommended.

https://www.thingiverse.com/thing:2974862

# Software Installation

The Software is written especially for the ESP8266 from Espressif, using the Arduino Framework.
There are two ways to setup the Development Environment and then flash the Software to the ESP8266

* First and recommended is platform.io installed in the Atom Editor (you could use any platformio compatible editor, even pure cli)
* Second is Arduino IDE with ESP8266 Core.


Also before you can upload any sketch to the wemos you will need to install the correct driver for the USB to serial converter chip on the wemos module.  
This can be found here:
https://wiki.wemos.cc/downloads

## The Platform.io way (recommended)

* Download/Clone this Repository and unpack it on your favourite folder
* Download and install Atom Editor and the platformio IDE Package..
	http://docs.platformio.org/en/latest/ide/atom.html#installation

* when platformio is up an running, open the Project with **File -> Open Folder** (browse the unpacked repository then click select folder)  
![open folder](https://raw.githubusercontent.com/FrYakaTKoP/simple-octo-ws2812/master/doc/img/sc\_pio\_openfolder.png)

* Click **PlatformIO: Build**  
![pio build](https://raw.githubusercontent.com/FrYakaTKoP/simple-octo-ws2812/master/doc/img/sc_pio_build.png)  
 Now platformio will download the needed dependencies (you need to be connected to the Internet)  
![pio build success](https://raw.githubusercontent.com/FrYakaTKoP/simple-octo-ws2812/master/doc/img/sc_pio_build_success.png)

* After PIO Build is finished you can now change the User settings in the **/src/config.h** file according {TBD} link to config description
* Click **PlatformIO: Upload** to upload the sketch to the ESP
  *you may need to try the upload a couple times, those modules can be hard to program via bootloader. You could also check the connection using the serial monitor*
	![pio upload](https://raw.githubusercontent.com/FrYakaTKoP/simple-octo-ws2812/master/doc/img/sc_pio_upload.png)
* enjoy
{TBD} add pic of the almighty **Illumination Cat** in all its glory

## The Arduino IDE Way

* Download/Clone this Repository and unpack it on your favourite folder
*  Download and install the latest version of the Arduino IDE  for your system  https://www.arduino.cc/en/Main/Software

* Add ESP Board Manager to your IDE  :
	Therefore open File>Preferences
	After "Additional Boards Manager URLs" insert: http://arduino.esp8266.com/stable/package_esp8266com_index.json

![001](https://raw.githubusercontent.com/FrYakaTKoP/simple-octo-ws2812/master/doc/img/001.png)  
	Then open Tools>Board>Boards Manager
	Search for ESP
	Install latest ESP8266 Board Manager
	![002](https://raw.githubusercontent.com/FrYakaTKoP/simple-octo-ws2812/master/doc/img/002.png)

* Add Additional Libraries
	Therefore open Sketch>Include Library>Manage Libraries...
	Search for Adafruit Neopixel
	Install latest Adafruit Neopixel library
	![003](https://raw.githubusercontent.com/FrYakaTKoP/simple-octo-ws2812/master/doc/img/003.png)  
	Search ArduinoJson
	Install latest ArduinoJson	![004](https://raw.githubusercontent.com/FrYakaTKoP/simple-octo-ws2812/master/doc/img/004.png)
* Restart Arduino IDE
* Open config.h and add your configuration
* Choose your board and com port
* Upload

# Configuration

There are 3 main configurations in the config.h file, which must be made by user...

1. Wifi SSID `const char\* SSID = "myWifi";`  
   The name of your Wireless Network

2. Wifi Password `const char\* SSID = "myWifi";`  
   The password to your Wireless Network

3. Printer Ip Address `String PRINTER_IP = "192.168.0.xxx";`  
   The Network address to your Printer. Same as you need to gain acces to your web interface.

## Example User Config:

```
#define LEDPIN D2 // where you Led is connected
const char* SSID = "myWifi";
const char* WPWD = "wifipassword";
String PRINTER_IP = "192.168.1.3";
uint16_t pollInterval = 500;
const uint8_t lenght = 1;
```


