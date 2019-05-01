/*
 * This file contains all subroutines related to DC motor control
 */

/*
 ************************************************
 *	Historie
 *
 *	V0.96
 *	- Auslagerung der Funktionen für die Steuerung (FSM) in eine eigene Datei ("Steuerung")
 *	- updateMotorSpeed() wird abgelöst durch eine separate Routine für jeden Motor
 *	- die Motoren können jetzt beschleunigt und langsam abgebremst werden (IsMotor_x_Ramping = true/false; IsMotor_x_SpeedingUp = true/false)
 *	- die Routinen Start_Motor_x initalisieren die H-Brücken für die Richtung, konfigurieren das PMW-Signal (steigend/fallend/konstant/aus) und die
 *		entsprechenden Flags und starten die Motoren
 *	- die Routinen Update_PWM_Motor_x sorgen bei einem entsprechend Zeitevent dafür, dass das PWM-Signal entsprechend der
 *		Beschleunigungs-/Bremskurve (=Ramping) gesteuert wird und bei Erreichen des Zielgeschwindigkeit das Ramping beendet
 *
 *
 *
 *	Neu in Version 0.95
 *	- Erkennung von langen Tastendrücken (Issue #17)
 *
 *
 ************************************************
 */

// #include "Torsteuerung.h"
#include <Arduino.h>

void reset_power_limiter();
void fastStopMotor_R();
void fastStopMotor_L();
void sendSyncImpuls();
void startMotor_R(int pmw_val, boolean opening);
void startMotor_L(int pmw_val, boolean opening);
void updateMotorSpeed(byte pmw_val);
void Update_PMW_Motor_R();
void Update_PMW_Motor_L();


/*
	this procedure generates a short impuls to set the flipflop  Q outputs of L6506
	after a too high current was detected
*/
void reset_power_limiter() {
	digitalWrite(Rst_I_Stopp, HIGH);	// set flipflop outputs
	digitalWrite(Rst_I_Stopp, LOW);		// deactivate flipflop set line
}
void fastStopMotor_R() {
	digitalWrite(H_Br_R_En, LOW);	// stop the PWM signal
	// short cut the motor windings for fast stop
	digitalWrite(H_Br_R_A, LOW);
	digitalWrite(H_Br_R_Z, LOW);
	IsMotor_R_Ramping = false;		// keine Geschwindigkeitsveränderung
	PWM_Motor_R = 0;				// Stillstand vermerken
}
void fastStopMotor_L() {
	digitalWrite(H_Br_L_En, LOW);	// stop the PWM signal
	// short cut the motor windings for fast stop
	digitalWrite(H_Br_L_A, LOW);
	digitalWrite(H_Br_L_Z, LOW);
	IsMotor_L_Ramping = false;		// keine Geschwindigkeitsveränderung
	PWM_Motor_L = 0;				// Stillstand vermerken
}

/*
 *	Steuerung der Motoren
 */

// SYNC-Impuls an den L6506-Baustein senden
// ACHTUNG: das Signal muss activ-low sein (d.h. normal=high)
void sendSyncImpuls() {
	// reset FlipFlop (SYNC) in L6506 with short impuls (active-low)
	digitalWrite(Rst_I_Stopp, LOW);
	digitalWrite(Rst_I_Stopp, HIGH);
}

