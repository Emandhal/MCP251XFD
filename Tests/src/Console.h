/*******************************************************************************
  File name:    Console.h
  Author:       FMA
  Version:      1.0
  Date (d/m/y): 08/12/2017
  Description:  Some functions for RS-232 console communication

  History :
*******************************************************************************/
#ifndef CONSOLE_H_
#define CONSOLE_H_
//=============================================================================

//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#include "stdio.h"
#include <stdarg.h>

#ifndef __cplusplus
# include "asf.h"
#else
extern "C" {
  void SetConsoleColor(int text, int fond);
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------



#ifndef __cplusplus

#	define __FORMATPRINTF23__		__attribute__((__format__(__printf__, 2, 3))) // 2: Format at second argument ; 3: args at third argument (...)
#	define __FORMATPRINTF40__		__attribute__((__format__(__printf__, 4, 0))) // 4: Format at fourth argument ; 0: for va_list
#	define __FORMATPRINTF34__		__attribute__((__format__(__printf__, 3, 4))) // 3: Format at third  argument ; 3: args at fourth argument (...)
#	define vprintf              viprintf

#else

#	define __FORMATPRINTF12__
#	define __FORMATPRINTF20__
#	define __FORMATPRINTF23__

#endif
//-----------------------------------------------------------------------------


//********************************************************************************************************************
// Console Receive API
//********************************************************************************************************************

//! Circular Buffer for Console transmit structure
typedef struct ConsoleRx ConsoleRx;
struct ConsoleRx
{
  void *UserAPIData;                                    //!< Optional, can be used to store API data or NULL

  //--- Interface driver call functions ---
  void (*fnInterfaceInit)(ConsoleRx *pApi);             //!< This function will be called at API initialization to configure the interface driver (UART)
  bool (*fnGetChar)(ConsoleRx *pApi, char *charToSend); //!< This function will be called when a character have to be get from the interface

  //--- Transmit buffer ---
  volatile size_t InPos;                                //!< This is the input position in the buffer (where data will be write before being send to UART)
  volatile size_t OutPos;                               //!< This is the output position in the buffer (where data will be read and send to UART)
  size_t BufferSize;                                    //!< The buffer size
  char *Buffer;                                         //!< The buffer itself (should be the same size as BufferSize)
};
//-----------------------------------------------------------------------------



/*! @brief Initialize the Console receive
 *
 * @param[in] *pApi Is the Console receive API to work with
 */
void InitConsoleRx(ConsoleRx* pApi);

//**********************************************************************************************************************************************************





//********************************************************************************************************************
// Console Transmit API
//********************************************************************************************************************

//! Circular Buffer for Console transmit structure
typedef struct ConsoleTx ConsoleTx;
struct ConsoleTx
{
  void *UserAPIData;                                    //!< Optional, can be used to store API data or NULL

  //--- Interface driver call functions ---
  void (*fnInterfaceInit)(ConsoleTx *pApi);             //!< This function will be called at API initialization to configure the interface driver (UART)
  bool (*fnSendChar)(ConsoleTx *pApi, char charToSend); //!< This function will be called when a character have to be send through interface

  //--- Transmit buffer ---
  volatile size_t InPos;                                //!< This is the input position in the buffer (where data will be write before being send to UART)
  volatile size_t OutPos;                               //!< This is the output position in the buffer (where data will be read and send to UART)
  size_t BufferSize;                                    //!< The buffer size
  char *Buffer;                                         //!< The buffer itself (should be the same size as BufferSize)
};
//-----------------------------------------------------------------------------



/*! @brief Initialize the Console transmit
 *
 * @param[in] *pApi Is the Console transmit API to work with
 */
void InitConsoleTx(ConsoleTx* pApi);

//**********************************************************************************************************************************************************



/*! @brief Set char to print buffer
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @param[in] aChar Is the char to be sent
 */
void SetCharToConsoleBuffer(ConsoleTx* pApi, const char aChar);



/*! @brief Set char array to print buffer
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @param[in] *string Pointer to one-line string to be sent (will stop at the first '\0' char)
 */
void SetStrToConsoleBuffer(ConsoleTx* pApi, const char* string);



/*! @brief Try to send next char in the console print buffer
 *
 * This function is for programs that need to send only in a main loop like in a while(true){} in the main
 * @param[in] *pApi Is the Console transmit API to work with
 * @return If the next char is actually sending
 */
bool TrySendingNextCharToConsole(ConsoleTx* pApi);



/*! @brief Indicate if there is a char to send to console
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @return If there is a char to send to console
 */
inline bool IsCharToSendToConsole(ConsoleTx* pApi)
{
  if (pApi == NULL) return false;
  return (pApi->OutPos != pApi->InPos);
}



/*! @brief Convert a char to a digit value
 *
 * @param[in] aChar The char to convert
 * @return return the digit value
 */
inline uint32_t CharToDigit(const char aChar)
{
  return (uint32_t)(aChar - '0');
}



//! Macro to get the lower case of a char
#define LowerCase(x)  (((((unsigned int)(x)) - 'A') < 26) ? (x) + 32 : (x))

//**********************************************************************************************************************************************************



//! Log type, sorted by severity.
typedef enum
{
  lsTitle   = 0, //! Show a title
  lsFatal   = 1, //! Fatal error! application will abort shortly
  lsError   = 2, //! Error! the application may work improperly
  lsWarning = 3, //! Warning! There's something wrong
  lsInfo    = 4, //! For your information, the application is safe
  lsTrace   = 5, //! Trace log only
  lsDebug   = 6, //! For Debugging purpose. Emitted only when DEBUG is defined
  lsSpecial = 7, //! For Debugging purpose. Emitted only when DEBUG is defined
  lsLast_,       //! Special value. Do not use and keep this the last value
} eSeverity;



/*! @brief Send a formated Logs to console
 *
 * @note DO NOT USE DIRECTLY, use LOG*() instead.
 * @param[in] *pApi Is the Console transmit API to work with
 * @param[in] *context Context string (usually, the system name emitting the log).
 * @param[in] whiteText If the text after the time counter change to white
 * @param[in] *format Format string (printf format), followed by arguments.
 * @param[in] args Arguments of the formated string.
 */
void __LOG(ConsoleTx* pApi, const char* context, bool whiteText, const char* format, va_list args) __FORMATPRINTF40__;



/*! @brief Send a formated ESQS+ Logs to console
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @param[in] severity This is the log severity.
 * @param[in] *format Format string (printf format), followed by arguments.
 * @param[in] args Arguments of the formated string.
 */
void LOG(ConsoleTx* pApi, eSeverity severity, const char* format, ...) __FORMATPRINTF34__;



#ifdef __cplusplus
/*! @brief Send a formated Simulation Logs to console
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @param[in] *format Format string (printf format), followed by arguments.
 * @param[in] args Arguments of the formated string.
 */
void LOGSIM(ConsoleTx* pApi, const char* format, ...);
#endif



#if (defined(DEBUG) || defined(_DEBUG))
/*! @brief Show the hexadecimal dump of the memory to console
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @param[in] *context Is the text to show for the dump
 * @param[in] *src This is the source pointer of the begining of data to dump
 * @param[in] size The size of data to dump.
 */
void __HexDump(ConsoleTx* pApi, const char* context, const void* src, unsigned int size);



/*! @brief Show the binary dump of the memory to console
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @param[in] *context Is the text to show for the dump
 * @param[in] *src This is the source pointer of the begining of data to dump
 * @param[in] size The size of data to dump
 */
void __BinDump(ConsoleTx* pApi, const char* context, const void* src, unsigned int size);
#endif





//! Log Title, use it instead of LOG!
#define LOGTITLE_(api, format, ...)             LOG(api, lsTitle, format, ##__VA_ARGS__)
//! Log Fatal, use it instead of LOG!
#define LOGFATAL_(api, format, ...)             LOG(api, lsFatal, format, ##__VA_ARGS__)
//! Log Error, use it instead of LOG!
#define LOGERROR_(api, format, ...)             LOG(api, lsError, format, ##__VA_ARGS__)
//! Log Warning, use it instead of LOG!
#define LOGWARN_(api, format, ...)              LOG(api, lsWarning, format, ##__VA_ARGS__)
//! Log Information, use it instead of LOG!
#define LOGINFO_(api, format, ...)              LOG(api, lsInfo, format, ##__VA_ARGS__)
//! Log Trace, use it instead of LOG!
#define LOGTRACE_(api, format, ...)             LOG(api, lsTrace, format, ##__VA_ARGS__)
#if (defined(DEBUG) || defined(_DEBUG))
  //! Log Debug, use it instead of LOG!
  #define LOGDEBUG_(api, format, ...)           LOG(api, lsDebug, format, ##__VA_ARGS__)
  //! Log Special, use it instead of LOG!
  #define LOGSPECIAL_(api, format, ...)         LOG(api, lsSpecial, format, ##__VA_ARGS__)
  //! Hexadecimal dump of memory
  #define HEXDUMP_(api, context, src, size)     __HexDump(context, src, size)
  //! Binary dump of memory
  #define BINDUMP_(api, context, src, size)     __BinDump(context, src, size)
#else
  #define LOGDEBUG_(api, format, ...)           do{}while(false)
  #define LOGSPECIAL_(api, format, ...)         do{}while(false)
  #define HEXDUM_P(api, context, src, size)     do{}while(false)
  #define BINDUM_P(api, context, src, size)     do{}while(false)
#endif





//********************************************************************************************************************
// Console conversion utils
//********************************************************************************************************************

/*! @brief Convert a String to int32
 *
 * If too many char are present, the result may be uncertain. If a wrong character is found, the result is 0
 * @param[in] *buff Buffer of char where the string int32 is stored with a null terminal
 * @return the converted float value
 */
int32_t String_ToInt32(char* buff);



/*! @brief Convert a string to int32 by reference
 *
 * If too many char are present, the result may be uncertain. If a wrong character is found, the result is 0
 * @param[in] *buff Buffer of char where the string int32 is stored with a null terminal
 * @param[out] *result Is the result of the conversion
 * @return The new char* position
 */
char* String_ToInt32ByRef(char* buff, int32_t* result);



/*! @brief Convert a uint32_t to String
 *
 * @param[in] num The decimal number to be converted
 * @param[out] *buff Buffer of char where the number will be stored with a null terminal
 * @return Length of the buffer
 */
uint32_t uint32_t_ToStr(uint32_t num, char* buff);



/*! @brief Convert a uint32_t to HexString
 *
 * @param[in] num The decimal number to be converted
 * @param[out] *buff Buffer of char where the hex number will be stored with a null terminal
 * @return Length of the buffer
 */
uint32_t uint32_t_ToHexStr(uint32_t num, char* buff);



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
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------
#endif /* CONSOLE_H_ */