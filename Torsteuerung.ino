//Steuerung zum Öffenen und Schließen eines Zweiflügeligen Tores mit zwei Motoren
//  je eine H-Brücke pro Motor, Geschwindigkeit per PWM wählbar
//Zeitverzörgertes Starten der beiden Flügel, weil sich die Tore überlappen
//  Mit zwei Jumpern kann die Zeit für die Zeitverzögerung gewählt werden, 3, 4, 5, 6 s
//Endanschläge bzw. Blockade wird mittles der Stromaufnahme überwacht.
//  Bei überschreiten der Stromschwelle werden die Antriebe hardwaremäßig abgeschaltet
//  Durch Vergleich der Ausgänge das Controllers und der Rückführung 
//  der Steuereingänge der H-Brücken wird die Stromabschaltung erkannt
//Ausgang für Leuchte (LEDs), wenn sich das Tor bewegt sollen diese blinken (blitzen)

#include "Torsteuerung.h"


void initialize_FSM() {
	IsDoorOpening			= true;		// Tor wird geöffnet; false --> schließen
	IsCurrentOverloaded		= false;	// Hardware-Strombegrenzung hat nicht angesprochen
	IsDoorBlocked			= false;	// Tür ist nicht blockiert
	IsButtonNeedsProcessing = false;	// keine Taste wurde betätigt, daher keine Aktion notwendig
	IsButtonReleased		= true;		// es ist gerade keine Taste gedrückt

	state = CLOSED;					// Annahme: Tor ist zu Beginn geschlossen
}

void setup() {
	Serial.begin(115200);
	initialize_IO();
	initialize_FSM();
	
}

void loop() {
  // put your main code here, to run repeatedly:
	V_Motoren = get_motor_speed();		// Gewünschte Motorgeschwindigkeit abfragen
	get_button_state();					// Status der Tasten abfragen
	check_is_motor_overloaded();		// prüfe, ob Überstromschaltung angesprochen hat
	if (IsCurrentOverloaded) {
		state = OVERLOAD;
		execEnterStateOVERLOAD();
	}
	check_is_motor_blocked();			// prüfe, ob Tor blockiert ist
	if (IsDoorBlocked && state != BLOCKED) {		// nur einmal ausführen
		state = BLOCKED;
		execEnterStateBLOCKED();
	}
	// prüfen, ob die Signallampe akualisiert werden muss
	if (IsFlashLightActive && (millis() >= nextTimerFlashEvent)) {
		toggleFlashLight(IsFlashLightOn);
	}
	switch (state) {
		case CLOSED:
			if(IsButtonNeedsProcessing) {
				state = OPENING;					// neuer Status: OPENING
				execEnterStateOPENING();
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
		break;
		case OPENING:
			if(IsButtonNeedsProcessing) {
				state = STOPPED;					// neuer Status: STOPPED
				execEnterStateSTOPPED();
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
			}
		break;
		case STOPPED:
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
		case CLOSING:
			if(IsButtonNeedsProcessing) {
				state = STOPPED;					// neuer Status: STOPPED
				execEnterStateSTOPPED();
				IsButtonNeedsProcessing = false;	// Tastendruck wurde bearbeitet
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
		
		break;
		default:
		
		break;
	}
	// end of main loop
    Serial.print (millis());
    Serial.print (": st:");
	Serial.print (state);
    Serial.print (": I-R:");
	Serial.print (Mot_R_Current);
    Serial.print (": I-L:");
	Serial.print (Mot_L_Current);
    Serial.print (": F-on:");
	Serial.print (flash_on_duration);
    Serial.print (": F-off:");
	Serial.print (flash_off_duration);
    Serial.print (": t_next:");
	Serial.print (nextTimerFlashEvent);
    Serial.print (": IsFlashLightActive:");
	Serial.print (IsFlashLightActive);
    Serial.print (": IsFlashLightOn:");
	Serial.print (IsFlashLightOn);
    
    Serial.println ("");
}
