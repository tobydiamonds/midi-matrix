#ifndef UI_h
#define UI_h

class UI {
    
public:
    virtual int Initialize() = 0;
    virtual int Activate() = 0;
    virtual int Deactivate() = 0;
    virtual int HandleButtonPressed(int sender, unsigned char index) = 0;
    virtual int HandleButtonReleased(int sender, unsigned char index) = 0;    
    
    typedef void (*SetBpmCallback)(int);
    void SetBpm(SetBpmCallback callback)
    {
      this->_setBpmCallback = callback;
    }
protected:
    SetBpmCallback _setBpmCallback;
    void TriggerSetBpmCallback(int factor)
    {
      if(_setBpmCallback != nullptr)
      {
        _setBpmCallback(factor);
      }
    }
};


#endif