/*
  N32B sysEx Firmware v30.0.0
  MIT License

  Copyright (c) 2022 SHIK
*/

#include "functions.h"

void onSerialMessage(const midi::Message<128> &message)
{
  if (MIDICoreSerial.getType() != midi::MidiType::ActiveSensing)
  {
    n32b_display.blinkDot(2);
  }
}

void updateKnob(uint8_t index)
{
  Knob_t &currentKnob = activePreset.knobInfo[index];
  bool needToUpdate = false;
  uint16_t shiftedValue;
  uint8_t MSBValue;
  uint8_t LSBValue;

  if (
      (knobValues[index][0] != knobValues[index][1]) &&
      (knobValues[index][0] != knobValues[index][2]) &&
      (knobValues[index][0] != knobValues[index][3]))
  {
    needToUpdate = true;
    shiftedValue = map(knobValues[index][0], 0, 1019, 0, 16383);
    MSBValue = shiftedValue >> 7;
    LSBValue = lowByte(shiftedValue) >> 1;
  }

  if (needToUpdate)
  {
    sendSysEx(currentKnob, MSBValue, LSBValue);

    knobValues[index][3] = knobValues[index][2];
    knobValues[index][2] = knobValues[index][1];
    knobValues[index][1] = knobValues[index][0];
  }
}

void sendSysEx(const struct Knob_t &currentKnob, uint8_t MSBvalue, uint8_t LSBvalue)
{
  if (currentKnob.messageSize == 0) return;

  std::vector<byte> sysExMessageWithValues;
  for (uint8_t i = 0; i < currentKnob.messageSize; i++)
  {
    sysExMessageWithValues.push_back(currentKnob.sysExData[i]);
  }
  sysExMessageWithValues.push_back(MSBvalue);
  sysExMessageWithValues.push_back(LSBvalue);

  // for (uint8_t i = 0; i < sysExMessageWithValues.size(); i++)
  // {
  //   Serial.print(sysExMessageWithValues[i], HEX);
  //   Serial.print(" ");
  // }

  byte *sysExMessage = &sysExMessageWithValues[0];

  MIDICoreSerial.sendSysEx(currentKnob.messageSize + 2, sysExMessage);
}

void changeChannel(bool direction)
{
  if (direction)
  {
    // Next Channel
    if (activePreset.channel < 16)
      activePreset.channel++;
    else
      activePreset.channel = 1;
  }
  else
  {
    // Previous Channel
    if (activePreset.channel > 1)
      activePreset.channel--;
    else
      activePreset.channel = 16;
  }
}

void changePreset(bool direction)
{
  if (direction)
  {
    // Next Preset
    if (currentPresetNumber < NUMBER_OF_PRESETS - 1)
      loadPreset(currentPresetNumber + 1);
    else
      loadPreset(0);
  }
  else
  {
    // Previous Preset
    if (currentPresetNumber > 0)
      loadPreset(currentPresetNumber - 1);
    else
      loadPreset(NUMBER_OF_PRESETS - 1);
  }
  // MIDICoreSerial.sendProgramChange(currentPresetNumber, 1);
}

void buttonReleaseAction(bool direction)
{
  direction ? isPressingAButton = false : isPressingBButton = false;

  if (millis() - pressedTime < SHORT_PRESS_TIME)
  {
    if (isPresetMode)
    {
      changePreset(direction);
      n32b_display.showPresetNumber(currentPresetNumber);
    }
    else
    {
      changeChannel(direction);
      n32b_display.showChannelNumber(activePreset.channel);
    }
  }

  MIDICoreSerial.turnThruOn();
}

void buttonPressAction(bool direction)
{
  pressedTime = millis();
  MIDICoreSerial.turnThruOff();
}

void renderButtonFunctions()
{
  // Must call the loop() function first
  buttonA.loop();
  buttonB.loop();

  if (buttonA.isPressed())
  {
    isPressingAButton = true;
    buttonPressAction(1);
  }

  if (buttonB.isPressed())
  {
    isPressingBButton = true;
    buttonPressAction(0);
  }

  if (buttonA.isReleased())
  {
    buttonReleaseAction(1);
  }

  if (buttonB.isReleased())
  {
    buttonReleaseAction(0);
  }

  // Switch between channelMode and presetMode
  if (
      (isPressingAButton || isPressingBButton) &&
      (millis() - pressedTime > (unsigned int)(SHORT_PRESS_TIME << 2)))
  {
    if (isPressingAButton)
    {
      isPresetMode = false;
      n32b_display.showChannelNumber(activePreset.channel);
    }
    if (isPressingBButton)
    {
      isPresetMode = true;
      n32b_display.showPresetNumber(currentPresetNumber);
    }
  }
}

void doMidiRead()
{
  MIDICoreSerial.read();
}
