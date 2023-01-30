/*******************************************************************************
    File name:    StringTools.c
    Author:       FMA
    Version:      1.0
    Date (d/m/y): 15/02/2021
    Description:  Tool to convert data to string and string to data in fixed
                  buffers

    History :
*******************************************************************************/

//-----------------------------------------------------------------------------
#include "StringTools.h"
//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
#include <cstdint>
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------





//=============================================================================
// Convert a uint32_t to String
//=============================================================================
uint32_t Uint32_ToStr(uint32_t num, char* buff)
{
  char hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
  uint32_t len = 0, k = 0;
  uint32_t NumVal = num;
  do // For every 4 bits
  {
    // Get the equivalent hex digit
    buff[len] = hex[NumVal % 10];
    len++;
    NumVal /= 10;
  } while (NumVal != 0);
  // Since we get the digits in the wrong order reverse the digits in the buffer
  for(; k < (len / 2); k++)
  { // Xor swapping
    buff[k] ^= buff[len - k - 1];
    buff[len - k - 1] ^= buff[k];
    buff[k] ^= buff[len - k - 1];
  }
  // Null terminate the buffer and return the length in digits
  buff[len] = '\0';
  return len;
}



//=============================================================================
// Convert a string to int32 by reference
//=============================================================================
char* String_ToInt32ByRef(char* buff, int32_t* result)
{
  if (buff[0] == 0) return 0;             // Empty string? return 0
  bool Sign = (buff[0] == '-');           // Minus character? Save it
  if (Sign || (buff[0] == '+')) buff++;   // Minus character or Plus character? Go to next one
  if (buff[0] == 0) return 0;             // Empty string? return 0

  int32_t IntPart = 0;                    // Here is the integer part

  // Integer part
  while ((uint32_t)(buff[0] - 0x30) <= 9) // Here if buff[0] = '\0', the result should be > 9 then break the while...
  {
    IntPart *= 10;                        // Multiply the int part by 10
    IntPart += buff[0] - 0x30;            // Add the unit value
    buff++;                               // Next char
  }

  *result = IntPart;                      // In all cases, set IntPart as result
  if (Sign)                               // If negative value...
    *result = -IntPart;                   // Then set negative IntPart as result
  return buff;                            // Return the new position
}



//=============================================================================
// Convert a string to int32
//=============================================================================
int32_t String_ToInt32(char* buff)
{
  int32_t Result = 0;
  (void)String_ToInt32ByRef(buff, &Result);
  return Result;                            // In all cases, return
}





//**********************************************************************************************************************************************************
//=============================================================================
// Convert an uint to hexadecimal string
//=============================================================================
void Uint_ToHexString(uint32_t value, char* buffer, size_t charCount)
{
  const char* HexaChars = "0123456789ABCDEF";
  //--- Transform the value into hex string ---
  do
  {
    charCount--;
    *buffer = HexaChars[(value >> (charCount << 2/*x4*/)) & 0x0F];
    buffer++;
  } while (charCount > 0);
}



//=============================================================================
// Convert an uint8_t buffer to hexadecimal string
//=============================================================================
void Uint8Buffer_ToHexString(const uint8_t* srcBuffer, size_t srcSize, char* destString, size_t destSize)
{
  uint8_t* pSrcBuf = (uint8_t*)srcBuffer;
  size_t BytesToWork = (destSize >> 1);                          // Divide by 2 because it's a hex string and 2 hex chars equals to 1 byte
  BytesToWork = (BytesToWork < srcSize ? BytesToWork : srcSize); // Take the least between the two buffers

  //--- Convert each bytes ---
  while (BytesToWork > 0)
  {
    Uint_ToHexString((uint32_t)(*pSrcBuf), destString, 2);
    destString += 2; // 2 hex chars for 1 byte
    pSrcBuf++;
    BytesToWork--;
  }
}



//=============================================================================
// Convert a uint32_t to HexString
//=============================================================================
uint32_t Uint32_ToHexStr(uint32_t num, char* buff)
{
  char hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' ,'A', 'B', 'C', 'D', 'E', 'F' };
  uint32_t len = 0, k = 0;
  uint32_t NumVal = num;
  do // For every 4 bits
  {
    // Get the equivalent hex digit
    buff[len] = hex[NumVal & 0xF];
    len++;
    NumVal >>= 4;
  } while (NumVal != 0);
  // Since we get the digits in the wrong order reverse the digits in the buffer
  for(; k < len/2 ; k++)
  { // Xor swapping
    buff[k] ^= buff[len - k - 1];
    buff[len - k - 1] ^= buff[k];
    buff[k] ^= buff[len - k - 1];
  }
  // Null terminate the buffer and return the length in digits
  buff[len] = '\0';
  return len;
}



