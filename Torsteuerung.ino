/*
 ************************************************
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
//Steuerung zum Öffenen und Schließen eines Zweiflügeligen Tores mit zwei Motoren
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
#include "IOControl.ino"

void initialize_FSM();

void initialize_FSM() {
	IsDoorOpening			= true;		// Tor wird geöffnet; false --> schließen
	IsCurrentOverloaded		= false;	// Hardware-Strombegrenzung hat nicht angesprochen
	IsDoorBlocked			= false;	// Tür ist nicht blockiert
	IsButtonNeedsProcessing = false;	// keine Taste wurde betätigt, daher keine Aktion notwendig
	IsButtonReleased		= true;		// es ist gerade keine Taste gedrückt
	IsMotorSpeedUpdated		= false;	// keine neue Motorgeschwindigkeit eingestellt

	state = CLOSED;					// Annahme: Tor ist zu Beginn geschlossen
}

void setup() {
	Serial.begin(115200);
	initialize_IO();		// alle Ein- und Ausgänge in einen definierten Zustand bringen
	initialize_FSM();		// den Status der Steuerung in einen definierten Status (=Geschlossen) bringen
	PWM_Motor_R =   0; 		//Rechter Motor, Initialisierung auf 0 (Motor aus)
	PWM_Motor_L =   0; 		//Linker Motor, Initialisierung auf 0 (Motor aus)

}

void loop() {
  // put your main code here, to run repeatedly:
	// Aktuelle Zeit abfragen bzw. Zeitstempel ermitteln und zwischenspeichern;
	// wird für die komplette Dauer einer Schleife zur Prüfung anstehender Events (Stati, Motoren, Flashlight ...) genutzt
	timestamp = millis();

	// V_Motoren = get_motor_speed();		// Gewünschte Motorgeschwindigkeit abfragen
/*
	PWM_Motor_R = get_motor_speed();		// Potentiometer für gewünschte Motorgeschwindigkeit für jeden Motor einzeln abfragen
	PWM_Motor_L = get_motor_speed();		// Potentiometer für gewünschte Motorgeschwindigkeit für jeden Motor einzeln abfragen
*/

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
	if (IsCurrentOverloaded) {
		state = OVERLOAD;
		execEnterStateOVERLOAD();
	}
	// jetzt prüfen, ob die Motoren mehr Strom ziehen als normal (--> Hindernis oder Endanschlag)
	check_is_motor_blocked();
	if (IsDoorBlocked && state != BLOCKED) {		// nur einmal ausführen
		state = BLOCKED;
		execEnterStateBLOCKED();
	}
	// prüfen, ob der Arbeitsmodus bzw. der Zustand der Signallampe akualisiert werden muss
	if (IsFlashLightActive && (timestamp >= nextTimerFlashEvent)) {
		toggleFlashLight(IsFlashLightOn);
	}

	// prüfen, ob für einen der Motoren der PMW-Wert aktualisiert werden mus (beschleunigen / bremsen)
	if (IsMotor_R_Ramping && (timestamp >= nextTimer_Motor_R_Event)) {
		Update_PMW_Motor_R();
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
    Serial.print ("Vor switch:");
	debugFlags();
*/
	switch (state) {
		case CLOSED:
			if(IsButtonNeedsProcessing) {			// Tor ist geschlossen; eine Taste wird gedrückt --> Öffnen
				state = OPENING;					// neuer Status: OPENING
				execEnterStateOPENING();
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
		break;
		case OPENING:
			// Tor öffnet sich gerade
			if(IsDoorAtEndStop) {
				state = OPENED;					// neuer Status: OPENED; Tor am Endanschlag
				execEnterStateOPENED();
				IsDoorAtEndStop = false;		// Fehlerflag wurde bearbeitet
			}
			else if(IsButtonNeedsProcessing) {
				state = STOPPED;					// neuer Status: STOPPED
				execEnterStateSTOPPED();
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
			else if (IsMotorSpeedUpdated) {
				updateMotorSpeed(V_Motoren);		// PWM-Signal für Motor anpassen
				IsMotorSpeedUpdated = false;		// Flag zurücksetzen
			}
		break;
		case STOPPED:
			if(IsButtonNeedsProcessing) {
				if (BottonWasPressedLong)
					IsDoorOpening = !IsDoorOpening;			// Drehrichtung der Motoren DOCH NICHT ändern
				if(IsDoorOpening) {
					state = OPENING;					// neuer Status: OPENING
					execEnterStateOPENING();
				}
				else {
					state = CLOSING;					// neuer Status: CLOSING
					execEnterStateCLOSING();
				}
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
		break;
		case CLOSING:
			if(IsDoorBlocked) {
				state = BLOCKED;					// neuer Status: BLOCKED; Tor ist auf ein Hindernis gestossen
				execEnterStateBLOCKED();
				IsDoorBlocked = false;				// Fehlerflag wurde bearbeitet
			}
			else if(IsButtonNeedsProcessing) {
				state = STOPPED;					// neuer Status: STOPPED
				execEnterStateSTOPPED();
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
			else if (IsMotorSpeedUpdated) {
				updateMotorSpeed(V_Motoren);		// PWM-Signal für Motor anpassen
				IsMotorSpeedUpdated = false;		// Flag zurücksetzen
			}
		break;
		case BLOCKED:
			if(IsButtonNeedsProcessing) {
				if(IsDoorOpening) {
					state = OPENING;					// neuer Status: OPENING
					execEnterStateOPENING();
				}
				else {
					state = CLOSING;					// neuer Status: CLOSING
					execEnterStateCLOSING();
				}
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
		break;
		case OVERLOAD:
			if(IsButtonNeedsProcessing) {
				if(IsDoorOpening) {
					state = OPENING;					// neuer Status: OPENING
					execEnterStateOPENING();
				}
				else {
					state = CLOSING;					// neuer Status: CLOSING
					execEnterStateCLOSING();
				}
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
		break;
		case OPENED:
			if(IsButtonNeedsProcessing) {
				state = CLOSING;					// neuer Status: CLOSING
				execEnterStateCLOSING();
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
		break;
		default:

		break;
	}
	// end of main loop

	Serial.print (millis());
    Serial.print (";\t st:");
	Serial.print (state);
    Serial.print (";\t PWM-R:");
	Serial.print (PWM_Motor_R);
    Serial.print (";\t PWM-L:");
	Serial.print (PWM_Motor_L);
/*
    Serial.print (";\t I-R:");
	Serial.print (Mot_R_Current);
    Serial.print (";\t I-L:");
	Serial.print (Mot_L_Current);
    Serial.print (";\t F-on:");
	Serial.print (flash_on_duration);
    Serial.print (";\t F-off:");
	Serial.print (flash_off_duration);
    Serial.print (";\t t_next:");
	Serial.print (nextTimerFlashEvent);

    Serial.print (";\t IsFlashLightActive:");
	Serial.print (IsFlashLightActive);
    Serial.print (";\t IsFlashLightOn:");
	Serial.print (IsFlashLightOn);

	debugFlags();
 */

    Serial.println ("");

}
