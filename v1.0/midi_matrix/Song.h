#ifndef Song_h
#define Song_h

#include "jRead.h" //https://www.codeproject.com/Articles/885389/jRead-An-in-place-JSON-Element-Reader
#include <avr/pgmspace.h>
#include "AlwaysOnMyMind.h"
#include "Clip.h"
#include "MidiDefs.h"
#include "Output.h"

struct ClipReference
{
  uint8_t TrackIdx;
  uint8_t ClipIdx;
  ClipReference(uint8_t trackIdx, uint8_t clipIdx)
  {
    TrackIdx = trackIdx;
    ClipIdx = clipIdx;
  }
};

class Bar
{
private:
  //ClipReference* _clips[TRACKS * CLIPS_PR_TRACK] = {nullptr};
  
  int _size=0;
public:
  unsigned long _set=0;
  Bar()
  {

  }

  Add(uint8_t trackIdx, uint8_t clipIdx)
  {
    int index = 1 << ((trackIdx*8)+clipIdx);
    _set = _set | index;
    _size++;
    //_clips[index] = new ClipReference(trackIdx, clipIdx);
  }




  ClipReference** GetAll(int& size)
  {
    ClipReference** result = new ClipReference*[_size] {nullptr};
    int result_index=0;

    for(int pos=0; pos<sizeof(_set)*8; pos++)
    {
      int index=(1 << pos);
      if(_set & index)
      {
        // pos 0 => trackidx=0, clipidx=0
        // pos 1 => trackidx=0, clipidx=1
        // pos 7 => trackidx=0, clipidx=7
        // pos 8 => trackidx=1, clipidx=0
        // pos 9 => trackidx=1, clipidx=1
        // pos 16=> trackidx=2, clipidx=0
        // pos 19=> trackidx=2, clipidx=3

        uint8_t trackIdx = pos / 8;
        uint8_t clipIdx = (trackIdx==0) ? pos : pos % 8;

        result[result_index++] = new ClipReference(trackIdx, clipIdx);

      }
    }
    size = result_index;

    return result;
  }


};

class Song
{
private:
  unsigned int _bar;

  bool _initialized;

  Bar* _bars;

  int _length;

  Clip** _currentClips;
  int _currentSize = 0;
  int _currentSixteens = -1;
  
  unsigned int GetMaxBars()
  {
    size_t length = strlen_P(always_on_my_mind_json);
    char json[length+1] {0};// = new char[length + 1];
    strcpy_P(json, always_on_my_mind_json);

    unsigned int max = 0;
    struct jReadElement song_clips;
    jRead(json, "{'clips'", &song_clips);
    for(int i=0; i<song_clips.elements; i++)
    {
      struct jReadElement song_clip_bars;
      jRead(json, "{'clips'[*{'bars'", &song_clip_bars);
      if(song_clip_bars.dataType == JREAD_ARRAY && song_clip_bars.elements > max)
        max = song_clip_bars.elements;
    }
    return max;
  }

  void PrintBars()
  {
    for(int i=0; i<_length;i++)
    {
      Serial.print("BAR ");
      Serial.print(i);
      Serial.print(" ");
      Serial.print(_bars[i]._set, BIN);

      int size=0;

      ClipReference** refs = _bars[i].GetAll(size);
      for(int j=0; j<size; j++)
      {
        Serial.print(" [");
        Serial.print(refs[j]->TrackIdx);
        Serial.print("][");
        Serial.print(refs[j]->ClipIdx);
        Serial.print("],");  
      }
      Serial.println();
    }
  }

public:
  Song() {
    _bar = 0;
    _initialized = false;
  }

