// ARDUINO AIRSOFT GAME SIMULATOR
// This sketch simulates airsoft games like bomb defusal, search and destroy, sabotage, and demolition.
// It uses a keypad for input, an LCD display for output, LEDs for status indication, a relay for activation,
// and a buzzer for sound effects. The game supports configurable times, passwords, and team controls.

// Include necessary libraries
#include <Wire.h>            // For I2C communication
#include <Keypad.h>          // Base keypad library
#include <Keypad_I2C.h>      // I2C version of keypad library
#include <LiquidCrystal_I2C.h> // I2C LCD library

// Define constants
#define KB1I2CADDR 0x20      // I2C address for the keypad
#define LCD_I2C_ADDR 0x3F    // I2C address for the LCD

// Initialize LCD: 16 columns, 2 rows
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 16, 2);

// Keypad configuration
const byte ROWS = 4; // Number of rows in the keypad
const byte COLS = 4; // Number of columns in the keypad
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'a'},
  {'4', '5', '6', 'b'},
  {'7', '8', '9', 'c'},
  {'*', '0', '#', 'd'}
};

// Pin connections for keypad (using analog and digital pins)
byte rowPins[ROWS] = {A0, A1, A2, A3}; // Row pinouts
byte colPins[COLS] = {A4, A5, 13, 12}; // Column pinouts

// Initialize I2C keypad
Keypad_I2C keypad = Keypad_I2C(makeKeymap(keys), rowPins, colPins, ROWS, COLS, KB1I2CADDR);

// Global variables for game state and input
char codeInput[8];          // Buffer for user-entered codes (e.g., passwords)
byte time[4];               // Array for time components (hours, minutes, etc.)
boolean refresh = true;     // Flag to refresh display once
char password[8];           // Stored password for game modes
int key = -1;               // Current key index or state
char lastKey;               // Last pressed key
char var;                   // Temporary variable for key handling
boolean passwordEnable = false; // Flag to enable password protection

// Button mappings for navigation and actions (based on keypad keys)
const char BT_RIGHT = '4';   // Right navigation
const char BT_UP = 'a';      // Up navigation
const char BT_DOWN = 'b';    // Down navigation
const char BT_LEFT = '6';    // Left navigation
const char BT_SEL = 'd';     // Select/OK key
const char BT_CANCEL = 'c';  // Cancel key
const char BT_DEFUSER = 'x'; // Defuser key (not implemented)

// LED pins for status indicators
const int REDLED = 11;       // Red LED (e.g., for alarm or red team)
const int GREENLED = 10;     // Green LED (e.g., for success or green team)
const int BLUELED = 12;      // Blue LED (e.g., for neutral or other states)

// Relay configuration
boolean relayEnable = false; // Flag to enable relay activation
const int RELAYPIN = 9;      // Pin for relay control
// Time in milliseconds the relay stays active (test this value carefully)
const int RELAY_TIME = 5000; // 5 seconds (adjusted from default 1 second)

// Game time configurations (in minutes/seconds)
int GAMEHOURS = 0;           // Game duration hours
int GAMEMINUTES = 45;        // Game duration minutes
int BOMBMINUTES = 4;         // Bomb timer minutes
int ACTIVATESECONDS = 5;     // Activation delay in seconds

// Game state flags
boolean endGame = false;     // Flag for game over
boolean sdStatus = false;    // Search and Destroy mode enabled
boolean saStatus = false;    // Sabotage mode enabled
boolean doStatus = false;    // Demolition mode enabled
boolean start = true;        // Game start flag
boolean defusing = false;    // Defusing in progress
boolean cancelando = false;  // Canceling in progress

// Sound configurations
boolean soundEnable = true;  // Enable sound effects
const int tonepin = 8;       // Pin for buzzer
const int alarmTone1 = 700;  // Low alarm tone frequency
const int alarmTone2 = 2600; // High alarm tone frequency
const int activeTone = 1330; // Activation tone frequency
const int errorTone = 100;   // Error tone frequency

