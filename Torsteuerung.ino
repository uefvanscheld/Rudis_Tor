/*
 ************************************************
 *
 *	Version 0.98
 *			- NEU: Unterscheidung von Betriebsmodi : Normal und Testbetrieb
 *			- NEU: Status OPENING und CLOSING hinzugefügt
 *			- NEU: Auswahl eines gewünschten Testprogramms
 *			- NEU: Einführung von Sub-Status
 *			- NEU: die Tore werden beim Öffnen und Schließen jetzt nacheinander mit Verzögerung gestartet
 *			- 
 *			
 *			
 *			
 *	Version 0.97 (speziell angepasst für die geplanten Testfahrten)
 *				s.a. Task auf dem Glo-Board: https://app.gitkraken.com/glo/board/W-6H0tbmZwAaeaqf/card/XIq2RPkhKgAP0kDB
 *	- Ergänzung um eine Ablaufsteuerung, die die verschiedenen Testphasen steuert
 *	- Dies sind:
 *	- a) Messung des minimalen PMW-Wertes, der die Erkennung der Überlastsituation möglich macht
 *	- b) Ermittlung des minimalen PMW-Wertes, bei dem sich ein Tor übehaupt bewegt
 *	- c) Messungen der Laufzeiten der Tore bei verschiedenen PWM-Werten
 *	- 
 *	- 
 *	- ToDos: Überstromerkennung für die einzelnen Motoren in einer Routine zusammenfassen, aber trotzdem beide Flags setzen
 *	# Festgestellte Fehler am 9.9.2019:
 *	- erledigt: in der Testphase 3 wird bei der Messung der Torlaufzeit die Pause des Tores am Endanschlag (zur Beruhigung des Tores) mitgemessen
 *	- erledigt: Beim Ändern der Geschwindigkeit für jeden Zyklus spricht der Text immer vom Schließen
 *	- am Ende der 3 Tests sollte es möglich sein, die Tore wieder zu schließen
 *
 *	Version 0.96
 *	- Funktion (#22) zum Beschleunigen und Abbremsen der Tore eingebaut (getrennt für jedes Tor)
	  Es wird für jeden Status ein Zielgeschwindigkeit (PWM duty cycle) festgelegt,
	  die dann ausgehend von der vorgefundenen Geschwindigkeit angesteuert wird
 *	- funktioniert soweit (Stand 1.5.2019)
 *	-
 *	-
 *
 *
 *
 *	Neu in Version 0.95
 *	- Erkennung von langen Tastendrücken (Issue #17)
 *
 *
 *
 ************************************************	
 */
//Steuerung zum Öffenen und Schließen eines zweiflügeligen Tores mit zwei Motoren
//  je eine H-Brücke pro Motor, Geschwindigkeit per PWM wählbar
//Zeitverzörgertes Starten der beiden Flügel, weil sich die Tore überlappen
//  Mit zwei Jumpern kann die Zeit für die Zeitverzögerung gewählt werden, 3, 4, 5, 6 s
//Endanschläge bzw. Blockade wird mittles der Stromaufnahme überwacht.
//  Bei überschreiten der Stromschwelle werden die Antriebe hardwaremäßig abgeschaltet
//  Durch Vergleich der Ausgänge das Controllers und der Rückführung
//  der Steuereingänge der H-Brücken wird die Stromabschaltung erkannt
//Ausgang für Leuchte (LEDs), wenn sich das Tor bewegt sollen diese blinken (blitzen)

#include <Arduino.h>
#include "Torsteuerung.h"



void setup() {
	Serial.begin(115200);
	initialize_IO();					// alle Ein- und Ausgänge in einen definierten Zustand bringen
	initialize_FSM();					// den Status der Steuerung in einen definierten Status (=Geschlossen) bringen
	PWM_Motor_R =   0; 					//Rechter Motor, Initialisierung auf 0 (Motor aus)
	PWM_Motor_L =   0; 					//Linker Motor, Initialisierung auf 0 (Motor aus)
			
	test_phase = 0;						// Variable für Phasen initialisieren
	nextTimer_Motor_L_Event = millis();	// Timer des linken Motors für des zeitversetzte Schließen vorbereiten
	IsDoorOpening			= true;		// beide Tore werden zuerst geöffnet
/* 
	state = INITIALIZED;				// alle Werte initialisiert
	execEnterStateINITIALIZED();
*/
	operationMode = TESTING;			// for now set operation mode to testing 
	state = IDLE;						// alle Werte wurden initialisiert
	execEnterStateIDLE();
	
	// if in testing mode ask user for debug messages
	if (operationMode == TESTING) {
		debugLevel = getDebugLevel();
	}

	
}

