/*!*****************************************************************************
 * @file    StringTools.h
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.0.0
 * @date    15/02/2021
 * @brief   Tool to convert data to string and string to data in fixed buffers
 ******************************************************************************/
#ifndef STRING_TOOLS_H_INC
#define STRING_TOOLS_H_INC
//=============================================================================

//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------



/*! @brief Convert a uint32_t to String
 *
 * @param[in] num The decimal number to be converted
 * @param[out] *buff Buffer of char where the number will be stored with a null terminal
 * @return Length of the buffer
 */
uint32_t Uint32_ToStr(uint32_t num, char* buff);


/*! @brief Convert a string to int32 by reference
 *
 * If too many char are present, the result may be uncertain. If a wrong character is found, the result is the value found and the value returned is a pointer to the wrong character
 * @param[in] *buff Buffer of char where the string int32 is stored with a null terminal
 * @param[out] *result Is the result of the conversion
 * @return The new char* position
 */
char* String_ToInt32ByRef(char* buff, int32_t* result);


/*! @brief Convert a String to int32
 *
 * If too many char are present, the result may be uncertain. If a wrong character is found, the result is 0
 * @param[in] *buff Buffer of char where the string int32 is stored with a null terminal
 * @return the converted float value
 */
int32_t String_ToInt32(char* buff);

//**********************************************************************************************************************************************************



/*! @brief Convert an uint to hexadecimal string
 *
 * If charCount is 2 then it convert the 8 least significant bits, when it's 4 then it convert the 16 least significant bits, when it's 8 then it convert the whole 32-bit unsigned int
 * @param value Is the value to convert
 * @param *buffer Is the string where data will be saved
 * @param charCount Is the count of char available in the destination string, it is also the buffer size
 */
void Uint_ToHexString(uint32_t value, char* buffer, size_t charCount);


/*! @brief Convert an uint8_t buffer to hexadecimal string
 *
 * Convert each bytes from the srcBuffer and save them in the destString. This function takes the least bytes between the two buffers
 * @param *srcBuffer Is the array of byte to convert
 * @param srcSize Is the count of byte to convert in the source buffer
 * @param *destString Is the hexadecimal string where data will be saved
 * @param destSize Is the count of char available in the destination string. If odd then the last char is discarded
 */
void Uint8Buffer_ToHexString(const uint8_t* srcBuffer, size_t srcSize, char* destString, size_t destSize);


/*! @brief Convert a uint32_t to HexString
 *
 * @param[in] num The decimal number to be converted
 * @param[out] *buff Buffer of char where the hex number will be stored with a null terminal
 * @return Length of the buffer
 */
uint32_t Uint32_ToHexStr(uint32_t num, char* buff);


/*! @brief Convert an hexadecimal string to uint32
 *
 * If too many char are present, the result may be uncertain. If a wrong character is found, the function stops
 * @param *srcString Is the hexadecimal string to convert
 * @return Returns the 32-bit unsigned int converted
 */
uint32_t HexString_ToUint32(const char* srcString);


/*! @brief Convert an hexadecimal string to uint32 by reference
 *
 * If too many char are present, the result may be uncertain. If a wrong character is found, the result is the value found and the value returned is a pointer to the wrong character
 * @param[in] *buff Buffer of char where the string int32 is stored with a null terminal
 * @param[out] *result Is the result of the conversion
 * @return The new char* position
 */
char* HexString_ToUint32ByRef(const char* srcString, uint32_t* result);


/*! @brief Convert an hexadecimal string to uint
 *
 * If charCount is 2 then it convert a single byte, when it's 4 then it convert a 16-bit unsigned int, when it's 8 then it convert a 32-bit unsigned int
 * @param *srcString Is the hexadecimal string to convert
 * @param charCount Is the count of char to convert in the buffer into a 32-bit unsigned int
 * @return Returns the 32-bit unsigned int converted
 */
uint32_t HexString_ToUint(const char* srcString, size_t charCount);


/*! @brief Convert an hexadecimal string to a uint8_t buffer
 *
 * Extract each bytes from the srcString and save them in the destBuffer. If the string terminate before the end of buffer then the function terminate
 * @param *srcString Is the hexadecimal string to convert
 * @param *destBuffer Is the byte array where data will be saved
 * @param destSize Is the count of byte available in the destination buffer
 * @return The actual byte count in the buffer
 */
size_t HexString_ToUint8Buffer(const char* srcString, uint8_t* destBuffer, size_t destSize);

//**********************************************************************************************************************************************************



/*! @brief Convert a String to float
 *
 * If too many char are present, the result may be uncertain. If a wrong character is found, the result is 0.0
 * @param[in] *buff Buffer of char where the string float is stored with a null terminal
 * @return the converted float value
 */
float String_ToFloat(char* buff);



/*! @brief Convert a float to string
 *
 * A dual call of this function can be performed: first call for the size of the conversion and the second (with the buffer sized with the value returned by the first call) for the real conversion
 * @param[in] Val Is the float value to convert
 * @param[out] *buff Is the array of char that will contain the final string (remain untouch if buffSize < needed string size)
 * @param[in] buffSize Is the available buffer size
 * @param[in] IntDigitsMin Is the minimum size of the integer part of the float. The negative char count in the int part
 * @param[in] DecDigits Is the digit count of the decimal part
 * @param[in] Round Indicate if the value should be rounded or not (following the DecDigits value)
 * @return The size of the buffer needed for the conversion. It's the conversion final size with the \0 string terminal
 */
size_t Float_ToString(float Val, char* buff, size_t buffSize, uint32_t IntDigitsMin, uint32_t DecDigits, bool Round);





//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* STRING_TOOLS_H_INC */