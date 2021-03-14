/*******************************************************************************
    File name:    MCP251XFD_V71InterfaceSync.c
    Author:       FMA
    Version:      1.0
    Date (d/m/y): 15/04/2020
    Description:  MCP251XFD interface for driver
                  This unit interface the MCP251XFD driver with the current hardware
                  This interface implements the synchronous use of the driver on a SAMV71
                  and is also specific with the SAMV71 Xplained Ultra board

    History :
*******************************************************************************/

//-----------------------------------------------------------------------------
#include "Conf_MCP251XFD.h"
#include "MCP251XFD.h"
#include "MCP251XFD_V71InterfaceSync.h"
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

static bool SPIconfigured = false;

//-----------------------------------------------------------------------------



//=============================================================================
// MCP251XFD_Ext1 INT pin configuration for the ATSAMV71
//=============================================================================
eERRORRESULT MCP251XFD_Ext1_IntPinInit_V71(MCP251XFD *pComp)
{
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  // INT from MCP251xFD board is linked to IRQ pin of EXT1 extension header (pin 9 as PD28)

  // Here it's always an input pin from the microcontroller side
  ioport_set_pin_dir(EXT1_PIN_IRQ, IOPORT_DIR_INPUT);
  ioport_set_pin_mode(EXT1_PIN_IRQ, IOPORT_MODE_PULLUP);
  ioport_set_pin_sense_mode(EXT1_PIN_IRQ, IOPORT_SENSE_FALLING);
  // Enable interrupt
  if (pio_handler_set_pin(EXT1_PIN_IRQ, PIO_IT_FALL_EDGE, &MCP251XFD_Ext1_INT_Handler) != 0) return ERR__CONFIGURATION;
  pio_enable_pin_interrupt(EXT1_PIN_IRQ);
  return ERR_OK;
}



//=============================================================================
// MCP251XFD_Ext1 INT interrupt handler
//=============================================================================
void MCP251XFD_Ext1_INT_Handler(uint32_t id, uint32_t mask)
{
  // OMG an interrupt occur !!!!
  nop();
}





//**********************************************************************************
//=============================================================================
// MCP251XFD_Ext1 INT0 (GPIO0) pin configuration for the ATSAMV71
//=============================================================================
eERRORRESULT MCP251XFD_Ext1_Int0Gpio0PinInit_V71(MCP251XFD *pComp)
{
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  // INT0 (Interrupt for TX) from MCP251xFD board is linked to USART_RX pin of EXT1 extension header (pin 13 as PB00)
  if (pComp->UserDriverData == NULL) return ERR__PARAMETER_ERROR;

  MCP251XFD_Config* Conf = (MCP251XFD_Config*)(pComp->UserDriverData);
  switch (Conf->GPIO0PinMode)
  {
    case MCP251XFD_PIN_AS_GPIO0_IN: // As input on the device then as output on the microcontroller
      {
        ioport_set_pin_dir(EXT1_PIN_13, IOPORT_DIR_OUTPUT);
        ioport_set_pin_level(EXT1_PIN_13, IOPORT_PIN_LEVEL_LOW);
      }
      break;
    case MCP251XFD_PIN_AS_GPIO0_OUT: // As output on the device then as input on the microcontroller
      {
        ioport_set_pin_dir(EXT1_PIN_13, IOPORT_DIR_INPUT);
        ioport_set_pin_mode(EXT1_PIN_13, IOPORT_MODE_PULLUP);
        ioport_set_pin_sense_mode(EXT1_PIN_13, IOPORT_SENSE_FALLING);
      }
      break;
    case MCP251XFD_PIN_AS_INT0_TX: // As interrupt on the device then as input with interruption on the microcontroller
      {
        ioport_set_pin_dir(EXT1_PIN_13, IOPORT_DIR_INPUT);
        ioport_set_pin_mode(EXT1_PIN_13, IOPORT_MODE_PULLUP);
        ioport_set_pin_sense_mode(EXT1_PIN_13, IOPORT_SENSE_FALLING);
        // Enable interrupt
        if (pio_handler_set_pin(EXT1_PIN_13, PIO_IT_FALL_EDGE, &MCP251XFD_Ext1_INT0_Handler) != 0) return ERR__CONFIGURATION;
        pio_enable_pin_interrupt(EXT1_PIN_13);
      }
      break;
    case MCP251XFD_PIN_AS_XSTBY:
      {

      }
      break;
  }
  return ERR_OK;
}



