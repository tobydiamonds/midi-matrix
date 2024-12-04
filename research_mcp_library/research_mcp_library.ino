#include <Arduino.h>
#include <Wire.h>
#include <MCP23017.h>

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

MCP23017 functions = MCP23017(0x20);
MCP23017 inout = MCP23017(0x21);

void setup() {
  pinMode(__RESET_PIN, OUTPUT);
  digitalWrite(__RESET_PIN, HIGH); // not reset functions

  Wire.begin();
  Serial.begin(115200);

  functions.init();
  functions.portMode(MCP23017Port::A, 0b11111111); //Port A as input
  functions.writeRegister(MCP23017Register::GPIO_A, 0x00);  //Reset port A 
  functions.writeRegister(MCP23017Register::IPOL_A, 0xFF);

  inout.init();
  inout.portMode(MCP23017Port::A, 0b11111111); //Port A as input
  inout.portMode(MCP23017Port::B, 0b11111111); //Port B as input
  inout.writeRegister(MCP23017Register::GPIO_A, 0x00);  //Reset port A 
  inout.writeRegister(MCP23017Register::GPIO_B, 0x00);  //Reset port B
  inout.writeRegister(MCP23017Register::IPOL_A, 0xFF);
  inout.writeRegister(MCP23017Register::IPOL_B, 0xFF);
}

uint8_t oldFunctionsA = 0;
uint8_t oldInoutA = 0;
uint8_t oldInoutB = 0;

void loop() {
  uint8_t functionsA = functions.readPort(MCP23017Port::A);
  uint8_t inoutA = inout.readPort(MCP23017Port::A);
  uint8_t inoutB = inout.readPort(MCP23017Port::B);

  bool update = false;

  if(functionsA != oldFunctionsA) {
    oldFunctionsA = functionsA;
    update = true;
  }

  if(inoutA != oldInoutA) {
    oldInoutA = inoutA;
    update = true;
  }

  if(inoutB != oldInoutB) {
    oldInoutB = inoutB;
    update = true;
  }

  if(update) {
    Serial.print(functionsA, BIN);
    Serial.print(" ");
    Serial.print(inoutA, BIN);
    Serial.print(" ");
    Serial.print(inoutB, BIN);

    Serial.println();
  }
}
