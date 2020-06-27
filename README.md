# diyscip
DIY Supervision and Control Intxx PxxxSPA with an ESP8266

:fr: [En Français](https://github.com/YorffoeG/diyscip/blob/master/README.fr.md)

---

This projet aims to add remote supervision and control ability to the Intxx PxxxSPA without altering the initial product. So this controller card takes place between the control panel and the motor block, through its specific 5 pins connector.

It connects to your WiFi network and communicate to a MQTT server for control (MQTT publish) and command (MQTT subscribe).

> :warning: **disclaimer:** This project is not affiliated with Intxx. It is distributed in the hope it will be useful but WITHOUT ANY WARRANTY. Any damaged on your spa or lost of original product warranty is in your own responsibility, including any consequences of using this project.

> :warning: **compatibility:** This project has been developed and test on model SSP-H20-1C only. I guess it should works with SSP-H-20-1 and SSP-H-20-2 because those models share the same control panel. For the others models, some minors software AND hardware modifications may be necessary. Feel free to contact me to share our experience and improve this project compatibility.

### How to build it ?
There are several types of hardware implementations:
1. single CD4051 
2. dual CD4051
3. no extra PCB hardware

Depending on your hardware and the pool type you need to edit `config.h` and choose PCB DESIGN and SPA MODEL.

#### 1. The original implementation (Single CD4051)
![image](docs/controller_1.jpg)

You need first the hardware ! Components are easy to find but still, it need some skills to build it.

The main part is a NodeMcu V3, based on the esp8266. I used [this one](https://www.amazon.fr/dp/B06Y1ZPNMS) for prototyping. Using a NodeMcu development board rather than a simple ESP8266 chip is for convenient: it's include a 5V (from SPA) to 3.3v converter and mainly it offers a USB connection for upload and debug software. Electronic schematic is [here](https://github.com/YorffoeG/diyscip/blob/master/docs/schematic.jpg).

As an IDE, i use [Visual Studio Code](https://code.visualstudio.com/) with [PlatformIO](https://platformio.org/): Free and in my opinion, it offers a better usability for source management.

For the spa connectors which are Intxx design specific, they need to be 3D printed. I use [those ones](https://www.thingiverse.com/thing:4130911) by Psykokwak. Thanks for the share ! :+1:


Don't feel comfortable with hardware manufacturing ? That's a Do It Yourself project :smile: But feel free to contact me at _diyscip(AT)runrunweb.net_, i may have some ready to use controller in my pocket. But keep in mind it's your responsibility to use it on your spa.

#### 2. ??? (Dual CD4051)

#### 3. Button push by software (no extra PCB)
![image](https://github.com/UlrichMai/MaiPureSpaController/raw/master/docs/D1_mini_mounted_on_display_panel_back.jpg)

The mini version of 8266 is taped directly on the back of the display panel and wired to the connector on the display PCB.
Please see [here](https://github.com/UlrichMai/MaiPureSpaController#hardware) for more information on this hardware setup.

### Settings the controller
Here we are ! The controller is in the place. At first start up, you need to connect your mobile or computer to the WiFi network "DIYSCIP_setup", a configuration screen enable you to set your home WiFi network and password then setting the MQTT server you connect to. After a checking of you settings, the controller reboots and start to operate !

To re-enter in setup mode, when your spa is powered on, press the temperature units change button 6 times quickly then switch off the spa (not electricaly but on control panel). The controller reboot and enter in setup mode.

![image](https://github.com/YorffoeG/diyscip/blob/master/docs/DIYSCIP_settings.jpg)


### Middleware and Frontend PWA App
I've also developped the middleware server and a frontend Progressive Web App for your spa ! For now it's not publicly open but email me if you want your controller to be registered in this app: _diyscip(AT)runrunweb.net_
You can also take a free demo tour : https://diyscip.web.app

![image](https://github.com/YorffoeG/diyscip/blob/master/docs/frontend_app.jpg)

### Behind the code
Based on reverse engineering of control panel electronics, 3 wires provide a clock, a data and a hold signal to two 8 bit shift registers in serial (74HC595). The outputs of those registers enable or not the leds (including those of the 7-digits displays). Each buttons are also connected to a register output. When it's pressed the corresponding output register is connected with data signal through a 1kOhm resistor.

> The corresponding function of each register output is probably depending of the spa models. This is the point that need to be adapted in software/hardware to others models than the SSP-H20-1C.

The DIYSCIP controller is plugged in parallel with signal from control panel and motor block.
For supervision, the onboard esp8266 read at clock frequency the data signal then the hold signal cut the flow to a 16 bits frame. By decoding this frame, you get the state of the leds and of the 7 digits display on control panel.

For control, it's a little be more tricky. The DIYSCIP hardware duplicates the shift register electrical logic. Then the esp8266 emulate pushed button by addressing an analogic multiplexer whom ouput become connected to the data signal on demand.

That's all ! now it can read and write states on spa, quite easy no ? :smile:

An thermistor complement the electronic parts. As it's on board and take place inside the control panel colomn it is not very occurate, but still, it's an informative data about the weather condition.

At last, a MQTT client send and receive control/command from a MQTT server through a TCP connexion (not a Websocket).


### The MQTT topics
The controller embedded MQTT client uses protocol v3.1.1.

Publish Topics:
- **spa/status** :  online | offline (Will topic)
- **spa/state** : number - raw state value, a bit per control, for debug mainly.
- **spa/state/power**  :  true | false
- **spa/state/filter** : true | false
- **spa/state/heater** : true | false
- **spa/state/heatreached** : true | false (true if heat is reached)
- **spa/state/bubble** : true | false
- **spa/temp/board** : number - in Celsius degree
- **spa/temp/water** : number - in Celsius degree
- **spa/temp/desired** : number - in Celsius degree

Subscribe Topics:
- **spa/state/power/set** : true | false
- **spa/state/filter/set** : true | false
- **spa/state/heater/set** : true | false
- **spa/state/temp/desired/set** : number - in Celsius degree