/*
*	Diese Routine wird beim Übergang in einen (neuen) Status aufgerufen, der einen Motorstart erfordert:
*	- OPENING (nach den Zuständen CLOSED, BLOCKED oder STOPPED)
*	- CLOSING (nach den Zuständen OPENED, BLOCKED oder STOPPED)
*	Aus der aktuellen Motorgeschwindigkeit und der neuen Zielgeschwindigkeit wird ermittelt, ob der Motor
*	beschleunigt oder abgebremst werden muss;
* 	Zudem werden die H-Brücken auf die korrekte Richtung der Motoren eingestellt
*	Dann wird der Motor mit dem aktuellen PWM-Wert gestartet ...
*	und im Fall einer Beschleunigung/Bremsung (ramping) wird der Zeitpunkt für den nächsten PWM-Update-Event gesetzt
*/
void startMotor_R(int pmw_val, boolean opening) {
	digitalWrite(H_Br_R_En, LOW);										// stop the PWM signal to prevent confusion
	sendSyncImpuls();													// sende SYNC-Impuls
	IsMotor_R_Ramping = false;
	if (parameter[state].Motor_R_Speed_Step)	{						// prüfen, ob es für diesen Status eine Geschwindigkeitsveränderung vorgesehen ist
		IsMotor_R_Ramping = true;										// --> ja, es soll beschleunigt oder gebremst werden
		if (PWM_Motor_R >= parameter[state].Motor_R_Speed_Target) { 	// nun ermitteln, ob beschleunigt oder gebremst werden muss
			IsMotor_R_SpeedingUp = false;								// --> Bremsen
		}
		else {
			IsMotor_R_SpeedingUp = true;								// --> Beschleunigen
		}
		// ... und den nächsten Timerevent für den Motor festlegen
		nextTimer_Motor_R_Event = timestamp + parameter[state].Motor_R_Speed_Interval;
	}
	// jetzt die Drehrichtung des Motors einstellen
	if(opening) {			// rechtes Tor öffnen
		digitalWrite(H_Br_R_Z, LOW);		// Motor_R (-) auf Masse legen
		digitalWrite(H_Br_R_A, HIGH);		// Motor_R (+) auf Vcc legen
	}
	else {					// rechtes Tor schließen
		digitalWrite(H_Br_R_A, LOW);		// Motor_R (+) auf Masse legen
		digitalWrite(H_Br_R_Z, HIGH);		// Motor_R (-) auf Vcc legen
	}
	// Motor jetzt mit der derzeit aktuellen Geschwindigkeit (PWM-Signal) starten
	analogWrite(H_Br_R_En, (int)PWM_Motor_R);
}




void startMotor_L(int pmw_val, boolean opening) {
	digitalWrite(H_Br_L_En, LOW);	// stop the PWM signal to prevent confusion
	sendSyncImpuls();				// sende SYNC-Impuls
	IsMotor_L_Ramping = false;
	if (parameter[state].Motor_L_Speed_Step)	{						// prüfen, ob es für diesen Status eine Geschwindigkeitsveränderung vorgesehen ist
		IsMotor_L_Ramping = true;										// --> ja, es soll beschleunigt oder gebremst werden
		if (PWM_Motor_L >= parameter[state].Motor_L_Speed_Target) { 	// nun ermitteln, ob beschleunigt oder gebremst werden muss
			IsMotor_L_SpeedingUp = false;								// --> Bremsen
		}
		else {
			IsMotor_L_SpeedingUp = true;								// --> Beschleunigen
		}
		// ... und den nächsten Timerevent für den Motor festlegen
		nextTimer_Motor_L_Event = timestamp + parameter[state].Motor_L_Speed_Interval;
	}
	// jetzt die Drehrichtung des Motors einstellen
	if(opening) {			// Linkes Tor öffnen
		digitalWrite(H_Br_L_Z, LOW);		// Motor_L (-) auf Masse legen
		digitalWrite(H_Br_L_A, HIGH);		// Motor_L (+) auf Vcc legen
	}
	else {					// linkes Tor schließen
		digitalWrite(H_Br_L_A, LOW);		// Motor_L (+) auf Masse legen
		digitalWrite(H_Br_L_Z, HIGH);		// Motor_L (-) auf Vcc legen
	}
	// Motor jetzt mit der derzeit aktuellen Geschwindigkeit (PWM-Signal) starten
	analogWrite(H_Br_L_En, (int)PWM_Motor_L);
}

