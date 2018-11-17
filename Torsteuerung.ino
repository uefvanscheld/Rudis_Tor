//Steuerung zum Öffenen und Schließen eines Zweiflügeligen Tores mit zwei Motoren
//  je eine H-Brücke pro Motor, Geschwindigkeit per PWM wählbar
//Zeitverzörgertes Starten der beiden Flügel, weil sich die Tore überlappen
//  Mit zwei Jumpern kann die Zeit für die Zeitverzögerung gewählt werden, 3, 4, 5, 6 s
//Endanschläge bzw. Blockade wird mittles der Stromaufnahme überwacht.
//  Bei überschreiten der Stromschwelle werden die Antriebe hardwaremäßig abgeschaltet
//  Durch Vergleich der Ausgänge das Controllers und der Rückführung 
//  der Steuereingänge der H-Brücken wird die Stromabschaltung erkannt
//Ausgang für Leuchte (LEDs), wenn sich das Tor bewegt sollen diese blinken (blitzen)

//Eingänge
const byte  Start_Funk=    6; //Start-Impuls Funkfernbedienung activ high
const byte  Start_Taste=   7; //Start-Impuls Funkfernbedienung activ low
const byte  Jumper1=       8; //Jumper 1 Kodierung Zeitverzögerung
const byte  Jumper2=      12; //Jumper 2 Kodierung Zeitverzögerung
const byte  Fb_H_Br_R_A=  14; //Feedback H-Brücke Rechts Auf
const byte  Fb_H_Br_R_Z=  15; //Feedback H-Brücke Rechts Zu
const byte  Fb_H_Br_L_A=  16; //Feedback H-Brücke Links Auf
const byte  Fb_H_Br_L_Z=  17; //Feedback H-Brücke Links Zu

//Ausgänge
const byte  H_Br_R_A=      2; //H-Brücke Rechts Auf
const byte  H_Br_R_Z=      3; //H-Brücke Rechts Zu
const byte  H_Br_L_A=      4; //H-Brücke Links Auf
const byte  H_Br_L_Z=      5; //H-Brücke Links Zu
const byte  Rst_I_Stopp=  10; //Reset Stromabschaltung
const byte  H_Br_En=      11; //H-Brücken Enable
const byte  Warnleuchte=  13; //Warnleuchte an

//Zeiten
const int   Wl_an=       200; //Zeit Warnleuchte an
const int   Wl_aus=      800; //Zeit Warnleuchte aus

//PWM Duty Cycle
const byte  V_Motoren=   255; //Geschwindigkeit der Motoren 255 = 100%




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