// Timing variables (unsigned long for millis() compatibility)
unsigned long iTime;         // Initial time
unsigned long timeCalcVar;   // Temporary time calculation variable
unsigned long redTime;       // Time for red team/phase
unsigned long greenTime;     // Time for green team/phase
unsigned long iZoneTime;     // Initial time for zone control
byte team = 0;               // Current team: 0=neutral, 1=green, 2=red

/**
 * setup() - Initializes hardware and displays splash screen.
 * This function runs once at startup. It sets up the LCD, keypad, pins,
 * and custom characters for the progress bar. It also shows a splash screen
 * with a delay to ensure visibility.
 */
void setup() {
  lcd.init();                // Initialize LCD (called once)
  lcd.backlight();           // Turn on backlight
  Serial.begin(9600);        // Start serial communication (for debugging if needed)
  
  // Display splash screen
  lcd.setCursor(0, 0);
  lcd.print("CZ Airsoft");   // Game title or brand
  lcd.setCursor(0, 1);
  lcd.print("AIRSOFT SIM v1.0"); // Version info
  tone(tonepin, 2400, 30);   // Play startup tone
  delay(2000);               // Delay to keep splash screen visible (optimization: added to fix quick clear issue)
  lcd.clear();               // Clear screen after splash
  
  // Keypad setup
  keypad.setHoldTime(50);    // Set hold time for long presses
  keypad.setDebounceTime(50);// Set debounce time to reduce noise
  keypad.addEventListener(keypadEvent); // Attach event listener
  
  // Set pin modes
  pinMode(GREENLED, OUTPUT); // Green LED output
  pinMode(REDLED, OUTPUT);   // Red LED output
  pinMode(BLUELED, OUTPUT);  // Blue LED output
  pinMode(RELAYPIN, OUTPUT); // Relay output
  
  // Define custom LCD characters for progress bar and arrows
  byte bar1[8] = { B10000, B10000, B10000, B10000, B10000, B10000, B10000, B10000 };
  byte bar2[8] = { B11000, B11000, B11000, B11000, B11000, B11000, B11000, B11000 };
  byte bar3[8] = { B11100, B11100, B11100, B11100, B11100, B11100, B11100, B11100 };
  byte bar4[8] = { B11110, B11110, B11110, B11110, B11110, B11110, B11110, B11110 };
  byte bar5[8] = { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111 };
  byte up[8] = { B00000, B00100, B01110, B11111, B11111, B00000, B00000, B00000 };
  byte down[8] = { B00000, B00000, B11111, B11111, B01110, B00100, B00000, B00000 };
  
  lcd.createChar(0, bar1);
  lcd.createChar(1, bar2);
  lcd.createChar(2, bar3);
  lcd.createChar(3, bar4);
  lcd.createChar(4, bar5);
  lcd.createChar(5, up);
  lcd.createChar(6, down);
}

/**
 * loop() - Main program loop.
 * This function runs repeatedly. It calls the main menu function to handle game logic.
 * All game interactions happen through menuPrincipal() (assumed defined elsewhere).
 */
void loop() {
  menuPrincipal(); // Call main menu (optimize: ensure this is defined in full code)
}

/**
 * keypadEvent(KeypadEvent key) - Handles keypad events like presses, holds, and releases.
 * This listener function detects key states:
 * - On hold: Sets flags for defusing or canceling if 'd' or 'c' is held.
 * - On release: Resets flags for defusing or canceling.
 * It allows for long-press actions in the game (e.g., holding to defuse a bomb).
 */
void keypadEvent(KeypadEvent key) {
  switch (keypad.getState()) {
    case RELEASED:
      switch (key) {
        case 'd':
          defusing = false; // Stop defusing on release
          break;
        case 'c':
          cancelando = false; // Stop canceling on release
          break;
      }
      break;
    case HOLD:
      switch (key) {
        case 'd':
          defusing = true; // Start defusing on hold
          break;
        case 'c':
          cancelando = true; // Start canceling on hold
          break;
      }
      break;
  }
}
