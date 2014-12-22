/**
 * @file    HuggiArduino_data_utils.ino
 * @author  Lucy Linder (lucy.derlin@gmail.com)
 * @date    September, 2014
 * @brief   Handle encoding/decoding of data exchanged through the fiber.
 *
 */

/**
 * Compute the LRC (longitudinal redundancy check) of a string.
 * This kind of checksum can be expressed as "the 8-bit two's-complement 
 * value of the sum of all bytes modulo 2^8."
 * @param s the string
 * @return the LRC
 */
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

/**
 * Encode the data following the convention:
 *  - length of the data: 1B
 *  - data 
 *  - checksum/LRC: 4B, hexadecimal value, padded with 0s.
 *  @param dest the destination
 *  @param src the data
 *  @note The size of the destination buffer must be at least
 *  size of data + 5B.
 */
void encodeData(char * dest, const char * src)
{
    int length = strlen(src);
    int checksum = computeChecksum(src);

    sprintf(dest, DATA_FORMAT, 
        length, src, checksum);

    Serial << " --> new data is " << dest << nl;

}

// return: the length of the decoded data, 0 if none/error
/**
 * Try to decode the data (see :encode for more details about the format).
 * @param encoded the encoded data
 * @param decoded where to store the decoded data, if any
 * @return the size of the decoded data, or 0 if the decoding process failed.
 */
byte decodeData(const char * encoded, char * decoded)
{
    *decoded = 0; // reset

    int length = *(encoded++); // get the length field of the payload
    
    int i = 0;
    for(; i < length; i++){
        if(*encoded == 0) return 0; // length is incorrect !
        decoded[i] = *(encoded++);  // extract payload
    }
    decoded[i] = 0;
    
    int givenChk;
    sscanf(encoded, "%X", &givenChk);           // get the checksum
    int computedChk = computeChecksum(decoded); // compute the checksum

    Serial << "data: " << decoded << " CHK: " << givenChk << " | " << computedChk << ", L is " << length << nl;

    return givenChk == computedChk ? length : 0; // compare checksums  
}
