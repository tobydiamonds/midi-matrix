#ifndef UIManager_h
#define UIManager_h

#include <Arduino.h>
#include "UI.h"

#define MAX_UI_ITEMS 5

class UIManager
{
private:

    UI *items[MAX_UI_ITEMS];
    int counter = 0;
    int current_item = -1;


public:
    UIManager()
    {
        for(unsigned int i=0; i<MAX_UI_ITEMS; i++)
        {
            items[i] = 0;
        }

    }

    int Add(UI *item)
    {
        if(counter >= MAX_UI_ITEMS) return 1;
        items[counter++] = item;
        Serial.print("ui added at index: ");
        Serial.println(counter-1);
        return 0;
    }

    int Initialize()
    {
        int result = 0;
        for(int i=0; i<counter; i++)
        {
            if(!result)
                result = items[i]->Initialize();
        }
        return result;
    }

    int HandleButtonPressed(int sender, unsigned char index)
    {
        if(counter == 0) return 1; // no ui in the list
        if(current_item == -1) return 1; // no ui active

        return items[current_item]->HandleButtonPressed(sender, index);
    }

    int HandleButtonReleased(int sender, unsigned char index)
    {
        if(counter == 0) return 1; // no ui in the list
        if(current_item == -1) return 1; // no ui active

        return items[current_item]->HandleButtonReleased(sender, index);
    }    

    int SwitchUI(int index)
    {
        if(index < 0) return 1;
        if(index > MAX_UI_ITEMS) return 1;
        if(index > counter-1) return 1;

        Serial.print("Activating UI: ");
        Serial.println(current_item);

        if(current_item != -1)
            items[current_item]->Deactivate();

        current_item = index;

        Serial.print("Activating UI: ");
        Serial.println(index);

        items[current_item]->Activate();

        return 0;
    }

};

#endif