//=============================================================================
// Convert an hexadecimal string to uint32 by reference
//=============================================================================
char* HexString_ToUint32ByRef(const char* srcString, uint32_t* result)
{
  char* pChar = (char*)srcString;
  *result = 0;

  //--- Check start with '0x' ---
  if (pChar[0] == '0')
  {
    if (pChar[1] == 'x') pChar += 2;        // Start with "0x"? Pass these chars
  }

  //--- Extract uint32 from hex string ---
  while (true)
  {
    uint32_t CurChar = (uint32_t)(*pChar);
    if ((uint32_t)(CurChar - 0x30) <= 9u)   // Char in '0'..'9' ? If CurChar = '\0', the result should be > 9 then don't fall here...
    {
      *result <<= 4;
      *result += CurChar - 0x30;
    }
    else
    {
      CurChar &= 0xDF;                      // Transform 'a'..'f' into 'A'..'F'
      if ((uint32_t)(CurChar - 0x41) <= 5u) // Char in 'A'..'F' ? If CurChar = '\0', the result should be > 5 then break the while...
      {
        *result <<= 4;
        *result += CurChar - 0x41 + 10;     // 0x41 for 'A' and add 10 to the value
      }
      else break;
    }
    pChar++;
  }

  return pChar;                             // Return the new position
}



//=============================================================================
// Convert an hexadecimal string to uint32
//=============================================================================
uint32_t HexString_ToUint32(const char* srcString)
{
  uint32_t Result = 0;
  (void)HexString_ToUint32ByRef(srcString, &Result);
  return Result;
}



//=============================================================================
// Convert an hexadecimal string to uint
//=============================================================================
uint32_t HexString_ToUint(const char* srcString, size_t charCount)
{
  char* pChar = (char*)srcString;
  uint32_t Result = 0;

  //--- Extract uint from hex string ---
  while (charCount > 0)
  {
    uint32_t CurChar = (uint32_t)(*pChar);
    if ((uint32_t)(CurChar - 0x30) <= 9u)     // Char in '0'..'9' ?
    {
      Result <<= 4;
      Result += CurChar - 0x30;
    }
    else
    {
      CurChar &= 0xDF;                      // Transform 'a'..'f' into 'A'..'F'
      if ((uint32_t)(CurChar - 0x41) <= 5u) // Char in 'A'..'F' ?
      {
        Result <<= 4;
        Result += CurChar - 0x41 + 10;    // 0x41 for 'A' and add 10 to the value
      }
      else break;
    }
    pChar++;
    charCount--;
  }
  return Result;
}



//=============================================================================
// Convert an hexadecimal string to a uint8_t buffer
//=============================================================================
size_t HexString_ToUint8Buffer(const char* srcString, uint8_t* destBuffer, size_t destSize)
{
  char* pSrcStr = (char*)srcString;
  size_t BytesToWork = destSize;

  //--- Convert each bytes ---
  while (BytesToWork > 0)
  {
    if ((pSrcStr[0] == 0) || (pSrcStr[1] == 0))
      return (destSize - BytesToWork);                     // Need 2 chars on the string else exit the function
    *destBuffer = (uint8_t)HexString_ToUint(pSrcStr, 2);
    destBuffer++;
    pSrcStr += 2;                                          // 2 hex chars for 1 byte
    BytesToWork--;
  }
  return (destSize - BytesToWork);
}





