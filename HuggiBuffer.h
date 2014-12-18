#ifndef HUGGI_BUFFER
#define HUGGI_BUFFER

#include "HuggiArduino.h"

#define MAX_STORED_HUGS     7


typedef struct Hug
{
    char id[ID_SIZE+1];
    char data[DATA_MAX_SIZE+1];
    long duration;
} Hug_t;


class HuggiBuffer {

public:
    HuggiBuffer() : top(0), bottom(0){}


    /**
     * get a pointer to the next available struct
     */
    Hug_t* getAvail();
    /**
     * commit the changes made to the next available struct
     */
    void commit();


    /**
     * get and remove the oldest hug not already sent
     */
     Hug_t* getNext();

     /**
      * remove the last hug returned by getNext()
      */
     void remove();

     /**
      * get the number of hugs currently in the buffer
      */
     byte getSize() { return size; }

     /**
      * test if the buffer is empty
      */
    bool isEmpty() { return size == 0; }

     /**
      * test if the buffer is full
      */
    bool isFull()  { return size  == MAX_STORED_HUGS; }

private:
    Hug_t hugs[MAX_STORED_HUGS + 1];
    byte top;
    byte bottom;
    byte size;
};


#ifdef TEST
#include <iostream>
void toString(std::ostream&, Hug_t&);
#else
#include <Serial.h>
#include "Streaming.h"
void toString(Stream&, Hug_t&);
#endif


extern volatile bool triggered;

#endif