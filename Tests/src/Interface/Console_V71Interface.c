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

//-----------------------------------------------------------------------------
#include "Conf_MCP251XFD.h"
#include "Console.h"
#include "Console_V71Interface.h"
//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifndef __cplusplus
#  include <asf.h>
#else
#  include <cstdint>
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------





//! The current Command Input buffer
CommandInputBuf CommandInput;



//=============================================================================
// Handler for Console USART interrupt.
//=============================================================================
void USART1_Handler(void)
{
  //--- Transmission interrupts ---
  if ((CONSOLE_UART->US_CSR & (US_CSR_TXRDY | US_CSR_TXEMPTY)) > 0) // Transmit interrupt rises
  {
    CONSOLE_UART->US_IDR = (US_IDR_TXRDY | US_IDR_TXEMPTY);         // Disable interrupts
    TrySendingNextCharToConsole(CONSOLE_TX);
  }


  //--- Reception interrupts ---
  if ((CONSOLE_UART->US_CSR & US_CSR_RXRDY) > 0)                    // Receive interrupt rises
  {
    char ReceivedChar;
    ConsoleRx_GetChar_V71(CONSOLE_RX, &ReceivedChar);

    switch (ReceivedChar)                                           // Do something with the current character received
    {
      case 0x2A: // '*'                                             // New command showed up
        CommandInput.BufPos = 0;                                    // Initialize buffer position
        CommandInput.Buffer[CommandInput.BufPos] = ReceivedChar;    // Set Command char
        CommandInput.BufPos++;                                      // Select next char
        break;
      case 0x0A: // '\n'
      case 0x0D: // '\r'
        CommandInput.Buffer[CommandInput.BufPos] = '\0';            // Set the end of string
        CommandInput.ToProcess = true;                              // Frame have to be processed as soon as possible (in the main loop). Not now, we are in an interrupt...
        break;
      default:
        CommandInput.Buffer[CommandInput.BufPos] = ReceivedChar;    // Set Command char
        CommandInput.BufPos++;                                      // Select next char
        if (CommandInput.BufPos >= COMMAND_BUFFER_SIZE)             // Little protection...
          CommandInput.BufPos = 0;
        break;
    }

    //--- For echo ---
    if ((ReceivedChar != '\r') && (ReceivedChar != '\n'))
      SetCharToConsoleBuffer(CONSOLE_TX, ReceivedChar);             // Display received char
  }
}





//**********************************************************************************************************************************************************





ConsoleTx Console_Tx_Conf =
{
  .UserAPIData = NULL,
  //--- Interface driver call functions ---
  .fnInterfaceInit = ConsoleTx_InterfaceInit_V71,
  .fnSendChar      = ConsoleTx_SendChar_V71,
  //--- Transmit buffer ---
  .BufferSize = CONSOLE_TX_BUFFER_SIZE,
  .Buffer     = &ConsoleTxBuffer[0],
};





//=============================================================================
// Console Tx interface configuration for the ATSAMV71
//=============================================================================
void ConsoleTx_InterfaceInit_V71(ConsoleTx *pApi)
{
  usart_serial_options_t uart_serial_options = {};
  uart_serial_options.baudrate   = CONF_CONSOLE_BAUDRATE;
  uart_serial_options.charlength = CONF_CONSOLE_CHAR_LENGTH;
  uart_serial_options.paritytype = CONF_CONSOLE_PARITY;
  uart_serial_options.stopbits   = CONF_CONSOLE_STOP_BITS;

  //--- Enable the peripheral clock in the PMC ---
  sysclk_enable_peripheral_clock(CONSOLE_UART_ID);

  //--- Configure console UART ---
//  usart_serial_init(CONSOLE_UART, &uart_serial_options); // For Console API
  stdio_serial_init(CONSOLE_UART, &uart_serial_options); // For printf

  //--- Enable Tx function ---
  usart_enable_tx(CONSOLE_UART);

  //--- Configure and enable interrupt of USART ---
  usart_disable_interrupt(CONSOLE_UART, US_IER_TXRDY | US_IER_TXEMPTY);
  NVIC_EnableIRQ(USART1_IRQn);                                          // *** USE WITH INTERRUPT CHAR SEND. IN SEND WHILE IDLE (while(true) in the main()) COMMENT THIS LINE
}



//=============================================================================
// Console send char to UART for the ATSAMV71
//=============================================================================
bool ConsoleTx_SendChar_V71(ConsoleTx *pApi, char charToSend)
{
  if ((CONSOLE_UART->US_CSR & US_CSR_TXRDY) == 0) return false; // Character is in the US_THR
  if ((CONSOLE_UART->US_CSR & US_CSR_TXEMPTY) == 0) return false;
//  if ((CONSOLE_UART->US_IMR & US_IMR_TXRDY) >  0) return false; // TX Ready interrupt is set  // *** USE WITH INTERRUPT CHAR SEND. IN SEND WHILE IDLE (while(true) in the main()) COMMENT THIS LINE

  CONSOLE_UART->US_THR = US_THR_TXCHR(charToSend);        // Send the char
  CONSOLE_UART->US_IER = (US_IER_TXRDY | US_IER_TXEMPTY); // Enable interrupts
  return true;
}





//**********************************************************************************************************************************************************





ConsoleRx Console_Rx_Conf =
{
  .UserAPIData = NULL,
  //--- Interface driver call functions ---
  .fnInterfaceInit = ConsoleRx_InterfaceInit_V71,
  .fnGetChar       = ConsoleRx_GetChar_V71,
  //--- Transmit buffer ---
  .BufferSize = CONSOLE_RX_BUFFER_SIZE,
  .Buffer     = &ConsoleRxBuffer[0],
};





//=============================================================================
// Console Rx interface configuration for the ATSAMV71
//=============================================================================
void ConsoleRx_InterfaceInit_V71(ConsoleRx *pApi)
{
  usart_serial_options_t uart_serial_options = {};
  uart_serial_options.baudrate   = CONF_CONSOLE_BAUDRATE;
  uart_serial_options.charlength = CONF_CONSOLE_CHAR_LENGTH;
  uart_serial_options.paritytype = CONF_CONSOLE_PARITY;
  uart_serial_options.stopbits   = CONF_CONSOLE_STOP_BITS;

  //--- Enable the peripheral clock in the PMC ---
  sysclk_enable_peripheral_clock(CONSOLE_UART_ID);

  //--- Configure console UART ---
//  usart_serial_init(CONSOLE_UART, &uart_serial_options); // For Console API
  stdio_serial_init(CONSOLE_UART, &uart_serial_options); // For printf

  //--- Enable Rx function ---
  usart_enable_rx(CONSOLE_UART);

  //--- Configure and enable interrupt of USART ---
  usart_enable_interrupt(CONSOLE_UART, US_IER_RXRDY);
  NVIC_EnableIRQ(USART1_IRQn);
}



//=============================================================================
// Console get char from UART of the ATSAMV71
//=============================================================================
bool ConsoleRx_GetChar_V71(ConsoleRx *pApi, char *charReceived)
{
  if ((CONSOLE_UART->US_CSR & US_CSR_RXRDY) == 0) return false;

  *charReceived = (char)(CONSOLE_UART->US_RHR & US_RHR_RXCHR_Msk); // Get the char
  return true;
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