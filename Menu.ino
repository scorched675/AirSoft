// Menu.ino - Menu Navigation and Quick Configuration Functions
// This file contains functions for the main menu, configuration submenu, and quick game setup.
// Menus allow selection of game modes (Search & Destroy, Sabotage, Domination) or configuration.
// Quick config prompts for game parameters like times, sound, relay, and password enable.
// Navigation uses keypad buttons (BT_UP, BT_DOWN, BT_SEL, BT_CANCEL).
// Optimizations:
// - Fixed bug in main menu: menu1 has 4 items (indices 0-3), updated checks from i<2 to i<3.
// - Removed gotos in configQuickGame(); replaced with do-while loops for input retry.
// - Added input validation and improved readability with local variables.
// - Consolidated tone plays and delays where possible.
// - Added comprehensive documentation for functions and sections.
// - Assumes functions like cls(), checkArrows(), startGameCount(), search(), sabotage(), domination(),
//   setNewPass(), getRealNumber(), tone() are defined elsewhere.
// - Uses strings from Lang.ino for display text.

/**
 * menuPrincipal() - Displays and handles the main menu for game mode selection.
 * Resets LEDs and game status flags. Displays menu items from menu1[] array.
 * Allows navigation with BT_UP/BT_DOWN, selection with BT_SEL.
 * On selection, configures quick game settings if applicable, starts countdown, and launches the chosen game mode.
 * Loops indefinitely until a selection is made.
 * 
 * Called from: loop(), gameOver(), or other exit points.
 */
void menuPrincipal() {
  // Reset LEDs and game mode flags
  digitalWrite(GREENLED, LOW);
  digitalWrite(REDLED, LOW);
  digitalWrite(BLUELED, LOW);
  saStatus = false;
  sdStatus = false;
  doStatus = false;

  // Clear LCD and initialize menu index
  cls();
  int menuIndex = 0;
  lcd.print(menu1[menuIndex]);
  lcd.setCursor(15, 1);
  checkArrows(menuIndex, 3);  // 4 items: indices 0-3

  while (true) {
    char key = keypad.waitForKey();
    if (key == BT_UP && menuIndex > 0) {
      if (soundEnable) tone(tonepin, 2400, 30);
      menuIndex--;
      cls();
      lcd.print(menu1[menuIndex]);
      checkArrows(menuIndex, 3);
      delay(50);
    } else if (key == BT_DOWN && menuIndex < 3) {  // Fixed: was i<2, now <3 for 4 items
      if (soundEnable) tone(tonepin, 2400, 30);
      menuIndex++;
      cls();
      lcd.print(menu1[menuIndex]);
      checkArrows(menuIndex, 3);
      delay(50);
    } else if (key == BT_SEL) {
      if (soundEnable) tone(tonepin, 2400, 30);
      cls();
      switch (menuIndex) {
        case 0:  // Search & Destroy
          sdStatus = true;
          configQuickGame();
          startGameCount();
          search();
          break;
        case 1:  // Sabotage
          saStatus = true;
          configQuickGame();
          startGameCount();
          sabotage();
          break;
        case 2:  // Domination
          doStatus = true;
          configQuickGame();
          startGameCount();
          domination();
          break;
        case 3:  // Configuration
          config();
          break;
      }
      return;  // Exit menu after selection (optimization: prevent fall-through)
    }
  }
}

/**
 * config() - Displays and handles the configuration submenu.
 * Displays items from menu2[] array (Game Config, Sound Config, Relay Test, Auto Test).
 * Allows navigation with BT_UP/BT_DOWN, selection with BT_SEL, exit with BT_CANCEL.
 * Currently implements Relay Test; other options are placeholders (commented out).
 * Loops indefinitely until cancel or selection completes.
 * 
 * Called from: menuPrincipal().
 */
