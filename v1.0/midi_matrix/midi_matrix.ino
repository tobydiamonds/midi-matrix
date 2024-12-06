#include <Arduino.h>
#include <Wire.h>
#include "ButtonArray.h"
#include "Input.h"
#include "Clip.h"
#include "Output.h"
#include "UIManager.h"
#include "UI_ClipMode.h"


/*
* i2c addresses A2 A1 A0  => address
* Functions      0  0  0      0
* InputOutput    0  0  1      1
* Track 1-2      0  1  0      2
* Track 3-4      0  1  1      3
* Track 5-6      1  0  0      4
* Track 7-8      1  0  1      5
*/

#define __RESET_PIN 8

MCP23017 functionsMcp(0x20);
MCP23017 inOutMcp(0x21);
MCP23017 tracks0102Mcp(0x22);
MCP23017 tracks0304Mcp(0x23);
MCP23017 tracks0506Mcp(0x24);
MCP23017 tracks0708Mcp(0x25);

TrackButtonArrayMapper trackButtonArrayMapper;

ButtonArray *functionButtons = new ButtonArray(0x20, functionsMcp);
ButtonArray *inOutButtons = new ButtonArray(0x21, inOutMcp);
ButtonArray *track0102Buttons = new ButtonArray(0x022, tracks0102Mcp, trackButtonArrayMapper);
ButtonArray *track0304Buttons = new ButtonArray(0x023, tracks0304Mcp, trackButtonArrayMapper);
ButtonArray *track0506Buttons = new ButtonArray(0x024, tracks0506Mcp, trackButtonArrayMapper);
ButtonArray *track0708Buttons = new ButtonArray(0x025, tracks0708Mcp, trackButtonArrayMapper);


UIManager *uimanager = new UIManager();
UI *clipui = new UIClipMode();

unsigned long position = 0;
unsigned int bpm = 120;

void OnButtonPressed(int sender, unsigned char index)
{
  uimanager->HandleButtonPressed(sender, index);
  /*char buf[80];
  sprintf(buf, "button pressed, array: 0x%02X  button nr: %d", sender, index);
  Serial.println(buf);*/
}

void OnButtonReleased(int sender, unsigned char index)
{
  uimanager->HandleButtonReleased(sender, index);
  /*char buf[80];
  sprintf(buf, "button released, array: 0x%02X  button nr: %d", sender, index);
  Serial.println(buf);*/
}

void OnSetBpm(int factor)
{
  unsigned long f = 120 * factor;
  bpm = f/100;
  Serial.print("setting bpm to "); Serial.println(bpm);
  SetupTimer1();
}

int GetBpm()
{
  return bpm;
}

void SetupTimer1() {
  noInterrupts(); // Disable interrupts during setup

  // Configure Timer1 for the desired interrupt frequency
  TCCR1A = 0;              // Normal operation mode
  TCCR1B = 0;              // Clear the control register
  TCNT1 = 0;               // Reset the timer count
  unsigned long interval = 60000 / (GetBpm() * 24); // Interval for MIDI clock ticks
  unsigned long compareMatch = (16 * interval * 1000) / 1024; // Convert ms to timer ticks (16 MHz clock, 1024 prescaler)

  OCR1A = compareMatch;    // Set the compare match register
  TCCR1B |= (1 << WGM12);  // CTC mode (Clear Timer on Compare)
  TCCR1B |= (1 << CS12) | (1 << CS10); // Set prescaler to 1024

  TIMSK1 |= (1 << OCIE1A); // Enable Timer1 compare interrupt
  interrupts();            // Enable interrupts globally
}
int quaters = 0;
int eights = 0;
int sixteens = 0;
// Timer1 interrupt service routine
ISR(TIMER1_COMPA_vect) {

  if(position == 0 || position % 6 == 0)
  {
    for(int t=0; t<TRACKS; t++)
    {
      for(int c=0; c<CLIPS_PR_TRACK; c++)
      {
        clips[t][c]->Play(sixteens, outputs[t]);
      }
    }
  }  

  if(position < ONE_BAR-1)
  {
    position++;    
    if(position % 6 == 0) sixteens++;
  }
  else
  {
    position = 0;
    quaters = 0;
    eights = 0;
    sixteens = 0;
  }  
}


void setup() {
  pinMode(__RESET_PIN, OUTPUT);
  digitalWrite(__RESET_PIN, HIGH); // not reset input arrays

  Wire.begin();
  // serial
  Serial.begin(115200); // debugging
  Serial1.begin(31250); // midi serial

  // init button arrays
  functionButtons->Initialize();
  functionButtons->ButtonPressed = OnButtonPressed;
  functionButtons->ButtonReleased = OnButtonReleased;  

  inOutButtons->Initialize();
  inOutButtons->ButtonPressed = OnButtonPressed;
  inOutButtons->ButtonReleased = OnButtonReleased;

  track0102Buttons->Initialize();
  track0102Buttons->ButtonPressed = OnButtonPressed;
  track0102Buttons->ButtonReleased = OnButtonReleased;

  track0304Buttons->Initialize();
  track0304Buttons->ButtonPressed = OnButtonPressed;
  track0304Buttons->ButtonReleased = OnButtonReleased;

  track0506Buttons->Initialize();
  track0506Buttons->ButtonPressed = OnButtonPressed;
  track0506Buttons->ButtonReleased = OnButtonReleased;

  track0708Buttons->Initialize();
  track0708Buttons->ButtonPressed = OnButtonPressed;
  track0708Buttons->ButtonReleased = OnButtonReleased;      

  // init UI
  clipui->SetBpm(OnSetBpm);
  uimanager->Add(clipui);
  uimanager->SwitchUI(0); // activate the clip-mode UI
  uimanager->Initialize();

  // init inputs
  InitInputs();

  // init clips
  InitClips();

  // init output
  InitOutputs();
  outputs[1]->IsEnabled = true;

  SetupTimer1();

}

extern unsigned int __bss_end; // End of global/static variables
extern unsigned int __heap_start; // Start of heap
extern void *__brkval;           // Current end of the heap
unsigned long lastMemReport = 0;
// Function to calculate free SRAM
int freeSRAM() {
  int free_memory;
  if ((int)__brkval == 0) {
    // If no heap allocation has been done, use the end of .bss
    free_memory = (int)&free_memory - (int)&__bss_end;
  } else {
    // Use the gap between heap and stack
    free_memory = (int)&free_memory - (int)__brkval;
  }
  return free_memory;
}

unsigned long now = 0;
void loop() {
  functionButtons->Run();
  inOutButtons->Run();
  track0102Buttons->Run();
  track0304Buttons->Run();
  track0506Buttons->Run();
  track0708Buttons->Run();


  now = millis();
  if(now > (lastMemReport + 5000))
  {
    lastMemReport = millis();
    Serial.print("Free SRAM: ");
    Serial.print(freeSRAM());
    Serial.println(" bytes");  
  }

}
