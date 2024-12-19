#ifndef Clip_h
#define Clip_h

#include <Arduino.h>
#include <SRAMsimple.h>
#include "MidiDefs.h"
#include "MidiMessage.h"
#include "Output.h"

#define CLIP_LOOPING 0b00010000
#define CLIP_RECORDING 0b00001000
#define CLIP_ARMED 0b00000100
#define CLIP_PLAYING 0b00000010
#define CLIP_SELECTED 0b00000001 
#define CLIP_NOT_SELECTED 0

#define CSPIN 10
SRAMsimple sram;

class Clip
{
private:
  //MidiMessage* midiMessages[CLIP_MAX_MESSAGES];
  uint16_t _hasMessages = 0;

  uint32_t GetMessageAddress(int position)
  {
      // each clip allocates CLIP_MAX_MESSAGES x 3 bytes for messages
      // with CLIP_MAX_MESSAGES = 16
      // 0.0 => offset = (0x8+0)x48 =    0
      // 0.1 => offset = (0x8+1)x48 =   48
      // 0.2 => offset = (0x8+2)x48 =   96
      // 1.0 => offset = (1x8+0)x48 =  384
      // 1.1 => offset = (1x8+1)x48 =  432
      // 2.0 => offset = (2x8+0)x48 =  768
      // 7.7 => offset = (7x8+7)x48 = 3024

      uint32_t offset = (this->TrackIdx*8+this->ClipIdx) * CLIP_MAX_MESSAGES * 3;
      return offset + position * 3;    
  }

  void SaveMessageToSRAM(int position, uint8_t type, uint8_t data1, uint8_t data2)
  {
      uint32_t address = GetMessageAddress(position);
      uint8_t buffer[3] {type, data1, data2};

      //Serial.print("Save message to SRAM [address::"); Serial.print(address); Serial.print("] : ");Serial.print(buffer[0]); Serial.print(" ");Serial.print(buffer[1]);Serial.print(" ");Serial.println(buffer[2]);

      sram.WriteByteArray(address, buffer, 3);
  }

  void LoadMessageFromSRAM(int position, uint8_t* buffer, size_t& size)
  {
      buffer[0] = 0;
      buffer[1] = 0;
      buffer[2] = 0;
      uint32_t address = GetMessageAddress(position);
      sram.ReadByteArray(address, buffer, 3);
      size = 3;

      if(buffer[0]<0x80) // correct
      {
        /*
Before Correction (Invalid Bytes):
Byte	Binary	Comment
Byte 1	01000000	Left-shifted from 10010000 (MSB: 1 lost)
Byte 2	10010000	Left-shifted from 00100100 (MSB: 0 lost)
Byte 3	11111100	Left-shifted from 01111111 (MSB: 0 lost)
After Correction (Valid Bytes):
Byte	Binary	Explanation
Byte 1	10010000	Reconstructed from 01000000 and 1 from Byte 2
Byte 2	00100100	Reconstructed from 10010000 and 0 from Byte 3
Byte 3	01111111	Reconstructed from 11111100 and trailing 0
*/
        buffer[0] = (buffer[0] >> 1) | 0x80; // shift right and set MSB
        buffer[1] = buffer[1] >> 1;
        buffer[2] = buffer[2] >> 1;
      }

      //Serial.print("Load message from SRAM [address::"); Serial.print(address); Serial.print("] : ");Serial.print(buffer[0]); Serial.print(" ");Serial.print(buffer[1]);Serial.print(" ");Serial.println(buffer[2]);
  }

  LoadAllMessagesFromSRAM(uint8_t* buffer, size_t& size)
  {
      uint32_t address = GetMessageAddress(0);
      sram.ReadByteArray(address, buffer, CLIP_MAX_MESSAGES*3);
      size = CLIP_MAX_MESSAGES*3;
  }

public:
    uint8_t TrackIdx;
    uint8_t ClipIdx;
    unsigned int TempoFactor;
    uint8_t Status; // x x x Loop Recording Armed Playing Selected
    unsigned int LastPosition;
    uint8_t CurrentBar;
    bool IsEndOfClip;
    bool OneShot;