void config() {
  cls();
  int menuIndex = 0;
  lcd.print(menu2[menuIndex]);
  checkArrows(menuIndex, 3);  // 4 items: indices 0-3
  delay(500);  // Short delay for display stability

  while (true) {
    char key = keypad.waitForKey();
    if (key == BT_UP && menuIndex > 0) {
      if (soundEnable) tone(tonepin, 2400, 30);
      menuIndex--;
      cls();
      lcd.print(menu2[menuIndex]);
      checkArrows(menuIndex, 3);
      delay(50);
    } else if (key == BT_DOWN && menuIndex < 3) {
      if (soundEnable) tone(tonepin, 2400, 30);
      menuIndex++;
      cls();
      lcd.print(menu2[menuIndex]);
      checkArrows(menuIndex, 3);
      delay(50);
    } else if (key == BT_CANCEL) {
      if (soundEnable) tone(tonepin, 2400, 30);
      menuPrincipal();
      return;
    } else if (key == BT_SEL) {
      if (soundEnable) tone(tonepin, 2400, 30);
      cls();
      switch (menuIndex) {
        case 0:
          // gameConfigMenu();  // Placeholder for full game config
          break;
        case 1:
          // soundConfigMenu();  // Placeholder for sound config
          break;
        case 2:  // Relay Test
          lcd.print("RELAYPIN ON!");
          digitalWrite(RELAYPIN, HIGH);
          delay(4000);
          cls();
          lcd.print("RELAYPIN OFF!");
          digitalWrite(RELAYPIN, LOW);
          delay(2000);
          config();  // Return to config menu
          return;
        case 3:
          // Auto Test placeholder
          break;
      }
    }
  }
}

/**
 * configQuickGame() - Prompts for quick game configuration parameters.
 * Depending on game mode (sdStatus, saStatus, doStatus), prompts for:
 * - Game time (hh:mm converted to total minutes)
 * - Bomb time (minutes)
 * - Arm time (seconds)
 * - Sound enable (yes/no)
 * - Relay enable (yes/no, for sd/sa)
 * - Password enable (yes/no with setNewPass(), for sd/sa)
 * Uses do-while loops for input retry instead of gotos.
 * Validates numeric input via getRealNumber().
 * Updates global variables like GAMEMINUTES, BOMBMINUTES, ACTIVATESECONDS, soundEnable, relayEnable, passwordEnable.
 * 
 * Called from: menuPrincipal() before starting a game mode.
 */
