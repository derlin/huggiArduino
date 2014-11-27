
int computeChecksum(const char * s)
{
    int checksum = 0;
    const char * p = s;

    while(*p)
    {
        checksum = (checksum + *p) & 0xFF;
        p++;
    }

    checksum = ((checksum ^ 0xFF) + 1) & 0xFF;
    return checksum;
}


void encodeData(char * dest, const char * src)
{
    int length = strlen(src);
    int checksum = computeChecksum(src);

    sprintf(dest, DATA_FORMAT, 
        length, src, checksum);

    Serial << " --> new data is " << dest << nl;

}

// return: the length of the decoded data, 0 if none/error
byte decodeData(const char * encoded, char * decoded)
{
    *decoded = 0; // reset

    int length = *(encoded++); // get length of payload
    // Serial << "LEN " << length << nl;
    
    int i = 0;
    for(; i < length; i++){
        if(*encoded == 0) return 0; // OOPS, length not correct !
        decoded[i] = *(encoded++); // extract payload
    }
    decoded[i] = 0;
    
    int givenChk;
    sscanf(encoded, "%X", &givenChk); // get the checksum
    int computedChk = computeChecksum(decoded); // compute the checksum

    Serial << "data: " << decoded << " CHK: " << givenChk << " | " << computedChk << ", L is " << length << nl;

    return givenChk == computedChk ? length : 0; // compare checksums  
}
