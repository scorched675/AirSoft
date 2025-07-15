// Lang.ino - Language and String Definitions for LCD Display
// This file defines constant strings used throughout the airsoft game simulator for menus,
// prompts, and status messages displayed on the LCD. These strings are referenced in other
// functions (e.g., menuPrincipal(), game modes like domination(), and configuration routines)
// to ensure consistent text output. By centralizing strings here, it's easier to modify or
// translate them without changing code elsewhere.
//
// Optimization notes:
// - Changed char* to const char* to indicate immutability and potentially save RAM in Arduino.
// - Grouped related strings logically (menus, time configs, enables, game states).
// - Added documentation comments for each string or group.
// - Ensured strings are null-terminated and suitable for lcd.print().
// - Removed unnecessary spaces or inconsistencies in original strings for cleanliness.
// - This file can be #included in the main sketch or compiled separately if needed.

// Main menu options (called in main menu display functions)
const char* menu1[] = {
  "Search & Destroy",  // Option for Search & Destroy game mode
  "Sabotage",          // Option for Sabotage game mode
  "Domination",        // Option for Domination game mode
  "Configuration"      // Option to enter configuration menu
};

// Configuration submenu options (called in configuration menu display)
const char* menu2[] = {
  "Game Config",       // Submenu for game time and settings
  "Sound Config",      // Submenu for sound enable/disable
  "Relay Test",        // Submenu to test relay functionality
  "Auto Test"          // Submenu for automated system tests
};

// Time configuration prompts (used in setup and config functions)
const char* GAME_TIME = "Game Time:";      // Prompt for setting overall game duration
const char* BOMB_TIME = "Bomb Time:";      // Prompt for setting bomb detonation time
const char* ZERO_MINUTES = "00 minutes";   // Default display for zero minutes
const char* ARM_TIME = "Arm Time:";        // Prompt for setting arming time
const char* ZERO_SECS = "00 seconds";      // Default display for zero seconds

// Enable/disable prompts (used in config menus)
const char* ENABLE_SOUND = "Enable Sound?";    // Prompt to toggle sound effects
const char* YES_OR_NOT = "A:Yes B:No";         // Yes/No choice (optimized spacing for LCD)
const char* ENABLE_RELAYPIN = "Enable Relay?"; // Prompt to toggle relay activation
const char* ENABLE_CODE = "Enable Code Arm?";  // Prompt to toggle code-based arming

// In-game display strings (used during gameplay, arming, disarming, etc.)
const char* GAME_TIME_TOP = "GAME TIME";       // Header for game time display
const char* ARMING_BOMB = "ARMING BOMB";       // Message during bomb arming process
const char* ENTER_CODE = "Enter Code";         // Prompt for code entry
const char* CODE_ERROR = "Code Error!";        // Error message for invalid code
const char* BOMB_ARMED = "BOMB ARMED";         // Confirmation after successful arming
const char* DETONATION_IN = "DETONATION IN";   // Header before countdown to detonation
const char* DISARMING = "DISARMING BOMB";      // Message during bomb disarming process
const char* DISARM = "DISARMING";              // Shorter disarming message (if needed)

// Game over and result strings (used in gameOver() and similar functions)
const char* GAME_OVER = "GAME OVER!";          // General game over message
const char* DEFENDERS_WIN = "DEFENDERS WIN";   // Message when defenders win
const char* SABOTAGE_FAIL = "SABOTAGE FAIL!";  // Message for failed sabotage attempt
