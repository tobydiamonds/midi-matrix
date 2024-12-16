#ifndef Song_h
#define Song_h

#include <ArduinoJson.h>
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
  
  unsigned int GetMaxBars(JsonDocument& doc)
  {
    unsigned int max = 0;
    JsonArrayConst song_clips = doc["clips"].as<JsonArrayConst>();
    for(int c=0; c<song_clips.size(); c++)
    {
      JsonVariantConst song_clip = song_clips[c];
      JsonArrayConst song_clip_bars = song_clip["bars"].as<JsonArrayConst>();
      if(song_clip_bars.size() > max)
        max = song_clip_bars.size();
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
    size_t length = strlen_P(always_on_my_mind_json);
    Serial.print("json string length: ");
    Serial.println(length);
    char* buffer = new char[length + 1];
    strcpy_P(buffer, always_on_my_mind_json);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buffer);
    delete[] buffer;
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return false;
    }

    _length = GetMaxBars(doc);
    Serial.print("song length: "); Serial.println(_length);
    _bars = new Bar[_length];
    for(int i=0; i<_length; i++)
      _bars[i]=Bar();

    // load messages into clips
    JsonArrayConst song_clips = doc["clips"].as<JsonArrayConst>();
    for(int c=0; c<song_clips.size(); c++)
    {
      JsonVariantConst song_clip = song_clips[c];

      int trackIdx = song_clip["track"].as<int>();
      int clipIdx = song_clip["clip"].as<int>();
      const char * description = song_clip["description"];

      // bars
      JsonArrayConst song_clip_bars = song_clip["bars"].as<JsonArrayConst>();
      for(int i=0; i<song_clip_bars.size(); i++)
      {
        int index = song_clip_bars[i].as<int>();
        _bars[index].Add(trackIdx, clipIdx);
      }
      
      // messages
      Serial.print("adding messages for clip["); Serial.print(trackIdx); Serial.print("][");Serial.print(clipIdx);Serial.print("]: ");Serial.println(description);

      Clip *clip = clips[trackIdx][clipIdx];
      JsonArrayConst song_clip_messages = song_clip["messages"].as<JsonArrayConst>();
      for(int i=0; i<song_clip_messages.size(); i++)
      {
        JsonArrayConst song_clip_message_data = song_clip_messages[i].as<JsonArrayConst>();
        for(int j=0; j<song_clip_message_data.size(); j++)
        {
          if((j+1)%3 == 0)
          {
            clip->AddMessage(i, song_clip_message_data[j-2], song_clip_message_data[j-1], song_clip_message_data[j]);
          }
        }
      }

      //delete doc;
     
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