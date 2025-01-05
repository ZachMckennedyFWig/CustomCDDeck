/**
 * Tilter.h 
 * Author: Zach Mckennedy
 * Date Created: 10/23/2024
 * Version 1.0
 * 
 * This class handles the functionality of the CD Deck Tilter
 */

#ifndef TILTER_H
#define TILTER_H

#include <AccelStepper.h>

// Variables for the stepper driver (in this case, bigtreetech tmc2209)

#define DIR_PIN 0           // GPIO pin of the stepper driver direction pin
#define STEP_PIN 1          // GPIO pin of the stepper driver step pin
#define MOTOR_INTERFACE 1   // Interface type for Accel Stepper Library 

#define MAX_SPEED 12000     // Max speed for stepper driver
#define ACCEL 10000         // Acceleration rate for the stepper driver

#define STEPS_UP 4000          // Stores the number of steps requred to raise the tilter
#define STEPS_CLEARANCE -854   // Stores the number of steps the tilter needs to lower for CD clearance
#define STEPS_DOWN -(STEPS_UP + STEPS_CLEARANCE)    // Stores the number of steps to lower the tilter

class Tilter {
    /**
     * Carriage Class
     */
    private:   
        bool FREE = true;   // Variable to store the state of the carriage

        bool RAISED = false;    // Variable to store the raised/lower state of the tilter

        AccelStepper tilter = AccelStepper(MOTOR_INTERFACE, STEP_PIN, DIR_PIN);
        
        enum STATE {
            RAISE = 0,
            CLEAR = 1,
            LOWER = 2
        };

        STATE MOVE_STATE;

    public:
        /**
         * Tilter default contructor 
         */
        Tilter();

        /**
         * is_free member function
         * 
         * Returns the state of the tilter (FREE variable)
         */
        bool is_free();

        /**
         * update member function
         * 
         * Updates motor position in a non-blocking way
         */
        int update();

        /**
         * raise_tilter member function
         * 
         * Raises the tilter 
         */
        void raise_tilter(); 

        /** 
         * lower_tilter member function 
         * 
         * Lowers the tilter
         */
        void lower_tilter();
};
#endif