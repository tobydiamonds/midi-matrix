#ifndef Song_h
#define Song_h

#include <ArduinoJson.h>
#include "AlwaysOnMyMind.h"
#include "Clip.h"

class Song
{
private:
  unsigned int _bar;
  JsonDocument _doc;
  bool _initialized;

  void removeWhitespace(char* str) {
      int writeIndex = 0; // Position to write non-whitespace characters
      int readIndex = 0;  // Position to read characters

      while (str[readIndex] != '\0') {
          // Check if the character is not a whitespace
          if (str[readIndex] != ' ' && str[readIndex] != '\t' && str[readIndex] != '\n' && str[readIndex] != '\r') {
              str[writeIndex++] = str[readIndex];
          }
          readIndex++;
      }
      // Null-terminate the result string
      str[writeIndex] = '\0';
  }

public:
  Song() {
    _bar = 0;
    _initialized = false;
  }

  bool Initialize()
  {
    size_t length = strlen_P(always_on_my_mind_json);
    Serial.print("String length: ");
    Serial.println(length);
    char* buffer = new char[length + 1];
    strcpy_P(buffer, always_on_my_mind_json);

    DeserializationError error = deserializeJson(_doc, buffer);
    delete[] buffer;
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return false;
    }

    // load messages into clips
    JsonArrayConst song_clips = _doc["clips"].as<JsonArrayConst>();
    for(int c=0; c<song_clips.size(); c++)
    {
      JsonVariantConst song_clip = song_clips[c];

      int trackIdx = song_clip["track"].as<int>();
      int clipIdx = song_clip["clip"].as<int>();
      const char * description = song_clip["description"];

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
    }

    _initialized = true;
    return true;
  }

  void Run(unsigned int position)
  {
    if(!_initialized) return;


  }

};


#endif

/*
song: {
  start: [0.0, 1.0, 2.0] // this list contains the starting clips - the song eveluting is described in how clips are repearting and/or linking
  clips: [ 
    0.0: {
      command: repeat-forever // this plays clip 0.0 until song is stopped - e.g. bass drum
      next-clip: null
      messages: [
        0: message-data-here
        4: 
        8: 
        12: 
      ]
    },
    0.1: {
      command: repeat // play clip 0.1 eight times and then play the linked clip
    },
    1.0: {
      command: play-next-clip // play clip 1.0 once and then play the linked clip (1.1)
      next-clip: 1.1
      messages: []
    }
    1.1: {
      command: play-next-clip // play clip 1.1 once and then play the linked clip (1.0)
      next-clip: 1.0
      messages: []
    },
    2.0: {
      command: repeat // play clip 2.0 four time and then play the linked clip (2.1)
      commandData: 4
      next-clip: 2.1
      message: []
    }
    2.1: {
      command: stop // play clip 2.1 once and then ... nothing
      next-clip: null
      messages: []
    }



  ]


}





*/