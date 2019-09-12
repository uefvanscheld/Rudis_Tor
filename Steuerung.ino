/*
 * This file contains all subroutines related to FSM states and 
 controlling them
 *
 *	Version 0.97 (speziell angepasst für die geplanten Testfahrten)
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

//  #include "Torsteuerung.h"
//
// next lines are related to state OPENING
//
void execEnterStateOPENING()  {
	startMotor_R(V_Motoren, IsDoorOpening);		// start MOTOR R
	startMotor_L(V_Motoren, IsDoorOpening);		// start MOTOR L
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen

}

//
// next lines are related to state INITIALIZED
//
void execEnterStateINITIALIZED()  {
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	Serial.println(F("Initalisierung abgeschlossen !"));
	Serial.println(F("****************************************************************************************"));
	Serial.println(F("Taste drücken, um Phase 1 zu starten."));
}

//
// next lines are related to state PHASE1_OPENING
//
void execEnterStatePHASE1_OPENING()  {
	IsDoor_R_Blocked = false;
	IsDoor_L_Blocked = false;

	PWM_Motor_R_Target = parameter[state].Motor_R_Speed_Target;	// Zielgeschwindigkeit ermitteln
	PWM_Motor_L_Target = parameter[state].Motor_L_Speed_Target;	// Zielgeschwindigkeit ermitteln

	startMotor_R(PWM_Motor_R, IsDoorOpening);		// start MOTOR R
	startMotor_L(PWM_Motor_L, IsDoorOpening);		// start MOTOR L
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	Serial.println (F("Start Phase 1: Beide Tore werden jetzt mit PWM=200 bis zum Anschlag geöffnet (PHASE1_OPENING)...."));

    Serial.print (F("Initiale Werte:\t"));
	Serial.print (millis());
    Serial.print (F(";\t st:"));
	Serial.print (state);
    Serial.print (F(";\t PWM-R:"));
	Serial.print (PWM_Motor_R);
    Serial.print (F(";\t PWM-L:"));
	Serial.print (PWM_Motor_L);

    Serial.print (F(";\t I-R:"));
	Serial.print (Mot_R_Current);
    Serial.print (F(";\t I-L:"));
	Serial.print (Mot_L_Current);

    Serial.print (F(";\t I-R-Lim:"));
	Serial.print (Mot_R_Current_Limit);
    Serial.print (F(";\t I-L-Lim:"));
	Serial.print (Mot_L_Current_Limit);
	
    Serial.print (F(";\t PWM-R-Ziel:"));
	Serial.print (PWM_Motor_R_Target);
    Serial.print (F(";\t PWM-L-Ziel:"));
	Serial.print (PWM_Motor_L_Target);

    Serial.print (F(";\t R-Blk:"));
	Serial.print (IsDoor_R_Blocked);
    Serial.print (F(";\t L-Blk:"));
	Serial.print (IsDoor_L_Blocked);

	Serial.println (F(""));


}

//
// next lines are related to state PHASE1_TESTING
//
void execEnterStatePHASE1_TESTING()  {
	testing_next_event = 0;						// zunächst gibt es keinen nächsten Timerevent
	PWM_Motor_R = 0;							// beide Motoren ausschalten
	PWM_Motor_L = 0;
	IsDoorOpening = true;						// Das linke Tor soll im geöffneten Zustand gegen den Anschlag gedrückt werden
	IsCurrent_R_Overloaded = false;				// Überlastflag löschen
	IsCurrent_L_Overloaded = false;				// Überlastflag löschen
	IsDoor_R_Blocked = false;
	IsDoor_L_Blocked = false;
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	startMotor_L(PWM_Motor_L, IsDoorOpening);	// start MOTOR L
	Serial.println (F("Phase 1: Ermitteln des erforderlichen PWM-Wertes für Erkennung Überlast/Anschlag beginnt (PHASE1_TESTING)...."));

    Serial.print (F("Initiale Werte:\t"));
	Serial.print (millis());
    Serial.print (F(";\t st:"));
	Serial.print (state);
    Serial.print (F(";\t PWM-R:"));
	Serial.print (PWM_Motor_R);
    Serial.print (F(";\t PWM-L:"));
	Serial.print (PWM_Motor_L);

    Serial.print (F(";\t I-R:"));
	Serial.print (Mot_R_Current);
    Serial.print (F(";\t I-L:"));
	Serial.print (Mot_L_Current);

    Serial.print (F(";\t I-R-Lim:"));
	Serial.print (Mot_R_Current_Limit);
    Serial.print (F(";\t I-L-Lim:"));
	Serial.print (Mot_L_Current_Limit);
	
    Serial.print (F(";\t PWM-R-Ziel:"));
	Serial.print (PWM_Motor_R_Target);
    Serial.print (F(";\t PWM-L-Ziel:"));
	Serial.print (PWM_Motor_L_Target);

    Serial.print (F(";\t R-Blk:"));
	Serial.print (IsDoor_R_Blocked);
    Serial.print (F(";\t L-Blk:"));
	Serial.print (IsDoor_L_Blocked);

	Serial.println (F(""));

}
//
// next lines are related to state PHASE1_DONE
//
void execEnterStatePHASE1_DONE()  {
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	Serial.println(F("Phase 1 abgeschlossen !"));
	Serial.println(F("****************************************************************************************"));
	Serial.println();
	Serial.println(F("Taste drücken, um Phase 2 zu starten."));
	Serial.println(F("Sobald sich nach Beginn der Phase 2 das rechte Tor bewegt, bitte die Taste betätigen !!"));
}

//
// next lines are related to state PHASE2_TESTING
//
void execEnterStatePHASE2_TESTING()  {
	Serial.println (F("Start Phase 2: Ermittlung des minimalen PWM-Werts für eine Torbewegung des RECHTEN Tores beginnt (PHASE2_TESTING)...."));
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	IsDoor_R_Blocked = false;
	IsDoor_L_Blocked = false;

	PWM_Motor_R = 0;							// beide Motoren ausschalten
	PWM_Motor_L = 0;							// beide Motoren ausschalten
	PWM_min_moving = PWM_Motor_R;				// Startwert in Variable für den minimalen PWM-Wert übernehmen
	Serial.print(F("Startwert für PWM: "));		// und ausgeben
	Serial.println(PWM_min_moving);

	startMotor_R(PWM_Motor_R, CloseDoor);		// Start von  MOTOR R

}

//
// next lines are related to state PHASE2_DONE
//
void execEnterStatePHASE2_DONE()  {
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	Serial.println(F("Phase 2 abgeschlossen !"));
	Serial.println(F("****************************************************************************************"));
	Serial.println();
	Serial.println(F("Taste drücken, um Phase 3 zu starten."));
}

//
// next lines are related to state PHASE3_TESTING
//
void execEnterStatePHASE3_TESTING()  {
	Serial.println (F("Start Phase 3: Messung der Torlaufzeiten des LINKEN Tores bei verschiedenen Geschwindigkeiten (PHASE3_TESTING)...."));
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	IsDoor_R_Blocked = false;
	IsDoor_L_Blocked = false;

	PWM_Motor_R = 0;											// beide Motoren ausschalten
	PWM_runtime = max(PWM_min_blocking , PWM_min_moving);		// PWM-Startwert ermitteln
	Serial.print (F("Startwert: "));
	Serial.println (PWM_runtime);
	PWM_Motor_L_Target = PWM_runtime;							// und auch als Zielgeschwindigkeit festlegen
	PWM_Motor_L = PWM_runtime;
	
	// jetzt die Startzeit festhalten
	Startzeit_Tor = timestamp;
	Runtime_Direction_Opening = CloseDoor;				// Drehrichtung des Motors merken
	startMotor_L_Tests(PWM_Motor_L_Target, Runtime_Direction_Opening);	// Start von  MOTOR L  (geänderte Version!!)
}

//
// next lines are related to state PHASE2_DONE
//
void execEnterStatePHASE3_DONE()  {
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	Serial.println(F("Phase 3 abgeschlossen !"));
	Serial.println(F("****************************************************************************************"));
	Serial.println(F("Alle Testphasen durchlaufen."));
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
//
// next lines are related to logging PWM, current (analog input) during phase 1 testing
//
void log_PWM_CURRENT() {
	Serial.print (timestamp);
	Serial.print (F(":\t PWM(L): "));
	Serial.print (PWM_Motor_L);
	Serial.print (F(";\t aktuelle Stromstärke: "));
	Serial.print (analogRead(Strom_L));
	Serial.print (F(";\t Stromstärke (vorheriger Wert): "));
	Serial.print (Mot_L_Current);
    Serial.println (F(""));
}

void log_PWM_RUNTIME() {
	Serial.print (timestamp - Startzeit_Tor);
	Serial.print (F(" [ms]:\t PWM(L): "));
	Serial.print (PWM_Motor_L);
	Serial.print (F(";\t aktuelle Stromstärke: "));
	Serial.print (analogRead(Strom_L));
	Serial.print (F(";\t IsDoor_L_Blocked: "));
	Serial.print (IsDoor_L_Blocked);
	Serial.print (F(";\t Runtime_Direction_Opening: "));
	Serial.print (Runtime_Direction_Opening);
    Serial.println (F(""));
}
void log_Phase3_Anschlag() {
	Serial.print (F("Phase 3: Tor am Anschlag angekommen !!"));
	Serial.print (F("Phase 3: Bei einem PWM-Wert von "));
	Serial.print (PWM_Motor_L);
	Serial.print (F(" beträgt die Laufzeit: "));
	Serial.print (Zielzeit_Tor - Startzeit_Tor);
	Serial.print (F(" [ms]"));
    Serial.println (F(""));
	Serial.println(F("****************************************************************************************"));
}


void log_Debugging() {

	Serial.print (millis());
    Serial.print (F(";\t st:"));
	Serial.print (state);

    Serial.print (F(";\t PWM-R:"));
	Serial.print (PWM_Motor_R);
    Serial.print (F(";\t PWM-L:"));
	Serial.print (PWM_Motor_L);

    Serial.print (F(";\t I-R:"));
	Serial.print (Mot_R_Current);
    Serial.print (F(";\t I-L:"));
	Serial.print (Mot_L_Current);

    Serial.print (F(";\t I-R-Lim:"));
	Serial.print (Mot_R_Current_Limit);
    Serial.print (F(";\t I-L-Lim:"));
	Serial.print (Mot_L_Current_Limit);

    Serial.print (F(";\t PWM-R-Ziel:"));
	Serial.print (PWM_Motor_R_Target);
    Serial.print (F(";\t PWM-L-Ziel:"));
	Serial.print (PWM_Motor_L_Target);
	
    Serial.print (F(";\t R-Blk:"));
	Serial.print (IsDoor_R_Blocked);
    Serial.print (F(";\t L-Blk:"));
	Serial.print (IsDoor_L_Blocked);

/*
    Serial.print (F(";\t F-on:"));
	Serial.print (flash_on_duration);
    Serial.print (F(";\t F-off:"));
	Serial.print (flash_off_duration);
    Serial.print (F(";\t t_next:"));
	Serial.print (nextTimerFlashEvent);

    Serial.print (F(";\t IsFlashLightActive:"));
	Serial.print (IsFlashLightActive);
    Serial.print (F(";\t IsFlashLightOn:"));
	Serial.print (IsFlashLightOn);

	debugFlags();

 */
    Serial.println (F(""));
}