//=============================================================================
// MCP251XFD_Ext1 INT0 interrupt handler
//=============================================================================
void MCP251XFD_Ext1_INT0_Handler(uint32_t id, uint32_t mask)
{
  // OMG an interrupt occur !!!!
  nop();
}





//**********************************************************************************
//=============================================================================
// MCP251XFD_Ext1 INT1 (GPIO1) pin configuration for the ATSAMV71
//=============================================================================
eERRORRESULT MCP251XFD_Ext1_Int1Gpio1PinInit_V71(MCP251XFD *pComp)
{
  // INT1 (Interrupt for RX) from MCP251xFD board is linked to USART_TX pin of EXT1 extension header (pin 14 as PB01)
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  if (pComp->UserDriverData == NULL) return ERR__PARAMETER_ERROR;

  MCP251XFD_Config* Conf = (MCP251XFD_Config*)(pComp->UserDriverData);
  switch (Conf->GPIO1PinMode)
  {
    case MCP251XFD_PIN_AS_GPIO1_IN: // As Input on the device then as output on the microcontroller
      {
        ioport_set_pin_dir(EXT1_PIN_14, IOPORT_DIR_OUTPUT);
        ioport_set_pin_level(EXT1_PIN_14, IOPORT_PIN_LEVEL_LOW);
      }
      break;
    case MCP251XFD_PIN_AS_GPIO1_OUT: // As output on the device then as input on the microcontroller
      {
        ioport_set_pin_dir(EXT1_PIN_14, IOPORT_DIR_INPUT);
        ioport_set_pin_mode(EXT1_PIN_14, IOPORT_MODE_PULLUP);
        ioport_set_pin_sense_mode(EXT1_PIN_14, IOPORT_SENSE_FALLING);
      }
      break;
    case MCP251XFD_PIN_AS_INT1_RX: // As interrupt on the device then as input with interruption on the microcontroller
      {
        ioport_set_pin_dir(EXT1_PIN_14, IOPORT_DIR_INPUT);
        ioport_set_pin_mode(EXT1_PIN_14, IOPORT_MODE_PULLUP);
        ioport_set_pin_sense_mode(EXT1_PIN_14, IOPORT_SENSE_FALLING);
        // Enable interrupt
        if (pio_handler_set_pin(EXT1_PIN_14, PIO_IT_FALL_EDGE, &MCP251XFD_Ext1_INT1_Handler) != 0) return ERR__CONFIGURATION;
        pio_enable_pin_interrupt(EXT1_PIN_14);
      }
      break;
  }
  return ERR_OK;
}



//=============================================================================
// MCP251XFD_Ext1 INT1 interrupt handler
//=============================================================================
void MCP251XFD_Ext1_INT1_Handler(uint32_t id, uint32_t mask)
{
  // OMG an interrupt occur !!!!
  nop();
}





//**********************************************************************************************************************************************************
//=============================================================================
// MCP251XFD_Ext2 INT pin configuration for the ATSAMV71
//=============================================================================
eERRORRESULT MCP251XFD_Ext2_IntPinInit_V71(MCP251XFD *pComp)
{
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  // INT from MCP251xFD board is linked to IRQ pin of EXT2 extension header (pin 9 as PA02)

  // Here it's always an input pin from the microcontroller side
  ioport_set_pin_dir(EXT2_PIN_IRQ, IOPORT_DIR_INPUT);
  ioport_set_pin_mode(EXT2_PIN_IRQ, IOPORT_MODE_PULLUP);
  ioport_set_pin_sense_mode(EXT2_PIN_IRQ, IOPORT_SENSE_FALLING);
  // Enable interrupt
  if (pio_handler_set_pin(EXT2_PIN_IRQ, PIO_IT_FALL_EDGE, &MCP251XFD_Ext2_INT_Handler) != 0) return ERR__CONFIGURATION;
  pio_enable_pin_interrupt(EXT2_PIN_IRQ);
  return ERR_OK;
}



//=============================================================================
// MCP251XFD_Ext2 INT interrupt handler
//=============================================================================
void MCP251XFD_Ext2_INT_Handler(uint32_t id, uint32_t mask)
{
  // OMG an interrupt occur !!!!
  nop();
}





