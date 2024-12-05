#ifndef ButtonArrayMapper_h
#define ButtonArrayMapper_h

class ButtonArrayMapper
{
public:
  virtual uint8_t MapPortAIndex(uint8_t index) = 0;
  virtual uint8_t MapPortBIndex(uint8_t index) = 0;  

};

class DefaultButtonArrayMapper : public ButtonArrayMapper
{
  uint8_t MapPortAIndex(uint8_t index)
  {
    return index+8;
  }

  uint8_t MapPortBIndex(uint8_t index)
  {
    return index;
  }  
};

class TrackButtonArrayMapper : public ButtonArrayMapper
{
  uint8_t MapPortAIndex(uint8_t index)
  {
    // this is tracks 2,4,6,8 - we want the index to be 8..15
    return (7-index)+8;
  }

  uint8_t MapPortBIndex(uint8_t index)
  {
    // this is tracks 1,3,5,7 - we want the index to be 0..7
    return 7-index;
  }  
};

#endif