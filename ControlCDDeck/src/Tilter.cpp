#include "Tilter.h"

Tilter::Tilter() {
    // Set the motor speed limit and acceleration value to small value for init
    tilter.setMaxSpeed(MAX_SPEED);
    tilter.setAcceleration(ACCEL);
}

// Returns the variable FREE 
bool Tilter::is_free() {return FREE;}

int Tilter::update() {
    switch(MOVE_STATE) {
        case RAISE:
            if(tilter.distanceToGo() != 0) {
                // If the stepper is still trying to reach the position update its pos
                tilter.run();
            }
            else {
                // When the stepper has reached the position, now move back down the clearance amount
                MOVE_STATE = CLEAR;
                tilter.move(STEPS_CLEARANCE);
            }
            break;
        case CLEAR:
            if(tilter.distanceToGo() != 0) {
                // If the stepper is still trying to reach the position update its pos
                tilter.run();
            }
            else {
                // Movement is now done
                FREE = true;
                RAISED = true;
            }
            break;
        case LOWER:
            if(tilter.distanceToGo() != 0) {
                // If the stepper is still trying to reach the position update its pos
                tilter.run();
            }
            else {
                // Movement is now done
                FREE = true;
                RAISED = false;
            }
            break;
    }
    return MOVE_STATE;
}

void Tilter::raise_tilter() {
    if(!RAISED) {
        // Update the availability of the motor
        FREE = false;
        // Update the state to update
        MOVE_STATE = RAISE;
        // Move the Stepper 
        tilter.move(STEPS_UP);
    }
}

void Tilter::lower_tilter() {
    if(RAISED) {
        // Update the availability of the motor
        FREE = false; 
        // Update the state to update
        MOVE_STATE = LOWER;
        // Move the stepper
        tilter.move(STEPS_DOWN);
    }
}