//**********************************************************************************
//=============================================================================
// MCP251XFD_Ext2 INT0 (GPIO0) pin configuration for the ATSAMV71
//=============================================================================
eERRORRESULT MCP251XFD_Ext2_Int0Gpio0PinInit_V71(MCP251XFD *pComp)
{
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  // INT0 (Interrupt for TX) from MCP251xFD board is linked to USART_RX pin of EXT2 extension header (pin 13 as PA21)
  if (pComp->UserDriverData == NULL) return ERR__PARAMETER_ERROR;

  MCP251XFD_Config* Conf = (MCP251XFD_Config*)(pComp->UserDriverData);
  switch (Conf->GPIO0PinMode)
  {
    case MCP251XFD_PIN_AS_GPIO0_IN: // As input on the device then as output on the microcontroller
      {
        ioport_set_pin_dir(EXT2_PIN_13, IOPORT_DIR_OUTPUT);
        ioport_set_pin_level(EXT2_PIN_13, IOPORT_PIN_LEVEL_LOW);
      }
      break;
    case MCP251XFD_PIN_AS_GPIO0_OUT: // As output on the device then as input on the microcontroller
      {
        ioport_set_pin_dir(EXT2_PIN_13, IOPORT_DIR_INPUT);
        ioport_set_pin_mode(EXT2_PIN_13, IOPORT_MODE_PULLUP);
        ioport_set_pin_sense_mode(EXT2_PIN_13, IOPORT_SENSE_FALLING);
      }
      break;
    case MCP251XFD_PIN_AS_INT0_TX: // As interrupt on the device then as input with interruption on the microcontroller
      {
        ioport_set_pin_dir(EXT2_PIN_13, IOPORT_DIR_INPUT);
        ioport_set_pin_mode(EXT2_PIN_13, IOPORT_MODE_PULLUP);
        ioport_set_pin_sense_mode(EXT2_PIN_13, IOPORT_SENSE_FALLING);
        // Enable interrupt
        if (pio_handler_set_pin(EXT2_PIN_13, PIO_IT_FALL_EDGE, &MCP251XFD_Ext2_INT0_Handler) != 0) return ERR__CONFIGURATION;
        pio_enable_pin_interrupt(EXT2_PIN_13);
      }
      break;
    case MCP251XFD_PIN_AS_XSTBY:
      {

      }
      break;
  }
  return ERR_OK;
}



//=============================================================================
// MCP251XFD_Ext2 INT0 interrupt handler
//=============================================================================
void MCP251XFD_Ext2_INT0_Handler(uint32_t id, uint32_t mask)
{
  // OMG an interrupt occur !!!!
  nop();
}





//**********************************************************************************
//=============================================================================
// MCP251XFD_Ext2 INT1 (GPIO1) pin configuration for the ATSAMV71
//=============================================================================
eERRORRESULT MCP251XFD_Ext2_Int1Gpio1PinInit_V71(MCP251XFD *pComp)
{
  // INT1 (Interrupt for RX) from MCP251xFD board is linked to USART_TX pin of EXT2 extension header (pin 14 as PB04)
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  if (pComp->UserDriverData == NULL) return ERR__PARAMETER_ERROR;

  MCP251XFD_Config* Conf = (MCP251XFD_Config*)(pComp->UserDriverData);
  switch (Conf->GPIO1PinMode)
  {
    case MCP251XFD_PIN_AS_GPIO1_IN: // As Input on the device then as output on the microcontroller
      {
        ioport_set_pin_dir(EXT2_PIN_14, IOPORT_DIR_OUTPUT);
        ioport_set_pin_level(EXT2_PIN_14, IOPORT_PIN_LEVEL_LOW);
      }
      break;
    case MCP251XFD_PIN_AS_GPIO1_OUT: // As output on the device then as input on the microcontroller
      {
        ioport_set_pin_dir(EXT2_PIN_14, IOPORT_DIR_INPUT);
        ioport_set_pin_mode(EXT2_PIN_14, IOPORT_MODE_PULLUP);
        ioport_set_pin_sense_mode(EXT2_PIN_14, IOPORT_SENSE_FALLING);
      }
      break;
    case MCP251XFD_PIN_AS_INT1_RX: // As interrupt on the device then as input with interruption on the microcontroller
      {
        ioport_set_pin_dir(EXT2_PIN_14, IOPORT_DIR_INPUT);
        ioport_set_pin_mode(EXT2_PIN_14, IOPORT_MODE_PULLUP);
        ioport_set_pin_sense_mode(EXT2_PIN_14, IOPORT_SENSE_FALLING);
        // Enable interrupt
        if (pio_handler_set_pin(EXT2_PIN_14, PIO_IT_FALL_EDGE, &MCP251XFD_Ext2_INT1_Handler) != 0) return ERR__CONFIGURATION;
        pio_enable_pin_interrupt(EXT2_PIN_14);
      }
      break;
  }
  return ERR_OK;
}



