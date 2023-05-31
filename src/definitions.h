/*
  N32B sysEx Firmware v30.0.0
  MIT License

  Copyright (c) 2022 SHIK
*/

/*
 * Definitions of constats
 */

#ifndef N32B_DEFINITIONS
#define N32B_DEFINITIONS

#include <Arduino.h>
#include <USB-MIDI.h>
#include <ezButton.h>
#include <ArduinoSTL.h>

#include "mux_factory.h"
#include "display.h"

USING_NAMESPACE_MIDI;

const uint8_t firmwareVersion[] PROGMEM = {30, 1, 1};

extern MidiInterface<USBMIDI_NAMESPACE::usbMidiTransport> MIDICoreUSB;
extern MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>> MIDICoreSerial;
extern MUX_FACTORY muxFactory;
extern N32B_DISPLAY n32b_display;
// extern ezButton buttonA;
// extern ezButton buttonB;

/* Pin setup */
enum PINS
{
  MUX_A_SIG = 8,
  MUX_B_SIG = 9,
  MIDI_TX_PIN = 1,
  MUX_S0 = 2,
  MUX_S1 = 3,
  MUX_S2 = 4,
  MUX_S3 = 5,
  LED_PIN = 17,
  DIN = 16,
  CS = 10,
  CLK = 15,
  BUTTON_A_PIN = A3,
  BUTTON_B_PIN = A2
};

enum COMMANDS_INDEXS
{
  MANUFACTURER_INDEX = 1,
  COMMAND_INDEX = 2,
  KNOB_INDEX = 3,
  MSBFIRST_INDEX = 4,
  VALUES_INDEX = 5,
  MIN_VALUE_INDEX = 6, // 6 MSB, 7 LSB
  MAX_VALUE_INDEX = 8, // 8 MSB, 9 LSB
  IS_SIGNED_INDEX = 10,
  START_SYSEX_INDEX = 11,
  SYSEX_LENGTH_INDEX = 12,
  SYSEX_MESSAGE = 13,
};

enum COMMANDS
{
  SET_KNOB_MODE = 1,         // Define knob mode (see KNOB_MODES)
  SAVE_PRESET = 2,           // Save the preset
  LOAD_PRESET = 3,           // Load a preset
  SEND_FIRMWARE_VERSION = 4, // Send the device firmware version
  SYNC_KNOBS = 5,            // Send active preset
  // CHANGE_CHANNEL = 6,        // Changes the global MIDI channel
  START_SYSEX_MESSAGE = 7, // Indicates start of sysEx message to store
  SET_THRU_MODE = 8,       // Set the midi THRU behavior
  SET_OUTPUT_MODE = 9,     // Set the midi OUTPUT behavior
  END_OF_TRANSMISSION = 99 // Notify end of transmission
};

enum KNOB_MODES
{
  KNOB_MODE_DISABLE = 0,
  KNOB_SYSEX = 6
};

// General definitions
enum DEFINITIONS
{
  SHIK_MANUFACTURER_ID = 32,
  NUMBER_OF_KNOBS = 32,
  NUMBER_OF_PRESETS = 1 // TODO: Change to something else
};

enum THRU_MODES
{
  THRU_OFF = 0,
  THRU_TRS_TRS = 1,
  THRU_TRS_USB = 2,
  THRU_USB_USB = 3,
  THRU_USB_TRS = 4,
  THRU_BOTH_DIRECTIONS = 5
};

enum OUTPUT_MODES
{
  OUTPUT_TRS = 0,
  OUTPUT_USB = 1,
  OUTPUT_BOTH = 2
};

// Knob settings structure
struct Knob_t
{
  uint8_t messageSize = 0;
  uint8_t valuesIndex = 0;
  bool MSBFirst = true;
  byte sysExData[12] = {0};
  uint8_t minValue = 0;
  uint8_t maxValue = 255;
  bool isSigned = true;
};

// A preset struct is defining the device preset structure
struct Preset_t
{
  Knob_t knobInfo[32];
  uint8_t thruMode;
  uint8_t outputMode;
};

// A device struct is defining the device structure
struct Device_t
{
  Preset_t activePreset;
  uint8_t knobValues[32][4]{0};
};

/* Device setup data */
extern Device_t device;
// extern byte currentPresetNumber;
// extern Preset_t activePreset;
// extern uint8_t knobValues[32][4];
// extern float EMA_a; // EMA alpha

/* Buttons variables */
extern const unsigned int reset_timeout; // Reset to factory preset timeout
// extern const uint8_t SHORT_PRESS_TIME;   // Milliseconds
// extern unsigned long pressedTime;
// extern bool isPressingAButton;
// extern bool isPressingBButton;

/* Mode variables */
// extern bool isPresetMode;

// byte index in EEPROM for the last used preset
extern uint8_t lastUsedPresetAddress;

#endif