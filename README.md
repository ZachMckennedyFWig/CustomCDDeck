## A CD Deck Inspired by the Legendary Beosound 9000
> Built from scratch using 3D printing, Machining, Python, Html/Javascript, C++, and Custom PCB's

[YOUTUBE VIDEO](https://www.youtube.com/watch?v=SDKLhXEmK_8)

<br/>

<div align="center">
  <img src="images\customCDPlayerHQ.png" alt="Image of the completed CD Deck" width="80%">
</div>

 <br/>

`CDControlDeck/` is the C++ code running on the Raspberry Pi Pico.\
`UI/` is the Python and HTML/Javascript used for the UI, Audio Playback, and CD drive control.\
`images/` are images used in this README
`CAD/` is the folder containing the STEP file for this CD player. 
___
### Hardware:
___
- [Raspberry Pi 4](https://www.raspberrypi.com/products/raspberry-pi-4-model-b/)
- [Schiit Modi HiFi DAC](https://www.schiit.com/products/modi-plus)
- [Pioneer BDR-XD08G CD Drive](https://www.amazon.com/dp/B0BN678FDT?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)
- [Raspberry Pi Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/)
- [Nema 17 Stepper Motors](https://www.omc-stepperonline.com/nema-17-bipolar-0-9deg-46ncm-65-1oz-in-2a-2-9v-42x42x48mm-4-wires-17hm19-2004s)
- [ADS 1115 16 bit ADC](https://www.amazon.com/HiLetgo-Converter-Programmable-Amplifier-Development/dp/B01DLHKMO2?source=ps-sl-shoppingads-lpcontext&ref_=fplfs&psc=1&smid=A30QSGOJR8LMXA&gQT=2)

___
### Libraries:
___
> For this project, Python was used for control of CD drive and audio playback with an html/javascript UI. C++ is running on the Raspberry Pi Pico and is used to control the motors and sensors.


  #### Python:
  - [eel](https://github.com/python-eel/Eel) for communication between the UI and the Python backend.
  - [python-vlc](https://pypi.org/project/python-vlc/) for control of playback of the CD
  - [pycdio](https://github.com/rocky/pycdio/tree/master) for lower level control of the CD player (python bindings to [libcdio](https://github.com/ShiftMediaProject/libcdio))

  #### C++:
  - [arduino-pico](https://github.com/earlephilhower/arduino-pico) to access arduino sensor and motor libraries on the pico.
  - [Accel-Stepper](https://github.com/waspinator/AccelStepper) for async control of stepper motors.
  - [Adafruit-ADS1X15](https://github.com/adafruit/Adafruit_ADS1X15) sensor library for the analogue to digital converter.

___
### CAD:
___
> *The current CAD assembly is not fully up to date but will be updated soon
<div align="center">
  <img src="images\CDPlayerCAD.png" alt="Image of the CAD of the CD Player Assembly" width="80%">
</div>

<br/>

Because most of this project was either 3D printed or Machined, it was very convienient to have a digital twin of the CD player in CAD during fabrication. The STEP file for this device can be found at `CAD/CustomCDPlayer.step`.
<br/> <br/>
If you dig through the CAD model, some of the parts might look strangely designed. This is because the main manufacturing method was 3D printing - to avoid using support material as much as possible a lot of parts have 45 degree tapers on overhangs or are split into multiple pieces at very specific locations. 
<br/>

___
### Electronics:
___
#### Wiring Diagram:
> This is a very rough diagram that doesn't go into some of the lower level details, but should be enough to understand how it works at a higher level.
<br/>
<div align="center">
  <img src="images\WIRINGDIAGRAM.png" alt="High Level Wiring Diagram" width="40%">
</div>

<br/>

#### Custom Carrier PCB:

> To avoid having a giant ratsnest of wires inside this thing, I decided to learn how to make a custom PCB carrier board for all the electronics for the Raspberry Pi Pico and get it manufactured by JLB PCB. Here's an image of that: <br/>
<div align="center">
  <img src="images\PCB.jpg" alt="Custom PCB Carrier Board" width="40%">
</div>

<br/>

___
### Reflections:
___

> This project has been months of work outside my full time job and has let me a learn some cool new skills such as simple PCB design and html/javascript UI design. It's also given me a much bigger appriciation for designers - It's really hard to make something look cool while still functioning properly.
>
> There are some oversights/intentional design flaws with this CD player that were made because of the lack of access to specific electronics I have, the biggest being the CD player using the clipping style CD players with latches on the spindle versus the superior clamping style CD drive that most high quality players use. I also could not for the life of me figure out a reliable way to hide the cursor on the Ui of the CD player on the raspberry pi, so that is still visable. Otherwise though, it functions well and will work exactly as shown in the youtube video. 
> 
> My dream would be to someday work for a company that designs synthesizers, audio equiptment, or interactive consumer electronics, so, if you are reading this and know any way for me to connect with those companies please do not hesitate to reach out to [my LinkedIn](https://www.linkedin.com/in/zach-mckennedy/)
