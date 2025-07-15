/**
 * domination() - Handles the Domination game mode.
 * This function manages the logic for a domination-style game where teams (green and red) compete
 * to control a zone. The zone can be neutral, captured by a team, or neutralized. It tracks control
 * times for each team, handles capturing/neutralizing progress with progress bars and sounds,
 * and ends the game when time runs out. LEDs blink to indicate current team control, and sounds
 * play for alerts. The game loops until time expires or is manually ended.
 * 
 * Called from: menuPrincipal() (assumed in the main menu logic from prior code).
 * Global dependencies: Uses global variables like GAMEMINUTES, team, iZoneTime, redTime, greenTime,
 * endGame, soundEnable, tonepin, activeTone, ACTIVATESECONDS, defusing, cancelando, etc.
 * Assumes functions like cls(), printTime(), printTimeDom(), drawBar(), gameOver() are defined elsewhere.
 */
void domination() {
  // Initialize game variables
  int minutes = GAMEMINUTES - 1;      // Game duration in minutes (adjusted for 0-based countdown)
  boolean showGameTime = true;        // Flag to toggle between displaying game time and zone time
  unsigned long currentTime;          // Current millis() for timing calculations
  unsigned long initialTime = millis(); // Start time of the game
  unsigned long elapsedGameTime;      // Elapsed game time in millis

  // Reset team and zone times
  team = 0;                           // Start with neutral team
  iZoneTime = 0;                      // Initial zone control time
  elapsedGameTime = 0;
  redTime = 0;                        // Red team control time
  greenTime = 0;                      // Green team control time

  int toneDuration = 50;              // Duration of tones in ms

  unsigned long lastDisplayToggle = millis(); // Last time display was toggled

  // Main game loop
  while (true) {  // Infinite loop until game over
    if (endGame) {
      gameOver();
      return;  // Exit after game over (optimization: added return to prevent fall-through)
    }

    keypad.getKey();  // Poll keypad for input

    elapsedGameTime = millis() - initialTime;  // Calculate elapsed game time

    // LED blinking for active team (every 1 second cycle)
    unsigned long blinkCycle = (millis() - initialTime) % 1000;
    if (blinkCycle >= 0 && blinkCycle <= 40) {
      if (team == 1) digitalWrite(GREENLED, HIGH);
      if (team == 2) digitalWrite(REDLED, HIGH);
    }
    if (blinkCycle >= 50 && blinkCycle <= 100) {
      if (team == 1) digitalWrite(GREENLED, LOW);
      if (team == 2) digitalWrite(REDLED, LOW);
    }

    // Play active tone sounds at specific intervals
    if (blinkCycle >= 0 && blinkCycle <= 40 && soundEnable) {
      tone(tonepin, activeTone, toneDuration);
    }
    if (blinkCycle >= 245 && blinkCycle <= 255 && (minutes - elapsedGameTime / 60000) < 2 && soundEnable) {
      tone(tonepin, activeTone, toneDuration);
    }
    if (blinkCycle >= 495 && blinkCycle <= 510 && (minutes - elapsedGameTime / 60000) < 4 && soundEnable) {
      tone(tonepin, activeTone, toneDuration);
    }
    if (blinkCycle >= 745 && blinkCycle <= 760 && (minutes - elapsedGameTime / 60000) < 2 && soundEnable) {
      tone(tonepin, activeTone, toneDuration);
    }

    // Toggle display every 2 seconds between game time and zone status
    if (millis() - lastDisplayToggle >= 2000) {
      lastDisplayToggle = millis();
      showGameTime = !showGameTime;
      cls();  // Clear LCD
    }

    // Display game time or zone control time
    if (showGameTime) {
      lcd.setCursor(3, 0);
      lcd.print("GAME TIME");
      lcd.setCursor(3, 1);
      printTime(minutes, elapsedGameTime);  // Print remaining game time
    } else {
      lcd.setCursor(2, 0);
      if (team == 0) lcd.print("NEUTRAL ZONE");
      if (team == 1) lcd.print(" GREEN ZONE");
      if (team == 2) lcd.print("  RED ZONE");

      if (team > 0) {
        lcd.setCursor(3, 1);
        printTimeDom(millis() - iZoneTime, true);  // Print zone control time
      }
    }

    // Check if game time has expired
    if (minutes - elapsedGameTime / 60000 == 0 && 59 - ((elapsedGameTime / 1000) % 60) == 0) {
      gameOver();
      return;
    }

    // Handle neutralizing a controlled zone (hold defuse or cancel)
    while ((defusing || cancelando) && team > 0) {
      cls();
      lcd.print("NEUTRALIZING...");
      lcd.setCursor(0, 1);
      unsigned int progressPercent = 0;
      unsigned long startActionTime = millis();  // Start time for neutralizing

      while (defusing || cancelando) {
        keypad.getKey();

        // Check for game time expiration during action
        elapsedGameTime = millis() - initialTime;
        if ((minutes - elapsedGameTime / 60000 == 0 && 59 - ((elapsedGameTime / 1000) % 60) == 0) ||
            minutes - elapsedGameTime / 60000 > 4000000000) {  // Overflow safeguard
          endGame = true;
          return;
        }

        // Alarm sounds and LED during neutralizing
        unsigned long actionCycle = (millis() - startActionTime) % 1000;
        if (actionCycle >= 0 && actionCycle <= 20 && soundEnable) {
          tone(tonepin, alarmTone1, 200);
        }
        if (actionCycle >= 480 && actionCycle <= 500 && soundEnable) {
          tone(tonepin, alarmTone2, 200);
        }

        // Calculate and draw progress
        unsigned long elapsedAction = millis() - startActionTime;
        progressPercent = (elapsedAction) / (ACTIVATESECONDS * 10);
        drawBar(progressPercent);

        // Complete neutralizing if progress reaches 100%
        if (progressPercent >= 100) {
          delay(1000);
          if (team == 1) greenTime += millis() - iZoneTime;
          if (team == 2) redTime += millis() - iZoneTime;
          iZoneTime = 0;
          team = 0;
          break;
        }
      }
      cls();
    }

    // Handle capturing zone for red team (hold defuse when neutral)
    while (defusing && team == 0) {
      cls();
      lcd.print(" CAPTURING ZONE");
      lcd.setCursor(0, 1);
      unsigned int progressPercent = 0;
      unsigned long startActionTime = millis();

      while (defusing) {
        keypad.getKey();

        // Check for game time expiration
        elapsedGameTime = millis() - initialTime;
        if ((minutes - elapsedGameTime / 60000 == 0 && 59 - ((elapsedGameTime / 1000) % 60) == 0) ||
            minutes - elapsedGameTime / 60000 > 4000000000) {
          endGame = true;
          return;
        }

        unsigned long actionCycle = (millis() - startActionTime) % 1000;
        if (actionCycle >= 0 && actionCycle <= 20) {
          digitalWrite(REDLED, HIGH);
          if (soundEnable) tone(tonepin, alarmTone1, 200);
        }
        if (actionCycle >= 480 && actionCycle <= 500) {
          if (soundEnable) tone(tonepin, alarmTone2, 200);
          digitalWrite(REDLED, LOW);
        }

        unsigned long elapsedAction = millis() - startActionTime;
        progressPercent = (elapsedAction) / (ACTIVATESECONDS * 10);
        drawBar(progressPercent);

        if (progressPercent >= 100) {
          digitalWrite(GREENLED, LOW);  // Ensure green LED off
          team = 2;
          iZoneTime = millis();
          delay(1000);
          break;
        }
      }
      cls();
      digitalWrite(REDLED, LOW);
    }

    // Handle capturing zone for green team (hold cancel when neutral)
    while (cancelando && team == 0) {
      cls();
      lcd.print(" CAPTURING ZONE");
      lcd.setCursor(0, 1);
      unsigned int progressPercent = 0;
      unsigned long startActionTime = millis();

      while (cancelando) {
        keypad.getKey();

        // Check for game time expiration
        elapsedGameTime = millis() - initialTime;
        if ((minutes - elapsedGameTime / 60000 == 0 && 59 - ((elapsedGameTime / 1000) % 60) == 0) ||
            minutes - elapsedGameTime / 60000 > 4000000000) {
          endGame = true;
          return;
        }

        unsigned long actionCycle = (millis() - startActionTime) % 1000;
        if (actionCycle >= 0 && actionCycle <= 20) {
          digitalWrite(GREENLED, HIGH);
          if (soundEnable) tone(tonepin, alarmTone1, 200);
        }
        if (actionCycle >= 480 && actionCycle <= 500) {
          if (soundEnable) tone(tonepin, alarmTone2, 200);
          digitalWrite(GREENLED, LOW);
        }

        unsigned long elapsedAction = millis() - startActionTime;
        progressPercent = (elapsedAction) / (ACTIVATESECONDS * 10);
        drawBar(progressPercent);

        if (progressPercent >= 100) {
          digitalWrite(REDLED, LOW);  // Ensure red LED off
          team = 1;
          iZoneTime = millis();
          delay(1000);
          break;
        }
      }
      cls();
      digitalWrite(GREENLED, LOW);
    }
  }
}

