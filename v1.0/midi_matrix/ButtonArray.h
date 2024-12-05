#ifndef ButtonArray_h
#define ButtonArray_h

#include <Arduino.h>
#include <Wire.h>
#include <MCP23017.h>
#include "ButtonArrayMapper.h"


#define BUTTON_ARRAY_SIZE 8
#define BUTTON_ARRAY_DEBOUNCE_DELAY 250
#define BUTTON_ARRAY_DEBOUNCE_RESET 500

class ButtonArray
{
private:
  uint8_t _i2cAddress;
  MCP23017& _mcp;
  uint8_t _lastStateA;
  unsigned long _lastPressA[BUTTON_ARRAY_SIZE];
  unsigned long _lastReleaseA[BUTTON_ARRAY_SIZE];  
  uint8_t _lastStateB;
  unsigned long _lastPressB[BUTTON_ARRAY_SIZE];
  unsigned long _lastReleaseB[BUTTON_ARRAY_SIZE];  
  ButtonArrayMapper* _mapper;

  bool IsBitSet(uint8_t value, uint8_t bitPosition)
  {
    return (value & (1<<bitPosition)) != 0;
  }


public:
  ButtonArray(uint8_t i2cAddress, MCP23017& mcp, ButtonArrayMapper& mapper) : _mcp(mcp), _mapper(&mapper)
  {
    _i2cAddress = i2cAddress;
  }

  ButtonArray(uint8_t i2cAddress, MCP23017& mcp) : _mcp(mcp)
  {
   _i2cAddress = i2cAddress;
   _mapper = new DefaultButtonArrayMapper();
  }

  void (*ButtonPressed)(int sender, uint8_t index) = 0;
  void (*ButtonReleased)(int sender, uint8_t index) = 0;  

  void Initialize()
  {
    _lastStateA = 0;
    _lastStateB = 0;
    for(int i=0; i<BUTTON_ARRAY_SIZE; i++)
    {
      _lastPressA[i] = 0;
      _lastReleaseA[i] = 0;
      _lastPressB[i] = 0;
      _lastReleaseB[i] = 0;
    }

    _mcp.init();
    _mcp.portMode(MCP23017Port::A, 0b11111111); //Port A as input
    _mcp.portMode(MCP23017Port::B, 0b11111111); //Port B as input
    _mcp.writeRegister(MCP23017Register::GPIO_A, 0x00);  //Reset port A 
    _mcp.writeRegister(MCP23017Register::GPIO_B, 0x00);  //Reset port B
    _mcp.writeRegister(MCP23017Register::IPOL_A, 0xFF);
    _mcp.writeRegister(MCP23017Register::IPOL_B, 0xFF);    
  }

  void Run()
  {
    unsigned long now = millis();
    uint8_t state = _mcp.readPort(MCP23017Port::A);
    if(state != _lastStateA)
    {
      // find out which bits has changed from 0=>1 : ButtonPressed and 1=>0 : ButtonReleased
      for(int i=0; i<BUTTON_ARRAY_SIZE; i++)
      {
        if(IsBitSet(state, i) && !IsBitSet(_lastStateA, i))
        {
          if(now > (_lastPressA[i] + BUTTON_ARRAY_DEBOUNCE_DELAY))
          {
            _lastPressA[i] = now;
            ButtonPressed(_i2cAddress, _mapper->MapPortAIndex(i));
          }
        }
        
        if(!IsBitSet(state, i) && IsBitSet(_lastStateA, i))
        {
          if(now > (_lastReleaseA[i] + BUTTON_ARRAY_DEBOUNCE_DELAY) && _lastPressA[i] > _lastReleaseA[i])
          {
            _lastReleaseA[i] = now;
            ButtonReleased(_i2cAddress, _mapper->MapPortAIndex(i));
          }
        }
      }

      _lastStateA = state;
    }
    state = _mcp.readPort(MCP23017Port::B);
    if(state != _lastStateB)
    {
      // find out which bits has changed from 0=>1 : ButtonPressed and 1=>0 : ButtonReleased
      for(int i=0; i<BUTTON_ARRAY_SIZE; i++)
      {
        if(IsBitSet(state, i) && !IsBitSet(_lastStateB, i))
        {
          if(now > (_lastPressB[i] + BUTTON_ARRAY_DEBOUNCE_DELAY))
          {
            _lastPressB[i] = now;
            ButtonPressed(_i2cAddress, _mapper->MapPortBIndex(i));
          }
        }
        
        if(!IsBitSet(state, i) && IsBitSet(_lastStateB, i))
        {
          if(now > (_lastReleaseB[i] + BUTTON_ARRAY_DEBOUNCE_DELAY) && _lastPressB[i] > _lastReleaseB[i])
          {
            _lastReleaseB[i] = now;
            ButtonReleased(_i2cAddress, _mapper->MapPortBIndex(i));
          }
        }
      }

      _lastStateB = state;
    }

  }

};

#endif