  bool Initialize()
  {
    // init bars
    _length = GetMaxBars();
    Serial.print("song length: "); Serial.println(_length);
    _bars = new Bar[_length];
    for(int i=0; i<_length; i++)
      _bars[i]=Bar();


    // load messages into clips
    size_t length = strlen_P(always_on_my_mind_json);
    char json[length+1] {0};// = new char[length + 1];
    strcpy_P(json, always_on_my_mind_json);
    Serial.print("json string length: ");
    Serial.println(length);

    char path[50];

    struct jReadElement song_clips;
    jRead(json, "{'clips'", &song_clips);
    for(int i=0; i<song_clips.elements; i++)
    {

      int trackIdx = jRead_int(json, "{'clips'[*{'track'", &i);
      int clipIdx = jRead_int(json, "{'clips'[*{'clip'", &i);
      char description[20];
      sprintf(path, "{'clips'[%d{'description'", i);
      jRead_string(json, path, description, sizeof(description), NULL);

      // bars
      //Serial.print("adding bars for clip["); Serial.print(trackIdx); Serial.print("][");Serial.print(clipIdx);Serial.print("]: ");Serial.println(description);

      
      sprintf(path, "{'clips'[%d{'bars'", i);
      struct jReadElement song_clip_bars;
      jRead(json, path, &song_clip_bars);
      for(int j=0; j<song_clip_bars.elements; j++)
      {
        sprintf(path, "{'clips'[%d{'bars'[%d", i,j);
        int index = jRead_int(json, path, 0);
        _bars[index].Add(trackIdx, clipIdx);
      }

     
      // messages
      //Serial.print("adding messages for clip["); Serial.print(trackIdx); Serial.print("][");Serial.print(clipIdx);Serial.print("]: ");Serial.println(description);

      Clip *clip = clips[trackIdx][clipIdx];
      struct jReadElement song_clip_messages;
      sprintf(path, "{'clips'[%d{'messages'", i);
      jRead(json, path, &song_clip_messages);
      for(int j=0; j<song_clip_messages.elements; j++)
      {
        struct jReadElement message;
        sprintf(path, "{'clips'[%d{'messages'[%d", i, j);
        jRead(json, path, &message);

        for(int k=0; k<message.elements; k++)
        {
          if((k+1)%3==0)
          {
            sprintf(path, "{'clips'[%d{'messages'[%d[%d", i, j, k-2);
            uint8_t type = jRead_int(json, path, 0);
            sprintf(path, "{'clips'[%d{'messages'[%d[%d", i, j, k-1);
            uint8_t data1 = jRead_int(json, path, 0);           
            sprintf(path, "{'clips'[%d{'messages'[%d[%d", i, j, k);
            uint8_t data2 = jRead_int(json, path, 0);

            //Serial.print("message[");Serial.print(j);Serial.print("]: "); Serial.print(type, HEX); Serial.print(" ");Serial.print(data1, HEX); Serial.print(" ");Serial.println(data2, HEX);

            clip->AddMessage(j, type, data1, data2);
          }
        }
      }

      // enable outputs
      outputs[trackIdx]->IsEnabled=true;
      // enables clips
      SetClipPlaying(clip);    
    }
    //PrintBars();



    _currentSize = 0;
    _currentSixteens = -1;
    _initialized = true;
    return true;    
  }

  void Run(unsigned int position)
  {
    if(!_initialized) return;

    // find clips to play in current bar
    if(position==0 && _bar < _length)
    {
      Serial.print("BAR ");
      Serial.println(_bar);      
      int size=0;
      ClipReference** bar_clips = _bars[_bar].GetAll(size);
      _currentClips = new Clip*[size] {nullptr};
      _currentSize = size;

      for(int i=0; i<size; i++)
      {
        if(bar_clips[i] != nullptr)
        {
          Clip *clip = clips[bar_clips[i]->TrackIdx][bar_clips[i]->ClipIdx];
          _currentClips[i] = clip;
        }
      }
    }

    // play clips in current bar
    int sixteens = position / 6;
    if(position == 0 || sixteens != _currentSixteens)
    {
      _currentSixteens = sixteens;
      if(_bar < _length)
      {
        for(int i=0; i<_currentSize; i++)
        {
          if(_currentClips[i]->Playing())
            _currentClips[i]->Play(_currentSixteens);
        }
      }
    }

    if(position==ONE_BAR-1)
    {
      _bar++;
      delete[] _currentClips;
    }

    if(_bar >= _length)
      _bar = 0;

  }

};


#endif