#define EEPROM_MAGIC    'L' // if configured, first 2 bytes are LL (avoid reading garbage)



// ========== EEPROM

/**
 * write a string to eeprom, address 0
 */
void writeToEEprom(byte address, char * data, int length)
{
    EEPROM.write(0, 0); // reset: not configured

    int i = 0;
    for(; i < length; i++) // write data 
    {
        EEPROM.write(address + 2 + i, data[i]);
        Serial << "written: " << data[i] << " at " <<
            address + 2 + i << nl;

    }
    EEPROM.write(address + 2 + i, 0); i++; // end of string
    
    // write magic
    EEPROM.write(address, EEPROM_MAGIC);
    EEPROM.write(address + 1, EEPROM_MAGIC);
    Serial << "[EP] written LL at " << address << nl;

    Serial << "done\n";
}

/**
 * read a string to eeprom, address 0
 */
byte readFromEEprom(byte address, char * data, int buff_size)
{
    byte len = 0;

    if(EEPROM.read(address) != EEPROM_MAGIC || 
       EEPROM.read(address + 1) != EEPROM_MAGIC)
    {
        Serial << "Not configured... So not reading!\n"; 
        return len;
    }

    address += 2;
    char * stop = data + buff_size;

    for(; data < stop, address + len < EEPROM_MAX_ADDRESS; len++, data++)
    {
        *data = EEPROM.read(address + len);

        if(*data == 0) break; // end of string
    }

    Serial << "[EP] done\n";
    return len;
}