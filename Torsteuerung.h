/*
 * Here we define all constants and variables used in this program
 */
 
//Eingänge digital
#define  Start_Funk	    6 	//Start-Impuls Funkfernbedienung activ high
#define  Start_Taste    7 	//Start-Impuls Funkfernbedienung activ low
#define  Jumper1        8 	//Jumper 1 Kodierung Zeitverzögerung
#define  Jumper2       12 	//Jumper 2 Kodierung Zeitverzögerung
#define  Fb_H_Br_R_A   14 	//= A0 Feedback H-Brücke Rechts Auf
#define  Fb_H_Br_R_Z   15 	//= A1 Feedback H-Brücke Rechts Zu
#define  Fb_H_Br_L_A   16 	//= A2 Feedback H-Brücke Links Auf
#define  Fb_H_Br_L_Z   17 	//= A3 Feedback H-Brücke Links Zu

// Eingänge analog
#define	 Strom_L		A5	// Strommessung Motor Links
#define	 Strom_R		A4	// Strommessung Motor Rechts
#define	 MotorSpeedPin	A6	// Potentiometer zur Einstellung der Motor-Geschwindigkeit

//Ausgänge
#define  H_Br_R_A       2 	//H-Brücke Rechts Auf
#define  H_Br_R_Z       3 	//H-Brücke Rechts Zu
#define  H_Br_L_A       4 	//H-Brücke Links Auf
#define  H_Br_L_Z       5 	//H-Brücke Links Zu
#define  Rst_I_Stopp    10 	//Reset Stromabschaltung
#define  H_Br_R_En      11 	//H-Brücke Motor Rechts Enable
#define  H_Br_L_En      9 	//H-Brücke Motor Links Enable
#define  Warnleuchte    13 	//Warnleuchte an

// Timing für Warnleuchte
#define  Wl_an        200 //Zeit Warnleuchte an
#define  Wl_aus       800 //Zeit Warnleuchte aus

//PWM Duty Cycle
int  V_Motoren =   0; //Geschwindigkeit der Motoren 255 = 100%

// Steuerungs-Flags
boolean	IsDoorOpening			= true;		// Richtung der Torbewegung: true --> Tor wird geöffnet; false --> schließen
boolean	IsCurrentOverloaded		= false;	// Hardware-Strombegrenzung hat nicht angesprochen
boolean IsDoorBlocked			= false;	// Tür ist nicht blockiert
boolean IsButtonNeedsProcessing = false;	// keine Taste wurde betätigt, daher keine Aktion notwendig
boolean IsButtonReleased		= true;		// es ist gerade keine Taste gedrückt
 
// possible states of the door control
enum state_list {CLOSED, OPENING, STOPPED, CLOSING, BLOCKED, OVERLOAD, OPENED };
state_list state = CLOSED;

// various var for motor control
int speed_pot = 0;			// Wert des Potentiometers, zur Einstellung der Motor-Geschwindigkeit
int	Mot_R_Current = 0;		// Variable für die Stromstärke am Motor Rechts
int Mot_L_Current = 0;		// Variable für die Stromstärke am Motor Links
char portD_Bitmask = B00111100;	// nur Pins D2-D5 betrachten
char portC_Bitmask = B00001111;	// nur Pins A0-A3 betrachten

// various variables for flash light control
boolean IsFlashLightOn			= false;	// Flag für den Zustand der Signallampe
boolean IsFlashLightActive		= false;	// Flag für die Nutzung der Signallampe
unsigned int flash_on_duration;				// Einschaltdauer der Signallampe im aktuellen Status [ms]
unsigned int flash_off_duration;			// Ausschaltdauer der Signallampe im aktuellen Status [ms]

// define variables for controlling timer events
unsigned long	nextTimerFlashEvent;	// Variable, die den Zeitpunkt für das nächste Schalten der Signallampe enthält
unsigned long	nextTimerMotorEvent;	// Variable, die den Zeitpunkt für das nächste Schalten der Motoren enthält
unsigned long	timestamp;				// Variable, die den aktuellen Zeitpunkt für weitere Berechnung zwischenspeichert


// define parameters for the different states 
typedef struct
 {
    int M_R_spd;				// Geschwindigkeit für den rechten Motor (0...255)
    int M_L_spd;				// Geschwindigkeit für den linken Motor (0...255)
    unsigned long duration;		// [ms]; wie lange soll der Status erhalten bleiben; 0, wenn keine Zeitbegrenzung erforderlich
	int curr_limit;				// (0...1024) Stromstärke, bei der der Status beendet werden soll; 0, wenn nicht relevant
								// Hindernis: 	1,3A --> 281
								// Endanschlag:	1,5A --> 324
	unsigned int flash_on;		// [ms]; Dauer, wie lange die Warnlampe in diesem Betriebsmodus eingeschaltet sein soll
	unsigned int flash_off;		// [ms]; Dauer, wie lange die Warnlampe ausgeschaltet sein soll
 }  state_params;

state_params parameter[7] = {
	//	speed R (0.255)	speed L,	duration [ms],	Stromstärke	,	Flash On,	Flash off
	{ 	128,			128, 		2000,			281,			0,			0		 	},		// #0 == CLOSED
	{ 	255,			255, 		16000,			324,			200,		800		 	},		// #1 == OPENING
	{ 	0,				0,	 		0,				0,				0,			0		 	},		// #2 == STOPPED
	{ 	255,			255, 		18000,			324,			200,		800		 	},		// #3 == CLOSING
	{ 	0,				0,	 		0,				0,				300,		300		 	},		// #4 == BLOCKED
	{ 	0,				0,	 		0,				0,				50,			50		 	},		// #5 == OVERLOAD
	{ 	0,				0,	 		0,				0,				0,			0		 	}		// #6 == OPENED
};
