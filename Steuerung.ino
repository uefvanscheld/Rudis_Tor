/*
 * This file contains all subroutines related to FSM states and 
 controlling them
 *
 *	Version 0.98
 *	- NEU: Behandlung von Substatus
 *	- NEU: 
 *	- NEU: 
 *	- Logging-Funktionen in separate Datei ausgelagert
 *	-  
 *	-  
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
/* 
void execEnterStateOPENING()  {
	startMotor_R(V_Motoren, IsDoorOpening);		// start MOTOR R
	startMotor_L(V_Motoren, IsDoorOpening);		// start MOTOR L
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen

}
 */
//
// next lines are related to state INITIALIZED
//
void execEnterStateINITIALIZED()  {

	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	Serial.println(F("****************************************************************************************"));
	Serial.println(F("Torsteuerung - Version 0.98"));
	Serial.println();
	Serial.println(F("Initalisierung abgeschlossen"));
	Serial.println(F("****************************************************************************************"));
	
}

//
// next lines are related to state IDLE
//
void execEnterStateIDLE()  {
	state = IDLE;
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	Serial.println(F("****************************************************************************************"));
	Serial.println(F("Status: IDLE  -  System im Wartezustand ..."));
	
}

//
// next lines are related to state PHASE1_OPENING
//
void execEnterStatePHASE1_OPENING()  {
	Serial.println(F("****************************************************************************************"));
	Serial.println(F("Taste drücken, um Phase 1 zu starten."));

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
void execEnterStatePHASE1()  {
	logDebug(FSM,#);
	state = PHASE1_TESTING;
	isCalledBy = 0;
	subStateStack = 2;							// es muss ein Substatus ausgeführt werden (hier: OPENING)
}

//
// next lines are related to state PHASE1_TESTING
//
void execExecStatePHASE1()  {
	logDebug(FSM,#);
	state = PHASE1_TESTING;
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
// next lines are related to EXIT state PHASE1
//
void execExitStatePHASE1()  {
	logDebug(FSM,#);
	state = IDLE;
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	Serial.println(F("Testprogramm 1 abgeschlossen !"));
	Serial.println(F("****************************************************************************************"));
	Serial.println();
}


//
// next lines are related to state PHASE1_DONE
//
void execEnterStatePHASE1_DONE()  {
	state = PHASE1_DONE;
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	Serial.println(F("Testprogramm 1 abgeschlossen !"));
	Serial.println(F("****************************************************************************************"));
	Serial.println();
}

//
// next lines are related to state PHASE2_TESTING
//
void execEnterStatePHASE2_TESTING()  {
	state = PHASE2_TESTING;
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
	state = PHASE2_DONE;
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
	state = PHASE3_TESTING;
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
// next lines are related to state PHASE3_DONE
//
void execEnterStatePHASE3_DONE()  {
	state = PHASE3_DONE;
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	Serial.println(F("Phase 3 abgeschlossen !"));
	Serial.println(F("****************************************************************************************"));
	Serial.println(F("Für den Start der Phase 4 jetzt bitte die Taste SO LANGE GEDRÜCKT HALTEN, bis sich das rechte Tor etwa auf der Hälfte des Weges befindet."));

}

//
// next lines are related to state PHASE4_CLOSING
//
void execEnterStatePHASE4_CLOSING()  {
	state = PHASE4_CLOSING;
	// jetzt erstmal auf den Tastdruck warten, damit die Phase und das Tor gestartet werden
	IsButtonNeedsProcessing = false;
	do
	{
		get_button_state();
	} while (IsButtonNeedsProcessing == false);
	// Taste wurde gedrückt; nun das rechte Tor schließen, solange die Taste gedrückt bleibt
	Serial.println (F("Start Phase 4: Ermittlung des maximalen PWM-Werts, mit dem ein Motor aus dem Stand ohne Überlastung gestartet werden kann, beginnt (PHASE4_TESTING)...."));
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	IsDoor_R_Blocked = false;
	IsDoor_L_Blocked = false;
	IsCurrent_R_Overloaded	= false;			// Hardware-Strombegrenzung für den rechten Motor hat nicht angesprochen
	IsCurrent_L_Overloaded	= false;			// Hardware-Strombegrenzung für den linken Motor hat nicht angesprochen

	PWM_Motor_R = 0;							// beide Motoren ausschalten
	PWM_Motor_L = 0;							// beide Motoren ausschalten
	PWM_min_moving = PWM_Motor_R;				// Startwert in Variable für den minimalen PWM-Wert übernehmen
	Serial.print(F("Startwert für PWM: "));		// und ausgeben
	Serial.println(PWM_min_moving);

	startMotor_R(PWM_Motor_R, CloseDoor);		// Start von  MOTOR R

}

//
// next lines are related to state PHASE4_TESTING
//
void execEnterStatePHASE4_TESTING()  {
	state = PHASE4_TESTING;
	IsDoorOpening = CloseDoor;
	// Taste wurde gedrückt; nun das rechte Tor schließen, solange die Taste gedrückt bleibt
	// Serial.println (F("Start Phase 4: Ermittlung des maximalen PWM-Werts, mit dem ein Motor aus dem Stand ohne Überlastung gestartet werden kann, beginnt (PHASE4_TESTING)...."));
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	IsDoor_R_Blocked = false;
	IsDoor_L_Blocked = false;
	IsCurrent_R_Overloaded	= false;					// Hardware-Strombegrenzung für den rechten Motor hat nicht angesprochen
	IsCurrent_L_Overloaded	= false;					// Hardware-Strombegrenzung für den linken Motor hat nicht angesprochen
		
	PWM_Motor_R = 0;									// beide Motoren ausschalten
	PWM_Motor_L = 0;									// beide Motoren ausschalten
	PWM_Motor_R = PWM_max_non_blocking_start;			// Startwert in Variable für den minimalen PWM-Wert übernehmen
	PWM_max_non_blocking = PWM_max_non_blocking_start;	// Startwert merken
	Serial.print(F("Startwert für PWM: "));				// und ausgeben
	Serial.println(PWM_Motor_R);
	delay(PWM_max_non_blocking_pause);
	startMotor_R(PWM_Motor_R, IsDoorOpening);			// Start von  MOTOR R ohne Beschleunigung
	nextTimer_Motor_R_Event = millis() + PWM_max_non_blocking_duration;
}

//
// next lines are related to state PHASE4_DONE
//
void execEnterStatePHASE4_DONE()  {
	state = PHASE4_DONE;
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	Serial.println(F("Phase 4 abgeschlossen !"));
	Serial.println(F("****************************************************************************************"));
	Serial.println();
	Serial.println(F("Taste drücken, um Phase 5 zu starten."));
}


void execEnterStatePHASE5_CLOSING()  {
	state = PHASE5_CLOSING;
	Serial.println (F("Zum Abschluss werden jetzt beide Tore geschlossen..."));
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	IsDoor_R_Blocked = false;
	IsDoor_L_Blocked = false;
	IsCurrent_R_Overloaded	= false;			// Hardware-Strombegrenzung für den rechten Motor hat nicht angesprochen
	IsCurrent_L_Overloaded	= false;			// Hardware-Strombegrenzung für den linken Motor hat nicht angesprochen

	PWM_Motor_R_Target = parameter[state].Motor_R_Speed_Target;	// Zielgeschwindigkeit ermitteln
	PWM_Motor_L_Target = parameter[state].Motor_L_Speed_Target;	// Zielgeschwindigkeit ermitteln

	PWM_Motor_R = 0;							// beide Motoren ausschalten
	PWM_Motor_L = 0;							// beide Motoren ausschalten

	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen

	startMotor_R(PWM_Motor_R, CloseDoor);		// start MOTOR R
	startMotor_L(PWM_Motor_L, CloseDoor);		// start MOTOR L

}


//
// next lines are related to state PHASE5_DONE
//
void execEnterStatePHASE5_DONE()  {
	state = PHASE5_DONE;
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
	Serial.println(F("Testprogramm komplett abgeschlossen !"));
	Serial.println(F("****************************************************************************************"));
}

//
// next lines are related to state BLOCKED
//
void execEnterStateBLOCKED() {
	state = BLOCKED;
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
	state = OVERLOAD;
	fastStopMotor_R();			// stop MOTOR R
	fastStopMotor_L();			// stop MOTOR L
	initializeFlashLightNewState(state);	// Warnlampe auf den neuen Status einstellen
	IsDoorOpening = !IsDoorOpening;	// Drehrichtung der Motoren ändern
	IsCurrentOverloaded = false;	// Fehlerflag wurde bearbeitet
}

//
// next lines are related to state OPENING
//
void execEnterStateOPENING()  {
	logDebug(FSM,#);
/* 	if (debugLevel) {
		strcpy(message, "Eintritt execEnterStateOPENING");
		logFSM();
	}
 */
	state = OPENING;
	Serial.println(F("****************************************************************************************"));
	Serial.println(F("Status: OPENING  -  Beide Tore werden jetzt geöffnet"));

	IsDoor_R_Blocked = false;
	IsDoor_L_Blocked = false;

	PWM_Motor_R_Target = parameter[state].Motor_R_Speed_Target;	// Zielgeschwindigkeit ermitteln
	PWM_Motor_L_Target = parameter[state].Motor_L_Speed_Target;	// Zielgeschwindigkeit ermitteln
	
	nextTimer_GatesDelay_Event = timestamp + GatesDelay;		// Timer für den verzögerten Start des linken Tores setzen
	
	startMotor_R(PWM_Motor_R, OpenDoor);						// rechten MOTOR starten
	initializeFlashLightNewState(state);						// Warnlampe auf den neuen Status einstellen

/* 	if (debugLevel) {
		strcpy(message, "Ende execEnterStateOPENING");
		logFSM();
	}
 */
 }

void execExitStateOPENING()  {
	logDebug(FSM,#);
/* 	if (debugLevel) {
		strcpy(message, "Begin execExitStateOPENING");
		logFSM();
	}
 */	Serial.println(F("****************************************************************************************"));
	Serial.println(F("Status: OPENING  -  Beide Tore sind jetzt bis zum Anschlag geöffnet"));

	// wenn das Öffnen von einem anderen Status aufgerufen wurde ....
	if (isCalledBy) {
		returnToParentStatus();			// ... in den aufrufenden Status zurückwechseln	
	}
	else {
		execEnterStateIDLE();			// wieder in den Status IDLE zurückkehren	
	}
/* 	if (debugLevel) {
		strcpy(message, "Ende execExitStateOPENING");
		logFSM();
	}

 */
	logDebug(FSM,#);
}


//
// next lines are related to state CLOSING
//
void execEnterStateCLOSING()  {
	state = CLOSING;
	startMotor_R(V_Motoren, IsDoorOpening);		// start MOTOR R
	startMotor_L(V_Motoren, IsDoorOpening);		// start MOTOR L
	initializeFlashLightNewState(state);		// Warnlampe auf den neuen Status einstellen
}

//
// next lines are related to state STOPPED
//
void execEnterStateSTOPPED()  {
	state = STOPPED;
	fastStopMotor_R();			// stop MOTOR R
	fastStopMotor_L();			// stop MOTOR L
	initializeFlashLightNewState(state);	// Warnlampe auf den neuen Status einstellen
	Serial.println (F("Die Tore wurden durch einen Tastendruck gestoppt."));
	IsDoorOpening = !IsDoorOpening;	// Drehrichtung der Motoren ändern
}

//
// next lines are related to state OPENED
//
void execEnterStateOPENED() {
	state = OPENED;
	fastStopMotor_R();	// stop MOTOR R
	fastStopMotor_L();	// stop MOTOR L
	initializeFlashLightNewState(state);	// Warnlampe auf den neuen Status einstellen
	IsDoorOpening = !IsDoorOpening;			// Drehrichtung der Motoren ändern
}


/* 
take selected test programm and move FSM into related state
 */
 void processTestSelection(byte testToLaunch) {
	 
	 switch (testToLaunch) {
		 case BLOCKCURRENT:
			state = PHASE1_TESTING;				// neuer Status: PHASE1_OPENING
			execEnterStatePHASE1();
		 break;
		 case MINMOVEPWM:
			state = PHASE2_TESTING;
			execEnterStatePHASE2_TESTING();
		 break;
		 case MOVETIME:
			state = PHASE3_TESTING;
			execEnterStatePHASE3_TESTING();
		 break;
		 case STARTNOTBLOCKING:
			state = PHASE4_CLOSING;
			execEnterStatePHASE4_CLOSING();
		 break;
		 case OPENGATES:
			execEnterStateOPENING();
		 break;
		 case CLOSEGATES:
			execEnterStateCLOSING();
		 break;
	 }
 }


void returnToParentStatus() {			// vom Sub-Status in den Parent-Status zurück wechseln	
	logDebug(FSM,#);
	state = isCalledBy;					// ab jetzt wieder den Parent-Status ausführen lassen 
//	isCalledBy = 0;						// IMPORTANT: it's parent's task to clear this
										// state = isCalledBy is indicator for parent that itself did call something in the past 
/*
	switch (state) {					// jetzt zum Parent status zurückspringen
		 case PHASE1_TESTING:
			execExecStatePHASE1();
		 break;
		 case PHASE2_TESTING:
			state = PHASE2_TESTING;
		 break;
		 case PHASE3_TESTING:
			state = PHASE3_TESTING;
			execEnterStatePHASE3_TESTING();
		 break;
		 case STARTNOTBLOCKING:
			execEnterStatePHASE4_CLOSING();
		 break;
		 case OPENGATES:
			state = OPENING;
			execEnterState_OPENING();
		 break;
		 case CLOSEGATES:
			state = CLOSING; 
			execEnterState_CLOSING();
		 break;
		
	}
*/
}

/*
	check if any timer expired and take proper actions
*/
void handleTimerEvents() {
//	logDebug(FSM,#);
//	logDebug(TIMER,#);
//	logDebug(MOTOR,#);
	// prüfen, ob der Arbeitsmodus bzw. der Zustand der Signallampe akualisiert werden muss
	if (IsFlashLightActive && (timestamp >= nextTimerFlashEvent)) {
		toggleFlashLight(IsFlashLightOn);
	}

	// prüfen, ob für einen der Motoren der PMW-Wert aktualisiert werden mus (beschleunigen / bremsen)
	if (IsMotor_R_Ramping && (timestamp >= nextTimer_Motor_R_Event)) {
		Update_PMW_Motor_R();
		if (state == PHASE2_TESTING) {			// in Test-Phase 2: 
			PWM_min_moving = PWM_Motor_R;		// Variable für den minimalen PWM-Wert an den neuen Wert anpassen
			Serial.print(F("Neuer PWM-Wert: "));	// und ausgeben
			Serial.println(PWM_min_moving);
		}
	}
	if (IsMotor_L_Ramping && (timestamp >= nextTimer_Motor_L_Event)) {
		Update_PMW_Motor_L();
	}

	// prüfen, ob beim Öffnen oder Schließen jetzt der zweite Motoren gestartet werden muss
	if (nextTimer_GatesDelay_Event > 0 && timestamp >= nextTimer_GatesDelay_Event) {		// 
//		logDebug(FSM,#);
		switch (state) {
		case OPENING:
			logDebug(FSM,#);
			startMotor_L(PWM_Motor_L, OpenDoor);		// beim Öffnen den linken Motor verzögert starten
		break;
		case CLOSING:
			logDebug(FSM,#);
			startMotor_R(PWM_Motor_R, CloseDoor);		// beim Schließen den rechten Motor verzögert starten
		break;
		default:
		break;
		}
//		logDebug(TIMER,#);
//		logDebug(MOTOR,#);
	}
}

