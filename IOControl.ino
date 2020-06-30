/*
 * This code covers all input and output functions (except motor control)
 * This includes:
 * - PCB push button
 * - remote control button
 * - 2 jumpers configuring the delays during opening and closing the doors
 * - flashing the signal light (normal operation & error condition)
 * - read the potentiometer for test purposes
 * - read jumpers which configure the delays of opening and closing doors
 *
 *	Version 0.97 (speziell angepasst für die geplanten Testfahrten)
 *
 */

// #include "Torsteuerung.h"

void initialize_FSM() {
  IsDoorOpening     = true;   		// Tor wird geöffnet; false --> schließen
  IsCurrentOverloaded   = false;  	// Hardware-Strombegrenzung hat nicht angesprochen
  IsDoorBlocked     = false;  		// Tür ist nicht blockiert
  IsButtonNeedsProcessing = false; 	// keine Taste wurde betätigt, daher keine Aktion notwendig
  IsButtonReleased    = true;   	// es ist gerade keine Taste gedrückt
  IsMotorSpeedUpdated   = false;	// keine neue Motorgeschwindigkeit eingestellt
  
  IsDoor_R_AtEndStop = false;
  IsDoor_L_AtEndStop = false;
  
  IsDoor_R_Blocked = false;
  IsDoor_L_Blocked = false;

  state = IDLE;						// Warten auf Eingabe: Tastendruck im Normalbetrieb, Testauswahl im Testbetrieb
  isCalledBy = 0;					// erstmal kein sub-state
  nextTimer_GatesDelay_Event = 0;	// kein Delay Event aktiv	
}


void initialize_IO()	{
	pinMode(Start_Funk,   INPUT);
	pinMode(Start_Taste,  INPUT_PULLUP);
	pinMode(Jumper1,      INPUT_PULLUP);
	pinMode(Jumper2,      INPUT_PULLUP);

	// inputs to monitor the hardware based current limiter
	pinMode(Fb_H_Br_R_A,  INPUT);
	pinMode(Fb_H_Br_R_Z,  INPUT);
	pinMode(Fb_H_Br_L_A,  INPUT);
	pinMode(Fb_H_Br_L_Z,  INPUT);

	// outputs to control motor direction
	pinMode(H_Br_R_A,     OUTPUT);
	pinMode(H_Br_R_Z,     OUTPUT);
	pinMode(H_Br_L_A,     OUTPUT);
	pinMode(H_Br_L_Z,     OUTPUT);

	pinMode(Rst_I_Stopp,  OUTPUT);		// reset current limiter
	pinMode(H_Br_R_En,    OUTPUT);		// PWM output for right motor
	pinMode(H_Br_L_En,    OUTPUT);		// PWM output for left motor
	pinMode(Warnleuchte,  OUTPUT);		// output for flash light

	// Initialize outputs
	digitalWrite(H_Br_R_En, LOW);		// stop right motor
	digitalWrite(H_Br_L_En, LOW);		// stop left motor
	digitalWrite(Rst_I_Stopp, HIGH);	// disable power limiter - ATTENTION: ACTIVE-LOW

}

/*
	this procedure reads the state of the two buttons and controls
	the variables which are responsible for indicating a pressed
	or released button to the FSM
	Neu mit V0.96: lange und kurze Tastendrücke unterscheiden
*/

