/*
  N32B sysEx Firmware v30.0.0
  MIT License

  Copyright (c) 2022 SHIK
*/

#include "storage.h"

// Check device version
bool isEEPROMvalid()
{
  for (uint8_t i = 3; i > 0; i--)
  {
    uint8_t byte = EEPROM.read(EEPROM.length() - i);
    uint8_t versionDigit = pgm_read_word_near(firmwareVersion + i - 1);
    if (!(byte == versionDigit))
      return false;
  }
  return true;
}

void formatFactory()
{
  // Set last used preset to number 0
  EEPROM.update(lastUsedPresetAddress, 0);

  // Create default preset
  Preset_t defaultPreset;
  // defaultPreset.channel = 1;

  for (uint8_t i = 0; i < NUMBER_OF_KNOBS; i++)
  {
    uint8_t indexId = pgm_read_word_near(knobsLocation + i);
    std::fill_n(defaultPreset.knobInfo[indexId].sysExData, 16, 0);
  }

  // TEMP
  // Manufacturer ID // Channel // Function no. // Group no. // Machine ID // Sub status (parameter 1-39)
  defaultPreset.knobInfo[15].messageSize = 6;
  defaultPreset.knobInfo[15].valuesIndex = 6;
  defaultPreset.knobInfo[15].MSBFirst = true;
  defaultPreset.knobInfo[15].sysExData[0] = 0x40; // Manufacturer
  defaultPreset.knobInfo[15].sysExData[1] = 0x0B; // Channel
  defaultPreset.knobInfo[15].sysExData[2] = 0x10; // Function no.
  defaultPreset.knobInfo[15].sysExData[3] = 0x00; // Group no.
  defaultPreset.knobInfo[15].sysExData[4] = 0x01; // Machine ID
  defaultPreset.knobInfo[15].sysExData[5] = 0x01; // Sub status (parameter 1-39)

  defaultPreset.knobInfo[12].messageSize = 5;
  defaultPreset.knobInfo[12].valuesIndex = 5;
  defaultPreset.knobInfo[12].MSBFirst = false;
  defaultPreset.knobInfo[12].sysExData[0] = 0x43; // ID (Manufacturer)
  defaultPreset.knobInfo[12].sysExData[1] = 0x75; // Sub stutus
  defaultPreset.knobInfo[12].sysExData[2] = 0x01; // System No.
  defaultPreset.knobInfo[12].sysExData[3] = 0x01; // Instrument No.
  defaultPreset.knobInfo[12].sysExData[4] = 0x48; // Parameter (40H - 7EH)


  // Write the default preset to all preset slots
  uint8_t baseAddress = 1;
  for (uint8_t p = 0; p < NUMBER_OF_PRESETS; p++)
  {
    for (uint16_t byteIndex = 0; byteIndex < sizeof(Preset_t); byteIndex++)
    {
      EEPROM.update(baseAddress + p * sizeof(Preset_t) + byteIndex, ((uint8_t *)(&defaultPreset))[byteIndex]);
    }
  }

  // we write the signature so that the device will never rewrite the factory presets
  for (uint8_t i = 3; i > 0; i--)
  {
    uint8_t versionDigit = pgm_read_word_near(firmwareVersion + i - 1);
    EEPROM.update(EEPROM.length() - i, versionDigit);
  }
}

// Loads the specified preset in the RAM and make it the last used preset
void loadPreset(uint8_t presetNumber)
{
  if (presetNumber < NUMBER_OF_PRESETS)
  {
    uint16_t baseAddress = 1 + (presetNumber * sizeof(Preset_t));
    // Read the active preset from EEPROM
    for (uint16_t byteIndex = 0; byteIndex < sizeof(Preset_t); byteIndex++)
    {
      ((uint8_t *)(&activePreset))[byteIndex] = EEPROM.read(baseAddress + byteIndex);
    }

    // Update the last used preset
    currentPresetNumber = presetNumber;
    n32b_display.showPresetNumber(presetNumber);

    // Save current preset as the active preset.
    // Is commented out to prevent EEPROM from over used
    // EEPROM.update(lastUsedPresetAddress, currentPresetNumber);
  }
}

void savePreset(uint8_t presetNumber)
{
  if (presetNumber < NUMBER_OF_PRESETS)
  {
    uint16_t baseAddress = presetNumber * sizeof(Preset_t) + 1;

    // write the active preset to EEPROM
    for (uint16_t byteIndex = 0; byteIndex < sizeof(Preset_t); byteIndex++)
    {
      EEPROM.update(baseAddress + byteIndex, ((uint8_t *)(&activePreset))[byteIndex]);
    }

    n32b_display.showSaveMessage();
  }
}
