// Splash.ino - End-Game Splash Screen Functions
// This file contains functions for displaying end-game outcomes (explosion, timeout failure, disarm success)
// in the airsoft game simulator. Each function resets LEDs, displays outcome messages on LCD,
// plays sounds or activates relay if applicable, and prompts the user to play again or return to main menu.
// Common logic (explosion sound, play again prompt) is extracted to reduce duplication.
// Optimizations:
// - Extracted explosionSound() for shared sound effect in explode and fail splashes.
// - Extracted playAgainPrompt() for shared "Play Again?" logic and game restart handling.
// - Made tones conditional on soundEnable.
// - Consolidated LCD clear operations (using cls() consistently).
// - Removed redundant LED resets in play again prompt.
// - Added checks for soundEnable in tones.
// - Improved readability with local variables and consistent formatting.
// - Assumes functions like cls(), tone(), noTone(), activateRelay(), startGameCount(),
//   search(), sabotage(), menuPrincipal() are defined elsewhere.
// - Global variables: endGame, REDLED, GREENLED, tonepin, relayEnable, soundEnable,
//   sdStatus, saStatus, start, var, keypad.

/**
 * explosionSound() - Plays a descending tone to simulate an explosion sound.
 * Loops from 200 Hz down to 1 Hz with 20ms delay per step (total ~4 seconds).
 * Only plays if soundEnable is true. Stops tone afterward.
 * 
 * Called from: explodeSplash(), failSplash().
 */
void explosionSound() {
  if (soundEnable) {
    for (int freq = 200; freq > 0; freq--) {
      tone(tonepin, freq);
      delay(20);
    }
    noTone(tonepin);
  }
}

/**
 * playAgainPrompt() - Displays "Play Again?" prompt and handles user input.
 * Clears LCD, shows prompt, waits for 'a' (yes: restart game based on status) or 'b' (no: main menu).
 * Resets LEDs before prompt. Plays confirmation tone on key press.
 * For Search & Destroy (sdStatus): Calls startGameCount() and search().
 * For Sabotage (saStatus): Sets start=true, calls startGameCount() and sabotage().
 * Loops until valid input.
 * 
 * Called from: explodeSplash(), failSplash(), disarmedSplash().
 */
void playAgainPrompt() {
  digitalWrite(REDLED, LOW);
  digitalWrite(GREENLED, LOW);
  cls();
  lcd.print("Play Again?");
  lcd.setCursor(0, 1);
  lcd.print("A : Yes B : No");

  while (true) {
    var = keypad.waitForKey();
    if (var == 'a') {
      if (soundEnable) tone(tonepin, 2400, 30);
      if (sdStatus) {
        startGameCount();
        search();
        return;  // Exit after restart
      } else if (saStatus) {
        startGameCount();
        start = true;  // Reset initial time for sabotage
        sabotage();
        return;
      }
    } else if (var == 'b') {
      if (soundEnable) tone(tonepin, 2400, 30);
      menuPrincipal();
      return;
    }
  }
}

/**
 * explodeSplash() - Displays splash for bomb explosion (terrorists win).
 * Resets LEDs and endGame flag. Shows "TERRORISTS WIN" and "GAME OVER".
 * Plays explosion sound, activates relay if enabled, delays 5s, then prompts to play again.
 * 
 * Called from: Game modes like destroySabotage(), destroy() on detonation.
 */
void explodeSplash() {
  digitalWrite(REDLED, LOW);
  digitalWrite(GREENLED, LOW);
  cls();
  delay(100);
  endGame = false;
  lcd.setCursor(1, 0);
  lcd.print("TERRORISTS WIN");
  lcd.setCursor(4, 1);
  lcd.print("GAME OVER");

  explosionSound();
  if (relayEnable) {
    activateRelay();
  }
  delay(5000);

  playAgainPrompt();
}

/**
 * failSplash() - Displays splash for game timeout failure.
 * Resets LEDs and endGame flag. Shows "TIME OUT" and "GAME OVER".
 * Plays explosion sound, activates relay if enabled, delays 5s, then prompts to play again.
 * 
 * Called from: Game modes like sabotage(), search() on time expiration.
 */
void failSplash() {
  digitalWrite(REDLED, LOW);
  digitalWrite(GREENLED, LOW);
  cls();
  delay(100);
  endGame = false;
  lcd.setCursor(1, 0);
  lcd.print("  TIME OUT");
  lcd.setCursor(4, 1);
  lcd.print("GAME OVER");

  explosionSound();
  if (relayEnable) {
    activateRelay();
  }
  delay(5000);

  playAgainPrompt();
}

/**
 * disarmedSplash() - Displays splash for successful bomb disarm (good guys win).
 * Resets LEDs and endGame flag. For sd or sa modes: Shows "BOMB DISARMED" and "GOOD GUYS WIN!",
 * flashes green LED for 5s. Then prompts to play again.
 * No sound or relay activation.
 * 
 * Called from: Game modes like destroy() on successful disarm.
 */
void disarmedSplash() {
  endGame = false;
  digitalWrite(REDLED, LOW);
  digitalWrite(GREENLED, LOW);

  if (sdStatus || saStatus) {
    cls();
    lcd.setCursor(2, 0);
    lcd.print("BOMB DISARMED");
    lcd.setCursor(3, 1);
    lcd.print("GOOD GUYS WIN!");
    digitalWrite(GREENLED, HIGH);
    delay(5000);
    digitalWrite(GREENLED, LOW);
  }

  playAgainPrompt();
}