void get_button_state()	{
  int val_push_button = 0;
  int val_RC_button = 0;
  boolean isOneButtonPressed = false;	// Flag, dass eine oder beide Tasten gerade gedrückt sind

  // check state of both buttons
  val_push_button = digitalRead(Start_Taste);  // read the PCB button input (active low)
  val_RC_button = digitalRead(Start_Funk);     // read the remote control input (active high)
  isOneButtonPressed = ((!digitalRead(Start_Taste)) | digitalRead(Start_Funk));		// Auswertung, ob eine der beiden Tasten gedrückt ist

  // Wenn eine der beiden Tasten NEU (IsButtonReleased == true) gedrückt wurde
  // if ((val_push_button == LOW || val_RC_button == HIGH) && IsButtonReleased == true && IsButtonNeedsProcessing == false){
  if (isOneButtonPressed && IsButtonReleased == true && IsButtonNeedsProcessing == false){
	tsButtonWasPressed = millis();		// Zeit merken, wann die Taste gedrückt wurde
	IsButtonReleased = false;			// vermerken, dass die Taste jetzt als gedrückt registriert ist
	// Klassifizierung des letzten Tastendrucks zur Vorbereitung auf den Neuen löschen
	BottonWasPressedShort = false;
	BottonWasPressedLong = false;
  }
  // Wenn am Ende eines Tastendrucks (IsButtonReleased == false) keine der beide Tasten mehr gedrückt ist ...
  // ... prüfen, ob es ein kurzer oder langer Tastendruck war und eine Aktion auslösen lassen
  else if (!isOneButtonPressed && IsButtonReleased == false) {		// Tastendruck ist zu Ende
	if ((millis() - tsButtonWasPressed) >= ButtonLongPressDuration) {		// war die Taste lange gedrückt ??
		BottonWasPressedLong = true;	// ja, es war ein langer Tastendruck
	}
	else {
		BottonWasPressedShort = true;	// nein, es war ein kurzer Tastendruck
	}
	IsButtonReleased = true;		// Ende des aktiven Tastendruck merken
	IsButtonNeedsProcessing = true;	// vermerken, dass nun eine Aktion zu erfolgen hat
  }
  // otherwise do nothing
}

/*
	Potentiometer lesen, um die Geschwindigkeit der Motoren zu steuern
	Der gelesene Wert (0-1023) muss durch 4 geteilt werden
*/
byte get_motor_speed() {
	unsigned int val = 0;
	unsigned int valpot = 0;
	valpot = analogRead(MotorSpeedPin);	// Wert lesen, durch 4 teilen und int-Wert zurückgeben
	val = (valpot>>2);	// Wert lesen, durch 4 teilen und int-Wert zurückgeben
	if (val != V_Motoren) {			// prüfen, ob sich der Wert verändert hat
		IsMotorSpeedUpdated = true;	// wenn ja, das entsprechende Flag setzen
	}
	return (val);

}

/*
	Status der Jumper auslesen und die entsprechenden Flags setzen
*/
byte get_jumper_status() {
	if (digitalRead(Jumper1) == LOW) {		// prüfen, ob der Jumper gesteckt ist
		IsJumper1Active = true;				// wenn ja, das entsprechende Flag setzen
	}
	else IsJumper1Active = false;			// ... ansonsten löschen

	if (digitalRead(Jumper2) == LOW) {		// prüfen, ob der Jumper gesteckt ist
		IsJumper2Active = true;				// wenn ja, das entsprechende Flag setzen
	}
	else IsJumper2Active = false;			// ... ansonsten löschen
	return(0);
}

// kontrolliere die Überstromschaltung
// dazu werden Port D (D0-D7) und Port C (A0-A7) gelesen und 4 bits miteinander verglichen
void check_is_motor_overloaded() {
	byte pd = 0;	// var für Port D
	byte pc = 0;	// var für Port C
/*
	Serial.print (F(";\t portD:"));
	Serial.print (PORTD, BIN);
	Serial.print (F(";\t DDRC:"));
	Serial.print (DDRC, BIN);
	Serial.print (F(";\t portC:"));
	Serial.print (PINC, BIN);
*/
	pd = (PORTD & portD_Bitmask)>>2;	// Port D auslesen, maskieren und
	pc = (PINC & portC_Bitmask); 	// Port C auslesen und maskieren
/*
	Serial.print (F(";\t pd:"));
	Serial.print (pd, BIN);
	Serial.print (F(";\t pc:"));
	Serial.print (pc, BIN);
    Serial.println (F(""));
*/
	if (pd != pc) {					// in case both are not the same ...
		IsCurrentOverloaded = true;	// ... set overload flag
	}
}