/**
 * gameOver() - Handles the end of the game.
 * This function is called when the game time expires. It updates final team times, displays the winner
 * based on which team controlled the zone longer, shows control times for each team, and prompts
 * the user to play again or return to the main menu.
 * 
 * Called from: domination() when time expires or endGame flag is set.
 * After displaying results, it either restarts domination() or calls menuPrincipal().
 */
void gameOver() {
  // Update final control time for the last controlling team
  if (team == 1) greenTime += millis() - iZoneTime;
  if (team == 2) redTime += millis() - iZoneTime;

  // Turn off LEDs
  digitalWrite(GREENLED, LOW);
  digitalWrite(REDLED, LOW);

  // Display game over and winner, cycling through info until defuse key is pressed
  while (!defusing) {
    keypad.getKey();
    if (defusing) break;

    cls();
    lcd.setCursor(3, 0);
    lcd.print("TIME OVER!");
    lcd.setCursor(0, 1);

    // Determine and display winner
    if (greenTime > redTime) {
      lcd.print(" GREEN TEAM WINS");
      digitalWrite(GREENLED, HIGH);
    } else {
      lcd.print("  RED TEAM WINS");
      digitalWrite(REDLED, HIGH);
    }
    delay(3000);

    if (defusing) break;

    cls();
    lcd.print("Red Time:");
    lcd.setCursor(5, 1);
    printTimeDom(redTime, false);
    delay(3000);

    if (defusing) break;

    cls();
    lcd.print("Green Time:");
    lcd.setCursor(5, 1);
    printTimeDom(greenTime, false);
    delay(3000);

    if (defusing) break;
  }

  // Reset LEDs after display
  digitalWrite(GREENLED, LOW);
  digitalWrite(REDLED, LOW);

  cls();
  delay(100);
  lcd.print("Play Again?");
  lcd.setCursor(0, 1);
  lcd.print("A : Yes B : No");

  // Wait for user choice
  while (true) {
    char choice = keypad.waitForKey();
    if (choice == 'a') {
      tone(tonepin, 2400, 30);
      cls();
      domination();  // Restart game
      return;
    }
    if (choice == 'b') {
      tone(tonepin, 2400, 30);
      menuPrincipal();  // Return to main menu
      return;
    }
  }
}
