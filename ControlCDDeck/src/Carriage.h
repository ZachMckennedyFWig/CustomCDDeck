/**
 * Carriage.h 
 * Author: Zach Mckennedy
 * Date Created: 10/20/2024
 * Version 1.0
 * 
 * This class is the implementation of the class that controls the carriage movement on the CD
 *  Deck. It combines a single stepper motor and an ADC + Potentiometer for positional control. 
 */

#ifndef CARRIAGE_H
#define CARRIAGE_H

#include <math.h>
#include <Adafruit_ADS1X15.h>
#include <AccelStepper.h>

// Variables for the stepper driver (in this case, bigtreetech tmc2209)

#define DIR_PIN 11          // GPIO pin of the stepper driver direction pin
#define STEP_PIN 12         // GPIO pin of the stepper driver step pin
#define EN_PIN 13           // GPIO pin of the enable pin on the stepper driver
#define MOTOR_INTERFACE 1   // Interface type for Accel Stepper Library 

#define MAX_SPEED 30000     // Max speed for stepper driver
#define ACCEL 60000         // Acceleration rate for the stepper driver

#define STEPS_PER_DIFF 58.67395962  // Stores the approximation for the number of steps per difference in ADC value
#define STEPS_PER_SLOT 10982        // Stores the number of steps between each slot

#define COMPENSATION_STEPS 90   // Stores the number of steps needed for compensating a direction change

class Carriage{
    /**
     * Carriage Class
     */
    private:
        bool INIT = false;  // Variable to track if the Carriage has been initialized     
        bool FREE = false;  // Variable to store the state of the carriage

        // Carriage slots valid positions start at 1 and end at 6, 0 is the initialization value and means no slot

        int target_slot = 0;   // Variable to store the target slot the carriage should be going to

        double ads_slot_val[6] = {32.3, 200.4, 388.7, 570.3, 760.0, 947.5};  // Array to store the ADS values for the different slot positions

        long slot_steps[5] = {11100, 10990, 10940, 10970, 10980};

        int car_dir = 1;    // Variable to store the travel direction of the carriage, used to compensate belt/flex slop.... 

        Adafruit_ADS1115* adc; // Variable to store the pointer to the adc object

        AccelStepper carriage = AccelStepper(MOTOR_INTERFACE, STEP_PIN, DIR_PIN);

        enum STATE {
            APPROX = 0,
            FINE = 1
        };

        STATE MOVE_STATE = APPROX;

        /**
         * calc_steps member function
         * 
         * Calculates the approximate number of steps needed for the carraige to move
         */
        int calc_steps(double adc_end, double adc_start);

        /**
         * get_adc member function
         * 
         * Returns the ADC value for the differential comparison on the potentiometer
         */
        double get_adc();

    public:
        /**
         * Carriage default contructor 
         * 
         * ads: pointer to the initialized ADS1115 16 bit adc object
         */
        Carriage(Adafruit_ADS1115* ads);

        /**
         * init member function
         * 
         * Initializes the carriage, alsigns it to slot 1
         */
        void init();

        /**
         * is_free member function
         * 
         * Returns the state of the carriage (FREE variable)
         */
        bool is_free();

        /**
         * update member function
         * 
         * Updates motor position in a non-blocking way
         */
        void update();

        /**
         * set_slot member function
         * 
         * Updates the target slot the CD Deck should go to 
         */
        void set_slot(int slot);

        /** get_slot member function
         * 
         * Returns the currently set slot
         */
        int get_slot();

        /** disable_motor member function
         * 
         * Disables the stepper driver for the motor
         */
        void disable_motor();

        /** enable_motor member function
         * 
         * Enables the stepper driver for the motor
         */
        void enable_motor();
};

#endif