// kontrolliere die Überstromerkennung für den rechten Motor 
// dazu werden Port D (D0-D7) und Port C (A0-A7) gelesen und 2 bits miteinander verglichen
void check_is_motor_R_overloaded() {
	byte pd = 0;	// var für Port D
	byte pc = 0;	// var für Port C
/*
	Serial.print (F(";\t portD:"));
	Serial.print (PORTD, BIN);
	Serial.print (F(";\t DDRC:"));
	Serial.print (DDRC, BIN);
	Serial.print (F(";\t portC:"));
	Serial.print (PINC, BIN);
*/
	pd = (PORTD & portD_Bitmask_R)>>2;	// Port D auslesen, für den rechten Motor maskieren und
	pc = (PINC & portC_Bitmask_R); 		// Port C auslesen und ebenfalls maskieren
/*
	Serial.print (F(";\t pd:"));
	Serial.print (pd, BIN);
	Serial.print (F(";\t pc:"));
	Serial.print (pc, BIN);
    Serial.println (F(""));
*/
	if (pd != pc) {					// in case both are not the same ...
		IsCurrent_R_Overloaded = true;	// ... set overload flag
	}
}
// kontrolliere die Überstromerkennung für den linken Motor 
// dazu werden Port D (D0-D7) und Port C (A0-A7) gelesen und 2 bits miteinander verglichen
void check_is_motor_L_overloaded() {
	byte pd = 0;	// var für Port D
	byte pc = 0;	// var für Port C
/*
	Serial.print (F(";\t portD:"));
	Serial.print (PORTD, BIN);
	Serial.print (F(";\t DDRC:"));
	Serial.print (DDRC, BIN);
	Serial.print (F(";\t portC:"));
	Serial.print (PINC, BIN);
*/
	pd = (PORTD & portD_Bitmask_L)>>2;	// Port D (PORTD verwenden, um auch die Ausgänge lesen zu können) auslesen, für den rechten Motor maskieren und
	pc = (PINC & portC_Bitmask_L); 		// Port C (read only input) auslesen und ebenfalls maskieren
/*
	Serial.print (F(";\t pd:"));
	Serial.print (pd, BIN);
	Serial.print (F(";\t pc:"));
	Serial.print (pc, BIN);
    Serial.println (F(""));
*/
	if (pd != pc) {					// in case both are not the same ...
		IsCurrent_L_Overloaded = true;	// ... set overload flag
	}
}
// kontrolliere die Stromstärke der Motoren anhand der Parameter-Tabelle (Array)
// und setze das entsprechende Flag
void check_is_motor_blocked() {
	// Stromstärke auslesen und gegen den Grenzwert vergleichen
	Mot_R_Current = analogRead(Strom_R);
	Mot_L_Current = analogRead(Strom_L);
	Mot_R_Current_Limit = parameter[state].curr_limit;
	Mot_L_Current_Limit = parameter[state].curr_limit;
	if (Mot_R_Current > Mot_R_Current_Limit){
		IsDoor_R_Blocked = true;
	}
	if (Mot_L_Current > Mot_L_Current_Limit){
		IsDoor_L_Blocked = true;
	}
}


/*
 *	handle flashing the lamp
 *	there are 2 modes of flashing:
 *	- normal operation of doors
 *	- alarm flashing in case of power overrun
 */

// Signallampe ansteuern
void updateFlashLight(boolean light_on) {
		if (light_on) {
			digitalWrite(Warnleuchte, HIGH);
		}
		else {
			digitalWrite(Warnleuchte, LOW);
		}
}

// Signallampe umschalten
void toggleFlashLight(boolean light_on) {
		if (light_on) {
			digitalWrite(Warnleuchte, LOW);
			IsFlashLightOn = false;
			nextTimerFlashEvent = nextTimerFlashEvent + long(flash_off_duration);
		}
		else {
			digitalWrite(Warnleuchte, HIGH);
			IsFlashLightOn = true;
			nextTimerFlashEvent = nextTimerFlashEvent + long(flash_on_duration);
		}
}

