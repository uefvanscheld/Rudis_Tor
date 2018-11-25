/*
 * This code covers all input and output functions (except motor control)
 * This includes:
 * - PCB push button
 * - remote control button
 * - 2 jumpers configuring the delays during opening and closing the doors
 * - flashing the signal light (normal operation & error condition)
 * - read the potentiometer for test purposes 
 * - read jumpers which configure the delays of opening and closing doors
 */

#include "Torsteuerung.h"

void initialize_IO() {
	pinMode(Start_Funk,   INPUT);
	pinMode(Start_Taste,  INPUT_PULLUP);
	pinMode(Jumper1,      INPUT_PULLUP);
	pinMode(Jumper2,      INPUT_PULLUP);
	
	// inputs to monitor the hardware based current limiter
	pinMode(Fb_H_Br_R_A,  INPUT);		
	pinMode(Fb_H_Br_R_Z,  INPUT);
	pinMode(Fb_H_Br_L_A,  INPUT);
	pinMode(Fb_H_Br_L_Z,  INPUT);

	// outputs to control motor direction
	pinMode(H_Br_R_A,     OUTPUT);
	pinMode(H_Br_R_Z,     OUTPUT);
	pinMode(H_Br_L_A,     OUTPUT);
	pinMode(H_Br_L_Z,     OUTPUT);
	
	pinMode(Rst_I_Stopp,  OUTPUT);		// reset current limiter
	pinMode(H_Br_R_En,    OUTPUT);		// PWM output for right motor 
	pinMode(H_Br_L_En,    OUTPUT);		// PWM output for left motor		
	pinMode(Warnleuchte,  OUTPUT);		// output for flash light
	
	// Initialize outputs
	DigitalWrite(H_Br_R_En, LOW);	// stop right motor
	DigitalWrite(H_Br_L_En, LOW);	// stop left motor
	DigitalWrite(Rst_I_Stopp, LOW);	// disable power limiter
	
}


/*
	this procedure reads the state of the two buttons and controls 
	the variables which are responsible for indicating a pressed
	or released button to the FSM
*/
void get_button_state(){
  int val_push_button = 0;
  int val_RC_button = 0;
  
  // check state of both buttons
  val_push_button = digitalRead(Start_Taste);  // read the PCB button input (active low)
  val_RC_button = digitalRead(Start_Funk);     // read the remote control input (active high)
  
  // if one of the buttons is pressed launch an action
  if ((val_push_button == LOW || val_RC_button == HIGH) && IsButtonReleased == true && IsButtonNeedsProcessing == false){
    IsButtonNeedsProcessing = true;
    // IsButtonPressed = true;
    IsButtonReleased = false;
  }
  // BOTH buttons need to be released before a new command can be launched 
  else if ((val_push_button == HIGH && val_RC_button == LOW) && IsButtonReleased == false) {
    // IsButtonPressed = false;
    IsButtonReleased = true;
  }
  // otherwise do nothing
}

/*
	Potentiometer lesen, um die Geschwindigkeit der Motoren zu steuern
	Der gelesene Wert (0-1023) muss durch 4 geteilt werden
*/
char get_motor_speed() {
	int val = 0;
	val = AnalogRead(MotorSpeedPin);	// Wert lesen
	return char(val>>2);	// durch 4 teilen und Byte-Wert zurückgeben
	
}

