/**
 * \file
 *
 * \brief SAMV71 Xplained Ultra board configuration
 *
 * Copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H





//=============================================================================
// Demo application configuration
//=============================================================================
//! Use EXT1 INT pin to check CAN and device status
#define APP_USE_EXT1_INT_PIN
//! Use EXT1 RX INT pin to check FIFO status
#define APP_USE_EXT1_INT1_PIN

//! Use EXT1 INT pin to check CAN and device status
#define APP_USE_EXT2_INT_PIN
//! Use EXT1 RX INT pin to check FIFO status
#define APP_USE_EXT2_INT1_PIN





//=============================================================================
// Board configuration
//=============================================================================
/* Enable ICache and DCache */
#define CONF_BOARD_ENABLE_CACHE_AT_INIT

/** Enable Com Port. */
#define CONF_BOARD_UART_CONSOLE

/** SPI MACRO definition */
#define CONF_BOARD_SPI

/** CAN1 MACRO definition */
#define CONF_BOARD_CAN1





#endif /* CONF_BOARD_H */