void initializeFlashLightNewState(char new_state) {
	// get flash light parameters for new state
	flash_on_duration 	= parameter[state].flash_on;
	flash_off_duration 	= parameter[state].flash_off;

	// check if flash light is needed
	if ((flash_on_duration == 0) || (flash_off_duration == 0)) {
		IsFlashLightActive = false;		// Signallampe deaktivieren
		IsFlashLightOn = false;			// Signallampe ausschalten
		updateFlashLight(IsFlashLightOn);
	}
	else {
		IsFlashLightActive = true;		// Signallampe aktivieren
		IsFlashLightOn = true;			// Signallampe einschalten
		updateFlashLight(IsFlashLightOn);
		// nächsten Schaltzeitpunkt bestimmen
		nextTimerFlashEvent = timestamp + long(flash_on_duration);
	}
}


/*
 * read serial port and wait for user to enter a single digit
 * only the first character is used, additional bytes are dropped
 * loop until user enters digits valid for calling routine
  */
byte getSerialDigit(byte min, byte max) {
	char recChar = 0;		
	char trash = 0;
	byte recDigit = 0;
	do {
		while (Serial.available() == 0) {				// wait for something to receive
		}
		recChar = Serial.read();						// read character entered first
		while (Serial.available()) {					// drop remaining buffer
			trash = Serial.read();
		}
		recDigit = recChar - 48;						// convert single byte ASCII to unsigned byte
	} while ((recDigit < min) || (recDigit > max));		// loop until user entered one of the allowed digits
	return (recDigit);
	// add here some code for error handling like "wait until a valid digit was entered or 0 to abort"
}

// Auswahlmenu mit der Liste der Testprogramme
byte getTestSelection() {
	byte sel;
	
	Serial.println(F("****************************************************************************************"));
	Serial.println(F("Testprogramm wählen:"));
	Serial.println();
	Serial.println(F("1 - Ermittlung der Blockierströme für alle PWM-Wertes (am Anschlag)"));
	Serial.println(F("2 - Ermittlung des minimalen PWM-Wertes, mit dem sich die Tore bewegen lassen"));
	Serial.println(F("3 - Messung der Zeiten für Öffnen und Schließen beider Tore bei unterschiedlichen PWM-Werten"));
	Serial.println(F("4 - Ermittlung max. PWM-Wertes für einen Motorstart aus dem Stand, ohne dass die Überlasterkennung anspricht"));
	Serial.println(F("5 - Öffnen beider Tore (rechtes Tor zuerst)"));
	Serial.println(F("6 - Schließen beider Tore (linkes Tor zuerst)"));
	Serial.println();
	Serial.print(F("Nummer des Programms eingeben [1.."));
	Serial.print(numTestProgramms);
	Serial.print(F("]: "));
	do {
		sel = getSerialDigit(1,6);
	} while ((sel == 0) || (sel > numTestProgramms));
	Serial.println();
	Serial.print(F("Testprogram <"));
	Serial.print(sel);
	Serial.println(F("> wird gestartet ..."));
	Serial.println(F("****************************************************************************************"));
	return (sel);
} 


// Auswahl, ob Debugmeldungen angezeigt werden sollen oder nicht
byte getDebugLevel() {
	byte sel;
	
	Serial.println(F("****************************************************************************************"));
	Serial.println(F("Debug-Level festlegen:"));
	Serial.println();
	Serial.println(F("0 - Debug-Mitteilungen ausschalten"));
	Serial.println(F("1 - Debug-Mitteilungen einschalten"));
	Serial.println();
	Serial.print(F("Debug-Level auswählen [0/1]: "));
	sel = getSerialDigit(0,1);
	Serial.println();
	Serial.print(F("Debug-Level <"));
	Serial.print(sel);
	Serial.println(F("> ausgewählt ..."));
	Serial.println(F("****************************************************************************************"));
	return (sel);
} 