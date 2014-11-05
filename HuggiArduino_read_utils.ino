
bool readExpected(Stream& serial, const char * expected, long timeout)
{
    // Serial << "   in readexp " << expected << nl;
    const char * p; 
    long lastRead = millis();
    char c;
    
    while((millis() - lastRead) < timeout)
    {
        p = expected;

        while((millis() - lastRead) < timeout)
        {
            if(serial.available() )
            {
                // get one char
                c = serial.read();

                // not the char expected, go back to the beginning
                if(c != *p) break;
                // we got one interesting char, reset timeout ??
                // lastRead = millis();
                // ge got all the chars expected
                if(*(++p) == 0) return true;
            }
        }
    }
    return false;
}


bool readLine(Stream& serial, char * dest, int dest_size, long timeout)
{
    long lastRead = millis();
    int count = 0;
    bool ret = false;
    char c;
    
    // Serial << "   in readline" << nl;
    while( count < dest_size && 
           millis() - lastRead < timeout)
    {   
        if(serial.available() )
        {
            // get one char
            c = serial.read();
            lastRead = millis();

            // newline, stop
            if(c == nl){
                ret = true;
                *dest = 0; // end string
                break;
            }
            // read char
            *(dest++) = c;
            count++;
        }
    }
    return ret;
}