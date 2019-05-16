/*
 * Here we define all constants and variables used in this program
 *
 *	V0.96: struktur state_params erweitert um folgende Parameter für's Bremsen und Beschleunigen, getrennt für jeden Motor:
 *			- Zielgeschwindigkeit in duty cycle [0..255; byte]
 *			- Zeitdauer [ms; integer] nach der die Geschwindigkeit jeweils wieder um eine Stufe erhöht/verringert wird
 *			- Wert des duty cycles [byte], um den die Geschwindigkeit jeweils verändert wird (+/-)
 *
 */

/*
 *	Hier finden sich einige grundlegende Parameter und Informationen zur Toranlage
 *	------------------------------------------------------------------------------
 *	Tore fest an den Anschlag drücken, damit sie nicht klappern: dafür ist ein PWM-Wert von 190 erforderlich
 *
 *	Zur Verhinderung eine Blockierung/Verkantung der Tore im geschlossenen Zustand:
 *	Beim Öffnen der Tore muss zuerst das rechte Tor starten, dann kann mit kurzer Verzögerung das linke Tor folgen
 *	Beim vollständigen Schließen muss zuerst das linke Tor am Anschlag ankommen, dann folgt das rechte Tor
 *
 *	Damit nur schnell eine Person das Tor passieren kann, reicht es, das rechte Tor für 5 sec zu öffnen, 
 *	dann kann das linke folgen oder die Tore werden durch zweimaliges Drücken der Taste gestoppt und wieder in die Gegenrichtung gelenkt
 *
 *
 *
 *
 *
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
boolean IsDoorBlocked			= false;	// Tor ist nicht blockiert
boolean IsDoor_R_Blocked		= false;	// Das rechte Tor ist nicht blockiert
boolean IsDoor_L_Blocked		= false;	// Das linke Tor ist nicht blockiert
boolean IsDoorAtEndStop			= false;	// Tor ist nicht am Endanschlag
boolean IsDoor_R_AtEndStop		= false;	// Das rechte Tor ist nicht am Endanschlag
boolean IsDoor_L_AtEndStop		= false;	// Das linke Tor ist nicht am Endanschlag
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
unsigned int	Mot_R_Current = 0;				// Variable für die Stromstärke am Motor Rechts
int speed_pot = 0;								// Wert des Potentiometers, zur Einstellung der Motor-Geschwindigkeit
unsigned int 	Mot_L_Current = 0;				// Variable für die Stromstärke am Motor Links
byte portD_Bitmask = B00111100;					// nur Pins D2-D5 betrachten
byte portC_Bitmask = B00001111;					// nur Pins A0-A3 betrachten
unsigned long	nextTimer_MotorRL_delay = 5000;	// Wert für die Verzögerung, mit der die beiden Tore beim Schließen an den Anschlag fahren

// various variables for flash light control
boolean IsFlashLightOn			= false;	// Flag für den Zustand der Signallampe
boolean IsFlashLightActive		= false;	// Flag für die Nutzung der Signallampe
unsigned int flash_on_duration;				// Einschaltdauer der Signallampe im aktuellen Status [ms]
unsigned int flash_off_duration;			// Ausschaltdauer der Signallampe im aktuellen Status [ms]

// define variables for controlling timer events
unsigned long	nextTimerFlashEvent;		// Variable,die den Zeitpunkt für das nächste Schalten der Signallampe enthält
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
enum state_list {	INITIALIZED, 		// 0
					PHASE1_OPENING,		// 1
					PHASE1_TESTING,		// 2
					PHASE1_DONE,		// 3
					PHASE2_TESTING,		// 4
					PHASE2_DONE,		// 5
					PHASE3_TESTING,		// 6
					PHASE3_DONE,		// 7
					STOPPED				// 8
				};

// create instance and initialize
state_list state = CLOSED;

// define parameters for the different states
typedef struct
 {
	byte Motor_R_Speed_Target;				// Zielwert für den duty cycle des rechten Motors in diesem Status; muss int sein für analogWrite() !!
	byte Motor_L_Speed_Target;				// Zielwert für den duty cycle des linken Motors in diesem Status; muss int sein für analogWrite() !!

    unsigned long duration;					// [ms]; wie lange soll der Status erhalten bleiben; 0, wenn keine Zeitbegrenzung erforderlich
	int curr_limit;							// (0...1024) Stromstärke, bei der der Status beendet werden soll; 
											// sinnvoll beim Öffnen und Schließen;
											// 0 bedeutet: nicht relevant
											// Hindernis: 	1,3A --> 281
											// Endanschlag:	1,5A --> 324
	unsigned int flash_on;					// [ms]; Dauer, wie lange die Warnlampe in diesem Betriebsmodus eingeschaltet sein soll
	unsigned int flash_off;					// [ms]; Dauer, wie lange die Warnlampe ausgeschaltet sein soll
	byte flash_count;						// Anzahl der Sequenzen aus "An & Aus" mit den gegebenen Parameters, die die Warnlampe durchlaufen soll
											// 0 = das Signal wird dauerhaft angezeigt

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
	// Target	|Target		|Status,|Blockier-	|Flash,	|Flash	|Flash	|Speed	|Speed	|Speed	|Speed
	// Speed R,	|Speed L,	|Dauer,	|strom-		| On 	| Off	|Count	|IntVal |IntVal |Step   |Step
	// (0.255),	|(0.255),	|[ms] ,	|stärke,	| [ms],	| [ms],	|	    | Re    | Li    | Re    | Li
	{ 	0,		0,	 		0,		0,			1000,	1000,	0,		0,		0,		0,		0,			},		// #0 == INITIALIZED, 		
	{ 	200,	200, 		0,		0,			200,	800,	0,		100,	100,	10,		10,			},		// #1 == PHASE1_OPENING,		
	{ 	0,		0,	 		0,		0,			500,	500,	0,		0,		0,		0,		0,			},		// #2 == PHASE1_TESTING,		
	{ 	0,		0,	 		0,		0,			1000,	1000,	5,		0,		0,		0,		0,			},		// #3 == PHASE1_DONE,		
	{ 	0,		0,	 		0,		0,			400,	400,	0,		100,	100,	0,		0,			},		// #4 == PHASE2_TESTING,		
	{ 	0,		0,	 		0,		0,			100,	100,	0,		100,	100,	0,		0,			},		// #5 == PHASE2_DONE,		
	{ 	0,		0,	 		0,		0,			400,	200,	0,		100,	100,	0,		0			},		// #6 == PHASE3_TESTING,		
	{ 	0,		0,	 		0,		0,			400,	200,	0,		100,	100,	0,		0			}		// #7 == PHASE3_DONE			
	{ 	0,		0,	 		0,		0,			300,	300,	0,		0,		0,		0,		0			}		// #8 == STOPPED			
};                                                                                                                               


//	ab hier die Variablen zur Steuerung der Testfahrten
//	generell
byte	test_phase = 0;			// gerade aktuell Phase [0..3]; ggf. normale Statussteuerung nutzen ???
unsigned long	testing_next_event_ = 0;	// nächster Timestamp, bei dem bei den Testfahrten etwas passieren soll

/* 
 * 	Phase 1: Ermittlung des minimalen PWM-Wertes, der zur Erkennung des Blockierstroms erforderlich ist
 *	bzw. Aufnahme einer Kennlinie für das Verhältnis von PWM-Wert zum Motorstrom bzw. dem analogen Eingangspegel in blockierten Zustand
 *	Dazu wird:
 *	a) dein Tor zuerst geschlossen (Fahrt mit hohem PWM-Wert, um das Schließen auf jeden erkennen zu können)
 *	b) dann sukzessive der PWM-Wert startend von Null in Stufen erhöht, bis die Elektronik den Blockiertstrom signaliert
 *
 */
byte	PWM_min_blocking = 0;						// PWM-Wert, ab dem der Blockierstrom erkannt werden kann
byte	PWM_min_blocking_start = 0;					// Startwert für den PWM-Wert, mit dem der Blockierstrom ermittelt werden soll
byte	PWM_min_blocking_inc = 10;					// Inkrement für den PWM-Wert, mit dem der Blockierstrom ermittelt werden soll
unsigned long	PWM_min_blocking_duration = 1500;	// Dauer [msec], für die der aktuelle PWM-Wert aufrecht erhalten werden soll, damit der Blockierzustand erkannt werden kann



/* 
 * 	Phase 2: Ermittlung des minimalen PWM-Wertes, mit dem sich ein Tor bewegen lässt
 *	
 *	
 *	
 *	
 *	
 *	
 */
byte	PWM_min_moving = 0;		// PWM, ab dem sich ein Tor bewegt

// Phase 3
unsigned long	Startzeit_Tor = 0;		// Zeitpunkt, an dem das Tor gestartet wurde
unsigned long	Zielzeit_Tor = 0;		// Zeitpunkt, an dem das Tor den anderen Anschlag erreicht hat
unsigned long	Laufzeit_Tor = 0;		// Dauer für das Öffnen/Schließen des Tores

