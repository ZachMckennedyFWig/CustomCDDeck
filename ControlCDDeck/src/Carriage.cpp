#include "Carriage.h"

/**
 * 
 * PRIVATE FUNCTIONS
 * 
 */
int Carriage::calc_steps(double adc_end, double adc_start) {
    // Used regression on the points of the carriage moving around then integrated to get this equation:
    // Q = -0.00000299033x^{3} + 0.0042645x^{2} + 57.19x
    // To solve for the number of steps we need we just have to do the definite integral of this

    double adc_end_sq = adc_end*adc_end;
    long adc_end_integration = long(-0.00000299033*adc_end_sq*adc_end + 0.0042645*adc_end_sq + 57.19*adc_end);

    double adc_start_sq = adc_start*adc_start;
    long adc_start_integration = long(-0.00000299033*adc_start_sq*adc_start + 0.0042645*adc_start_sq + 57.19*adc_start);

    return adc_end_integration - adc_start_integration;
}

double Carriage::get_adc(){
    // Read the differential voltage between A0 and A1
    int16_t differential_wiper = adc->readADC_Differential_2_3();
    int16_t differential_source = adc->readADC_Differential_0_1();

    return double(differential_wiper)/double(differential_source) * 1000.0;
}

/**
 * 
 * PUBLIC FUNCTIONS
 * 
 */

Carriage::Carriage(Adafruit_ADS1115* ads ) {
    // Set the adc to the ads object
    adc = ads;
    
    // Enable the Carriage motor
    pinMode(EN_PIN, OUTPUT);
    digitalWrite(EN_PIN, LOW);

    // Set the motor speed limit and acceleration value to small value for init
    carriage.setMaxSpeed(10000);
    carriage.setAcceleration(5000);
}

void Carriage::init() {
    // Ensure the stepper driver is enabled
    digitalWrite(EN_PIN, LOW);

    // Move the Carriage to slot 1
    target_slot = 1;

    // Calculate the difference between the current pos and the target pos
    double adc_diff = ads_slot_val[0] - get_adc();

    // Calculate the number of steps the carriage needs to take, multiply by 0.95 to undershoot by a small margin
    //long appx_steps = -calc_steps(ads_slot_val[target_slot-1], get_adc());
    long appx_steps = -long(adc_diff * STEPS_PER_DIFF);

    // Send the stepper the command to move
    carriage.move(appx_steps);

    while(carriage.distanceToGo() != 0) {
        carriage.run();
    }
    
    adc_diff = ads_slot_val[0] - get_adc();
    // Check if they are within 0.1 of each other
    while(fabs(adc_diff) > 0.035) {
        // If so, check if its greater or less than the target value (by checking if its above or below zero)
        if(adc_diff > 0.0) {
            // If its above the current value, step the motor forwards 5 steps
            carriage.move(-1);
        }
        else {
            // If its below the current value, step the motor backwards 5 setps
            carriage.move(1);
        }
        // Get the difference in the target adc value versus the current adc value
        adc_diff = ads_slot_val[0] - get_adc();

        // Move the motor
        while(carriage.distanceToGo() != 0) {
            carriage.run();
        }
    }

    // Set the motor speed limit and acceleration value
    carriage.setMaxSpeed(MAX_SPEED);
    carriage.setAcceleration(ACCEL);

    // Set the free variable to true to free up the carriage.
    FREE = true;
}

bool Carriage::is_free(){return FREE;} //  Return the state of FREE variable

void Carriage::update() {
    if(carriage.distanceToGo() != 0) {
        carriage.run();
    }
    else {FREE = true;}
    /*
    else {
        // Check to see if the Carriage is approximately near its target pos 
        if(fabs(ads_slot_val[target_slot-1] - get_adc()) > 5.0) {
            // Store the target slot so we can return to it later
            int temp_target = target_slot;
            // If the slot is wayyy off, re-INIT
            INIT=false;
            init();
            // Retry sending the Carriage to the slot it was trying to reach
            set_slot(temp_target);
        }
        else {FREE = true;}
    }
    */
}

void Carriage::set_slot(int slot) {
    // Check if the selected slot is different than the one its already on
    if(slot != target_slot) {
        // Update the state of the Carriage
        FREE = false;

        // Ensure the motor is enabled
        enable_motor();

        // Calculate the number of steps the carriage needs to take, multiply by 0.95 to undershoot by a small margin
        //long appx_steps = -calc_steps(ads_slot_val[target_slot-1], get_adc());
        //long appx_steps = (target_slot - slot)*STEPS_PER_SLOT;

        int index = min(target_slot, slot) - 1;
        int diff = abs(target_slot - slot);

        long step_sum = 0;

        for(int i = index; i < index + diff; i++) {
            step_sum += slot_steps[i];
        }

        // If the carriage is switching directions, add 50 steps
        if(slot > target_slot && car_dir == -1){
            car_dir = 1;
            step_sum += COMPENSATION_STEPS;
        }
        else if(slot < target_slot && car_dir == 1){
            car_dir = -1;
            step_sum += COMPENSATION_STEPS;
        }

        if(slot > target_slot) {step_sum = -step_sum;}

        // Set the target slot 
        target_slot = slot;

        // Send the stepper the command to move
        carriage.move(step_sum);
    }
}

// Returns the target slot variable
int Carriage::get_slot() {return target_slot;}

// Disables the stepper driver 
void Carriage::disable_motor() {digitalWrite(EN_PIN, HIGH);}

// Enables the stepper driver 
void Carriage::enable_motor() {digitalWrite(EN_PIN, LOW);}