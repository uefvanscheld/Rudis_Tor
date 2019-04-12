/*
 * This file contains all subroutines related states and and controlling them
 *	
 *	Version 0.96
 *		- Neueinführung dieser Datei, damit FSM- und Motorsteuerung getrennt sind
 *	
 *	
 *	
 *	
 *	
 *	
 */

// #include "Torsteuerung.h"

//
// next lines are related to state OPENING
//
void execEnterStateOPENING()  {
	startMotor_R(V_Motoren, IsDoorOpening);		// start MOTOR R
	startMotor_L(V_Motoren, IsDoorOpening);		// start MOTOR L
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
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
	IsCurrentOverloaded = false;	// Fehlerflag wurde bearbeitet
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

//
// next lines are related to state OPENED
//
void execEnterStateOPENED() {
	fastStopMotor_R();	// stop MOTOR R
	fastStopMotor_L();	// stop MOTOR L
	initializeFlashLightNewState(state);	// Warnlampe auf den neuen Status einstellen
	IsDoorOpening = !IsDoorOpening;			// Drehrichtung der Motoren ändern
}

