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

void setup() {
  pinMode(__RESET_PIN, OUTPUT);
  digitalWrite(__RESET_PIN, HIGH); // not reset input arrays

  Wire.begin();
  Serial.begin(115200);

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
  uimanager->Add(clipui);
  uimanager->SwitchUI(0); // activate the clip-mode UI
  uimanager->Initialize();

  // init inputs
  InitInputs();

  // init clips
  InitClips();

  // init output
  InitOutputs();

}

void loop() {
  functionButtons->Run();
  inOutButtons->Run();
  track0102Buttons->Run();
  track0304Buttons->Run();
  track0506Buttons->Run();
  track0708Buttons->Run();

}
