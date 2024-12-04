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
    }

    bool Selected() { return Status & CLIP_SELECTED; }
    bool Playing() { return Status & CLIP_PLAYING; }
    bool Armed() { return Status & CLIP_ARMED; }
    bool Recording() { return Status & CLIP_RECORDING; }
    bool Loop() { return Status & CLIP_LOOPING; }

    bool EndOfClip() { return IsEndOfClip; }


    bool AddMessage(unsigned char position, unsigned char type, unsigned char data1, unsigned char data2)
    {

    }

    void RemoveMessages()
    {

    }    

    void ListMessages()
    {
    }

    void Play(unsigned int position, Output* output)
    {
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


void ListClipMessages(Clip *clip)
{
    clip->ListMessages();
}

void AddMessageToClip(unsigned char position, Clip *clip, unsigned char type, unsigned char data1, unsigned char data2)
{
    char buf[100];
    sprintf(buf, "adding message for clip[%d][%d]: [%d]", clip->TrackIdx, clip->ClipIdx, clip->CurrentBar);
    Serial.print(buf);
}

void RemoveMessagesFromClip(Clip* clip)
{
}

void BeforeStartClip(Clip* clip)
{
    // load messages
}

void StartClip(Clip* clip)
{
    clip->CurrentBar = 0;
    SetClipPlaying(clip);

        Serial.print("CLIP [");
        Serial.print(clip->TrackIdx);
        Serial.print("][");
        Serial.print(clip->ClipIdx);
        Serial.print("] Selected: ");
        Serial.print(clip->Selected());
        Serial.print("  Playing: ");
        Serial.print(clip->Playing());
        Serial.print("  Armed: ");
        Serial.println(clip->Armed());    
}

void AfterStopClip(Clip* clip)
{
    
}

void StopClip(Clip* clip)
{
    ResetClipPlaying(clip);
    AfterStopClip(clip);
}



void ClipChangeTempoFactor(Clip *clip, int factor)
{
    // set the value
    clip->TempoFactor = factor;

    // re-arrange data in the message array
}

void PlayClip(unsigned char position, Output *output, Clip *clip)
{
    if(position >= ONE_BAR)
        return;

    clip->Play(position, output);
}

#endif