//=============================================================================
// MCP251XFD_Ext2 INT1 interrupt handler
//=============================================================================
void MCP251XFD_Ext2_INT1_Handler(uint32_t id, uint32_t mask)
{
  // OMG an interrupt occur !!!!
  nop();
}





//**********************************************************************************************************************************************************
//=============================================================================
// MCP251XFD SPI driver interface configuration for the ATSAMV71
//=============================================================================
eERRORRESULT MCP251XFD_InterfaceInit_V71(void *pIntDev, uint8_t chipSelect, const uint32_t sckFreq)
{
  if (pIntDev == NULL) return ERR__SPI_PARAMETER_ERROR;
  Spi *SPI_Ext = (Spi *)pIntDev;

  ioport_set_pin_mode(SPI0_NPCS1_GPIO, SPI0_NPCS1_FLAGS);
  ioport_disable_pin(SPI0_NPCS1_GPIO);
  ioport_set_pin_mode(SPI0_NPCS3_GPIO, SPI0_NPCS3_FLAGS);
  ioport_disable_pin(SPI0_NPCS3_GPIO);

  //--- Configure an SPI peripheral ---
  spi_enable_clock(SPI_Ext);
  spi_disable(SPI_Ext);
  if (SPIconfigured == false)
  {
    spi_reset(SPI_Ext);
    spi_set_lastxfer(SPI_Ext);
    spi_set_master_mode(SPI_Ext);
    spi_disable_mode_fault_detect(SPI_Ext); // For multimaster SPI bus. Not used here
    spi_disable_peripheral_select_decode(SPI_Ext);
    spi_set_variable_peripheral_select(SPI_Ext);
    spi_enable_tx_on_rx_empty(SPI_Ext);
  }
  spi_set_clock_polarity(SPI_Ext, chipSelect, SPI_CLK_POLARITY);
  spi_set_clock_phase(SPI_Ext, chipSelect, SPI_CLK_PHASE);
  spi_set_bits_per_transfer(SPI_Ext, chipSelect, SPI_CSR_BITS_8_BIT);
  spi_configure_cs_behavior(SPI_Ext, chipSelect, SPI_CS_KEEP_LOW);
//  spi_configure_cs_behavior(SPI_Ext, chipSelect, SPI_CS_RISE_FORCED);
//  spi_configure_cs_behavior(SPI_Ext, chipSelect, SPI_CS_RISE_NO_TX);

  int16_t div = spi_calc_baudrate_div(sckFreq, sysclk_get_peripheral_hz());
  if (div < 0) return ERR__SPI_CONFIG_ERROR;
  spi_set_baudrate_div(SPI_Ext, chipSelect, (uint8_t)div);
  spi_set_transfer_delay(SPI_Ext, chipSelect, SPI_DLYBS, SPI_DLYBCT);
//  spi_set_delay_between_chip_select(SPI_Ext, SPI_DLYBCS);

  spi_enable(SPI_Ext);
  SPIconfigured = true;
  return ERR_OK;
}



