/*******************************************************************************
    File name:    MCP251XFD_V71InterfaceSync.h
    Author:       FMA
    Version:      1.0
    Date (d/m/y): 15/04/2020
    Description:  MCP251XFD interface for driver
                  This unit interface the MCP251XFD driver with the current hardware
                  This interface implements the synchronous use of the driver on a SAMV71
                  and is also specific with the SAMV71 Xplained Ultra board
    History :
*******************************************************************************/
#ifndef MCP251XFD_V71INTERFACESYNC_H_INC
#define MCP251XFD_V71INTERFACESYNC_H_INC
//=============================================================================

//-----------------------------------------------------------------------------
#include <stdlib.h>
#include "CRC16_CMS.h"
#include "Main.h"
//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------



//! Chip select (nCSx)
#define SPI_CS_EXT1       ( 1 )    // SPI_nCS1 for the MCP2517FD Click on EXT1
//! Chip select (nCSx)
#define SPI_CS_EXT2       ( 3 )    // SPI_nCS3 for the MCP2517FD Click on EXT2
//! Clock polarity
#define SPI_CLK_POLARITY  ( 0 )    // Mode 0,0 or Mode 1,1
//! Clock phase
#define SPI_CLK_PHASE     ( 1 )    // Mode 0,0 or Mode 1,1
//! Delay before SPCK
#define SPI_DLYBS         ( 0x01 ) // Tspick/2 needed
//! Delay between consecutive transfers
#define SPI_DLYBCT        ( 0x01 ) // To conform last SCK rise to nCS rise time (1 Tspick)
//! Delay Between Chip Selects
#define SPI_DLYBCS        ( 0x01 ) // To conform 1 Tspick needed



#define TIMEOUT_SPI_INTERFACE  ( 15000 )



//-----------------------------------------------------------------------------





/*! @brief MCP251XFD_Ext1 INT pin configuration for the ATSAMV71
 *
 * This function will be called at driver initialization to configure the INT pin to input and more by the user if necessary
 * @param[in] *pComp Is the pointed structure of the device that call this function
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT MCP251XFD_Ext1_IntPinInit_V71(MCP251XFD *pComp);



/*! @brief MCP251XFD_Ext1 INT interrupt handler
 *
 * This function will be called when an interrupt on the pin linked to the INT pin of the MCP251XFD_Ext1 (an interrupt in the MCP251XFD_Ext1 occur)
 * @param[in] id Is the PIO identifier (PIOA, PIOB, PIOC, PIOD or PIOE)
 * @param[in] mask Is the mask of the pin where the interrupt occur
 * @return Returns an #eERRORRESULT value enum
 */
void MCP251XFD_Ext1_INT_Handler(uint32_t id, uint32_t mask);

//*****************************************************************************



/*! @brief MCP251XFD_Ext1 INT0 pin configuration for the ATSAMV71
 *
 * This function will be called at driver initialization to configure the INT0 (GPIO0) pin to input and more by the user if necessary
 * @param[in] *pComp Is the pointed structure of the device that call this function
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT MCP251XFD_Ext1_Int0Gpio0PinInit_V71(MCP251XFD *pComp);



/*! @brief MCP251XFD_Ext1 INT0 interrupt handler
 *
 * This function will be called when an interrupt on the pin linked to the INT0 pin of the MCP251XFD_Ext1 (a TX interrupt in the MCP251XFD_Ext1 occur)
 * @param[in] id Is the PIO identifier (PIOA, PIOB, PIOC, PIOD or PIOE)
 * @param[in] mask Is the mask of the pin where the interrupt occur
 * @return Returns an #eERRORRESULT value enum
 */
void MCP251XFD_Ext1_INT0_Handler(uint32_t id, uint32_t mask);

//*****************************************************************************



/*! @brief MCP251XFD_Ext1 INT1 pin configuration for the ATSAMV71
 *
 * This function will be called at driver initialization to configure the INT1 (GPIO1) pin to input and more by the user if necessary
 * @param[in] *pComp Is the pointed structure of the device that call this function
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT MCP251XFD_Ext1_Int1Gpio1PinInit_V71(MCP251XFD *pComp);



/*! @brief MCP251XFD_Ext1 INT1 interrupt handler
 *
 * This function will be called when an interrupt on the pin linked to the INT1 pin of the MCP251XFD_Ext1 (a RX interrupt in the MCP251XFD_Ext1 occur)
 * @param[in] id Is the PIO identifier (PIOA, PIOB, PIOC, PIOD or PIOE)
 * @param[in] mask Is the mask of the pin where the interrupt occur
 * @return Returns an #eERRORRESULT value enum
 */
void MCP251XFD_Ext1_INT1_Handler(uint32_t id, uint32_t mask);

//********************************************************************************************************************





/*! @brief MCP251XFD_Ext2 INT pin configuration for the ATSAMV71
 *
 * This function will be called at driver initialization to configure the INT pin to input and more by the user if necessary
 * @param[in] *pComp Is the pointed structure of the device that call this function
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT MCP251XFD_Ext2_IntPinInit_V71(MCP251XFD *pComp);



/*! @brief MCP251XFD_Ext2 INT interrupt handler
 *
 * This function will be called when an interrupt on the pin linked to the INT pin of the MCP251XFD_Ext2 (an interrupt in the MCP251XFD_Ext2 occur)
 * @param[in] id Is the PIO identifier (PIOA, PIOB, PIOC, PIOD or PIOE)
 * @param[in] mask Is the mask of the pin where the interrupt occur
 * @return Returns an #eERRORRESULT value enum
 */
