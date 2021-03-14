/*******************************************************************************
    File name:    Console_V71InterfaceSync.h
    Author:       FMA
    Version:      1.0
    Date (d/m/y): 28/04/2020
    Description:  Console interface for the Console Transmit
                  This unit interface the Console API with the current hardware
                  This interface implements the synchronous use of the API on a SAMV71
                  and is also specific with the SAMV71 Xplained Ultra board
    History :
*******************************************************************************/
#ifndef CONSOLE_V71INTERFACESYNC_H_INC
#define CONSOLE_V71INTERFACESYNC_H_INC
//=============================================================================

//-----------------------------------------------------------------------------
#include <stdlib.h>
//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------



//! Log Title, use it instead of LOG!
#define LOGTITLE(format, ...)           LOG(CONSOLE_TX, lsTitle, format, ##__VA_ARGS__)
//! Log Fatal, use it instead of LOG!
#define LOGFATAL(format, ...)           LOG(CONSOLE_TX, lsFatal, format, ##__VA_ARGS__)
//! Log Error, use it instead of LOG!
#define LOGERROR(format, ...)           LOG(CONSOLE_TX, lsError, format, ##__VA_ARGS__)
//! Log Warning, use it instead of LOG!
#define LOGWARN(format, ...)            LOG(CONSOLE_TX, lsWarning, format, ##__VA_ARGS__)
//! Log Information, use it instead of LOG!
#define LOGINFO(format, ...)            LOG(CONSOLE_TX, lsInfo, format, ##__VA_ARGS__)
//! Log Trace, use it instead of LOG!
#define LOGTRACE(format, ...)           LOG(CONSOLE_TX, lsTrace, format, ##__VA_ARGS__)
//! Log Debug, use it instead of LOG!
#	define LOGDEBUG(format, ...)          LOG(CONSOLE_TX, lsDebug, format, ##__VA_ARGS__)
//! Log Special, use it instead of LOG!
#	define LOGSPECIAL(format, ...)        LOG(CONSOLE_TX, lsSpecial, format, ##__VA_ARGS__)
//! Hexadecimal dump of memory
#define HEXDUMP(context, src, size)     __HexDump(CONSOLE_TX, context, src, size)
//! Binary dump of memory
#define BINDUMP(context, src, size)     __BinDump(CONSOLE_TX, context, src, size)





//**********************************************************************************************************************************************************





//! Define the console transmission buffer size, must be determined according to the max length of a string and the UART speed
#define CONSOLE_TX_BUFFER_SIZE  100

//! The actual console transmission buffer
char ConsoleTxBuffer[CONSOLE_TX_BUFFER_SIZE];

//! The console transmission configuration
extern ConsoleTx Console_Tx_Conf;

//! Define to simplify the naming at the functions calling
#define CONSOLE_TX  &Console_Tx_Conf





//! Command buffer size
#define COMMAND_BUFFER_SIZE   10

/*! @brief Command Input buffer
 *
 * This structure contains infos concerning the input data for the received command
 */
typedef struct
{
	volatile uint32_t BufPos;         //!< Position in the buffer
	char Buffer[COMMAND_BUFFER_SIZE]; //!< Raw buffer with the frame to be processed
	volatile bool ToProcess;          //!< Indicate that the frame in buffer should be processed or not
} CommandInputBuf;

//! The current Command Input buffer
extern CommandInputBuf CommandInput;



//-----------------------------------------------------------------------------



/*! @brief Console Tx interface configuration for the ATSAMV71
 *
 * This function will be called at API initialization to configure the interface driver (UART)
 * @param[in] *pApi Is the pointed structure of the API that call this function
 */
void ConsoleTx_InterfaceInit_V71(ConsoleTx *pApi);



/*! @brief Console send char to UART for the ATSAMV71
 *
 * This function will be called when a character have to be send through interface
 * @param[in] *pApi Is the pointed structure of the API that call this function
 * @param[in] charToSend Is the char to send through the interface
 * @return If the char have been send or not
 */
bool ConsoleTx_SendChar_V71(ConsoleTx *pApi, char charToSend);





//**********************************************************************************************************************************************************





//! Define the console reception buffer size, must be determined according to the max length of a string that will be received
#define CONSOLE_RX_BUFFER_SIZE  100

//! The actual console reception buffer
char ConsoleRxBuffer[CONSOLE_RX_BUFFER_SIZE];

//! The console reception configuration
extern ConsoleRx Console_Rx_Conf;

//! Define to simplify the naming at the functions calling
#define CONSOLE_RX  &Console_Rx_Conf



//-----------------------------------------------------------------------------



/*! @brief Console Rx interface configuration for the ATSAMV71
 *
 * This function will be called at API initialization to configure the interface driver (UART)
 * @param[in] *pApi Is the pointed structure of the API that call this function
 */
void ConsoleRx_InterfaceInit_V71(ConsoleRx *pApi);



/*! @brief Console get char from UART of the ATSAMV71
 *
 * This function will be called when a character have to be get from the interface
 * @param[in] *pApi Is the pointed structure of the API that call this function
 * @param[out] *charToSend Is the char received from the interface
 * @return If the char have been send or not
 */
bool ConsoleRx_GetChar_V71(ConsoleRx *pApi, char *charReceived);





//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------
#endif /* CONSOLE_V71INTERFACESYNC_H_INC */