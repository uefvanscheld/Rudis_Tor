/*
 * Here we define all constants and variables used in this program
 */
 
//Eingänge digital
#define  Start_Funk=    6; //Start-Impuls Funkfernbedienung activ high
#define  Start_Taste    7; //Start-Impuls Funkfernbedienung activ low
#define  Jumper1        8; //Jumper 1 Kodierung Zeitverzögerung
#define  Jumper2       12; //Jumper 2 Kodierung Zeitverzögerung
#define  Fb_H_Br_R_A   14; //= A0; Feedback H-Brücke Rechts Auf
#define  Fb_H_Br_R_Z   15; //= A1; Feedback H-Brücke Rechts Zu
#define  Fb_H_Br_L_A   16; //= A2; Feedback H-Brücke Links Auf
#define  Fb_H_Br_L_Z   17; //= A3; Feedback H-Brücke Links Zu
// Eingänge analog
#define	 Strom_L		A5;	// Strommessung Motor Links
#define	 Strom_R		A4;	// Strommessung Motor Rechts
#define	 MotorSpeedPin	A6;	// Potentiometer zur Einstellung der Motor-Geschwindigkeit

//Ausgänge
#define  H_Br_R_A       2; //H-Brücke Rechts Auf
#define  H_Br_R_Z       3; //H-Brücke Rechts Zu
#define  H_Br_L_A       4; //H-Brücke Links Auf
#define  H_Br_L_Z       5; //H-Brücke Links Zu
#define  Rst_I_Stopp    10; //Reset Stromabschaltung
#define  H_Br_R_En      11; //H-Brücke Motor Rechts Enable
#define  H_Br_L_En      9; 	//H-Brücke Motor Links Enable
#define  Warnleuchte    13; //Warnleuchte an

// Timing für Warnleuchte
#define  Wl_an        200; //Zeit Warnleuchte an
#define  Wl_aus       800; //Zeit Warnleuchte aus

//PWM Duty Cycle
char  V_Motoren =   0; //Geschwindigkeit der Motoren 255 = 100%

// Steuerungs-Flags
boolean	IsDoorOpening			= true;		// Tor wird geöffnet; false --> schließen
boolean	IsCurrentOverrun		= false;	// Hardware-Strombegrenzung hat nicht angesprochen
boolean IsDoorBlocked			= false;	// Tür ist nicht blockiert
boolean IsButtonNeedsProcessing = false;	// keine Taste wurde betätigt, daher keine Aktion notwendig
boolean IsButtonReleased		= true;		// es ist gerade keine Taste gedrückt
 
// possible states of the door control
enum state_list{ CLOSED, OPENING, STOPPED, CLOSING, BLOCKED, OVERLOAD };
state_list state;


int speed_pot = 0;			// Wert des Potentiometers, zur Einstellung der Motor-Geschwindigkeit
int	Mot_R_Current = 0;		// Variable für die Stromstärke am Motor Rechts
int Mot_L_Current = 0;		// Variable für die Stromstärke am Motor Links

