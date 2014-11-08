#include "HuggiBuffer.h"

#define AddModulo(x, i)     ( ((x)+(i)) % MAX_STORED_HUGS )

// --------

bool HuggiBuffer::isEmpty()
{
    return top == bottom;
}

// --------

bool HuggiBuffer::isFull()
{
    return AddModulo(top, 1) == bottom;
}

Hug_t* HuggiBuffer::getAvail()
{
    Hug_t* hug;

    if(!isFull())
    {
        hug = &hugs[top];
        hug->id[0] = hug->data[0] = hug->duration = 0; // reset 
        
    }

    return hug;

}

// --------

void HuggiBuffer::commit()
{
    top = AddModulo(top, 1);
}

// --------

Hug_t* HuggiBuffer::getNext()
{
    Hug_t* hug;

    if(!isEmpty() )
    {
        hug = &hugs[bottom];
        bottom = AddModulo(bottom, 1);
    } 

    return hug;

}

// --------

#ifdef TEST
void toString(std::ostream& s, Hug_t& h)
#else
void toString(Stream& s, Hug_t& h)
#endif
{
    s << "Hug with " << h.data << "(" << h.id << ") : duration = " << h.duration << nl;
}