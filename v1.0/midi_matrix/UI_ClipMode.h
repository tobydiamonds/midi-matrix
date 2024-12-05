#ifndef UI_ClipMode_h
#define UI_ClipMode_h

#include <Arduino.h>
#include "UI.h"
#include "Output.h"
#include "Input.h"
#include "Clip.h"

#define MAX_SENDERS 6
#define MAX_INDEXES 16

#define SENDER_FUNCTIONS 0
#define SENDER_INPUT_OUTPUT 1
#define SENDER_TRACK_1_2 2
#define SENDER_TRACK_3_4 3
#define SENDER_TRACK_5_6 4
#define SENDER_TRACK_7_8 5

class UIClipMode: public UI {

private:
/* sender | index
* eg. buttons[0][6] => Function#6
*
* 0: functions (8-15)       => 15: arm, 8-14: function 1-7
* 1: input/output (0-15)    => 0-3: input 1-4, 8-15: output 1-8
* 2: track 1+2 (0-15)       => 0-7: track 1 clip 1-8, 8-15: track 2 clip 1-8
* 3: track 3+4 (0-15)       => 0-7: track 3 clip 1-8, 8-15: track 4 clip 1-8
* 4: track 5+6 (0-15)       => 0-7: track 5 clip 1-8, 8-15: track 6 clip 1-8
* 5: track 7+8 (0-15)       => 0-7: track 7 clip 1-8, 8-15: track 8 clip 1-8
*/
    bool pressed[MAX_SENDERS][MAX_INDEXES];
    bool isArmed = false;
    unsigned int tempofactors[7] = {25,50,75,100,200,333,400};

    bool IsAnyInputPressed()
    {
        for(int i=0; i<INPUTS; i++)
        {
            if(pressed[SENDER_INPUT_OUTPUT][i+8])
                return true;
        }
        return false;
    }

    void HandleOutputPressed(unsigned char index)
    {
        Output* output = outputs[index];

        // are we assigning output to an input?
        if(IsAnyInputPressed())
        {
            for(int i=0; i<INPUTS; i++)
            {
                if(pressed[SENDER_INPUT_OUTPUT][i+8])
                {
                    Input *input = inputs[i];
                    output->InputChannel = input->Channel;
                    Serial.print("OUTPUT ");
                    Serial.print(output->Channel);
                    Serial.print(" set to input channel ");
                    Serial.println(output->InputChannel);
                }
            }
        }
        else // otherwise we are changing enable output's enable state
        {
            output->IsEnabled = !output->IsEnabled;

            Serial.print("OUTPUT ");
            Serial.print(output->Channel);
            if(output->IsEnabled)
                Serial.println(" enabled");
            else
                Serial.println(" disabled");
        }
    }

    void HandleArmPressed()
    {
        isArmed = !isArmed;
        if(isArmed)
            Serial.println("SYSTEM ARMED!");
        else
            Serial.println("System disarmed");

    }

    void HandleFunctionPressed(unsigned char index)
    {
        index = index - 8;
        if(index < 0 || index > 6) return; // must be inside the tempofactors array
        // set tempo for all pressed clips
        for(int i=SENDER_TRACK_1_2; i<=SENDER_TRACK_7_8; i++) // [2..5]
        {
            for(int j=0; j<MAX_INDEXES; j++)
            {
                if(pressed[i][j]) // the clip is pressed
                {
                    unsigned char c = (j<8) ? j : j-8;
                    unsigned char t = i-2;
                    Clip *clip = clips[t][c];
                    if(clip!=0)
                        ClipChangeTempoFactor(clip,tempofactors[index]);

                    Serial.print("CLIP [");
                    Serial.print(clip->TrackIdx);
                    Serial.print("][");
                    Serial.print(clip->ClipIdx);
                    Serial.print("] Tempo factor: ");
                    Serial.println(clip->TempoFactor);
                }
            }
        }
    }

