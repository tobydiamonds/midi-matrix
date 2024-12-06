#ifndef Output_h
#define Output_h

#define OUTPUTS 8

#include <Arduino.h>
#include "MidiDefs.h"
#include "MidiMessage.h"

struct Output
{
    unsigned char Channel;
    bool IsEnabled;
    unsigned char InputChannel;

    Output(unsigned char channel, unsigned char inputChannel)
    {
        Channel = channel;
        IsEnabled = true;
        InputChannel = inputChannel;
    }
};
Output *outputs[OUTPUTS];

void InitOutputs()
{
    for(unsigned char o=0; o<OUTPUTS; o++)
    {
        outputs[o] = new Output(o,o);
        outputs[o]->IsEnabled = false;
    }
}

void PlayMessage(Output *output, unsigned char type, unsigned char data1, unsigned char data2)
{
    if (output->IsEnabled || type == 0x80) // always send not-off messages
    {
        unsigned char status = GetStatus(type, output->Channel);
        unsigned char messageLength = ExpectedMessageLength(type);

        // char buf[80];
        // sprintf(buf, "output: %d  enabled: %d  status: 0x%02X  data1: 0x%02X  data2: 0x%02X", output->Channel, output->IsEnabled, status, data1, data2);
        // Serial.println(buf);

        Serial1.write(status);
        // Serial1.write(data1);
        // Serial1.write(data2);

        if (messageLength >= 1)
        {
            Serial1.write(data1);
        }
        if (messageLength >= 2)
        {
            Serial1.write(data2);
        }
    }
}

void PlayMessage(Output *output, MidiMessage *message)
{
    PlayMessage(output, message->Type, message->Data1, message->Data2);
}


#endif