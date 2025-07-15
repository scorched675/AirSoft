/**
 * comparePassword() - Compares the user-entered code with the stored password.
 * This function iterates through the 8-character arrays codeInput and password.
 * If all characters match, it returns true; otherwise, false.
 * Used for password verification in game modes requiring authentication.
 * 
 * Returns: true if passwords match, false otherwise.
 */
boolean comparePassword() {
  for (int i = 0; i < 8; i++) {
    if (codeInput[i] != password[i]) {
      return false;
    }
  }
  return true;
}

/**
 * setCode() - Reads an 8-character code from the keypad and stores it in codeInput.
 * This function displays asterisks (*) for previous characters to mask input (like a PIN entry).
 * It uses getNumber() to fetch each valid keypress. Plays a tone on each successful input.
 * The LCD cursor is set to (0,1) before starting.
 * 
 * Used for entering codes during game actions (e.g., defusing).
 */
void setCode() {
  lcd.setCursor(0, 1);
  for (int i = 0; i < 8; i++) {
    while (true) {
      char inputChar = getNumber();
      if (inputChar != 'x') {
        codeInput[i] = inputChar;
        if (i != 0) {
          lcd.setCursor(i - 1, 1);
          lcd.print("*");
        }
        lcd.setCursor(i, 1);  // Ensure cursor is at current position
        lcd.print(inputChar);
        if (soundEnable) tone(tonepin, 2400, 30);
        break;
      }
    }
  }
}

/**
 * setCodeTime() - Reads an 8-character code from the keypad with a time limit.
 * Similar to setCode(), but enforces a time limit based on ACTIVATESECONDS.
 * If time expires for any digit, sets that digit to 'x' and continues.
 * Displays remaining time on LCD at (11,0). Plays tone on successful input.
 * 
 * Used for time-sensitive code entries (e.g., during bomb activation/defusal).
 */
void setCodeTime() {
  unsigned long startTime = millis();  // Start timer for the entire entry
  for (int i = 0; i < 8; i++) {
    while (true) {
      // Check if time has expired
      if (ACTIVATESECONDS * 1000 + startTime - millis() <= 100) {
        codeInput[i] = 'x';
        break;
      }

      // Display remaining time
      lcd.setCursor(11, 0);
      printTimeDom(ACTIVATESECONDS * 1000 + startTime - millis(), false);

      char inputChar = getNumber();
      if (inputChar != 'x') {
        codeInput[i] = inputChar;
        if (i != 0) {
          lcd.setCursor(i - 1, 1);
          lcd.print("*");
        }
        lcd.setCursor(i, 1);
        lcd.print(inputChar);
        if (soundEnable) tone(tonepin, 2400, 30);
        break;
      }
    }
  }
}

/**
 * setPass() - Reads an 8-character password from the keypad and stores it in password.
 * Similar to setCode(), masks previous characters with asterisks.
 * Plays a tone on each successful input. LCD cursor set to (0,1).
 * 
 * Used internally by setNewPass() for setting the system password.
 */
void setPass() {
  lcd.setCursor(0, 1);
  for (int i = 0; i < 8; i++) {
    while (true) {
      char inputChar = getNumber();
      if (inputChar != 'x') {
        password[i] = inputChar;
        if (i != 0) {
          lcd.setCursor(i - 1, 1);
          lcd.print("*");
        }
        lcd.setCursor(i, 1);
        lcd.print(inputChar);
        if (soundEnable) tone(tonepin, 2400, 30);
        break;
      }
    }
  }
}

/**
 * setNewPass() - Sets a new password by prompting for entry twice and verifying match.
 * Prompts user to enter and retype the password. If they match (via comparePassword()),
 * confirms success; otherwise, shows error and retries. Plays error tone on mismatch.
 * Clears LCD between prompts and shows messages.
 * 
 * Called from configuration menus to change the game password.
 */
void setNewPass() {
  while (true) {
    cls();  // Clear LCD
    lcd.setCursor(0, 0);
    lcd.print("Enter New Pass");
    setPass();

    cls();
    lcd.setCursor(0, 0);
    lcd.print("Retype Pass");
    setCode();

    if (comparePassword()) {
      cls();
      lcd.setCursor(0, 0);
      lcd.print("Password Set OK!");
      delay(2000);
      break;
    } else {
      cls();
      lcd.setCursor(0, 0);
      lcd.print("ERROR Dont Match!");
      if (soundEnable) tone(tonepin, errorTone, 200);
      delay(2000);
    }
  }
}

/**
 * getNumber() - Waits for a keypad press and returns the character if it's a number (0-9).
 * Loops until a key is pressed. Returns the char for '0'-'9', or 'x' for any other key
 * (including navigation keys like 'a','b','c','d','*','#').
 * Uses non-blocking keypad.getKey(), so may loop tightly if no key is pressed.
 * 
 * Returns: char '0'-'9' if number pressed, 'x' otherwise.
 */
char getNumber() {
  while (true) {
    char keyPressed = keypad.getKey();
    if (keyPressed) {
      switch (keyPressed) {
        case '0' ... '9':
          return keyPressed;
        default:
          return 'x';
      }
    }
    // No delay here to keep responsive, but consider adding if CPU usage is an issue
  }
}

/**
 * getRealNumber() - Blocking wait for a keypad press, returns byte 0-9 if number, 11 if invalid.
 * Uses blocking keypad.waitForKey(). Converts '0'-'9' to byte values 0-9.
 * Any other key returns 11 (invalid).
 * 
 * Returns: byte 0-9 for numbers, 11 for invalid keys.
 */
byte getRealNumber() {
  while (true) {
    char keyPressed = keypad.waitForKey();
    if (keyPressed >= '0' && keyPressed <= '9') {
      return keyPressed - '0';  // Convert char to byte (0-9)
    } else {
      return 11;  // Invalid
    }
  }
}
