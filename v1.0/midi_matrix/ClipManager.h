#ifndef ClipManager_h
#define ClipManager_h

#include "Clip.h"

class ClipManager
{

public:
  void Run(unsigned long position, unsigned int sixteens)
  {
    if(position == 0 || position % 6 == 0)
    {
      for(int t=0; t<TRACKS; t++)
      {
        for(int c=0; c<CLIPS_PR_TRACK; c++)
        {
          Clip *clip = clips[t][c];
          if(clip->HasMessages())
          {

          }

          

          //clips[t][c]->Play(sixteens, outputs[t]);
        }
      }    
    }
  }

};


#endif