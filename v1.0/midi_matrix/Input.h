#ifndef Input_h
#define Input_h

#define INPUTS 4

struct Input
{
    unsigned char Channel;

    Input(unsigned char channel)
    {
        Channel = channel;
    }
};

Input *inputs[INPUTS];

void InitInputs()
{
    for(unsigned char i=0; i<INPUTS; i++)
    {
        inputs[i] = new Input(i);
    }
}


#endif