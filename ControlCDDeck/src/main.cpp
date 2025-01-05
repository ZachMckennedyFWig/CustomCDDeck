#include <Arduino.h>
#include "hardware/pll.h"
#include <clocks.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <AccelStepper.h>
#include "Carriage.h"
#include "Tilter.h"


// Variables for the ADS pins
const int adc_sda = 14;
const int adc_scl = 15;

// Create an instance of the ADS1115
Adafruit_ADS1115 ads;
// Create a two wire instance for the i2c communication
TwoWire myWire(adc_sda, adc_scl);

Carriage BIGCAR(&ads);
Tilter BIGTILT;

int temp_addr = 1;

int temp_slot = 1;

int trigger = false;

void gset_sys_clock_pll(uint32_t vco_freq, uint post_div1, uint post_div2) 
{
    if (!running_on_fpga()) 
    {
        // Set initial configuration to 48 MHz (using USB PLL)
        clock_configure(clk_sys,
                        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                        48 * MHZ,
                        48 * MHZ);

        // Set the VCO frequency and dividers for the system PLL
        pll_init(pll_sys, 1, vco_freq, post_div1, post_div2);
        uint32_t freq = vco_freq / (post_div1 * post_div2);

        // Configure the reference clock to 12 MHz from XOSC
        clock_configure(clk_ref,
                        CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC,
                        0, // No aux mux
                        12 * MHZ,
                        12 * MHZ);

        // Configure the system clock to the new frequency
        clock_configure(clk_sys,
                        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                        freq, freq);

        // Configure the peripheral clock to the same frequency
        clock_configure(clk_peri,
                        0, // No aux mux
                        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                        freq, freq);
    }
}

// the setup function runs once when you press reset or power the board
void setup() {

  gset_sys_clock_pll(1080 * MHZ, 2, 2);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial);
  //Serial.println("started");
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on

  delay(250);

  // Reset I2C bus because weird ADC initialization issue. 
  myWire.end();
  delay(500);
  myWire.begin();

  // Write to Config register (0x01)
  myWire.beginTransmission(0x48);
  myWire.write(0x01);    // Points to Config register
  // Reset to default values:
  // 0x8583 - FSR ±2.048V, Single-shot mode, Compare disable, Alert/Rdy active low
  myWire.write(0x85);    // MSB
  myWire.write(0x83);    // LSB
  myWire.endTransmission();
    
  delay(25);  // Wait for the changes to take effect

  // Initialize the ADS1115
  ads.begin(0x48, &myWire);
  //Serial.println("ADS BEGIN");

  delay(250);

  BIGCAR.init();
  Serial.print("init");
}

// the loop function runs over and over again forever
void loop() {

  // Every 100ms Check if there is a serial message in the buffer
  if(Serial.available() >= 2) {
    // First byte is the selected slot
    int slot = Serial.read();
    // Check if the slot is within range 
    if(slot > 0 && slot <= 6) {
      // Second byte is the tilter state
      int tilter = Serial.read();
      // Check if the tilter is within range
      if(tilter >= 0 && tilter <= 1) {
        // Code to move the carriage and tilter
        if(slot != BIGCAR.get_slot()) {
          // Lower the titer
          BIGTILT.lower_tilter();
          while(!BIGTILT.is_free()) {BIGTILT.update();}

          delay(250);

          // Move the Carriage
          BIGCAR.set_slot(slot);
          while(!BIGCAR.is_free()) {BIGCAR.update();}

          delay(250);

          // Disable the stepper before lifting the tilter
          //BIGCAR.disable_motor();

          // Update tilter
          if(tilter) {BIGTILT.raise_tilter();}
          // While the tilter is busy, update its state
          while(!BIGTILT.is_free()) {
            int stat = BIGTILT.update();
            if(stat == 1 && trigger == false) {
              trigger = true;
              Serial.print("done");
            }
          }

          // Re-Enable the stepper motor to avoid unwanted movement
          //BIGCAR.enable_motor();
        }
        else {
          // Move only the tilter to the requested state
          if(tilter) {
            // Raise the tilter
            BIGTILT.raise_tilter();
            // Disable carriage stepper
            //BIGCAR.disable_motor();
          }
          else {
            // Lower the tilter
            BIGTILT.lower_tilter();
          }
          // While the tilter is busy, update its state
          while(!BIGTILT.is_free()) {
            int stat = BIGTILT.update();
            if(stat == 1 && trigger == false) {
              trigger = true;
              Serial.print("done");
            }
          }
          // Re-Enable the carriage stepper
          //BIGCAR.enable_motor();
        }
        // Message back saying that the movement is done
        if(trigger == true) {trigger = false;}
        else {Serial.print("done");}
      }
    }
    else {
      // If theres something wrong with the serial data, clear the entire current buffer
      int bufsize = Serial.available();
      for(int i = 0; i < bufsize; i++) {
        int temp = Serial.read();
        Serial.println(temp);
      }
    }
  }
  delay(100);
  
  /*
  if(BIGCAR.is_free()) {
    Serial.print("GOING TO SLOT ");
    Serial.println(temp_slot); // Print with 8 decimal places
    delay(100);
    BIGCAR.set_slot(temp_slot);
    if(temp_slot == 6) {temp_addr = -1;}
    else if(temp_slot == 1) {temp_addr = 1;}
    temp_slot += temp_addr;
  }
  else {
    BIGCAR.update();
  }

  */

  // 0, 193.3, 387.7, 571.5, 754.5, 946.2
  // y=-0.9571x^{2}+193.3x+1.029 <-- Regression fit of the CD positions
}