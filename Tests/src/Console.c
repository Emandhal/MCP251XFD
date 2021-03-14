/*******************************************************************************
  File name:    Console.c
  Author:       FMA
  Version:      1.0
  Date (d/m/y): 08/12/2017
  Description:  Some functions for RS-232 console communication

  History :
*******************************************************************************/

//-----------------------------------------------------------------------------
#include "stdarg.h"
#include "stdio.h"
#include "Main.h"
#include "Console.h"
#include "string.h"
//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
#include "stdafx.h"
extern "C" {
  void SetConsoleColor(int text, int fond)
  {// 0: noir          8: gris
   // 1: bleu foncé    9: bleu
   // 2: vert         10: vert fluo
   // 3: bleu-gris    11: turquoise
   // 4: marron       12: rouge
   // 5: pourpre      13: rose fluo
   // 6: kaki         14: jaune fluo
   // 7: gris clair   15: blanc
    HANDLE H = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(H, (fond << 4) + text);
  }
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//=============================================================================
// Initialize the Console Reception
//=============================================================================
void InitConsoleRx(ConsoleRx* pApi)
{
  if (pApi == NULL) return;
  if (pApi->fnInterfaceInit == NULL) return;

  pApi->fnInterfaceInit(pApi);

  // Initialize the print buffer
  if (pApi->Buffer == NULL) return;
  if (pApi->BufferSize == 0) return;
  pApi->InPos  = 0;
  pApi->OutPos = 0;
  memset(pApi->Buffer, 0, (sizeof(pApi->Buffer[0]) * pApi->BufferSize));
}



//**********************************************************************************************************************************************************
//=============================================================================
// Initialize the Console Transmit
//=============================================================================
void InitConsoleTx(ConsoleTx* pApi)
{
  if (pApi == NULL) return;
  if (pApi->fnInterfaceInit == NULL) return;

  pApi->fnInterfaceInit(pApi);

  // Initialize the print buffer
  if (pApi->Buffer == NULL) return;
  if (pApi->BufferSize == 0) return;
  pApi->InPos  = 0;
  pApi->OutPos = 0;
  memset(pApi->Buffer, 0, (sizeof(pApi->Buffer[0]) * pApi->BufferSize));
}



//**********************************************************************************************************************************************************
//=============================================================================
// Set char to print buffer
//=============================================================================
void SetCharToConsoleBuffer(ConsoleTx* pApi, const char aChar)
{
  if (pApi == NULL) return;
  if (pApi->Buffer == NULL) return;
  if (aChar == '\0') return;

  bool BufferFull = false;
  // Check buffer full
  if (pApi->OutPos == 0) BufferFull = (pApi->InPos == (pApi->BufferSize - 1));
  else BufferFull = (pApi->InPos == (pApi->OutPos - 1));
  if (BufferFull)
    while (!TrySendingNextCharToConsole(pApi)) // If buffer full force sending next char
      if (pApi->InPos == pApi->OutPos) break;  // But both InPos and OutPos should not be at the same position
  // Store the new char
  pApi->Buffer[pApi->InPos++] = aChar;
  if (pApi->InPos >= pApi->BufferSize) pApi->InPos = 0;
}



//=============================================================================
// Set char array to print buffer
//=============================================================================
void SetStrToConsoleBuffer(ConsoleTx* pApi, const char* string)
{
  while (*string != '\0')
  {
    SetCharToConsoleBuffer(pApi, *string);
    string++;
  }
}



//=============================================================================
// Try to send next char in the console print buffer
//=============================================================================
bool TrySendingNextCharToConsole(ConsoleTx* pApi)
{
  if (pApi == NULL) return false;
  if (pApi->fnSendChar == NULL) return false;
  if (pApi->Buffer == NULL) return false;
  bool Result = false;

  if (pApi->OutPos != pApi->InPos)
  {
    uint8_t CharToSend = pApi->Buffer[pApi->OutPos];
    if (pApi->fnSendChar(pApi, CharToSend))
    {
      pApi->Buffer[pApi->OutPos] = 0;
      pApi->OutPos++;
      if (pApi->OutPos >= pApi->BufferSize) pApi->OutPos = 0;
      Result = true;
    }
  }
  return Result;
}



//**********************************************************************************************************************************************************
//=============================================================================
// Send a formated Logs to console (DO NOT USE DIRECTLY, use LOG*() instead)
//=============================================================================
void __LOG(ConsoleTx* pApi, const char* context, bool whiteText, const char* format, va_list args)
{
  const char* FormatLine = "%s [%u:%02u:%02u:%02u] ";

  // Fast div 1000 (+/- one unit error is not critical for logging purpose)
  uint64_t Val = msCount;
  uint32_t Time = (uint32_t)((Val * 0x00418937) >> 32); // Magic number : Here 0x418937 is 0xFFFFFFFF / 1000d. This is the overflow limit of an uint32
  uint32_t NewTime;
  // Extract fields
  NewTime = (Time / 60); uint32_t Sec = Time - (NewTime * 60); Time = NewTime;
  NewTime = (Time / 60); uint32_t Min = Time - (NewTime * 60); Time = NewTime;
  NewTime = (Time / 24); uint32_t Hor = Time - (NewTime * 24); Time = NewTime;
  uint32_t d = Time;


#ifndef __cplusplus
  char TmpBuff[200];
  siprintf(TmpBuff, FormatLine, context, (unsigned int)d, (unsigned int)Hor, (unsigned int)Min, (unsigned int)Sec);
  SetStrToConsoleBuffer(pApi, TmpBuff);
  if (whiteText) SetStrToConsoleBuffer(pApi, "\x001B[0m");
  vsiprintf(TmpBuff, format, args);
  SetStrToConsoleBuffer(pApi, TmpBuff);
  SetStrToConsoleBuffer(pApi, "\r\n");
  TrySendingNextCharToConsole(pApi);
#else
  printf(FormatLine, context, (unsigned int)d, (unsigned int)Hor, (unsigned int)Min, (unsigned int)Sec);
  vprintf(format, args);
  printf("\r\n");
#endif
}



//! Severity line color
#ifdef __cplusplus
const int SeverityColors[(size_t)lsLast_] =
{
  lsTitle   = 10, // lsTitle   -> Color: Text=green  ; Background=black
  lsFatal   = 12, // lsFatal   -> Color: Text=red    ; Background=black
  lsError   = 12, // lsError   -> Color: Text=red    ; Background=black
  lsWarning = 14, // lsWarning -> Color: Text=yellow ; Background=black
  lsInfo    = 11, // lsInfo    -> Color: Text=blue   ; Background=black
  lsTrace   = 15, // lsTrace   -> Color: Text=white  ; Background=black
  lsDebug   =  8, // lsDebug   -> Color: Text=grey   ; Background=black
  lsSpecial =  6, // lsSpecial -> Color: Text=kaki   ; Background=black
};
#else
const char* SeverityColors[(size_t)lsLast_] =
{
  "\x001B[1;32m", // lsTitle   -> Color: Text=green          ; Background=black ; Bold
  "\x001B[1;91m", // lsFatal   -> Color: Text=red bright     ; Background=black ; Bold
  "\x001B[0;91m", // lsError   -> Color: Text=red bright     ; Background=black
  "\x001B[0;93m", // lsWarning -> Color: Text=yellow bright  ; Background=black
  "\x001B[0;36m", // lsInfo    -> Color: Text=cyan           ; Background=black
  "\x001B[0;97m", // lsTrace   -> Color: Text=white          ; Background=black
  "\x001B[0;37m", // lsDebug   -> Color: Text=grey "white"   ; Background=black
  "\x001B[0;33m", // lsSpecial -> Color: Text=yellow         ; Background=black
};
#endif



//=============================================================================
// Send a formated Logs to console
//=============================================================================
void LOG(ConsoleTx* pApi, eSeverity severity, const char* format, ...)
{
  va_list args;
  va_start(args, format);

#ifdef __cplusplus
  SetConsoleColor(SeverityColors[(size_t)severity], 0);
#else
  SetStrToConsoleBuffer(pApi, SeverityColors[(size_t)severity]);
#endif

  bool KeepColorFor = (severity == lsFatal) | (severity == lsDebug);
  __LOG(pApi, "DEMO", !KeepColorFor, format, args);

  va_end(args);
}

#ifdef __cplusplus
//=============================================================================
// Send a formated Simulation Logs to console
//=============================================================================
void LOGSIM(ConsoleTx* pApi, const char* format, ...)
{
  va_list args;
  va_start(args, format);

#ifdef __cplusplus
  SetConsoleColor(3, 0);                      // Color: Text=blue-grey ; Background=black
#else
  SetStrToConsoleBuffer(pApi,"\x001B[0;96m"); // Color: Text=cyan bright ; Background=black
#endif

  __LOG(pApi, "SIMU", false, format, args);

  va_end(args);
}
#endif





//**********************************************************************************************************************************************************
#if (defined(DEBUG) || defined(_DEBUG))
//=============================================================================
// Show the hexadecimal dump of the memory to console
//=============================================================================
void __HexDump(ConsoleTx* pApi, const char* context, const void* src, unsigned int size)
{
  static const char* Hexa = "0123456789ABCDEF";

  #define ROW_LENGTH  16         // 16 bytes per row
  char HexaDump[ROW_LENGTH * 3]; // [2 digit hexa + space] - 1 space + 1 zero terminal
  char HexaChar[ROW_LENGTH + 1]; // [1 char] + 1 zero terminal

  LOGDEBUG_(pApi, "Dump %d bytes at 0x%08X - %s", size, (unsigned int)src, context);

  unsigned char* pSrc = (unsigned char*)src;
  HexaChar[ROW_LENGTH] = 0;
  for (int32_t i = ((size+ROW_LENGTH-1) / ROW_LENGTH); --i >= 0; pSrc += ROW_LENGTH, size -= ROW_LENGTH)
  {
    memset(HexaDump, ' ', sizeof(HexaDump));
    memset(HexaChar, '.', ROW_LENGTH);
    for (int j = (size >= ROW_LENGTH ? ROW_LENGTH : size); --j >= 0;)
    {
      HexaDump[j * 3 + 0] = Hexa[(pSrc[j] >> 4) & 0xF];
      HexaDump[j * 3 + 1] = Hexa[(pSrc[j] >> 0) & 0xF];
      //HexaDump[j * 3 + 2] = ' ';
      HexaChar[j] = (pSrc[j] < 0x20) ? '.' : pSrc[j];
    }
    HexaDump[ROW_LENGTH * 3 - 1] = 0;

    char TmpBuff[10 + 3 + (ROW_LENGTH * 3) + 2 + (ROW_LENGTH + 1) + 4];
    siprintf(TmpBuff, "  %08X : %s \"%s\"\r\n", (unsigned int)pSrc, HexaDump, HexaChar);
    SetStrToConsoleBuffer(pApi, TmpBuff);
  }
  #undef ROW_LENGTH
}



//=============================================================================
// Show the binary dump of the memory to console
//=============================================================================
void __BinDump(ConsoleTx* pApi, const char* context, const void* src, unsigned int size)
{
  static const char* Bin  = "01";
  static const char* Hexa = "0123456789ABCDEF";

  #define ROW_LENGTH  4          // 4 bytes per row
  char BinDump[ROW_LENGTH * 9]; // [8 digit bin  + space] - 1 space + 1 zero terminal
  char BinHexa[ROW_LENGTH * 3]; // [2 digit hexa + space] - 1 space + 1 zero terminal

  LOGDEBUG_(pApi, "Dump %d bytes at 0x%08X - %s", size, (unsigned int)src, context);

  unsigned char* pSrc = (unsigned char*)src;
  BinHexa[ROW_LENGTH] = 0;
  for (int32_t i = ((size+ROW_LENGTH-1) / ROW_LENGTH); --i >= 0; pSrc += ROW_LENGTH, size -= ROW_LENGTH)
  {
    memset(BinDump, ' ', sizeof(BinDump));
    memset(BinHexa, ' ', sizeof(BinHexa));
    for (int j = (size >= ROW_LENGTH ? ROW_LENGTH : size); --j >= 0;)
    {
      BinDump[j * 9 + 0] = Bin[(pSrc[j] >> 7) & 0x1];
      BinDump[j * 9 + 1] = Bin[(pSrc[j] >> 6) & 0x1];
      BinDump[j * 9 + 2] = Bin[(pSrc[j] >> 5) & 0x1];
      BinDump[j * 9 + 3] = Bin[(pSrc[j] >> 4) & 0x1];
      BinDump[j * 9 + 4] = Bin[(pSrc[j] >> 3) & 0x1];
      BinDump[j * 9 + 5] = Bin[(pSrc[j] >> 2) & 0x1];
      BinDump[j * 9 + 6] = Bin[(pSrc[j] >> 1) & 0x1];
      BinDump[j * 9 + 7] = Bin[(pSrc[j] >> 0) & 0x1];
      //BinDump[j * 3 + 8] = ' ';
      BinHexa[j * 3 + 0] = Hexa[(pSrc[j] >> 4) & 0xF];
      BinHexa[j * 3 + 1] = Hexa[(pSrc[j] >> 0) & 0xF];
      //BinHexa[j * 3 + 2] = ' ';

    }
    BinDump[ROW_LENGTH * 9 - 1] = 0;
    BinHexa[ROW_LENGTH * 3 - 1] = 0;

    char TmpBuff[10 + 3 + (ROW_LENGTH * 9) + 2 + (ROW_LENGTH * 3) + 4];
    siprintf(TmpBuff, "  %08X : %s - %s\r\n", (unsigned int)pSrc, BinDump, BinHexa);
    SetStrToConsoleBuffer(pApi, TmpBuff);
  }
  #undef ROW_LENGTH
}
#endif





//**********************************************************************************************************************************************************
//=============================================================================
// Convert a string to int32
//=============================================================================
int32_t String_ToInt32(char* buff)
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
  if (buff[0] != 0) return 0;             // Error, the last character should be '\0' set result to 0

  if (Sign)                               // If negative value...
    return -IntPart;                      // Then return a negative int32
  return IntPart;                         // In all cases, return
}



//=============================================================================
// Convert a string to int32 by reference
//=============================================================================
char* String_ToInt32ByRef(char* buff, int32_t* Result)
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

  *Result = IntPart;                      // In all cases, set IntPart as result
  if (Sign)                               // If negative value...
    *Result = -IntPart;                   // Then set negative IntPart as result
  return buff;                            // Return the new position
}



//=============================================================================
// Convert a uint32_t to String
//=============================================================================
uint32_t uint32_t_ToStr(uint32_t num, char* buff)
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
// Convert a uint32_t to HexString
//=============================================================================
uint32_t uint32_t_ToHexStr(uint32_t num, char* buff)
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