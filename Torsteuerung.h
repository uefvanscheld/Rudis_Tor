/*
 * Here we define all constants and variables used in this program
 */
 
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


