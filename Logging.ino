/*
 * This code covers all data logging to serial output
 * 
 * V0.98
 *	- NEW: initial version 
 * 	-
 * 	-
 * 	-
 * 	-
 *
 */

/*
 *	next lines are related to logging PWM, current (analog input)
 */
void log_PWM_CURRENT() {
	Serial.print (timestamp);
	Serial.print (F(":\t PWM(L): "));
	Serial.print (PWM_Motor_L);
	Serial.print (F(";\t aktuelle Stromstärke: "));
	Serial.print (analogRead(Strom_L));
	Serial.print (F(";\t Stromstärke (vorheriger Wert): "));
	Serial.print (Mot_L_Current);
    Serial.println (F(""));
}

void log_PWM_CURRENT_R() {
	Serial.print (timestamp);
	Serial.print (F(":\t PWM(R): "));
	Serial.print (PWM_Motor_R);
	Serial.print (F(";\t aktuelle Stromstärke: "));
	Serial.print (analogRead(Strom_R));
	Serial.print (F(";\t Stromstärke (vorheriger Wert): "));
	Serial.print (Mot_R_Current);
    Serial.println (F(""));
}

void log_PWM_RUNTIME() {
	Serial.print (timestamp - Startzeit_Tor);
	Serial.print (F(" [ms]:\t PWM(L): "));
	Serial.print (PWM_Motor_L);
	Serial.print (F(";\t aktuelle Stromstärke: "));
	Serial.print (analogRead(Strom_L));
	Serial.print (F(";\t IsDoor_L_Blocked: "));
	Serial.print (IsDoor_L_Blocked);
	Serial.print (F(";\t Runtime_Direction_Opening: "));
	Serial.print (Runtime_Direction_Opening);
    Serial.println (F(""));
}
void log_Phase3_Anschlag() {
	Serial.print (F("Phase 3: Tor am Anschlag angekommen !!"));
	Serial.print (F("Phase 3: Bei einem PWM-Wert von "));
	Serial.print (PWM_Motor_L);
	Serial.print (F(" beträgt die Laufzeit: "));
	Serial.print (Zielzeit_Tor - Startzeit_Tor);
	Serial.print (F(" [ms]"));
    Serial.println (F(""));
	Serial.println(F("****************************************************************************************"));
}


void log_Debugging() {

	Serial.print (millis());
    Serial.print (F(";\t st:"));
	Serial.print (state);

    Serial.print (F(";\t PWM-R:"));
	Serial.print (PWM_Motor_R);
    Serial.print (F(";\t PWM-L:"));
	Serial.print (PWM_Motor_L);

    Serial.print (F(";\t I-R:"));
	Serial.print (Mot_R_Current);
    Serial.print (F(";\t I-L:"));
	Serial.print (Mot_L_Current);

    Serial.print (F(";\t I-R-Lim:"));
	Serial.print (Mot_R_Current_Limit);
    Serial.print (F(";\t I-L-Lim:"));
	Serial.print (Mot_L_Current_Limit);

    Serial.print (F(";\t PWM-R-Ziel:"));
	Serial.print (PWM_Motor_R_Target);
    Serial.print (F(";\t PWM-L-Ziel:"));
	Serial.print (PWM_Motor_L_Target);
	
    Serial.print (F(";\t R-Blk:"));
	Serial.print (IsDoor_R_Blocked);
    Serial.print (F(";\t L-Blk:"));
	Serial.print (IsDoor_L_Blocked);

/*
    Serial.print (F(";\t F-on:"));
	Serial.print (flash_on_duration);
    Serial.print (F(";\t F-off:"));
	Serial.print (flash_off_duration);
    Serial.print (F(";\t t_next:"));
	Serial.print (nextTimerFlashEvent);

    Serial.print (F(";\t IsFlashLightActive:"));
	Serial.print (IsFlashLightActive);
    Serial.print (F(";\t IsFlashLightOn:"));
	Serial.print (IsFlashLightOn);

	debugFlags();

 */
    Serial.println (F(""));
}

void debugFlags() {
	unsigned int flagmap = 0;
	flagmap = ((IsDoorOpening) | (IsCurrentOverloaded << 1) | (IsDoorBlocked << 2) | (IsDoorAtEndStop << 3) | (IsButtonNeedsProcessing << 4) | (IsButtonReleased << 5) | (IsMotorSpeedUpdated << 6) | (IsJumper1Active << 7) | (IsJumper2Active << 8)  );
	flagmap = flagmap + (1 << 9);	// eine führende '1' ergänzen, damit die binäre Ausgabe immer die gleiche Länge hat
	Serial.print (F(";\t flags:"));
	Serial.print (flagmap, BIN);
	Serial.print (F(" (Jum2-Jum1-SpdUpd-ButtRel-ButtPro-EndStp-Blk-Ovl-Opng);"));
}

void logMessage() {
	Serial.print (millis());	// add timestamp
    Serial.print (F(":\t"));
	Serial.println (message);
}

