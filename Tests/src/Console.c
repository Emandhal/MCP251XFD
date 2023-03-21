/*!*****************************************************************************
 * @file    Console.c
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.0.0
 * @date    08/12/2017
 * @brief   Some functions for RS-232 console communication
 ******************************************************************************/

//-----------------------------------------------------------------------------
#include "stdarg.h"
#include "stdio.h"
#include "Main.h"
#include "Console.h"
#include "string.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus
#include "stdafx.h"
extern "C" {
#endif
//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
#ifdef USE_CONSOLE_RX


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

//-----------------------------------------------------------------------------
#endif /* USE_CONSOLE_RX */





//**********************************************************************************************************************************************************
#ifdef USE_CONSOLE_TX


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
  lsTitle   = wccLIME  , // lsTitle   -> Color: Text=green  ; Background=black
  lsFatal   = wccRED   , // lsFatal   -> Color: Text=red    ; Background=black
  lsError   = wccRED   , // lsError   -> Color: Text=red    ; Background=black
  lsWarning = wccYELLOW, // lsWarning -> Color: Text=yellow ; Background=black
  lsInfo    = wccAQUA  , // lsInfo    -> Color: Text=blue   ; Background=black
  lsTrace   = wccWHITE , // lsTrace   -> Color: Text=white  ; Background=black
  lsDebug   = wccGRAY  , // lsDebug   -> Color: Text=grey   ; Background=black
  lsSpecial = wccOLIVE , // lsSpecial -> Color: Text=kaki   ; Background=black
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
  SetConsoleColor(SeverityColors[(size_t)severity], wccBLACK);
#else
  SetStrToConsoleBuffer(pApi, SeverityColors[(size_t)severity]);
#endif

  bool KeepColorFor = (severity == lsFatal) || (severity == lsDebug);
  __LOG(pApi, "DEMO", !KeepColorFor, format, args);

  va_end(args);
}



#ifdef __cplusplus
//=============================================================================
// Set the Windows console color
//=============================================================================
void SetConsoleColor(eWinConsoleColor text, eWinConsoleColor background)
{
  HANDLE H = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(H, ((int)background << 4) + (int)text);
}



//=============================================================================
// Send a formated Simulation Logs to console
//=============================================================================
void LOGSIM(ConsoleTx* pApi, const char* format, ...)
{
  va_list args;
  va_start(args, format);

#ifdef __cplusplus
  SetConsoleColor(wccTEAL, wccBLACK);         // Color: Text=blue-grey ; Background=black
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

//-----------------------------------------------------------------------------
#endif /* USE_CONSOLE_TX */





//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif