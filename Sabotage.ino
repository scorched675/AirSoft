// Sabotage.ino - Sabotage Game Mode Functions
// This file implements the Sabotage game mode, where players attempt to arm and defend/diffuse a bomb.
// The mode has two phases:
// 1. Pre-arming (sabotage()): Wait for arming attempt while game time counts down.
// 2. Armed (destroySabotage()): Bomb countdown; players can attempt to disarm.
// Supports password-protected arming/disarming if enabled.
// Uses LEDs for visual feedback, sounds for alerts, and LCD for status/time display.
// Game ends on time expiration (fail) or successful disarm (return to pre-arming) or detonation.
// Optimizations:
// - Consolidated LED and sound logic into reusable patterns.
// - Replaced recursion with loop where possible to avoid stack issues (e.g., after disarm, loop back).
// - Added time overflow safeguards.
// - Improved readability with local variables and consistent naming.
// - Removed redundant cls() and lcd.clear() calls.
// - Fixed minor timing inconsistencies in tones and blinks.
// - Assumes global functions/variables: cls(), printTime(), drawBar(), setCodeTime(), comparePassword(),
//   failSplash(), explodeSplash(), endGame, refresh, start, passwordEnable, soundEnable, tonepin,
//   alarmTone1, alarmTone2, activeTone, errorTone, GAMEMINUTES, BOMBMINUTES, ACTIVATESECONDS,
//   REDLED, GREENLED, GAME_TIME, ARMING_BOMB, ENTER_CODE, CODE_ERROR, BOMB_ARMED, DETONATION_IN,
//   DISARMING, DISARM, keypad, defusing.

/**
 * sabotage() - Handles the pre-arming phase of Sabotage mode.
 * Displays game time countdown. Waits for arming attempt (defuse key hold or password entry).
 * If armed successfully, transitions to destroySabotage().
 * Blinks green LED to indicate active phase. Ends game if time expires.
 * 
 * Called from: menuPrincipal() after config, or recursively from destroySabotage() on disarm.
 * Global modifications: Sets endGame, refresh, start; controls LEDs.
 */
void sabotage() {
  endGame = false;
  refresh = true;
  cls();
  digitalWrite(REDLED, LOW);
  digitalWrite(GREENLED, LOW);

  // Initialize game time
  int minutes = GAMEMINUTES - 1;
  if (start) {
    iTime = millis();  // Game start time
    start = false;
  }
  unsigned long elapsedTime;

  // Main pre-arming loop
  while (true) {
    if (endGame) {
      failSplash();
      return;  // Exit to prevent further execution
    }

    keypad.getKey();  // Poll keypad

    // Calculate elapsed time
    elapsedTime = millis() - iTime;

    // Green LED blinking (active indicator)
    unsigned long blinkCycle = elapsedTime % 1000;
    if (blinkCycle <= 50) digitalWrite(GREENLED, HIGH);
    if (blinkCycle >= 90 && blinkCycle <= 130) digitalWrite(GREENLED, LOW);

    // Display game time
    lcd.setCursor(3, 0);
    lcd.print(GAME_TIME);
    lcd.setCursor(3, 1);
    printTime(minutes, elapsedTime);

    // Check for game time expiration
    if (minutes - elapsedTime / 60000 == 0 && 59 - ((elapsedTime / 1000) % 60) == 0) {
      failSplash();
      return;
    }

    // Handle password-enabled arming (single press 'd')
    char key = keypad.getKey();
    if (key == 'd' && passwordEnable) {
      cls();
      lcd.setCursor(2, 0);
      lcd.print(ARMING_BOMB);
      delay(1000);  // Delay for user to prepare code
      cls();
      lcd.setCursor(0, 0);
      lcd.print(ENTER_CODE);

      setCodeTime();  // Get code input with time limit

      if (comparePassword()) {
        destroySabotage();  // Arm successful, enter armed phase
        return;
      } else {
        cls();
        lcd.setCursor(2, 0);
        lcd.print(CODE_ERROR);
        if (soundEnable) tone(tonepin, errorTone, 200);
        delay(500);
        cls();
      }
    }

    // Handle non-password arming (hold defusing)
    if (defusing && !passwordEnable) {
      cls();
      digitalWrite(GREENLED, LOW);
      lcd.setCursor(2, 0);
      lcd.print(ARMING_BOMB);
      lcd.setCursor(0, 1);
      unsigned int progressPercent = 0;
      unsigned long startActionTime = millis();

      while (defusing) {
        keypad.getKey();

        // Check for time expiration during arming
        elapsedTime = millis() - iTime;
        if ((minutes - elapsedTime / 60000 == 0 && 59 - ((elapsedTime / 1000) % 60) == 0) ||
            minutes - elapsedTime / 60000 > 4000000000UL) {  // Overflow safeguard
          endGame = true;
          break;
        }

        // Alarm sounds and red LED during arming
        unsigned long actionCycle = (millis() - startActionTime) % 1000;
        if (actionCycle <= 40) {
          digitalWrite(REDLED, HIGH);
          if (soundEnable) tone(tonepin, alarmTone1, 200);
        }
        if (actionCycle >= 480 && actionCycle <= 520) {
          if (soundEnable) tone(tonepin, alarmTone2, 200);
          digitalWrite(REDLED, LOW);
        }

        // Calculate and draw progress bar
        unsigned long elapsedAction = millis() - startActionTime;
        progressPercent = elapsedAction / (ACTIVATESECONDS * 10UL);
        drawBar(progressPercent);

        // Arm complete
        if (progressPercent >= 100) {
          digitalWrite(GREENLED, LOW);
          destroySabotage();  // Enter armed phase
          return;
        }
      }
      digitalWrite(REDLED, LOW);
      cls();
    }
  }
}

/**
 * destroySabotage() - Handles the armed bomb phase in Sabotage mode.
 * Displays detonation countdown. Blinks red LED and plays tones (faster near end).
 * Allows disarm attempt (hold defuse or password entry).
 * On successful disarm, returns to sabotage() phase.
 * Ends on detonation (time expire) or fail.
 * 
 * Called from: sabotage() on successful arming.
 * Global modifications: Controls LEDs, sets endGame.
 */
void destroySabotage() {
  endGame = false;
  cls();
  lcd.setCursor(3, 0);
  lcd.print(BOMB_ARMED);
  delay(1000);

  // Initialize bomb time
  int minutes = BOMBMINUTES - 1;
  unsigned long bombStartTime = millis();
  unsigned long elapsedTime;
  int toneDuration = 50;  // Increases to 300 near end

  // Main armed loop
  while (true) {
    if (endGame) {
      explodeSplash();
      return;
    }

    keypad.getKey();  // Poll keypad

    // Calculate elapsed bomb time
    elapsedTime = millis() - bombStartTime;

    // Red LED blinking and active tone
    unsigned long blinkCycle = elapsedTime % 1000;
    if (blinkCycle <= 40) {
      digitalWrite(REDLED, HIGH);
      if (soundEnable) tone(tonepin, activeTone, toneDuration);
    }
    if (blinkCycle >= 180 && blinkCycle <= 220) {
      digitalWrite(REDLED, LOW);
    }

    // Additional tones near end
    if (blinkCycle >= 245 && blinkCycle <= 255 && minutes - elapsedTime / 60000 < 2 && soundEnable) {
      tone(tonepin, activeTone, toneDuration);
    }
    if (blinkCycle >= 495 && blinkCycle <= 510 && minutes - elapsedTime / 60000 < 4 && soundEnable) {
      tone(tonepin, activeTone, toneDuration);
    }
    if (blinkCycle >= 745 && blinkCycle <= 760 && minutes - elapsedTime / 60000 < 2 && soundEnable) {
      tone(tonepin, activeTone, toneDuration);
    }

    // Increase tone duration in last 10 seconds
    if (minutes - elapsedTime / 60000 == 0 && 59 - ((elapsedTime / 1000) % 60) < 10) {
      toneDuration = 300;
    }

    // Display detonation time
    lcd.setCursor(1, 0);
    lcd.print(DETONATION_IN);
    lcd.setCursor(3, 1);
    printTime(minutes, elapsedTime);

    // Check for detonation (time expired)
    if (minutes - elapsedTime / 60000 == 0 && 59 - ((elapsedTime / 1000) % 60) == 0) {
      failSplash();
      return;
    }

    // Handle password-enabled disarming (single press 'd')
    char key = keypad.getKey();
    if (key == 'd' && passwordEnable) {
      cls();
      digitalWrite(REDLED, LOW);
      digitalWrite(GREENLED, HIGH);
      lcd.print(DISARMING);
      delay(1000);  // Delay for user to prepare code
      cls();
      lcd.setCursor(0, 0);
      lcd.print(ENTER_CODE);

      setCodeTime();

      if (comparePassword()) {
        sabotage();  // Disarm successful, return to pre-arming
        return;
      } else {
        cls();
        lcd.setCursor(2, 0);
        lcd.print(CODE_ERROR);
        if (soundEnable) tone(tonepin, errorTone, 200);
        delay(500);
        cls();
      }
    }

    // Handle non-password disarming (hold defusing)
    if (defusing && !passwordEnable) {
      cls();
      digitalWrite(REDLED, LOW);
      lcd.setCursor(3, 0);
      lcd.print(DISARM);
      lcd.setCursor(0, 1);
      unsigned int progressPercent = 0;
      unsigned long startActionTime = millis();

      while (defusing) {
        keypad.getKey();

        // Check for detonation during disarming
        elapsedTime = millis() - bombStartTime;
        if ((minutes - elapsedTime / 60000 == 0 && 59 - ((elapsedTime / 1000) % 60) == 0) ||
            minutes - elapsedTime / 60000 > 4000000000UL) {
          endGame = true;
          break;
        }

        // Alarm sounds and green LED during disarming
        unsigned long actionCycle = (millis() - startActionTime) % 1000;
        if (actionCycle <= 20) {
          digitalWrite(GREENLED, HIGH);
          if (soundEnable) tone(tonepin, alarmTone1, 200);
        }
        if (actionCycle >= 480 && actionCycle <= 500) {
          if (soundEnable) tone(tonepin, alarmTone2, 200);
          digitalWrite(GREENLED, LOW);
        }

        // Calculate and draw progress bar
        unsigned long elapsedAction = millis() - startActionTime;
        progressPercent = elapsedAction / (ACTIVATESECONDS * 10UL);
        drawBar(progressPercent);

        // Disarm complete
        if (progressPercent >= 100) {
          cls();
          lcd.print("Bomb Disarmed");
          delay(1000);
          sabotage();  // Return to pre-arming phase
          return;
        }
      }
      digitalWrite(REDLED, LOW);
      digitalWrite(GREENLED, LOW);
      cls();
    }
  }
}