void configQuickGame() {
  cls();

  // Game Time (for all modes)
  if (sdStatus || doStatus || saStatus) {
    boolean accepted = false;
    do {
      cls();
      lcd.print(GAME_TIME);
      delay(100);
      lcd.setCursor(0, 1);
      lcd.print("00:00 hh:mm");
      lcd.cursor();
      lcd.blink();
      lcd.setCursor(0, 1);

      boolean colonPrinted = false;
      for (int i = 0; i < 4; i++) {
        if (i == 2 && !colonPrinted) {
          lcd.print(":");
          colonPrinted = true;
        }
        while (true) {
          byte digit = getRealNumber();
          if (digit != 11) {  // Valid 0-9
            time[i] = digit;
            lcd.print(digit);
            if (soundEnable) tone(tonepin, 2400, 30);
            break;
          }
        }
      }
      lcd.noCursor();
      lcd.noBlink();
      lcd.setCursor(13, 1);
      lcd.print("ok?");

      // Confirm or retry
      while (true) {
        char key = keypad.waitForKey();
        if (key == BT_SEL) {  // Accept
          if (soundEnable) tone(tonepin, 2400, 30);
          GAMEMINUTES = (time[0] * 600) + (time[1] * 60) + (time[2] * 10) + time[3];
          accepted = true;
          break;
        } else if (key == BT_CANCEL) {  // Retry
          if (soundEnable) tone(tonepin, 2400, 30);
          break;
        }
      }
    } while (!accepted);
    cls();
  }

  // Bomb Time (for sd and sa)
  if (sdStatus || saStatus) {
    boolean accepted = false;
    do {
      cls();
      lcd.print(BOMB_TIME);
      delay(100);
      lcd.setCursor(0, 1);
      lcd.print(ZERO_MINUTES);
      lcd.cursor();
      lcd.blink();
      lcd.setCursor(0, 1);

      for (int i = 0; i < 2; i++) {
        while (true) {
          byte digit = getRealNumber();
          if (digit != 11) {
            time[i] = digit;
            lcd.print(digit);
            if (soundEnable) tone(tonepin, 2400, 30);
            break;
          }
        }
      }
      lcd.noCursor();
      lcd.noBlink();
      lcd.setCursor(13, 1);
      lcd.print("ok?");

      while (true) {
        char key = keypad.waitForKey();
        if (key == BT_SEL) {
          if (soundEnable) tone(tonepin, 2400, 30);
          BOMBMINUTES = (time[0] * 10) + time[1];
          accepted = true;
          break;
        } else if (key == BT_CANCEL) {
          if (soundEnable) tone(tonepin, 2400, 30);
          break;
        }
      }
    } while (!accepted);
    cls();
  }

  // Arm Time (for all modes)
  if (sdStatus || doStatus || saStatus) {
    boolean accepted = false;
    do {
      cls();
      lcd.print(ARM_TIME);
      delay(100);
      lcd.setCursor(0, 1);
      lcd.print(ZERO_SECS);
      lcd.cursor();
      lcd.blink();
      lcd.setCursor(0, 1);

      for (int i = 0; i < 2; i++) {
        while (true) {
          byte digit = getRealNumber();
          if (digit != 11) {
            time[i] = digit;
            lcd.print(digit);
            if (soundEnable) tone(tonepin, 2400, 30);
            break;
          }
        }
      }
      lcd.noCursor();
      lcd.noBlink();
      lcd.setCursor(13, 1);
      lcd.print("ok?");

      while (true) {
        char key = keypad.waitForKey();
        if (key == BT_SEL) {
          if (soundEnable) tone(tonepin, 2400, 30);
          ACTIVATESECONDS = (time[0] * 10) + time[1];
          accepted = true;
          break;
        } else if (key == BT_CANCEL) {
          if (soundEnable) tone(tonepin, 2400, 30);
          break;
        }
      }
    } while (!accepted);
    cls();
  }

  // Enable Sound (for all modes)
  if (sdStatus || saStatus || doStatus) {
    cls();
    lcd.print(ENABLE_SOUND);
    lcd.setCursor(0, 1);
    lcd.print(YES_OR_NOT);

    while (true) {
      char key = keypad.waitForKey();
      if (key == 'a') {
        soundEnable = true;
        if (soundEnable) tone(tonepin, 2400, 30);
        break;
      } else if (key == 'b') {
        soundEnable = false;
        if (soundEnable) tone(tonepin, 2400, 30);  // Still play if was enabled before
        break;
      }
    }
    cls();
  }

  // Enable Relay (for sd and sa)
  if (sdStatus || saStatus) {
    cls();
    lcd.print(ENABLE_RELAYPIN);
    lcd.setCursor(0, 1);
    lcd.print(YES_OR_NOT);

    while (true) {
      char key = keypad.waitForKey();
      if (key == 'a') {
        relayEnable = true;
        if (soundEnable) tone(tonepin, 2400, 30);
        break;
      } else if (key == 'b') {
        relayEnable = false;
        if (soundEnable) tone(tonepin, 2400, 30);
        break;
      }
    }
    cls();
  }

  // Enable Password (for sd and sa)
  if (sdStatus || saStatus) {
    cls();
    lcd.print(ENABLE_CODE);
    lcd.setCursor(0, 1);
    lcd.print(YES_OR_NOT);

    while (true) {
      char key = keypad.waitForKey();
      if (key == 'a') {
        if (soundEnable) tone(tonepin, 2400, 30);
        setNewPass();
        passwordEnable = true;
        break;
      } else if (key == 'b') {
        if (soundEnable) tone(tonepin, 2400, 30);
        passwordEnable = false;
        break;
      }
    }
    cls();
  }
}
