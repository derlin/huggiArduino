#ifndef HUGGI_BUFFER
#define HUGGI_BUFFER

#include "HuggiArduino.h"

#define MAX_STORED_HUGS     7

/**
 * Data relative to one hug.
 */
typedef struct Hug
{
    char id[ID_SIZE+1];         //!< The hugger id
    char data[DATA_MAX_SIZE+1]; //!< The data received from the partner
    long duration;              //!< The duration of the hug
} Hug_t;

/**
 * A cyclic buffer for storing when there is no connected device. 
 */
class HuggiBuffer {

public:
    HuggiBuffer() : top(0), bottom(0){}


    /**
     * @return A pointer to the next available slot
     */
    Hug_t* getAvail();

    /**
     * Add one hug to the buffer. You can get a pointer to 
     * the next available slot by calling :getAvail().
     */
    void commit();


    /**
     * Get the oldest hug in this buffer.
     * @return A pointer to the oldest hug
     */
     Hug_t* getNext();

     /**
      * Remove the oldest hug from this buffer.
      * The oldest hug is the one returned by :getNext().
      */
     void remove();

     /**
      * Get the number of hugs currently in the buffer.
      * @return The number of hugs 
      */
     byte getSize() { return size; }

     /**
      * Test if the buffer is empty
      * @return true if this buffer is empty, false otherwise.
      */
    bool isEmpty() { return size == 0; }

     /**
      * Test if the buffer is full.
      * @return true if this buffer is full, false otherwise.
      *
      */
    bool isFull()  { return size  == MAX_STORED_HUGS; }

private:
    Hug_t hugs[MAX_STORED_HUGS + 1];  //!< The buffer
    byte top;                         //!< The top of this buffer, i.e. the next available slot
    byte bottom;                      //!< The top of this buffer, i.e. the oldest hug
    byte size;                        //!< The number of hugs in this buffer
};


#ifdef TEST
  #include <iostream>
  void toString(std::ostream&, Hug_t&);
#else
  #include <Serial.h>
  #include "Streaming.h"
  void toString(Stream&, Hug_t&);
#endif

#endif