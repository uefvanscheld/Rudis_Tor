/*
 * This file contains all subroutines related to DC motor control
 */

#include "Torsteuerung.h"


/* 
	this procedure generates a short impuls to set the flipflop  Q outputs of L6506
	after a too high current was detected
*/
void reset_power_limiter() {
	DigitalWrite(Rst_I_Stopp, HIGH);	// set flipflop outputs
	DigitalWrite(Rst_I_Stopp, LOW);		// deactivate flipflop set line
}

//
// next lines are related to state BLOCKED
//
void fastStopMotor_R() {
	DigitalWrite(H_Br_R_En, LOW);	// stop the PWM signal
	// short cut the motor windings for fast stop
	DigitalWrite(H_Br_R_A, LOW); 
	DigitalWrite(H_Br_R_Z, LOW);
}
void fastStopMotor_L() {
	DigitalWrite(H_Br_L_En, LOW);	// stop the PWM signal
	// short cut the motor windings for fast stop
	DigitalWrite(H_Br_L_A, LOW); 
	DigitalWrite(H_Br_L_Z, LOW);
}
void execEnterStateBLOCKED() {
	fastStopMotor_R();	// stop MOTOR R
	fastStopMotor_L();	// stop MOTOR L
}


