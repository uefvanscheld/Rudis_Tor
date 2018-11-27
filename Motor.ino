/*
 * This file contains all subroutines related to DC motor control
 */

// #include "Torsteuerung.h"


/* 
	this procedure generates a short impuls to set the flipflop  Q outputs of L6506
	after a too high current was detected
*/
void reset_power_limiter() {
	digitalWrite(Rst_I_Stopp, HIGH);	// set flipflop outputs
	digitalWrite(Rst_I_Stopp, LOW);		// deactivate flipflop set line
}
void fastStopMotor_R() {
	digitalWrite(H_Br_R_En, LOW);	// stop the PWM signal
	// short cut the motor windings for fast stop
	digitalWrite(H_Br_R_A, LOW); 
	digitalWrite(H_Br_R_Z, LOW);
}
void fastStopMotor_L() {
	digitalWrite(H_Br_L_En, LOW);	// stop the PWM signal
	// short cut the motor windings for fast stop
	digitalWrite(H_Br_L_A, LOW); 
	digitalWrite(H_Br_L_Z, LOW);
}

/*
 *	Steuerung der Motoren
 */
void startMotor_R(int pmw_val, boolean opening) {
	digitalWrite(H_Br_R_En, LOW);	// stop the PWM signal
	if(opening) {			// rechtes Tor öffnen
		digitalWrite(H_Br_R_Z, LOW);		// Motor_R (-) auf Masse legen
		digitalWrite(H_Br_R_A, HIGH);		// Motor_R (+) auf Vcc legen
		analogWrite(H_Br_R_En, pmw_val);	// PWM-Signal aktivieren
	}
	else {					// rechtes Tor schließen
		digitalWrite(H_Br_R_A, LOW);		// Motor_R (+) auf Masse legen
		digitalWrite(H_Br_R_Z, HIGH);		// Motor_R (-) auf Vcc legen
		analogWrite(H_Br_R_En, pmw_val);	// PWM-Signal aktivieren		
	}
}	
void startMotor_L(int pmw_val, boolean opening) {
	digitalWrite(H_Br_L_En, LOW);	// stop the PWM signal to prevent
	// reset FlipFlop (SYNC) in L6506 with short impuls
	digitalWrite(Rst_I_Stopp, HIGH);	
	digitalWrite(Rst_I_Stopp, LOW);		
	
	if(opening) {			// Linkes Tor öffnen
		digitalWrite(H_Br_L_Z, LOW);		// Motor_L (-) auf Masse legen
		digitalWrite(H_Br_L_A, HIGH);		// Motor_L (+) auf Vcc legen
		analogWrite(H_Br_L_En, pmw_val);	// PWM-Signal aktivieren
	}
	else {					// linkes Tor schließen
		digitalWrite(H_Br_L_A, LOW);		// Motor_L (+) auf Masse legen
		digitalWrite(H_Br_L_Z, HIGH);		// Motor_L (-) auf Vcc legen
		analogWrite(H_Br_L_En, pmw_val);	// PWM-Signal aktivieren		
	}
}	

//
// next lines are related to state BLOCKED
//
void execEnterStateBLOCKED() {
	fastStopMotor_R();	// stop MOTOR R
	fastStopMotor_L();	// stop MOTOR L
	initializeFlashLightNewState(state);	// Warnlampe auf den neuen Status einstellen
	IsDoorOpening = !IsDoorOpening;	// Drehrichtung der Motoren ändern
	IsDoorBlocked = false;			// Warnung wurde bearbeitet
}

//
// next lines are related to state OVERLOAD
//
void execEnterStateOVERLOAD()  {
	fastStopMotor_R();			// stop MOTOR R
	fastStopMotor_L();			// stop MOTOR L
	initializeFlashLightNewState(state);	// Warnlampe auf den neuen Status einstellen
	IsDoorOpening = !IsDoorOpening;	// Drehrichtung der Motoren ändern
}

//
// next lines are related to state OPENING
//
void execEnterStateOPENING()  {
	startMotor_R(V_Motoren, IsDoorOpening);		// start MOTOR R
	startMotor_L(V_Motoren, IsDoorOpening);		// start MOTOR L
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
}

//
// next lines are related to state CLOSING
//
void execEnterStateCLOSING()  {
	startMotor_R(V_Motoren, IsDoorOpening);		// start MOTOR R
	startMotor_L(V_Motoren, IsDoorOpening);		// start MOTOR L
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
}

//
// next lines are related to state STOPPED
//
void execEnterStateSTOPPED()  {
	fastStopMotor_R();			// stop MOTOR R
	fastStopMotor_L();			// stop MOTOR L
	initializeFlashLightNewState(state);	// Warnlampe auf den neuen Status einstellen
	IsDoorOpening = !IsDoorOpening;	// Drehrichtung der Motoren ändern
}