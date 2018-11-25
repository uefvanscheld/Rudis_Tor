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
	IsCurrentOverrun		= false;	// Hardware-Strombegrenzung hat nicht angesprochen
	IsDoorBlocked			= false;	// Tür ist nicht blockiert
	IsButtonNeedsProcessing = false;	// keine Taste wurde betätigt, daher keine Aktion notwendig
	IsButtonReleased		= true;		// es ist gerade keine Taste gedrückt

	state = CLOSED;					// Annahme: Tor ist zu Beginn geschlossen
}

void setup() {
	state_d state;	//
	initialize_IO();
	initialize_FSM();
}

void loop() {
  // put your main code here, to run repeatedly:

}