//=============================================================================
// MCP251XFD SPI transfer data for the ATSAMV71
//=============================================================================
eERRORRESULT MCP251XFD_InterfaceTransfer_V71(void *pIntDev, uint8_t chipSelect, uint8_t *txData, uint8_t *rxData, size_t size)
{
  if (pIntDev == NULL) return ERR__SPI_PARAMETER_ERROR;
  if (txData == NULL) return ERR__SPI_PARAMETER_ERROR;
  Spi *SPI_Ext = (Spi *)pIntDev;
  uint8_t DataRead;
  
  __disable_irq();
  uint32_t Timeout;
  while (size > 0)
  {
    //--- Transmit data ---
    Timeout = TIMEOUT_SPI_INTERFACE;
    while (!(SPI_Ext->SPI_SR & SPI_SR_TDRE))   // SPI Tx ready ?
      if (!Timeout--) return ERR__SPI_TIMEOUT; // Timeout ? return an error

    uint32_t value = SPI_TDR_TD(*txData) | SPI_TDR_PCS(spi_get_pcs(chipSelect));
    if (size == 1) value |= SPI_TDR_LASTXFER;
    SPI_Ext->SPI_TDR = value;
    txData++;

    //--- Receive data ---
    Timeout = TIMEOUT_SPI_INTERFACE;
    while (!(SPI_Ext->SPI_SR & SPI_SR_RDRF))   // SPI Rx ready ?
      if (!Timeout--) return ERR__SPI_TIMEOUT; // Timeout ? return an error

    DataRead = (uint8_t)(SPI_Ext->SPI_RDR & 0xFF);
    if (rxData != NULL) *rxData = DataRead;
    rxData++;

    size--;
  }
  SPI_Ext->SPI_CR |= SPI_CR_LASTXFER;
  __enable_irq();

  return ERR_OK;
}





//**********************************************************************************************************************************************************
//=============================================================================
// MCP251XFD_X get millisecond
//=============================================================================
uint32_t GetCurrentms_V71(void)
{
  return msCount;
}


//=============================================================================
// MCP251XFD_X compute CRC16-CMS
//=============================================================================
uint16_t ComputeCRC16_V71(const uint8_t* data, size_t size)
{
  return ComputeCRC16CMS(data, size);
}





//**********************************************************************************************************************************************************
//=============================================================================
// Calculate the SPI clock speed in Hz
//=============================================================================
uint32_t GetSPIClock(Spi *pSpi, uint8_t aCS)
{
  return sysclk_get_peripheral_hz() / ((pSpi->SPI_CSR[aCS] & SPI_CSR_SCBR_Msk) >> SPI_CSR_SCBR_Pos);
}





#ifdef USE_MCP251XFD_TOOLS
//=============================================================================
// Calculate the min time before the GPIO to change state (depend on SPI (SAMV71) and driver conf)
//=============================================================================
uint32_t CalculateMinTimeBeforeGPIOChangeState_V71(MCP251XFD *pComp, Spi *pSpi, uint8_t aCS)
{
  float PeriphClkHz = sysclk_get_peripheral_hz();
  float PeriphTime  = 1000000000.0f / PeriphClkHz;                                                        // 1/PeriphClk*(1/1ns) => (1/1ns)*PeriphClk = ns
  float SpiClkHz    = PeriphClkHz / (float)((pSpi->SPI_CSR[aCS] & SPI_CSR_SCBR_Msk) >> SPI_CSR_SCBR_Pos); // Get SCBR (PeriphClk div) register value
  float SpiTime     = 1000000000.0f / SpiClkHz;                                                           // 1/SpiClkHz*(1/1ns) => (1/1ns)*SpiClkHz = ns

  // Calculate delay before SPCK time
  float DLYBSval  = (float)((pSpi->SPI_CSR[aCS] & SPI_CSR_DLYBS_Msk) >> SPI_CSR_DLYBS_Pos);               // Get DLYBS register value
  float DLYBStime = (DLYBSval * PeriphTime) + (SpiTime / 2);                                              // DLYBStime = DLYBS*PeriphTime + Half-Period SpiClk

  // Calculate delay between consecutive transfer
  float DLYBCTval  = (float)((pSpi->SPI_CSR[aCS] & SPI_CSR_DLYBCT_Msk) >> SPI_CSR_DLYBCT_Pos);            // Get DLYBCT register value
  float DLYBCTtime = (DLYBCTval * PeriphTime * 32.0f);                                                    // DLYBCTtime = DLYBCT*(PeriphTime*32)

  // Calculate number of byte to send
  uint32_t BytesCount = 3;                                                                                // 3 bytes by default (1 Cmd + 1 SFR address + 1 byte to write)
  if ((pComp->DriverConfig & DRIVER_USE_SAFE_WRITE    ) > 0) BytesCount += 2;                             // If CRC read or safe write then 2 more for CRC
  if ((pComp->DriverConfig & DRIVER_USE_READ_WRITE_CRC) > 0) BytesCount += 1;                             // If CRC read or write CRC then 1 more for Length

  return (uint32_t)((BytesCount/*bytes*/ * 8/*bits*/) * SpiTime + DLYBStime + (DLYBCTtime * BytesCount/*Between bytes + end*/));
}
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