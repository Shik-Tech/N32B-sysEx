/*
  N32B sysEx Firmware v30.0.0
  MIT License

  Copyright (c) 2022 SHIK
*/

#include "sysex.h"

void processSysex(unsigned char *data, unsigned int size)
{
    if (size > 3 && data[MANUFACTURER_INDEX] == SHIK_MANUFACTURER_ID)
    {
        switch (data[COMMAND_INDEX])
        {
        case SET_KNOB_MODE:
            if (data[START_SYSEX_INDEX] == START_SYSEX_MESSAGE)
            {
                for (uint8_t i = 0; i < data[SYSEX_LENGTH_INDEX]; i++)
                {
                    device.activePreset.knobInfo[data[KNOB_INDEX]].sysExData[i] = data[SYSEX_MESSAGE + i];
                }
            }

            device.activePreset.knobInfo[data[KNOB_INDEX]].valuesIndex = data[VALUES_INDEX];
            device.activePreset.knobInfo[data[KNOB_INDEX]].MSBFirst = data[MSBFIRST_INDEX];
            device.activePreset.knobInfo[data[KNOB_INDEX]].minValue = data[MIN_VALUE_INDEX] << 4 | data[MIN_VALUE_INDEX + 1];
            device.activePreset.knobInfo[data[KNOB_INDEX]].maxValue = data[MAX_VALUE_INDEX] << 4 | data[MAX_VALUE_INDEX + 1];
            device.activePreset.knobInfo[data[KNOB_INDEX]].isSigned = data[IS_SIGNED_INDEX];
            device.activePreset.knobInfo[data[KNOB_INDEX]].messageSize = data[SYSEX_LENGTH_INDEX];

            // byte testSysExEncoded[10];
            // midi::encodeSysEx(&data[SYSEX_INDEX], testSysExEncoded, sizeof(data[SYSEX_INDEX]));
            // for (uint8_t i = 0; i < 10; i++){
            //     Serial.print(testSysExEncoded[i]);
            //     Serial.print("");
            // }

            // midi::encodeSysEx(&data[SYSEX_INDEX], activePreset.knobInfo[data[KNOB_INDEX]].sysExData, sizeof(data[SYSEX_INDEX]));

            /*
             * TODO: handle Korg special case:
             * https://github.com/FortySevenEffects/arduino_midi_library/blob/master/doc/sysex-codec.md
             */
            // void handleSysEx(byte * inData, unsigned inSize)
            // {
            //     // SysEx body data starts at 3rd byte: F0 42 aa bb cc dd F7
            //     // 42 being the hex value of the Korg SysEx ID.
            //     const unsigned dataStartOffset = 2;
            //     const unsigned encodedDataLength = inSize - 3; // Remove F0 42 & F7

            //     // Create a large enough buffer where to decode the message
            //     byte decodedData[64];

            //     const unsigned decodedSize = decodeSysEx(inData + dataStartOffset,
            //                                              decodedData,
            //                                              encodedDataLength,
            //                                              true); // flip header bits
            //     // Do stuff with your message
            // }
            break;
        case SET_THRU_MODE:
            setMidiThruMode(data[KNOB_INDEX]);
            break;
        case SET_OUTPUT_MODE:
            setMidiOutputMode(data[KNOB_INDEX]);
            break;
        case SAVE_PRESET:
            savePreset(data[KNOB_INDEX]);
            break;
        case LOAD_PRESET:
            loadPreset(data[KNOB_INDEX]);
            break;
        // case CHANGE_CHANNEL:
        //     handleChangeChannel(data[KNOB_INDEX]);
        //     break;
        case SEND_FIRMWARE_VERSION:
            sendDeviceFirmwareVersion();
            break;
        case SYNC_KNOBS:
            sendActivePreset();
            break;
        default:
            break;
        }
    }
    else
    {
        // Serial.println("Manufacturer ID is not matching the N32B");
    }
}

// void handleChangeChannel(byte channel)
// {
//     if (channel < 17 && channel > 0)
//     {
//         activePreset.channel = channel;
//     }
// }

// Change preset on program change
void handleProgramChange(byte channel, byte number)
{
    if (number < NUMBER_OF_PRESETS)
    {
        loadPreset(number);
    }
}

void sendDeviceFirmwareVersion()
{
    uint8_t data[5] = {SHIK_MANUFACTURER_ID, SEND_FIRMWARE_VERSION};

    // Send firmware version
    for (uint8_t i = 3; i > 0; i--)
    {
        data[i + 1] = EEPROM.read(EEPROM.length() - i);
    }

    MIDICoreUSB.sendSysEx(5, data);
}
void sendActivePreset()
{
    byte *sysExMessage;
    // Send current preset
    for (uint8_t i = 0; i < NUMBER_OF_KNOBS; i++)
    {
        uint8_t indexId = pgm_read_word_near(knobsLocation + i);
        uint8_t minValueMSB = device.activePreset.knobInfo[indexId].minValue >> 4;
        uint8_t minValueLSB = device.activePreset.knobInfo[indexId].minValue & 0x0F;
        uint8_t maxValueMSB = device.activePreset.knobInfo[indexId].maxValue >> 4;
        uint8_t maxValueLSB = device.activePreset.knobInfo[indexId].maxValue & 0x0F;

        std::vector<uint8_t> knobsData = {
            SHIK_MANUFACTURER_ID,
            SYNC_KNOBS,
            indexId,
            device.activePreset.knobInfo[indexId].MSBFirst,
            device.activePreset.knobInfo[indexId].valuesIndex,
            minValueMSB,
            minValueLSB,
            maxValueMSB,
            maxValueLSB,
            device.activePreset.knobInfo[indexId].isSigned,
            device.activePreset.knobInfo[indexId].messageSize};

        for (uint8_t byteIndex = 0; byteIndex < device.activePreset.knobInfo[indexId].messageSize; byteIndex++)
        {
            knobsData.push_back(device.activePreset.knobInfo[indexId].sysExData[byteIndex]);
        }

        sysExMessage = &knobsData[0];
        MIDICoreUSB.sendSysEx(knobsData.size(), sysExMessage);
    }
    std::vector<uint8_t> presetData = {
        SHIK_MANUFACTURER_ID,
        SET_THRU_MODE,
        device.activePreset.thruMode};

    sysExMessage = &presetData[0];
    MIDICoreUSB.sendSysEx(presetData.size(), sysExMessage);
}
void setMidiThruMode(byte mode)
{
    device.activePreset.thruMode = mode;
}
void setMidiOutputMode(byte mode)
{
    device.activePreset.outputMode = mode;
}