    void HandleClipPressed(unsigned char t, unsigned char c)
    {
        Clip* clip = clips[t][c];
        if(isArmed)
        {
            if(clip->Armed())
            {
                ResetClipArmed(clip);
                SetClipPlaying(clip); // when ending recording swith to playmode
                ListClipMessages(clip);
            }
            else
                SetClipArmed(clip);
        }
        else
        {
            if(clip->Selected())
                ResetClipSelected(clip);
            else
                SetClipSelected(clip);   
        }

        Serial.print("CLIP [");
        Serial.print(clip->TrackIdx);
        Serial.print("][");
        Serial.print(clip->ClipIdx);
        Serial.print("] Selected: ");
        Serial.print(clip->Selected());
        Serial.print("  Playing: ");
        Serial.print(clip->Playing());
        Serial.print("  Armed: ");
        Serial.print(clip->Armed());
        Serial.print("  Tempo factor: ");
        Serial.print(clip->TempoFactor);
        Serial.println();
    }

    void PrintPressed()
    {
      Serial.println("#inputs");
      for(int i=0; i<4; i++)
      {
        if(pressed[SENDER_INPUT_OUTPUT][i])
          Serial.print("X");
        else
          Serial.print("o");
        Serial.print(" ");
      }
      Serial.println();
      Serial.println("#outputs");
      for(int i=8; i<15; i++)
      {
        if(pressed[SENDER_INPUT_OUTPUT][i])
          Serial.print("X");
        else
          Serial.print("o");
        Serial.print(" ");
      }
      Serial.println();      


      Serial.println("#clips");
      for(int i=SENDER_TRACK_1_2; i<=SENDER_TRACK_7_8; i++) // [2..5]
        {
            for(int j=0; j<MAX_INDEXES; j++)
            {
                if(pressed[i][j]) // the clip is pressed
                  Serial.print("X");
                else
                  Serial.print("o");

                Serial.print(" ");
                if((j+1)%8==0)
                  Serial.println();
            }
        }

      Serial.println("#functions");
      for(int i=15; i>=8; i--)
      {
        if(pressed[SENDER_FUNCTIONS][i])
          Serial.print("X");
        else
          Serial.print("o");
        Serial.print(" ");
      }
      Serial.println();  
    }

public:
    int Initialize()
    {
        //pressed[MAX_SENDERS][MAX_INDEXES]
        for(int i=0; i<MAX_SENDERS; i++)
        {
          for(int j=0; j<MAX_INDEXES; j++)
          {
            pressed[i][j] = false;
          }
        }
        return 0;
    }

    int Activate()
    {
        return 0;
    }

    int Deactivate()
    {
        return 0;
    }

    int HandleButtonPressed(int sender, unsigned char index)
    {
        sender = sender - 0x20;
        if(sender < 0 || sender >= MAX_SENDERS) return 1;
        if(index < 0 || index >= MAX_INDEXES) return 1;

        pressed[sender][index] = true;

        Serial.print("sender:"); Serial.print(sender); Serial.print(" index:"), Serial.println(index);

        PrintPressed();

        if(sender==SENDER_FUNCTIONS && index == 15)
            HandleArmPressed();
        if(sender==SENDER_FUNCTIONS && index >= 8 && index < 15)
            HandleFunctionPressed(index);
        if(sender==SENDER_INPUT_OUTPUT && index >= 0 && index < 8 )
            HandleOutputPressed(index);

        if(sender==SENDER_TRACK_1_2)
        {
            if(index < 8)
                HandleClipPressed(0, index); 
            else
                HandleClipPressed(1, index-8);
        }
        else if(sender==SENDER_TRACK_3_4)
        {
            if(index < 8)
                HandleClipPressed(2, index);
            else
                HandleClipPressed(3, index-8);            
        }
        else if(sender==SENDER_TRACK_5_6)
        {
            if(index < 8)
                HandleClipPressed(4, index);
            else
                HandleClipPressed(5, index-8);            
        }            
        else if(sender==SENDER_TRACK_7_8)
        {
            if(index < 8)
                HandleClipPressed(6, index);
            else
                HandleClipPressed(7, index-8);            
        }

        return 0;
    }
    
    int HandleButtonReleased(int sender, unsigned char index)
    {
        sender = sender - 0x20;
        if(sender < 0 || sender >= MAX_SENDERS) return 1;
        if(index < 0 || index >= MAX_INDEXES) return 1;

        pressed[sender][index] = false;

        PrintPressed();

        return 0;
    }    

};

#endif