/*
  N32B sysEx Firmware v30.0.0
  MIT License

  Copyright (c) 2022 SHIK
*/

#include "functions.h"

void onUsbMessage(const midi::Message<128> &message)
{
  MIDICoreSerial.send(message);
  // n32b_display.blinkDot(2);
}

void onSerialMessage(const midi::Message<128> &message)
{
 if (MIDICoreSerial.getType() != midi::MidiType::ActiveSensing)
  {
    MIDICoreUSB.send(message.type, message.data1, message.data2, message.channel);
    // n32b_display.blinkDot(2);
  }
}

void updateKnob(uint8_t index)
{
  Knob_t &currentKnob = activePreset.knobInfo[index];
  int8_t shiftedValue;
  uint8_t outputShifted;
  uint8_t MSBValue;
  uint8_t LSBValue;

  if (
      (knobValues[index][0] != knobValues[index][1]) &&
      (knobValues[index][0] != knobValues[index][2]) &&
      (knobValues[index][0] != knobValues[index][3]))
  {
    if (currentKnob.isSigned)
    {
      shiftedValue = map(knobValues[index][0], 0, 255, -currentKnob.maxValue, currentKnob.maxValue);
      outputShifted = shiftedValue < 0 ? abs(shiftedValue) | 128 : shiftedValue;
    }
    else
    {
      outputShifted = map(knobValues[index][0], 0, 255, currentKnob.minValue, currentKnob.maxValue);
    }
    MSBValue = outputShifted >> 4;
    LSBValue = outputShifted & 0x0F;

    sendSysEx(currentKnob, MSBValue, LSBValue);

    knobValues[index][3] = knobValues[index][2];
    knobValues[index][2] = knobValues[index][1];
    knobValues[index][1] = knobValues[index][0];
  }
}

void sendSysEx(const struct Knob_t &currentKnob, uint8_t MSBvalue, uint8_t LSBvalue)
{
  // byte testSysExEncoded[10];
  // byte testSysExDecoded[10];
  // const byte lengthDecoded = midi::decodeSysEx(currentKnob.sysExData, testSysExDecoded, 10);
  // const byte lengthEncoded = midi::encodeSysEx(testSysExDecoded, testSysExEncoded, lengthDecoded);

  // Serial.println(lengthDecoded);
  // Serial.println(lengthEncoded);

  // for (uint8_t i = 0; i < 10; i++)
  // {
  //   Serial.print(currentKnob.sysExData[i], HEX);
  //   Serial.print(" ");
  // }
  // Serial.println("");

  // for (uint8_t i = 0; i < 10; i++)
  // {
  //   Serial.print(testSysExDecoded[i]);
  //   Serial.print(" ");
  // }
  // Serial.println("");

  // for (uint8_t i = 0; i < 10; i++)
  // {
  //   Serial.print(testSysExEncoded[i], HEX);
  //   Serial.print(" ");
  // }

  // Serial.println("");
  // Serial.println("-----------");

  if (currentKnob.messageSize == 0)
    return;

  std::vector<byte> sysExMessageWithValues;

  for (uint8_t index = 0; index < currentKnob.messageSize + 2; index++)
  {
    if (index == currentKnob.valuesIndex)
    {
      sysExMessageWithValues.push_back(currentKnob.MSBFirst ? MSBvalue : LSBvalue);
      sysExMessageWithValues.push_back(currentKnob.MSBFirst ? LSBvalue : MSBvalue);
      // sysExMessageWithValues.push_back(MSBvalue);
    }
    if (index != currentKnob.messageSize)
    {
      sysExMessageWithValues.push_back(currentKnob.sysExData[index]);
    }
  }

  // for (uint8_t i = 0; i < sysExMessageWithValues.size(); i++)
  // {
  //   Serial.print(sysExMessageWithValues[i], HEX);
  //   Serial.print(" ");
  // }
  // Serial.println("");

  byte *sysExMessage = &sysExMessageWithValues[0];

  MIDICoreSerial.sendSysEx(currentKnob.messageSize + 2, sysExMessage);
  MIDICoreUSB.sendSysEx(currentKnob.messageSize + 2, sysExMessage);
}

void changeChannel(bool direction)
{
  // if (direction)
  // {
  //   // Next Channel
  //   if (activePreset.channel < 16)
  //     activePreset.channel++;
  //   else
  //     activePreset.channel = 1;
  // }
  // else
  // {
  //   // Previous Channel
  //   if (activePreset.channel > 1)
  //     activePreset.channel--;
  //   else
  //     activePreset.channel = 16;
  // }
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
      // n32b_display.showChannelNumber(activePreset.channel);
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
      // n32b_display.showChannelNumber(activePreset.channel);
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
  MIDICoreUSB.read();
  MIDICoreSerial.read();
}