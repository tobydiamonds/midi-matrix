#ifndef Clip_h
#define Clip_h

#include <Arduino.h>
#include "MidiDefs.h"
#include "MidiMessage.h"
#include "Output.h"

#define CLIP_LOOPING 0b00010000
#define CLIP_RECORDING 0b00001000
#define CLIP_ARMED 0b00000100
#define CLIP_PLAYING 0b00000010
#define CLIP_SELECTED 0b00000001 
#define CLIP_NOT_SELECTED 0

class Clip
{
private:
  MidiMessage* midiMessages[CLIP_MAX_MESSAGES];

public:
    unsigned char TrackIdx;
    unsigned char ClipIdx;
    unsigned int TempoFactor;
    unsigned char Status; // x x x Loop Recording Armed Playing Selected
    unsigned int LastPosition;
    unsigned char CurrentBar;
    bool IsEndOfClip;
    

    Clip(unsigned char trackidx, unsigned char clipidx)
    {
        TrackIdx = trackidx;
        ClipIdx = clipidx;
        TempoFactor = 100; // 100%
        Status = CLIP_NOT_SELECTED;
        CurrentBar = 0;
        IsEndOfClip = false;
        LastPosition = (CLIP_MAX_BARS * ONE_BAR)-1; // 4 bars with 96 positions each

        // Initialize the array
        for (int i = 0; i < CLIP_MAX_MESSAGES; i++) {
          midiMessages[i] = nullptr;
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
      midiMessages[position] = new MidiMessage(type, data1, data2);
    }

    MidiMessage* GetMessage(int position)
    {
      if (position < 0 || position >= CLIP_MAX_MESSAGES) {
        return nullptr;
      }
      return midiMessages[position];      
    }

    void RemoveMessages()
    {
      for(int i=0; i<CLIP_MAX_MESSAGES; i++)
      {
        if (midiMessages[i] != nullptr) {
          delete midiMessages[i];
          midiMessages[i] = nullptr;
        }        
      }
    }    

    void ListMessages()
    {
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
    }

    bool HasMessages()
    {
      for (int i = 0; i < CLIP_MAX_MESSAGES; i++) {
        if (midiMessages[i] != nullptr)
          return true;
      }
      return false;
    }

    void Play(unsigned int position, Output* output)
    {
      MidiMessage *message = GetMessage(position);
      if(message != nullptr && (this->Playing() || message->Type == 0x80)) {
       /* Serial.print("CLIP [");
        Serial.print(TrackIdx);
        Serial.print("][");
        Serial.print(ClipIdx);
        Serial.print("] playing message: ");
        Serial.print(message->Type, HEX);
        Serial.print(message->Data1, HEX);
        Serial.println(message->Data2, HEX);*/
        PlayMessage(output, message);
      }
    }
};


Clip *clips[TRACKS][CLIPS_PR_TRACK];

void InitClips()
{
    for(unsigned char t=0; t<TRACKS; t++)
    {
        for(unsigned char c=0; c<CLIPS_PR_TRACK; c++)
        {  
            clips[t][c] = new Clip(t, c);
        }
    }
    //InitTrackMessages();

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
    }
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