/*******************************************************************************
  File name:    MCANFunctions.c
  Author:       FMA
  Version:      1.0
  Date (d/m/y): 29/04/2020
  Description:  MCAN functions for the DEMO

  History :
*******************************************************************************/

//-----------------------------------------------------------------------------
#include "Main.h"
#include "MCANFunctions.h"
#include "MCP251XFD_V71InterfaceSync.h"
//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
#include "stdafx.h"
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------



// MCAN1 module instance
struct mcan_module mcan_instance;



//**********************************************************************************************************************************************************
//=============================================================================
// Configure the MCAN1 device
//=============================================================================
void ConfigureMCAN1Device(void)
{
  /* Initialize the module. */
  struct mcan_config config_mcan;
  mcan_get_config_defaults(&config_mcan);
  mcan_init(&mcan_instance, MCAN_MODULE, &config_mcan);

  mcan_enable_fd_mode(&mcan_instance);
  mcan_start(&mcan_instance);

  /* Enable interrupts for this MCAN module */
  irq_register_handler(MCAN1_INT0_IRQn, 1);
  mcan_enable_interrupt(&mcan_instance, MCAN_FORMAT_ERROR | MCAN_ACKNOWLEDGE_ERROR | MCAN_BUS_OFF);
  mcan_set_standard_filter_0();
}



void mcan_set_standard_filter_0(void)
{
  struct mcan_standard_message_filter_element sd_filter;

  mcan_get_standard_message_filter_element_default(&sd_filter);
  sd_filter.S0.bit.SFID2 = MCAN_RX_STANDARD_FILTER_ID_0_BUFFER_INDEX;
  sd_filter.S0.bit.SFID1 = MCAN_RX_STANDARD_FILTER_ID_0;
  sd_filter.S0.bit.SFEC = MCAN_STANDARD_MESSAGE_FILTER_ELEMENT_S0_SFEC_STRXBUF_Val;

  mcan_set_rx_standard_filter(&mcan_instance, &sd_filter,
  MCAN_RX_STANDARD_FILTER_INDEX_0);
  mcan_enable_interrupt(&mcan_instance, MCAN_RX_BUFFER_NEW_MESSAGE);
}





bool mcan_send_standard_message(uint32_t id_value, uint8_t *data, size_t data_size)
{
  uint32_t i;
  struct mcan_tx_element tx_element;
  uint32_t status = mcan_tx_get_fifo_queue_status(&mcan_instance);

  /* check if the FIFO for standard message is full */
  if (status & MCAN_TXFQS_TFQF) return false;

  mcan_get_tx_buffer_element_defaults(&tx_element);
  tx_element.T0.reg |= MCAN_TX_ELEMENT_T0_STANDARD_ID(id_value);
  tx_element.T1.bit.DLC = data_size;
  if (data != NULL)
  {
    for (i = 0; i < 8; i++)
    {
      tx_element.data[i] = *data;
      data++;
    }
  }
  else if (data_size > 0) return false;

  if (((mcan_instance.hw->MCAN_TXFQS >> 0) & 0x3F) == 0) return false;
  uint32_t PutIndex  = (mcan_instance.hw->MCAN_TXFQS >> 16) & 0x1F;
  mcan_set_tx_buffer_element(&mcan_instance, &tx_element, PutIndex);
  mcan_tx_transfer_request(&mcan_instance, 1 << PutIndex);
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