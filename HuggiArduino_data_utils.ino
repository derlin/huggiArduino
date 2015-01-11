/**
 * @file    HuggiArduino_data_utils.ino
 * @author  Lucy Linder (lucy.derlin@gmail.com)
 * @date    September, 2014
 * @brief   Handle encoding/decoding of data exchanged through the fiber.
 */

/**
 * Compute the CRC8 (cyclic redundancy check, 1 byte) of a string.
 * This code is based on the CRC8 formulas by Dallas/Maxim and released under 
 * the therms of the GNU GPL 3.0 license.
 * See \href  http://www.leonardomiliani.com/en/2013/un-semplice-crc8-per-arduino/ 
 * for more information.
 * @param s the string
 * @return the CRC
 */
int computeChecksum(const char * s)
{
    char len = strlen(s);
    char crc = 0x00;
    while (len--) {
        char extract = *s++;
        for (char i = 8; i; i--) {
            char sum = (crc ^ extract) & 0x01;
            crc >>= 1;
            if (sum) {
                crc ^= 0x8C;
            }
            extract >>= 1;
        }
    }
    return crc;
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
    
    int givenChk = *encoded;     // get the checksum
    int computedChk = computeChecksum(decoded); // compute the checksum

    Serial << "data: " << decoded << " CHK: " << givenChk << " | " << computedChk << ", L is " << length << nl;

    return givenChk == computedChk ? length : 0; // compare checksums  
}