void MCP251XFD_Ext2_INT_Handler(uint32_t id, uint32_t mask);

//*****************************************************************************



/*! @brief MCP251XFD_Ext2 INT0 pin configuration for the ATSAMV71
 *
 * This function will be called at driver initialization to configure the INT0 (GPIO0) pin to input and more by the user if necessary
 * @param[in] *pComp Is the pointed structure of the device that call this function
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT MCP251XFD_Ext2_Int0Gpio0PinInit_V71(MCP251XFD *pComp);



/*! @brief MCP251XFD_Ext2 INT0 interrupt handler
 *
 * This function will be called when an interrupt on the pin linked to the INT0 pin of the MCP251XFD_Ext2 (a TX interrupt in the MCP251XFD_Ext2 occur)
 * @param[in] id Is the PIO identifier (PIOA, PIOB, PIOC, PIOD or PIOE)
 * @param[in] mask Is the mask of the pin where the interrupt occur
 * @return Returns an #eERRORRESULT value enum
 */
void MCP251XFD_Ext2_INT0_Handler(uint32_t id, uint32_t mask);

//*****************************************************************************



/*! @brief MCP251XFD_Ext2 INT1 pin configuration for the ATSAMV71
 *
 * This function will be called at driver initialization to configure the INT1 (GPIO1) pin to input and more by the user if necessary
 * @param[in] *pComp Is the pointed structure of the device that call this function
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT MCP251XFD_Ext2_Int1Gpio1PinInit_V71(MCP251XFD *pComp);



/*! @brief MCP251XFD_Ext2 INT1 interrupt handler
 *
 * This function will be called when an interrupt on the pin linked to the INT1 pin of the MCP251XFD_Ext2 (a RX interrupt in the MCP251XFD_Ext2 occur)
 * @param[in] id Is the PIO identifier (PIOA, PIOB, PIOC, PIOD or PIOE)
 * @param[in] mask Is the mask of the pin where the interrupt occur
 * @return Returns an #eERRORRESULT value enum
 */
void MCP251XFD_Ext2_INT1_Handler(uint32_t id, uint32_t mask);

//********************************************************************************************************************



/*! @brief MCP251XFD_Ext1 SPI interface configuration for the ATSAMV71
 *
 * This function will be called at driver initialization to configure the interface driver SPI
 * @param[in] *pIntDev Is the MCP251XFD_Desc.InterfaceDevice of the device that call the interface initialization
 * @param[in] chipSelect Is the Chip Select index to use for the SPI initialization
 * @param[in] sckFreq Is the SCK frequency in Hz to set at the interface initialization
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT MCP251XFD_InterfaceInit_V71(void *pIntDev, uint8_t chipSelect, const uint32_t sckFreq);



/*! @brief MCP251XFD_Ext1 SPI transfer for the ATSAMV71
 *
 * This function will be called at driver read/write data from/to the interface driver SPI
 * @param[in] *pIntDev Is the MCP251XFD_Desc.InterfaceDevice of the device that call this function
 * @param[in] chipSelect Is the Chip Select index to use for the SPI transfer
 * @param[in] *txData Is the buffer to be transmit to through the SPI interface
 * @param[out] *rxData Is the buffer to be received to through the SPI interface (can be NULL if it's just a send of data)
 * @param[in] size Is the size of data to be send and received trough SPI. txData and rxData shall be at least the same size
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT MCP251XFD_InterfaceTransfer_V71(void *pIntDev, uint8_t chipSelect, uint8_t *txData, uint8_t *rxData, size_t size);

//********************************************************************************************************************



/*! @brief MCP251XFD_X get millisecond
 *
 * This function will be called when the driver needs to get current millisecond
 */
uint32_t GetCurrentms_V71(void);



/*! @brief MCP251XFD_X compute CRC16-CMS
 *
 * This function will be called when a CRC16-CMS computation is needed (ie. in CRC mode or Safe Write). In normal mode, this can point to NULL.
 * @param[in] *data Is the pointed byte stream
 * @param[in] size Is the size of the pointed byte stream
 * @return The CRC computed
 */
uint16_t ComputeCRC16_V71(const uint8_t* data, size_t size);

//********************************************************************************************************************



/*! @brief Calculate the SPI clock speed in Hz
 *
 * The calculus depend on SPI configuration of the SAMV71
 * @param[in] *pSpi Is the pointed structure of the SPI registers configuration
 * @param[in] aCS Is the SPI Chip Select to work with
 * @return Return the time in nanoseconds
 */
uint32_t GetSPIClock(Spi *pSpi, uint8_t aCS);





#ifdef USE_MCP251XFD_TOOLS
/*! @brief Calculate the min time before the GPIO to change state
 *
 * The calculus depend on SPI configuration of the SAMV71 and driver configuration.
 * Time take by processor between each bytes not taken into account
 * @param[in] *pComp Is the pointed structure of the driver configuration
 * @param[in] *pSpi Is the pointed structure of the SPI registers configuration
 * @param[in] aCS Is the SPI Chip Select to work with
 * @return Return the time in nanoseconds
 */
uint32_t CalculateMinTimeBeforeGPIOChangeState_V71(MCP251XFD *pComp, Spi *pSpi, uint8_t aCS);
#endif





//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------
#endif /* MCP251XFD_V71INTERFACESYNC_H_INC */