    Clip(uint8_t trackidx, uint8_t clipidx)
    {
        TrackIdx = trackidx;
        ClipIdx = clipidx;
        TempoFactor = 100; // 100%
        Status = CLIP_NOT_SELECTED;
        CurrentBar = 0;
        IsEndOfClip = false;
        OneShot = true;
        LastPosition = (CLIP_MAX_BARS * ONE_BAR)-1; // 4 bars with 96 positions each

        // Initialize the array
        for (int i = 0; i < CLIP_MAX_MESSAGES; i++) {
          //midiMessages[i] = nullptr;

          // initialise sram
          this->SaveMessageToSRAM(i, 0,0,0);
        }        

        
    }

    bool Selected() { return Status & CLIP_SELECTED; }
    bool Playing() { return Status & CLIP_PLAYING; }
    bool Armed() { return Status & CLIP_ARMED; }
    bool Recording() { return Status & CLIP_RECORDING; }
    bool Loop() { return Status & CLIP_LOOPING; }

    bool EndOfClip() { return IsEndOfClip; }


    bool AddMessage(int position, uint8_t type, uint8_t data1, uint8_t data2)
    {
      if (position < 0 || position >= CLIP_MAX_MESSAGES) {
        Serial.println("Invalid position!");
        return;
      }

      // Allocate memory for the message and store it
      //midiMessages[position] = new MidiMessage(type, data1, data2);

      // store message in SRAM
      this->SaveMessageToSRAM(position, type, data1, data2);

      _hasMessages |= (1 << position);
    }

    //MidiMessage* GetMessage(int position)
    void GetMessage(int position, uint8_t &type, uint8_t &data1, uint8_t &data2)
    {
      if (position < 0 || position >= CLIP_MAX_MESSAGES || !this->HasMessage(position)) {
        //return nullptr;
        type=0;
        data1=0;
        data2=0;
        return;
      }
      uint8_t buffer[3] {0};
      size_t size = 0;
      this->LoadMessageFromSRAM(position, buffer, size);
      if(size<2 || buffer[0]<0x80) //return nullptr;
      {
        type=0;
        data1=0;
        data2=0;
        return;
      }
      type=buffer[0];
      data1=buffer[1];
      data2=buffer[2];

      //return new MidiMessage(buffer[0], buffer[1], buffer[2]);
      //return midiMessages[position];      
    }

    void RemoveMessages()
    {
      for(int i=0; i<CLIP_MAX_MESSAGES; i++)
      {
        /*if (midiMessages[i] != nullptr) {
          delete midiMessages[i];
          midiMessages[i] = nullptr;
        } */       
        this->SaveMessageToSRAM(i, 0,0,0);
      }
      _hasMessages = false;
    }    

    void ListMessages()
    {
      uint8_t messages[CLIP_MAX_MESSAGES*3] {0};
      size_t size=0;
      this->LoadAllMessagesFromSRAM(messages, size);
      for (int i = 0; i < CLIP_MAX_MESSAGES; i++) {
        int offset = i*3;
        if(messages[offset]!=0)
        {
          Serial.print("Position ");
          Serial.print(i);
          Serial.print(": ");
          Serial.print(messages[offset], HEX);
          Serial.print(" ");
          Serial.print(messages[offset+1]);
          Serial.print(" ");
          Serial.println(messages[offset+2]);          
        }
      }
      /*
      for (int i = 0; i < CLIP_MAX_MESSAGES; i++) {
        if (midiMessages[i] != nullptr) {
          Serial.print("Position ");
          Serial.print(i);
          Serial.print(": ");
          Serial.print(midiMessages[i]->Type, HEX);
          Serial.print(" ");
          Serial.print(midiMessages[i]->Data1);
          Serial.print(" ");
          Serial.println(midiMessages[i]->Data2);
        }
      }  
      */    
    }