//**********************************************************************************************************************************************************
//=============================================================================
// Convert a string to float
//=============================================================================
float String_ToFloat(char* buff)
{
  if (buff[0] == 0) return 0.0f;                                    // Empty string? return 0.0
  bool Sign = (buff[0] == '-');                                     // Minus character? Save it
  if (Sign || (buff[0] == '+')) buff++;                             // Minus character or Plus character? Go to next one
  if (buff[0] == 0) return 0.0f;                                    // Empty string? return 0.0

  uint32_t IntPart = 0;                                             // Here is the integer part
  uint32_t FracPart = 0;                                            // Here is the decimal part
  uint32_t Pow10 = 1;                                               // Here is the factor of the decimal part

  // Integer part
  while ((uint32_t)(buff[0] - 0x30) <= 9)                           // Here if buff[0] = '\0', the result should be > 9 then break the while...
  {
    IntPart *= 10;                                                  // Multiply the int part by 10
    IntPart += buff[0] - 0x30;                                      // Add the unit value
    buff++;                                                         // Next char
  }

  // Decimal part
  if ((buff[0] == '.') || (buff[0] == ','))                         // Decimal separator ?
  {
    buff++;
    while ((uint32_t)(buff[0] - 0x30) <= 9)                         // Here if buff[0] = '\0', the result should be > 9 then break the while...
    {
      FracPart *= 10;                                               // Multiply the int part by 10
      FracPart += buff[0] - 0x30;                                   // Add the unit value
      buff++;                                                       // Next char
      Pow10 *= 10;                                                  // Multiply the the power divide by 10
    }
  }
  if (buff[0] != 0) return 0;                                       // Error, the last character should be '\0' set result to 0.0

  float Result = (float)IntPart + ((float)FracPart / (float)Pow10); // Here, Pow10 could not be 0 (start from 1 and multiply by 10)
  if (Sign)                                                         // If negative value...
    return -Result;                                                 // Then return a negative float
  return Result;                                                    // In all cases, return
}



//=============================================================================
// Convert a float to string
//=============================================================================
size_t Float_ToString(float Val, char* buff, size_t buffSize, uint32_t IntDigitsMin, uint32_t DecDigits, bool Round)
{ // A dual call of this function can be performed: first call for the size of the conversion and the second (with the buffer sized with the value returned by the first call) for the real conversion
  if (IntDigitsMin < 1) IntDigitsMin = 1; // Force 1 integer digit minimum
  bool Neg = (Val < 0);                   // Negative value ?
  if (Neg) Val = -Val;                    // If it's a negative value, set it positive

  int64_t iVal = 0;
  float Rounding = 0.0f;

  if (Round)                                                     // If value need to be rounded
  {
    Rounding = 5.0f;
    for (int32_t z = (DecDigits + 1); --z >= 0;) Rounding /= 10; // Set the rounding value
  }

  //--- Calculate the decimal multiplier ---
  float Mul = 1.0f;
  for (int32_t zz = DecDigits; --zz >= 0;) Mul *= 10;            // Count the number of decimal digits
  // Set the full value into a int64 and discard unwanted decimals digits
  iVal = (int64_t)((Val + Rounding) * Mul);

  //--- Calculate size needed ---
  uint32_t IntSize, SizeNeeded = 0;
  for (int64_t tmpVal = iVal; tmpVal > 0; tmpVal /= 10) SizeNeeded++;                   // Calculate spaces for digits
  if (SizeNeeded >= DecDigits) IntSize = SizeNeeded - DecDigits; else IntSize = 0;      // Set the integer digits count
  if (IntSize == 0) IntSize++;                                                          // Integer digits count should be a minimum of 1
  if (Neg) IntSize++;                                                                   // The negative char count for a digit in the integer part
  if (SizeNeeded < (IntDigitsMin + DecDigits)) SizeNeeded = (IntDigitsMin + DecDigits); // If not enough integer and decimal digits, and the good amount of digits
  if (DecDigits > 0) SizeNeeded++;                                                      // If there is some decimal digits, add one for the decimal separator
  if (Neg && (IntSize > IntDigitsMin)) SizeNeeded++;                                    // If negative value and the minimum int size is already reach, need a char for the '-'
  SizeNeeded++;                                                                         // Add one for the '\0' string terminal character

  //--- Fill string if possible ---
  if (buffSize >= SizeNeeded)                                                                              // Fill buffer if the size is good
  {
    size_t DigitCount = SizeNeeded;                                                                        // Get the digits count
    for (uint32_t Space = IntSize; Space < IntDigitsMin; Space++) { buff[0] = ' '; buff++; DigitCount--; } // Add ' ' char until the integer digit minimum is reach
    if (Neg) { buff[0] = '-'; buff++; DigitCount--; }                                                      // Add the '-' char if negative

    // Add digits
    buff += DigitCount - 1;                        // Go to end of buffer
    buff[0] = 0;                                   // Add the \0 string terminal
    int64_t tmpVal = iVal;                         // Get full value integer
    for (int32_t Digit = DigitCount; --Digit > 0;)
    {
      buff--;                                      // Decrement buff
      buff[0] = (char)((tmpVal % 10) + 0x30);
      tmpVal /= 10;
      if (DecDigits == 1)
      {
        buff--;                                    // Decrement buff
        buff[0] = '.';                             // Add the decimal separator
        Digit--;
      }
      if (DecDigits > 0) DecDigits--;
    }
  }
  return SizeNeeded; // return the final size
}





//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------