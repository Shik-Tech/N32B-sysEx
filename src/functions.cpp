/*
  N32B sysEx Firmware v30.0.0
  MIT License

  Copyright (c) 2022 SHIK
*/

#include "functions.h"

void onUsbMessage(const midi::Message<128> &message)
{
  if (message.type != midi::MidiType::ActiveSensing)
  {
    switch (device.activePreset.thruMode)
    {
    case THRU_USB_USB:
      MIDICoreUSB.send(message.type, message.data1, message.data2, message.channel);
      break;

    case THRU_USB_TRS:
      MIDICoreSerial.send(message);
      break;
    case THRU_BOTH_DIRECTIONS:
      MIDICoreUSB.send(message.type, message.data1, message.data2, message.channel);
      MIDICoreSerial.send(message);
    }
    n32b_display.blinkDot(2);
  }
}

void onSerialMessage(const midi::Message<128> &message)
{
  if (message.type != midi::MidiType::ActiveSensing)
  {
    switch (device.activePreset.thruMode)
    {
    case THRU_TRS_TRS:
      MIDICoreSerial.send(message);
      break;

    case THRU_TRS_USB:
      MIDICoreUSB.send(message.type, message.data1, message.data2, message.channel);
      break;
    case THRU_BOTH_DIRECTIONS:
      MIDICoreUSB.send(message.type, message.data1, message.data2, message.channel);
      MIDICoreSerial.send(message);
    }
    n32b_display.blinkDot(2);
  }
}

void updateKnob(uint8_t index)
{
  Knob_t &currentKnob = device.activePreset.knobInfo[index];
  int8_t shiftedValue;
  uint8_t outputShifted;
  uint8_t MSBValue;
  uint8_t LSBValue;

  if (
      (device.knobValues[index][0] != device.knobValues[index][1]) &&
      (device.knobValues[index][0] != device.knobValues[index][2]) &&
      (device.knobValues[index][0] != device.knobValues[index][3]))
  {
    if (currentKnob.isSigned)
    {
      shiftedValue = map(device.knobValues[index][0], 0, 255, -currentKnob.maxValue, currentKnob.maxValue);
      outputShifted = shiftedValue < 0 ? abs(shiftedValue) | 128 : shiftedValue;
    }
    else
    {
      outputShifted = map(device.knobValues[index][0], 0, 255, currentKnob.minValue, currentKnob.maxValue);
    }
    MSBValue = outputShifted >> 4;
    LSBValue = outputShifted & 0x0F;

    sendSysEx(currentKnob, MSBValue, LSBValue);

    device.knobValues[index][3] = device.knobValues[index][2];
    device.knobValues[index][2] = device.knobValues[index][1];
    device.knobValues[index][1] = device.knobValues[index][0];
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

  byte *sysExMessage = &sysExMessageWithValues[0];

  if (device.activePreset.outputMode == OUTPUT_TRS ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreSerial.sendSysEx(currentKnob.messageSize + 2, sysExMessage);
  }
  if (device.activePreset.outputMode == OUTPUT_USB ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreUSB.sendSysEx(currentKnob.messageSize + 2, sysExMessage);
  }

  n32b_display.blinkDot(1);
}

void doMidiRead()
{
  MIDICoreUSB.read();
  MIDICoreSerial.read();
}