    bool HasMessage(int position)
    {
      return _hasMessages & (1 << position);
    }

    bool HasMessages()
    {
      return _hasMessages > 0;
    }
/*
    void Play(unsigned int position, Output* output)
    {
      MidiMessage *message = GetMessage(position);
      if(message != nullptr) {
        PlayMessage(output, message);
      }
    }
*/
    void Play(unsigned int position)
    {
      if(!this->HasMessage(position)) return;
      //MidiMessage *message = GetMessage(position);
      uint8_t type=0;
      uint8_t data1=0;
      uint8_t data2=0;
      GetMessage(position, type, data1, data2);
      if(type == 0) return;
      //if(message != nullptr) {
      Output* output = outputs[this->TrackIdx];
      //PlayMessage(output, message);
      PlayMessage(output, type, data1, data2);
    }
};


Clip *clips[TRACKS][CLIPS_PR_TRACK];

void InitClips()
{
    for(uint8_t t=0; t<TRACKS; t++)
    {
        for(uint8_t c=0; c<CLIPS_PR_TRACK; c++)
        {  
            clips[t][c] = new Clip(t, c);
        }
    }
    //InitTrackMessages();
/*
    clips[0][0]->AddMessage(0, 0x90, 72, 127); // note on 1/1/1 (bar/beat/16th)
    clips[0][0]->AddMessage(1, 0x80, 72, 0); // note off  1/1/8

    clips[0][0]->AddMessage(4, 0x90, 60, 127);
    clips[0][0]->AddMessage(5, 0x80, 60, 0);   

    clips[0][0]->AddMessage(8, 0x90, 60, 127);
    clips[0][0]->AddMessage(9, 0x80, 60, 0);   

    clips[0][0]->AddMessage(12, 0x90, 60, 127);
    clips[0][0]->AddMessage(13, 0x80, 60, 0);     


    for(int i=0; i<=15; i++)
    {
      if(i%2==0)
      {
        clips[1][0]->AddMessage(i, 0x90, 36, 127);
        clips[1][0]->AddMessage(i+1, 0x80, 36, 0);
      }
    }

    uint8_t base = 60;
    int scaleIntervals[] = {0, 2, 4, 5, 7, 9, 11, 12};
    int scaleIntervals2[] = {7, 5, 2, 1, 1, 8, 12, 11};
    for(int i=0; i<15; i++)
    {
      if(i%2==0)
      {
        uint8_t note = base + scaleIntervals[i/2];
        clips[2][0]->AddMessage(i, 0x90, note, 127);
        clips[2][0]->AddMessage(i+1, 0x80, note, 0);

        note = base + scaleIntervals2[i/2];
        clips[2][1]->AddMessage(i, 0x90, note, 127);
        clips[2][1]->AddMessage(i+1, 0x80, note, 0);        
      }
    }*/
}

void SetClipSelected(Clip* clip) { clip->Status |= CLIP_SELECTED; }
void SetClipPlaying(Clip* clip) { clip->Status |= CLIP_PLAYING; }
void SetClipArmed(Clip* clip) { clip->Status |= CLIP_ARMED; }
void SetClipRecording(Clip* clip) { clip->Status |= CLIP_RECORDING; }
void SetClipLoop(Clip* clip) { clip->Status |= CLIP_LOOPING; }
void ResetClipSelected(Clip* clip) { clip->Status &= ~CLIP_SELECTED; }
void ResetClipPlaying(Clip* clip) { clip->Status &= ~CLIP_PLAYING; }
void ResetClipArmed(Clip* clip) { clip->Status &= ~CLIP_ARMED; }
void ResetClipRecording(Clip* clip) { clip->Status &= ~CLIP_RECORDING; }
void ResetClipLoop(Clip* clip) { clip->Status &= ~CLIP_LOOPING; }


void ClipChangeTempoFactor(Clip *clip, int factor)
{
    // set the value
    clip->TempoFactor = factor;

    // re-arrange data in the message array
}



#endif