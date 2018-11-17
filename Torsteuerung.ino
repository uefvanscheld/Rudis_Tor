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




void setup() {
  pinMode(Start_Funk,   INPUT);
  pinMode(Start_Taste,  INPUT_PULLUP);
  pinMode(Jumper1,      INPUT_PULLUP);
  pinMode(Jumper2,      INPUT_PULLUP);
  pinMode(Fb_H_Br_R_A,  INPUT);
  pinMode(Fb_H_Br_R_Z,  INPUT);
  pinMode(Fb_H_Br_L_A,  INPUT);
  pinMode(Fb_H_Br_L_Z,  INPUT);

  pinMode(H_Br_R_A,     OUTPUT);
  pinMode(H_Br_R_Z,     OUTPUT);
  pinMode(H_Br_L_A,     OUTPUT);
  pinMode(H_Br_L_Z,     OUTPUT);
  pinMode(Rst_I_Stopp,  OUTPUT);
  pinMode(H_Br_En,      OUTPUT);
  pinMode(Warnleuchte,  OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
