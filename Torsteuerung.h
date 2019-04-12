/*
 * Here we define all constants and variables used in this program
 *
 *	V0.96: struktur state_params erweitert um folgende Parameter für's Bremsen und Beschleunigen, getrennt für jeden Motor:
 *			- Zielgeschwindigkeit in duty cycle [0..255; byte]
 *			- Zeitdauer [ms; integer] nach der die Geschwindigkeit jeweils wieder um eine Stufe erhöht/verringert wird
 *			- Wert des duty cycles [byte], um den die Geschwindigkeit jeweils verändert wird (+/-) 			
 * 
 */
 
//Eingänge digital
#define  Start_Funk	    6 	//Start-Impuls Funkfernbedienung activ high
#define  Start_Taste    7 	//Start-Impuls Funkfernbedienung activ low
#define  Jumper1        8 	//Jumper 1 Kodierung Zeitverzögerung
#define  Jumper2       12 	//Jumper 2 Kodierung Zeitverzögerung
#define  Fb_H_Br_R_A   A0 	//= A0 Feedback H-Brücke Rechts Auf
#define  Fb_H_Br_R_Z   A1 	//= A1 Feedback H-Brücke Rechts Zu
#define  Fb_H_Br_L_A   A2 	//= A2 Feedback H-Brücke Links Auf
#define  Fb_H_Br_L_Z   A3 	//= A3 Feedback H-Brücke Links Zu

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


// Steuerungs-Flags
boolean	IsDoorOpening			= true;		// Richtung der Torbewegung: true --> Tor wird geöffnet; false --> schließen
boolean	IsCurrentOverloaded		= false;	// Hardware-Strombegrenzung hat nicht angesprochen
boolean IsDoorBlocked			= false;	// Tür ist nicht blockiert
boolean IsDoorAtEndStop			= false;	// Tür ist nicht am Endanschlag 
boolean IsMotorSpeedUpdated		= false;	// am Poti wurde eine neue Motorgeschwindigkeit eingestellt (PWM duty cycle)
boolean IsJumper1Active			= false;	// ist Jumper 1 gesteckt  ?
boolean IsJumper2Active			= false;	// ist Jumper 2 gesteckt  ?

// Variablen für Tastensteuerung inkl. lange und kurzer Tastdrücke
boolean IsButtonReleased		= true;		// Indikator für: es ist gerade keine Taste gedrückt
boolean	IsBottonPressed			= false;	// Indikator für: eine Taste wird gerade gedrückt
boolean IsButtonNeedsProcessing = false;	// Indikator für: eine Taste wurde betätigt, daher ist eine Aktion erforderlich
boolean	BottonWasPressedShort	= false;	// Indikator für: es war ein kurzer Tastendruck
boolean	BottonWasPressedLong	= false;	// Indikator für: es war ein langer Tastendruck	
unsigned long	tsButtonWasPressed;					// Zeitpunkt in [msec], wann die Taste herunter gedrueckt wurde
unsigned int	ButtonLongPressDuration	= 3000;		// Zeit in [msec], die eine Taste gedrückt werden muss, damit ein langer Tasetndruck erkannt wird 


// various var for motor control
int speed_pot = 0;					// Wert des Potentiometers, zur Einstellung der Motor-Geschwindigkeit
unsigned int	Mot_R_Current = 0;	// Variable für die Stromstärke am Motor Rechts
unsigned int 	Mot_L_Current = 0;	// Variable für die Stromstärke am Motor Links
byte portD_Bitmask = B00111100;		// nur Pins D2-D5 betrachten
byte portC_Bitmask = B00001111;		// nur Pins A0-A3 betrachten

// various variables for flash light control
boolean IsFlashLightOn			= false;	// Flag für den Zustand der Signallampe
boolean IsFlashLightActive		= false;	// Flag für die Nutzung der Signallampe
unsigned int flash_on_duration;				// Einschaltdauer der Signallampe im aktuellen Status [ms]
unsigned int flash_off_duration;			// Ausschaltdauer der Signallampe im aktuellen Status [ms]

// define variables for controlling timer events
unsigned long	nextTimerFlashEvent;		// Variable, die den Zeitpunkt für das nächste Schalten der Signallampe enthält
unsigned long	nextTimer_Motor_R_Event;	// Variable, die den Zeitpunkt für das nächste Schalten der Motoren enthält
unsigned long	nextTimer_Motor_L_Event;	// Variable, die den Zeitpunkt für das nächste Schalten der Motoren enthält
unsigned long	timestamp;					// Variable, die den aktuellen Zeitpunkt für weitere Berechnung zwischenspeichert

