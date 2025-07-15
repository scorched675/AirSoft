// Zutils.ino - Utility Functions for Airsoft Game Simulator
// This file contains helper functions for LCD display management, time printing,
// progress bars, game countdown, menu navigation indicators, and relay activation.
// These are used across game modes for consistent UI and hardware control.
// Optimizations:
// - In drawBar(): Simplified loop logic using integer division/modulo for full/partial bars.
//   Reduced redundant checks; now handles 0-100% more efficiently.
// - In printTime(): Removed unnecessary refresh logic and Serial.print (debug remnant).
//   Optimized formatting with leading zeros and direct calculations.
// - In printTimeDom(): Consistent formatting; optional millis display.
// - In startGameCount(): Minor tone conditional on soundEnable (if defined globally).
// - In checkArrows(): Simplified conditions for drawing up/down arrows based on menu position.
// - In activateRelay(): No changes needed; simple and efficient.
// - Added input validation where applicable (e.g., percent clamped in drawBar).
// - Improved readability: Consistent variable naming, added comments, removed unused code.
// Assumptions: lcd is LiquidCrystal_I2C object; custom chars 0-4 for bar segments, 5=up arrow, 6=down arrow.
// Globals: tonepin, RELAYPIN, RELAY_TIME, soundEnable (if used), keypad.

/**
 * drawBar(byte percent) - Draws a horizontal progress bar on the LCD at cursor (0,1).
 * Uses custom characters (0-4) for partial/full bar segments. Each segment represents 12.5% (1/8th of LCD width).
 * Clamps percent to 0-100. Clears the line if percent=0.
 * 
 * @param percent Progress percentage (0-100).
 */
void drawBar(byte percent) {
  if (percent > 100) percent = 100;  // Clamp to max 100%
  int fullBars = percent / (100 / 16);  // 16 possible segments (assuming 16-char LCD width)
  lcd.setCursor(0, 1);
  
  // Draw full bars (char 4 = full segment)
  for (int i = 0; i < fullBars / 5; i++) {  // Each full bar is 5 units (full char)
    lcd.write(4);
  }
  
  // Draw partial bar if any remainder
  byte remainder = fullBars % 5;
  if (remainder > 0) {
    lcd.write(remainder - 1);  // Chars 0-3 for 1-4 units
  }
  
  // Clear remaining space (optional: for cleaner display if bar shrinks)
  for (int i = fullBars; i < 16; i++) {
    lcd.print(" ");
  }
}

/**
 * cls() - Clears the LCD screen and resets cursor to (0,0).
 * Simple wrapper for common operation to ensure consistent clearing.
 */
void cls() {
  lcd.clear();
  lcd.setCursor(0, 0);
}

/**
 * printTime(unsigned long minutes, unsigned long aTime) - Prints countdown time on LCD.
 * Format: [hh:]mm:ss:ms (hours optional if <1). Uses pre-calculated initial minutes and elapsed aTime (millis).
 * Handles leading zeros. Millis is approximate (999 - millis()%1000 for effect).
 * Cursor should be set before calling (e.g., to (3,1)).
 * 
 * @param minutes Initial minutes for countdown.
 * @param aTime Elapsed time in milliseconds.
 */
void printTime(unsigned long minutes, unsigned long aTime) {
  unsigned long remainingMinutes = minutes - (aTime / 60000UL);
  
  // Hours (if applicable)
  unsigned long hours = remainingMinutes / 60;
  if (hours >= 1) {
    if (hours < 10) lcd.print("0");
    lcd.print(hours);
    lcd.print(":");
  }
  
  // Minutes
  unsigned long mins = remainingMinutes % 60;
  if (mins < 10) lcd.print("0");
  lcd.print(mins);
  lcd.print(":");
  
  // Seconds
  unsigned long secs = aTime / 1000UL;
  unsigned long remainingSecs = 59 - (secs % 60);
  if (remainingSecs < 10) lcd.print("0");
  lcd.print(remainingSecs);
  lcd.print(":");
  
  // Milliseconds (approximate descending effect)
  lcd.print(999 - (millis() % 1000));
}

/**
 * printTimeDom(unsigned long aTime, boolean showMillis) - Prints elapsed time for Domination mode.
 * Format: mm:ss[:ms] (millis optional). Cursor should be set before calling.
 * Handles leading zeros. Millis is approximate descending effect.
 * 
 * @param aTime Elapsed time in milliseconds.
 * @param showMillis If true, append :ms; else omit.
 */
void printTimeDom(unsigned long aTime, boolean showMillis) {
  // Minutes
  unsigned long mins = aTime / 60000UL;
  if (mins < 10) lcd.print("0");
  lcd.print(mins);
  lcd.print(":");
  
  // Seconds
  unsigned long secs = (aTime / 1000UL) % 60;
  if (secs < 10) lcd.print("0");
  lcd.print(secs);
  
  // Optional milliseconds
  if (showMillis) {
    lcd.print(":");
    lcd.print(999 - (millis() % 1000));
  }
}

/**
 * startGameCount() - Displays ready prompt and starts game countdown (5 to 1).
 * Waits for any key to start. Plays tone per count if soundEnable is true.
 * Clears LCD after countdown.
 */
void startGameCount() {
  cls();
  lcd.setCursor(1, 0);
  lcd.print("Ready to Begin");
  lcd.setCursor(0, 1);
  lcd.print("Push ANY Button");
  keypad.waitForKey();  // Wait for input to proceed
  
  cls();
  lcd.setCursor(1, 0);
  lcd.print("Starting Game");
  
  // Countdown from 5 to 1
  for (int i = 5; i > 0; i--) {
    lcd.setCursor(5, 1);
    if (soundEnable) tone(tonepin, 2000, 100);
    lcd.print("IN ");
    lcd.print(i);
    delay(1000);
  }
  cls();
}

/**
 * checkArrows(byte i, byte maxx) - Draws up/down arrows on LCD for menu navigation.
 * Positions arrows at (15,0) for up and (15,1) for down based on current index i and max index.
 * Arrow chars: 5=up, 6=down.
 * 
 * @param i Current menu index (0 to maxx).
 * @param maxx Maximum menu index.
 */
void checkArrows(byte i, byte maxx) {
  if (i == 0) {  // Only down arrow
    lcd.setCursor(15, 1);
    lcd.write(6);
  } else if (i == maxx) {  // Only up arrow
    lcd.setCursor(15, 0);
    lcd.write(5);
  } else if (i > 0 && i < maxx) {  // Both arrows
    lcd.setCursor(15, 0);
    lcd.write(5);
    lcd.setCursor(15, 1);
    lcd.write(6);
  }
}

/**
 * activateRelay() - Activates the relay pin for RELAY_TIME milliseconds.
 * Sets RELAYPIN HIGH, delays, then LOW. Used for end-game effects (e.g., explosion simulation).
 */
void activateRelay() {
  digitalWrite(RELAYPIN, HIGH);
  delay(RELAY_TIME);
  digitalWrite(RELAYPIN, LOW);
}
