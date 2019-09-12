/*
 ************************************************
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

//#define DEBUG DEBUG

void setup() {
	Serial.begin(115200);
	initialize_IO();					// alle Ein- und Ausgänge in einen definierten Zustand bringen
	initialize_FSM();					// den Status der Steuerung in einen definierten Status (=Geschlossen) bringen
	PWM_Motor_R =   0; 					//Rechter Motor, Initialisierung auf 0 (Motor aus)
	PWM_Motor_L =   0; 					//Linker Motor, Initialisierung auf 0 (Motor aus)
			
	test_phase = 0;						// Variable für Phasen initialisieren
	nextTimer_Motor_L_Event = millis();	// Timer des linken Motors für des zeitversetzte Schließen vorbereiten
	IsDoorOpening			= true;		// beide Tore werden zuerst geöffnet
	state = INITIALIZED;				// alle Werte initialisiert
	execEnterStateINITIALIZED();
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

	/*
	*
	*	nun die Ereignisse bzw. Zustände auswerten und entsprechende Statusübergänge auslösung
	*
	*/

/*
    Serial.print (F("Vor switch:"));
	debugFlags();
*/
	switch (state) {
		case INITIALIZED:
			if(IsButtonNeedsProcessing) {			// System ist initialisiert; ein Knopfdruck startet das Öffnen beider Tore
				state = PHASE1_OPENING;				// neuer Status: PHASE1_OPENING
				execEnterStatePHASE1_OPENING();
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
			// execEnterStateINITIALIZED();
		break;
		case PHASE1_OPENING:
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
		case PHASE1_TESTING:													// die Tests erfolgen nur mit dem linken Tor
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
				state = PHASE1_DONE;											// neuer Status: PHASE1_DONE
				execEnterStatePHASE1_DONE();
			}
		break;
		case PHASE1_DONE:
			if(IsButtonNeedsProcessing) {			// der nächste Knopfdruck startet die Phase 2
				state = PHASE2_TESTING;				// neuer Status: PHASE2_TESTING
				execEnterStatePHASE2_TESTING();
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
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

		default:

		break;
	}

#ifdef DEBUG
	log_Debugging();
#endif	// DEBUG

}	// end of main loop

