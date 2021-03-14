/*******************************************************************************
  File name:    MCANFunctions.h
  Author:       FMA
  Version:      1.0
  Date (d/m/y): 29/04/2020
  Description:  Some functions to show on console for the DEMO

  History :
*******************************************************************************/
#ifndef MCANFUNCTIONS_H_
#define MCANFUNCTIONS_H_
//=============================================================================

//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#include "stdio.h"
#include <stdarg.h>
#include "MCP251XFD.h"
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------





/* mcan_filter_setting */
#define MCAN_RX_STANDARD_FILTER_INDEX_0					0
#define MCAN_RX_STANDARD_FILTER_INDEX_1					1
#define MCAN_RX_FRAME_CHECK_CHANNEL						0x001
#define MCAN_ID_ARINC429_CHANNEL						0x0AC
#define MCAN_RX_STANDARD_FILTER_ID_0					0x45A
#define MCAN_RX_STANDARD_FILTER_ID_0_BUFFER_INDEX		2
#define MCAN_RX_STANDARD_FILTER_ID_1					0x469
#define MCAN_RX_EXTENDED_FILTER_INDEX_0					0
#define MCAN_RX_EXTENDED_FILTER_INDEX_1					1
#define MCAN_RX_EXTENDED_FILTER_ID_0					0x100000A5
#define MCAN_RX_EXTENDED_FILTER_ID_0_BUFFER_INDEX		1
#define MCAN_RX_EXTENDED_FILTER_ID_1					0x10000096





//=============================================================================
// Configure the MCAN1 device
//=============================================================================
void ConfigureMCAN1Device(void);

void mcan_set_standard_filter_0(void);


extern struct mcan_module mcan_instance;

#define MCAN_LOWEST_SIZE_SID     0x084
uint8_t TransmitMessageData[CONF_MCAN_ELEMENT_DATA_SIZE];

bool mcan_send_standard_message(uint32_t id_value, uint8_t *data, size_t data_size);





//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------
#endif /* MCANFUNCTIONS_H_ */