void loop() {
  // put your main code here, to run repeatedly:
	// Aktuelle Zeit abfragen bzw. Zeitstempel ermitteln und zwischenspeichern;
	// wird für die komplette Dauer einer Schleife zur Prüfung anstehender Events (Stati, Motoren, Flashlight ...) genutzt
	timestamp = millis();

	/*
		jetzt auf wichtige Ereignisse prüfen:
		- Tastendruck (manuell oder Fernbedienung)
		- Zustand der Jumper zur Steuerung der Torverzögerung
		- Blockierung oder Überlast der Motoren
		- Zeitgesteuerte; Blinken der Signallampe, Beschleunigung /Bremsen der Motoren

		Diese Ereignisse lösen nachfolgend direkte Aktionen (z.B. Blockierung --> Motoren sofort stoppen) ...
		oder Statuswechsel aus (z.B. Geschlossen --> Tastendruck --> Öffnen)

	*/
	get_button_state();					// Status der Tasten abfragen
	get_jumper_status();				// Status der Jumper abfragen

	check_is_motor_overloaded();		// prüfe, ob die Überstromschaltung angesprochen hat
	/*
	* wird für die Testfahrten nicht benötigt
	if (IsCurrentOverloaded) {
		state = OVERLOAD;
		execEnterStateOVERLOAD();
	}
	*/
	// jetzt prüfen, ob die Motoren mehr Strom ziehen als normal (--> Hindernis oder Endanschlag)
	check_is_motor_blocked();
/*	Das wird für die Testfahrten nicht benötigt
	if (IsDoorBlocked && state != BLOCKED) {		// nur einmal ausführen
		state = BLOCKED;
		execEnterStateBLOCKED();
	}
*/
	// jetzt prüfen, ob irgendwelche Timer abgelaufen sind
//	logDebug(TIMER,#);
	handleTimerEvents();
	 
	/*
	*
	*	nun die Ereignisse bzw. Zustände auswerten und entsprechende Statusübergänge auslösen
	*
	*/

/*
    Serial.print (F("Vor switch:"));
	debugFlags();
*/
	switch (state) {
		case IDLE:
			if (operationMode == TESTING) {
					activeTestProgramm = getTestSelection();	// select for next test program to execute
					processTestSelection(activeTestProgramm);		// ... and take related actions ...
			}
			else {
				// do nothing in case of normal operation ???
				// just wait for a push button event to happen
			}
/* 
			if(IsButtonNeedsProcessing) {			// System ist initialisiert; ein Knopfdruck startet das Öffnen beider Tore
				state = PHASE1_OPENING;				// neuer Status: PHASE1_OPENING
				execEnterStatePHASE1_OPENING();
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
 */			
		break;
		
/* 		case PHASE1_OPENING:
			// beide Tore werden gerade geöffnet
			if (!(IsDoor_R_AtEndStop && IsDoor_L_AtEndStop)) {	// solange noch nicht beide Motoren am Anschlag sind
				if (IsDoor_R_Blocked) {							// falls der rechte Motor blockiert ist, stoppen
					fastStopMotor_R();
					if (!IsDoor_R_AtEndStop) Serial.println (F("Start Phase 1: Das rechte Tor hat jetzt den Anschlag erreicht (PHASE1_OPENING)...."));
					IsDoor_R_AtEndStop = true;
				}
				if (IsDoor_L_Blocked) {							// falls der linke Motor blockiert ist, stoppen
					fastStopMotor_L();
					if (!IsDoor_L_AtEndStop) Serial.println (F("Start Phase 1: Das linke Tor hat jetzt den Anschlag erreicht (PHASE1_OPENING)...."));
					IsDoor_L_AtEndStop = true;
				}
			}
			else if(IsButtonNeedsProcessing) {					// ... oder	falls die Taste gedrückt wurde, beide Motoren stoppen
				state = STOPPED;								// neuer Status: STOPPED
				execEnterStateSTOPPED();			
				IsButtonNeedsProcessing = false;				// Tastendruck wurde bearbeitet
			}
			else if(IsDoor_R_AtEndStop && IsDoor_L_AtEndStop){	// wenn beide Tore geöffnet sind, dann das Ermitteln des Abschalt-PWM-Wertes starten
				Serial.println (F("Phase 1: Beide Tore sind nun bis zum Anschlag geöffnet und gestoppt"));
				Serial.println (F("Phase 1: ...  3 Sekunden Pause zur Beruhigung der Tore ....."));
				state = PHASE1_TESTING;							// neuer Status: PHASE1_TESTING
				delay(3000);	// nur für Test am Breadboard: warten, bis die Stromtasten wieder losgelassen wurden
				execEnterStatePHASE1_TESTING();
			}
		break;
 */
		case PHASE1_TESTING:													// die Tests erfolgen nur mit dem linken Tor
			logDebug(FSM,#);
			// zuerst(!!) prüfen, ob zuvor an einen Substatus übergeben wurde (isCalledBy ist gesetzt und identisch zum eigenen/aktuellen Status) 
			if (isCalledBy == PHASE1_TESTING) {			
				// jetzt die Verwaltung des status-internen Zustandes
				if (subStateStack == 2) {			
					subStateStack--;											// Substatus nach Ausführung vom Stack löschen
					isCalledBy = 0;
					logDebug(FSM,#);
				}
				logDebug(FSM,#);
			}
			// wenn die Tore noch nicht geöffnet wurden, an den Substatus übergeben
			else if (subStateStack == 2 && isCalledBy == 0) {
				isCalledBy = PHASE1_TESTING;									// Aufruf an OPENING
				logDebug(FSM,#);
				execEnterStateOPENING();
				break;	// jetzt den Hauptstatus erstmal verlassen und den Substatus ausführen
			}
			if (subStateStack == 1 && isCalledBy == 0) {						// nach dem Öffnen der Tore ....
				execExecStatePHASE1();											// ... das eigentliche Testprogramm initiieren
				subStateStack--;												// und vermerken, dass das erledigt ist
				logDebug(FSM,#);
			}
			else {							// ab hier nun die eigentlich Ausführung von Testprogramm 1
				logDebug(FSM,#);
				if (!IsDoor_L_Blocked) {											// solange die Stomstärke noch nicht den Wert für die Hinderniserkennung erreicht hat...
					log_PWM_CURRENT();												// den aktuellen PWM-Wert und die gemessene Stromstärke (= Analogwert) ausgeben
					if (testing_next_event <= timestamp) {							// wenn die Testzeit zu Ende ist ... 
						PWM_Motor_L = PWM_Motor_L + PWM_min_blocking_inc; 			// PWM-Wert um den definierten Wert erhöhen ...	
						analogWrite(H_Br_L_En, PWM_Motor_L);						// PWM-Signal für den Motor aktualisieren ...
						testing_next_event = timestamp + PWM_min_blocking_duration; // ... und neuen Zeitpunkt für Beenden der nächsten Messung festlegen
						Serial.print(F("Neuer PWM-Wert: "));
						Serial.println(PWM_Motor_L);
					}
				}
				else {																// falls der Motor überlastet wurde ....
					log_PWM_CURRENT();												// den letzten PWM-Wert und die gemessene Stromstärke (= Analogwert) ausgeben
					PWM_min_blocking = PWM_Motor_L;									// den PWM-Wert sichern für Phase 2
					fastStopMotor_L();												// Motor sofort ausschalten
					Serial.print(F("Blockierstromstärke erkannt; der erforderliche PWM-Wert dafür beträgt: "));
					Serial.println(PWM_min_blocking);
					state = PHASE1_DONE;						// neuer Status: PHASE1_DONE
					execEnterStatePHASE1_DONE();					
				}
			}
		break;
		case PHASE1_DONE:
			state = IDLE;						// neuer Status: IDLE
			execEnterStateIDLE();
			IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
		break;
		case PHASE2_TESTING:								// die Tests erfolgen nur mit dem rechten Tor
			if(IsButtonNeedsProcessing) {					// wenn die Taste gedrückt wurde, wurde eine Bewegung erkannt und der rechte Motoren wird gestoppt
				fastStopMotor_R();							// stop MOTOR R
				fastStopMotor_L();							// stop MOTOR L
				Serial.print(F("Der minimale PWM-Wert um ein Tor zu bewegen beträgt: "));	// und ausgeben
				Serial.println(PWM_min_moving);
				state = PHASE2_DONE;						// neuer Status: PHASE2_DONE
				execEnterStatePHASE2_DONE();					
				IsButtonNeedsProcessing = false;			// Tastendruck wurde bearbeitet
			}
		case PHASE2_DONE:
			if(IsButtonNeedsProcessing) {			// der nächste Knopfdruck startet die Phase 3
				state = PHASE3_TESTING;				// neuer Status: PHASE3_TESTING
				execEnterStatePHASE3_TESTING();
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
		break;
		case PHASE3_TESTING:				// die Tests erfolgen nur mit dem linken Tor
			log_PWM_RUNTIME();				// aktuelle Zeit, den PWM-Wert und die gemessene Stromstärke (= Analogwert) ausgeben
			if (IsDoor_L_Blocked) {			// wenn die Tür am Anschlag ist ....
				IsDoor_L_Blocked = false;	// Blockade-Flag löschen
				Zielzeit_Tor = millis();	// Zeitpunkt festhalten
				log_Phase3_Anschlag();
				fastStopMotor_L();			// Motor sofort ausschalten
				delay(3000);				// warten, bis sich das Tor beruhigt hat
				// wenn bereits mit  maximaler Geschwindigkeit gemessen wurde und das Tor wieder geöffnet ist, ist die Phase 3 zu Ende
				if ((PWM_Motor_L_Target == PWM_max) && (Runtime_Direction_Opening )) {		
					state = PHASE3_DONE;					// neuer Status: PHASE3_DONE
					execEnterStatePHASE3_DONE();
					break;				
				}
				// Jetzt die Drehrichtung umdrehen; Phase 3 startet mit Schließen (Runtime_Direction_Opening = false)
				Runtime_Direction_Opening = !Runtime_Direction_Opening;
				// bevor das Tor wieder geschlossen wird (d.h. nach einem kompletten Zyklus aus Schließen und Öffnen), 
				// erst die Geschwindigkeit erhöhen ...
				if (Runtime_Direction_Opening == false) {
					// ... die neue Geschwindigkeit festlegen ...
					if ((PWM_max - PWM_Motor_L_Target) >= PWM_runtime_inc) {
						PWM_Motor_L_Target = PWM_Motor_L_Target + PWM_runtime_inc;
					}
					else {
						PWM_Motor_L_Target = PWM_max;
					}
					Serial.print (F("Messung der Torlaufzeit für Schließen jetzt mit PWM-Wert: "));
					Serial.print (PWM_Motor_L_Target);
					Serial.println (F(""));	
				}
				else {
					Serial.print (F("Messung der Torlaufzeit für Öffnen jetzt mit PWM-Wert: "));
					Serial.print (PWM_Motor_L_Target);
					Serial.println (F(""));	
				}

				// jetzt die Startzeit festhalten
				Startzeit_Tor = millis();

				// ... und den Motor wieder starten
				startMotor_L_Tests(PWM_Motor_L_Target, Runtime_Direction_Opening);	// Start von  MOTOR L  (geänderte Version!!)


			}
			else if(IsButtonNeedsProcessing) {					// ... oder	falls die Taste gedrückt wurde, beide Motoren stoppen
				state = STOPPED;								// neuer Status: STOPPED
				execEnterStateSTOPPED();			
				IsButtonNeedsProcessing = false;				// Tastendruck wurde bearbeitet
			}

		break;	// end of PHASE3_TESTING
		case PHASE3_DONE:
			if(IsButtonNeedsProcessing) {			// der nächste Knopfdruck startet die Phase 4
				state = PHASE4_TESTING;				// neuer Status: PHASE4_TESTING
				execEnterStatePHASE4_CLOSING();
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
		break;
		case PHASE4_CLOSING:								// die Tests erfolgen nur mit dem rechten Tor
			// zunächst das rechte Tor schließen, bis der Motor durch Loslassen der Taste gestoppt wird
			log_PWM_CURRENT_R();
			if(IsButtonNeedsProcessing) {					// solange die Taste gedrückt bleibt, nichts tun und den rechten Motor weiter laufen lassen
				IsButtonNeedsProcessing = false;			// Tastendruck wurde bearbeitet
			}
			else {					// wenn die Taste losgelassen wurde: stoppen der Motoren
				fastStopMotor_R();							// stop MOTOR R
				fastStopMotor_L();							// stop MOTOR L
				Serial.println(F("Motor gestoppt; nun beginnt die Suche nach der maximalen unbeschleunigten Startgeschwindigkeit ... "));	// und ausgeben
				state = PHASE4_TESTING;						// neuer Status: PHASE2_DONE
				execEnterStatePHASE4_TESTING();					
				IsButtonNeedsProcessing = false;			// Tastendruck wurde bearbeitet
			}
		break;
		case PHASE4_TESTING:								// die Tests erfolgen nur mit dem rechten Tor
			// nun das rechte Tor mit zunehmend höheren PWM ohne Beschleunigung starten, bis die Überlasterkennung anspricht
			// dabei jedesmal die Richtung wechseln
			log_PWM_CURRENT_R();
			check_is_motor_R_overloaded();		// Überlasterkennung für den rechten Motor abfragen
			// falls überlastet ...
			if (IsCurrent_R_Overloaded) {
				Serial.print(F("Überlastung des rechten Motors erkannt bei PWM-Wert"));
				Serial.println(PWM_max_non_blocking);
				IsCurrent_R_Overloaded = false;
				fastStopMotor_R();							// stop MOTOR R
				PWM_Motor_R = 0;
				sendSyncImpuls(); 							// Fehlerstatus zurücksetzen
				state = PHASE4_DONE;						// neuer Status: PHASE2_DONE
				execEnterStatePHASE4_DONE();					
			}
			// die Überstromerkennung hat NICHT angesprochen
			if ((timestamp > nextTimer_Motor_R_Event) && (PWM_Motor_R != 0)) {
				fastStopMotor_R();							// stop MOTOR R
				PWM_Motor_R = 0;
				Serial.println(F("Überlasterkennung hat nicht angesprochen; 8Sek Pause zum Ausschwingen des Tores...."));
				// Tor ausschwingen lassen
				nextTimer_Motor_R_Event = timestamp + PWM_max_non_blocking_pause;
			}
			// ... wenn das Tor ausgeschwungen hat, dann neuen Versuch starten
			else if ((timestamp > nextTimer_Motor_R_Event) && (PWM_Motor_R == 0)) {
				IsDoorOpening = !IsDoorOpening;		// Drehrichtung wechseln
				PWM_max_non_blocking = PWM_max_non_blocking + PWM_max_non_blocking_inc;		// PWM-Wert erhöhen
				PWM_Motor_R = PWM_max_non_blocking;
				Serial.print(F("Nächster Versuch mit PWM-Wert: "));	// und ausgeben
				Serial.println(PWM_Motor_R);
				startMotor_R(PWM_Motor_R, IsDoorOpening);			// Start von  MOTOR R ohne Beschleunigung
				nextTimer_Motor_R_Event = timestamp + PWM_max_non_blocking_duration;
			}
		break;
		case PHASE4_DONE:
			if(IsButtonNeedsProcessing) {			// der nächste Knopfdruck startet die Phase 5
				state = PHASE5_CLOSING;				// neuer Status: PHASE5_TESTING
				execEnterStatePHASE5_CLOSING();
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
		break;
		case PHASE5_CLOSING:
			// beide Tore werden gerade geöffnet
			if (!(IsDoor_R_AtEndStop && IsDoor_L_AtEndStop)) {	// solange noch nicht beide Motoren am Anschlag sind
				if (IsDoor_R_Blocked) {							// falls der rechte Motor blockiert ist, stoppen
					fastStopMotor_R();
					if (!IsDoor_R_AtEndStop) Serial.println (F("Start Phase 5: Das rechte Tor hat jetzt den Anschlag erreicht (PHASE5_CLOSING)...."));
					IsDoor_R_AtEndStop = true;
				}
				if (IsDoor_L_Blocked) {							// falls der linke Motor blockiert ist, stoppen
					fastStopMotor_L();
					if (!IsDoor_L_AtEndStop) Serial.println (F("Start Phase 5: Das linke Tor hat jetzt den Anschlag erreicht (PHASE5_CLOSING)...."));
					IsDoor_L_AtEndStop = true;
				}
			}
			else if(IsButtonNeedsProcessing) {					// ... oder	falls die Taste gedrückt wurde, beide Motoren stoppen
				state = STOPPED;								// neuer Status: STOPPED
				execEnterStateSTOPPED();		
				IsButtonNeedsProcessing = false;				// Tastendruck wurde bearbeitet
			}
			else if(IsDoor_R_AtEndStop && IsDoor_L_AtEndStop){	// wenn beide Tore geschlossen sind, dann Testprogramm beenden
				Serial.println (F("Phase 5: Beide Tore sind nun geschlossen"));
				state = PHASE5_DONE;							// neuer Status: PHASE1_TESTING
				execEnterStatePHASE5_DONE();
			}
		break;
		case OPENING:
			logDebug(FSM,#);
/* 			if (debugLevel) {
				strcpy(message, "Start: case OPENING");
				logFSM();
			}
 */			// beide Tore werden geöffnet
			if (!(IsDoor_R_AtEndStop && IsDoor_L_AtEndStop)) {	// solange noch nicht beide Motoren am Anschlag sind
				if (IsDoor_R_Blocked) {							// falls der rechte Motor blockiert ist, stoppen
					fastStopMotor_R();
					if (!IsDoor_R_AtEndStop) Serial.println (F("OPENING: Das rechte Tor hat jetzt den Anschlag erreicht ...."));
					IsDoor_R_AtEndStop = true;
				}
				if (IsDoor_L_Blocked) {							// falls der linke Motor blockiert ist, stoppen
					fastStopMotor_L();
					if (!IsDoor_L_AtEndStop) Serial.println (F("OPENING: Das linke Tor hat jetzt den Anschlag erreicht (PHASE1_OPENING)...."));
					IsDoor_L_AtEndStop = true;
				}
			}
			else if(IsButtonNeedsProcessing) {					// ... oder	falls die Taste gedrückt wurde, beide Motoren stoppen
				state = STOPPED;								// neuer Status: STOPPED
				execEnterStateSTOPPED();			
				IsButtonNeedsProcessing = false;				// Tastendruck wurde bearbeitet
			}
			else if(IsDoor_R_AtEndStop && IsDoor_L_AtEndStop){	// wenn beide Tore geöffnet sind, den Status verlassen (je nach Bedingung)
				execExitStateOPENING();							// ... den Status verlassen
/* 
				Serial.println (F("Phase 1: Beide Tore sind nun bis zum Anschlag geöffnet und gestoppt"));
				Serial.println (F("Phase 1: ...  3 Sekunden Pause zur Beruhigung der Tore ....."));
				state = PHASE1_TESTING;							// neuer Status: PHASE1_TESTING
				delay(3000);	// nur für Test am Breadboard: warten, bis die Stromtasten wieder losgelassen wurden
				execEnterStatePHASE1_TESTING();
 */
			}
/* 			if (debugLevel) {
				strcpy(message, "Ende: case OPENING");
				logFSM();
			}
 */		break;

		
		default:	

		break;
	}

#ifdef DEBUG
	log_Debugging();
#endif	// DEBUG

}	// end of main loop

