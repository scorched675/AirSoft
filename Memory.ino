// EEPROM Configuration Management
// This section handles reading and writing configuration data to EEPROM for persistent storage.
// It uses a magic number (signature) to detect if the EEPROM has been initialized with valid data.
// If not initialized, it writes default values. This prevents using garbage data on first boot or after resets.
// EEPROM addresses are allocated as follows (example; adjust based on actual variables):
// - Address 0-1: Magic number (16-bit: 0xBAD0)
// - Address 2: Initialization flag (1 byte: 1 = initialized)
// - Address 3-4: RELAY_TIME (16-bit, in seconds; multiply by 1000 for ms)
// - Add more addresses for other configs like GAMEHOURS, GAMEMINUTES, etc.
//
// Include EEPROM library (add to top of main sketch if not already)
#include <EEPROM.h>

// Define magic number for validation (16-bit to reduce collision chance)
#define EEPROM_MAGIC_NUMBER 0xBAD0

// EEPROM address definitions (for clarity and easy adjustment)
#define EEPROM_ADDR_MAGIC   0  // 2 bytes: Magic number
#define EEPROM_ADDR_INIT    2  // 1 byte: Initialization flag (1 = initialized)
#define EEPROM_ADDR_RELAY_TIME 3  // 2 bytes: RELAY_TIME in seconds

/**
 * getConfig() - Loads configuration from EEPROM, initializes with defaults if necessary.
 * This function checks for the magic number and init flag to verify if EEPROM contains valid data.
 * If not, it writes default values and sets the flags. Then, it reads and applies the stored configs
 * to global variables (e.g., RELAY_TIME).
 * 
 * Called from: setup() to load configs at startup.
 * Assumptions: Global variables like RELAY_TIME are defined elsewhere.
 * Note: EEPROM has limited write cycles (~100,000), so avoid frequent writes.
 */
void getConfig() {
  // Read magic number (16-bit)
  uint16_t storedMagic = (EEPROM.read(EEPROM_ADDR_MAGIC) << 8) | EEPROM.read(EEPROM_ADDR_MAGIC + 1);
  
  // Check if initialized (magic matches and init flag is set)
  if (storedMagic != EEPROM_MAGIC_NUMBER || EEPROM.read(EEPROM_ADDR_INIT) != 1) {
    // Not initialized: Write default values
    writeDefaultConfig();
  }
  
  // Read configurations (example for RELAY_TIME)
  // RELAY_TIME stored as seconds (16-bit), convert to ms
  uint16_t relaySeconds = (EEPROM.read(EEPROM_ADDR_RELAY_TIME) << 8) | EEPROM.read(EEPROM_ADDR_RELAY_TIME + 1);
  RELAY_TIME = relaySeconds * 1000UL;  // Use unsigned long to avoid overflow
  
  // Add reads for other variables here, e.g.:
  // GAMEHOURS = EEPROM.read(EEPROM_ADDR_GAME_HOURS);
  // GAMEMINUTES = EEPROM.read(EEPROM_ADDR_GAME_MINUTES);
  // etc.
}

/**
 * writeDefaultConfig() - Writes default configuration values to EEPROM.
 * Sets the magic number, init flag, and default values for configs.
 * This is called only when EEPROM is uninitialized.
 * 
 * Update defaults here to match initial global variable values.
 */
void writeDefaultConfig() {
  // Write magic number (16-bit)
  EEPROM.write(EEPROM_ADDR_MAGIC, (EEPROM_MAGIC_NUMBER >> 8) & 0xFF);  // High byte
  EEPROM.write(EEPROM_ADDR_MAGIC + 1, EEPROM_MAGIC_NUMBER & 0xFF);    // Low byte
  
  // Write init flag
  EEPROM.write(EEPROM_ADDR_INIT, 1);
  
  // Write default RELAY_TIME (5000 ms = 5 seconds)
  uint16_t defaultRelaySeconds = 5;
  EEPROM.write(EEPROM_ADDR_RELAY_TIME, (defaultRelaySeconds >> 8) & 0xFF);  // High byte
  EEPROM.write(EEPROM_ADDR_RELAY_TIME + 1, defaultRelaySeconds & 0xFF);     // Low byte
  
  // Add writes for other defaults, e.g.:
  // EEPROM.write(EEPROM_ADDR_GAME_HOURS, 0);
  // EEPROM.write(EEPROM_ADDR_GAME_MINUTES, 45);
  // etc.
  
  // Commit changes (on some Arduino boards, EEPROM.commit() may be needed; check docs)
  // For AVR-based Arduinos (e.g., Uno), writes are automatic.
}
