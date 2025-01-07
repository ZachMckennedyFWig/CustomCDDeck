## A CD Deck Inspired by the Legendary Beosound 9000
> Built from scratch using 3D printing, Machining, Python, Html/Javascript, C++, and Custom PCB's

### Hardware:
---
- [Raspberry Pi 4](https://www.raspberrypi.com/products/raspberry-pi-4-model-b/)
- [Schiit Modi HiFi DAC](https://www.schiit.com/products/modi-plus)
- [Pioneer BDR-XD08G CD Drive](https://www.amazon.com/dp/B0BN678FDT?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)
- [Raspberry Pi Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/)
- [Nema 17 Stepper Motors](https://www.omc-stepperonline.com/nema-17-bipolar-0-9deg-46ncm-65-1oz-in-2a-2-9v-42x42x48mm-4-wires-17hm19-2004s)
- [ADS 1115 16 bit ADC](https://www.amazon.com/HiLetgo-Converter-Programmable-Amplifier-Development/dp/B01DLHKMO2?source=ps-sl-shoppingads-lpcontext&ref_=fplfs&psc=1&smid=A30QSGOJR8LMXA&gQT=2)

### Libraries:
---
> For this project, Python was used for control of CD drive and audio playback with an html/javascript UI. C++ is running on the Raspberry Pi Pico and is used to control the motors and sensors.


  #### Python:
  - [eel](https://github.com/python-eel/Eel) for communication between the UI and the Python backend.
  - [python-vlc](https://pypi.org/project/python-vlc/) for control of playback of the CD
  - [pycdio](https://github.com/rocky/pycdio/tree/master) for lower level control of the CD player (python bindings to [libcdio](https://github.com/ShiftMediaProject/libcdio))

  #### C++:
  - [arduino-pico](https://github.com/earlephilhower/arduino-pico) to access arduino sensor and motor libraries on the pico.
  - [Accel-Stepper](https://github.com/waspinator/AccelStepper) for async control of stepper motors.
  - [Adafruit-ADS1X15](https://github.com/adafruit/Adafruit_ADS1X15) sensor library for the analogue to digital converter.