//PWM Duty Cycle and motor control variables
byte  V_Motoren =   0; 					//Geschwindigkeit der Motoren 255 = 100% ---> überflüssig, wenn Beschleunigungsfunktion fertig implementiert ist
byte  PWM_Motor_R =   0; 				//Geschwindigkeit des rechten Motors, Initialisierung auf 0 (Motor aus); 255 = 100%
byte  PWM_Motor_L =   0; 				//Geschwindigkeit des linken Motors, Initialisierung auf 0 (Motor aus); 255 = 100%
boolean IsMotor_R_Ramping = false;		// Flag, das anzeigt, ob sich der Motor gerade in einer Beschleunigungs- bzw. Bremsphase befindet
boolean IsMotor_L_Ramping = false;		// Flag, das anzeigt, ob sich der Motor gerade in einer Beschleunigungs- bzw. Bremsphase befindet
boolean IsMotor_R_SpeedingUp = false;	// Flag, das anzeigt, ob der Motor gerade beschleunigt (=true) oder abbremst (=false)
boolean IsMotor_L_SpeedingUp = false;	// Flag, das anzeigt, ob der Motor gerade beschleunigt (=true) oder abbremst (=false)
 
// possible states of the door control and their numeric equivalent
enum state_list {	CLOSED, 	// 0
					OPENING,	// 1
					STOPPED,	// 2
					CLOSING,	// 3
					BLOCKED,	// 4
					OVERLOAD,	// 5
					OPENED		// 6
				};

// create instance and initialize				
state_list state = CLOSED;

// define parameters for the different states 
typedef struct
 {
	byte Motor_R_Speed_Target;				// Zielwert für den duty cycle des rechten Motors in diesem Status; muss int sein für analogWrite() !!
	byte Motor_L_Speed_Target;				// Zielwert für den duty cycle des linken Motors in diesem Status; muss int sein für analogWrite() !!

    unsigned long duration;					// [ms]; wie lange soll der Status erhalten bleiben; 0, wenn keine Zeitbegrenzung erforderlich
	int curr_limit;							// (0...1024) Stromstärke, bei der der Status beendet werden soll; 0, wenn nicht relevant
											// Hindernis: 	1,3A --> 281
											// Endanschlag:	1,5A --> 324
	unsigned int flash_on;					// [ms]; Dauer, wie lange die Warnlampe in diesem Betriebsmodus eingeschaltet sein soll
	unsigned int flash_off;					// [ms]; Dauer, wie lange die Warnlampe ausgeschaltet sein soll

		// bei den nächsten 4 Werten ist darauf zu achten, dass die erforderliche Zeit bis zum Erreichen der Zielgeschwindigkeit 
		// (=(Vziel - Vaktuell)*Speed_Interval/Speed_Step) kleiner ist, als die geplante Dauer des Zustand (duration)
		// anderenfalls erreichen die Motoren nicht ihre Zielgeschwindigkeit und bewegen sich ...
		// u.U. mit einer falschen Geschwindigkeit durch den nächsten Status (z.B. nach der Anfangs-Beschleunigung beim Öffnen)
		// oder haben einen sprunghaften Übergang bei der Geschwindigkeit
	unsigned int Motor_R_Speed_Interval;	// [ms]; Dauer bis zur nächsten Geschwindigkeitsänderung beim Beschleunigen
	unsigned int Motor_L_Speed_Interval;	// [ms]; Dauer bis zur nächsten Geschwindigkeitsänderung beim Beschleunigen
		// bei den nächsten beiden Werten: 0 bedeutet: kein Beschleunigen oder Bremsen, d.h. die Veränderung der Geschwindigkeit ist deaktiviert
		// In diesem Fall übernehmen die Tore die Geschwindigkeit des vorhergehenden Zustand und bewegen sich damit weiter, falls keine anderen Parameter dies verhindern
	byte Motor_R_Speed_Step;				// Wert, um den sich das PWM-Signal jedesmal verändert; immer positiv
	byte Motor_L_Speed_Step;				// Wert, um den sich das PWM-Signal jedesmal verändert; immer positiv
	
}  state_params;

state_params parameter[7] = {
	// Target	|Target		|Status,|Blockier-	|Flash,	|Flash	|Speed	|Speed	|Speed	|Speed	
	// Speed R,	|Speed L,	|Dauer,	|strom-		| On 	| Off	|IntVal |IntVal |Step   |Step
	// (0.255),	|(0.255),	|[ms] ,	|stärke,	| [ms],	| [ms],	| Re    | Li    | Re    | Li
	{ 	0,		0,	 		0,		0,			0,		0,		100,	100,	0,		0,		 		},		// #0 == CLOSED
	{ 	255,	90, 		16000,	324,		200,	800,	300,	300,	10,		10,			 	},		// #1 == OPENING
	{ 	0,		0,	 		0,		0,			0,		0,		100,	100,	0,		0,			 	},		// #2 == STOPPED
	{ 	255,	255, 		18000,	281,		200,	800,	100,	100,	10,		10,			 	},		// #3 == CLOSING
	{ 	0,		0,	 		0,		0,			400,	400,	100,	100,	0,		0,		 		},		// #4 == BLOCKED
	{ 	0,		0,	 		0,		0,			100,	100,	100,	100,	0,		0,			 	},		// #5 == OVERLOAD
	{ 	0,		0,	 		0,		0,			400,	200,	100,	100,	0,		0			 	}		// #6 == OPENED
};
