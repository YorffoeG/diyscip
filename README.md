# diyscip
DIY Supervision and Control Intxx PxxxSPA with an ESP8266

:fr: [En Français](https://github.com/YorffoeG/diyscip/blob/master/README.fr.md)

---

This projet aims to add remote supervision and control ability to the Intxx PxxxSPA without altering the initial product. So this controller card takes place between the control panel and the motor block, through its specific 5 pins connector.

It connects to your WiFi network and communicate to a MQTT server for control (MQTT publish) and command (MQTT subscribe).

> :warning: **disclaimer:** This project is not affiliated with Intxx. It is distributed in the hope it will be useful but WITHOUT ANY WARRANTY. Any damaged on your spa or lost of original product warranty is in your own responsibility, including any consequences of using this project.


![image](https://github.com/YorffoeG/diyscip/blob/master/docs/controller_PCB_V2.jpg)

### How to build it ?
You need first the hardware ! Components are easy to find but still, it need some skills to build it.

As an IDE, i use [Visual Studio Code](https://code.visualstudio.com/) with [PlatformIO](https://platformio.org/): Free and in my opinion, it offers a better usability for source management.

For the spa connectors which are Intxx design specific, they need to be 3D printed. I use [those ones](https://www.thingiverse.com/thing:4130911) by Psykokwak. Thanks for the share ! :+1:

#### PCB_V1
Originaly, V1 design was built with a NodeMcu V3, based on the esp8266. I used [this one](https://www.amazon.fr/dp/B06Y1ZPNMS) for prototyping. Using a NodeMcu development board rather than a simple ESP8266 chip is for convinient: it's include a 5V (from SPA) to 3.3v converter and mainly it offers a USB connection for upload and debug software. Electronic schematic is [here](https://github.com/YorffoeG/diyscip/blob/master/docs/schematic_PCB_V1.jpg).

**It's only supporting SSP-xxx spa models**

#### PCB_V2
Based on PCB_V1 experience, V2 design improve electical compatibility and add support to SJB models. It use directly an esp8266 and add electrical level shifters from TTL signal (spa) to CMOS signal (esp8266). I know there are lots of debate on Internet to know if esp8266 IO are or not 5V tolerant, even it seems to work with TTL signal, it's not afficialy supported, so i prefered to respect the rules of art. A 5V to 3.3v voltage converter also ensure esp8266 power supply.
A second analog multiplexer is added to make the pcb more generic and enable a compatibility to all Intxx spa model.
Electronic schematic is [here](https://github.com/YorffoeG/diyscip/blob/master/docs/schematic_PCB_V2.jpg).

**It's now supporting SSP-xxx and SJB-xxx spa model**



Don't feel comfortable with hardware manufacturing ? That's a Do It Yourself project :smile: But feel free to contact me at _diyscip(AT)runrunweb.net_, i may have some ready to use controller in my pocket. But keep in mind it's your responsibility to use it on your spa.

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


The DIYSCIP controller is plugged in parallel with signal from control panel and motor block.
For supervision, the onboard esp8266 read at clock frequency the data signal then the hold signal cut the flow to a 16 bits frame. By decoding this frame, you get the state of the leds and of the 7 digits display on control panel.

For control, it's a little be more tricky. The DIYSCIP hardware duplicates the shift register electrical logic. Then the esp8266 emulate pushed button by addressing an analogic multiplexer whom ouput become connected to the data signal on demand.

That's all ! now it can read and write states on spa, quite easy no ? :smile:

An thermistor complement the electronic parts. As it's on board and take place inside the control panel colomn it is not very occurate, but still, it's an informative data about the weather condition.

At last, a MQTT client send and receive control/command from a MQTT server through a TCP connexion (not a Websocket).


### The MQTT topics
The controller embedded MQTT client uses protocol v3.1.1.

Publish Topics:
- **spa/model** : _string_ - the spa model configured
- **spa/status** :  online | offline (Will topic)
- **spa/sys/version** : _string_ - the firmware version
- **spa/sys/wifi** : _number_ - Wifi signal level from 0 to 100 (below 20 is very poor)
- **spa/state** : _number_ - raw state value, a bit per control, for debug mainly.
- **spa/state/power**  :  on | off
- **spa/state/filter** : on | off
- **spa/state/heater** : on | off
- **spa/state/heatreached** : on | off (on if heat is reached)
- **spa/state/bubble** : on | off
- **spa/temp/board** : _number_ - in Celsius degree
- **spa/temp/water** : _number_ - in Celsius degree
- **spa/temp/desired** : _number_ - in Celsius degree
- **spa/sanitizer** : _number_ - (SJB model only) hours left of sanitizer, 0 if off

Subscribe Topics:
- **spa/state/power/set** : on | off
- **spa/state/filter/set** : on | off
- **spa/state/heater/set** : on | off
- **spa/temp/desired/set** : _number_ - in Celsius degree from 20 to 40
- **spa/sanitizer/set** : _number_ - (SJB model only) 0 to turn off sanitizer, or value 3, 5, 8 to enable