void updateMotorSpeed(byte pmw_val) {
	// keine Änderung bzgl. Drehrichtung etc.
	// beide Motoren erhalten die gleiche Geschwindigkeit
	analogWrite(H_Br_R_En, pmw_val);	// PWM-Signal aktivieren bzw. aktualisieren
	analogWrite(H_Br_L_En, pmw_val);	// PWM-Signal aktivieren bzw. aktualisieren
}

/*
*	Der PWM-Wert für den rechten Motor muss angepasst werden
*	Diese Routine wird aufgerufen, wenn ein Zeitinterval für den Motor abgelaufen ist
*/
void Update_PMW_Motor_R() {
	byte target_speed = parameter[state].Motor_R_Speed_Target;	// Zielgeschwindigkeit ermitteln
	// ermitteln, ob die Geschwindigkeit erhöht oder reduziert werden muss
	if (IsMotor_R_SpeedingUp) {
		// PWM-Wert nur so stark erhöhen, dass der Zielwert nicht überschritten wird; besonders wichtig bei Zielwert = 255, um Überlauf zu vermeiden
		PWM_Motor_R = PWM_Motor_R + min((target_speed - PWM_Motor_R),parameter[state].Motor_R_Speed_Step);
	}
	else {
		// PWM-Wert nur so stark reduzieren, dass der Zielwert nicht unterschritten wird; besonders wichtig bei Zielwert = 0, um Überlauf zu vermeiden
		PWM_Motor_R = PWM_Motor_R - min((PWM_Motor_R - target_speed),parameter[state].Motor_R_Speed_Step);

	}
	analogWrite(H_Br_R_En, (int)PWM_Motor_R);	// PWM-Signal aktualisieren
	// prüfen, ob der rechte Motor seine Zielgeschwindigkeit erreicht hat;
	if (PWM_Motor_R == target_speed) {				// falls ja, dann das Flag für Beschleunigen/Bremsen löschen
		IsMotor_R_Ramping = false;
	}
	else {											// falls nicht, dann den Zeitpunkt für die nächste Geschwindigkeitsanpassung ermitteln
		nextTimer_Motor_R_Event = nextTimer_Motor_R_Event + parameter[state].Motor_R_Speed_Interval;
	}
}

// Der PWM-Wert für den linken Motor muss angepasst werden
void Update_PMW_Motor_L() {
	byte target_speed = parameter[state].Motor_L_Speed_Target;	// Zielgeschwindigkeit ermitteln
	// ermitteln, ob die Geschwindigkeit erhöht oder reduziert werden muss
	if (IsMotor_L_SpeedingUp) {
		// PWM-Wert nur so stark erhöhen, dass der Zielwert nicht überschritten wird; besonders wichtig bei Zielwert = 255, um Überlauf zu vermeiden
		PWM_Motor_L = PWM_Motor_L + min((target_speed - PWM_Motor_L),parameter[state].Motor_L_Speed_Step);
	}
	else {
		// PWM-Wert nur so stark reduzieren, dass der Zielwert nicht unterschritten wird; besonders wichtig bei Zielwert = 0, um Überlauf zu vermeiden
		PWM_Motor_L = PWM_Motor_L - min((PWM_Motor_L - target_speed),parameter[state].Motor_L_Speed_Step);

	}
	analogWrite(H_Br_L_En, (int)PWM_Motor_L);	// PWM-Signal aktualisieren
	// prüfen, ob der rechte Motor seine Zielgeschwindigkeit erreicht hat;
	if (PWM_Motor_L == target_speed) {				// falls ja, dann das Flag für Beschleunigen/Bremsen löschen
		IsMotor_L_Ramping = false;
	}
	else {											// falls nicht, dann den Zeitpunkt für die nächste Geschwindigkeitsanpassung ermitteln
		nextTimer_Motor_L_Event = nextTimer_Motor_L_Event + parameter[state].Motor_L_Speed_Interval;
	}
}
