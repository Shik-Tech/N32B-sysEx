/*
  N32B sysEx Firmware v30.0.0
  MIT License

  Copyright (c) 2022 SHIK
*/

#ifndef FUNCTIONS_h
#define FUNCTIONS_h

#include <Arduino.h>
#include <USB-MIDI.h>
// #include <bitset>

#include "definitions.h"
#include "storage.h"

USING_NAMESPACE_MIDI;

void onUsbMessage(const midi::Message<128> &);
void onSerialMessage(const midi::Message<128> &);

void updateKnob(uint8_t);

void sendSysEx(const struct Knob_t &, uint8_t, uint8_t);

// void changeChannel(bool);
// void changePreset(bool);

// void buttonReleaseAction(bool);
// void buttonPressAction(bool);
// void renderButtonFunctions();

void doMidiRead();
// typedef std::bitset<sizeof(int)> IntBits;

#endif